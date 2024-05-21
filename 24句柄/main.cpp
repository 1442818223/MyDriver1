#include <ntifs.h>   
#include<ntddk.h>
#include<wdm.h>
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

//PsGetProcessImageFileName函数是 Windows 驱动开发中的一个非公开（或称为不完全文档化）的函数，
// 它可用于获取给定进程的镜像文件名。该函数通常用于驱动程序中，用于在运行时获取进程的镜像文件名。获取进程的主模块名称
extern "C" {  //因为是cpp所以要加extern "C"
    NTKERNELAPI
        UCHAR*
        PsGetProcessImageFileName(
            __in PEPROCESS Process
        );
}

PVOID _HANLDE = NULL;

////对象回调函数
OB_PREOP_CALLBACK_STATUS ProtectProcess(
    PVOID RegistrationContext,
    POB_PRE_OPERATION_INFORMATION OperationInformation  //句柄对象
) {

    // 获取进程的映像文件名
    PUCHAR imageFileName = PsGetProcessImageFileName((PEPROCESS)OperationInformation->Object);
   //HANDLE pid = PsGetProcessId((PEPROCESS)OperationInformation->Object);

    if (strstr((const char*)imageFileName,"PlantsVsZombie")) {
        OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0; //创建句柄信息中的期望访问权限，将其设置为0，也就是无任何访问权限。
        OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = 0;//修改复制句柄信息中的期望访问权限，将其设置为0，即无任何访问权限
    }

    // 打印进程名
    DbgPrintEx(77, 0, "Process name: %s\n", imageFileName);

    // 返回处理成功
    return OB_PREOP_SUCCESS;
}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	DbgPrintEx(77, 0, "卸载成功\n");
   
    if (_HANLDE != NULL) {
        ObUnRegisterCallbacks(_HANLDE);
        _HANLDE = NULL; // 清空句柄变量
    }

	return;
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{
	NTSTATUS status=STATUS_SUCCESS;
	
    //一:
	PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)pDriver->DriverSection;
	ldr->Flags |= 0x20;

    //二:
    OB_OPERATION_REGISTRATION oor = { 0 };
    OB_CALLBACK_REGISTRATION ob = { 0 };
    memset(&oor, 0, sizeof(oor));
    memset(&oor, 0, sizeof(oor)); //初始化结构体变量

    oor.ObjectType = PsProcessType;  /////////注册为进程对象注册回调/////////////////////////////////
    oor.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE; //句柄创建和注册
    oor.PreOperation = ProtectProcess;  //前回调
    oor.PostOperation = NULL;  //后回调

    ob.Version = ObGetFilterVersion();
    ob.OperationRegistrationCount = 1;  //就一个进程操作前回调
    ob.OperationRegistration = &oor;
    ob.RegistrationContext = NULL;  //参数
    RtlInitUnicodeString(&ob.Altitude, L"321000");  //进程回调的高度(排名)

    //三:注册
    //ObRegisterCallbacks用于注册一个或多个用于接收 对象句柄操作通知 的回调函数。
    status = ObRegisterCallbacks(&ob, &_HANLDE);   
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(77, 0, "Failed to register callbacks: %08X\n", status);
        return status; // 注册失败，返回错误状态
    }

	DbgPrintEx(77, 0, "加载成功\n");

    return 0;
}