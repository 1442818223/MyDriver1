#include <ntifs.h>   
#include<ntddk.h>
#include<wdm.h>
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

//PsGetProcessImageFileName������ Windows ���������е�һ���ǹ��������Ϊ����ȫ�ĵ������ĺ�����
// �������ڻ�ȡ�������̵ľ����ļ������ú���ͨ���������������У�����������ʱ��ȡ���̵ľ����ļ�������ȡ���̵���ģ������
extern "C" {  //��Ϊ��cpp����Ҫ��extern "C"
    NTKERNELAPI
        UCHAR*
        PsGetProcessImageFileName(
            __in PEPROCESS Process
        );
}

PVOID _HANLDE = NULL;

////����ص�����
OB_PREOP_CALLBACK_STATUS ProtectProcess(
    PVOID RegistrationContext,
    POB_PRE_OPERATION_INFORMATION OperationInformation  //�������
) {

    // ��ȡ���̵�ӳ���ļ���
    PUCHAR imageFileName = PsGetProcessImageFileName((PEPROCESS)OperationInformation->Object);
   //HANDLE pid = PsGetProcessId((PEPROCESS)OperationInformation->Object);

    if (strstr((const char*)imageFileName,"PlantsVsZombie")) {
        OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0; //���������Ϣ�е���������Ȩ�ޣ���������Ϊ0��Ҳ�������κη���Ȩ�ޡ�
        OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = 0;//�޸ĸ��ƾ����Ϣ�е���������Ȩ�ޣ���������Ϊ0�������κη���Ȩ��
    }

    // ��ӡ������
    DbgPrintEx(77, 0, "Process name: %s\n", imageFileName);

    // ���ش���ɹ�
    return OB_PREOP_SUCCESS;
}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	DbgPrintEx(77, 0, "ж�سɹ�\n");
   
    if (_HANLDE != NULL) {
        ObUnRegisterCallbacks(_HANLDE);
        _HANLDE = NULL; // ��վ������
    }

	return;
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{
	NTSTATUS status=STATUS_SUCCESS;
	
    //һ:
	PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)pDriver->DriverSection;
	ldr->Flags |= 0x20;

    //��:
    OB_OPERATION_REGISTRATION oor = { 0 };
    OB_CALLBACK_REGISTRATION ob = { 0 };
    memset(&oor, 0, sizeof(oor));
    memset(&oor, 0, sizeof(oor)); //��ʼ���ṹ�����

    oor.ObjectType = PsProcessType;  /////////ע��Ϊ���̶���ע��ص�/////////////////////////////////
    oor.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE; //���������ע��
    oor.PreOperation = ProtectProcess;  //ǰ�ص�
    oor.PostOperation = NULL;  //��ص�

    ob.Version = ObGetFilterVersion();
    ob.OperationRegistrationCount = 1;  //��һ�����̲���ǰ�ص�
    ob.OperationRegistration = &oor;
    ob.RegistrationContext = NULL;  //����
    RtlInitUnicodeString(&ob.Altitude, L"321000");  //���̻ص��ĸ߶�(����)

    //��:ע��
    //ObRegisterCallbacks����ע��һ���������ڽ��� ����������֪ͨ �Ļص�������
    status = ObRegisterCallbacks(&ob, &_HANLDE);   
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(77, 0, "Failed to register callbacks: %08X\n", status);
        return status; // ע��ʧ�ܣ����ش���״̬
    }

	DbgPrintEx(77, 0, "���سɹ�\n");

    return 0;
}