#include<ntifs.h>
#include<ntddk.h>
#include"driverentry.h"

EXTERN_C NTKERNELAPI PPEB PsGetProcessPeb(PEPROCESS Process);

HIDE_MEMORY_BUFFER HideMemoryList[100];



typedef struct _MEMLOAD_DATA
{
	ULONG MemVadRootOffset;
	ULONG BuildNumber;
} MEMLOAD_DATA, * PMEMLOAD_DATA;

MEMLOAD_DATA MapLoad;


VOID RtlZeroMemoryEx(PVOID Dst, SIZE_T Size) {

	for (SIZE_T i = 0; i < Size; i++) {

		((CHAR*)Dst)[i] = (CHAR)0;
	}

}
VOID RtlCopyMemoryEx(PVOID Dst, PVOID Src, SIZE_T Size) {
	for (SIZE_T i = 0; i < Size; i++) {

		((CHAR*)Dst)[i] = ((CHAR*)Src)[i];
	}

}


TABLE_SEARCH_RESULT WIN1X_MiFindNodeOrParent(WIN1X_PMM_AVL_TABLE Table, ULONG_PTR StartingVpn, WIN1X_PMM_AVL_NODE* NodeOrParent)
{
	WIN1X_PMM_AVL_NODE Child;

	WIN1X_PMM_AVL_NODE NodeToExamine;

	WIN1X_PMMVAD_SHORT VpnCompare;

	ULONG_PTR startVpn;

	ULONG_PTR endVpn;

	if (Table->NumberGenericTableElements != 0) {

		NodeToExamine = (WIN1X_PMM_AVL_NODE)(Table->BalancedRoot);

		for (;;) {

			VpnCompare = (WIN1X_PMMVAD_SHORT)NodeToExamine;

			startVpn = VpnCompare->StartingVpn;

			endVpn = VpnCompare->EndingVpn;

			startVpn |= (ULONGLONG)VpnCompare->StartingVpnHigh << 32;

			endVpn |= (ULONGLONG)VpnCompare->EndingVpnHigh << 32;

			if (StartingVpn < startVpn) {

				Child = NodeToExamine->LeftChild;

				if (Child != NULL) {

					NodeToExamine = Child;
				}
				else {

					//
					// Node is not in the tree.  Set the output
					// parameter to point to what would be its
					// parent and return which child it would be.
					//

					*NodeOrParent = NodeToExamine;

					return TableInsertAsLeft;
				}
			}
			else if (StartingVpn <= endVpn) {

				//
				// This is the node.
				//

				*NodeOrParent = NodeToExamine;

				return TableFoundNode;
			}
			else {

				Child = NodeToExamine->RightChild;

				if (Child != NULL) {

					NodeToExamine = Child;
				}
				else {

					//
					// Node is not in the tree.  Set the output
					// parameter to point to what would be its
					// parent and return which child it would be.
					//

					*NodeOrParent = NodeToExamine;

					return TableInsertAsRight;
				}
			}
		};
	}

	return TableEmptyTree;
}



NTSTATUS VadHideMemory(PEPROCESS pProcess, PVOID Address, SIZE_T Size, BOOLEAN Mdl, PHIDE_MEMORY_BUFFER Buffer)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	WIN1X_PMM_AVL_TABLE pTable = (WIN1X_PMM_AVL_TABLE)((PUCHAR)pProcess + MapLoad.MemVadRootOffset);

	WIN1X_PMM_AVL_NODE pNode = NULL;

	ULONGLONG VpnStart = (ULONGLONG)Address >> PAGE_SHIFT;

	if (WIN1X_MiFindNodeOrParent(pTable, VpnStart, &pNode) == TableFoundNode) {

		WIN1X_PMMVAD_SHORT pVadShort = (WIN1X_PMMVAD_SHORT)pNode;

		Buffer->pMdl = NULL;

		Buffer->pProcess = pProcess;

		Buffer->pVadShort = pVadShort;

		Buffer->StartingVpn = pVadShort->StartingVpn;

		Buffer->EndingVpn = pVadShort->EndingVpn;

		if (Mdl == TRUE) {

			PMDL pMdl = IoAllocateMdl(Address, (ULONG)(Size), FALSE, FALSE, NULL);

			if (pMdl) {

				//��Ҫ���ڽ�һ������ҳ�������������ڴ���
				MmProbeAndLockPages(pMdl, UserMode, IoReadAccess);

				Buffer->pMdl = pMdl;
			}
		}

		pVadShort->StartingVpn = pVadShort->EndingVpn;

		Status = STATUS_SUCCESS;
	}

	return Status;
}



