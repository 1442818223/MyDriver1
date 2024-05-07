#include <ntifs.h>  
typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;// ������˳�����ӵ�����
    LIST_ENTRY InMemoryOrderLinks;// ���ڴ�˳�����ӵ�����
    LIST_ENTRY InInitializationOrderLinks;// ����ʼ��˳�����ӵ�����
    PVOID DllBase;// DLL �Ļ���ַ
    PVOID EntryPoint;// DLL ����ڵ��ַ  
    ULONG SizeOfImage;// DLL ӳ��Ĵ�С
    UNICODE_STRING FullDllName;// ������ DLL ����
    UNICODE_STRING BaseDllName;// DLL �Ļ�������
    ULONG Flags;// ��־
    USHORT LoadCount;// װ�ؼ�����
    USHORT TlsIndex;// TLS ����
    union {
        LIST_ENTRY HashLinks;// ��ϣ����
        struct {
            PVOID SectionPointer;// ��ָ��
            ULONG CheckSum;// У���
        };
    };
    union {
        struct {
            ULONG TimeDateStamp;// ʱ���
        };
        struct {
            PVOID LoadedImports;// �Ѽ��صĵ����
        };
    };
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h>

    NTKERNELAPI
        NTSTATUS
        ObReferenceObjectByName(
            IN PUNICODE_STRING ObjectName,
            IN ULONG Attributes,
            IN PACCESS_STATE PassedAccessState OPTIONAL,// ����Ȩ�޿���д0 д0��ȫ���ʲ��ܿ���FILE_ALL_ACCESS,
            IN ACCESS_MASK DesiredAccess OPTIONAL,
            IN POBJECT_TYPE ObjectType,//��������
            IN KPROCESSOR_MODE AccessMode,//�ں�ģʽ ������ģʽ enum ����
            IN OUT PVOID ParseContext OPTIONAL,
            OUT PVOID* Object//������� ����Ҫ�õ�����������
        );
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
extern POBJECT_TYPE* IoDriverObjectType;
#ifdef __cplusplus
}
#endif

void DriverHide(PWCH ObjName)
{

    LARGE_INTEGER in = { 0 };
    in.QuadPart = -50000000;  // 5 ���ӳ�(�������ʱ��,��������ʱ��)
    KeDelayExecutionThread(KernelMode, 
        FALSE, //��ʾ�߳��Ƿ�����ڵȴ��ڼ䱻�жϡ�
        &in);


    UNICODE_STRING driverName1;
    RtlInitUnicodeString(&driverName1, ObjName);//��������������

    PDRIVER_OBJECT Driver = (PDRIVER_OBJECT)ExAllocatePoolWithTag(NonPagedPool, sizeof(DRIVER_OBJECT), 'ziji');//��ʼ����������

    NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&Driver);//ͨ�������õ������Ķ���

    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(77, 0, "Failed to obtain driver object, status: 0x%X\n", status);
        return;
    }

    if (Driver == NULL)
    {
        DbgPrintEx(77, 0, "Driver object is NULL\n");
        return;
    }

    PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)Driver->DriverSection;

    if (ldr == NULL)
    {
        DbgPrintEx(77, 0, "Driver section is NULL\n");
        ObDereferenceObject(Driver);
        return;
    }

    DbgPrintEx(77, 0, "Driver name: %wZ\n", &ldr->FullDllName);//�������

    // Remove driver from linked list
    RemoveEntryList(&ldr->InLoadOrderLinks);

    Driver->DriverInit = NULL;
    Driver->DriverSection = NULL;
    Driver->Type = 0;

    ObDereferenceObject(Driver);//���ڼ��ٶ�������ü�����
}

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
	DbgPrintEx(77, 0, "ж�سɹ�\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;
	
    PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)pDriver->DriverSection;
    PLDR_DATA_TABLE_ENTRY pre = (PLDR_DATA_TABLE_ENTRY)ldr->InLoadOrderLinks.Flink;//����Ľڵ�
    PLDR_DATA_TABLE_ENTRY next = (PLDR_DATA_TABLE_ENTRY)pre->InLoadOrderLinks.Flink;//��һ�������Ľڵ�
  
    UNICODE_STRING driverName = { 0 };
    RtlInitUnicodeString(&driverName, L"HTTP.sys");//��ȡ����

    UNICODE_STRING driverName1 = { 0 };
    RtlInitUnicodeString(&driverName1, L"\\driver\\HTTP");
	
    while (next != pre)//����һ��
    {

        if (next->BaseDllName.Length != 0 && RtlCompareUnicodeString(&driverName, &next->BaseDllName, TRUE) == 0)//�ж�����ǲ��ǿյģ�Ȼ�������Ƿ�Ե���
        {

            PDRIVER_OBJECT Driver = (PDRIVER_OBJECT)ExAllocatePoolWithTag(NonPagedPool, sizeof(DRIVER_OBJECT), 'MyTa');
            NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, 0, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)Driver);//ͨ�������õ������Ķ���

            if (NT_SUCCESS(status))
            {
                RemoveEntryList(&next->InLoadOrderLinks);
                Driver->DriverInit = NULL;
                Driver->DriverSection = NULL;//��˫������ȥ����ͬʱ��һЩ�����Ķ���Ҳ����0
                Driver->Type = 0;
            }
            DbgPrintEx(77, 0, "[db]:driver name = %wZ\r\n", &next->FullDllName);

            break;
        }

        next = (PLDR_DATA_TABLE_ENTRY)next->InLoadOrderLinks.Flink;//������һ��(��Զ���һ��pre���Ա�)
    }


    //�����Լ�
    HANDLE hThread = NULL;
    NTSTATUS status = PsCreateSystemThread(&hThread,THREAD_ALL_ACCESS,NULL,NULL,NULL, (PKSTART_ROUTINE)DriverHide,L"\\driver\\15����ժ�������ڵ�");
	return 0;
}