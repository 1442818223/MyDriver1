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
	DbgPrintEx(77, 0, "驱动卸载!\r\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	pDriver->DriverUnload = DriverUnload;
	UNREFERENCED_PARAMETER(pReg);
	DbgPrintEx(77, 0, "驱动加载!\r\n");

	// 有IO通讯的不能删文件
	PKLDR_DATA_TABLE_ENTRY ldr = (PKLDR_DATA_TABLE_ENTRY)pDriver->DriverSection;

	if (ldr)
	{
		SelfDeleteFile(ldr->FullDllName.Buffer);
	}

	DeleteRegisterPath(pReg);

	LARGE_INTEGER interval;
	interval.QuadPart = -10000000; // 1秒 (100纳秒单位, 所以 -1,000,000 表示 100毫秒)

	while (1)
	{
		DbgPrintEx(77, 0, "运行中!\r\n");

		// 让线程休息 1 秒
		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	}

	return STATUS_SUCCESS;
}
