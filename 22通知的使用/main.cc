#include <ntifs.h>    //Ҫ������ײ�ͷ�ļ����ڵ�һλ����,��Ȼ������ض�������
#include<ntddk.h>
//#include<ntstrsafe.h>
#include<wdm.h>

//PsGetProcessImageFileName������ Windows ���������е�һ���ǹ��������Ϊ����ȫ�ĵ������ĺ�����
// �������ڻ�ȡ�������̵ľ����ļ������ú���ͨ���������������У�����������ʱ��ȡ���̵ľ����ļ�������ȡ���̵���ģ������
extern "C" {  //��Ϊ��cpp����Ҫ��extern "C"
	NTKERNELAPI
		UCHAR*
		PsGetProcessImageFileName(
			__in PEPROCESS Process
		);
}
//���̴����ص�
VOID ProcessNotifyFun(HANDLE pid, HANDLE pid2, BOOLEAN	bcareaf) {  //PsSetCreateProcessNotifyRoutine�ص���ʽ�������
	//����һ ������pid  ������ ��������ɾ����pid ������ �Ƿ񴴽� true Ϊ����ʱ�ص�

	if (bcareaf) {

		//PsGetCurrentProcess(); //��ȡ��ǰ���̵Ľ��̽ṹ��
		PEPROCESS tempep = NULL;

		PsLookupProcessByProcessId(pid2, &tempep); //��ѯ�ض�����ID�Ľ��̽ṹ��
		if (!tempep)
		{
			return;
		}

		PUCHAR processname = PsGetProcessImageFileName(tempep);
		DbgPrintEx(77, 0, "������:%s\n", processname);
		// ��С��������ü���
		ObDereferenceObject(tempep);
	}
	return;
}

//ģ����ػص�
VOID MyLoadImageNotify(PUNICODE_STRING FullImageName, //DLLģ����
	HANDLE ProcessId, //���ط���pid
	PIMAGE_INFO ImageInfo) {
	                                        
	PEPROCESS tempep = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	UCHAR* imagename = NULL;

	// ͨ������ ID ��ȡ���̶���
	status = PsLookupProcessByProcessId(ProcessId, &tempep);
	if (NT_SUCCESS(status)) {
		// ��ȡ���̵ľ����ļ���
		imagename = PsGetProcessImageFileName(tempep);

		// ��ӡ���������Ϣ
		DbgPrintEx(77, 0, "���ط��ľ����ļ���:<%s> DLLģ����: <%wZ> ������ģ���ַ: (%p) ��С: <%llx>\n",
			imagename, FullImageName, ImageInfo->ImageBase, ImageInfo->ImageSize);

		// �ͷŽ��̶���
		ObDereferenceObject(tempep);
	}
	else {
		// ��ӡ������Ϣ
		DbgPrintEx(77, 0, "Failed to lookup process by PID: %x\n", ProcessId);
	}

	return;
}

//�̻߳ص�
VOID MythreadNotify( HANDLE ProcessId,  //�ṩ�����̴߳��������ٵĽ��̵�ID
	 HANDLE ThreadId, //�ṩ����֪ͨ���̵߳�ID
	 BOOLEAN Create	) //TRUE�����ʾ�̱߳����������Ϊ FALSE�����ʾ�̱߳����١�
{
	if (Create) {
		PEPROCESS tempep = NULL;
		NTSTATUS status = STATUS_SUCCESS;
		UCHAR* imagename = NULL;

		// ͨ������ ID ��ȡ���̶���
		status = PsLookupProcessByProcessId(ProcessId, &tempep);
		if (NT_SUCCESS(status)) {
			// ��ȡ���̵ľ����ļ���
			imagename = PsGetProcessImageFileName(tempep);

			// ��ӡ���������Ϣ
			DbgPrint("<%s> create thread <%d>\n", imagename, ThreadId);

			// �ͷŽ��̶���
			ObDereferenceObject(tempep);
		}
		else {
			// ��ӡ������Ϣ
			DbgPrint("Failed to lookup process by PID: %p\n", ProcessId);
		}
	}
	
	return;
	

}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	PsSetCreateProcessNotifyRoutine(ProcessNotifyFun, TRUE); //ɾ�����̻ص�����

	PsRemoveLoadImageNotifyRoutine(MyLoadImageNotify);   //ж��ģ����ػص�

	PsRemoveCreateThreadNotifyRoutine(MythreadNotify);   //ж���̻߳ص�����

	DbgPrintEx(77, 0, "ж�سɹ�\n");

	return;
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{

	DbgPrintEx(77, 0, "���سɹ�\n");

	pDriver->DriverUnload = UnloadDriver;
	
	NTSTATUS 	status = STATUS_SUCCESS;
	DbgBreakPoint();
	PsSetCreateProcessNotifyRoutine(ProcessNotifyFun, FALSE);
	DbgBreakPoint();
	
	status = PsSetLoadImageNotifyRoutine(MyLoadImageNotify);  //ÿ��ģ���м��ص�ʱ�򴥷�֪ͨ

	status = PsSetCreateThreadNotifyRoutine(MythreadNotify); //�߳�ע��ص�
	return 0;

}