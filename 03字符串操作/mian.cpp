#include<ntddk.h>
#include<ntstrsafe.h>

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {

	DbgPrintEx(77, 0, "卸载成功\n");

}


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	pDriver->DriverUnload = UnloadDriver;

	UNICODE_STRING unDemo1;  //这是个结构体字符串类型
	RtlInitUnicodeString(&unDemo1, L"Demo1");  //初始化结构体字符串
	DbgPrintEx(77, 0, "字符串一:%wZ\n", unDemo1);

	UNICODE_STRING unDemo2;
	WCHAR wchar[20] = { 0 };
	unDemo2.Buffer = wchar; 
	unDemo2.MaximumLength = 20 * sizeof(WCHAR); // 设置最大长度  **必须设置
	RtlCopyUnicodeString(&unDemo2, &unDemo1); //将字符串1拷贝到字符串2
	DbgPrintEx(77, 0, "字符串二:%wZ\n", unDemo2);

	UNICODE_STRING unDemo; // 目标字符串
	WCHAR wchar2[20] = { 0 }; // 用于存储目标字符串的缓冲区
	//unDemo.Buffer = wchar2;
    //unDemo.MaximumLength = sizeof(wchar2); // 设置最大长度
	RtlInitEmptyUnicodeString(&unDemo, wchar2, sizeof(wchar2));// 初始化一个空的 UNICODE_STRING 结构体
	PCWSTR sourceString = L"我的字符串";// 要复制的源字符串
	NTSTATUS status = RtlUnicodeStringCopyString(&unDemo, sourceString);// 将源字符串复制到目标字符串中
	DbgPrintEx(77, 0, "复制成功，目标字符串：%wZ\n", &unDemo);

	return 0;
}