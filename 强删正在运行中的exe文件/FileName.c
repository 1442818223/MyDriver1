#include<ntifs.h>
#include<ntddk.h>
EXTERN_C PPEB  PsGetProcessPeb(PEPROCESS Process);
EXTERN_C NTSTATUS ZwQuerySystemInformation(
	IN ULONG SystemInformationClass,  //处理进程信息,只需要处理类别为5的即可
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength
);

typedef struct _CURDIRw
{
	UNICODE_STRING DosPath;                                         //0x0
	VOID* Handle;                                                           //0x10
}CURDIRw, * PCURDIRw;

typedef struct _RTL_DRIVE_LETTER_CURDIRw
{
	USHORT Flags;                                                           //0x0
	USHORT Length;                                                          //0x2
	ULONG TimeStamp;                                                        //0x4
	STRING DosPath;                                                 //0x8
}RTL_DRIVE_LETTER_CURDIRw;

typedef struct _RTL_USER_PROCESS_PARAMETERS64w
{
	ULONG MaximumLength;                                                    //0x0
	ULONG Length;                                                           //0x4
	ULONG Flags;                                                            //0x8
	ULONG DebugFlags;                                                       //0xc
	VOID* ConsoleHandle;                                                    //0x10
	ULONG ConsoleFlags;                                                     //0x18
	VOID* StandardInput;                                                    //0x20
	VOID* StandardOutput;                                                   //0x28
	VOID* StandardError;                                                    //0x30
	CURDIRw CurrentDirectory;                                        //0x38
	UNICODE_STRING DllPath;                                         //0x50
	UNICODE_STRING ImagePathName;                                   //0x60
	UNICODE_STRING CommandLine;                                     //0x70
	VOID* Environment;                                                      //0x80
	ULONG StartingX;                                                        //0x88
	ULONG StartingY;                                                        //0x8c
	ULONG CountX;                                                           //0x90
	ULONG CountY;                                                           //0x94
	ULONG CountCharsX;                                                      //0x98
	ULONG CountCharsY;                                                      //0x9c
	ULONG FillAttribute;                                                    //0xa0
	ULONG WindowFlags;                                                      //0xa4
	ULONG ShowWindowFlags;                                                  //0xa8
	UNICODE_STRING WindowTitle;                                     //0xb0
	UNICODE_STRING DesktopInfo;                                     //0xc0
	UNICODE_STRING ShellInfo;                                       //0xd0
	UNICODE_STRING RuntimeData;                                     //0xe0
	RTL_DRIVE_LETTER_CURDIRw CurrentDirectores[32];                  //0xf0
	ULONGLONG EnvironmentSize;                                              //0x3f0
	ULONGLONG EnvironmentVersion;                                           //0x3f8
	VOID* PackageDependencyData;                                            //0x400
	ULONG ProcessGroupId;                                                   //0x408
	ULONG LoaderThreads;                                                    //0x40c
	UNICODE_STRING RedirectionDllName;                              //0x410
	UNICODE_STRING HeapPartitionName;                               //0x420
	ULONGLONG* DefaultThreadpoolCpuSetMasks;                                //0x430
	ULONG DefaultThreadpoolCpuSetMaskCount;                                 //0x438
	ULONG DefaultThreadpoolThreadMaximum;                                   //0x43c
}RTL_USER_PROCESS_PARAMETERS64w, * PRTL_USER_PROCESS_PARAMETERS64w;




typedef struct _MLDR_DATA_TABLE_ENTRYw
{
	LIST_ENTRY InLoadOrderLinks;                                    //0x0
	LIST_ENTRY InMemoryOrderLinks;                                  //0x10
	LIST_ENTRY InInitializationOrderLinks;                          //0x20
	VOID* DllBase;                                                  //0x30
	VOID* EntryPoint;                                               //0x38
	ULONG SizeOfImage;                                              //0x40
	UNICODE_STRING FullDllName;                                     //0x48
	UNICODE_STRING BaseDllName;                                     //0x58
}MLDR_DATA_TABLE_ENTRYw, * PMLDR_DATA_TABLE_ENTRYw;






