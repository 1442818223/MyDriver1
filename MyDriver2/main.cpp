#include<ntddk.h>

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {

	DbgPrintEx(77,0,"ж�سɹ�");

}


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;

	DbgPrintEx(77, 0, "RegPath: %wZ\n", *pReg);
	DbgPrintEx(77, 0, "�жϼ�: %d\n", KeGetCurrentIrql()); //PASSIVE_LEVEL(0) ���Է��ʷ�ҳ�ڴ�ͷǷ�ҳ�ڴ�

	KIRQL oldkirql=0;
	oldkirql = KeRaiseIrqlToDpcLevel(); //��������Ӳ�����ȼ��� DISPATCH_LEVEL
	DbgPrintEx(77, 0, "��ǰ�жϼ�: %d\n", oldkirql);
	KeLowerIrql(oldkirql);  //���͵�ǰ�жϼ�
	return 0;
}