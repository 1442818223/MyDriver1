#include <ntddk.h>
#include <ntifs.h>




VOID nothing(HANDLE ppid, HANDLE mypid, BOOLEAN bereate)
{
	DbgPrint("ProcessNotify\n");
}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	PsSetCreateProcessNotifyRoutine(nothing, TRUE); //TRUE:从通知列表中移除该回调函数。不移除会继续通知而导致蓝屏
	// 输出卸载成功信息
	DbgPrintEx(77, 0, "卸载成功\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{
	// 输出加载成功信息
	DbgPrintEx(77, 0, "加载成功\n");

	// 设置卸载函数
	pDriver->DriverUnload = UnloadDriver;

	//用于注册一个回调函数，在每次创建新进程时都会被调用。
	PsSetCreateProcessNotifyRoutine(nothing,FALSE);

	return STATUS_SUCCESS;
}



