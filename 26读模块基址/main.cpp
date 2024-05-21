#include<ntifs.h>
#include <ntddk.h>
#include<windef.h>
#include<ntdef.h>


// �����豸����
#define _DEVICE_NAME L"\\device\\mydevice"
// ���������������
#define _SYB_NAME	 L"\\??\\sysmblicname"
// ���� IOCTL ������
#define CTL_TALK  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x9000, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct DATA
{
	HANDLE pid;//Ҫ��д�Ľ���ID
	unsigned __int64 address;//Ҫ��д�ĵ�ַ
	DWORD size;//��д����
	PVOID data;//Ҫ��д������,  ��ָ�� ��Ӧ�κ���������

	WCHAR ModuleName[100];           //ģ����
}Data, * PDATA;

typedef struct ��
{
	DWORD  outֵ = 0; // Ҫ����ʲô�������;Ͷ���ʲô����
	ULONGLONG outmodule = 0;
}cu, * pcu;


typedef struct _PEB_LDR_DATA64
{
	ULONG Length;                                                           //0x0
	UCHAR Initialized;                                                      //0x4
	PVOID SsHandle;                                                         //0x8
	LIST_ENTRY InLoadOrderModuleList;                               //0x10
	LIST_ENTRY InMemoryOrderModuleList;                             //0x20
	LIST_ENTRY InInitializationOrderModuleList;                     //0x30
	PVOID EntryInProgress;                                                  //0x40
	UCHAR ShutdownInProgress;                                               //0x48
	PVOID ShutdownThreadId;                                                 //0x50
}PEB_LDR_DATA64, * PPEB_LDR_DATA64;


typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks; // ������˳�����ӵ�����
	LIST_ENTRY InMemoryOrderLinks;// ���ڴ�˳�����ӵ�����
	LIST_ENTRY InInitializationOrderLinks;// ����ʼ��˳�����ӵ�����
	PVOID DllBase;// DLL �Ļ���ַ
	PVOID EntryPoint;// DLL ����ڵ��ַ  
	ULONG SizeOfImage;// DLL ӳ��Ĵ�С
	UNICODE_STRING FullDllName;// ������ DLL ����
	UNICODE_STRING BaseDllName;// DLL �Ļ�������
	ULONG Flags;// ��־
	WORD LoadCount;// װ�ؼ�����
	WORD TlsIndex;// TLS ����
	union
	{
		LIST_ENTRY HashLinks;
		struct
		{
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union
	{
		ULONG TimeDateStamp;
		PVOID LoadedImports;
	};
	PVOID EntryPointActivationContext;
	PVOID PatchInformation;
	LIST_ENTRY ForwarderLinks;
	LIST_ENTRY ServiceTagLinks;
	LIST_ENTRY StaticLinks;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB64
{
	UCHAR InheritedAddressSpace;                                            //0x0
	UCHAR ReadImageFileExecOptions;                                         //0x1
	UCHAR BeingDebugged;                                                    //0x2
	union
	{
		UCHAR BitField;                                                     //0x3
		struct
		{
			UCHAR ImageUsesLargePages : 1;                                    //0x3
			UCHAR IsProtectedProcess : 1;                                     //0x3
			UCHAR IsImageDynamicallyRelocated : 1;                            //0x3
			UCHAR SkipPatchingUser32Forwarders : 1;                           //0x3
			UCHAR IsPackagedProcess : 1;                                      //0x3
			UCHAR IsAppContainer : 1;                                         //0x3
			UCHAR IsProtectedProcessLight : 1;                                //0x3
			UCHAR IsLongPathAwareProcess : 1;                                 //0x3
		};
	};
	UCHAR Padding0[4];                                                      //0x4
	ULONGLONG Mutant;                                                       //0x8
	ULONGLONG ImageBaseAddress;                                             //0x10
	PEB_LDR_DATA64* Ldr;  //ָ����̼��������ݽṹ��ָ�롣
	ULONGLONG ProcessParameters;                                            //0x20
	ULONGLONG SubSystemData;                                                //0x28
	ULONGLONG ProcessHeap;                                                  //0x30
	ULONGLONG FastPebLock;                                                  //0x38
	ULONGLONG AtlThunkSListPtr;                                             //0x40
	ULONGLONG IFEOKey;                                                      //0x48
}PEB64, * PPEB64;


extern "C"
PPEB NTAPI PsGetProcessPeb(IN PEPROCESS Process);


ULONGLONG PsGetModuleBase(IN HANDLE Pid, IN PUNICODE_STRING ModuleName) {
	PEPROCESS Process = NULL;
	ULONGLONG ModuleBase = 0;
	NTSTATUS status = STATUS_SUCCESS;
	KAPC_STATE kapc_state = { 0 };
	ULONG64 dllBaseAddr = 0;

	// ���ҽ���
	status = PsLookupProcessByProcessId((HANDLE)Pid, &Process);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Lookup Failed\n"));
		return 0;
	}
	// ���ӵ�Ŀ�����
	KeStackAttachProcess(Process, &kapc_state);
	
	PPEB64 Peb = NULL;
	// ��ȡPEB
	Peb = (PPEB64)PsGetProcessPeb(Process);
	if (Peb == NULL) {
		ObDereferenceObject(Process);
		KdPrint(("Lookup Failed\n"));
		KeUnstackDetachProcess(&kapc_state);
		return 0;
	}
	
	// ��ȡPEB_LDR_DATA
	PPEB_LDR_DATA64 PebLdrData = (PPEB_LDR_DATA64)Peb->Ldr;
	if (PebLdrData == NULL) {
		ObDereferenceObject(Process);
		KeUnstackDetachProcess(&kapc_state);
		return 0;
	}
	PLIST_ENTRY ListEntryStart = NULL, ListEntryEnd = NULL;
	PLDR_DATA_TABLE_ENTRY LdrDataEntry = NULL;

	ListEntryStart = ListEntryEnd = PebLdrData->InMemoryOrderModuleList.Blink;
	__try {
		do {
			
			LdrDataEntry =(PLDR_DATA_TABLE_ENTRY)CONTAINING_RECORD(ListEntryStart, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
			if (RtlCompareUnicodeString(&LdrDataEntry->BaseDllName, ModuleName, TRUE) == 0) {
				dllBaseAddr = (ULONG64)LdrDataEntry->DllBase;
				break;
			}
			ListEntryStart = ListEntryStart->Blink;
		} while (ListEntryStart != ListEntryEnd);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint("Bad memory access\n");
		dllBaseAddr = 0;
	}

	ObDereferenceObject(Process);
	KeUnstackDetachProcess(&kapc_state);

	return dllBaseAddr;
}



// �����봦����
NTSTATUS DispatchControl(PDEVICE_OBJECT pDevice, PIRP pIrp) {
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

	if (pStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
		PVOID pBuff = pIrp->AssociatedIrp.SystemBuffer;
		ULONG CtlCode = pStack->Parameters.DeviceIoControl.IoControlCode;
		int inputbufferlength = pStack->Parameters.DeviceIoControl.InputBufferLength;
		int outputbufferlength = pStack->Parameters.DeviceIoControl.OutputBufferLength;

		NTSTATUS status = STATUS_SUCCESS;
		PEPROCESS process = NULL;
		KAPC_STATE apcstate = { 0 };
		PMDL mdl = NULL;
		PVOID mappedAddr = NULL;
		ULONG64 dllBaseAddr = 0;


		switch (CtlCode) {
		case CTL_TALK: {
			PDATA data = (PDATA)pBuff;
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "inputbufferlength: %d,outputbufferlength: %d\n", inputbufferlength, outputbufferlength);




			//��ȡ���̶���
			status = PsLookupProcessByProcessId(data->pid, &process);
			if (!NT_SUCCESS(status)) {
				DbgPrintEx(77, 0, "Failed to get process object\n");
				return status;
			}

			//���ӽ���
			KeStackAttachProcess(process, &apcstate);

			__try {
				//MDL ��һ�ֿ�������һ�鲻���������ڴ�ҳ�Ľṹ��ϵͳͨ���������������ڴ��������ڴ�֮���ӳ�䡣
			   //�����ڴ��������б� (MDL)    
				mdl = IoAllocateMdl((PVOID)data->address, data->size, FALSE, FALSE, NULL);
				if (!mdl) {
					DbgPrintEx(77, 0, "Failed to get process object\n");
					status = STATUS_INSUFFICIENT_RESOURCES;
					__leave; // �����˳� __try ��
				}

				//��ȷ���ڴ汻�������Ҳ��ᱻ����ϵͳ����������
				MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);


				//������ڳɹ�ʱ����һ�������ַ���õ�ַ�������ں�ģʽ��ʹ�ã����Է��ʻ����������ݡ�
				// ���޷�ӳ�������ҳ���ʱ�����᷵��NULL��
				//// ��ȡϵͳ��ַ
				mappedAddr = MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority);
				if (!mappedAddr) {
					status = STATUS_INSUFFICIENT_RESOURCES;
					__leave;
				}



				�� x = { 0 };

				UNICODE_STRING moudlename = { 0 };
				RtlInitUnicodeString(&moudlename, data->ModuleName);
				x.outmodule = PsGetModuleBase(data->pid, &moudlename);//��ȡģ���ַ
			
				PVOID ��ʳ = ExAllocatePoolWithTag(NonPagedPool, data->size, 'tag1'); // ����Ƿ�ҳ���ڴ�
				RtlCopyMemory(��ʳ, mappedAddr, data->size);
				if (��ʳ) {
					x.outֵ = *(DWORD*)��ʳ;

					RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, &x, sizeof(��));
					pIrp->IoStatus.Information = sizeof(��);
				}
				else {
					status = STATUS_INSUFFICIENT_RESOURCES;
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				status = GetExceptionCode();
			}

			if (mdl) {
				MmUnlockPages(mdl);
				IoFreeMdl(mdl);
			}

			KeUnstackDetachProcess(&apcstate);

			//���ں˶���������-1
			ObDereferenceObject(process);
			break;
		}
		default:
			DbgPrintEx(77, 0, "Invalid control code\n");
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}

		pIrp->IoStatus.Status = status;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return status;
	}

	return STATUS_INVALID_DEVICE_REQUEST;
}

