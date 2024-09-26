#include <windows.h>
#include <stdio.h>

// 启用调试权限
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
	// 打开当前进程的访问令牌
	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		printf("OpenProcessToken error: %lu\n", GetLastError());
		return 1;
	}

	// 启用 SeDebugPrivilege
	if (!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {
		printf("Failed to enable SeDebugPrivilege.\n");
		CloseHandle(hToken);
		return 1;
	}

	// 现在尝试打开系统进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 35156); // 替换为实际的进程ID winlogon.exe
	if (hProcess == NULL) {
		printf("Failed to open process. Error: %lu\n", GetLastError());
		system("pause");
		CloseHandle(hToken);
		return 1;
	}

	printf("Process handle: %p\n", hProcess);

	// 关闭访问令牌句柄
	CloseHandle(hToken);

	// 暂停，等待用户操作
	system("pause");

	// 启动新进程的结构体
	STARTUPINFOA sInfo = { 0 };
	sInfo.cb = sizeof(STARTUPINFOA);
	PROCESS_INFORMATION pInfo = { 0 };

	// 创建新进程
	if (!CreateProcessA(
		"修改时间.exe",           // 可执行文件的路径
		NULL,              // 命令行参数
		NULL,              // 进程的安全属性
		NULL,              // 线程的安全属性
		TRUE,              // 是否继承句柄////////此进程目前是拥有winlogon.exe的句柄的
		CREATE_NEW_CONSOLE, // 创建一个新的控制台窗口
		NULL,              // 新进程的环境
		NULL,              // 当前目录
		&sInfo,            // 启动信息
		&pInfo             // 进程信息
	)) {
		printf("Failed to create process. Error: %lu\n", GetLastError());
		return 1;
	}

	// 再次暂停
	system("pause");

	return 0;
}
