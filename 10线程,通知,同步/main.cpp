#include <ntddk.h>
/*
SynchronizationEvent 主要用于同步线程，而 NotificationEvent 则主要用于线程间的通信。
*/
KEVENT event;

HANDLE t1, t2, t3;


VOID thread1()
{
    INT i = 0;
    do
    {
        DbgPrintEx(77, 0, "[db]thread1,%d次\n", i);
        i++;
    } while (i < 100);

    KeSetEvent(&event, 0, TRUE); //用于将一个事件对象设置为 signaled 状态。
    //TRUE，唤醒等待该事件的线程； FALSE，则不唤醒。
}

VOID thread2()
{
    DbgPrintEx(77, 0, "[db]线程2等待中\n");

    KeWaitForSingleObject(&event,
        Executive,//等待的原因，通常是一个枚举值，例如 Executive、WrExecutive 等。
        KernelMode,//等待的模式，通常是 KernelMode 或 UserMode。
        FALSE,//一个布尔值，指示线程是否可以在等待期间接收警报。如果为 TRUE，则线程可以被警报唤醒；如果为 FALSE，则不可以。
        NULL);

    INT i = 0;

    do
    {
        DbgPrintEx(77, 0, "[db]thread2,%d次\n", i);
        i++;
    } while (i < 100);
   
    KeResetEvent(&event);//用于重置事件对象的状态为非 signaled。
    // KeSetEvent(&event, 0, TRUE);
}

VOID thread3()
{
    DbgPrintEx(77, 0, "[db]线程3等待中\n");

    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    INT i = 0;

    do
    {
        DbgPrintEx(77, 0, "[db]thread3,%d次\n", i);
        i++;
    } while (i < 100);
   // KeSetEvent(&event, 0, TRUE);
   
}

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "卸载成功\n");
}
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    pDriver->DriverUnload = UnloadDriver;
    NTSTATUS status;
    KeInitializeEvent(&event,
        NotificationEvent,//SynchronizationEvent 用于同步，而 NotificationEvent 用于通知。
        FALSE);//如果为 TRUE，事件将被设置为 signaled 状态；如果为 FALSE，事件将被设置为非 signaled 状态。
    status = PsCreateSystemThread(&t1, THREAD_ALL_ACCESS, 0, NULL, NULL, (PKSTART_ROUTINE)thread1, NULL);

    status = PsCreateSystemThread(&t2, THREAD_ALL_ACCESS, 0, NULL, NULL, (PKSTART_ROUTINE)thread2, NULL);

    status = PsCreateSystemThread(&t3, THREAD_ALL_ACCESS, 0, NULL, NULL, (PKSTART_ROUTINE)thread3, NULL);

    return 0;

}