NTSTATUS VadShowMemory(PHIDE_MEMORY_BUFFER Buffer) {

	NTSTATUS Status = STATUS_UNSUCCESSFUL;


	WIN1X_PMMVAD_SHORT pVadShort = (WIN1X_PMMVAD_SHORT)Buffer->pVadShort;

	pVadShort->StartingVpn = (ULONG)Buffer->StartingVpn;

	if (Buffer->pMdl != NULL) {

		MmUnlockPages(Buffer->pMdl);

		IoFreeMdl(Buffer->pMdl);
	}

	Status = STATUS_SUCCESS;

	return Status;
}



NTSTATUS AddHideMemoryItem(PEPROCESS pProcess, PVOID Address, SIZE_T Size, BOOLEAN Mdl) {

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	for (SIZE_T i = 0; i < ARRAYSIZE(HideMemoryList); i++) {

		if (pProcess != NULL && HideMemoryList[i].pProcess == NULL) {

			HIDE_MEMORY_BUFFER Buffer;

			RtlZeroMemoryEx(&Buffer, sizeof(Buffer));

			Status = VadHideMemory(pProcess, Address, Size, Mdl, &Buffer);

			if (NT_SUCCESS(Status)) {

				RtlCopyMemoryEx(&HideMemoryList[i], &Buffer, sizeof(HIDE_MEMORY_BUFFER));

				break;
			}
		}
	}

	return Status;
}

NTSTATUS DelHideMemoryItem(PEPROCESS pProcess) {

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	for (SIZE_T i = 0; i < ARRAYSIZE(HideMemoryList); i++) {

		if (HideMemoryList[i].pProcess != NULL && HideMemoryList[i].pProcess == pProcess) {

			Status = VadShowMemory(&HideMemoryList[i]);

			if (NT_SUCCESS(Status)) {

				RtlZeroMemoryEx(&HideMemoryList[i], sizeof(HideMemoryList[i]));
			}
		}
	}

	return Status;
}




