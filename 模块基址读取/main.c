
#include "Module.h"
VOID DrvUnload(PDRIVER_OBJECT pDriver) {

	DbgPrintEx(77, 0, "Çý¶¯Ð¶ÔØ!\n");
}
NTSTATUS DriverEntry(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath
) {
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	ULONG_PTR moudleBase = GetModuleR3(3048, "kernel32.dll", NULL);

	if (moudleBase) {
		DbgPrintEx(77,0,"Module Base Address: %p\n", (PVOID)moudleBase);
	}
	else {
		DbgPrintEx(77, 0, "Failed to find the module.\n");
	}



	DriverObject->DriverUnload = DrvUnload;
	return STATUS_SUCCESS;
}