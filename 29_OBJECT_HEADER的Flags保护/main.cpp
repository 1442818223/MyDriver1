#include <ntifs.h>

// 声明一个外部函数来获取进程映像文件名
EXTERN_C NTKERNELAPI UCHAR* PsGetProcessImageFileName(PEPROCESS Process);

// 设置进程标志
VOID SetProcessFlags(PEPROCESS Process) {
    PUCHAR ObjectHeader = NULL;
#ifdef _WIN64
    ObjectHeader = ((PUCHAR)Process - 0x30);
    DbgPrintEx(77, 0, "Setting flag at address: %p\n", ObjectHeader + 0x1b);
    *(ObjectHeader + 0x1b) |= 4;
#else
    ObjectHeader = ((PUCHAR)Process - 0x18);
    DbgPrintEx(77, 0, "Setting flag at address: %p\n", ObjectHeader + 0xf);
    *(ObjectHeader + 0xf) |= 4;
#endif
}

// 清除进程标志
VOID CloseProcessFlags(PEPROCESS Process) {
    PUCHAR ObjectHeader = NULL;
#ifdef _WIN64
    ObjectHeader = ((PUCHAR)Process - 0x30);
    DbgPrintEx(77, 0, "Clearing flag at address: %p\n", ObjectHeader + 0x1b);
    *(ObjectHeader + 0x1b) &= ~4;
#else
    ObjectHeader = ((PUCHAR)Process - 0x18);
    DbgPrintEx(77, 0, "Clearing flag at address: %p\n", ObjectHeader + 0xf);
    *(ObjectHeader + 0xf) &= ~4;
#endif
}

// 根据名称查找进程
PEPROCESS FindProcessByName(char* name) {
    PEPROCESS findProcess = NULL;
    DbgPrintEx(77, 0, "Looking for process: %s\n", name);

    for (int i = 4; i < 0x1000000; i += 4) {
        PEPROCESS Process = NULL;
        NTSTATUS status = PsLookupProcessByProcessId((HANDLE)i, &Process);
        if (!NT_SUCCESS(status)) {
            continue;
        }

        PUCHAR processName = PsGetProcessImageFileName(Process);
        if (processName) {
            DbgPrintEx(77, 0, "Found process: %s\n", processName);
            if (_stricmp((const char*)processName, name) == 0) {
                findProcess = Process;
                break;
            }
        }
        ObDereferenceObject(Process);
    }

    return findProcess;
}


VOID DriverUnload(PDRIVER_OBJECT pDriver) {
    DbgPrintEx(77, 0, "Driver Unloaded!\n");
    PEPROCESS Process = FindProcessByName("PlantsVsZombie");
    if (!Process) {
        DbgPrintEx(77, 0, "Process not found during unload.\n");
        return;
    }
    CloseProcessFlags(Process);
    ObDereferenceObject(Process);
}


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    DbgPrintEx(77, 0, "Driver Loaded!\n");
    PEPROCESS Process = FindProcessByName("PlantsVsZombie");
    if (!Process) {
        DbgPrintEx(77, 0, "Process not found during load.\n");
        return STATUS_UNSUCCESSFUL;
    }

    SetProcessFlags(Process);
    ObDereferenceObject(Process);

    pDriver->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}
