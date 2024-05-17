#include <ntifs.h>   
#include<ntddk.h>
#include<wdm.h>

LARGE_INTEGER cookie;

NTSTATUS RegistryCallback(
	 PVOID CallbackContext, //回调函数的参数
	 PVOID Argument1,  //可以决定操作类型
	 PVOID Argument2  //可以决定操作数据
) {
    NTSTATUS status = STATUS_SUCCESS;

	//DbgPrintEx(77, 0, "参数一:%p\n", CallbackContext);    ********************************************************
    REG_NOTIFY_CLASS  tempclass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;  //注册表的类型有很多
                                   //这里千万不要转成*(REG_NOTIFY_CLASS*)  因为REG_NOTIFY_CLASS不是基本数据类型
                                                                          // 不能这样搞******************************

    UNICODE_STRING tempname = { 0 };
    tempname.Length = 0;
    tempname.MaximumLength = 1024 * sizeof(WCHAR);
    tempname.Buffer = (PWCH)ExAllocatePool(NonPagedPool, tempname.MaximumLength);
    RtlZeroMemory(tempname.Buffer, tempname.MaximumLength);
    RtlInitUnicodeString(&tempname, L"*ASDIOPXXAADCGR*");


    switch (tempclass) {
    case RegNtPreOpenKey: //不带Ex是为了兼容老版本
    case RegNtPreOpenKeyEx:
    case RegNtPreCreateKey:
    case RegNtPreCreateKeyEx: //创建函数也可以打开
    {
        
        REG_CREATE_KEY_INFORMATION_V1 pkeinfo = *(REG_CREATE_KEY_INFORMATION_V1*)Argument2;
        __try {
            // 打印键的信息
            DbgPrintEx(77, 0, "key info <%wZ>\n", pkeinfo.CompleteName);

            // 检查键的名称是否匹配
            if (FsRtlIsNameInExpression(&tempname, pkeinfo.CompleteName, TRUE, NULL)) {
                DbgPrintEx(77, 0, "Bad Create\n");
                status = STATUS_UNSUCCESSFUL;   //回调函数返回STATUS_UNSUCCESSFUL就表示捕捉到创建的注册表会拦截返回失败
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            DbgPrintEx(77, 0, "bad memory\n");
        }
        break;
    }
 
    default:
        break;
    }

    return status;
}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	DbgPrintEx(77, 0, "卸载成功\n");


	// 使用保存的 Cookie 注销回调
	CmUnRegisterCallback(cookie);

	return;
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{

	DbgPrintEx(77, 0, "加载成功\n");

	NTSTATUS status;
	
	// 注册回调函数
	status = CmRegisterCallback(RegistryCallback, //回调函数
		(PVOID)0x123456, //回调函数的参数
		&cookie); //注册码
	if (!NT_SUCCESS(status)) {
        DbgPrintEx(77, 0, "注册回调失败: %08x\n", status);
		return status;
	}

	pDriver->DriverUnload = UnloadDriver;

	return 0;
}