#include <ntifs.h>   
#include<ntddk.h>
#include<wdm.h>

VOID FindProcessNotify() {
    UNICODE_STRING apiname = {0};
    PUCHAR apiaddr = NULL;
    LONG offset = 0;   //Ӳ�������Ե�ַ�����Ǹ���,����Ҫ���з��ŵ�
    PUCHAR PspCreateProcessNotifyRoutine = NULL;
                                    
    RtlInitUnicodeString(&apiname, L"PsSetCreateProcessNotifyRoutine");
    apiaddr =(PUCHAR)MmGetSystemRoutineAddress(&apiname);
    if (!apiaddr) {
        DbgPrintEx(77, 0, "Not found\n");    
        return;
    }

    DbgPrintEx(77, 0, "PsSetCreateThreadNotifyRoutine Addr :0x%llp\n", (PVOID)apiaddr);

    //kd > u PsSetCreateProcessNotifyRoutine
    //    nt!PsSetCreateProcessNotifyRoutine:
    //    fffff801`2ba38d20 4883ec28                sub     rsp, 28h
    //    fffff801`2ba38d24 8ac2                    mov     al, dl
    //    fffff801`2ba38d26 33d2 xor edx, edx
    //    fffff801`2ba38d28 84c0                    test    al, al
    //    fffff801`2ba38d2a 0f95c2                  setne   dl
    //    fffff801`2ba38d2d e8d6010000              call    nt!PspSetCreateProcessNotifyRoutine(fffff801`2ba38f08)
    apiaddr = apiaddr + 13;
    offset = *(PLONG)(apiaddr + 1);   //(PLONG)(apiaddr + 1) ֻ�ǽ� apiaddr + 1 ǿ������ת��Ϊ PLONG ���ͣ�������ָ�룩��
    //��û�л�ȡ�����ַ����ֵ�� (PLONG)(apiaddr + 1) �õ�����һ��ָ���ڴ��ַ apiaddr + 1 ��ָ�롣
   // Ϊ�˻�ȡ�����ַ�ϴ洢����ֵ����Ҫʹ��* �������������ָ��ǰ�棬��* (PLONG)(apiaddr + 1)��
   // �������ܻ�ȡ���õ�ַ�ϵĳ�����ֵ�����ֵ����������Ҫ�����ƫ������
    apiaddr = apiaddr + 5 + offset;

    //DbgBreakPoint();
    //fffff801`2ba38f6a 4c8d2ddf344d00       lea r13,[nt!PspCreateProcessNotifyRoutine (fffff801`2bf0c450)] 
    for (int i = 0; i < 1000; i++) {
    if (MmIsAddressValid(apiaddr + i) && *(apiaddr + i) == 0x4c && (*(apiaddr + i + 1) == 0x8d) && (*(apiaddr + i + 2) == 0x2d)) {
        //DbgBreakPoint();
        apiaddr = apiaddr + i;
        offset = *(PLONG)(apiaddr + 3);
        PspCreateProcessNotifyRoutine = apiaddr + 7 + offset;
        break;   
    }
}
    //DbgBreakPoint();
    DbgPrintEx(77, 0, "Routine Nums Addr ��%p\n", PspCreateProcessNotifyRoutine);

    if (!PspCreateProcessNotifyRoutine)
    {
        return;
    }

    PDWORD64 tempnotifylist = (PDWORD64)PspCreateProcessNotifyRoutine;
    DWORD64 tempnotify = 0;
    DWORD64 reallynotify = 0;
    for (int i = 0; i < 64; i++) {
        tempnotify = *(tempnotifylist + i);
        if (tempnotify == 0)
        {
            break;
        }                        
        tempnotify = tempnotify & 0xfffffffffffffff8;
        reallynotify = *(PDWORD64)tempnotify;
        DbgPrintEx(77, 0, "Create process notify: <%p>\n", reallynotify);
        
        if (MmIsAddressValid((PVOID)reallynotify)) {
            PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)reallynotify, TRUE);
        }
    }
    return;
}  


VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{

	DbgPrintEx(77, 0, "ж�سɹ�\n");

	return;
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{

	DbgPrintEx(77, 0, "���سɹ�\n");

    FindProcessNotify();

	pDriver->DriverUnload = UnloadDriver;

    return 0;
}