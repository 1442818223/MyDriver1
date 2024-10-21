#include <ntifs.h>
#include "ProtectedProcess.h"


VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	

	DestoryObRegister();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	
	SetProtectPid(10480);

	InitObRegister();


	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}