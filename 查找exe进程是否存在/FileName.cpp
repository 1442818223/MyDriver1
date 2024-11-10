#include <ntifs.h>
#include <ntstrsafe.h>


WCHAR* ConvertToWideChar(CHAR* str)
{
	if (str == NULL)
	{
		return NULL;
	}

	// ��ȡ�ַ����ĳ���
	size_t len = strlen(str);
	// ����ת����Ŀ��ַ�����Ļ�������С
	ULONG wideLen = (ULONG)((len + 1) * sizeof(WCHAR)); // +1 ��Ϊ�˴�ſ��ַ�
	WCHAR* wideStr = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, wideLen, 'WChr');

	if (wideStr != NULL)
	{
		// ����ת��
		NTSTATUS status = RtlMultiByteToUnicodeN(wideStr, wideLen, NULL, str, (ULONG)len);
		if (!NT_SUCCESS(status))
		{
			ExFreePoolWithTag(wideStr, 'WChr');
			return NULL; // ת��ʧ��
		}
		wideStr[len] = L'\0'; // �Կ��ַ�����
	}
	return wideStr;
}

BOOLEAN WaitForProcess(CHAR* ProcessName)
{
	BOOLEAN found = FALSE;
	ULONG i = 0;
	PEPROCESS eproc = NULL;

	// ת�� CHAR* �� WCHAR*
	WCHAR* wideProcessName = ConvertToWideChar(ProcessName);
	if (wideProcessName == NULL)
	{
		return FALSE; // �ڴ����ʧ��
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
			{//ָ�� str �����һ��ƥ���ַ� \ ��ָ�롣���δ�ҵ����ַ����򷵻� NULL��
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
					found = TRUE;
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

	if (found)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] �ҵ�����: %ws\n", wideProcessName);
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] δ�ҵ�����: %s\n", ProcessName);
	}

	return found;
}
VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);  // ��ǲ���δʹ��



}
EXTERN_C
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{

	UNREFERENCED_PARAMETER(RegistryPath);  // ��ǲ���δʹ��

	DriverObject->DriverUnload = DriverUnload;

	while (1)
	{
		if (WaitForProcess("dwm.exe"))
			//	if (WaitForProcess("Bandizip.exe"))
		{

			break;
		}

		// ���û���ҵ����̣�����ѡ��ȴ�һ��ʱ�������
		LARGE_INTEGER shortInterval;
		shortInterval.QuadPart = -10000000LL;  // 1����ӳ٣���λΪ100����
		KeDelayExecutionThread(KernelMode, FALSE, &shortInterval);
	}


	return 0;

}