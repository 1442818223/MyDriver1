#include <ntifs.h>

typedef struct _AAA {
	int id;
	int y;
	int x;
}AAA, * PAAA;

RTL_GENERIC_TABLE gTABLE;

RTL_GENERIC_COMPARE_RESULTS NTAPI GenericCmp(   //相当于=运算符重载
    struct _RTL_GENERIC_TABLE* Table,
    PVOID FirstStruct,
    PVOID SecondStruct
) {
    PAAA a1 = (PAAA)FirstStruct;
    PAAA a2 = (PAAA)SecondStruct;

    if (a1->id == a2->id)
        return GenericEqual;
    else if (a1->id > a2->id)
        return GenericGreaterThan;
    else
        return GenericLessThan;
}

PVOID NTAPI GenericAllocate(
    struct _RTL_GENERIC_TABLE* Table,
    CLONG ByteSize
)
{
    return ExAllocatePool(NonPagedPool, ByteSize);
}

VOID NTAPI GenericFree(
    struct _RTL_GENERIC_TABLE* Table,
    PVOID Buffer
)
{
    ExFreePool(Buffer);
}



VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
	DbgPrintEx(77, 0, "卸载成功\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;
   

    // 创建一些 AAA 类型的结构体实例
    AAA aaa = { 1, 2, 3 };
    AAA aaal = { 2, 4, 5 };
    AAA aaa2 = { 3, 6, 7 };
    AAA aaa3 = { 4, 8, 9 };


    // 初始化泛型表
    RtlInitializeGenericTable(&gTABLE, GenericCmp, GenericAllocate, GenericFree, NULL);
    BOOLEAN newE = FALSE;  

    // 插入元素到泛型表中
    RtlInsertElementGenericTable(&gTABLE, &aaa, sizeof(AAA), &newE);//参数4为out TRUE，则表示成功插入了新的元素
    RtlInsertElementGenericTable(&gTABLE, &aaal, sizeof(AAA), &newE);
    RtlInsertElementGenericTable(&gTABLE, &aaa2, sizeof(AAA), &newE);
    RtlInsertElementGenericTable(&gTABLE, &aaa3, sizeof(AAA), &newE);

    AAA node = { 3, 0, 0 };
    // 查找指定节点
    //如果找到了匹配的元素，则返回指向该元素的指针；如果找不到匹配的元素，则返回 NULL。
    AAA* xxx = (AAA*)RtlLookupElementGenericTable(&gTABLE, &node);

    // 获取泛型表中元素的数量
    int number = RtlNumberGenericTableElements(&gTABLE);

    // 在使用 RtlEnumerateGenericTableWithoutSplaying 遍历泛型表时，需要使用一个重启键（RestartKey）。
    // 在第一次遍历时，将 RestartKey 设置为 NULL。
    PVOID RestartKey = NULL;
    AAA* xx;
    // 判断树是否为空
    if (!RtlIsGenericTableEmpty(&gTABLE))
    {
        // 遍历树
                       //函数的返回值是一个指向当前遍历的元素的指针。
        for (xx = (AAA*)RtlEnumerateGenericTableWithoutSplaying(&gTABLE, &RestartKey);
            xx != NULL;
            xx = (AAA*)RtlEnumerateGenericTableWithoutSplaying(&gTABLE, &RestartKey))
        {
            DbgPrintEx(77, 0, "%x\r\n", xx->id);
        }
    }

    // 删除指定元素
    RtlDeleteElementGenericTable(&gTABLE, &node);

	return 0;
}