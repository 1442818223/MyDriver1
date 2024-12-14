#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>

#include "ssdt.h"
#include "Module.h"
#include "tools.h"


int ReadIndex(ULONG64 addr, int offset)
{
	PUCHAR base = NULL;
	SIZE_T size = PAGE_SIZE;
	NTSTATUS status = ZwAllocateVirtualMemory(NtCurrentProcess(), &base, 0, &size, MEM_COMMIT, PAGE_READWRITE);

	if (!NT_SUCCESS(status)) return -1;

	ULONG proc = NULL;

	memset(base, 0, size);

	//Copy是为了防止缺页
	status = MmCopyVirtualMemory(IoGetCurrentProcess(), addr, IoGetCurrentProcess(), base, 0x300, UserMode, &proc);

	if (!NT_SUCCESS(status))
	{
		ZwFreeVirtualMemory(NtCurrentProcess(), &base, &size, MEM_RELEASE);
		return -1;
	}

	//int index = *(int*)(base + 0x4);
	ULONG number = GetWindowsVersionNumber();
	int index = 0;
	if (!offset)
	{
		index = *(int*)(base + 0x4);
	}
	else
	{
		if (number != 7)
		{
			index = *(int*)(base + 0x4);
		}
		else
		{
			PUCHAR temp = base + offset;
			for (int i = 0; i < 200; i++)
			{
				if (temp[i] == 0x4C && temp[i + 1] == 0x8B && temp[i + 2] == 0xD1)
				{
					index = *(int*)(temp + i + 4);
					break;
				}
			}
		}
	}

	ZwFreeVirtualMemory(NtCurrentProcess(), &base, &size, MEM_RELEASE);

	return index;
}


int GetFuncIndexPlus(char* funcName, int subAddr, int offset)
{
	int index = -1;
	PEPROCESS Process = FindProcess("explorer.exe");//这里要附加 因为驱动层没有dll模块
	if (Process == NULL) return index;


	char* moudleName = "ntdll.dll";

	char* FuncName = funcName;

	ULONG_PTR imageBase = 0;
	ULONG_PTR imageSize = 0;
	KAPC_STATE kApcState = { 0 };

	KeStackAttachProcess(Process, &kApcState);

	do
	{

		imageBase = GetModuleR3(PsGetProcessId(Process), moudleName, &imageSize);

		if (!imageBase) break;

		ULONG_PTR funcAddr = GetProcAddressR(imageBase, FuncName, TRUE);

		if (!funcAddr) break;

		if (subAddr)
		{
			funcAddr -= subAddr;
		}

		index = ReadIndex(funcAddr, offset);

	} while (0);



	KeUnstackDetachProcess(&kApcState);

	ObDereferenceObject(Process);

	return index;
}

int GetFuncIndex(char* funcName, int offset)
{
	return GetFuncIndexPlus(funcName, 0, offset);
}
int GetNtSuspendThreadIndex()
{
	static int index = -1;
	if (index != -1) return index;

	char* FuncName = NULL;

	FuncName = "NtSuspendThread";   //找到上面导出的定位下面没导出的

	index = GetFuncIndex(FuncName, 0);
	return index;
}


void DriverUnload(PDRIVER_OBJECT DriverObject) {
	
	DbgPrintEx(77, 0, "Driver unloaded\n");
}

EXTERN_C
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	
	DriverObject->DriverUnload = DriverUnload;

	int a= GetNtSuspendThreadIndex();
	DbgPrintEx(77, 0, "索引%x\n", a);

	return STATUS_SUCCESS;
}

