#include"interface.h"

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING path)
{
	DbgPrint("�����Ѽ���,·��:%wZ", path);

	driver->DriverUnload = DriverUnload;

	CreateDevice(driver);

	driver->MajorFunction[IRP_MJ_CREATE] = DriverIrpCtl;
	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverIrpCtl;
	driver->MajorFunction[IRP_MJ_CLOSE] = DriverIrpCtl;


	return STATUS_SUCCESS;
}

void DriverUnload(PDRIVER_OBJECT driver)
{

	if (driver->DeviceObject)
	{
		UNICODE_STRING SymbolName;
		RtlInitUnicodeString(&SymbolName, SYMBOLNAME);

		IoDeleteSymbolicLink(&SymbolName);
		IoDeleteDevice(driver->DeviceObject);
	}

	DbgPrint("������ж��");
}

NTSTATUS CreateDevice(PDRIVER_OBJECT driver)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT device = NULL;
	UNICODE_STRING DeviceName;

	RtlInitUnicodeString(&DeviceName, DEVICENAME);

	status = IoCreateDevice(
		driver,
		sizeof(driver->DriverExtension),
		&DeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&device
	);

	if (status == STATUS_SUCCESS)
	{
		UNICODE_STRING SymbolName;
		RtlInitUnicodeString(&SymbolName, SYMBOLNAME);

		status = IoCreateSymbolicLink(&SymbolName, &DeviceName);

		if (status != STATUS_SUCCESS)
		{
			DbgPrint("������������ʧ��");
			IoDeleteDevice(device);
		}
	}

	DbgPrint("�����豸�Ѵ���");

	return status;

}

NTSTATUS DriverIrpCtl(PDEVICE_OBJECT device, PIRP pirp)
{
	PIO_STACK_LOCATION stack;

	stack = IoGetCurrentIrpStackLocation(pirp);

	Data* data;

	switch (stack->MajorFunction)
	{

	case IRP_MJ_CREATE:
	{
		DbgPrint("�豸�Ѵ�");
		break;
	}

	case IRP_MJ_CLOSE:
	{
		DbgPrint("�豸�ѹر�");
		break;
	}

	case IRP_MJ_DEVICE_CONTROL:
	{
		data = (Data*)pirp->AssociatedIrp.SystemBuffer;//ǰ�˴�������ֵ �ṹ��  ����Ӧ��ǿ��ת����������
		DbgPrint("PID:%d  ��ַ:%x  ��С:%d", data->pid, data->address, data->size);
		switch (stack->Parameters.DeviceIoControl.IoControlCode)
		{

		case READCODE:
		{
			ReadMemory(data);
			break;
		}

		case WRITECODE:
		{
			WriteMemory(data);
			break;
		}

		}

		pirp->IoStatus.Information = sizeof(data);

		break;
	}

	}

	pirp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

BOOL ReadMemory(Data* data)
{
	if (data == NULL)
	{
		DbgPrintEx(77,0,"Invalid data pointer");
		return FALSE;
	}
	BOOL bRet = TRUE;
	PEPROCESS process = NULL;

	PsLookupProcessByProcessId((HANDLE)data->pid, &process);

	if (process == NULL)
	{
		DbgPrint("��ȡ���̶���ʧ��");
		return FALSE;
	}

	PVOID GetData;
	__try
	{
		GetData = ExAllocatePool(PagedPool, data->size);
	}
	__except (1)
	{
		DbgPrint("�ڴ����ʧ��");
		return FALSE;
	}

	KAPC_STATE stack = { 0 };
	KeStackAttachProcess(process, &stack);


	//�ҿ���ô˵��  �ֲ��Ƕ�д�����ڴ�  ����ѶaCE   �����ξͺ���  ��������޷���д��
	__try
	{
		ProbeForRead((volatile void*)data->address, data->size, 1);
		RtlCopyMemory(GetData, (const void*)data->address, data->size);
	}
	__except (1)
	{
		DbgPrint("��ȡ�ڴ����");
		bRet = FALSE;
	}

	ObDereferenceObject(process);
	KeUnstackDetachProcess(&stack);

	RtlCopyMemory(data->data, GetData, data->size);

	/*DbgPrint("����ID:%d",data->pid);
	for (int i = 0; i < data->size; i++)
	{
		//data->data[i] = GetData[i];
		DbgPrint("��ַ:%x ����:%x data:%x", data->address+i,GetData[i],data->data[i]);
	}
	DbgPrint("������");*/

	ExFreePool(GetData);
	return bRet;
}

BOOL WriteMemory(Data* data)
{
	if (data == NULL)
	{
		DbgPrintEx(77,0,"Invalid data pointer");
		return FALSE;
	}
	BOOL bRet = TRUE;
	PEPROCESS process = NULL;

	PsLookupProcessByProcessId(HANDLE(data->pid), &process);
	if (process == NULL)
	{
		DbgPrint("��ȡ���̶���ʧ��");
		return FALSE;
	}

	//�ڽ�����̵�ַ�ռ�֮ǰ�ȸ�ֵ
	PVOID GetData;
	__try
	{
		GetData = ExAllocatePool(PagedPool, data->size);
	}
	__except (1)
	{
		DbgPrint("�ڴ����ʧ��");
		return FALSE;
	}


	GetData = data->data;  //�ṹ���� �û��� ��  д����ֵ�������Data  ����ȥ��������ĸ�ֵ


	KAPC_STATE stack = { 0 };
	KeStackAttachProcess(process, &stack);

	PMDL mdl = IoAllocateMdl(PVOID(data->address), data->size, 0, 0, NULL);
	if (mdl == NULL)
	{
		DbgPrint("����MDLʧ��");
		return FALSE;
	}

	MmBuildMdlForNonPagedPool(mdl);

	PVOID ChangeData = NULL;

	__try
	{
		ChangeData = MmMapLockedPages(mdl, KernelMode);  //�����ڴ�ҳ  
		RtlCopyMemory(ChangeData, GetData, data->size);//���ǰѺ����ֵ��former��ֵ
	}
	__except (1)
	{
		DbgPrint("�ڴ�ӳ��ʧ��,%d", sizeof(ChangeData));
		bRet = FALSE;
		goto END;
	}

END:
	IoFreeMdl(mdl);
	ExFreePool(GetData);
	KeUnstackDetachProcess(&stack);
	ObDereferenceObject(process);

	return bRet;
}