typedef struct _PEB_LDR_DATAw
{
	ULONG Length;                                                           //0x0
	UCHAR Initialized;                                                      //0x4
	VOID* SsHandle;                                                         //0x8
	LIST_ENTRY InLoadOrderModuleList;                               //0x10
	LIST_ENTRY InMemoryOrderModuleList;                             //0x20
	LIST_ENTRY InInitializationOrderModuleList;                     //0x30
	VOID* EntryInProgress;                                                  //0x40
	UCHAR ShutdownInProgress;                                               //0x48
	VOID* ShutdownThreadId;                                                 //0x50
}PEB_LDR_DATAw, * PPEB_LDR_DATAw;


typedef struct _PEBw
{
	ULONG64 x;
	VOID* Mutant;                                                           //0x8
	VOID* ImageBaseAddress;                                                 //0x10
	PEB_LDR_DATAw* Ldr;														 //0x18

}PEBw, * PPEBw;



typedef struct _MPEB64w
{
	UCHAR InheritedAddressSpace;                                            //0x0
	UCHAR ReadImageFileExecOptions;                                         //0x1
	UCHAR BeingDebugged;                                                    //0x2
	UCHAR BitField;
	UCHAR Padding0[4];                                                      //0x4
	ULONGLONG Mutant;                                                       //0x8
	ULONGLONG ImageBaseAddress;                                             //0x10
	PPEB_LDR_DATAw Ldr;                                                          //0x18
	PRTL_USER_PROCESS_PARAMETERS64w ProcessParameters;                                            //0x20
}MPEB64w, * PMPEB64w;

#pragma pack(4)
typedef struct _PEB_LDR_DATA32w
{
	ULONG Length;                                                           //0x0
	UCHAR Initialized;                                                      //0x4
	ULONG SsHandle;                                                         //0x8
	LIST_ENTRY32 InLoadOrderModuleList;										//0xc
	LIST_ENTRY32 InMemoryOrderModuleList;								   //0x14
	LIST_ENTRY32 InInitializationOrderModuleList;                          //0x1c
	ULONG EntryInProgress;                                                  //0x24
	UCHAR ShutdownInProgress;                                               //0x28
	ULONG ShutdownThreadId;                                                 //0x2c
}PEB_LDR_DATA32w, * PPEB_LDR_DATA32w;

typedef struct _LDR_DATA_TABLE_ENTRY32w
{
	LIST_ENTRY32 InLoadOrderLinks;                                    //0x0
	LIST_ENTRY32 InMemoryOrderLinks;                                  //0x8
	LIST_ENTRY32 InInitializationOrderLinks;                          //0x10
	ULONG DllBase;                                                          //0x18
	ULONG EntryPoint;                                                       //0x1c
	ULONG SizeOfImage;                                                      //0x20
	UNICODE_STRING32 FullDllName;                                     //0x24
	UNICODE_STRING32 BaseDllName;                                     //0x2c
}LDR_DATA_TABLE_ENTRY32w, * PLDR_DATA_TABLE_ENTRY32w;

typedef struct _CURDIR32w
{
	UNICODE_STRING32 DosPath;                                         //0x0
	ULONG Handle;                                                           //0x8
}CURDIR32w, * PCURDIR32w;

typedef struct _RTL_DRIVE_LETTER_CURDIR32w
{
	USHORT Flags;                                                           //0x0
	USHORT Length;                                                          //0x2
	ULONG TimeStamp;                                                        //0x4
	STRING32 DosPath;                                                 //0x8
}RTL_DRIVE_LETTER_CURDIR32w, * PRTL_DRIVE_LETTER_CURDIR32w;

