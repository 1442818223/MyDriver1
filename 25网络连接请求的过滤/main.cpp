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
 
    //��ʱ��������IRP����������

    IoSkipCurrentIrpStackLocation(pirp);   //��֤�豸��ջ�ڴ˲��ޱ仯

    //һ������������ֹ�������豸,���ܻ����������ļ��豸��
    //��IRP���� �����Ǹ��������� ���� �� ���� ���豸����ͷ��
    return IoCallDriver(pdodevobj,//ָ�� IRP Ŀ���豸�����ָ�롣   
        pirp);//ָ��Ҫ���ݸ���һ����������� IRP ��ָ�롣
}

NTSTATUS MyDispatch(PDEVICE_OBJECT pdevice, PIRP pirp) {
    PIO_STACK_LOCATION pirpstack = NULL;
    NTSTATUS status = STATUS_SUCCESS;    


    if (pdevice == pfilterdevobj) {
        pirpstack = IoGetCurrentIrpStackLocation(pirp);
        if (pirpstack == NULL) {
            return STATUS_UNSUCCESSFUL;
        }

        if (pirpstack->MinorFunction == TDI_CONNECT) { //��Ҫ��������
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

            //�豸���豸֮�������²��ϵ,�ϲ����������豸stack�����������ǿ��Կ�����
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
        // ȡ�� IRP��
        Irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.DeviceQueueEntry);

        // ȡ�� I/O ����
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
  
    // ��ж����������ǰȡ�����л��ڶ����е� IRP��
   // CancelPendingIrp(pfilterdevobj);

        //���
        IoDetachDevice(pdodevobj); 
        //ɾ�������Լ����豸
        IoDeleteDevice(pfilterdevobj);

    DbgPrintEx(77, 0, "ж�سɹ�!\n");
    return;
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pdriver, PUNICODE_STRING reg_path) {
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    DbgPrintEx(77, 0, "���سɹ�!\n");

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
   // �������豸���� pfilterdevobj ���ӵ�Ŀ���豸���� pdodevobj �ϣ��Ӷ�ʹ�����������������غʹ����ݵ�Ŀ���豸������
    status = IoAttachDevice(pfilterdevobj,// Ҫ���ӵ�Ŀ���豸���豸����
        &devicename,//Ŀ���豸�������ַ�����
        &pdodevobj);//ָ����ո����豸����ָ���ָ�롣   *********���ǵ��豸����һ���豸 ����豸����ָ���������豸֮��********
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(77, 0,"Error attaching device: %x\n", status);
        IoDeleteDevice(pfilterdevobj);
        return status;
    }

    pdriver->DriverUnload = DrvUnload;
    return STATUS_SUCCESS;
}
