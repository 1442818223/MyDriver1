#include<stdio.h>
#include <windows.h>

HANDLE hEvents[5] = { 0 };
DWORD WINAPI workThread1(PVOID context)
{
	printf("workThread1--- 1\r\n");
	int ret = WaitForSingleObject(hEvents[0], -1);
	printf("workThread1-- -2\r\n");
	return 0;
}

DWORD WINAPI workThread2(PVOID context)
{
	printf("workThread2--- 1\r\n");
	int ret = WaitForSingleObject(hEvents[0], -1);
	printf("workThread2-- -2\r\n");
	return 0;
}
int main() {


	/*
	 HANDLE
     WINAPI
     CreateEventA(
	_In_opt_ LPSECURITY_ATTRIBUTES lpEventAttributes,
	_In_ BOOL bManualReset, //����һ�κ��Ƿ��ֶ����ź���Ϊ��
	_In_ BOOL bInitialState, //��ʼ����ʱ����û���ź�
	_In_opt_ LPCSTR lpName
	);
	
	*/
	for(int i = 0; i < 5; i++)
	{
		hEvents[i] = CreateEventA(NULL, TRUE, FALSE, NULL);  //����Ĳ���2 TRUE�Ƿ��ù�����Ч���˹ؼ�����
	}
	CreateThread(NULL, NULL, workThread1, NULL, NULL,NULL);
	CreateThread(NULL, NULL, workThread2, NULL, NULL,NULL);
			
	system("pause");
	SetEvent(hEvents[0]);    //ֻ������һ���ͺ������ź�,�����̶߳���ͬһ��

}

