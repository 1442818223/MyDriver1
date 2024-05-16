#include <ntddk.h>
/*
SynchronizationEvent ��Ҫ����ͬ���̣߳��� NotificationEvent ����Ҫ�����̼߳��ͨ�š�

֪ͨ�¼���Notification Event��:
��һ��֪ͨ�¼�������Ϊ����̬��signaled����
�����������״ֱ̬����ʽ������Ϊδ����̬��non-signaled����
��ʹ��һ�������ȴ��߳����¼�������������������¼������Իᱣ���伤��״̬��
ֱ����ʽ���� KeResetEvent ���á��������Ҫ�õ�����¼�����ͬ������ô������Ա��Ҫ�ֶ�����Ϊ������״̬��

ͬ���¼���Synchronization Event��:
����ͬ���¼�������������Ϊ����̬��
ϵͳ�����ͷŵ�һ���ȴ����¼����߳�ʱ�Զ���������Ϊδ����̬��
����ζ��ͬһ��ͬ���¼�����ͬʱ���Ѷ���ȴ����̣߳�ֻ�е��¼��ٴα�����Ϊ����̬ʱ����һ���ȴ����̲߳ű����ѡ�
*/
KEVENT event;

HANDLE t1, t2, t3;


VOID thread1()
{
    INT i = 0;
    do
    {
        DbgPrintEx(77, 0, "[db]thread1,%d��\n", i);
        i++;
    } while (i < 100);
                     //IO_NO_INCREMENT���ü��㲻����
    KeSetEvent(&event, IO_NO_INCREMENT, TRUE); //���ڽ�һ���¼���������Ϊ signaled ״̬��
    //TRUE�����ѵȴ����¼����̣߳� FALSE���򲻻��ѡ�


    //�������ͨ�����̵߳���󱻵��ã��԰�ȫ�رպ��˳��̡߳�
    PsTerminateSystemThread(0);
}

VOID thread2()
{
    DbgPrintEx(77, 0, "[db]�߳�2�ȴ���\n");

    //�ȴ�һ���¼���������Ϊ signaled ״̬
    KeWaitForSingleObject(&event,
        Executive,//�ȴ���ԭ��ͨ����һ��ö��ֵ������ Executive��WrExecutive �ȡ�
        KernelMode,//�ȴ���ģʽ��ͨ���� KernelMode �� UserMode��
        FALSE,//һ������ֵ��ָʾ�߳��Ƿ�����ڵȴ��ڼ���վ��������Ϊ TRUE�����߳̿��Ա��������ѣ����Ϊ FALSE���򲻿��ԡ�
        NULL);

    INT i = 0;

    do
    {
        DbgPrintEx(77, 0, "[db]thread2,%d��\n", i);
        i++;
    } while (i < 100);
   
    KeResetEvent(&event);//���������¼������״̬Ϊ�� signaled��
    // KeSetEvent(&event, 0, TRUE);
}

VOID thread3()
{
    DbgPrintEx(77, 0, "[db]�߳�3�ȴ���\n");

    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    INT i = 0;

    do
    {
        DbgPrintEx(77, 0, "[db]thread3,%d��\n", i);
        i++;
    } while (i < 100);
   // KeSetEvent(&event, 0, TRUE);
   
}

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "ж�سɹ�\n");
}
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    pDriver->DriverUnload = UnloadDriver;
    NTSTATUS status;
    KeInitializeEvent(&event,
        NotificationEvent,//SynchronizationEvent ����ͬ������ NotificationEvent ����֪ͨ��
        FALSE);//���Ϊ TRUE���¼���������Ϊ signaled ״̬�����Ϊ FALSE���¼���������Ϊ�� signaled ״̬��
    status = PsCreateSystemThread(&t1, THREAD_ALL_ACCESS, 0, NULL, NULL, (PKSTART_ROUTINE)thread1, NULL);

    status = PsCreateSystemThread(&t2, THREAD_ALL_ACCESS, 0, NULL, NULL, (PKSTART_ROUTINE)thread2, NULL);

    status = PsCreateSystemThread(&t3, THREAD_ALL_ACCESS, 0, NULL, NULL, (PKSTART_ROUTINE)thread3, NULL);

    return 0;

}