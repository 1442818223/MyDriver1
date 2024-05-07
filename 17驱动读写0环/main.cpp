#include"interface.h"

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING path)
{
	DbgPrint("驱动已加载,路径:%wZ", path);

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

	DbgPrint("驱动已卸载");
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
			DbgPrint("创建符号链接失败");
			IoDeleteDevice(device);
		}
	}

	DbgPrint("驱动设备已创建");

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
		DbgPrint("设备已打开");
		break;
	}

	case IRP_MJ_CLOSE:
	{
		DbgPrint("设备已关闭");
		break;
	}

	case IRP_MJ_DEVICE_CONTROL:
	{
		data = (Data*)pirp->AssociatedIrp.SystemBuffer;//前端传过来的值 结构体  这里应该强制转换数据类型
		DbgPrint("PID:%d  地址:%x  大小:%d", data->pid, data->address, data->size);
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
		DbgPrint("获取进程对象失败");
		return FALSE;
	}

	PVOID GetData;
	__try
	{
		GetData = ExAllocatePool(PagedPool, data->size);
	}
	__except (1)
	{
		DbgPrint("内存分配失败");
		return FALSE;
	}

	KAPC_STATE stack = { 0 };
	KeStackAttachProcess(process, &stack);


	//挂靠怎么说呢  又不是读写物理内存  过腾讯aCE   做做梦就好了  驱动检测无法读写的
	__try
	{
		ProbeForRead((volatile void*)data->address, data->size, 1);
		RtlCopyMemory(GetData, (const void*)data->address, data->size);
	}
	__except (1)
	{
		DbgPrint("读取内存出错");
		bRet = FALSE;
	}

	ObDereferenceObject(process);
	KeUnstackDetachProcess(&stack);

	RtlCopyMemory(data->data, GetData, data->size);

	/*DbgPrint("进程ID:%d",data->pid);
	for (int i = 0; i < data->size; i++)
	{
		//data->data[i] = GetData[i];
		DbgPrint("地址:%x 数据:%x data:%x", data->address+i,GetData[i],data->data[i]);
	}
	DbgPrint("输出完毕");*/

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
		DbgPrint("获取进程对象失败");
		return FALSE;
	}

	//在进入进程地址空间之前先赋值
	PVOID GetData;
	__try
	{
		GetData = ExAllocatePool(PagedPool, data->size);
	}
	__except (1)
	{
		DbgPrint("内存分配失败");
		return FALSE;
	}


	GetData = data->data;  //结构体中 用户的 的  写入数值传给这个Data  让它去复制下面的赋值


	KAPC_STATE stack = { 0 };
	KeStackAttachProcess(process, &stack);

	PMDL mdl = IoAllocateMdl(PVOID(data->address), data->size, 0, 0, NULL);
	if (mdl == NULL)
	{
		DbgPrint("创建MDL失败");
		return FALSE;
	}

	MmBuildMdlForNonPagedPool(mdl);

	PVOID ChangeData = NULL;

	__try
	{
		ChangeData = MmMapLockedPages(mdl, KernelMode);  //拷贝内存页  
		RtlCopyMemory(ChangeData, GetData, data->size);//就是把后面的值给former的值
	}
	__except (1)
	{
		DbgPrint("内存映射失败,%d", sizeof(ChangeData));
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
