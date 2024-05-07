#include <ntifs.h>

typedef struct _AAA {
	int id;
	int y;
	int x;
}AAA, * PAAA;

RTL_GENERIC_TABLE gTABLE;

RTL_GENERIC_COMPARE_RESULTS NTAPI GenericCmp(   //�൱��=���������
    struct _RTL_GENERIC_TABLE* Table,
    PVOID FirstStruct,
    PVOID SecondStruct
) {
    PAAA a1 = (PAAA)FirstStruct;
    PAAA a2 = (PAAA)SecondStruct;

    if (a1->id == a2->id)
        return GenericEqual;
    else if (a1->id > a2->id)
        return GenericGreaterThan;
    else
        return GenericLessThan;
}

PVOID NTAPI GenericAllocate(
    struct _RTL_GENERIC_TABLE* Table,
    CLONG ByteSize
)
{
    return ExAllocatePool(NonPagedPool, ByteSize);
}

VOID NTAPI GenericFree(
    struct _RTL_GENERIC_TABLE* Table,
    PVOID Buffer
)
{
    ExFreePool(Buffer);
}



VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
	DbgPrintEx(77, 0, "ж�سɹ�\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;
   

    // ����һЩ AAA ���͵Ľṹ��ʵ��
    AAA aaa = { 1, 2, 3 };
    AAA aaal = { 2, 4, 5 };
    AAA aaa2 = { 3, 6, 7 };
    AAA aaa3 = { 4, 8, 9 };


    // ��ʼ�����ͱ�
    RtlInitializeGenericTable(&gTABLE, GenericCmp, GenericAllocate, GenericFree, NULL);
    BOOLEAN newE = FALSE;  

    // ����Ԫ�ص����ͱ���
    RtlInsertElementGenericTable(&gTABLE, &aaa, sizeof(AAA), &newE);//����4Ϊout TRUE�����ʾ�ɹ��������µ�Ԫ��
    RtlInsertElementGenericTable(&gTABLE, &aaal, sizeof(AAA), &newE);
    RtlInsertElementGenericTable(&gTABLE, &aaa2, sizeof(AAA), &newE);
    RtlInsertElementGenericTable(&gTABLE, &aaa3, sizeof(AAA), &newE);

    AAA node = { 3, 0, 0 };
    // ����ָ���ڵ�
    //����ҵ���ƥ���Ԫ�أ��򷵻�ָ���Ԫ�ص�ָ�룻����Ҳ���ƥ���Ԫ�أ��򷵻� NULL��
    AAA* xxx = (AAA*)RtlLookupElementGenericTable(&gTABLE, &node);

    // ��ȡ���ͱ���Ԫ�ص�����
    int number = RtlNumberGenericTableElements(&gTABLE);

    // ��ʹ�� RtlEnumerateGenericTableWithoutSplaying �������ͱ�ʱ����Ҫʹ��һ����������RestartKey����
    // �ڵ�һ�α���ʱ���� RestartKey ����Ϊ NULL��
    PVOID RestartKey = NULL;
    AAA* xx;
    // �ж����Ƿ�Ϊ��
    if (!RtlIsGenericTableEmpty(&gTABLE))
    {
        // ������
                       //�����ķ���ֵ��һ��ָ��ǰ������Ԫ�ص�ָ�롣
        for (xx = (AAA*)RtlEnumerateGenericTableWithoutSplaying(&gTABLE, &RestartKey);
            xx != NULL;
            xx = (AAA*)RtlEnumerateGenericTableWithoutSplaying(&gTABLE, &RestartKey))
        {
            DbgPrintEx(77, 0, "%x\r\n", xx->id);
        }
    }

    // ɾ��ָ��Ԫ��
    RtlDeleteElementGenericTable(&gTABLE, &node);

	return 0;
}