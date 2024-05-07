#include <ntddk.h>
VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "ж�سɹ�\n");
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    pDriver->DriverUnload = UnloadDriver;

    //����һ�� UNICODE_STRING �ṹ���������ʹ�� RtlInitUnicodeString ������ʼ������ʾ�ļ�·��
    UNICODE_STRING filePath;
    RtlInitUnicodeString(&filePath, L"\\??\\C:\\1.txt"); // �ļ�·��

    //���� OBJECT_ATTRIBUTES �ṹ���������ʹ�� InitializeObjectAttributes ������ʼ���������ļ����������
    OBJECT_ATTRIBUTES fileAttributes;                     //OBJ_CASE_INSENSITIVE����־λ����ʾ·������ĸ�Ĵ�Сд�����С�
    InitializeObjectAttributes(&fileAttributes, &filePath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    HANDLE fileHandle = 0;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;

    // ���Դ��ļ�
    status = ZwCreateFile(&fileHandle,
        GENERIC_READ,
        &fileAttributes,
        &ioStatusBlock,//IO_STATUS_BLOCK�ṹ��ָ�룬���ڽ��ղ����Ľ����Ϣ��
        NULL,  //ָ���ļ��ķ����С
        FILE_ATTRIBUTE_NORMAL,//�ļ�����
        FILE_SHARE_READ,//
        FILE_OPEN,//������򿪷�ʽ�ı�־
        FILE_SYNCHRONOUS_IO_NONALERT, //�ļ�����ѡ��ı�־����ͬ��/�첽����������/�ǻ���ȡ�
        NULL, 0);

    // ������ļ�ʧ��
    if (!NT_SUCCESS(status)) {
        // ���������Ϣ
        DbgPrintEx(77, 0, "[DB]Failed to open file: 0x%X\n", fileHandle);

        // ׼��һ������������ȡ�ļ�����
        CHAR buffer[512];
        ULONG bytesRead;

        // ���Զ�ȡ�ļ�����
        status = ZwReadFile(fileHandle, NULL, NULL, NULL, &ioStatusBlock, buffer, sizeof(buffer), NULL, NULL);
        if (NT_SUCCESS(status)) {
            // ����ļ�����
            DbgPrintEx(77, 0, "File contents:\n%s\n", buffer);
        }
        else {
            // �����ȡ�ļ�ʧ�ܵĴ�����Ϣ
            DbgPrintEx(77, 0, "Failed to read file: 0x%X\n", status);
        }
    }
    else {
        // ������ļ��ɹ�
        // ����ļ����
        DbgPrintEx(77, 0, "[DB]Open successfully: 0x%X\n", fileHandle);

        // ׼��һ������������ȡ�ļ�����
        CHAR buffer[512];
        ULONG bytesRead;

        // ���Զ�ȡ�ļ�����
        status = ZwReadFile(fileHandle, NULL, NULL, NULL, &ioStatusBlock, buffer, sizeof(buffer), NULL, NULL);
        if (NT_SUCCESS(status)) {
            // ����ļ�����
            DbgPrintEx(77, 0, "File contents:\n%s\n", buffer);
        }
        else {
            // �����ȡ�ļ�ʧ�ܵĴ�����Ϣ
            DbgPrintEx(77, 0, "Failed to read file: 0x%X\n", status);
        }

        // �ر��ļ����
        ZwClose(fileHandle);
    }

    return 0;
}