#include<ntifs.h>
//#include <ntddk.h>
#include<windef.h>

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
}Data, * PDATA;



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

	
		
		switch (CtlCode) {
		case CTL_TALK: {
			PDATA data = (PDATA)pBuff;
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "inputbufferlength: %d,outputbufferlength: %d\n", inputbufferlength,outputbufferlength);
			
			//��ȡ���̶���
			status = PsLookupProcessByProcessId(data->pid, &process);
			if (!NT_SUCCESS(status)) {
				DbgPrintEx(77,0,"Failed to get process object\n");
				return status;
			}

			//���ӽ���
			KeStackAttachProcess(process, &apcstate);  

			__try {
			     //MDL ��һ�ֿ�������һ�鲻���������ڴ�ҳ�Ľṹ��ϵͳͨ���������������ڴ��������ڴ�֮���ӳ�䡣
                //�����ڴ��������б� (MDL)    
				mdl = IoAllocateMdl((PVOID)data->address, data->size, FALSE, FALSE, NULL);
				if (!mdl) {
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

				

				RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, mappedAddr, data->size);
				pIrp->IoStatus.Information = data->size;

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



