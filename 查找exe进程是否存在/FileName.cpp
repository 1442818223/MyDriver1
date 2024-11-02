#include <ntifs.h>
#include <ntstrsafe.h>


WCHAR* ConvertToWideChar(CHAR* str)
{
	if (str == NULL)
	{
		return NULL;
	}

	// 获取字符串的长度
	size_t len = strlen(str);
	// 计算转换后的宽字符所需的缓冲区大小
	ULONG wideLen = (ULONG)((len + 1) * sizeof(WCHAR)); // +1 是为了存放空字符
	WCHAR* wideStr = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, wideLen, 'WChr');

	if (wideStr != NULL)
	{
		// 进行转换
		NTSTATUS status = RtlMultiByteToUnicodeN(wideStr, wideLen, NULL, str, (ULONG)len);
		if (!NT_SUCCESS(status))
		{
			ExFreePoolWithTag(wideStr, 'WChr');
			return NULL; // 转换失败
		}
		wideStr[len] = L'\0'; // 以空字符结束
	}
	return wideStr;
}

BOOLEAN WaitForProcess(CHAR* ProcessName)
{
	BOOLEAN found = FALSE;
	ULONG i = 0;
	PEPROCESS eproc = NULL;

	// 转换 CHAR* 到 WCHAR*
	WCHAR* wideProcessName = ConvertToWideChar(ProcessName);
	if (wideProcessName == NULL)
	{
		return FALSE; // 内存分配失败
	}

	UNICODE_STRING targetProcessName;
	RtlInitUnicodeString(&targetProcessName, wideProcessName);

	for (i = 4; i < 100000000; i += 4)
	{
		NTSTATUS status = PsLookupProcessByProcessId((HANDLE)i, &eproc);
		if (NT_SUCCESS(status) && eproc != NULL)
		{
			PUNICODE_STRING processNameString = NULL;
			status = SeLocateProcessImageName(eproc, &processNameString);

			if (NT_SUCCESS(status) && processNameString->Length > 0)
			{//指向 str 中最后一个匹配字符 \ 的指针。如果未找到该字符，则返回 NULL。
				WCHAR* fileName = wcsrchr(processNameString->Buffer, L'\\');
				if (fileName)
				{
					fileName++; // 跳过'\'字符
				}
				else
				{
					fileName = processNameString->Buffer;
				}

				DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 当前进程名: %ws\n", fileName);

				UNICODE_STRING currentProcessName;
				RtlInitUnicodeString(&currentProcessName, fileName);

				// 使用 RtlEqualUnicodeString 进行比较
				if (RtlEqualUnicodeString(&currentProcessName, &targetProcessName, TRUE)) // TRUE 表示不区分大小写
				{
					found = TRUE;
					ExFreePoolWithTag(processNameString, 0);
					ObDereferenceObject(eproc);
					break;  // 找到进程后退出循环
				}

				ExFreePoolWithTag(processNameString, 0);
			}

			ObDereferenceObject(eproc);
		}
	}

	ExFreePoolWithTag(wideProcessName, 0);

	if (found)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 找到进程: %ws\n", wideProcessName);
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 未找到进程: %s\n", ProcessName);
	}

	return found;
}
VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);  // 标记参数未使用



}
EXTERN_C
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{

	UNREFERENCED_PARAMETER(RegistryPath);  // 标记参数未使用

	DriverObject->DriverUnload = DriverUnload;

	while (1)
	{
		if (WaitForProcess("dwm.exe"))
			//	if (WaitForProcess("Bandizip.exe"))
		{

			break;
		}

		// 如果没有找到进程，可以选择等待一段时间后重试
		LARGE_INTEGER shortInterval;
		shortInterval.QuadPart = -10000000LL;  // 1秒的延迟，单位为100纳秒
		KeDelayExecutionThread(KernelMode, FALSE, &shortInterval);
	}


	return 0;

}