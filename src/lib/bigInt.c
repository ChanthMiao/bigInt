#include "bigInt.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/*
 * 以下宏定义涉及到内存的使用细节，可按需修改。
 * 注意，不要单独修改任何宏定义，否则会引发运行时内存错误。
 * SEG_MEM_BYTES：单节数据段占用的实际内存大小。
 * SEG_LOGIC_BYTES：单节数据段占用的实际内存大小。
 */
#define SEG_MEM_BYTES (sizeof(seg_t))
#define SEG_LOGIC_BYTES (SEG_MEM_BYTES / ((size_t)4u) * ((size_t)3u))
#define SEG_CHAR_BYTES (((size_t)2u) * SEG_LOGIC_BYTES)
#define SEG_MOD (((seg_t)1) << (SEG_LOGIC_BYTES * 8u))
#define SEG_N_LIMIT (((size_t)-1u) / SEG_MEM_BYTES / ((size_t)2u))
#define STATIC_CHAR_BUF_BYTES ((size_t)256)
#define STATIC_SEG_BUF_BYTES ((size_t)512)
//预留字符栈空间。
static char internal_char_buf_0[STATIC_CHAR_BUF_BYTES];
static char internal_char_buf_1[STATIC_CHAR_BUF_BYTES];
//内部数值栈空间。
static seg_t trash_segs[2u];
static seg_t internal_buf_0[STATIC_SEG_BUF_BYTES];
static seg_t internal_buf_1[STATIC_SEG_BUF_BYTES];
static seg_t internal_buf_2[STATIC_SEG_BUF_BYTES];
static seg_t internal_buf_3[STATIC_SEG_BUF_BYTES];
static seg_t internal_buf_4[STATIC_SEG_BUF_BYTES];
static seg_t internal_buf_5[STATIC_SEG_BUF_BYTES];
static seg_t internal_buf_6[STATIC_SEG_BUF_BYTES];
typedef struct _Hex {
    unsigned flag;          //标记位。
    size_t length;          //转换成数值后所需要的连续字节数。
    const char *format_str; //格式化hex字符串（无0x前缀）。
} Hex;
// Hex标记位。
#define HEX_NEGATIVE 0x1u
#define HEX_INSTACK 0x2u
#define Hex_NULL 0x4u
/*
 * 检查给定字符串，如果符合hex形式,返回合法Hex结构体。
 * 注：关于Hex形式字符串定义——大小写无关，不含空白字符的，16进制字符串。
 */
