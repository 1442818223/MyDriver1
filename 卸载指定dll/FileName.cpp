#include <windows.h>
#include <tlhelp32.h>
#include <iostream>


void UnloadDLL(DWORD pid, const wchar_t* dllName) {
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess) {
		std::cerr << "OpenProcess failed: " << GetLastError() << std::endl;
		return;
	}

	HMODULE hModule;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		MODULEENTRY32W me32; // 使用宽字符版本
		me32.dwSize = sizeof(MODULEENTRY32W);
		if (Module32FirstW(hSnapshot, &me32)) { // 使用宽字符版本
			do {
				if (wcscmp(me32.szModule, dllName) == 0) { // 使用wcscmp
					hModule = me32.hModule;
					HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
						(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "FreeLibrary"),
						hModule, 0, NULL);
					if (hThread) {
						WaitForSingleObject(hThread, INFINITE);
						CloseHandle(hThread);
					}
					break;
				}
			} while (Module32NextW(hSnapshot, &me32)); // 使用宽字符版本
		}
		CloseHandle(hSnapshot);
	}

	CloseHandle(hProcess);
}

int main() {
	const wchar_t* dllName = L"Dll1.dll"; // 要卸载的 DLL 名

	HWND h = FindWindow(NULL, L"植物大战僵尸中文版");//获取窗口句柄
	DWORD processid;
	GetWindowThreadProcessId(h, &processid);//获取进程id


	DWORD pid = processid;
	if (pid) {
		UnloadDLL(pid, dllName);
	}
	else {
		std::wcout << L"Process not found!" << std::endl; // 使用宽字符输出
	}

	return 0;
}
