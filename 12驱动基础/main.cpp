#include <ntifs.h>
#include <ntstrsafe.h>


VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	DbgPrintEx(77, 0, "卸载成功\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{
	pDriver->DriverUnload = UnloadDriver;

	UNICODE_STRING Uname = { 0 };
	RtlInitUnicodeString(&Uname, L"Demo1");
	STRING Astr = { 0 };
	RtlInitAnsiString(&Astr,"dawdsaddsa");
	NTSTATUS status= RtlAnsiStringToUnicodeString(&Uname, &Astr,true); //将 ANSI 字符串转换为 Unicode 字符串
	                                                            //true为申请空间
	if (NT_SUCCESS(status)) {
	
		DbgPrintEx(77, 0, "转换后字符串地址 宽: %x,窄:%x\n", Uname.Buffer,Astr.Buffer);
		RtlFreeUnicodeString(&Uname); //释放内存
	}


	char bb[1000] = { 0 };
	RtlStringCbPrintfA(bb,1000,"-------- %d---- %s", 10, "fsdfhasikdfi"); //格式化字符串 RtlStringCbPrintfW是宽字符的
	DbgPrintEx(77, 0, "bb=%s",bb);




	return 0;
}