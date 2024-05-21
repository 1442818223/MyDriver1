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
	_In_ BOOL bManualReset, //用完一次后是否手动将信号置为有
	_In_ BOOL bInitialState, //初始化的时候有没有信号
	_In_opt_ LPCSTR lpName
	);
	
	*/
	for(int i = 0; i < 5; i++)
	{
		hEvents[i] = CreateEventA(NULL, TRUE, FALSE, NULL);  //这里的参数2 TRUE是否用过还有效起到了关键作用
	}
	CreateThread(NULL, NULL, workThread1, NULL, NULL,NULL);
	CreateThread(NULL, NULL, workThread2, NULL, NULL,NULL);
			
	system("pause");
	SetEvent(hEvents[0]);    //只设置了一个型号量有信号,两个线程都用同一个

}

