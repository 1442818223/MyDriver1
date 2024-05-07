#include <ntddk.h>
#include <ntstrsafe.h>

ULONG uDemo;
HANDLE hThread1;
HANDLE hThread2;
CLIENT_ID cid1;
CLIENT_ID cid2;

KSPIN_LOCK my_spin_lock = {0};


//普通自旋锁
//VOID thread1() {
//    int i = 0;
//    KIRQL oldIrql=0;
//    KeAcquireSpinLock(&my_spin_lock, &oldIrql);
//    do {
//        DbgPrintEx(77, 0, "[DB]thread1:%x\n", uDemo);
//        uDemo = 0x99;
//    } while (i++ < 9999);
//    KeReleaseSpinLock(&my_spin_lock, oldIrql);
//}
//VOID thread2() {
//    int i = 0;
//    KIRQL oldIrql;
//    KeAcquireSpinLock(&my_spin_lock, &oldIrql);
//    do {
//        DbgPrintEx(77, 0, "[DB]thread2:%x\n", uDemo);
//        uDemo = 0x88;
//    } while (i++ < 9999);
//    KeReleaseSpinLock(&my_spin_lock, oldIrql);
//}

//队列自旋锁
//...
VOID thread1() {
    int i = 0;
    KLOCK_QUEUE_HANDLE oldIrql ;
    KeAcquireInStackQueuedSpinLock(&my_spin_lock, &oldIrql);
    do {
        DbgPrintEx(77, 0, "[DB]thread1:%x\n", uDemo);
        uDemo = 0x99;
    } while (i++ < 9999);
    KeReleaseInStackQueuedSpinLock(&oldIrql);
}
VOID thread2() {
    int i = 0;
    KLOCK_QUEUE_HANDLE oldIrql;
    KeAcquireInStackQueuedSpinLock(&my_spin_lock, &oldIrql);
    do {
        DbgPrintEx(77, 0, "[DB]thread2:%x\n", uDemo);
        uDemo = 0x88;
    } while (i++ < 9999);
    KeReleaseInStackQueuedSpinLock(&oldIrql);
}

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "卸载成功\n");
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    KeInitializeSpinLock(&my_spin_lock);
  

    uDemo = 1;
    NTSTATUS Status = STATUS_SUCCESS;
    Status = PsCreateSystemThread(
        &hThread1,
        0,
        NULL,
        NtCurrentProcess(),
        &cid1,
        (PKSTART_ROUTINE)thread1,
        NULL);

    Status = PsCreateSystemThread(
        &hThread2,//新创建的线程句柄
        0,//创建的权限
        NULL,//线程的属性，一般设为NULL
        NtCurrentProcess(),//指定创建用户线程还是系统线程。如果为NULL，为系统进程
        &cid2, //CLIENT_ID 结构的指针，用于标识新创建线程的客户端ID。
        (PKSTART_ROUTINE)thread2,//新线程的运行地址
        NULL);//新线程接收的参数

    pDriver->DriverUnload = UnloadDriver;
    return 0;
}
