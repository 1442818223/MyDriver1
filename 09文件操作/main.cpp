#include <ntddk.h>
#include <Ntifs.h>

#define FILE_PATH L"\\??\\C:\\1.txt"


HANDLE fileHandle;

NTSTATUS OpenFile() {
    UNICODE_STRING filePath;
    OBJECT_ATTRIBUTES fileAttributes;
    RtlInitUnicodeString(&filePath, FILE_PATH);
    InitializeObjectAttributes(&fileAttributes, &filePath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

    status = ZwCreateFile(&fileHandle,         //Ҳ������ZwOpenFile��
        GENERIC_WRITE | GENERIC_READ,
        &fileAttributes,
        &ioStatus,//ָ�� IO_STATUS_BLOCK �ṹ��ָ�룬���ڽ��ղ����Ľ����Ϣ
        NULL,
        FILE_ATTRIBUTE_NORMAL,//ָ���ļ������ԣ�����ͨ�ļ���Ŀ¼�ȡ�
        0,                    //�ļ��Ĺ������Ȩ�ޣ�����������������̹����ȡ��д���Ȩ�ޡ�
        FILE_OPEN_IF,         //�ļ��Ĵ�����򿪷�ʽ���紴�����ļ������Ѵ����ļ��ȡ�
        //����ļ��Ѵ��ڣ���򿪸��ļ�������ļ������ڣ��򴴽����ļ�
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,  //ָ���ļ�������ѡ���ͬ��/�첽����������/�ǻ���ȡ�
        //ָ���������ļ�������һ��Ŀ¼|ָ���ļ��� I/O ������ͬ���Ҳ������������ġ�
        NULL,
        0);

    return status;
}
NTSTATUS WriteToFile() {
    NTSTATUS status;
    UNICODE_STRING textToWrite;
    RtlInitUnicodeString(&textToWrite, L"waaaaaw");
    IO_STATUS_BLOCK ioStatus;

    status = ZwWriteFile(fileHandle,
        NULL,
        NULL,
        NULL,
        &ioStatus,//ָ��һ�� IO_STATUS_BLOCK �ṹ��ָ�룬���ڽ��� I/O �����Ľ����Ϣ��
        textToWrite.Buffer, //ָ��Ҫд�����ݵĻ�������ָ�롣
        textToWrite.Length, //Ҫд������ݵĳ��ȣ����ֽ�Ϊ��λ��
        NULL,
        NULL);

    return status;
}
NTSTATUS ReadFromFile() {
    NTSTATUS status;
    LARGE_INTEGER byteOffset;
    byteOffset.QuadPart = 0;
    ULONG bytesRead;
    IO_STATUS_BLOCK ioStatus;

    CHAR buffer[1024]; // ���ڴ洢��ȡ������

    status = ZwReadFile(fileHandle,
        NULL,
        NULL,
        NULL,
        &ioStatus,
        buffer,
        sizeof(buffer),
        &byteOffset,//��ѡ������ָ�����ļ��е��ĸ�λ�ÿ�ʼ��ȡ���ݡ�
        NULL);

    if (NT_SUCCESS(status)) {
        KdPrint(("File Content: %s\n", buffer));
    }

    return status;
}
VOID CloseFile() {
    ZwClose(fileHandle);  //���ں�̬δ�رվ���ᵼ��3���޷��򿪺�ɾ��       
}
VOID DeleteFile() {
    UNICODE_STRING filePath;
    OBJECT_ATTRIBUTES fileAttributes;
    RtlInitUnicodeString(&filePath, FILE_PATH);
    InitializeObjectAttributes(&fileAttributes, &filePath, OBJ_CASE_INSENSITIVE, NULL, NULL);
    
    ZwDeleteFile(&fileAttributes);

}

VOID Query() { //��ѯ�ļ���Ϣ

    IO_STATUS_BLOCK ioStatus; 
    FILE_STANDARD_INFORMATION fbi = { 0 };//�����ѯ�Ľ��
    ZwQueryInformationFile(fileHandle, &ioStatus, &fbi, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);


    PVOID filebuffer = NULL;
    filebuffer = ExAllocatePool(NonPagedPool, fbi.EndOfFile.QuadPart);

    RtlZeroMemory(filebuffer, fbi.EndOfFile.QuadPart);
    LARGE_INTEGER readoffset = { 0 };//��ѡ������ָ�����ļ��е��ĸ�λ�ÿ�ʼ��ȡ���ݡ�
    readoffset.QuadPart = 0;

    ZwReadFile(fileHandle, NULL, NULL, NULL, &ioStatus, filebuffer, fbi.EndOfFile.QuadPart, &readoffset, NULL);
    DbgPrintEx(77, 0, "IoIfo(ʵ�ʶ������ֽ���):%d\n", ioStatus.Information);

}


VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "ж�سɹ�\n");
}
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    pDriver->DriverUnload = UnloadDriver;
   
    NTSTATUS status;


    status = OpenFile();
    if (!NT_SUCCESS(status)) {
        KdPrint(("Failed to open file: %08X\n", status));
        return status;
    }

    status = WriteToFile();
    if (!NT_SUCCESS(status)) {
        KdPrint(("Failed to write to file: %08X\n", status));
        CloseFile();
        return status;
    }

    status = ReadFromFile();
    if (!NT_SUCCESS(status)) {
        KdPrint(("Failed to read from file: %08X\n", status));
    }

    CloseFile();
   
 
    return 0;

}