// ��IRP_MJ_READ�ķ�ʽ��������
NTSTATUS ReadpatchControl(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
	// ��ȡ IRP ջ��Ϣ(������)
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

	// ��ȡ������
	LARGE_INTEGER ByteOffset = pStack->Parameters.Read.ByteOffset;
	ULONG Length = pStack->Parameters.Read.Length;

	// ��ȡ���뻺����ָ��
	int* pBuff = (int*)pIrp->AssociatedIrp.SystemBuffer;   // ��Ӧ3������Ҫ���ĵ�ַ   
	// ������յ�������
	DbgPrintEx(77, 0, "[db]:%d\r\n", *pBuff); //��ʱ��0,������û�õ���ûӳ��

	*pBuff = 100;

	// ���÷�����Ϣ
	pIrp->IoStatus.Information = Length;     //���س��ȱ�������,�����þͷ�����ȥ
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// ��� IRP ����
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


// ������ǲ����
NTSTATUS DisPatchCreate(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
	// ��������ɹ���Ϣ
	DbgPrintEx(77, 0, "������ǲͨ�ųɹ�\n");

	// ��� IRP ����
	IoCompleteRequest(pIrp, 0);

	return STATUS_SUCCESS;
}

// �ر��豸�Ĳ�����ǲ����
NTSTATUS DisPatchDetach(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
	// ��������ɹ���Ϣ
	DbgPrintEx(77, 0, " �ر��豸��ǲͨ�ųɹ�\n");

	// ��� IRP ����
	IoCompleteRequest(pIrp, 0);

	return STATUS_SUCCESS;
}

// ж����������
VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	// ���ж�سɹ���Ϣ
	DbgPrintEx(77, 0, "ж�سɹ�\n");

	// ɾ����������
	UNICODE_STRING uSymblicLinkname;
	RtlInitUnicodeString(&uSymblicLinkname, _SYB_NAME);
	IoDeleteSymbolicLink(&uSymblicLinkname);

	// ɾ���豸����
	if (pDriver->DeviceObject) {
		IoDeleteDevice(pDriver->DeviceObject);
	}
}


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{
	// ������سɹ���Ϣ
	DbgPrintEx(77, 0, "���سɹ�\n");

	
	// ����ж�غ���
	pDriver->DriverUnload = UnloadDriver;

	UNICODE_STRING uDeviceName;
	UNICODE_STRING uSymbliclinkname;
	PDEVICE_OBJECT pDevice;

	// ��ʼ���豸���ƺͷ�����������
	RtlInitUnicodeString(&uDeviceName, _DEVICE_NAME);
	RtlInitUnicodeString(&uSymbliclinkname, _SYB_NAME);

	// �����豸����                                                ��ʾ���豸֧�ְ�ȫ��
	IoCreateDevice(pDriver, 0, &uDeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDevice);
	// ������������
	IoCreateSymbolicLink(&uSymbliclinkname, &uDeviceName);

	// �����豸�����־
	pDevice->Flags &= ~DO_DEVICE_INITIALIZING;//ʹ�豸�����״̬�ӳ�ʼ��״̬��Ϊ�ǳ�ʼ��״̬��
	pDevice->Flags |= DO_BUFFERED_IO;

	// ���� IRP �ַ�����
	pDriver->MajorFunction[IRP_MJ_CREATE] = DisPatchCreate;

	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchControl;
	//pDriver->MajorFunction[IRP_MJ_READ] = ReadpatchControl;       //�� 

	return STATUS_SUCCESS;
}



