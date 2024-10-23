#include <ntddk.h>
#include "Й§ЛњЦїТы.h"



VOID DriverUnload(PDRIVER_OBJECT DriverObject) {



}

EXTERN_C
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);







	DriverObject->DriverUnload = DriverUnload;

	return STATUS_SUCCESS;
}

