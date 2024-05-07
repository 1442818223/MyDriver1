#include<ntddk.h>
#include<ntstrsafe.h>
#include<wdm.h>


typedef struct _list_demo {
	INT id;
	LIST_ENTRY list_entry;
}LIST_DEMO,*PLIST_DEMO;


VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
	DbgPrintEx(77, 0, "ж�سɹ�\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;
	
	//�ڵ�ͷ
	LIST_ENTRY listDemoHead;
	InitializeListHead(&listDemoHead);
	                                     //ExAllocatePoolWithTag �� Windows �ں˱�����������ں˳��з����ڴ�ĺ�����
	PLIST_ENTRY listDemo1 = (PLIST_ENTRY)ExAllocatePoolWithTag(NonPagedPool, sizeof(LIST_ENTRY), 'list');
	PLIST_ENTRY listDemo2 = (PLIST_ENTRY)ExAllocatePoolWithTag(NonPagedPool, sizeof(LIST_ENTRY), 'list');
	if (!listDemo1 || !listDemo2) {
		DbgPrintEx(77, 0, "�ڴ����ʧ��\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	LIST_DEMO listdemoDemol;
	LIST_DEMO listdemoDemo2;
	listdemoDemol.list_entry = *listDemo1;
	listdemoDemo2.list_entry = *listDemo2;
	listdemoDemol.id = 1;
	listdemoDemo2.id = 2;

	//β��
	InsertTailList(&listDemoHead, &listdemoDemol.list_entry);
	InsertTailList(&listDemoHead, &listdemoDemo2.list_entry);
	
	PLIST_ENTRY currentEntry = listDemoHead.Flink;
	while (currentEntry != &listDemoHead) {
		//����ǰ�ڵ������ CONTAINING_RECORD ������þ��Ǹ��ݽṹ����ĳ����Ա�ĵ�ַ�����������ṹ��ĵ�ַ��
		PLIST_DEMO dataStruct = CONTAINING_RECORD(currentEntry, LIST_DEMO, list_entry);
		DbgPrintEx(77,0,"[db]%d\n",dataStruct->id);
		//�ƶ�����һ���ڵ�
		currentEntry = currentEntry->Flink;
	}

	return 0;
}