#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include<iostream>

using namespace std;

extern "C"  ULONG64 add1(ULONG64 a, ULONG64 b);  //c调用汇编call必须extern "C"申明



EXTERN_C int test1(int a, int b) {   //汇编调用c函数 c函数也必须extern "C"声明
    return a - b;
}
extern "C" ULONG64 test2();

int main()
{
    //c调汇编
    ULONG64 a =add1(1, 2);
    printf("%lld\n",a);


    //=========================================================================================================


    //汇编调c
    ULONG64 x = test2();
    printf("%lld\n", x);


    return 0;
}


