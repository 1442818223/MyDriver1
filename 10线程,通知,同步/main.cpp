#include <ntddk.h>
/*
SynchronizationEvent 主要用于同步线程，而 NotificationEvent 则主要用于线程间的通信。

通知事件（Notification Event）:
当一个通知事件被设置为激发态（signaled）后，
它将保持这个状态直到显式被重置为未激发态（non-signaled）。
即使有一个或多个等待线程因事件被激发而解除阻塞，事件对象仍会保持其激发状态，
直到显式调用 KeResetEvent 重置。如果还需要用到这个事件进行同步，那么开发人员需要手动设置为不激发状态。

同步事件（Synchronization Event）:
对于同步事件，当它被设置为激发态后，
系统会在释放第一个等待该事件的线程时自动将其重置为未激发态。
这意味着同一个同步事件不能同时唤醒多个等待的线程，只有当事件再次被设置为激发态时，下一个等待的线程才被唤醒。
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
                     //IO_NO_INCREMENT引用计算不增加
    KeSetEvent(&event, IO_NO_INCREMENT, TRUE); //用于将一个事件对象设置为 signaled 状态。
    //TRUE，唤醒等待该事件的线程； FALSE，则不唤醒。


    //这个函数通常在线程的最后被调用，以安全关闭和退出线程。
    PsTerminateSystemThread(0);
}

VOID thread2()
{
    DbgPrintEx(77, 0, "[db]线程2等待中\n");

    //等待一个事件对象被设置为 signaled 状态
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