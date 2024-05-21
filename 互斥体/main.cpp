#include <windows.h>
#include <stdio.h>

HANDLE hMutex; // 互斥体句柄
int counter = 0; // 共享资源

DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	for (int i = 0; i < 5; i++) {
		// 等待互斥体
		DWORD waitResult = WaitForSingleObject(hMutex, INFINITE);         ////
		if (waitResult == WAIT_OBJECT_0) {   //WAIT_OBJECT_0表示指定的对象的状态已经变为有信号状态
			// 获得互斥体所有权
			printf("Thread %d acquired the mutex.\n", GetCurrentThreadId());

			// 访问共享资源
			int temp = counter;
			Sleep(1000); // 模拟对共享资源的处理时间
			counter = temp + 1;
			printf("Thread %d updated counter to %d.\n", GetCurrentThreadId(), counter);

			// 释放互斥体
			if (!ReleaseMutex(hMutex)) {                         ////
				printf("ReleaseMutex error: %d\n", GetLastError());
			}
			else {
				printf("Thread %d released the mutex.\n", GetCurrentThreadId());
			}
		}
		else {
			printf("WaitForSingleObject error: %d\n", GetLastError());
		}
		Sleep(1000); // 模拟其他操作
	}
	return 0;
}

int main() {
	// 创建一个未命名的互斥体
	hMutex = CreateMutex(NULL, FALSE, NULL);
	if (hMutex == NULL) {
		printf("CreateMutex error: %d\n", GetLastError());
		return 1;
	}

	// 创建两个线程
	HANDLE hThread1 = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
	HANDLE hThread2 = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);

	// 等待两个线程结束
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	// 关闭线程句柄
	CloseHandle(hThread1);
	CloseHandle(hThread2);

	// 关闭互斥体句柄
	CloseHandle(hMutex);

	printf("Final counter value: %d\n", counter);

	return 0;
}
