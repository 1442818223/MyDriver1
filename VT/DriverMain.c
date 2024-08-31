#include <ntifs.h>
#include "VMXTools.h"


VOID KeGenericCallDpc(__in PKDEFERRED_ROUTINE Routine,__in_opt PVOID Context);

VOID KeSignalCallDpcDone(__in PVOID SystemArgument1);

LOGICAL KeSignalCallDpcSynchronize(__in PVOID SystemArgument2);

VOID VmxStartVT(_In_ struct _KDPC *Dpc,_In_opt_ PVOID DeferredContext,_In_opt_ PVOID SystemArgument1,_In_opt_ PVOID SystemArgument2)
{

	if (VmxIsCheckSupportVTCPUID())
	{
		DbgPrintEx(77, 0, "[db]:VmxIsCheckSupportVTCPUID  number = %d\r\n", KeGetCurrentProcessorNumber());
	}

	if (VmxIsCheckSupportVTBIOS())
	{
		DbgPrintEx(77, 0, "[db]:VmxIsCheckSupportVTBIOS  number = %d\r\n", KeGetCurrentProcessorNumber());
	}

	if (VmxIsCheckSupportVTCr4())
	{
		DbgPrintEx(77, 0, "[db]:VmxIsCheckSupportVTCr4  number = %d\r\n", KeGetCurrentProcessorNumber());
	}

	KeSignalCallDpcDone(SystemArgument1);
	KeSignalCallDpcSynchronize(SystemArgument2);
}

VOID VmxStopVT(_In_ struct _KDPC *Dpc, _In_opt_ PVOID DeferredContext, _In_opt_ PVOID SystemArgument1, _In_opt_ PVOID SystemArgument2)
{
	KeSignalCallDpcDone(SystemArgument1);
	KeSignalCallDpcSynchronize(SystemArgument2);
}




VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	//DbgPrintEx(77, 0, "[db]:DriverUnLoad!\r\n");
	KdPrintEx((77, 0, "[db]:DriverUnLoad!\r\n"));
	KeGenericCallDpc(VmxStopVT, NULL);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	//DbgPrintEx(77, 0, "[db]:DriverLoad!\r\n");
	
	KdPrintEx((77, 0, "[db]:DriverLoad!\r\n"));

	KeGenericCallDpc(VmxStartVT, NULL);

	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}