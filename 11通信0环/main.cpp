#include <ntddk.h>
#include <ntifs.h>

// 定义设备名称
#define _DEVICE_NAME L"\\device\\mydevice"
// 定义符号链接名字
#define _SYB_NAME	 L"\\??\\sysmblicname"
// 定义 IOCTL 控制码
#define CTL_TALK  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x9000, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 控制码处理函数
NTSTATUS DispatchControl(PDEVICE_OBJECT pDevice, PIRP pIrp)
{	// 获取 IRP 栈信息(上下文)
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

	if (pStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
		// 获取输入缓冲区指针
		PVOID pBuff = pIrp->AssociatedIrp.SystemBuffer;
		// 获取控制码                     //是哪个就拿哪个
		ULONG CtlCode = pStack->Parameters.DeviceIoControl.IoControlCode;
		
		//拿输入长度
		//int size = pStack->Parameters.DeviceIoControl.InputBufferLength;     
		int outputbufferlength = pStack->Parameters.DeviceIoControl.OutputBufferLength;     
		// 用于记录数据长度
		ULONG uLen = { 0 };

		// 获取数据长度
		uLen = strlen((const char*)pBuff);

		// 根据控制码进行不同操作
		switch (CtlCode)
		{
		case CTL_TALK:
		{
			// 输出接收到的数据长度
			DbgPrintEx(77, 0, "长度:%d", uLen);
			// 输出接收到的数据
			DbgPrintEx(77, 0, "接收到的数据为:%s", pBuff);

			//发送数据到3环
			int y = 500;
			memcpy(pIrp->AssociatedIrp.SystemBuffer, &y, 4);
			pIrp->IoStatus.Information = outputbufferlength; 


		}
		default:
			break;
		}
	}

	// 设置返回信息
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// 完成 IRP 请求
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

// 创建派遣函数
NTSTATUS DisPatchCreate(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
	// 输出创建成功信息
	DbgPrintEx(77, 0, "创建派遣通信成功\n");

	// 设置返回信息
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// 完成 IRP 请求
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MyClose(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
	
	DbgPrintEx(77, 0, "Close成功\n");

	// 设置返回信息
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// 完成 IRP 请求
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MyCleaup(PDEVICE_OBJECT pDevice, PIRP pIrp)
{

	DbgPrintEx(77, 0, "Cleaup成功\n");

	// 设置返回信息
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// 完成 IRP 请求
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


// 卸载驱动函数
VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	// 输出卸载成功信息
	DbgPrintEx(77, 0, "卸载成功\n");

	// 删除符号链接
	if (pDriver->DeviceObject)
	{
		UNICODE_STRING uSymblicLinkname;
		RtlInitUnicodeString(&uSymblicLinkname, _SYB_NAME);
		IoDeleteSymbolicLink(&uSymblicLinkname);
		// 删除设备对象
		IoDeleteDevice(pDriver->DeviceObject);
	}
}


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{
	// 输出加载成功信息
	DbgPrintEx(77, 0, "加载成功\n");

	// 设置卸载函数
	pDriver->DriverUnload = UnloadDriver;

	UNICODE_STRING uDeviceName;
	UNICODE_STRING uSymbliclinkname;
	PDEVICE_OBJECT pDevice;

	// 初始化设备名称和符号链接名称
	RtlInitUnicodeString(&uDeviceName, _DEVICE_NAME);
	RtlInitUnicodeString(&uSymbliclinkname, _SYB_NAME);

	// 创建设备对象                                                表示该设备支持安全打开
	IoCreateDevice(pDriver, 0, &uDeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDevice);
	                    //参数二设备扩展的大小，通常是 sizeof(DEVICE_EXTENSION)。
	// 创建符号链接
	IoCreateSymbolicLink(&uSymbliclinkname, &uDeviceName);

	// 设置设备对象标志
	pDevice->Flags &= ~DO_DEVICE_INITIALIZING;//使设备对象的状态从初始化状态变为非初始化状态。
	pDevice->Flags |= DO_BUFFERED_IO;
	  
	// 设置 IRP 分发函数
	pDriver->MajorFunction[IRP_MJ_CREATE] = DisPatchCreate;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchControl;
	//关闭有两个函数
	pDriver->MajorFunction[IRP_MJ_CLEANUP] = MyClose;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = MyCleaup;

	return STATUS_SUCCESS;
}