typedef struct _RTL_USER_PROCESS_PARAMETERS32w
{
	ULONG MaximumLength;                                                    //0x0
	ULONG Length;                                                           //0x4
	ULONG Flags;                                                            //0x8
	ULONG DebugFlags;                                                       //0xc
	ULONG ConsoleHandle;                                                    //0x10
	ULONG ConsoleFlags;                                                     //0x14
	ULONG StandardInput;                                                    //0x18
	ULONG StandardOutput;                                                   //0x1c
	ULONG StandardError;                                                    //0x20
	CURDIR32w CurrentDirectory;                                        //0x24
	UNICODE_STRING32 DllPath;                                         //0x30
	UNICODE_STRING32 ImagePathName;                                   //0x38
	UNICODE_STRING32 CommandLine;                                     //0x40
	ULONG Environment;                                                      //0x48
	ULONG StartingX;                                                        //0x4c
	ULONG StartingY;                                                        //0x50
	ULONG CountX;                                                           //0x54
	ULONG CountY;                                                           //0x58
	ULONG CountCharsX;                                                      //0x5c
	ULONG CountCharsY;                                                      //0x60
	ULONG FillAttribute;                                                    //0x64
	ULONG WindowFlags;                                                      //0x68
	ULONG ShowWindowFlags;                                                  //0x6c
	UNICODE_STRING32 WindowTitle;                                     //0x70
	UNICODE_STRING32 DesktopInfo;                                     //0x78
	UNICODE_STRING32 ShellInfo;                                       //0x80
	UNICODE_STRING32 RuntimeData;                                     //0x88
	RTL_DRIVE_LETTER_CURDIR32w CurrentDirectores[32];                  //0x90
	ULONG EnvironmentSize;                                                  //0x290
	ULONG EnvironmentVersion;                                               //0x294
	ULONG PackageDependencyData;                                            //0x298
	ULONG ProcessGroupId;                                                   //0x29c
	ULONG LoaderThreads;                                                    //0x2a0
	UNICODE_STRING32 RedirectionDllName;                              //0x2a4
	UNICODE_STRING32 HeapPartitionName;                               //0x2ac
	ULONG DefaultThreadpoolCpuSetMasks;                                //0x2b4
	ULONG DefaultThreadpoolCpuSetMaskCount;                                 //0x2b8
	ULONG DefaultThreadpoolThreadMaximum;                                   //0x2bc
}RTL_USER_PROCESS_PARAMETERS32w, * PRTL_USER_PROCESS_PARAMETERS32w;

typedef struct _MPEB32w
{
	UCHAR InheritedAddressSpace;                                            //0x0
	UCHAR ReadImageFileExecOptions;                                         //0x1
	UCHAR BeingDebugged;                                                    //0x2
	UCHAR BitField;
	ULONG Mutant;                                                           //0x4
	ULONG ImageBaseAddress;                                                 //0x8
	ULONG Ldr;                                                              //0xc
	ULONG ProcessParameters;                                                //0x10
}MPEB32w, * PMPEB32w;
#pragma pack(8)



EXTERN_C NTSTATUS MmCopyVirtualMemory(
	IN PEPROCESS FromProcess,
	IN CONST VOID* FromAddress,
	IN PEPROCESS ToProcess,
	OUT PVOID ToAddress,
	IN SIZE_T BufferSize,
	IN KPROCESSOR_MODE PreviousMode,
	OUT PSIZE_T NumberOfBytesCopied
);

EXTERN_C void* PsGetProcessWow64Process(PEPROCESS Process);

EXTERN_C PCHAR PsGetProcessImageFileName(PEPROCESS Process);


EXTERN_C NTSTATUS PsReferenceProcessFilePointer(PEPROCESS Process, PFILE_OBJECT* OutFileObject);

EXTERN_C NTSTATUS ObQueryNameString(PVOID Object, POBJECT_NAME_INFORMATION ObjectNameInfo, ULONG Length, PULONG ReturnLength);


