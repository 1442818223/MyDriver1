#include <ntddk.h>
#include "Offset.h"

ULONG CalcPIDOffset()
{
    PEPROCESS peprocess = IoGetCurrentProcess();
    HANDLE pid = PsGetCurrentProcessId();
    PLIST_ENTRY list = NULL;
    int i;

    for (i = 0; i < PAGE_SIZE; i += 4)
    {
        if (*(PHANDLE)((PCHAR)peprocess + i) == pid)
        {
            list = (PLIST_ENTRY)((unsigned char*)peprocess + i + sizeof(HANDLE));
            
            if (MmIsAddressValid(list))
            {
                if (list == list->Flink->Blink)
                {
                    return i;
                }
            }
        }
    }

    return 0;
}

ULONG CalcProcessNameOffset()
{
    PEPROCESS ntosKrnl = PsInitialSystemProcess;
    int i = 0;

    for (i = 0; i < PAGE_SIZE; i++)
    {
        if (RtlCompareMemory((PCHAR)ntosKrnl + i, "System", 6) == 6)
        {
            return i;
        }
    }

    return 0;
}