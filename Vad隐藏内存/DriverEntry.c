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

				//主要用于将一组虚拟页面锁定到物理内存中
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
	LIST_ENTRY InLoadOrderLinks;// 按加载顺序链接的链表
	LIST_ENTRY InMemoryOrderLinks;// 按内存顺序链接的链表
	LIST_ENTRY InInitializationOrderLinks;// 按初始化顺序链接的链表
	PVOID DllBase;// DLL 的基地址
	PVOID EntryPoint;// DLL 的入口点地址  
	ULONG SizeOfImage;// DLL 映像的大小
	UNICODE_STRING FullDllName;// 完整的 DLL 名称
	UNICODE_STRING BaseDllName;// DLL 的基本名称
	ULONG Flags;// 标志
	USHORT LoadCount;// 装载计数器
	USHORT TlsIndex;// TLS 索引
	union {
		LIST_ENTRY HashLinks;// 哈希链接
		struct {
			PVOID SectionPointer;// 段指针
			ULONG CheckSum;// 校验和
		};
	};
	union {
		struct {
			ULONG TimeDateStamp;// 时间戳
		};
		struct {
			PVOID LoadedImports;// 已加载的导入表
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
				//DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+]找到 DLL 基址=%p 大小=%ld 路径=%wZ\n", entry->DllBase,entry->SizeOfImage, &entry->FullDllName);
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

HANDLE MyEnumModule(char* ProcessName, char* DllName, ULONG_PTR* DllBase)
{
	ULONG i = 0;
	PEPROCESS eproc = NULL;

	// 转换 CHAR* 到 WCHAR*
	WCHAR* wideProcessName = ConvertToWideChar(ProcessName);
	if (wideProcessName == NULL)
	{
		return NULL; // 内存分配失败
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
					ExFreePoolWithTag(wideProcessName, 0); // 在找到后释放内存
					ObDereferenceObject(eproc);
					EnumModule(eproc, DllName, DllBase); // 调用 EnumModule 处理
					return (HANDLE)i;  // 返回匹配的PID
				}

				ExFreePoolWithTag(processNameString, 0); // 释放进程名称
			}

			ObDereferenceObject(eproc);
		}
	}

	ExFreePoolWithTag(wideProcessName, 0); // 释放转换后的名称
	return NULL;  // 未找到匹配的进程
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	// 执行清理操作
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



