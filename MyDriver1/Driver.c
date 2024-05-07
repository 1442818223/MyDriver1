#include<ntddk.h>


NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver,PUNICODE_STRING pReg) {


	DbgPrintEx(77,0,"hello world\n");
	DbgPrintEx(77,0,"RegPath : % wZ\n",*pReg);
	 

}