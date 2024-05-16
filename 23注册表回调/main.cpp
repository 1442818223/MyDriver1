#include <ntifs.h>   
#include<ntddk.h>
#include<wdm.h>

LARGE_INTEGER cookie;

NTSTATUS
RegistryCallback(
	 PVOID CallbackContext,
	 PVOID Argument1,  //���Ծ�����������
	 PVOID Argument2  //���Ծ�����������
) {

	//DbgPrintEx(77, 0, "����һ:%p\n", CallbackContext);



	return STATUS_SUCCESS;
}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	DbgPrintEx(77, 0, "ж�سɹ�\n");


	// ʹ�ñ���� Cookie ע���ص�
	CmUnRegisterCallback(cookie);

	return;
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{

	DbgPrintEx(77, 0, "���سɹ�\n");

	NTSTATUS status;
	
	// ע��ص�����
	status = CmRegisterCallback(RegistryCallback,
		(PVOID)0x123456, //�ص������Ĳ���
		&cookie); //ע����
	if (!NT_SUCCESS(status)) {
		DbgPrint("ע��ص�ʧ��: %08x\n", status);
		return status;
	}

	pDriver->DriverUnload = UnloadDriver;

	return 0;
}