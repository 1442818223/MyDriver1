#include <ntifs.h>
#include <windef.h>
#include <intrin.h>

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

ULONG GetPid(CHAR* ProcessName)
{
	ULONG foundPID = 0;  // 初始化为 0，表示未找到
	ULONG i = 0;
	PEPROCESS eproc = NULL;

	// 转换 CHAR* 到 WCHAR*
	WCHAR* wideProcessName = ConvertToWideChar(ProcessName);
	if (wideProcessName == NULL)
	{
		return 0; // 内存分配失败
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
			{
				WCHAR* fileName = wcsrchr(processNameString->Buffer, L'\\');
				if (fileName)
				{
					fileName++; // 跳过 '\' 字符
				}
				else
				{
					fileName = processNameString->Buffer;
				}

				UNICODE_STRING currentProcessName;
				RtlInitUnicodeString(&currentProcessName, fileName);

				if (RtlEqualUnicodeString(&currentProcessName, &targetProcessName, TRUE))
				{
					foundPID = i;  // 找到进程，返回其 PID
					ExFreePoolWithTag(processNameString, 0);
					ObDereferenceObject(eproc);
					break;
				}

				ExFreePoolWithTag(processNameString, 0);
			}

			ObDereferenceObject(eproc);
		}
	}

	ExFreePoolWithTag(wideProcessName, 0);
	return foundPID;
}
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{

}
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	UNICODE_STRING apiName;
	RtlInitUnicodeString(&apiName, L"PsGetProcessSignatureLevel");
	PUCHAR PsGetProcessSignatureLevelPtrAddr = MmGetSystemRoutineAddress(&apiName);

	if (!PsGetProcessSignatureLevelPtrAddr) {
		DbgPrintEx(77, 0, "DriverEntry: PsGetProcessSignatureLevel not found\n");
		return STATUS_NOT_FOUND;
	}

	DbgPrintEx(77, 0, "DriverEntry: PsGetProcessSignatureLevel address found\n");

	for (int i = 0; i < 20; i++) {
		if (*(PsGetProcessSignatureLevelPtrAddr + i) == 0x88 && *(PsGetProcessSignatureLevelPtrAddr + i + 1) == 0x02
			&& *(PsGetProcessSignatureLevelPtrAddr + i + 2) == 0x8a && *(PsGetProcessSignatureLevelPtrAddr + i + 3) == 0x81
			
			) {
			PsGetProcessSignatureLevelPtrAddr = PsGetProcessSignatureLevelPtrAddr + i;
			break;
		}
	}

	// 删除调试断点
	//DbgBreakPoint();

	PsGetProcessSignatureLevelPtrAddr = PsGetProcessSignatureLevelPtrAddr + 4;

	ULONG32 mmm = *(PULONG32)PsGetProcessSignatureLevelPtrAddr;

	DbgPrintEx(77, 0, " mmm value located: 0x%X\n", mmm);

	ULONG xcom = 0;
	xcom = GetPid("xcom.exe");
	//xcom = GetPid("Bandizip.exe");
	PEPROCESS eprocess = NULL;
	NTSTATUS status = PsLookupProcessByProcessId(xcom, &eprocess);


	if (NT_SUCCESS(status) && eprocess != NULL) {
		*(PUCHAR)((PUCHAR)eprocess + mmm ) = 0;
		*(PUCHAR)((PUCHAR)eprocess + mmm+1 ) = 0;

		*(PUCHAR)((PUCHAR)eprocess + mmm + 2) = 0; //0x72是锁住  0是解除
		DbgPrintEx(77, 0, " Process modified successfully\n");
		ObDereferenceObject(eprocess);  // 确保释放对象引用
	}
	else {
		DbgPrintEx(77, 0, " Process not found\n");
	}

	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}