#pragma once
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


typedef struct _list_demo {
	INT id;
	LIST_ENTRY list_entry;
}LIST_DEMO,*PLIST_DEMO;


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

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
	DbgPrintEx(77, 0, "卸载成功\n");
	PsSetCreateProcessNotifyRoutine(ProcessNotifyFun, TRUE); //删除进程回调函数
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;
	
	//节点头
	LIST_ENTRY listDemoHead;
	InitializeListHead(&listDemoHead);
	                                     //ExAllocatePoolWithTag 是 Windows 内核编程中用于在内核池中分配内存的函数。
	PLIST_ENTRY listDemo1 = (PLIST_ENTRY)ExAllocatePoolWithTag(NonPagedPool, sizeof(LIST_ENTRY), 'list');
	PLIST_ENTRY listDemo2 = (PLIST_ENTRY)ExAllocatePoolWithTag(NonPagedPool, sizeof(LIST_ENTRY), 'list');
	if (!listDemo1 || !listDemo2) {
		DbgPrintEx(77, 0, "内存分配失败\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	LIST_DEMO listdemoDemol;
	LIST_DEMO listdemoDemo2;
	listdemoDemol.list_entry = *listDemo1;
	listdemoDemo2.list_entry = *listDemo2;
	listdemoDemol.id = 1;
	listdemoDemo2.id = 2;

	//尾插
	InsertTailList(&listDemoHead, &listdemoDemol.list_entry);
	InsertTailList(&listDemoHead, &listdemoDemo2.list_entry);
	
	PLIST_ENTRY currentEntry = listDemoHead.Flink;
	while (currentEntry != &listDemoHead) {
		//处理当前节点的数据 CONTAINING_RECORD 宏的作用就是根据结构体中某个成员的地址来计算整个结构体的地址。
		PLIST_DEMO dataStruct = CONTAINING_RECORD(currentEntry, LIST_DEMO, list_entry);
		DbgPrintEx(77,0,"[db]%d\n",dataStruct->id);
		//移动到下一个节点
		currentEntry = currentEntry->Flink;
	}


	PsSetCreateProcessNotifyRoutine(ProcessNotifyFun,FALSE); //注册一个回调函数来监听进程创建和删除事件
                                                    //FALSE被添加
	 
	return 0;
}

