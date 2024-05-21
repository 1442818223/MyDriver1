#include <ntifs.h>
#include <tdikrnl.h>
#include <tdi.h>
#include <windef.h>

#define HTONS(a) (((0xFF & (a)) << 8) + ((0xFF00 & (a)) >> 8))


PDEVICE_OBJECT pfilterdevobj = NULL;
PDEVICE_OBJECT pdodevobj = NULL;

typedef struct _NETWORK_ADDRESS {
    UCHAR address[4];
    unsigned short port;
} NETWORK_ADDRESS, * PNETWORK_ADDRESS;

NTSTATUS NotSupported(PDEVICE_OBJECT pdevice, PIRP pirp) {
 
    //此时网络驱动IRP先流向我们

    IoSkipCurrentIrpStackLocation(pirp);   //保证设备堆栈在此层无变化

    //一个网络驱动不止有网络设备,可能还有其他的文件设备等
    //让IRP返回 到我们给网络驱动 附加 并 生成 的设备对象头部
    return IoCallDriver(pdodevobj,//指向 IRP 目标设备对象的指针。   
        pirp);//指向要传递给下一个驱动程序的 IRP 的指针。
}

NTSTATUS MyDispatch(PDEVICE_OBJECT pdevice, PIRP pirp) {
    PIO_STACK_LOCATION pirpstack = NULL;
    NTSTATUS status = STATUS_SUCCESS;    


    if (pdevice == pfilterdevobj) {
        pirpstack = IoGetCurrentIrpStackLocation(pirp);
        if (pirpstack == NULL) {
            return STATUS_UNSUCCESSFUL;
        }

        if (pirpstack->MinorFunction == TDI_CONNECT) { //次要操作类型
            PTDI_REQUEST_KERNEL_CONNECT ptdiconnect = (PTDI_REQUEST_KERNEL_CONNECT)(&pirpstack->Parameters);
            PTA_ADDRESS ta_addr = ((PTRANSPORT_ADDRESS)ptdiconnect->RequestConnectionInformation->RemoteAddress)->Address;
            PTDI_ADDRESS_IP tdi_addr = (PTDI_ADDRESS_IP)(ta_addr->Address);
            DWORD address = tdi_addr->in_addr;
            unsigned short port = tdi_addr->sin_port;

            NETWORK_ADDRESS data = { 0 };
            data.address[0] = ((PUCHAR)&address)[0];
            data.address[1] = ((PUCHAR)&address)[1];
            data.address[2] = ((PUCHAR)&address)[2];
            data.address[3] = ((PUCHAR)&address)[3];

            port = HTONS(port);
            data.port = port;

            //设备与设备之间是上下册关系,上层流下来的设备stack到我们这里是可以看见的
            DbgPrintEx(77, 0,"connect %d-%d-%d-%d port:%d\n", data.address[0], data.address[1], data.address[2], data.address[3], data.port);
           
            IoSkipCurrentIrpStackLocation(pirp);
            return IoCallDriver(pdodevobj, pirp);
        }

    }
    IoSkipCurrentIrpStackLocation(pirp);
    return IoCallDriver(pdodevobj, pirp);
}
VOID CancelPendingIrp(PDEVICE_OBJECT DeviceObject) {
    PIRP Irp;
    PDRIVER_CANCEL cancelRoutine;
    KIRQL OldIrql;
    PLIST_ENTRY entry;

    IoAcquireCancelSpinLock(&OldIrql);

    for (entry = DeviceObject->DeviceQueue.DeviceListHead.Flink;
        entry != &DeviceObject->DeviceQueue.DeviceListHead;
        entry = entry->Flink)
    {
        // 取得 IRP。
        Irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.DeviceQueueEntry);

        // 取消 I/O 请求。
        cancelRoutine = Irp->CancelRoutine;
        Irp->CancelIrql = OldIrql;
        Irp->Cancel = TRUE;
        Irp->CancelRoutine = NULL;
        cancelRoutine(DeviceObject, Irp);

        IoReleaseCancelSpinLock(OldIrql);
        IoAcquireCancelSpinLock(&OldIrql);
    }

    IoReleaseCancelSpinLock(OldIrql);
}
VOID DrvUnload(PDRIVER_OBJECT pdriver) {
  
    // 在卸载驱动程序前取消所有还在队列中的 IRP。
   // CancelPendingIrp(pfilterdevobj);

        //解绑
        IoDetachDevice(pdodevobj); 
        //删除我们自己的设备
        IoDeleteDevice(pfilterdevobj);

    DbgPrintEx(77, 0, "卸载成功!\n");
    return;
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pdriver, PUNICODE_STRING reg_path) {
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    DbgPrintEx(77, 0, "加载成功!\n");

    UNICODE_STRING devicename;

    status = IoCreateDevice(pdriver, 0, NULL, FILE_DEVICE_NETWORK, FILE_DEVICE_SECURE_OPEN, FALSE, &pfilterdevobj);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(77, 0,"Error creating device: %x\n", status);
        return status;
    }

    for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        pdriver->MajorFunction[i] = NotSupported;
    }
    pdriver->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = MyDispatch;

    RtlInitUnicodeString(&devicename, L"\\Device\\Tcp");
   // 将过滤设备对象 pfilterdevobj 附加到目标设备对象 pdodevobj 上，从而使你的驱动程序可以拦截和处理传递到目标设备的请求。
    status = IoAttachDevice(pfilterdevobj,// 要附加到目标设备的设备对象。
        &devicename,//目标设备的名称字符串。
        &pdodevobj);//指向接收附加设备对象指针的指针。   *********我们的设备生成一个设备 这个设备绑定在指定的驱动设备之上********
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(77, 0,"Error attaching device: %x\n", status);
        IoDeleteDevice(pfilterdevobj);
        return status;
    }

    pdriver->DriverUnload = DrvUnload;
    return STATUS_SUCCESS;
}
