#include <ntifs.h>    //要将这个底层头文件放在第一位才行,不然会出现重定义现象
#include<ntddk.h>
//#include<ntstrsafe.h>
#include<wdm.h>

//PsGetProcessImageFileName函数是 Windows 驱动开发中的一个非公开（或称为不完全文档化）的函数，
// 它可用于获取给定进程的镜像文件名。该函数通常用于驱动程序中，用于在运行时获取进程的镜像文件名。获取进程的主模块名称
extern "C" {  //因为是cpp所以要加extern "C"
	NTKERNELAPI
		UCHAR*
		PsGetProcessImageFileName(
			__in PEPROCESS Process
		);
}



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
		//获取传输的 IOCTL 控制码          //是哪个就拿哪个
		ULONG CtlCode = pStack->Parameters.DeviceIoControl.IoControlCode;


		//拿输入,输出长度   
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
			KdPrintEx((77, 0, "[db]:%s\r\n", pBuff));    //KdPrintEx这个打印函数的好处自动识别是否为dbg模式

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



// 用IRP_MJ_READ的方式传递数据
NTSTATUS ReadpatchControl(PDEVICE_OBJECT pDevice, PIRP pIrp)
{

	// 获取 IRP 栈信息(上下文)
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

	// 获取控制码                           //是哪个就拿哪个
	LARGE_INTEGER ByteOffset = pStack->Parameters.Read.ByteOffset;
	ULONG Length = pStack->Parameters.Read.Length;

	// 获取输入缓冲区指针
	int* pBuff = (int*)pIrp->AssociatedIrp.SystemBuffer;    //对应3环请求要读的地址
	*pBuff = 100;

	// 输出接收到的数据
	DbgPrintEx(77, 0, "[db]:%d\r\n", *pBuff);   //KdPrintEx这个打印函数的好处自动识别是否为dbg模式


	// 设置返回信息
	pIrp->IoStatus.Information = Length;  //不写传不回去
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	// 完成 IRP 请求
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

// 用IRP_MJ_WRITE的方式传递数据
NTSTATUS WritepatchControl(PDEVICE_OBJECT pDevice, PIRP pIrp)
{

	// 获取 IRP 栈信息(上下文)
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

	//是哪个就拿哪个
	LARGE_INTEGER ByteOffset = pStack->Parameters.Write.ByteOffset;
	ULONG Length = pStack->Parameters.Write.Length;

	// 获取输入缓冲区指针
	PCHAR pBuff = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;    //对应3环请求要读的地址

	DbgPrintEx(77, 0, "[db]:%s\r\n", pBuff);

	//RtlZeroMemory(pDevice->DeviceExtension, 200); 
	//RtlCopyMemory(pDevice->DeviceExtension, pBuff, Length); //映射到本地

	//// 输出接收到的数据
	//DbgPrintEx(77, 0, "[db]:接收地址为:%p 拿到的数据为:%d\r\n", pBuff, (PCHAR)pDevice->DeviceExtension);


	// 设置返回信息
	pIrp->IoStatus.Information = Length;  //不写传不回去
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


VOID TimerWork(PVOID context) {
	DbgPrint("Irql =%d\n", KeGetCurrentIrql());
	DbgPrint("Processname-%s\n", PsGetProcessImageFileName(PsGetCurrentProcess()));
	return;
}
PVOID DpcRoutine(PVOID conetxt){
	DbgPrint("-- - Current Irql - %d-- - \n", KeGetCurrentIrql());
	return;
}
BOOLEAN workout = FALSE;
VOID WorkItemRoutine(PVOID context) {
	DbgPrint("Irgl =%d'n", KeGetCurrentIrql());
	DbgPrint("Processname-%s\n", PsGetProcessImageFileName(PsGetCurrentProcess()));
	LARGE_INTEGER sleeptime = {0};
	sleeptime.QuadPart = -10 * 1000 * 1000;
	while (1) {
		
		if (workout) {  //如果驱动被卸载就跳出循环,否则会蓝屏
			break;      
		}
		DbgPrint("线程队列的死循环\n");
		KeDelayExecutionThread(KernelMode,FALSE,&sleeptime); //线程休眠函数

	}

	DbgPrint("Processnane=%s\n", PsGetProcessImageFileName(PsGetCurrentProcess()));
	return;
}



// 卸载驱动函数
VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	workout = TRUE;

	LARGE_INTEGER sleeptime{ 0 };
	sleeptime.QuadPart = -10 * 1000 * 1000 * 2;
	KeDelayExecutionThread(KernelMode, FALSE, &sleeptime);     //设置2秒延时是为了防止ExInitializeWorkItem的回调
	                                                            //没有释放就UnloadDriver

	// 输出卸载成功信息
	DbgPrintEx(77, 0, "卸载成功\n");

	//IoStopTimer(pDriver->DeviceObject);  //结束定时器  
	
	// 删除符号链接
	if (pDriver->DeviceObject)
	{
		UNICODE_STRING uSymblicLinkname;
		RtlInitUnicodeString(&uSymblicLinkname, _SYB_NAME);
		IoDeleteSymbolicLink(&uSymblicLinkname);
		// 删除设备对象
		IoDeleteDevice(pDriver->DeviceObject);
	}
	return;
}

KTIMER kerneltime = { 0 };
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

//==========================================================================================================
	//一:
	//给设备设置定时器(本质上是一个IRP_MJ_DEVICE_CONTROL) ,大概一秒一次
	IoInitializeTimer(pDevice,(PIO_TIMER_ROUTINE) TimerWork, NULL); //给一个设备初始化一个计时器对象,大概一秒一次
	IoStartTimer(pDevice);


	//二: DPC定时器
	KDPC dpcobj = { 0 };
	KeInitializeTimer(&kerneltime);//初始化内核计时器对象
	//当驱动程序想要发送一个高优先级的请求来执行代码时，它会使用 KeInitializeDpc 初始化一个 DPC 并将其排队。
	KeInitializeDpc(&dpcobj, (PKDEFERRED_ROUTINE)DpcRoutine, NULL);//初始化的 DPC 对象
	LARGE_INTEGER 	dpctime = { 0 };
	dpctime.QuadPart = -10 * 1000 * 1000 * 2;
	KeSetTimer(&kerneltime, dpctime, &dpcobj); //给DCP设置一个定时器 ,也可以单纯等待不设置DPC ,设置一次,只生效一次
	//KeWaitForSingleObject(&kerneltime, Executive, KernelMode, FALSE, NULL);//设置kerneltime的等待会生效
	DbgPrint("Dpe time has be worked\n");
	
//==========================================================================================================

	//向系统线程池中添加工作项    
	WORK_QUEUE_ITEM workobj = { 0 };
	ExInitializeWorkItem(&workobj, WorkItemRoutine, NULL); //将工作项队列到系统工作队列中
	ExQueueWorkItem(&workobj, CriticalWorkQueue); //将工作项排队到系统工作队列中
	DbgPrint("WorkItem has be worked\n");


//==========================================================================================================
	// 设置 IRP 分发函数
	pDriver->MajorFunction[IRP_MJ_CREATE] = DisPatchCreate;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DisPatchDetach;

	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchControl;
	//pDriver->MajorFunction[IRP_MJ_READ] = ReadpatchControl;       //读 

	//pDriver->MajorFunction[IRP_MJ_WRITE] = WritepatchControl;       //写

	return STATUS_SUCCESS;
}



