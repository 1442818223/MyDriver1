#include <ntddk.h>
#define FILE_PATH L"\\??\\C:\\1.txt"


HANDLE fileHandle;

NTSTATUS OpenFile() {
    UNICODE_STRING filePath;
    OBJECT_ATTRIBUTES fileAttributes;
    RtlInitUnicodeString(&filePath, FILE_PATH);
    InitializeObjectAttributes(&fileAttributes, &filePath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

    status = ZwCreateFile(&fileHandle,
        GENERIC_WRITE | GENERIC_READ,
        &fileAttributes,
        &ioStatus,//指向 IO_STATUS_BLOCK 结构的指针，用于接收操作的结果信息
        NULL,
        FILE_ATTRIBUTE_NORMAL,//指定文件的属性，如普通文件、目录等。
        0,                    //文件的共享访问权限，例如可以与其他进程共享读取、写入等权限。
        FILE_OPEN_IF,         //文件的创建或打开方式，如创建新文件、打开已存在文件等。
        //如果文件已存在，则打开该文件；如果文件不存在，则创建新文件
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,  //指定文件创建的选项，如同步/异步操作、缓冲/非缓冲等。
        //指定创建的文件对象不是一个目录|指定文件的 I/O 操作是同步且不会引发警报的。
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
        &ioStatus,//指向一个 IO_STATUS_BLOCK 结构的指针，用于接收 I/O 操作的结果信息。
        textToWrite.Buffer, //指向要写入数据的缓冲区的指针。
        textToWrite.Length, //要写入的数据的长度，以字节为单位。
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

    CHAR buffer[1024]; // 用于存储读取的内容

    status = ZwReadFile(fileHandle,
        NULL,
        NULL,
        NULL,
        &ioStatus,
        buffer,
        sizeof(buffer),
        &byteOffset,//可选参数，指定从文件中的哪个位置开始读取数据。
        NULL);

    if (NT_SUCCESS(status)) {
        KdPrint(("File Content: %s\n", buffer));
    }

    return status;
}
VOID CloseFile() {
    ZwClose(fileHandle);
}


VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "卸载成功\n");
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