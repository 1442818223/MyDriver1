#include <ntifs.h>
#include <windef.h>
#include <intrin.h>

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

ULONG GetPid(CHAR* ProcessName)
{
	ULONG foundPID = 0;  // ��ʼ��Ϊ 0����ʾδ�ҵ�
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
				WCHAR* fileName = wcsrchr(processNameString->Buffer, L'\\');
				if (fileName)
				{
					fileName++; // ���� '\' �ַ�
				}
				else
				{
					fileName = processNameString->Buffer;
				}

				UNICODE_STRING currentProcessName;
				RtlInitUnicodeString(&currentProcessName, fileName);

				if (RtlEqualUnicodeString(&currentProcessName, &targetProcessName, TRUE))
				{
					foundPID = i;  // �ҵ����̣������� PID
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

	// ɾ�����Զϵ�
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

		*(PUCHAR)((PUCHAR)eprocess + mmm + 2) = 0; //0x72����ס  0�ǽ��
		DbgPrintEx(77, 0, " Process modified successfully\n");
		ObDereferenceObject(eprocess);  // ȷ���ͷŶ�������
	}
	else {
		DbgPrintEx(77, 0, " Process not found\n");
	}

	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}