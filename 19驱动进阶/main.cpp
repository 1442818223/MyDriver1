#include <ntddk.h>


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
		//��ȡ����� IOCTL ������          //���ĸ������ĸ�
		ULONG CtlCode = pStack->Parameters.DeviceIoControl.IoControlCode; 
	

		//������,�������
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
			KdPrintEx((77, 0, "[db]:%s\r\n", pBuff));    //KdPrintEx�����ӡ�����ĺô��Զ�ʶ���Ƿ�Ϊdbgģʽ

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



// ��IRP_MJ_READ�ķ�ʽ��������
NTSTATUS ReadpatchControl(PDEVICE_OBJECT pDevice, PIRP pIrp)
{	
	
	// ��ȡ IRP ջ��Ϣ(������)
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

	// ��ȡ������                           //���ĸ������ĸ�
	LARGE_INTEGER ByteOffset = pStack->Parameters.Read.ByteOffset;
	ULONG Length = pStack->Parameters.Read.Length;

	// ��ȡ���뻺����ָ��
	int* pBuff = (int*)pIrp->AssociatedIrp.SystemBuffer;    //��Ӧ3������Ҫ���ĵ�ַ
	*pBuff = 100;

	// ������յ�������
	DbgPrintEx(77, 0, "[db]:%d\r\n", *pBuff);   //KdPrintEx�����ӡ�����ĺô��Զ�ʶ���Ƿ�Ϊdbgģʽ


	// ���÷�����Ϣ
	pIrp->IoStatus.Information = Length;  //��д������ȥ
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// ��� IRP ����
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

// ��IRP_MJ_WRITE�ķ�ʽ��������
NTSTATUS WritepatchControl(PDEVICE_OBJECT pDevice, PIRP pIrp)
{

	// ��ȡ IRP ջ��Ϣ(������)
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
   
	                                           //���ĸ������ĸ�
	LARGE_INTEGER ByteOffset = pStack->Parameters.Write.ByteOffset;
	ULONG Length = pStack->Parameters.Write.Length;

	// ��ȡ���뻺����ָ��
	PCHAR pBuff = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;    //��Ӧ3������Ҫ���ĵ�ַ

	DbgPrintEx(77, 0, "[db]:%s\r\n", pBuff);

	//RtlZeroMemory(pDevice->DeviceExtension, 200); 
	//RtlCopyMemory(pDevice->DeviceExtension, pBuff, Length); //ӳ�䵽����

	//// ������յ�������
	//DbgPrintEx(77, 0, "[db]:���յ�ַΪ:%p �õ�������Ϊ:%d\r\n", pBuff, (PCHAR)pDevice->DeviceExtension);


	// ���÷�����Ϣ
	pIrp->IoStatus.Information = Length;  //��д������ȥ
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
	// ������������
	IoCreateSymbolicLink(&uSymbliclinkname, &uDeviceName);

	// �����豸�����־
	pDevice->Flags &= ~DO_DEVICE_INITIALIZING;//ʹ�豸�����״̬�ӳ�ʼ��״̬��Ϊ�ǳ�ʼ��״̬��
	pDevice->Flags |= DO_BUFFERED_IO;

	// ���� IRP �ַ�����
	pDriver->MajorFunction[IRP_MJ_CREATE] = DisPatchCreate;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DisPatchDetach;

	//pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchControl;
	//pDriver->MajorFunction[IRP_MJ_READ] = ReadpatchControl;       //�� 

	pDriver->MajorFunction[IRP_MJ_WRITE] = WritepatchControl;       //д

	return STATUS_SUCCESS;
}



