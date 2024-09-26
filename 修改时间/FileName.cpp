#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>

void getCurrentSystemTime(SYSTEMTIME* st) {
	GetSystemTime(st);
}

void setSystemTime(int year, int month, int day, int hour, int minute, int second) {
	SYSTEMTIME st;
	st.wYear = year;
	st.wMonth = month;
	st.wDay = day;
	st.wHour = hour;
	st.wMinute = minute;
	st.wSecond = second;
	st.wMilliseconds = 0;

	SetSystemTime(&st);
}

int readTimeFromFile(const wchar_t* filename, int* year, int* month, int* day, int* hour, int* minute, int* second) {
	FILE* file = _wfopen(filename, L"r");
	if (file == NULL) {
		wprintf(L"Failed to open file: %ls\n", filename);
		return -1;
	}

	// 读取并解析时间
	if (fwscanf(file, L"%d-%d-%d %d:%d:%d", year, month, day, hour, minute, second) != 6) {
		wprintf(L"Failed to read time from file.\n");
		fclose(file);
		return -1;
	}

	fclose(file);
	return 0;
}

int findInfFile(wchar_t* infFileName) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile(L"*.inf", &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		wprintf(L"No .inf files found.\n");
		return -1;
	}

	// 找到第一个.inf文件
	wcscpy(infFileName, findFileData.cFileName);
	FindClose(hFind);
	return 0;
}

int main() {





	SYSTEMTIME currentTime;
	int year, month, day, hour, minute, second;
	wchar_t infFileName[MAX_PATH];

	// 获取当前系统时间
	getCurrentSystemTime(&currentTime);
	wprintf(L"Current Date: %02d/%02d/%04d\n", currentTime.wDay, currentTime.wMonth, currentTime.wYear);
	wprintf(L"Current Time: %02d:%02d:%02d\n", currentTime.wHour, currentTime.wMinute, currentTime.wSecond);

	// 查找.inf文件
	if (findInfFile(infFileName) != 0) {
		return -1;  // 找不到文件，退出
	}

	wprintf(L"Found INF File: %ls\n", infFileName);

	// 从找到的文件读取时间
	if (readTimeFromFile(infFileName, &year, &month, &day, &hour, &minute, &second) != 0) {
		return -1;  // 读取失败，退出
	}

	// 设置特定时间
	setSystemTime(year, month, day, hour, minute, second);
	wprintf(L"Set Date: %04d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);

	// 执行1.bat
	system("1.bat");

	// 恢复原来的时间
	setSystemTime(currentTime.wYear, currentTime.wMonth, currentTime.wDay,
		currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
	wprintf(L"Restored to original time: %02d/%02d/%04d %02d:%02d:%02d\n",
		currentTime.wDay, currentTime.wMonth, currentTime.wYear,
		currentTime.wHour, currentTime.wMinute, currentTime.wSecond);

	return 0;
}
