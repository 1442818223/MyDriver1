#include<ntddk.h>

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {

	DbgPrintEx(77,0,"卸载成功");

}


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;

	DbgPrintEx(77, 0, "RegPath: %wZ\n", *pReg);
	DbgPrintEx(77, 0, "中断级: %d\n", KeGetCurrentIrql()); //PASSIVE_LEVEL(0) 可以访问分页内存和非分页内存


	return 0;
}