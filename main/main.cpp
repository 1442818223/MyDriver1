#include <windows.h>
#include <stdio.h>

int main() {
	/*
	HANDLE CreateSemaphoreA(
	LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,// Ĭ�ϰ�ȫ����
	LONG lInitialCount,// ��ʼ���� (�ź�������ĳ�ʼ����ֵ����ֵ������ڻ�����㣬����С�ڻ���� lMaximumCount�����ź����ļ���ֵ������ʱ����״̬Ϊ���źţ��������ֵΪ��ʱ����״̬Ϊ���źš�)
	LONG lMaximumCount,// ������
	LPCSTR lpName// �����ź���
    );
	*/

	
	HANDLE hSemaphoreA = CreateSemaphoreA(NULL, 2, 10, NULL);     //������������2֮�����Ĳ���ÿ�ζ������ӻ��߼���2   0��1����1
	if (hSemaphoreA == NULL) {
		printf("CreateSemaphoreA error: %d\n", GetLastError());
		return 1;
	}


	/*
  BOOL ReleaseSemaphore(
  HANDLE hSemaphore,  // �ź������
  LONG lReleaseCount, // ���Ӽ���������
  LPLONG lpPreviousCount // ָ����� ��ǰ ����ֵ�ı�����ָ�루����Ϊ NULL��  (�ñ��������ź�����ǰһ������ֵ������˲���Ϊ NULL���򲻷���ǰһ������ֵ��)
);
	*/
	LONG pre = 0;
	LONG prel = 0;

	// �ͷ��ź��������Ӽ���
	if (!ReleaseSemaphore(hSemaphoreA, 2, &pre)) {         //������ֻ��һ��������,����2����CreateSemaphoreA�Ĳ�����
		printf("ReleaseSemaphore error: %d\n", GetLastError());
		CloseHandle(hSemaphoreA);
		return 1;
	}

	if (!ReleaseSemaphore(hSemaphoreA, 1, &prel)) {       
		printf("ReleaseSemaphore error: %d\n", GetLastError());
		CloseHandle(hSemaphoreA);
		return 1;
	}

	// ��ӡ���
	printf("Semaphore Handle: %p, Previous Count 1: %ld, Previous Count 2: %ld\n",
		hSemaphoreA, pre, prel);

	// �ر��ź������
	CloseHandle(hSemaphoreA);
	


	return 0;
}
