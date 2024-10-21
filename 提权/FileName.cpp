#include <windows.h>
#include <stdio.h>

// ���õ���Ȩ��
BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) {
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
		printf("LookupPrivilegeValue error: %lu\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		printf("AdjustTokenPrivileges error: %lu\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
		printf("The token does not have the specified privilege.\n");
		return FALSE;
	}

	return TRUE;
}

int main() {
	// �򿪵�ǰ���̵ķ�������
	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		printf("OpenProcessToken error: %lu\n", GetLastError());
		return 1;
	}

	// ���� SeDebugPrivilege
	if (!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {
		printf("Failed to enable SeDebugPrivilege.\n");
		CloseHandle(hToken);
		return 1;
	}

	// ���ڳ��Դ�ϵͳ����
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 35156); // �滻Ϊʵ�ʵĽ���ID winlogon.exe
	if (hProcess == NULL) {
		printf("Failed to open process. Error: %lu\n", GetLastError());
		system("pause");
		CloseHandle(hToken);
		return 1;
	}

	printf("Process handle: %p\n", hProcess);

	// �رշ������ƾ��
	CloseHandle(hToken);

	// ��ͣ���ȴ��û�����
	system("pause");

	// �����½��̵Ľṹ��
	STARTUPINFOA sInfo = { 0 };
	sInfo.cb = sizeof(STARTUPINFOA);
	PROCESS_INFORMATION pInfo = { 0 };

	// �����½���
	if (!CreateProcessA(
		"�޸�ʱ��.exe",           // ��ִ���ļ���·��
		NULL,              // �����в���
		NULL,              // ���̵İ�ȫ����
		NULL,              // �̵߳İ�ȫ����
		TRUE,              // �Ƿ�̳о��////////�˽���Ŀǰ��ӵ��winlogon.exe�ľ����
		CREATE_NEW_CONSOLE, // ����һ���µĿ���̨����
		NULL,              // �½��̵Ļ���
		NULL,              // ��ǰĿ¼
		&sInfo,            // ������Ϣ
		&pInfo             // ������Ϣ
	)) {
		printf("Failed to create process. Error: %lu\n", GetLastError());
		return 1;
	}

	// �ٴ���ͣ
	system("pause");

	return 0;
}
