#define _CRT_SECURE_NO_WARNINGS
#include"interface.h"
#include<iostream>
#include<stdlib.h>

#include<stdio.h>
#include<windows.h>
#include <iostream>
#include <psapi.h>
DWORD_PTR GetProcessBaseAddress(DWORD processID)
{
	DWORD_PTR   baseAddress = 0;
	HANDLE      processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	HMODULE* moduleArray;
	LPBYTE      moduleArrayBytes;
	DWORD       bytesRequired;

	if (processHandle)
	{
		if (EnumProcessModules(processHandle, NULL, 0, &bytesRequired))
		{
			if (bytesRequired)
			{
				moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);

				if (moduleArrayBytes)
				{
					unsigned int moduleCount;

					moduleCount = bytesRequired / sizeof(HMODULE);
					moduleArray = (HMODULE*)moduleArrayBytes;

					if (EnumProcessModules(processHandle, moduleArray, bytesRequired, &bytesRequired))
					{
						baseAddress = (DWORD_PTR)moduleArray[0];
					}

					LocalFree(moduleArrayBytes);
				}
			}
		}

		CloseHandle(processHandle);
	}

	return baseAddress;
}
int main() {

	HWND h = FindWindow(NULL, L"植物大战僵尸中文版");//获取窗口句柄
	DWORD processid;
	GetWindowThreadProcessId(h, &processid);//获取进程id
	
	//驱动链接qingk
	printf("\n");
	std::cout << "驱动链接成功--来自用户层的画" << std::endl;

	
	//[[base+2A9EC0]+768]+5560]
	DWORD_PTR base = GetProcessBaseAddress(processid);
	printf("16进制基址为: %x\n", base);
	
	HANDLE pid = (HANDLE)processid;

	DWORD tes = driver.ReadMemoryByTear<DWORD>(pid, base + 0x2A9EC0, sizeof(DWORD_PTR));
	printf("测试输出%d\n", tes);
	DWORD tes2 = driver.ReadMemoryByTear<DWORD>(pid, tes + 0x768, sizeof(DWORD_PTR));
	printf("测试输出%d\n", tes2);


	driver.WriteMemoryByTear<DWORD>(pid, tes2+0x5560, sizeof(DWORD), 500);
	printf("写入成功");
	system("pause");
}