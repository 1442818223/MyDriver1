//#include <windows.h>
//#include <stdio.h>
//#include <psapi.h>
//
//#pragma comment(lib, "psapi.lib")
//
//int main() {
//	LPVOID drivers[1024]; // ���ڴ洢��������Ļ���ַ
//	DWORD cbNeeded;       // ʵ����Ҫ���ֽ���
//	int count;            // ������������
//
//	// ö�������Ѽ��ص��豸��������
//	if (EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded)) {
//		// ����������������
//		count = cbNeeded / sizeof(drivers[0]);
//		printf("Number of drivers loaded: %d\n\n", count);
//
//		for (int i = 0; i < count; i++) {
//			char driverName[MAX_PATH];
//
//			// ��ȡ����������ļ���
//			if (GetDeviceDriverBaseNameA(drivers[i], driverName, sizeof(driverName))) {
//				printf("Driver %d: %s\n", i + 1, driverName);
//			}
//			else {
//				printf("Driver %d: [Unknown]\n", i + 1);
//			}
//		}
//	}
//	else {
//		printf("Failed to enumerate device drivers. Error code: %lu\n", GetLastError());
//	}
//
//	return 0;
//}


#include <windows.h>
#include <stdio.h>
#include <tchar.h>

void ListDriverServices() {
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services"), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		printf("Failed to open registry key.\n");
		return;
	}

	TCHAR subKeyName[256];
	DWORD index = 0;
	DWORD nameSize = sizeof(subKeyName) / sizeof(TCHAR);

	while (RegEnumKeyEx(hKey, index, subKeyName, &nameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
		HKEY hSubKey;
		if (RegOpenKeyEx(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
			TCHAR imagePath[1024];
			DWORD pathSize = sizeof(imagePath);
			if (RegQueryValueEx(hSubKey, _T("ImagePath"), NULL, NULL, (LPBYTE)imagePath, &pathSize) == ERROR_SUCCESS) {
				if (_tcsstr(imagePath, _T("1.sys"))) { // ����Ŀ������
					_tprintf(_T("Service Name: %s\n"), subKeyName);
					_tprintf(_T("Image Path: %s\n"), imagePath);
				}
			}
			RegCloseKey(hSubKey);
		}
		index++;
		nameSize = sizeof(subKeyName) / sizeof(TCHAR);
	}
	RegCloseKey(hKey);
}

int main() {
	ListDriverServices();
	return 0;
}

