#include<ntddk.h>
#include<ntstrsafe.h>

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {

	DbgPrintEx(77, 0, "ж�سɹ�\n");

}


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;

	UNICODE_STRING unDemo1;  //���Ǹ��ṹ���ַ�������
	RtlInitUnicodeString(&unDemo1, L"Demo1");  //��ʼ���ṹ���ַ���
	DbgPrintEx(77, 0, "�ַ���һ:%wZ\n", unDemo1);

	UNICODE_STRING unDemo2;
	WCHAR wchar[20] = { 0 };
	unDemo2.Buffer = wchar; 
	unDemo2.MaximumLength = 20 * sizeof(WCHAR); // ������󳤶�  **��������
	RtlCopyUnicodeString(&unDemo2, &unDemo1); //���ַ���1�������ַ���2
	DbgPrintEx(77, 0, "�ַ�����:%wZ\n", unDemo2);

	UNICODE_STRING unDemo; // Ŀ���ַ���
	WCHAR wchar2[20] = { 0 }; // ���ڴ洢Ŀ���ַ����Ļ�����
	//unDemo.Buffer = wchar2;
    //unDemo.MaximumLength = sizeof(wchar2); // ������󳤶�
	RtlInitEmptyUnicodeString(&unDemo, wchar2, sizeof(wchar2));// ��ʼ��һ���յ� UNICODE_STRING �ṹ��
	PCWSTR sourceString = L"�ҵ��ַ���";// Ҫ���Ƶ�Դ�ַ���
	NTSTATUS status = RtlUnicodeStringCopyString(&unDemo, sourceString);// ��Դ�ַ������Ƶ�Ŀ���ַ�����
	DbgPrintEx(77, 0, "���Ƴɹ���Ŀ���ַ�����%wZ\n", &unDemo);

	return 0;
}