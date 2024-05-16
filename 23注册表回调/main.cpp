#include <ntifs.h>   
#include<ntddk.h>
#include<wdm.h>

LARGE_INTEGER cookie;

NTSTATUS
RegistryCallback(
	 PVOID CallbackContext,
	 PVOID Argument1,  //可以决定操作类型
	 PVOID Argument2  //可以决定操作数据
) {

	//DbgPrintEx(77, 0, "参数一:%p\n", CallbackContext);



	return STATUS_SUCCESS;
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
	status = CmRegisterCallback(RegistryCallback,
		(PVOID)0x123456, //回调函数的参数
		&cookie); //注册码
	if (!NT_SUCCESS(status)) {
		DbgPrint("注册回调失败: %08x\n", status);
		return status;
	}

	pDriver->DriverUnload = UnloadDriver;

	return 0;
}