ULONG64 LdrInPebOffset = 0x018;
ULONG64 ModListInPebOffset = 0x010;
typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;// ������˳�����ӵ�����
	LIST_ENTRY InMemoryOrderLinks;// ���ڴ�˳�����ӵ�����
	LIST_ENTRY InInitializationOrderLinks;// ����ʼ��˳�����ӵ�����
	PVOID DllBase;// DLL �Ļ���ַ
	PVOID EntryPoint;// DLL ����ڵ��ַ  
	ULONG SizeOfImage;// DLL ӳ��Ĵ�С
	UNICODE_STRING FullDllName;// ������ DLL ����
	UNICODE_STRING BaseDllName;// DLL �Ļ�������
	ULONG Flags;// ��־
	USHORT LoadCount;// װ�ؼ�����
	USHORT TlsIndex;// TLS ����
	union {
		LIST_ENTRY HashLinks;// ��ϣ����
		struct {
			PVOID SectionPointer;// ��ָ��
			ULONG CheckSum;// У���
		};
	};
	union {
		struct {
			ULONG TimeDateStamp;// ʱ���
		};
		struct {
			PVOID LoadedImports;// �Ѽ��صĵ����
		};
	};
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;
VOID EnumModule(PEPROCESS Process, char* DllName, ULONG_PTR* DllBase)
{
	SIZE_T Peb = 0;
	SIZE_T Ldr = 0;
	PLIST_ENTRY ModListHead = 0;
	PLIST_ENTRY Module = 0;
	KAPC_STATE ks;
	UNICODE_STRING targetDllName;
	ANSI_STRING ansiString;

	if (!MmIsAddressValid(Process))
		return;

	Peb = (SIZE_T)PsGetProcessPeb(Process);

	if (!Peb)
		return;

	KeStackAttachProcess(Process, &ks);
	__try
	{
		Ldr = Peb + (SIZE_T)LdrInPebOffset;
		ProbeForRead((CONST PVOID)Ldr, 8, 8);
		ModListHead = (PLIST_ENTRY)(*(PULONG64)Ldr + ModListInPebOffset);
		ProbeForRead((CONST PVOID)ModListHead, 8, 8);
		Module = ModListHead->Flink;

		RtlInitAnsiString(&ansiString, DllName);
		RtlAnsiStringToUnicodeString(&targetDllName, &ansiString, TRUE);

		while (ModListHead != Module)
		{
			PLDR_DATA_TABLE_ENTRY entry = (PLDR_DATA_TABLE_ENTRY)Module;
			if (RtlCompareUnicodeString(&entry->BaseDllName, &targetDllName, TRUE) == 0)
			{
				*DllBase = (ULONG_PTR)entry->DllBase;
				//DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+]�ҵ� DLL ��ַ=%p ��С=%ld ·��=%wZ\n", entry->DllBase,entry->SizeOfImage, &entry->FullDllName);
				break;
			}
			Module = Module->Flink;
			ProbeForRead((CONST PVOID)Module, 80, 8);
		}
		RtlFreeUnicodeString(&targetDllName);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { ; }

	KeUnstackDetachProcess(&ks);
}

PEPROCESS LookupProcess(HANDLE Pid)
{
	PEPROCESS eprocess = NULL;
	if (NT_SUCCESS(PsLookupProcessByProcessId(Pid, &eprocess)))
		return eprocess;
	else
		return NULL;
}
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

HANDLE MyEnumModule(char* ProcessName, char* DllName, ULONG_PTR* DllBase)
{
	ULONG i = 0;
	PEPROCESS eproc = NULL;

	// ת�� CHAR* �� WCHAR*
	WCHAR* wideProcessName = ConvertToWideChar(ProcessName);
	if (wideProcessName == NULL)
	{
		return NULL; // �ڴ����ʧ��
	}

	UNICODE_STRING targetProcessName;
	RtlInitUnicodeString(&targetProcessName, wideProcessName);

	for (i = 4; i < 100000000; i += 4)
	{
		eproc = LookupProcess((HANDLE)i);
		if (eproc != NULL)
		{
			PUNICODE_STRING processNameString = NULL;
			NTSTATUS status = SeLocateProcessImageName(eproc, &processNameString);

			if (NT_SUCCESS(status) && processNameString->Length > 0)
			{
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
					ExFreePoolWithTag(wideProcessName, 0); // ���ҵ����ͷ��ڴ�
					ObDereferenceObject(eproc);
					EnumModule(eproc, DllName, DllBase); // ���� EnumModule ����
					return (HANDLE)i;  // ����ƥ���PID
				}

				ExFreePoolWithTag(processNameString, 0); // �ͷŽ�������
			}

			ObDereferenceObject(eproc);
		}
	}

	ExFreePoolWithTag(wideProcessName, 0); // �ͷ�ת���������
	return NULL;  // δ�ҵ�ƥ��Ľ���
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	// ִ���������
	DbgPrint("Driver Unloading...\n");


}
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath) 
{
	RTL_OSVERSIONINFOW OSVersion = { sizeof(RTL_OSVERSIONINFOW) };
	RtlGetVersion(&OSVersion);
	MapLoad.BuildNumber = OSVersion.dwBuildNumber;


		//_EPROCESS  -> struct _RTL_AVL_TREE VadRoot;                                        
		MapLoad.MemVadRootOffset = (MapLoad.BuildNumber <= 10240) ? 0x608 :
			(MapLoad.BuildNumber <= 10586) ? 0x610 :
			(MapLoad.BuildNumber <= 14393) ? 0x620 :
			(MapLoad.BuildNumber <= 17763) ? 0x628 :
			(MapLoad.BuildNumber <= 18363) ? 0x658 : 0x7D8;
	




		ULONG_PTR base = 0;
		 HANDLE pid= MyEnumModule("Bandizip.exe","ark.x64.dll",&base);

		 PEPROCESS Process = NULL;
		 NTSTATUS status = PsLookupProcessByProcessId(pid, &Process);
		 KAPC_STATE ks;
		 KeStackAttachProcess(Process, &ks);

		AddHideMemoryItem(IoGetCurrentProcess(), base, PAGE_SIZE +PAGE_SIZE, TRUE);


		KeUnstackDetachProcess(&ks);



		pDriverObject->DriverUnload = DriverUnload;


		return STATUS_SUCCESS;

}



