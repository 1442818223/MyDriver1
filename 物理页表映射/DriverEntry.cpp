#include <ntifs.h>


VOID DriverUnload(PDRIVER_OBJECT pDriver)
{

}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{

	//DbgBreakPoint();
	//0x490a6000 000000002c2ca002
	//PHYSICAL_ADDRESS phys = { 0 };
	//phys.QuadPart = 0x2c2ca000;
	//PVOID mem =MmMapIoSpace(phys, PAGE_SIZE, MmNonCached);
	pDriver->DriverUnload = DriverUnload;

	// 打开物理内存设备的Unicode字符串
	HANDLE hMemory = NULL;
	UNICODE_STRING unName = { 0 };
	RtlInitUnicodeString(&unName, L"\\Device\\PhysicalMemory");
	// 初始化对象属性
	OBJECT_ATTRIBUTES obj;
	InitializeObjectAttributes(&obj, &unName, OBJ_CASE_INSENSITIVE, NULL, NULL);
	NTSTATUS status = ZwOpenSection(&hMemory, SECTION_ALL_ACCESS, &obj);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	PVOID base = NULL;// 用于保存映射视图的基地址
	SIZE_T sizeView = PAGE_SIZE; // 要映射的视图大小。
	LARGE_INTEGER lage = { 0 };
	lage.QuadPart = 0x2c2ca000;// 要映射的物理地址

	PVOID sectionObj = NULL;
	status = ObReferenceObjectByHandle(hMemory, SECTION_ALL_ACCESS, NULL, KernelMode, &sectionObj, NULL);

	if (!NT_SUCCESS(status))
	{
		return status;
	}


	// 映射视图到当前进程的地址空间
	status = ZwMapViewOfSection(hMemory,
		NtCurrentProcess(), &base,
		0, PAGE_SIZE, &lage, &sizeView, ViewUnmap, MEM_TOP_DOWN, PAGE_READWRITE);


	//ZwUnmapViewOfSection(NtCurrentProcess(), base);


	ZwClose(hMemory);

	return STATUS_SUCCESS;
}