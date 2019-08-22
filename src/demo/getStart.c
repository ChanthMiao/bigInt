#include "bigInt.h"
#include <stdio.h>
int main(int argc, char const *argv[])
{
    bigInt *sample1;
    bigInt *sample2;
    bigInt *sample3;
    bigInt *sample4;
    bigInt *sample5;
    bigInt *sample6;
    bigInt *sample7;
    sample1 = bigInt_Init_str(NULL, 0, NULL, "0x001e23f45678b9253d676cb5da84246c8420");
    printf("sample1 = ");
    bigInt_Print(sample1, 16, stdout);
    putc('\n', stdout);
    sample2 = bigInt_Init_str(NULL, 0, NULL, "0x001e23f456723dea197b9253d676cda2fd78cb5da84246c842");
    printf("sample2 = ");
    bigInt_Print(sample2, 16, stdout);
    putc('\n', stdout);
    sample3 = bigInt_Init_str(NULL, 0, NULL, "+0x001e23f45678b9253d676cb5da84246c8420");
    printf("sample3 = ");
    bigInt_Print(sample3, 16, stdout);
    putc('\n', stdout);
    sample4 = bigInt_Init_str(NULL, 0, NULL, "-0x001e23f45678b9253d676cb5da84246c8420");
    printf("sample4 = ");
    bigInt_Print(sample4, 16, stdout);
    putc('\n', stdout);
    sample5 = bigInt_Sub(NULL, sample1, sample2);
    printf("sample5 = sample1 - sample2 = ");
    bigInt_Print(sample5, 16, stdout);
    putc('\n', stdout);
    sample6 = bigInt_Add(NULL, sample1, sample4);
    printf("sample6 = sample1 + sample4 = ");
    bigInt_Print(sample6, 16, stdout);
    putc('\n', stdout);
    sample7 = bigInt_Mul(NULL, sample1, sample4);
    printf("sample6 = sample1 * sample4 = ");
    bigInt_Print(sample7, 16, stdout);
    putc('\n', stdout);
    int ucp = bigInt_Comp(sample1, sample2);
    if (ucp == 1) {
        printf("sample1 > sample2\n");
    } else if (ucp == 0) {
        printf("sample1 == sample2\n");
    } else if (ucp == -1) {
        printf("sample1 < sample2\n");
    } else {
        printf("error\n");
    }
    ucp = bigInt_Comp(sample1, sample3);
    if (ucp == 1) {
        printf("sample1 > sample3\n");
    } else if (ucp == 0) {
        printf("sample1 == sample3\n");
    } else if (ucp == -1) {
        printf("sample1 < sample3\n");
    } else {
        printf("error\n");
    }
    ucp = bigInt_Comp(sample1, sample4);
    if (ucp == 1) {
        printf("sample1 > sample4\n");
    } else if (ucp == 0) {
        printf("sample1 == sample4\n");
    } else if (ucp == -1) {
        printf("sample1 < sample4\n");
    } else {
        printf("error\n");
    }
    bigInt_DELETE(sample1);
    bigInt_DELETE(sample2);
    bigInt_DELETE(sample3);
    bigInt_DELETE(sample4);
    bigInt_DELETE(sample5);
    bigInt_DELETE(sample6);
    bigInt_DELETE(sample7);
    return 0;
}
