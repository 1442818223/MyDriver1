#include <ntifs.h>

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;// 按加载顺序链接的链表
    LIST_ENTRY InMemoryOrderLinks;// 按内存顺序链接的链表
    LIST_ENTRY InInitializationOrderLinks;// 按初始化顺序链接的链表
    PVOID DllBase;// DLL 的基地址
    PVOID EntryPoint;// DLL 的入口点地址  
    ULONG SizeOfImage;// DLL 映像的大小
    UNICODE_STRING FullDllName;// 完整的 DLL 名称
    UNICODE_STRING BaseDllName;// DLL 的基本名称
    ULONG Flags;// 标志
    USHORT LoadCount;// 装载计数器
    USHORT TlsIndex;// TLS 索引
    union {
        LIST_ENTRY HashLinks;// 哈希链接
        struct {
            PVOID SectionPointer;// 段指针
            ULONG CheckSum;// 校验和
        };
    };
    union {
        struct {
            ULONG TimeDateStamp;// 时间戳
        };
        struct {
            PVOID LoadedImports;// 已加载的导入表
        };
    };
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;



VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "卸载成功\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

    pDriver->DriverUnload = UnloadDriver;



    PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)pDriver->DriverSection; // 获取驱动程序的数据表格入口

    PLDR_DATA_TABLE_ENTRY pre = (PLDR_DATA_TABLE_ENTRY)ldr->InLoadOrderLinks.Flink; // 获取当前模块的下一个模块的数据表格入口

    PLDR_DATA_TABLE_ENTRY next = (PLDR_DATA_TABLE_ENTRY)pre->InLoadOrderLinks.Flink; // 获取下一个模块的数据表格入口

    int count = 0; // 计数器初始化

    while (next != pre) { // 循环遍历模块列表
        DbgPrintEx(77, 0, "[db]:%d driver name =%wZ\n", count++, &next->FullDllName); // 输出当前模块的驱动名称
        next = (PLDR_DATA_TABLE_ENTRY)next->InLoadOrderLinks.Flink; // 获取下一个模块的数据表格入口
    }



    return 0;
}