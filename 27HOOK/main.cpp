#include <ntifs.h>
#include <windef.h>
#include <intrin.h>

bool writeToReadOnlyMemory(void* address, void* buffer, size_t size) {
    // 分配一个MDL
    PMDL Mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);
    if (!Mdl)
        return false;

    // 锁定并且检查页面
    MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);

    // 将页面映射到内存中
    PVOID Mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
    if (!Mapping) {
        MmUnlockPages(Mdl);
        IoFreeMdl(Mdl);
        return false;
    }

    // 修改页面保护属性为可读写
    MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);

    // 写入内存
    RtlCopyMemory(Mapping, buffer, size);

    // 取消映射并解锁页面
    MmUnmapLockedPages(Mapping, Mdl);
    MmUnlockPages(Mdl);

    // 释放MDL
    IoFreeMdl(Mdl);

    return true;
}


PVOID NtOpenProcessPtrAddr = NULL;
PVOID JmpBridgePtr = NULL; ; 
PMDL mdl;
typedef __kernel_entry NTSYSCALLAPI NTSTATUS(*PNTOPENPROCESS)(
    PHANDLE            ProcessHandle,
    ACCESS_MASK        DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PCLIENT_ID         ClientId
    );

/*
 nt!NtOpenProcess:
fffff804`63579c70 4883ec38             sub rsp,38h
fffff804`63579c74 65488b042588010000   mov rax,qword ptr gs:[188h]
fffff804`63579c7d 448a9032020000       mov r10b,byte ptr [rax+232h]
fffff804`63579c84 4488542428           mov byte ptr [rsp+28h],r10b
fffff804`63579c89 4488542420           mov byte ptr [rsp+20h],r10b
fffff804`63579c8e e8bd000000           call nt!PsOpenProcess (fffff804`63579d50)
fffff804`63579c93 4883c438             add rsp,38h
fffff804`63579c97 c3                   ret
*/

NTSTATUS MyNtOpenProcess(
    PHANDLE ProcessHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PCLIENT_ID ClientId
) {
    PNTOPENPROCESS tempCall = (PNTOPENPROCESS)JmpBridgePtr;
    if (ClientId->UniqueProcess == (HANDLE)11304) {
        return STATUS_UNSUCCESSFUL;
    }
    return tempCall(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
}

KIRQL WPOFFx64() {
    KIRQL irql = KeRaiseIrqlToDpcLevel();   //提升中断级到DPC  ,在写入 nt!NtOpenProcess无法打断我们
    UINT64 cr0 = __readcr0();  //拿到cr0
    cr0 &= 0xfffffffffffeffff;   //关闭写保护
    __writecr0(cr0);  //写回
    _disable();   //关闭中断
    return irql;
}

void WPONx64(KIRQL irql) {
    UINT64 cr0 = __readcr0();
    cr0 |= 0x10000;
    __writecr0(cr0);
    _enable();
    KeLowerIrql(irql);
    return;
}

VOID DrvUnload(PDRIVER_OBJECT pDriver) {
   // KIRQL tempIrql = WPOFFx64();
   // RtlCopyMemory(NtOpenProcessPtrAddr, JmpBridgePtr, 13);

    writeToReadOnlyMemory(NtOpenProcessPtrAddr, JmpBridgePtr,13);
   // WPONx64(tempIrql);
    if (JmpBridgePtr) {
        ExFreePool(JmpBridgePtr);
        JmpBridgePtr = NULL;
    }
    DbgPrintEx(77, 0, "驱动卸载!\n");
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING regPath) {
    DbgPrintEx(77, 0, "驱动加载!\n");
    
    UNICODE_STRING apiName;
    RtlInitUnicodeString(&apiName, L"NtOpenProcess");
    NtOpenProcessPtrAddr = MmGetSystemRoutineAddress(&apiName);
    if (!NtOpenProcessPtrAddr) {
        return STATUS_NOT_FOUND;
    }


    /*
    mov rax,0x1122334455667788
    jmp rax 
    */
    BYTE hookCode[] = {  //拿x64dbg 实验出来的跳转指令模版      
        0x48, 0xB8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xFF, 0xE0
    };
    //跳板容器的开辟
    JmpBridgePtr = ExAllocatePool(NonPagedPool, 0x1000);
    
    //保存头两行指令,并构造跳转(让NtOpenProcessPtrAddr开头就执行跳转到我们的MyNtOpenProcess)
    if (!JmpBridgePtr) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(JmpBridgePtr, 0x1000);
    RtlCopyMemory(JmpBridgePtr, NtOpenProcessPtrAddr, 13);   //将破坏的前两句保存下来 虽然是12字节但是指令完整性需要保存13字节
   
    //构造跳转(跳回NtOpenProcessPtrAddr第13字节剩下的没有被修改的指令位置)
    ULONG_PTR tempJmp = ((ULONG_PTR)NtOpenProcessPtrAddr + 13); 
    *((PULONG_PTR)(hookCode + 2)) = tempJmp;
    RtlCopyMemory((PUCHAR)JmpBridgePtr + 13, hookCode, sizeof(hookCode));

  
    //修改NtOpenProcess头部
    *((PULONG_PTR)(hookCode + 2)) = (ULONG_PTR)&MyNtOpenProcess; //替换x64dbg生成的模版地址
   
    //DbgBreakPoint();
  //  KIRQL tempIrql = WPOFFx64();
   
    writeToReadOnlyMemory(NtOpenProcessPtrAddr, hookCode,sizeof(hookCode));

    //WPONx64(tempIrql);

    pDriver->DriverUnload = DrvUnload;
    return STATUS_SUCCESS;
}
