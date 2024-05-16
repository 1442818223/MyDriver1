#include <ntifs.h>    //要将这个底层头文件放在第一位才行,不然会出现重定义现象
#include<ntddk.h>
//#include<ntstrsafe.h>
#include<wdm.h>

//PsGetProcessImageFileName函数是 Windows 驱动开发中的一个非公开（或称为不完全文档化）的函数，
// 它可用于获取给定进程的镜像文件名。该函数通常用于驱动程序中，用于在运行时获取进程的镜像文件名。获取进程的主模块名称
extern "C" {  //因为是cpp所以要加extern "C"
	NTKERNELAPI
		UCHAR*
		PsGetProcessImageFileName(
			__in PEPROCESS Process
		);
}
//进程创建回调
VOID ProcessNotifyFun(HANDLE pid, HANDLE pid2, BOOLEAN	bcareaf) {  //PsSetCreateProcessNotifyRoutine回调格式就是这个
	//参数一 父进程pid  参数二 被创建或删除的pid 参数三 是否创建 true 为创建时回调

	if (bcareaf) {

		//PsGetCurrentProcess(); //获取当前进程的进程结构体
		PEPROCESS tempep = NULL;

		PsLookupProcessByProcessId(pid2, &tempep); //查询特定进程ID的进程结构体
		if (!tempep)
		{
			return;
		}

		PUCHAR processname = PsGetProcessImageFileName(tempep);
		DbgPrintEx(77, 0, "进程名:%s\n", processname);
		// 减小对象的引用计数
		ObDereferenceObject(tempep);
	}
	return;
}

//模块加载回调
VOID MyLoadImageNotify(PUNICODE_STRING FullImageName, //DLL模块名
	HANDLE ProcessId, //加载方的pid
	PIMAGE_INFO ImageInfo) {
	                                        
	PEPROCESS tempep = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	UCHAR* imagename = NULL;

	// 通过进程 ID 获取进程对象
	status = PsLookupProcessByProcessId(ProcessId, &tempep);
	if (NT_SUCCESS(status)) {
		// 获取进程的镜像文件名
		imagename = PsGetProcessImageFileName(tempep);

		// 打印进程相关信息
		DbgPrintEx(77, 0, "加载方的镜像文件名:<%s> DLL模块名: <%wZ> 被加载模块基址: (%p) 大小: <%llx>\n",
			imagename, FullImageName, ImageInfo->ImageBase, ImageInfo->ImageSize);

		// 释放进程对象
		ObDereferenceObject(tempep);
	}
	else {
		// 打印错误信息
		DbgPrintEx(77, 0, "Failed to lookup process by PID: %x\n", ProcessId);
	}

	return;
}

//线程回调
VOID MythreadNotify( HANDLE ProcessId,  //提供触发线程创建或销毁的进程的ID
	 HANDLE ThreadId, //提供触发通知的线程的ID
	 BOOLEAN Create	) //TRUE，则表示线程被创建；如果为 FALSE，则表示线程被销毁。
{
	if (Create) {
		PEPROCESS tempep = NULL;
		NTSTATUS status = STATUS_SUCCESS;
		UCHAR* imagename = NULL;

		// 通过进程 ID 获取进程对象
		status = PsLookupProcessByProcessId(ProcessId, &tempep);
		if (NT_SUCCESS(status)) {
			// 获取进程的镜像文件名
			imagename = PsGetProcessImageFileName(tempep);

			// 打印进程相关信息
			DbgPrint("<%s> create thread <%d>\n", imagename, ThreadId);

			// 释放进程对象
			ObDereferenceObject(tempep);
		}
		else {
			// 打印错误信息
			DbgPrint("Failed to lookup process by PID: %p\n", ProcessId);
		}
	}
	
	return;
	

}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	PsSetCreateProcessNotifyRoutine(ProcessNotifyFun, TRUE); //删除进程回调函数

	PsRemoveLoadImageNotifyRoutine(MyLoadImageNotify);   //卸载模块加载回调

	PsRemoveCreateThreadNotifyRoutine(MythreadNotify);   //卸载线程回调函数

	DbgPrintEx(77, 0, "卸载成功\n");

	return;
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{

	DbgPrintEx(77, 0, "加载成功\n");

	pDriver->DriverUnload = UnloadDriver;
	
	NTSTATUS 	status = STATUS_SUCCESS;
	DbgBreakPoint();
	PsSetCreateProcessNotifyRoutine(ProcessNotifyFun, FALSE);
	DbgBreakPoint();
	
	status = PsSetLoadImageNotifyRoutine(MyLoadImageNotify);  //每当模块有加载的时候触发通知

	status = PsSetCreateThreadNotifyRoutine(MythreadNotify); //线程注册回调
	return 0;

}