#include <ntddk.h>
VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "ж�سɹ�\n");
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    pDriver->DriverUnload = UnloadDriver;

    //�������б�
    //PVOID pMem = ExAllocatePoolWithTag(NonPagedPool, 1024 * 3, 'd1');
    //if (pMem)
    //{
    //    DbgPrintEx(77, 0, "mem1:%p\n", pMem);
    //    ExFreePoolWithTag(pMem, 'd1');
    //    pMem = ExAllocatePoolWithTag(NonPagedPool, 1024 * 3, 'd1');
    //    DbgPrintEx(77, 0, "mem2:%p\n", pMem);
    //}

    // �����б�
    //�����ڴ��Դ洢�����б����
    PNPAGED_LOOKASIDE_LIST look_List = (PNPAGED_LOOKASIDE_LIST)ExAllocatePool(NonPagedPool, sizeof(NPAGED_LOOKASIDE_LIST));
    DbgBreakPoint();
    PVOID pMem = NULL;
    DbgPrintEx(77, 0, "�������������б����������: �ڴ��ͷź󲻻��������ͷŵ�POOL�У����Ǽ����������б�������\n");
    
    if (look_List) {
        //��ʼ���Ƿ�ҳ���е������б�                            ȷ����������ڴ汻���Ϊ����ִ�� (NX)
        ExInitializeNPagedLookasideList(look_List, NULL, NULL, POOL_NX_ALLOCATION, 1024*3, 'asid', 0);
  
        // �������б��з����ڴ�
        pMem = ExAllocateFromNPagedLookasideList(look_List);
        if (pMem) {
            // ��ӡ������ڴ��ַ
            DbgPrintEx(77, 0, "mem1: %p\n", pMem);
            // ���ڴ淵�ص������б���
            ExFreeToNPagedLookasideList(look_List, pMem);
        }

        // �ٴδ������б��з����ڴ�
        pMem = ExAllocateFromNPagedLookasideList(look_List);
        if (pMem) {
            DbgPrintEx(77, 0, "mem2: %p\n", pMem);
            ExDeleteNPagedLookasideList(look_List);//����ɾ�������б�
        }
    }


    return 0;
}