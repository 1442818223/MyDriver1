#include<ntddk.h>
#include<ntstrsafe.h>
#include<wdm.h>


typedef struct _list_demo {
	INT id;
	LIST_ENTRY list_entry;
}LIST_DEMO,*PLIST_DEMO;


VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
	DbgPrintEx(77, 0, "卸载成功\n");
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

	return 0;
}