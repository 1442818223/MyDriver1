#include <ntifs.h>
#include <ntstrsafe.h>


VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	DbgPrintEx(77, 0, "ж�سɹ�\n");
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{
	pDriver->DriverUnload = UnloadDriver;

	UNICODE_STRING Uname = { 0 };
	RtlInitUnicodeString(&Uname, L"Demo1");
	STRING Astr = { 0 };
	RtlInitAnsiString(&Astr,"dawdsaddsa");
	NTSTATUS status= RtlAnsiStringToUnicodeString(&Uname, &Astr,true); //�� ANSI �ַ���ת��Ϊ Unicode �ַ���
	                                                            //trueΪ����ռ�
	if (NT_SUCCESS(status)) {
	
		DbgPrintEx(77, 0, "ת�����ַ�����ַ ��: %x,խ:%x\n", Uname.Buffer,Astr.Buffer);
		RtlFreeUnicodeString(&Uname); //�ͷ��ڴ�
	}


	char bb[1000] = { 0 };
	RtlStringCbPrintfA(bb,1000,"-------- %d---- %s", 10, "fsdfhasikdfi"); //��ʽ���ַ��� RtlStringCbPrintfW�ǿ��ַ���
	DbgPrintEx(77, 0, "bb=%s",bb);




	return 0;
}