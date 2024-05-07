#include <ntddk.h>
#include <ntifs.h>

// �����豸����
#define _DEVICE_NAME L"\\device\\mydevice"
// ���������������
#define _SYB_NAME	 L"\\??\\sysmblicname"
// ���� IOCTL ������
#define CTL_TALK  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x9000, METHOD_BUFFERED, FILE_ANY_ACCESS)

// �����봦����
NTSTATUS DispatchControl(PDEVICE_OBJECT pDevice, PIRP pIrp)
{	// ��ȡ IRP ջ��Ϣ(������)
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

	if (pStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
		// ��ȡ���뻺����ָ��
		PVOID pBuff = pIrp->AssociatedIrp.SystemBuffer;
		// ��ȡ������                     //���ĸ������ĸ�
		ULONG CtlCode = pStack->Parameters.DeviceIoControl.IoControlCode;
		
		//�����볤��
		//int size = pStack->Parameters.DeviceIoControl.InputBufferLength;     
		int outputbufferlength = pStack->Parameters.DeviceIoControl.OutputBufferLength;     
		// ���ڼ�¼���ݳ���
		ULONG uLen = { 0 };

		// ��ȡ���ݳ���
		uLen = strlen((const char*)pBuff);

		// ���ݿ�������в�ͬ����
		switch (CtlCode)
		{
		case CTL_TALK:
		{
			// ������յ������ݳ���
			DbgPrintEx(77, 0, "����:%d", uLen);
			// ������յ�������
			DbgPrintEx(77, 0, "���յ�������Ϊ:%s", pBuff);

			//�������ݵ�3��
			int y = 500;
			memcpy(pIrp->AssociatedIrp.SystemBuffer, &y, 4);
			pIrp->IoStatus.Information = outputbufferlength; 


		}
		default:
			break;
		}
	}

	// ���÷�����Ϣ
	pIrp->IoStatus.Information = 0;
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

	// ���÷�����Ϣ
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// ��� IRP ����
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MyClose(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
	
	DbgPrintEx(77, 0, "Close�ɹ�\n");

	// ���÷�����Ϣ
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// ��� IRP ����
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MyCleaup(PDEVICE_OBJECT pDevice, PIRP pIrp)
{

	DbgPrintEx(77, 0, "Cleaup�ɹ�\n");

	// ���÷�����Ϣ
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// ��� IRP ����
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


// ж����������
VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	// ���ж�سɹ���Ϣ
	DbgPrintEx(77, 0, "ж�سɹ�\n");

	// ɾ����������
	if (pDriver->DeviceObject)
	{
		UNICODE_STRING uSymblicLinkname;
		RtlInitUnicodeString(&uSymblicLinkname, _SYB_NAME);
		IoDeleteSymbolicLink(&uSymblicLinkname);
		// ɾ���豸����
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
	                    //�������豸��չ�Ĵ�С��ͨ���� sizeof(DEVICE_EXTENSION)��
	// ������������
	IoCreateSymbolicLink(&uSymbliclinkname, &uDeviceName);

	// �����豸�����־
	pDevice->Flags &= ~DO_DEVICE_INITIALIZING;//ʹ�豸�����״̬�ӳ�ʼ��״̬��Ϊ�ǳ�ʼ��״̬��
	pDevice->Flags |= DO_BUFFERED_IO;
	  
	// ���� IRP �ַ�����
	pDriver->MajorFunction[IRP_MJ_CREATE] = DisPatchCreate;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchControl;
	//�ر�����������
	pDriver->MajorFunction[IRP_MJ_CLEANUP] = MyClose;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = MyCleaup;

	return STATUS_SUCCESS;
}