UNICODE_STRING GetProcessFullImagePath(HANDLE pid)
{
	UNICODE_STRING failureResult;
	RtlInitUnicodeString(&failureResult, L""); // Initialize with empty string to indicate failure

	PEPROCESS eProcess = NULL;
	PMPEB64w peb64 = NULL;
	PMPEB32w peb32 = NULL;
	PRTL_USER_PROCESS_PARAMETERS64w processParameters64 = NULL;
	PRTL_USER_PROCESS_PARAMETERS32w processParameters32 = NULL;
	KAPC_STATE ks;

	// Get the EPROCESS structure for the given PID
	if (NT_SUCCESS(PsLookupProcessByProcessId(pid, &eProcess)))
	{
		// Attach to the target process
		KeStackAttachProcess(eProcess, &ks);

		// Check if it's a 32-bit or 64-bit process
		peb64 = (PMPEB64w)PsGetProcessPeb(eProcess);
		peb32 = (PMPEB32w)PsGetProcessWow64Process(eProcess); // This will be non-NULL for 32-bit processes

		if (peb32) // If it's a 32-bit process (WOW64)
		{
			if (peb32->ProcessParameters)
			{
				processParameters32 = (PRTL_USER_PROCESS_PARAMETERS32w)peb32->ProcessParameters;
				if (processParameters32)
				{
					UNICODE_STRING32 processImagePath32 = processParameters32->ImagePathName;

					// Convert UNICODE_STRING32 to UNICODE_STRING for 32-bit systems
					UNICODE_STRING processImagePath;
					RtlInitUnicodeString(&processImagePath, (PCWSTR)processImagePath32.Buffer);

					// Allocate memory for the path string
					WCHAR* pathBuffer = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, processImagePath.Length + sizeof(WCHAR), 'Path');
					if (pathBuffer)
					{
						RtlCopyMemory(pathBuffer, processImagePath.Buffer, processImagePath.Length);
						processImagePath.Buffer = pathBuffer;
						processImagePath.Buffer[processImagePath.Length / sizeof(WCHAR)] = L'\0';

						// Detach and cleanup
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
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] ProcessParameters is NULL for 32-bit process\n");
				}
			}
			else
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] PEB32 or ProcessParameters is NULL for 32-bit process\n");
			}
		}
		else if (peb64) // If it's a 64-bit process
		{
			if (peb64->ProcessParameters)
			{
				processParameters64 = (PRTL_USER_PROCESS_PARAMETERS64w)peb64->ProcessParameters;
				if (processParameters64)
				{
					UNICODE_STRING processImagePath = processParameters64->ImagePathName;

					// Allocate memory for the path string
					WCHAR* pathBuffer = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, processImagePath.Length + sizeof(WCHAR), 'Path');
					if (pathBuffer)
					{
						RtlCopyMemory(pathBuffer, processImagePath.Buffer, processImagePath.Length);
						processImagePath.Buffer = pathBuffer;
						processImagePath.Buffer[processImagePath.Length / sizeof(WCHAR)] = L'\0';

						// Detach and cleanup
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
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] ProcessParameters is NULL for 64-bit process\n");
				}
			}
			else
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] PEB64 or ProcessParameters is NULL for 64-bit process\n");
			}
		}
		else
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Unable to determine PEB type (either 32-bit or 64-bit)\n");
		}

		// Detach from the target process if necessary
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


