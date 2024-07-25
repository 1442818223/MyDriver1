#include <ntifs.h>
#include <windef.h>
#include <intrin.h>

bool writeToReadOnlyMemory(void* address, void* buffer, size_t size) {
    // ����һ��MDL
    PMDL Mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);
    if (!Mdl)
        return false;

    // �������Ҽ��ҳ��
    MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);

    // ��ҳ��ӳ�䵽�ڴ���
    PVOID Mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
    if (!Mapping) {
        MmUnlockPages(Mdl);
        IoFreeMdl(Mdl);
        return false;
    }

    // �޸�ҳ�汣������Ϊ�ɶ�д
    MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);

    // д���ڴ�
    RtlCopyMemory(Mapping, buffer, size);

    // ȡ��ӳ�䲢����ҳ��
    MmUnmapLockedPages(Mapping, Mdl);
    MmUnlockPages(Mdl);

    // �ͷ�MDL
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
    KIRQL irql = KeRaiseIrqlToDpcLevel();   //�����жϼ���DPC  ,��д�� nt!NtOpenProcess�޷��������
    UINT64 cr0 = __readcr0();  //�õ�cr0
    cr0 &= 0xfffffffffffeffff;   //�ر�д����
    __writecr0(cr0);  //д��
    _disable();   //�ر��ж�
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
    DbgPrintEx(77, 0, "����ж��!\n");
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING regPath) {
    DbgPrintEx(77, 0, "��������!\n");
    
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
    BYTE hookCode[] = {  //��x64dbg ʵ���������תָ��ģ��      
        0x48, 0xB8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xFF, 0xE0
    };
    //���������Ŀ���
    JmpBridgePtr = ExAllocatePool(NonPagedPool, 0x1000);
    
    //����ͷ����ָ��,��������ת(��NtOpenProcessPtrAddr��ͷ��ִ����ת�����ǵ�MyNtOpenProcess)
    if (!JmpBridgePtr) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(JmpBridgePtr, 0x1000);
    RtlCopyMemory(JmpBridgePtr, NtOpenProcessPtrAddr, 13);   //���ƻ���ǰ���䱣������ ��Ȼ��12�ֽڵ���ָ����������Ҫ����13�ֽ�
   
    //������ת(����NtOpenProcessPtrAddr��13�ֽ�ʣ�µ�û�б��޸ĵ�ָ��λ��)
    ULONG_PTR tempJmp = ((ULONG_PTR)NtOpenProcessPtrAddr + 13); 
    *((PULONG_PTR)(hookCode + 2)) = tempJmp;
    RtlCopyMemory((PUCHAR)JmpBridgePtr + 13, hookCode, sizeof(hookCode));

  
    //�޸�NtOpenProcessͷ��
    *((PULONG_PTR)(hookCode + 2)) = (ULONG_PTR)&MyNtOpenProcess; //�滻x64dbg���ɵ�ģ���ַ
   
    //DbgBreakPoint();
  //  KIRQL tempIrql = WPOFFx64();
   
    writeToReadOnlyMemory(NtOpenProcessPtrAddr, hookCode,sizeof(hookCode));

    //WPONx64(tempIrql);

    pDriver->DriverUnload = DrvUnload;
    return STATUS_SUCCESS;
}
