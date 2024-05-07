#include <ntddk.h>

// ����Ҫ������ע����·��
#define REGISTRY_PATH L"\\Registry\\Machine\\Software\\MyApp"  //��ӦHKEY_LOCAL_MACHINE\SOFTWARE\MyApp
// ����Ҫ������ֵ�����ƺ�����
#define VALUE_NAME L"version"
#define VALUE_DATA 12

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "ж�سɹ�\n");
}
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    pDriver->DriverUnload = UnloadDriver;

        NTSTATUS status;
    UNICODE_STRING registryPath;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE registryKey;
    UNICODE_STRING valueName;
    ULONG valueData;
    PKEY_VALUE_PARTIAL_INFORMATION  pKeyinfo = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, 0x10, 'info');
    ULONG queryLen;
    ULONG relqueryLen;

    // ��ʼ��ע���·��
    RtlInitUnicodeString(&registryPath, REGISTRY_PATH);
    // ��ʼ����������                                              ��Сд������
    InitializeObjectAttributes(&objectAttributes, &registryPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
    // �������ע����
    status = ZwCreateKey(&registryKey,//ָ�����ڽ��մ����ļ��ľ����ָ�롣
        KEY_ALL_ACCESS, //ָ�����´����ļ��ķ���Ȩ�ޡ�
        &objectAttributes,//ָ�� OBJECT_ATTRIBUTES �ṹ��ָ�룬���а����йؼ��Ķ������ԣ�����������������Ͱ�ȫ�������ȡ�
        0, NULL,
        REG_OPTION_NON_VOLATILE,//����ʧ��,��ʾ������һֱ��
        NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

/*
дע�����
*/
    // ��ʼ��ֵ����
    RtlInitUnicodeString(&valueName, VALUE_NAME);
    // ��������ֵ����
    valueData = VALUE_DATA;
    // д������ֵ��ע���
    status = ZwSetValueKey(registryKey, &valueName, 0, REG_DWORD, &valueData, sizeof(valueData));
    if (NT_SUCCESS(status))
    {
        DbgPrintEx(77, 0, "set successfully!\n");
    }

/*
��ע�����
*/
// ���β�ѯ��ȡ��������Ļ�������С
    status = ZwQueryValueKey(registryKey,//ע�����ľ��
        &valueName, //ע���ֵ������
        KeyValuePartialInformation, //Ҫ��ѯ����Ϣ���,�˴�Ϊ:��ѯָ��ֵ�Ĳ�����Ϣ
        pKeyinfo,//���ղ�ѯ����Ļ�����ָ��
        sizeof(KEY_VALUE_PARTIAL_INFORMATION),
        &queryLen);                                       //����ʵ�ʲ�ѯ����ĳ���
    DbgPrintEx(77, 0, "[statu1]:%d\n", status);
    DbgPrintEx(77, 0, "[query1]%d\n", *(PUCHAR)(pKeyinfo->Data));
    // �ͷŷ�ֹ�ڴ�й¶
    ExFreePoolWithTag(pKeyinfo, 'info');

    // ������������С���ڴ��
    PKEY_VALUE_PARTIAL_INFORMATION pKeyinfo2 = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, queryLen, 'info');
    if (pKeyinfo)
    {
        if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW)//��������С�����
        {
            relqueryLen = queryLen;
            status = ZwQueryValueKey(registryKey,
                &valueName,
                KeyValuePartialInformation,
                pKeyinfo2,
                relqueryLen,
                &queryLen);
            DbgPrintEx(77, 0, "[statu2]:%d\n", status);
            if (NT_SUCCESS(status))
            {
                DbgPrintEx(77, 0, "[query2]%d\n", *(PUCHAR)(pKeyinfo2->Data));
            }
            else
            {
                DbgPrintEx(77, 0, "Wrong!");
            }

        }
    }

/*
ɾע�����
*/
    status = ZwDeleteKey(registryKey);
    if (NT_SUCCESS(status))
    {
        DbgPrintEx(77, 0, "Handle deleted!\n");
    }



    // �ر�ע����
    status = ZwClose(registryKey);
    if (NT_SUCCESS(status))
    {
        DbgPrintEx(77, 0, "Handle closed!\n");
    }

    return 0;
}