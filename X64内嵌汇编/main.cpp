#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include<iostream>

using namespace std;

extern "C"  ULONG64 add1(ULONG64 a, ULONG64 b);  //c���û��call����extern "C"����



EXTERN_C int test1(int a, int b) {   //������c���� c����Ҳ����extern "C"����
    return a - b;
}
extern "C" ULONG64 test2();

int main()
{
    //c�����
    ULONG64 a =add1(1, 2);
    printf("%lld\n",a);


    //=========================================================================================================


    //����c
    ULONG64 x = test2();
    printf("%lld\n", x);


    return 0;
}


