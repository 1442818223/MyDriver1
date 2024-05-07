#include <ntddk.h>
VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "卸载成功\n");
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    pDriver->DriverUnload = UnloadDriver;

    //定义一个 UNICODE_STRING 结构体变量，并使用 RtlInitUnicodeString 函数初始化，表示文件路径
    UNICODE_STRING filePath;
    RtlInitUnicodeString(&filePath, L"\\??\\C:\\1.txt"); // 文件路径

    //定义 OBJECT_ATTRIBUTES 结构体变量，并使用 InitializeObjectAttributes 函数初始化，描述文件对象的属性
    OBJECT_ATTRIBUTES fileAttributes;                     //OBJ_CASE_INSENSITIVE：标志位，表示路径名字母的大小写不敏感。
    InitializeObjectAttributes(&fileAttributes, &filePath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    HANDLE fileHandle = 0;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;

    // 尝试打开文件
    status = ZwCreateFile(&fileHandle,
        GENERIC_READ,
        &fileAttributes,
        &ioStatusBlock,//IO_STATUS_BLOCK结构的指针，用于接收操作的结果信息，
        NULL,  //指定文件的分配大小
        FILE_ATTRIBUTE_NORMAL,//文件属性
        FILE_SHARE_READ,//
        FILE_OPEN,//创建或打开方式的标志
        FILE_SYNCHRONOUS_IO_NONALERT, //文件创建选项的标志，如同步/异步操作、缓冲/非缓冲等。
        NULL, 0);

    // 如果打开文件失败
    if (!NT_SUCCESS(status)) {
        // 输出错误信息
        DbgPrintEx(77, 0, "[DB]Failed to open file: 0x%X\n", fileHandle);

        // 准备一个缓冲区来读取文件内容
        CHAR buffer[512];
        ULONG bytesRead;

        // 尝试读取文件内容
        status = ZwReadFile(fileHandle, NULL, NULL, NULL, &ioStatusBlock, buffer, sizeof(buffer), NULL, NULL);
        if (NT_SUCCESS(status)) {
            // 输出文件内容
            DbgPrintEx(77, 0, "File contents:\n%s\n", buffer);
        }
        else {
            // 输出读取文件失败的错误信息
            DbgPrintEx(77, 0, "Failed to read file: 0x%X\n", status);
        }
    }
    else {
        // 如果打开文件成功
        // 输出文件句柄
        DbgPrintEx(77, 0, "[DB]Open successfully: 0x%X\n", fileHandle);

        // 准备一个缓冲区来读取文件内容
        CHAR buffer[512];
        ULONG bytesRead;

        // 尝试读取文件内容
        status = ZwReadFile(fileHandle, NULL, NULL, NULL, &ioStatusBlock, buffer, sizeof(buffer), NULL, NULL);
        if (NT_SUCCESS(status)) {
            // 输出文件内容
            DbgPrintEx(77, 0, "File contents:\n%s\n", buffer);
        }
        else {
            // 输出读取文件失败的错误信息
            DbgPrintEx(77, 0, "Failed to read file: 0x%X\n", status);
        }

        // 关闭文件句柄
        ZwClose(fileHandle);
    }

    return 0;
}