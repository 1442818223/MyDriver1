#include<ntifs.h>
//#include <ntddk.h>
#include<windef.h>

// 定义设备名称
#define _DEVICE_NAME L"\\device\\mydevice"
// 定义符号链接名字
#define _SYB_NAME	 L"\\??\\sysmblicname"
// 定义 IOCTL 控制码
#define CTL_TALK  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x9000, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct DATA
{
	HANDLE pid;//要读写的进程ID
	unsigned __int64 address;//要读写的地址
	DWORD size;//读写长度
	PVOID data;//要读写的数据,  空指针 对应任何数据类型
}Data, * PDATA;



// 控制码处理函数
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
			
			//获取进程对象
			status = PsLookupProcessByProcessId(data->pid, &process);
			if (!NT_SUCCESS(status)) {
				DbgPrintEx(77,0,"Failed to get process object\n");
				return status;
			}

			//附加进程
			KeStackAttachProcess(process, &apcstate);  

			__try {
			     //MDL 是一种可以描述一组不连续物理内存页的结构，系统通过它来处理虚拟内存与物理内存之间的映射。
                //分配内存描述符列表 (MDL)    
				mdl = IoAllocateMdl((PVOID)data->address, data->size, FALSE, FALSE, NULL);
				if (!mdl) {
					status = STATUS_INSUFFICIENT_RESOURCES;
					__leave; // 立即退出 __try 块
				}

				//以确保内存被锁定并且不会被操作系统换出到磁盘
				MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);


				//这个宏在成功时返回一个虚拟地址，该地址可以在内核模式下使用，用以访问缓冲区的内容。
				// 在无法映射或锁定页面的时候，它会返回NULL。
				//// 获取系统地址
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

			//让内核对象引用数-1
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

// 用IRP_MJ_READ的方式传递数据
NTSTATUS ReadpatchControl(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
	// 获取 IRP 栈信息(上下文)
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

	// 获取控制码
	LARGE_INTEGER ByteOffset = pStack->Parameters.Read.ByteOffset;
	ULONG Length = pStack->Parameters.Read.Length;

	// 获取输入缓冲区指针
	int* pBuff = (int*)pIrp->AssociatedIrp.SystemBuffer;   // 对应3环请求要读的地址   
	// 输出接收到的数据
	DbgPrintEx(77, 0, "[db]:%d\r\n", *pBuff); //此时是0,可能是没用到就没映射

	*pBuff = 100;

	// 设置返回信息
	pIrp->IoStatus.Information = Length;     //返回长度必须设置,不设置就反不回去
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

	// 完成 IRP 请求
	IoCompleteRequest(pIrp, 0);

	return STATUS_SUCCESS;
}

// 关闭设备的操作派遣函数
NTSTATUS DisPatchDetach(PDEVICE_OBJECT pDevice, PIRP pIrp)
{
	// 输出创建成功信息
	DbgPrintEx(77, 0, " 关闭设备派遣通信成功\n");

	// 完成 IRP 请求
	IoCompleteRequest(pIrp, 0);

	return STATUS_SUCCESS;
}

// 卸载驱动函数
VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	// 输出卸载成功信息
	DbgPrintEx(77, 0, "卸载成功\n");

	// 删除符号链接
	UNICODE_STRING uSymblicLinkname;
	RtlInitUnicodeString(&uSymblicLinkname, _SYB_NAME);
	IoDeleteSymbolicLink(&uSymblicLinkname);

	// 删除设备对象
	if (pDriver->DeviceObject) {
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
	// 创建符号链接
	IoCreateSymbolicLink(&uSymbliclinkname, &uDeviceName);

	// 设置设备对象标志
	pDevice->Flags &= ~DO_DEVICE_INITIALIZING;//使设备对象的状态从初始化状态变为非初始化状态。
	pDevice->Flags |= DO_BUFFERED_IO;

	// 设置 IRP 分发函数
	pDriver->MajorFunction[IRP_MJ_CREATE] = DisPatchCreate;

	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchControl;
	//pDriver->MajorFunction[IRP_MJ_READ] = ReadpatchControl;       //读 

	return STATUS_SUCCESS;
}



