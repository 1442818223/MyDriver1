#include <ntddk.h>
#include <ntifs.h>




VOID nothing(HANDLE ppid, HANDLE mypid, BOOLEAN bereate)
{
	DbgPrint("ProcessNotify\n");
}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	PsSetCreateProcessNotifyRoutine(nothing, TRUE); //TRUE:��֪ͨ�б����Ƴ��ûص����������Ƴ������֪ͨ����������
	// ���ж�سɹ���Ϣ
	DbgPrintEx(77, 0, "ж�سɹ�\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{
	// ������سɹ���Ϣ
	DbgPrintEx(77, 0, "���سɹ�\n");

	// ����ж�غ���
	pDriver->DriverUnload = UnloadDriver;

	//����ע��һ���ص���������ÿ�δ����½���ʱ���ᱻ���á�
	PsSetCreateProcessNotifyRoutine(nothing,FALSE);

	return STATUS_SUCCESS;
}



