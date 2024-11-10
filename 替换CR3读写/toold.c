#include "tools.h"
#include "strct.h"

//PEPROCESS FindProcessByName(PWCH name)
//{
//
//	PEPROCESS Process = NULL;
//	PEPROCESS findProcess = NULL;
//	for (int i = 8; i < 0x1000000; i += 4)
//	{
//		NTSTATUS status = PsLookupProcessByProcessId((HANDLE)i, &Process);
//		if (!NT_SUCCESS(status))
//		{
//			continue;
//		}
//
//		PUNICODE_STRING ProcessName = NULL;
//		status = SeLocateProcessImageName(Process, &ProcessName);
//
//		if (!NT_SUCCESS(status))
//		{
//			ObDereferenceObject(Process);
//			continue;
//		}
//
//		//DbgPrintEx(77, 0, "Checking process ID: %d\n", i); // 打印当前检查的进程ID
//
//		if (ProcessName->Length) {
//			_wcsupr(ProcessName->Buffer);
//			// 仅比较文件名而不是完整路径
//			PWCH fileName = wcsrchr(ProcessName->Buffer, L'\\');
//			if (fileName) {
//				fileName++;// 跳过'\'字符
//			}
//			else {
//				fileName = ProcessName->Buffer;// 没有'\'则直接使用整个名称
//			}
//
//			//DbgPrintEx(77, 0, "Comparing with: %ws\n", fileName);
//
//			// 使用不区分大小写的比较
//			if (_wcsicmp(fileName, name) == 0) {
//				findProcess = Process;
//				//DbgPrintEx(77, 0, "Matched process: %ws\n", ProcessName->Buffer);
//				ExFreePoolWithTag(ProcessName, 0);
//				break;
//			}
//		}
//
//		ExFreePoolWithTag(ProcessName, 0);
//		ObDereferenceObject(Process);
//	}
//	if (findProcess == NULL) {
//		DbgPrintEx(77, 0, "No process found with name: %ws\n", name); // 未找到目标进程
//	}
//	return findProcess;
//}


WCHAR* ConvertToWideChar(CHAR* str) {
	if (str == NULL) {
		return NULL;
	}

	// 获取字符串的长度
	size_t len = strlen(str);
	// 计算转换后的宽字符所需的缓冲区大小
	ULONG wideLen = (ULONG)((len + 1) * sizeof(WCHAR)); // +1 是为了存放空字符
	WCHAR* wideStr = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, wideLen, 'WChr');

	if (wideStr != NULL) {
		// 进行转换
		NTSTATUS status = RtlMultiByteToUnicodeN(wideStr, wideLen, NULL, str, (ULONG)len);
		if (!NT_SUCCESS(status)) {
			ExFreePoolWithTag(wideStr, 'WChr');
			return NULL; // 转换失败
		}
		wideStr[len] = L'\0'; // 以空字符结束
	}
	return wideStr;
}

PEPROCESS FindProcessByName(CHAR* ProcessName) {
	PEPROCESS foundProcess = NULL;
	ULONG i = 0;
	PEPROCESS eproc = NULL;

	// 转换 CHAR* 到 WCHAR*
	WCHAR* wideProcessName = ConvertToWideChar(ProcessName);
	if (wideProcessName == NULL) {
		return NULL; // 内存分配失败
	}

	UNICODE_STRING targetProcessName;
	RtlInitUnicodeString(&targetProcessName, wideProcessName);

	for (i = 4; i < 100000000; i += 4) {
		NTSTATUS status = PsLookupProcessByProcessId((HANDLE)i, &eproc);
		if (NT_SUCCESS(status) && eproc != NULL) {
			PUNICODE_STRING processNameString = NULL;
			status = SeLocateProcessImageName(eproc, &processNameString);

			if (NT_SUCCESS(status) && processNameString->Length > 0) {
				// 获取进程文件名（去掉路径）
				WCHAR* fileName = wcsrchr(processNameString->Buffer, L'\\');
				if (fileName) {
					fileName++; // 跳过'\'字符
				}
				else {
					fileName = processNameString->Buffer;
				}

				DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 当前进程名: %ws\n", fileName);

				UNICODE_STRING currentProcessName;
				RtlInitUnicodeString(&currentProcessName, fileName);

				// 使用 RtlEqualUnicodeString 进行比较
				if (RtlEqualUnicodeString(&currentProcessName, &targetProcessName, TRUE)) { // TRUE 表示不区分大小写
					foundProcess = eproc; // 找到进程
					ExFreePoolWithTag(processNameString, 0);
					break;
				}

				ExFreePoolWithTag(processNameString, 0);
			}

			if (foundProcess == NULL) {
				ObDereferenceObject(eproc);
			}
		}
	}

	ExFreePoolWithTag(wideProcessName, 0);

	if (foundProcess) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 找到进程: %ws\n", wideProcessName);
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 未找到进程: %s\n", ProcessName);
	}

	return foundProcess;
}

ULONG FatchPid(CHAR* ProcessName)
{
	ULONG foundPid = 0; // 如果没有找到进程，返回 0
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
				// 查找文件名中最后一个 '\\' 字符
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
					foundPid = i; // 返回找到的 PID
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

	if (foundPid != 0)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 找到进程: %ws (PID: %lu)\n", wideProcessName, foundPid);
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 未找到进程: %s\n", ProcessName);
	}

	return foundPid; // 返回进程 PID
}





//Zw的会走ssdt  nt的不会但是需要把MODE切换成Kernel才能绕过三环非法调用地址检测,而Zw会自动切换
MODE SetPreviousMode(PETHREAD Thread, MODE Mode)
{
	static ULONG ModeOffset = 0;

	if (!ModeOffset)
	{
		PUCHAR Temp = (PUCHAR)ExGetPreviousMode;

		for (int i = 0; i < 100; i++)
		{
			if (Temp[i] == 0xc3)// 查找 RET 指令
			{
				ModeOffset = *(PUCHAR)(Temp + i - 4);

				break;
			}
		}

	}
	MODE J_Mode = *((PUCHAR)Thread + ModeOffset);

	*((PUCHAR)Thread + ModeOffset) = Mode;

	return J_Mode;
}


