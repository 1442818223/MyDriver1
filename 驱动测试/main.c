#include <ntddk.h>


VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "66666666666666666666666666666666666666666\n");
}


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DriverObject->DriverUnload = DriverUnload;  //�ڴ����ʱû��Ҫ��������������� ����Ҳ������
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "9999999999999999999999999999999999999\n");


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Custom system call handler registered successfully\n");

	return STATUS_SUCCESS;
}

