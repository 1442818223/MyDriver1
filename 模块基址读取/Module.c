#include "Module.h"

ULONG_PTR GetModuleX86(PEPROCESS Process, PPEB32 peb32, PUNICODE_STRING ModuleName, PULONG_PTR SizeImage) {
	SIZE_T retSize = 0;
	NTSTATUS status = MmCopyVirtualMemory(Process, peb32, Process, peb32, 0x1, UserMode, &retSize); //自己读下自己防止缺页蓝屏


	PPEB_LDR_DATA32 pebldr = (PPEB_LDR_DATA32)peb32->Ldr;
	PLIST_ENTRY32 pList32 = (PLIST_ENTRY32)&pebldr->InLoadOrderModuleList;
	PLDR_DATA_TABLE_ENTRY32 plistNext = (PLDR_DATA_TABLE_ENTRY32)ULongToPtr(pList32->Flink);

	ULONG_PTR Module = 0;
	while (pList32 != plistNext) {
		PWCH baseDllName = (PWCH)UlongToPtr(plistNext->BaseDllName.Buffer);
		UNICODE_STRING uBaseName = { 0 };
		RtlInitUnicodeString(&uBaseName, baseDllName);

		if (RtlCompareUnicodeString(&uBaseName, ModuleName, TRUE) == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Base: %p, Size: %llu\n", plistNext->DllBase, plistNext->SizeOfImage);
			Module =plistNext->DllBase;
			if (SizeImage)  *SizeImage = plistNext->SizeOfImage;
			break;
		}

		plistNext = (PLDR_DATA_TABLE_ENTRY32)UlongToPtr(plistNext->InLoadOrderLinks.Flink);
	}

	return Module;
}

ULONG_PTR GetModuleX64(PEPROCESS Process, PPEB peb, PUNICODE_STRING ModuleName, PULONG_PTR SizeImage) {
	SIZE_T retSize = 0;
	NTSTATUS status = MmCopyVirtualMemory(Process, peb, Process, peb, 0x1, UserMode, &retSize);


	PPEB_LDR_DATA pebldr = (PPEB_LDR_DATA)peb->Ldr;
	PLIST_ENTRY pList = &pebldr->InLoadOrderModuleList;
	PLDR_DATA_TABLE_ENTRY plistNext = (PLDR_DATA_TABLE_ENTRY)pList->Flink;

	ULONG_PTR Module = 0;
	while (pList != plistNext) {

		if (RtlCompareUnicodeString(&plistNext->BaseDllName, ModuleName, TRUE) == 0) {
			DbgPrintEx(77, 0, "[GetModuleR3]: imageBase = %p, sizeofimage = %x\n", plistNext->DllBase, plistNext->SizeOfImage);
			Module = plistNext->DllBase;
			if (SizeImage) *SizeImage = plistNext->SizeOfImage;
			break;
		}

		plistNext = (PLDR_DATA_TABLE_ENTRY)plistNext->InLoadOrderLinks.Flink;
	}

	return Module;
}




ULONG_PTR GetModuleR3(HANDLE pid, char* moduleName, PULONG_PTR sizeImage) {
	if (!moduleName) return 0;

	PEPROCESS Process = NULL;
	KAPC_STATE ApcState = { 0 };
	ULONG_PTR module = 0;
	NTSTATUS status = PsLookupProcessByProcessId(pid, &Process);
	if (!NT_SUCCESS(status)) {
		return 0;
	}

	STRING aModuleName = { 0 };
	UNICODE_STRING uModuleName = { 0 };
	RtlInitAnsiString(&aModuleName, moduleName);
	status = RtlAnsiStringToUnicodeString(&uModuleName, &aModuleName, TRUE);
	if (!NT_SUCCESS(status)) {
		ObDereferenceObject(Process);
		return 0;
	}
	_wcsupr(uModuleName.Buffer);

	KeStackAttachProcess(Process, &ApcState);

	do {
		PPEB peb = PsGetProcessPeb(Process);
		PPEB32 peb32 = (PPEB32)PsGetProcessWow64Process(Process);
		if (peb32) {
			module = GetModuleX86(Process, peb32, &uModuleName, sizeImage);
		}
		else {
			module = GetModuleX64(Process, peb, &uModuleName, sizeImage);
		}
	} while (0);

	KeUnstackDetachProcess(&ApcState);
	RtlFreeUnicodeString(&uModuleName);
	ObDereferenceObject(Process);

	return module;
}

