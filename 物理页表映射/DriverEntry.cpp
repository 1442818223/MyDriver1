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

	// �������ڴ��豸��Unicode�ַ���
	HANDLE hMemory = NULL;
	UNICODE_STRING unName = { 0 };
	RtlInitUnicodeString(&unName, L"\\Device\\PhysicalMemory");
	// ��ʼ����������
	OBJECT_ATTRIBUTES obj;
	InitializeObjectAttributes(&obj, &unName, OBJ_CASE_INSENSITIVE, NULL, NULL);
	NTSTATUS status = ZwOpenSection(&hMemory, SECTION_ALL_ACCESS, &obj);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	PVOID base = NULL;// ���ڱ���ӳ����ͼ�Ļ���ַ
	SIZE_T sizeView = PAGE_SIZE; // Ҫӳ�����ͼ��С��
	LARGE_INTEGER lage = { 0 };
	lage.QuadPart = 0x2c2ca000;// Ҫӳ��������ַ

	PVOID sectionObj = NULL;
	status = ObReferenceObjectByHandle(hMemory, SECTION_ALL_ACCESS, NULL, KernelMode, &sectionObj, NULL);

	if (!NT_SUCCESS(status))
	{
		return status;
	}


	// ӳ����ͼ����ǰ���̵ĵ�ַ�ռ�
	status = ZwMapViewOfSection(hMemory,
		NtCurrentProcess(), &base,
		0, PAGE_SIZE, &lage, &sizeView, ViewUnmap, MEM_TOP_DOWN, PAGE_READWRITE);


	//ZwUnmapViewOfSection(NtCurrentProcess(), base);


	ZwClose(hMemory);

	return STATUS_SUCCESS;
}