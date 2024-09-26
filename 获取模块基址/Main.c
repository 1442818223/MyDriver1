#include <ntifs.h>
#include "Module.h"
#include "tools.h"


ULONG_PTR GetFunctionBase(char* exe, char* moudlename, char* function)
{

	PEPROCESS Process = FindProcess(exe);

	char* moudleName = moudlename;

	char* FuncName = function;

	ULONG_PTR imageBase = 0;
	ULONG_PTR imageSize = 0;
	KAPC_STATE kApcState = { 0 };
	ULONG_PTR funcAddr = NULL;
	KeStackAttachProcess(Process, &kApcState);

	do
	{

		imageBase = GetModuleR3(PsGetProcessId(Process), moudleName, &imageSize);
		if (!imageBase) break;
		DbgPrintEx(77, 0, "imageBase address: %llx\n", imageBase);

		 funcAddr = GetProcAddressR(imageBase, FuncName, FALSE); //参数3 是否是64位程序 
		if (!funcAddr) break;
		DbgPrintEx(77, 0, "funcAddr address: %llx\n", funcAddr);



	} while (0);



	KeUnstackDetachProcess(&kApcState);

	ObDereferenceObject(Process);

	return funcAddr;
}



VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	//char* exe = "explorer.exe";
	char* exe = "Plants.exe";
	char* moudleName = "win32u.dll";
	char* FuncName = "NtUserGetForegroundWindow";

	ULONG_PTR funcAddr=GetFunctionBase( exe,  moudleName, FuncName);
	if (funcAddr) {
		DbgPrintEx(77,0,"Function address: %llx\n", funcAddr);
	}
	else {
		DbgPrintEx(77, 0, "Failed to get function address.\n");
	}


	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}