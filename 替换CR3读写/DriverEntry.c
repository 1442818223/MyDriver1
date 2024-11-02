#include <ntifs.h>
#include "strct.h"
#include "tools.h"
#include "Memory.h"


NTSTATUS FK_ReadMemory(HANDLE pid, ULONG64 baseAddress, ULONG64 buffer, ULONG64 size) {
	
	static PEPROCESS winlogProcess = NULL;
	
	PEPROCESS Process = NULL;
	PVOID BaseAddress = NULL;

	NTSTATUS st = PsLookupProcessByProcessId(pid, &Process);
	if (!NT_SUCCESS(st)) {
		return 0;  
	}

	// 检查进程是否仍然有效
	if (PsGetProcessExitStatus(Process) != STATUS_PENDING) {
		ObDereferenceObject(Process);  // 释放进程对象的引用
		return 0; 
	}


	if (!winlogProcess) {
		winlogProcess = FindProcessByName("winlogon.exe");
	}
	if (!winlogProcess) {
		ObDereferenceObject(Process);
		return 0;
	}


	PVOID Object = ExAllocatePool(NonPagedPool, PAGE_SIZE);
	memset(Object, 0, PAGE_SIZE);

	memcpy(Object, (PUCHAR)winlogProcess - 0x30, 0xef0); //头+eprocesss
	
	PEPROCESS fakeProcess = (PEPROCESS)((PUCHAR)Object + 0x30);


	//ULONG64 gameCr3 = *(PULONG64)((ULONG64)Process + 0x28);//游戏的cr3

	ULONG cr3 = *(PULONG)((PUCHAR)Process + 0x28);


	ULONG msize = PAGE_SIZE;
	HANDLE hMemory = NULL;
	UNICODE_STRING unName = { 0 };
	RtlInitUnicodeString(&unName, L"\\Device\\PhysicalMemory");
	// 初始化对象属性
	OBJECT_ATTRIBUTES obj;
	InitializeObjectAttributes(&obj, &unName, OBJ_CASE_INSENSITIVE, NULL, NULL);
	NTSTATUS status = ZwOpenSection(&hMemory, SECTION_ALL_ACCESS, &obj);
	if (!NT_SUCCESS(status)) {
		DbgPrintEx(77, 0, "Failed to open physical memory section: 0x%lx\n", status);
		return status;
	}
	PVOID mem = NULL;// 用于保存映射视图的基地址
	SIZE_T sizeView = PAGE_SIZE; // 要映射的视图大小。
	LARGE_INTEGER lage = { 0 };
	lage.QuadPart = cr3;// 要映射的物理地址

	PVOID sectionObj = NULL;
	status = ObReferenceObjectByHandle(hMemory, SECTION_ALL_ACCESS, NULL, KernelMode, &sectionObj, NULL);
	if (!NT_SUCCESS(status)) {
		DbgPrintEx(77, 0, "Failed to reference section object: 0x%lx\n", status);
		ZwClose(hMemory);
		return status;
	}
	// 映射视图到当前进程的地址空间
	status = ZwMapViewOfSection(hMemory,
		NtCurrentProcess(), &mem,
		0, msize, &lage, &sizeView, ViewUnmap, MEM_TOP_DOWN, PAGE_READWRITE);
	if (!NT_SUCCESS(status)) {
		DbgPrintEx(77, 0, "Failed to map view of section: 0x%lx\n", status);
		ObDereferenceObject(sectionObj);
		ZwClose(hMemory);
		return status;
	}
	//复制CR3	
	PVOID srcCr3 = (PVOID)ExAllocatePool(NonPagedPool, PAGE_SIZE);
	if (!srcCr3) {
		DbgPrintEx(77, 0, "Failed to allocate memory for srcCr3\n");
		ZwClose(hMemory);
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	memset(srcCr3, 0, PAGE_SIZE);
	memcpy(srcCr3, mem, msize);
	PHYSICAL_ADDRESS srcphyCr3 = MmGetPhysicalAddress(srcCr3);





	//*(PULONG64)((ULONG64)fakeProcess + 0x28) = gameCr3;
	*(PULONG64)((ULONG64)fakeProcess + 0x28) = srcphyCr3.LowPart;

	SIZE_T retSize = 0;
	st = MmCopyVirtualMemory(fakeProcess, baseAddress, IoGetCurrentProcess(), buffer, size, UserMode, &retSize);


	ObDereferenceObject(Process);  

	ExFreePool(Object);


	return st;  // 返回读取内存的状态
}



VOID DriverUnload(PDRIVER_OBJECT pDriver)
{

}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	PEPROCESS Process = FindProcessByName("dwm.exe");
	DbgBreakPoint();
	PUCHAR shellcode = NtAllocateMemory(PsGetCurrentProcessId(), PAGE_SIZE);

	KAPC_STATE kapeState = { 0 };
	KeStackAttachProcess(Process, &kapeState);

	
	NTSTATUS status = FK_ReadMemory(10592, 0x00403DE0, shellcode, 10);
	if (!NT_SUCCESS(status)) {                                        //总是返回失败 不知道为啥
		DbgPrintEx(77, 0, "Memory read failed.\n");///////////////////////////////////////////////////////////////////
	}
	else {
		// 打印 shellcode 作为字节数组
		DbgPrintEx(77, 0, "Shellcode bytes: ");
		for (int i = 0; i < 10; i++) {
			DbgPrintEx(77, 0, "%02X ", shellcode[i]);
		}
		DbgPrintEx(77, 0, "\n");
	}


	// 解除上下文附加
	KeUnstackDetachProcess(&kapeState);
	ObDereferenceObject(Process);
	
	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;

}