BIGINT_PUBLIC(Hex) bigInt_ValueStrCheck(const char *str)
{
    Hex rt = {.flag = 0u, .length = 0u, .format_str = NULL};
    if (str == NULL) { //参数为NULL，返回带NULL标记的Hex。
        rt.flag = Hex_NULL;
        return rt;
    }
    size_t clen = 0u;
    const char *header = str;
    if (*header == '-') { //处理正负号。
        rt.flag |= HEX_NEGATIVE;
        header++;
    } else if (*header == '+') {
        header++;
    }
    if (*header == '0' && (tolower((unsigned char)*(header + 1u)) == 'x')) {
        header += 2u; //如果存在0x前缀，跳过。
    }
    const char *tail = header;
    if (*header == '0') { //处理首部冗余的0。
        while (*tail != '\0' && *tail == '0') {
            tail++;
        }
        if (*tail == '\0') {
            header = --tail;
        } else {
            header = tail;
        }
    }
    tail = header;          //重置尾指针。
    while (*tail != '\0') { //字符计数。
        if (isxdigit((unsigned char)*tail)) {
            clen++;
            tail++;
        } else {
            break;
        }
    }
    if (tail == header) { //参数为无效字符串，返回带NULL标记的Hex。
        rt.flag = Hex_NULL;
        return rt;
    }
    char *tmp = NULL;                   //视有效字符串长度分配内存。
    if (clen < STATIC_CHAR_BUF_BYTES) { //长度在可接受范围内，指定栈内buf。
        rt.flag |= HEX_INSTACK;
        tmp = internal_char_buf_0;
    } else {
        tmp = (char *)malloc(clen); //过长，堆内申请。
    }
    memset(internal_char_buf_0, 0, clen);
    for (size_t i = 0; i < clen; i++) { //格式化拷贝。
        tmp[i] = (char)toupper((unsigned char)header[i]);
    }
    tmp[clen] = '\0';
    rt.format_str = tmp;
    rt.length = clen / 2u + (clen % 2u);
    return rt; //返回有效Hex结构体。
}
BIGINT_PUBLIC(int) bigInt_Opnd_Check(const bigInt *a, const bigInt *b)
{
    if (a == NULL || b == NULL) {
        return 1;
    } else if (a->capacity == 0u || b->capacity == 0u || a->data == NULL || b->data == NULL) {
        return 1;
    } else if ((a->flag & (BIGINT_NAN | BIGINT_OUT_RANGE | BIGINT_CAPACITY_ERR)) &&
               (b->flag & (BIGINT_NAN | BIGINT_OUT_RANGE | BIGINT_CAPACITY_ERR))) {
        return 2;
    } else {
        return 0;
    }
}
BIGINT_PUBLIC(bigInt *) bigInt_PreInit(bigInt *target, const size_t capacity, seg_t *data_ptr)
{
    if (capacity > SEG_N_LIMIT || capacity == 0) {
        return NULL; //容量申请超过安全限制，请求非法。
    }
    if (target == NULL) { //未指定结构体地址，自动申请内存。
        target = (bigInt *)malloc(sizeof(bigInt));
        target->flag = 0u;
    } else { //使用指定结构体，并设置标记位。
        target->flag = 0u;
        target->flag |= BIGINT_STRUCT_NOALLOC;
    }
    if (data_ptr == NULL) { //未指定数据段地址，自动申请。
        target->data = (seg_t *)calloc(capacity, SEG_MEM_BYTES);
    } else {                     //使用指定数据段，并设置标记位。
        target->data = data_ptr; //内存信任，危险操作！！！
        target->flag |= BIGINT_PTR_NOALLOC;
    }
    memset(target->data, 0, capacity); //清零。
    target->capacity = capacity;
    target->length = 0u;
    return target;
}
/*
从给定hex字符串生成大整数。
*/
BIGINT_PUBLIC(bigInt *) bigInt_Init_str(bigInt *target, const size_t capacity, seg_t *data_ptr, const char *valueStr)
{
    Hex tmp = bigInt_ValueStrCheck(valueStr);
    if (tmp.flag & Hex_NULL) {
        return NULL;
    }
    if (target == NULL) { //未指定结构体地址，自动申请内存。
        target = (bigInt *)malloc(sizeof(bigInt));
        target->flag = 0u;
    } else { //使用指定结构体，并设置标记位。
        target->flag = 0u;
        target->flag |= BIGINT_STRUCT_NOALLOC;
    }
    if (tmp.flag & HEX_NEGATIVE) { //判断符号。
        target->flag |= BIGINT_NEGATIVE;
    }
    target->length = tmp.length / SEG_LOGIC_BYTES + ((tmp.length % SEG_LOGIC_BYTES) ? 1u : 0u); //计算数值分段。
    size_t safe_segs = target->length + 2u;
    if (safe_segs > SEG_N_LIMIT) { //预估突破安全值，提前退出。
        if (!(target->flag & BIGINT_STRUCT_NOALLOC)) {
            free((void *)target);
        }
        if (!(tmp.flag & HEX_INSTACK)) {
            free((void *)tmp.format_str);
        }
        return NULL;
    } else if (safe_segs < capacity) {
        safe_segs = capacity; //使用自定义容量。
    }
    if (data_ptr == NULL) { //未指定数据段地址，自动申请。
        target->data = (seg_t *)calloc(safe_segs, SEG_MEM_BYTES);
    } else {                     //使用指定数据段，并设置标记位。
        target->data = data_ptr; //内存信任，危险操作！！！
        target->flag |= BIGINT_PTR_NOALLOC;
    }
    target->capacity = safe_segs;
    memset(target->data, 0, safe_segs);
    memset(internal_char_buf_1, 0, SEG_CHAR_BYTES);
    size_t char_length = strlen(tmp.format_str);
    const char *curr = tmp.format_str;
    seg_t *seg_ptr = (target->data) + (target->length) - 1u;
    if (char_length % SEG_CHAR_BYTES) { //处理不对齐的首部。
        strncpy(internal_char_buf_1, curr, char_length % SEG_CHAR_BYTES);
        *(seg_ptr) = strtoll(internal_char_buf_1, NULL, 16);
        curr += (char_length % SEG_CHAR_BYTES);
        seg_ptr--;
    }
    while ((*curr) != '\0') { //连续赋值。
        strncpy(internal_char_buf_1, curr, SEG_CHAR_BYTES);
        *(seg_ptr) = strtoll(internal_char_buf_1, NULL, 16);
        curr += SEG_CHAR_BYTES;
        seg_ptr--;
    }
    if (!(tmp.flag & HEX_INSTACK)) {
        free((void *)tmp.format_str); //堆上内存释放。
    }
    return target;
}
/*冻结指定大整数结构体，返回标记位。*/
BIGINT_PUBLIC(flag_t) bigInt_Lock(bigInt *target)
{
    flag_t pre = target->flag;
    target->flag |= BIGINT_READONLY;
    return pre;
}
/*解结指定大整数结构体。*/
BIGINT_PUBLIC(void) bigInt_Unlock(bigInt *target) { target->flag &= (~BIGINT_READONLY); }
/*重置指定大数结构体为0。成功，返回capacity。失败返回0。*/
BIGINT_PUBLIC(size_t) bigInt_Clear(bigInt *target)
{
    if (target == NULL) {
        return 0;
    } else if (target->data == NULL || target->capacity == 0u) {
        return 0;
    } else if (target->flag & BIGINT_READONLY) {
        return 0;
    } else {
        memset(target->data, 0, (target->capacity) * (SEG_MEM_BYTES));
        target->flag = 0u;
        target->length = 1u;
        return target->capacity;
    }
}
/*忽略BIGINT_READONLY，删除指定的大整数。*/
BIGINT_PUBLIC(void) bigInt_DELETE(const bigInt *target)
{
    if (target == NULL) {
        return;
    }
    //通过标记位识别到自动申请的内存，释放。
    if (target->data != NULL && target->capacity > 0u && target->capacity <= SEG_N_LIMIT) {
        memset(target->data, 0, (target->capacity) * (SEG_MEM_BYTES));
        if (!(target->flag & BIGINT_PTR_NOALLOC)) {
            free((void *)target->data);
        }
    }
    if (!(target->flag & BIGINT_STRUCT_NOALLOC)) {
        free((void *)target); //通过标记位识别到自动申请的内存，释放。
    }
}
/*按指定进制输出大整数。*/
BIGINT_PUBLIC(void) bigInt_Print(const bigInt *target, int base, FILE *dst)
{
    if (target == NULL) {
        fprintf(dst, "NULL");
    } else if (target->data == NULL || target->capacity == 0u) {
        fprintf(dst, "NULL");
    } else if ((target->flag) & BIGINT_NAN) {
        fprintf(dst, "Nan");
        return;
    } else if ((target->flag) & BIGINT_OUT_RANGE) {
        fprintf(dst, "Out of range error");
        return;
    } else if ((target->flag) & BIGINT_NEGATIVE) {
        fprintf(dst, "-");
    }
    const seg_t *curr = (target->data) + (target->length) - 1u;
    if (base == 8) {
        while (curr >= (target->data)) {
            fprintf(dst, "%llo", *curr);
            curr--;
        }
    } else {
        while (curr >= (target->data)) {
            fprintf(dst, "%llX", *curr);
            curr--;
        }
    }
    return;
}
/*比较绝对值大小。参数非法则返回-2。*/
BIGINT_PUBLIC(int) bigInt_Comp_Abs(const bigInt *a, const bigInt *b)
{
    if (a == NULL || b == NULL || a->data == NULL || b->data == NULL) {
        return -2;
    } else if (a->capacity == 0u || a->length == 0u || b->capacity == 0u || b->length == 0u) {
        return -2;
    } else if ((a->flag & BIGINT_NAN) || (b->flag & BIGINT_NAN)) {
        return -2;
    } else if (a->length > b->length) {
        return 1;
    } else if (a->length < b->length) {
        return -1;
    } else {
        const seg_t *ptr_a = a->data + a->length - 1u;
        const seg_t *ptr_b = b->data + b->length - 1u;
        for (size_t i = 0; i < a->length; i++) {
            if ((*ptr_a) > (*ptr_b)) {
                return 1;
            } else if ((*ptr_a) < (*ptr_b)) {
                return -1;
            } else {
                ptr_a--;
                ptr_b--;
            }
        }
        return 0;
    }
}
/*比较大小。参数非法则返回-2。*/
BIGINT_PUBLIC(int) bigInt_Comp(const bigInt *a, const bigInt *b)
{
    int abs_comp_code = bigInt_Comp_Abs(a, b);
    if (abs_comp_code == -2) {
        return abs_comp_code;
    } else if ((a->flag & BIGINT_NEGATIVE) == (b->flag & BIGINT_NEGATIVE)) {
        if (a->flag & BIGINT_NEGATIVE) {
            return -abs_comp_code;
        } else {
            return abs_comp_code;
        }
    } else {
        if (a->flag & BIGINT_NEGATIVE) {
            return -1;
        } else {
            return 1;
        }
    }
}
BIGINT_PUBLIC(size_t) bigInt_Assume_Capacity(const char mode, const bigInt *a, const bigInt *b)
{ //假定源操作数均为合法值，不做检查。
    switch (mode) {
    case '+':
        if ((a->flag & BIGINT_NEGATIVE) == (b->flag & BIGINT_NEGATIVE)) {
            return (a->length > b->length) ? (a->length + 3u) : (b->length + 3u);
        } else {
            return (a->length > b->length) ? (a->length + 2u) : (b->length + 2u);
        }
        break;
    case '-':
        if ((a->flag & BIGINT_NEGATIVE) == (b->flag & BIGINT_NEGATIVE)) {
            return (a->length > b->length) ? (a->length + 2u) : (b->length + 2u);
        } else {
            return (a->length > b->length) ? (a->length + 3u) : (b->length + 3u);
        }
        break;
    case '*':
        if ((a->length == 1u && *(a->data) == 0) || (b->length == 1u && *(b->data) == 0)) {
            return 3u;
        } else {
            return (a->length + b->length);
        }
        break;
    default: // TODO add more mode support.
        break;
    }
    return 0;
}
/*
大整数加法 。
*/
BIGINT_PUBLIC(bigInt *) bigInt_Add(bigInt *sum, const bigInt *a, const bigInt *b)
{ //检查源操作数
    int opnd_status = bigInt_Opnd_Check(a, b);
    if (opnd_status == 1) { //存在无效操作数。
        return NULL;
    } else if (opnd_status == 2) { //操作数含非法值。
        if (sum == NULL) {
            sum = bigInt_PreInit(NULL, 2u, trash_segs);
        } else if (sum->data == NULL || sum->capacity == 0u) {
            sum->data = trash_segs;
            sum->flag &= BIGINT_STRUCT_NOALLOC;
            sum->flag |= BIGINT_PTR_NOALLOC;
        }
        sum->capacity = 2u;
        sum->length = 0u;
        sum->flag |= (a->flag & (BIGINT_NAN | BIGINT_OUT_RANGE | BIGINT_CAPACITY_ERR));
        sum->flag |= (b->flag & (BIGINT_NAN | BIGINT_OUT_RANGE | BIGINT_CAPACITY_ERR));
        return sum;
    }
    if (sum == NULL || sum->flag & BIGINT_READONLY) { //未提供可用目标操作数，自动申请。capacity估算安全至上。
        size_t safe_capacity = bigInt_Assume_Capacity('+', a, b);
        sum = bigInt_PreInit(NULL, safe_capacity, NULL);
    } else { //否则，信任用户提供的目标操作数。用户必须谨慎使用预分配功能，避免不必要的内存读写错误。
        sum->flag &= (BIGINT_STRUCT_NOALLOC | BIGINT_PTR_NOALLOC); //仅保留内存标记。
    }
    const bigInt *x = NULL;
    const bigInt *y = NULL;
    if (b->length > a->length) {
        x = b;
        y = a;
    } else {
        x = a;
        y = b;
    }
    const seg_t *seg_ptr_x = x->data;
    const seg_t *seg_ptr_y = y->data;
    seg_t *seg_ptr_dst = sum->data;
    const seg_t *border = seg_ptr_y + y->length;
    seg_t carry = 0u;
    sum->flag |= (x->flag & BIGINT_NEGATIVE);                         //正负号追随绝对值较大的一方。
    if ((x->flag & BIGINT_NEGATIVE) == (y->flag & BIGINT_NEGATIVE)) { //同号加法。
        while (seg_ptr_y < border) {
            (*seg_ptr_dst) = (*seg_ptr_x) + (*seg_ptr_y) + carry;
            carry = (*seg_ptr_dst) / SEG_MOD;
            (*seg_ptr_dst) %= SEG_MOD;
            seg_ptr_dst++;
            seg_ptr_x++;
            seg_ptr_y++;
        }
        border = x->data + x->length;
        while (seg_ptr_x < border) {
            (*seg_ptr_dst) = (*seg_ptr_x) + carry;
            carry = (*seg_ptr_dst) / SEG_MOD;
            (*seg_ptr_dst) %= SEG_MOD;
            seg_ptr_dst++;
            seg_ptr_x++;
        }
        (*seg_ptr_dst) = carry;
        sum->length = (carry) ? (x->length + 1u) : x->length;
    } else {
        while (seg_ptr_y < border) {
            (*seg_ptr_dst) = (*seg_ptr_x) - (*seg_ptr_y) - carry;
            carry = (*seg_ptr_dst < 0) ? 1 : 0;
            (*seg_ptr_dst) = (*seg_ptr_dst < 0) ? (*seg_ptr_dst + SEG_MOD) : (*seg_ptr_dst);
            seg_ptr_dst++;
            seg_ptr_x++;
            seg_ptr_y++;
        }
        border = x->data + x->length;
        while (seg_ptr_x < border) {
            (*seg_ptr_dst) = (*seg_ptr_x) - carry;
            carry = (*seg_ptr_dst < 0) ? 1 : 0;
            (*seg_ptr_dst) = (*seg_ptr_dst < 0) ? (*seg_ptr_dst + SEG_MOD) : (*seg_ptr_dst);
            seg_ptr_dst++;
            seg_ptr_x++;
        }
        seg_ptr_dst--;
        sum->length = x->length;
        for (border = sum->data; seg_ptr_dst > border && *seg_ptr_dst == 0; seg_ptr_dst--) {
            sum->length--;
        }
    }
    return sum;
}
BIGINT_PUBLIC(bigInt *) bigInt_Sub(bigInt *dif, const bigInt *a, const bigInt *b)
{ //转换为加法。
    bigInt neg_b = *b;
    neg_b.flag ^= BIGINT_NEGATIVE;
    neg_b.flag |= BIGINT_STRUCT_NOALLOC;
    return bigInt_Add(dif, a, &neg_b);
}
BIGINT_PUBLIC(bigInt *) bigInt_Mul(bigInt *mul, const bigInt *a, const bigInt *b)
{ //检查源操作数。
    int opnd_status = bigInt_Opnd_Check(a, b);
    if (opnd_status == 1) { //存在无效操作数。
        return NULL;
    } else if (opnd_status == 2) { //操作数含非法值。
        if (mul == NULL) {
            mul = bigInt_PreInit(NULL, 2u, trash_segs);
        } else if (mul->data == NULL || mul->capacity == 0u) {
            mul->data = trash_segs;
            mul->flag &= BIGINT_STRUCT_NOALLOC;
            mul->flag |= BIGINT_PTR_NOALLOC;
            mul->capacity = 2u;
        }
        mul->length = 0u;
        mul->flag |= (a->flag & (BIGINT_NAN | BIGINT_OUT_RANGE | BIGINT_CAPACITY_ERR));
        mul->flag |= (b->flag & (BIGINT_NAN | BIGINT_OUT_RANGE | BIGINT_CAPACITY_ERR));
        return mul;
    }
    if (mul == NULL || mul->flag & BIGINT_READONLY) { //未提供可用目标操作数，自动申请。capacity估算安全至上。
        size_t safe_capacity = bigInt_Assume_Capacity('*', a, b);
        mul = bigInt_PreInit(NULL, safe_capacity, NULL);
    } else { //否则，信任用户提供的目标操作数。用户必须谨慎使用预分配功能，避免不必要的内存读写错误。
        mul->flag &= (BIGINT_STRUCT_NOALLOC | BIGINT_PTR_NOALLOC); //仅保留内存标记。
    }
    const bigInt *x = NULL;
    const bigInt *y = NULL;
    if (b->length > a->length) {
        x = b;
        y = a;
    } else {
        x = a;
        y = b;
    }
    if (y->length == 1u && *(y->data) == 0u) { // 0乘，短路返回0。
        mul->length = 1u;
        *(mul->data) = 0;
        return mul;
    }
    //确定符号。
    mul->flag |= (((x->flag & BIGINT_NEGATIVE) == (y->flag & BIGINT_NEGATIVE)) ? 0u : BIGINT_NEGATIVE);
    const seg_t *seg_ptr_x = NULL;
    const seg_t *seg_ptr_y = NULL;
    seg_t *seg_ptr_dst = mul->data;
    const seg_t *border_x = x->data + x->length;
    const seg_t *border_y = y->data + y->length;
    seg_t carry = 0;
    memset(mul->data, 0, mul->capacity * SEG_MEM_BYTES);
    for (seg_ptr_y = y->data; seg_ptr_y < border_y; seg_ptr_y++) {
        if (*seg_ptr_y != 0) {
            seg_ptr_dst += (seg_ptr_y - y->data);
            for (seg_ptr_x = x->data; seg_ptr_x < border_y; seg_ptr_x++, seg_ptr_dst++) {
                (*seg_ptr_dst) += ((*seg_ptr_x) * (*seg_ptr_y) + carry);
                carry = (*seg_ptr_dst) / SEG_MOD;
                (*seg_ptr_dst) %= SEG_MOD;
            }
            (*seg_ptr_dst) = carry;
        }
    }
    mul->length = (carry) ? (x->length + y->length) : (x->length + y->length - 1u);
    return mul;
}