
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include<iostream>

EXTERN_C VOID callgate(PVOID address);
EXTERN_C VOID callgateRet();


int main() {
	//    ������ַ       inter cpu 10�ֽ�   ;  AMD cpu 8�ֽ�
	char bufcode[10] = { 0,0,0,0,0,0,0,0,0x73,0 }; //��������ֻ�õ��������ֽ�

	printf("%p\n", callgateRet);
	system("pause");

	callgate(bufcode);


}