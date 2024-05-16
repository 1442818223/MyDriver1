#include<ntddk.h>

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {

	DbgPrintEx(77,0,"卸载成功");

}


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;

	DbgPrintEx(77, 0, "RegPath: %wZ\n", *pReg);
	DbgPrintEx(77, 0, "中断级: %d\n", KeGetCurrentIrql()); //PASSIVE_LEVEL(0) 可以访问分页内存和非分页内存

	KIRQL oldkirql=0;
	oldkirql = KeRaiseIrqlToDpcLevel(); //用来提升硬件优先级到 DISPATCH_LEVEL
	DbgPrintEx(77, 0, "当前中断级: %d\n", oldkirql);
	KeLowerIrql(oldkirql);  //降低当前中断级
	return 0;
}