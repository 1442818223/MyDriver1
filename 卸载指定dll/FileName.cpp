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
		MODULEENTRY32W me32; // ʹ�ÿ��ַ��汾
		me32.dwSize = sizeof(MODULEENTRY32W);
		if (Module32FirstW(hSnapshot, &me32)) { // ʹ�ÿ��ַ��汾
			do {
				if (wcscmp(me32.szModule, dllName) == 0) { // ʹ��wcscmp
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
			} while (Module32NextW(hSnapshot, &me32)); // ʹ�ÿ��ַ��汾
		}
		CloseHandle(hSnapshot);
	}

	CloseHandle(hProcess);
}

int main() {
	const wchar_t* dllName = L"Dll1.dll"; // Ҫж�ص� DLL ��

	HWND h = FindWindow(NULL, L"ֲ���ս��ʬ���İ�");//��ȡ���ھ��
	DWORD processid;
	GetWindowThreadProcessId(h, &processid);//��ȡ����id


	DWORD pid = processid;
	if (pid) {
		UnloadDLL(pid, dllName);
	}
	else {
		std::wcout << L"Process not found!" << std::endl; // ʹ�ÿ��ַ����
	}

	return 0;
}
