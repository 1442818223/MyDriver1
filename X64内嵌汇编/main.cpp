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


string GetCpuid()
{
	char SysType[13]; // Array consisting of 13 single bytes/characters
	string CpuID; // The string that will be used to add all the characters toStarting coding in assembly language 

	_asm
	{
		//Execute CPUID with EAX = 0 to get the CPU producer
		xor eax, eax
		cpuid

		//MOV EBX to EAX and get the characters one by one by using shift out right bitwise operation.
		mov eax, ebx
		mov SysType[0], al
		mov SysType[1], ah
		shr eax, 16
		mov SysType[2], al
		mov SysType[3], ah

		//Get the second part the same way but these values are stored in EDX
		mov eax, edx
		mov SysType[4], al
		mov SysType[5], ah
		shr EAX, 16
		mov SysType[6], al
		mov SysType[7], ah

		//Get the third part
		mov eax, ecx
		mov SysType[8], al
		mov SysType[9], ah
		SHR EAX, 16
		mov SysType[10], al
		mov SysType[11], ah
		mov SysType[12], 00
	}

	CpuID.assign(SysType, 12);

	return CpuID;
}


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


