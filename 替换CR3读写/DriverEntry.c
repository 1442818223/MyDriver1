#include <ntifs.h>
#include "strct.h"
#include "tools.h"
#include "Memory.h"
NTKERNELAPI PUCHAR NTAPI PsGetProcessImageFileName(PEPROCESS Process);
PEPROCESS FindProcess(char* processName)
{
	PEPROCESS eprocess = NULL;
	KAPC_STATE kapc = { 0 };
	for (int i = 8; i < 0x10000; i += 4)
	{
		PEPROCESS tempProcess = NULL;
		NTSTATUS status = PsLookupProcessByProcessId((HANDLE)i, &tempProcess);
		if (NT_SUCCESS(status))
		{
			char* name = PsGetProcessImageFileName(tempProcess);
			if (name && _stricmp(name, processName) == 0)
			{
				eprocess = tempProcess;
				break;
			}
			ObDereferenceObject(tempProcess);

		}
	}

	return eprocess;
}
NTSTATUS FK_ReadMemory(HANDLE pid, ULONG64 baseAddress, ULONG64 buffer, ULONG64 size) {
    DbgPrintEx(77, 0, "Source address: 0x%llx, Target address: 0x%llx\n", baseAddress, buffer);
    DbgPrintEx(77, 0, "Size to copy: %llu\n", size);

    static PEPROCESS winlogProcess = NULL;
    PEPROCESS Process = NULL;
    PVOID BaseAddress = NULL;

    NTSTATUS st = PsLookupProcessByProcessId(pid, &Process);
    if (!NT_SUCCESS(st)) {
        return st;  // Return the error code from PsLookupProcessByProcessId
    }

    // 检查进程是否仍然有效
    if (PsGetProcessExitStatus(Process) != STATUS_PENDING) {
        ObDereferenceObject(Process);  // 释放进程对象的引用
        DbgPrintEx(77, 0, "要读的进程不存在\n");
        return STATUS_INVALID_PARAMETER;
    }

    // 检查并初始化 winlogProcess
    if (!winlogProcess) {
        winlogProcess = FindProcessByName("winlogon.exe");
    }
    if (!winlogProcess) {
        ObDereferenceObject(Process);
        return STATUS_NOT_FOUND;
    }

    PVOID Object = ExAllocatePool(NonPagedPool, PAGE_SIZE);
    if (!Object) {
        ObDereferenceObject(Process);
        DbgPrintEx(77, 0, "Failed to allocate memory for Object\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(Object, 0, PAGE_SIZE);

    // 获取物理地址并映射内存
    PHYSICAL_ADDRESS phy = MmGetPhysicalAddress((PUCHAR)winlogProcess - 0x30);
    PULONG64 mem1 = (PULONG64)MmMapIoSpace(phy, PAGE_SIZE, MmCached);
    if (!mem1) {
        ExFreePool(Object);
        ObDereferenceObject(Process);
        DbgPrintEx(77, 0, "Failed to map physical memory\n");
        return STATUS_UNSUCCESSFUL;
    }

    memcpy(Object, mem1, PAGE_SIZE); // 复制eprocess头部
    MmUnmapIoSpace(mem1, PAGE_SIZE);


    // 创建假的PEPROCESS
    PEPROCESS fakeProcess = (PEPROCESS)((PUCHAR)Object + 0x30);

    // 打开物理内存
    HANDLE hMemory = NULL;
    UNICODE_STRING unName = { 0 };
    RtlInitUnicodeString(&unName, L"\\Device\\PhysicalMemory");
    OBJECT_ATTRIBUTES obj;
    InitializeObjectAttributes(&obj, &unName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    NTSTATUS status = ZwOpenSection(&hMemory, SECTION_ALL_ACCESS, &obj);
    if (!NT_SUCCESS(status)) {
        ExFreePool(Object);
        ObDereferenceObject(Process);
        DbgPrintEx(77, 0, "ZwOpenSection失败: 0x%lx\n", status);
        return status;
    }

    // 映射物理内存
    PVOID mem = NULL;
    SIZE_T sizeView = PAGE_SIZE; // 要映射的视图大小。
    LARGE_INTEGER lage = { 0 };
    lage.QuadPart = *(PULONG64)((PUCHAR)Process + 0x28);
    status = ZwMapViewOfSection(hMemory, NtCurrentProcess(), &mem,
        0, PAGE_SIZE, &lage, &sizeView, ViewUnmap, MEM_TOP_DOWN, PAGE_READWRITE);
    if (!NT_SUCCESS(status)) {
        ZwClose(hMemory);
        ExFreePool(Object);
        ObDereferenceObject(Process);
        DbgPrintEx(77, 0, "ZwMapViewOfSection失败: 0x%lx\n", status);
        return status;
    }

    // 复制 CR3
    PVOID srcCr3 = ExAllocatePool(NonPagedPool, PAGE_SIZE);
    if (!srcCr3) {
        ZwClose(hMemory);
        ExFreePool(Object);
        ObDereferenceObject(Process);
        DbgPrintEx(77, 0, "ExAllocatePool失败\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(srcCr3, 0, PAGE_SIZE);
    memcpy(srcCr3, mem, PAGE_SIZE);
    PHYSICAL_ADDRESS srcphyCr3 = MmGetPhysicalAddress(srcCr3);
    *(PULONG64)((ULONG64)fakeProcess + 0x28) = srcphyCr3.QuadPart;
    
    DbgBreakPoint();
    
    // 使用 MmCopyVirtualMemory 读取内存
    SIZE_T retSize = 0;
    st = MmCopyVirtualMemory(fakeProcess, baseAddress, IoGetCurrentProcess(), buffer, size, UserMode, &retSize);
    if (!NT_SUCCESS(st)) {
        DbgPrintEx(77, 0, "MmCopyVirtualMemory 失败: 0x%lx\n", st);
    }
    else {
        DbgPrintEx(77, 0, "Memory read successful, bytes read: %zu\n", retSize);
    }

    // 释放资源
    ZwClose(hMemory);
    ExFreePool(Object);
    ExFreePool(srcCr3);
    ObDereferenceObject(Process);

    return st;
}




VOID DriverUnload(PDRIVER_OBJECT pDriver)
{

}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	ULONG aidPid= FatchPid("KmdManager.exe");


	PUCHAR shellcode = NtAllocateMemory(PsGetCurrentProcessId(), PAGE_SIZE);


	PEPROCESS Process = FindProcessByName("dwm.exe");
	KAPC_STATE kapeState = { 0 };
	KeStackAttachProcess(Process, &kapeState);

    DbgBreakPoint();
	NTSTATUS status = FK_ReadMemory(aidPid, 0x00402317, shellcode, 10);
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