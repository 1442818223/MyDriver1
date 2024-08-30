#include "tools.h"
typedef struct _KLDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	PVOID ExceptionTable;
	ULONG ExceptionTableSize;
	// ULONG padding on IA64
	PVOID GpValue;
	PVOID NonPagedDebugInfo;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT __Unused5;
	PVOID SectionPointer;
	ULONG CheckSum;
	// ULONG padding on IA64
	PVOID LoadedImports;
	PVOID PatchInformation;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	UNREFERENCED_PARAMETER(pDriver);
	DbgPrintEx(77, 0, "����ж��!\r\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	pDriver->DriverUnload = DriverUnload;
	UNREFERENCED_PARAMETER(pReg);
	DbgPrintEx(77, 0, "��������!\r\n");

	// ��IOͨѶ�Ĳ���ɾ�ļ�
	PKLDR_DATA_TABLE_ENTRY ldr = (PKLDR_DATA_TABLE_ENTRY)pDriver->DriverSection;

	if (ldr)
	{
		SelfDeleteFile(ldr->FullDllName.Buffer);
	}

	DeleteRegisterPath(pReg);

	LARGE_INTEGER interval;
	interval.QuadPart = -10000000; // 1�� (100���뵥λ, ���� -1,000,000 ��ʾ 100����)

	while (1)
	{
		DbgPrintEx(77, 0, "������!\r\n");

		// ���߳���Ϣ 1 ��
		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	}

	return STATUS_SUCCESS;
}
