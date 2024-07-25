#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>

typedef struct _PEB_LDR_DATA64 {
	ULONG Length;
	UCHAR Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID EntryInProgress;
	UCHAR ShutdownInProgress;
	PVOID ShutdownThreadId;
} PEB_LDR_DATA64, * PPEB_LDR_DATA64;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	unsigned short LoadCount;
	unsigned short TlsIndex;
	union {
		LIST_ENTRY HashLinks;
		struct {
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union {
		ULONG TimeDateStamp;
		PVOID LoadedImports;
	};
	PVOID EntryPointActivationContext;
	PVOID PatchInformation;
	LIST_ENTRY ForwarderLinks;
	LIST_ENTRY ServiceTagLinks;
	LIST_ENTRY StaticLinks;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB64 {
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	union {
		UCHAR BitField;
		struct {
			UCHAR ImageUsesLargePages : 1;
			UCHAR IsProtectedProcess : 1;
			UCHAR IsImageDynamicallyRelocated : 1;
			UCHAR SkipPatchingUser32Forwarders : 1;
			UCHAR IsPackagedProcess : 1;
			UCHAR IsAppContainer : 1;
			UCHAR IsProtectedProcessLight : 1;
			UCHAR IsLongPathAwareProcess : 1;
		};
	};
	UCHAR Padding0[4];
	ULONGLONG Mutant;
	ULONGLONG ImageBaseAddress;
	PPEB_LDR_DATA64 Ldr;
	ULONGLONG ProcessParameters;
	ULONGLONG SubSystemData;
	ULONGLONG ProcessHeap;
	ULONGLONG FastPebLock;
	ULONGLONG AtlThunkSListPtr;
	ULONGLONG IFEOKey;
} PEB64, * PPEB64;

typedef struct _CURDIR {
	UNICODE_STRING DosPath;
	VOID* Handle;
} CURDIR, * PCURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	ULONG MaximumLength;
	ULONG Length;
	ULONG Flags;
	ULONG DebugFlags;
	VOID* ConsoleHandle;
	ULONG ConsoleFlags;
	VOID* StandardInput;
	VOID* StandardOutput;
	VOID* StandardError;
	CURDIR CurrentDirectory;
	UNICODE_STRING DllPath;
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

EXTERN_C
NTKERNELAPI PPEB PsGetProcessPeb(PEPROCESS Process);

UNICODE_STRING GetProcessFullImagePath(HANDLE pid)
{
	UNICODE_STRING failureResult;
	RtlInitUnicodeString(&failureResult, L""); // Initialize with empty string to indicate failure

	PEPROCESS eProcess = NULL;
	PPEB64 peb = NULL;
	PRTL_USER_PROCESS_PARAMETERS processParameters = NULL;
	KAPC_STATE ks;

	// Get the EPROCESS structure for the given PID
	if (NT_SUCCESS(PsLookupProcessByProcessId(pid, &eProcess)))
	{
		// Attach to the target process
		KeStackAttachProcess(eProcess, &ks);

		__try
		{
			// Access PEB
			peb = (PPEB64)PsGetProcessPeb(eProcess);
			if (peb && peb->ProcessParameters)
			{
				// Check and cast ProcessParameters
				processParameters = (PRTL_USER_PROCESS_PARAMETERS)peb->ProcessParameters;
				if (processParameters)
				{
					// Ensure ImagePathName is valid before using
					UNICODE_STRING processImagePath = processParameters->ImagePathName;

					// Allocate memory for the path string
					WCHAR* pathBuffer = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, processImagePath.Length + sizeof(WCHAR), 'Path');
					if (pathBuffer)
					{
						RtlCopyMemory(pathBuffer, processImagePath.Buffer, processImagePath.Length);
						processImagePath.Buffer = pathBuffer;
						processImagePath.Buffer[processImagePath.Length / sizeof(WCHAR)] = L'\0';

						// Use the pathBuffer for your operations
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Process Full Path: %wZ\n", &processImagePath);

						// Clean up
						ExFreePoolWithTag(pathBuffer, 'Path');
						KeUnstackDetachProcess(&ks);
						ObDereferenceObject(eProcess);

						// Return the copied ImagePathName
						return processImagePath;
					}
					else
					{
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Failed to allocate memory for path buffer\n");
					}
				}
				else
				{
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] ProcessParameters is NULL\n");
				}
			}
			else
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] PEB or ProcessParameters is NULL\n");
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Exception occurred while accessing process memory\n");
		}

		// Detach from the target process
		KeUnstackDetachProcess(&ks);

		// Release reference count
		ObDereferenceObject(eProcess);
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Failed to get process for PID: %u\n", pid);
	}

	// Return the failure value
	return failureResult;
}


VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);  // Mark parameter as unused
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Driver unloaded successfully!\n");
}

EXTERN_C
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	UNICODE_STRING pathResult = GetProcessFullImagePath((HANDLE)6404);

	// Initialize the failure result for comparison
	UNICODE_STRING failureResult;
	RtlInitUnicodeString(&failureResult, L""); // Empty string to indicate failure

	// Compare the result with the failure result
	if (RtlCompareUnicodeString(&pathResult, &failureResult, TRUE) != 0)
	{
		// Path found
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Path found: %wZ\n", &pathResult);
	}
	else
	{
		// Path not found
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Path not found!\n");
	}

	// Set the driver unload routine
	DriverObject->DriverUnload = DriverUnload;

	return STATUS_SUCCESS;
}
