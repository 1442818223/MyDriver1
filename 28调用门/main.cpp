
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include<iostream>

EXTERN_C VOID callgate(PVOID address);
EXTERN_C VOID callgateRet();


int main() {
	//    函数地址       inter cpu 10字节   ;  AMD cpu 8字节
	char bufcode[10] = { 0,0,0,0,0,0,0,0,0x73,0 }; //门描述符只用到后面两字节

	printf("%p\n", callgateRet);
	system("pause");

	callgate(bufcode);


}