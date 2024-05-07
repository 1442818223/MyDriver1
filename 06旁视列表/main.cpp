#include <ntddk.h>
VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "卸载成功\n");
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    pDriver->DriverUnload = UnloadDriver;

    //非旁视列表
    //PVOID pMem = ExAllocatePoolWithTag(NonPagedPool, 1024 * 3, 'd1');
    //if (pMem)
    //{
    //    DbgPrintEx(77, 0, "mem1:%p\n", pMem);
    //    ExFreePoolWithTag(pMem, 'd1');
    //    pMem = ExAllocatePoolWithTag(NonPagedPool, 1024 * 3, 'd1');
    //    DbgPrintEx(77, 0, "mem2:%p\n", pMem);
    //}

    // 旁视列表
    //分配内存以存储旁视列表对象
    PNPAGED_LOOKASIDE_LIST look_List = (PNPAGED_LOOKASIDE_LIST)ExAllocatePool(NonPagedPool, sizeof(NPAGED_LOOKASIDE_LIST));
    DbgBreakPoint();
    PVOID pMem = NULL;
    DbgPrintEx(77, 0, "接下来是旁视列表的特性试验: 内存释放后不会立即被释放到POOL中，而是继续由旁视列表对象管理\n");
    
    if (look_List) {
        //初始化非分页池中的旁视列表                            确保所分配的内存被标记为不可执行 (NX)
        ExInitializeNPagedLookasideList(look_List, NULL, NULL, POOL_NX_ALLOCATION, 1024*3, 'asid', 0);
  
        // 从旁视列表中分配内存
        pMem = ExAllocateFromNPagedLookasideList(look_List);
        if (pMem) {
            // 打印分配的内存地址
            DbgPrintEx(77, 0, "mem1: %p\n", pMem);
            // 将内存返回到旁视列表中
            ExFreeToNPagedLookasideList(look_List, pMem);
        }

        // 再次从旁视列表中分配内存
        pMem = ExAllocateFromNPagedLookasideList(look_List);
        if (pMem) {
            DbgPrintEx(77, 0, "mem2: %p\n", pMem);
            ExDeleteNPagedLookasideList(look_List);//彻底删除旁视列表
        }
    }


    return 0;
}