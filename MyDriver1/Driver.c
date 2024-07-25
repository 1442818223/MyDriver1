#include <ntddk.h>

// 定义自定义系统调用处理函数
NTSTATUS MyCustomSystemCallHandler(
    _In_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_ PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG BytesReturned
)
{
    UNREFERENCED_PARAMETER(InputBuffer);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBuffer);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(BytesReturned);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Custom system call executed\n");

    return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    // 注销自定义系统调用处理函数
    PsSetCreateProcessNotifyRoutineEx(MyCustomSystemCallHandler, TRUE);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "66666666666666666666666666666666666666666\n");
}


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    DriverObject->DriverUnload = DriverUnload;
    UNREFERENCED_PARAMETER(RegistryPath);
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "9999999999999999999999999999999999999\n");
    // 注册自定义系统调用处理函数
    NTSTATUS status = PsSetCreateProcessNotifyRoutineEx(MyCustomSystemCallHandler, FALSE);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Failed to register custom system call handler\n");
        return status;
    }

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Custom system call handler registered successfully\n");

    return STATUS_SUCCESS;
}

