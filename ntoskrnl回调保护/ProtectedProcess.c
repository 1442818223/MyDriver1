#include "ProtectedProcess.h"
#include "ExportFunc.h"
#include "Search.h"

extern POBJECT_TYPE * IoDriverObjectType;

HANDLE regHandle = NULL;
HANDLE gProtectId;

BOOLEAN SetProtectPid(HANDLE pid)
{
	gProtectId = pid;
}

OB_PREOP_CALLBACK_STATUS PreCallback(
	_In_ PVOID RegistrationContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation
)
{
	PEPROCESS Process = OperationInformation->Object;
	if (!Process) return OB_PREOP_SUCCESS;              //查询失败就略过

	HANDLE currentPid = PsGetCurrentProcessId();

	HANDLE targetPid = PsGetProcessId(Process);

	if (currentPid == gProtectId) return OB_PREOP_SUCCESS;   //要是本身就略过

	if (targetPid != gProtectId) return OB_PREOP_SUCCESS; //不是要保护的就略过




	//剩下的就是要保护的
	//DbgPrintEx(77, 0, "[db]:PreCallback\r\n");
	//OperationInformation->Object
	// 检查操作类型是否为句柄创建
	if (OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE)
	{
		OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
		OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess = 0;
	}
	else // 处理句柄复制的情况
	{
		// 将句柄的访问权限设为 0，阻止句柄复制操作
		OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = 0;
		OperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess = 0;
	}

	return OB_PREOP_SUCCESS;
}

//通过驱动程序的设备对象名称获取其 PDRIVER_OBJECT 对象的指针
PDRIVER_OBJECT GetDriverObjectByName(PWCH ObjName)
{
	
	UNICODE_STRING driverName1 = { 0 };
	
	RtlInitUnicodeString(&driverName1, ObjName);
	
	PDRIVER_OBJECT Driver = NULL;
	
	// 使用 ObReferenceObjectByName 函数获取驱动对象
	NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, 0, 0, *IoDriverObjectType, KernelMode, NULL, &Driver);
	
	if (NT_SUCCESS(status))
	{
		ObDereferenceObject(Driver);
	}
	
	return Driver;
}



VOID DestoryObRegister()
{
	if (regHandle) ObUnRegisterCallbacks(regHandle);
}

NTSTATUS InitObRegister()
{
	                                                //设备对象名
	PDRIVER_OBJECT pDirver = GetDriverObjectByName(L"\\Driver\\WMIxWDM");

	if (!pDirver) return STATUS_UNSUCCESSFUL;

	ULONG64 jmpRcx =  searchCode("ntoskrnl.exe", ".text", "FFE1", 0); //FFE1:jmp rax

	if (!jmpRcx) return STATUS_UNSUCCESSFUL;

	OB_OPERATION_REGISTRATION obOp = { 0 };

	obOp.ObjectType = PsProcessType;
	obOp.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	obOp.PreOperation = jmpRcx;  //前回调   [函数藏在参数里面]  这样做的好处是能隐藏函数地址 将参数识别为代码段直接执行
	//obOp.PostOperation = PostCallBack;

	OB_CALLBACK_REGISTRATION obCallRegster = { 0 };
	obCallRegster.Version = ObGetFilterVersion();
	obCallRegster.OperationRegistrationCount = 1;
	obCallRegster.RegistrationContext = PreCallback; //参数
	obCallRegster.OperationRegistration = &obOp;

	RtlInitUnicodeString(&obCallRegster.Altitude, L"456340");

	//PKLDR_DATA_TABLE_ENTRY ldr = (PKLDR_DATA_TABLE_ENTRY)pDirver->DriverSection;  //为空,没在pDirver上下文环境
	//ldr->Flags |= 0x20;

	//开始打补丁(对上面的0x20权限进行补丁)
	RTL_OSVERSIONINFOEXW version = { 0 };
	RtlGetVersion(&version);
	PUCHAR findFunc = NULL;
	//WIN7
	if (version.dwBuildNumber == 7600 || version.dwBuildNumber == 7601)
	{
		PUCHAR func = (PUCHAR)ObRegisterCallbacks;
		for (int i = 0; i < 0x500; i++)
		{
			if (func[i] == 0x74 && func[i + 2] == 0xe8 && func[i + 7] == 0x3b && func[i + 8] == 0xc3)
			{
				LARGE_INTEGER larger;
				larger.QuadPart = func + i + 7;
				larger.LowPart += *(PULONG)(func + i + 3);
				findFunc = larger.QuadPart;
				break;
			}
		}
	}
	else
	{
		PUCHAR func = (PUCHAR)ObRegisterCallbacks;
		for (int i = 0; i < 0x500; i++)
		{
			if (func[i] == 0xBA && func[i + 5] == 0xe8 && func[i + 10] == 0x85 && func[i + 11] == 0xc0)
			{
				LARGE_INTEGER larger;
				larger.QuadPart = func + i + 10;
				larger.LowPart += *(PULONG)(func + i + 6);
				findFunc = larger.QuadPart;
				break;
			}
		}
	}

	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (findFunc)
	{
		PHYSICAL_ADDRESS phy = MmGetPhysicalAddress(findFunc);//获取 指定虚拟地址 对应 的物理地址(里面的值)
		PVOID mem = MmMapIoSpace(phy, 10, MmNonCached);//将物理内存地址映射到虚拟地址空间中
		if (mem)
		{
			UCHAR bufCode[10] = { 0 };
			UCHAR patch[] = { 0xb0,0x1,0xc3 };  //找到权限检测函数头部直接返回1
			memcpy(bufCode, mem, 10);
			memcpy(mem, patch, sizeof(patch));
			status = ObRegisterCallbacks(&obCallRegster, &regHandle);
			memcpy(mem, bufCode, 10); //返回完之后我们就给他修复回去
		}
	}
	


	return status;
}