NTSTATUS SelfDeleteFileexe(UNICODE_STRING* path)
{
	// 检查路径是否有效
	if (path == NULL || path->Buffer == NULL || path->Length == 0)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 提供的路径无效！\n");
		return STATUS_INVALID_PARAMETER;
	}

	// 打印原始路径
   // DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 删除文件的原始路径: %wZ\n", path);

	// 定义 \\??\\ 前缀   三环的exe必须加这个前缀
	UNICODE_STRING prefix;
	RtlInitUnicodeString(&prefix, L"\\??\\");

	// 计算新路径长度
	USHORT newLength = prefix.Length + path->Length;
	WCHAR* newBuffer = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, newLength + sizeof(WCHAR), 'path');

	if (newBuffer == NULL)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 内存分配失败！\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// 复制前缀和原始路径到新缓冲区
	RtlCopyMemory(newBuffer, prefix.Buffer, prefix.Length);
	RtlCopyMemory((PUCHAR)newBuffer + prefix.Length, path->Buffer, path->Length);

	// 确保字符串以NULL结尾
	newBuffer[newLength / sizeof(WCHAR)] = UNICODE_NULL;

	// 初始化新的 UNICODE_STRING
	UNICODE_STRING newPath;
	newPath.Length = newLength;
	newPath.MaximumLength = newLength + sizeof(WCHAR);
	newPath.Buffer = newBuffer;

	// 打印新路径
   // DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] 删除文件的完整路径: %wZ\n", &newPath);

	// 初始化对象属性
	OBJECT_ATTRIBUTES objFile = { 0 };
	HANDLE hFile = NULL;
	IO_STATUS_BLOCK ioBlock = { 0 };
	NTSTATUS status;

	InitializeObjectAttributes(&objFile, &newPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

	// 创建文件句柄
	status = ZwCreateFile(
		&hFile,
		GENERIC_READ, // 确保请求 DELETE 权限
		&objFile,
		&ioBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FILE_NON_DIRECTORY_FILE,
		NULL,
		NULL);

	// 释放内存并处理句柄和状态码
	ExFreePool(newBuffer);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] ZwCreateFile 失败，状态码: 0x%X\n", status);
		return status;
	}

	PVOID Object = NULL;
	status = ObReferenceObjectByHandle(
		hFile,
		FILE_ALL_ACCESS, // 确保 DELETE 权限
		*IoFileObjectType,
		KernelMode,
		&Object,
		NULL
	);

	if (!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] ObReferenceObjectByHandle 失败，状态码: 0x%X\n", status);
		return status;
	}

	PFILE_OBJECT pFile = (PFILE_OBJECT)Object;

	pFile->DeleteAccess = TRUE;
	pFile->DeletePending = FALSE;

	pFile->SectionObjectPointer->DataSectionObject = NULL;
	pFile->SectionObjectPointer->ImageSectionObject = NULL;
	//pFile->SectionObjectPointer->SharedCacheMap = NULL;

	ObDereferenceObject(pFile);
	ZwClose(hFile);

	// 删除文件
	status = ZwDeleteFile(&objFile);

	// DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] ZwDeleteFile 状态码: 0x%X\n", status);

	return status;




}


//强删exe文件.
BOOLEAN DeleteExe(HANDLE pid) {

	//HANDLE pid = NULL;
	//while (1)
	//{
	//	if (WaitForProcess("R3.exe"))
	//		//if (WaitForProcess("ida64.exe"))
	//			//if (WaitForProcess("Bandizip.exe"))
	//	{

	//		break;
	//	}

	//	// 如果没有找到进程，可以选择等待一段时间后重试
	//	LARGE_INTEGER shortInterval;
	//	shortInterval.QuadPart = -10000000LL;  // 1秒的延迟，单位为100纳秒
	//	KeDelayExecutionThread(KernelMode, FALSE, &shortInterval);
	//}

	//do {

	//	pid = MyFindProcessByName("R3.exe");
	//	//pid = MyFindProcessByName("ida64.exe");
	//	// MyEnumModule("crossfire.exe", "cshell_x64.dll", &ModuleBase2);
	//	//	pid = MyEnumModule("Bandizip.exe", "ntdll.dll", &ModuleBase);

	//} while (pid == NULL);



	UNICODE_STRING fullPath = GetProcessFullImagePath(pid);

	UNICODE_STRING failureResult;
	RtlInitUnicodeString(&failureResult, L""); // Empty string to indicate failure

	// Compare the result with the failure result
	if (RtlCompareUnicodeString(&fullPath, &failureResult, TRUE) != 0)
	{
		// Path found
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Path found: %wZ\n", &fullPath);


		SelfDeleteFileexe(&fullPath);
	}
	else
	{
		// Path not found
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[db+] Path not found!\n");
	}


	return TRUE;
}
VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	
}
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	DbgBreakPoint();
	DeleteExe(3784);
}