#ifndef bigInt_H
#define bigInt_H
#ifdef __cplusplus
extern "C" {
#endif
#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif
#ifdef __WINDOWS__
/* 以下预处理用于处理windows平台下的动态库生成。（写法摘自cJSON）
BIGINT_HIDE_SYMBOLS - 当你不希望导出动态库时，使用这个宏定义。
BIGINT_EXPORT_SYMBOLS - 当你希望导出动态库时，使用这个宏定义(default)
BIGINT_IMPORT_SYMBOLS - 当你希望导入动态库时，使用这个宏定义
当然，对于支持可见性属性设置（默认对外可见）的*nix平台而言，可参照以下两种操作显在动态符号表显式隐藏符号。
1）-fvisibility=hidden (for gcc)
2）-xldscope=hidden (for sun cc)
（上述参数可写入CFLAGS）
尽管在默认情况下，*nix下动态库成员对外可见，你仍然可以使用宏定义BIGINT_API_VISIBILITY来显式设置可见性。
*/
#define BIGINT_CDECL __cdecl
#define BIGINT_STDCALL __stdcall
/* 默认开启windows下符号导出以避免出现动态库编译错误。 */
#if !defined(BIGINT_HIDE_SYMBOLS) && !defined(BIGINT_IMPORT_SYMBOLS) && !defined(BIGINT_EXPORT_SYMBOLS)
#define BIGINT_EXPORT_SYMBOLS
#endif
#if defined(BIGINT_HIDE_SYMBOLS)
#define BIGINT_PUBLIC(type) type BIGINT_STDCALL
#elif defined(BIGINT_EXPORT_SYMBOLS)
#define BIGINT_PUBLIC(type) __declspec(dllexport) type BIGINT_STDCALL
#elif defined(BIGINT_IMPORT_SYMBOLS)
#define BIGINT_PUBLIC(type) __declspec(dllimport) type BIGINT_STDCALL
#endif
#else /* !__WINDOWS__ */
#define BIGINT_CDECL
#define BIGINT_STDCALL
#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(BIGINT_API_VISIBILITY)
//默认可见，此处的gcc拓展可省略。
#define BIGINT_PUBLIC(type) __attribute__((visibility("default"))) type
#else
#define BIGINT_PUBLIC(type) type
#endif
#endif
#define BIGINT_NAN (1u << 0)
#define BIGINT_READONLY (1u << 1)
#define BIGINT_PTR_NOALLOC (1u << 2)
#define BIGINT_NEGATIVE (1u << 3)
#define BIGINT_STRUCT_NOALLOC (1u << 4)
#define BIGINT_OUT_RANGE (1u << 5)
#define BIGINT_CAPACITY_ERR (1u << 6)
#include <stdio.h>
//实际存储数据的分段类型。
typedef long long int seg_t;
//定义充当标记的数据类型。
typedef unsigned int flag_t;
//定义大整数数据类型。
typedef struct _bigInt {
    flag_t flag;     //标记位。
    size_t length;   //当前已使用的分段数目。
    size_t capacity; //实际分配的分段数目。
    seg_t *data;     // long long int型指针，指向实际存储数据的数组（最低位元素）
} bigInt;
/*
保守估算目标参数所需要的capacity。
不会对源操作数a，b做合法值检测，务必确保a，b为合法数值。
*/
BIGINT_PUBLIC(size_t) bigInt_Assume_Capacity(const char mode, const bigInt *a, const bigInt *b);
/*
预分配大整数结构内存。
！！！（内部api，危险操作，慎重使用。）
参数target：可用的bigInt结构体地址，或NULL。NULL告知函数自动申请内存空间。
参数capacity：指定数据段段容量/段，> 0。
参数data_ptr：指向连续可用内存的指针，或NULL。NULL告知函数自动申请内存空间。
返回值：成功，返回就绪target。否则，NULL。
*/
BIGINT_PUBLIC(bigInt *) bigInt_PreInit(bigInt *target, const size_t capacity, seg_t *data_ptr);
/*
从给定hex字符串生成大整数。
参数target：可用的bigInt结构体地址，或NULL。NULL告知函数自动申请内存空间。
参数capacity：指定数据段段容量/段（> 0），不保证为实际段数。实际段数同时还受给定字符串长度影响。
参数data_ptr：指向连续可用内存的指针，或NULL。NULL告知函数自动申请内存空间。
参数valueStr：给定hex字符串，不可为NULL或空字符串。
返回值：成功，返回就绪target。否则，NULL。
*/
BIGINT_PUBLIC(bigInt *) bigInt_Init_str(bigInt *target, const size_t capacity, seg_t *data_ptr, const char *valueStr);
/*冻结指定大整数结构体，返回标记位。*/
BIGINT_PUBLIC(flag_t) bigInt_Lock(bigInt *target);
/*解结指定大整数结构体。*/
BIGINT_PUBLIC(void) bigInt_Unlock(bigInt *target);
/*重置指定大数结构体为0。成功，返回capacity。失败返回0。*/
BIGINT_PUBLIC(size_t) bigInt_Clear(bigInt *target);
/*删除指定的大整数。*/
BIGINT_PUBLIC(void) bigInt_DELETE(const bigInt *target);
/*按指定进制(8/16)输出大整数。*/
BIGINT_PUBLIC(void) bigInt_Print(const bigInt *target, int base, FILE *dst);
/*比较绝对值大小。参数非法则返回-2。*/
BIGINT_PUBLIC(int) bigInt_Comp_Abs(const bigInt *a, const bigInt *b);
/*比较大小。参数非法则返回-2。*/
BIGINT_PUBLIC(int) bigInt_Comp(const bigInt *a, const bigInt *b);
BIGINT_PUBLIC(size_t) bigInt_Assume_Capacity(const char mode, const bigInt *a, const bigInt *b);
/*
大整数加法 。
当a或b为NULL时，返回NULL;
否则返回bigInt指针，其指向的大整数结构可能为合法值，也可能为异常值。
关于异常值的检查，请自行核实标记位。
参数target：可用的bigInt结构体地址，或NULL。NULL告知函数自动申请内存空间。
*/
BIGINT_PUBLIC(bigInt *) bigInt_Add(bigInt *sum, const bigInt *a, const bigInt *b);
/*
大整数减法 。
实际调用加法处理。
*/
BIGINT_PUBLIC(bigInt *) bigInt_Sub(bigInt *dif, const bigInt *a, const bigInt *b);
/*
大整数乘法 。
使用细节与加法类似，但对0值做了短路处理。
*/
BIGINT_PUBLIC(bigInt *) bigInt_Mul(bigInt *sum, const bigInt *a, const bigInt *b);
// TODO 待实现：快速除法、模运算、模下幂运算、可接受范围内阶乘。
#ifdef __cplusplus
}
#endif
#endif // bigInt_H
