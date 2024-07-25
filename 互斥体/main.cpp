#include <windows.h>
#include <stdio.h>

HANDLE hMutex; // ��������
int counter = 0; // ������Դ

DWORD WINAPI ThreadProc(LPVOID lpParameter) {
	for (int i = 0; i < 5; i++) {
		// �ȴ�������
		DWORD waitResult = WaitForSingleObject(hMutex, INFINITE);         ////
		if (waitResult == WAIT_OBJECT_0) {   //WAIT_OBJECT_0��ʾָ���Ķ����״̬�Ѿ���Ϊ���ź�״̬
			// ��û���������Ȩ
			printf("Thread %d acquired the mutex.\n", GetCurrentThreadId());

			// ���ʹ�����Դ
			int temp = counter;
			Sleep(1000); // ģ��Թ�����Դ�Ĵ���ʱ��
			counter = temp + 1;
			printf("Thread %d updated counter to %d.\n", GetCurrentThreadId(), counter);

			// �ͷŻ�����
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
		Sleep(1000); // ģ����������
	}
	return 0;
}

int main() {
	// ����һ��δ�����Ļ�����
	hMutex = CreateMutex(NULL, FALSE, NULL);
	if (hMutex == NULL) {
		printf("CreateMutex error: %d\n", GetLastError());
		return 1;
	}

	// ���������߳�
	HANDLE hThread1 = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
	HANDLE hThread2 = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);

	// �ȴ������߳̽���
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	// �ر��߳̾��
	CloseHandle(hThread1);
	CloseHandle(hThread2);

	// �رջ�������
	CloseHandle(hMutex);

	printf("Final counter value: %d\n", counter);

	return 0;
}
