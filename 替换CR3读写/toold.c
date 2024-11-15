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
//		//DbgPrintEx(77, 0, "Checking process ID: %d\n", i); // ��ӡ��ǰ���Ľ���ID
//
//		if (ProcessName->Length) {
//			_wcsupr(ProcessName->Buffer);
//			// ���Ƚ��ļ�������������·��
//			PWCH fileName = wcsrchr(ProcessName->Buffer, L'\\');
//			if (fileName) {
//				fileName++;// ����'\'�ַ�
//			}
//			else {
//				fileName = ProcessName->Buffer;// û��'\'��ֱ��ʹ����������
//			}
//
//			//DbgPrintEx(77, 0, "Comparing with: %ws\n", fileName);
//
//			// ʹ�ò����ִ�Сд�ıȽ�
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
//		DbgPrintEx(77, 0, "No process found with name: %ws\n", name); // δ�ҵ�Ŀ�����
//	}
//	return findProcess;
//}


WCHAR* ConvertToWideChar(CHAR* str) {
	if (str == NULL) {
		return NULL;
	}

	// ��ȡ�ַ����ĳ���
	size_t len = strlen(str);
	// ����ת����Ŀ��ַ�����Ļ�������С
	ULONG wideLen = (ULONG)((len + 1) * sizeof(WCHAR)); // +1 ��Ϊ�˴�ſ��ַ�
	WCHAR* wideStr = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, wideLen, 'WChr');

	if (wideStr != NULL) {
		// ����ת��
		NTSTATUS status = RtlMultiByteToUnicodeN(wideStr, wideLen, NULL, str, (ULONG)len);
		if (!NT_SUCCESS(status)) {
			ExFreePoolWithTag(wideStr, 'WChr');
			return NULL; // ת��ʧ��
		}
		wideStr[len] = L'\0'; // �Կ��ַ�����
	}
	return wideStr;
}

PEPROCESS FindProcessByName(CHAR* ProcessName) {
	PEPROCESS foundProcess = NULL;
	ULONG i = 0;
	PEPROCESS eproc = NULL;

	// ת�� CHAR* �� WCHAR*
	WCHAR* wideProcessName = ConvertToWideChar(ProcessName);
	if (wideProcessName == NULL) {
		return NULL; // �ڴ����ʧ��
	}

	UNICODE_STRING targetProcessName;
	RtlInitUnicodeString(&targetProcessName, wideProcessName);

	for (i = 4; i < 100000000; i += 4) {
		NTSTATUS status = PsLookupProcessByProcessId((HANDLE)i, &eproc);
		if (NT_SUCCESS(status) && eproc != NULL) {
			PUNICODE_STRING processNameString = NULL;
			status = SeLocateProcessImageName(eproc, &processNameString);

			if (NT_SUCCESS(status) && processNameString->Length > 0) {
				// ��ȡ�����ļ�����ȥ��·����
				WCHAR* fileName = wcsrchr(processNameString->Buffer, L'\\');
				if (fileName) {
					fileName++; // ����'\'�ַ�
				}
				else {
					fileName = processNameString->Buffer;
				}

				DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] ��ǰ������: %ws\n", fileName);

				UNICODE_STRING currentProcessName;
				RtlInitUnicodeString(&currentProcessName, fileName);

				// ʹ�� RtlEqualUnicodeString ���бȽ�
				if (RtlEqualUnicodeString(&currentProcessName, &targetProcessName, TRUE)) { // TRUE ��ʾ�����ִ�Сд
					foundProcess = eproc; // �ҵ�����
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
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] �ҵ�����: %ws\n", wideProcessName);
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] δ�ҵ�����: %s\n", ProcessName);
	}

	return foundProcess;
}

ULONG FatchPid(CHAR* ProcessName)
{
	ULONG foundPid = 0; // ���û���ҵ����̣����� 0
	ULONG i = 0;
	PEPROCESS eproc = NULL;

	// ת�� CHAR* �� WCHAR*
	WCHAR* wideProcessName = ConvertToWideChar(ProcessName);
	if (wideProcessName == NULL)
	{
		return 0; // �ڴ����ʧ��
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
				// �����ļ��������һ�� '\\' �ַ�
				WCHAR* fileName = wcsrchr(processNameString->Buffer, L'\\');
				if (fileName)
				{
					fileName++; // ����'\'�ַ�
				}
				else
				{
					fileName = processNameString->Buffer;
				}

				DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] ��ǰ������: %ws\n", fileName);

				UNICODE_STRING currentProcessName;
				RtlInitUnicodeString(&currentProcessName, fileName);

				// ʹ�� RtlEqualUnicodeString ���бȽ�
				if (RtlEqualUnicodeString(&currentProcessName, &targetProcessName, TRUE)) // TRUE ��ʾ�����ִ�Сд
				{
					foundPid = i; // �����ҵ��� PID
					ExFreePoolWithTag(processNameString, 0);
					ObDereferenceObject(eproc);
					break;  // �ҵ����̺��˳�ѭ��
				}

				ExFreePoolWithTag(processNameString, 0);
			}

			ObDereferenceObject(eproc);
		}
	}

	ExFreePoolWithTag(wideProcessName, 0);

	if (foundPid != 0)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] �ҵ�����: %ws (PID: %lu)\n", wideProcessName, foundPid);
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] δ�ҵ�����: %s\n", ProcessName);
	}

	return foundPid; // ���ؽ��� PID
}





//Zw�Ļ���ssdt  nt�Ĳ��ᵫ����Ҫ��MODE�л���Kernel�����ƹ������Ƿ����õ�ַ���,��Zw���Զ��л�
MODE SetPreviousMode(PETHREAD Thread, MODE Mode)
{
	static ULONG ModeOffset = 0;

	if (!ModeOffset)
	{
		PUCHAR Temp = (PUCHAR)ExGetPreviousMode;

		for (int i = 0; i < 100; i++)
		{
			if (Temp[i] == 0xc3)// ���� RET ָ��
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


