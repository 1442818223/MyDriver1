#include <ntifs.h>

typedef struct _CHPCALLBACKLIST {  //别忘记写typedef
    LIST_ENTRY CallbackList;
    ULONG64 Unknown;
    LARGE_INTEGER Cookie;
    ULONG64 FuncContext;
    ULONG64 Func;
} CHPCALLBACKLIST, * PCHPCALLBACKLIST;

ULONG64 tempCookies[50] = { 0 };

VOID EnumCmpCallback() {
    UNICODE_STRING apiName = { 0 };
    PUCHAR apiAddr = NULL;
    BOOLEAN bFound = FALSE;

    RtlInitUnicodeString(&apiName, L"CmUnRegisterCallback");
    apiAddr = (PUCHAR)MmGetSystemRoutineAddress(&apiName);
    if (apiAddr == NULL) {
        DbgPrintEx(77, 0, "API address not found.\n");
        return;
    }

   
    for (int i = 0; i < 1000; i++) {
        if (*(apiAddr + i) == 0x48 && *(apiAddr + i + 5) == 0x48 && *(apiAddr + i + 5 + 1) == 0x8d && *(apiAddr + i + 5 + 1 + 1) == 0x0d) {
            apiAddr = apiAddr + i + 5;
            bFound = TRUE;
            break;
        }
    }

    if (!bFound) {
        DbgPrintEx(77, 0, "Hot patch not found.\n");
        return;
    }

    LONG offset = *(PLONG)(apiAddr + 3);
    PULONG_PTR callbackListHead = (PULONG_PTR)(apiAddr + 7 + offset);
    PLIST_ENTRY pList = NULL;
    PCHPCALLBACKLIST tempNotifyList = NULL;
    PCHPCALLBACKLIST notify = NULL;

    tempNotifyList = (PCHPCALLBACKLIST)(*callbackListHead);
    pList = (PLIST_ENTRY)tempNotifyList;

    LARGE_INTEGER tempRemove = { 0 };

    pList = tempNotifyList->CallbackList.Flink;

    ULONG i = 0;
    do {
        notify = (PCHPCALLBACKLIST)pList;
        if (MmIsAddressValid(notify)) {
            DbgPrintEx(77, 0, "[CmCallback] Function=%p\tCookie=%p\n", (PVOID)(notify->Func), (PVOID)(notify->Cookie.QuadPart));
            if (notify->Cookie.QuadPart != 0) {
                     //不能在此处做CmUnRegisterCallback卸载操作,因为移除了链表会变的
                    tempCookies[i] = notify->Cookie.QuadPart;
                    i++;
                
            }
        }
        pList = pList->Flink;
    } while (pList != (PLIST_ENTRY)callbackListHead);

    for (i = 0; i < 50; i++) {
        if (tempCookies[i] == 0)
            break;
        tempRemove.QuadPart = tempCookies[i];
        CmUnRegisterCallback(tempRemove);//拿到Cookie后一个一个地做卸载操作
        tempRemove.QuadPart = 0;
    }
    return;

}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
    DbgPrintEx(77, 0, "卸载成功\n");



    return;
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{

    DbgPrintEx(77, 0, "加载成功\n");

    NTSTATUS status;

    EnumCmpCallback();


    pDriver->DriverUnload = UnloadDriver;

    return 0;
}