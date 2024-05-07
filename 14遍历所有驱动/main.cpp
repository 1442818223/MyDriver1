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



VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "ж�سɹ�\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

    pDriver->DriverUnload = UnloadDriver;



    PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)pDriver->DriverSection; // ��ȡ������������ݱ�����

    PLDR_DATA_TABLE_ENTRY pre = (PLDR_DATA_TABLE_ENTRY)ldr->InLoadOrderLinks.Flink; // ��ȡ��ǰģ�����һ��ģ������ݱ�����

    PLDR_DATA_TABLE_ENTRY next = (PLDR_DATA_TABLE_ENTRY)pre->InLoadOrderLinks.Flink; // ��ȡ��һ��ģ������ݱ�����

    int count = 0; // ��������ʼ��

    while (next != pre) { // ѭ������ģ���б�
        DbgPrintEx(77, 0, "[db]:%d driver name =%wZ\n", count++, &next->FullDllName); // �����ǰģ�����������
        next = (PLDR_DATA_TABLE_ENTRY)next->InLoadOrderLinks.Flink; // ��ȡ��һ��ģ������ݱ�����
    }



    return 0;
}