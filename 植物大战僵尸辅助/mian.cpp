#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<windows.h>
#include <iostream>
#include <psapi.h>

DWORD_PTR GetProcessBaseAddress(DWORD processID)   //GetModuleHandleA只能获取本进程的句柄(强转就是基地址)
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
//修改进程代码区代码 参数: 进程句柄 修改代码起始地址 硬编码指针 代码字节数
BOOL WriteProcessCodeMemory(HANDLE hProcess, LPVOID lpStartAddress, LPCVOID lpBuffer, SIZE_T nSize) {
	DWORD dwOldProtect;
	//取消页保护
	if (!VirtualProtectEx(hProcess, lpStartAddress, nSize, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
		return FALSE;
	}
	BOOL bResult = WriteProcessMemory(hProcess, lpStartAddress, lpBuffer, nSize, NULL);//写入代码
	VirtualProtectEx(hProcess, lpStartAddress, nSize, dwOldProtect, &dwOldProtect);//开启页保护
	return bResult;
}
int main()
{
	HWND h = FindWindow(NULL, L"植物大战僵尸中文版");//获取窗口句柄
	DWORD processid;
	GetWindowThreadProcessId(h, &processid);//获取进程id

	HANDLE Hprocess = NULL;
	Hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processid);//打开进程
	
	DWORD_PTR base = GetProcessBaseAddress(processid); //获取别的进程的 获取本进程就是GetModuleHandleA
	printf("16进制基址为: %x\n", base);
	
		if (Hprocess == 0)
		{

			printf("打开进程失败！\n");

		}
		else
		{
			//[[base+2A9EC0]+768]+5560]
			DWORD_PTR temp = 0;//临时变量1，用于叠加偏移
			ReadProcessMemory(Hprocess, (LPCVOID)(base + 0x2A9EC0), &temp, sizeof(DWORD_PTR), NULL);
			printf("第一次读:%x\n", temp);

			DWORD_PTR temp2 = 0;
			ReadProcessMemory(Hprocess, (LPCVOID)(temp + 0x768), &temp2, sizeof(DWORD_PTR), NULL);
			printf("第二次读:%x\n", temp2);

			printf("打开进程成功！\n");
			printf("阳光修改为：");
			int n;
			scanf("%d", &n);

			DWORD a = WriteProcessMemory(Hprocess, (LPVOID)(temp2 + 0x5560), &n, 4, 0);//注意参数
			if (a == 0)
				printf("修改失败！");
			else
				printf("修改成功！");

		}
		//PlantsVsZombies.exe + 88E73 - C6 45 48 01 - mov byte ptr[ebp + 48], 01

		//005227B5 - C7 87 C8000000 00000000 - mov[edi + 000000C8], 00000000 这种不行
		//PlantsVsZombies.exe + 1227B5 - C7 87 C8000000 00000000 - mov[edi + 000000C8], 00000000


		unsigned char lpBuffer[] = {0xc6,0x45,0x48,1};
		SIZE_T nSize=sizeof(lpBuffer);
		BOOL bResult = WriteProcessCodeMemory(Hprocess, (LPVOID)(base + 0x88e73), lpBuffer, nSize);
		if (bResult) {
			printf("无CD修改成功！\n");
		}

		unsigned char lpBuffer2[] = { 0xc7,0x87 ,0xC8,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		SIZE_T nSize2 = sizeof(lpBuffer2);
	    bResult = WriteProcessCodeMemory(Hprocess, (LPVOID)(base + 0x1227B5), lpBuffer2, nSize2);
		if (bResult) {
			printf("小僵尸秒杀开启成功！\n");
		}

		//PlantsVsZombies.exe+1BA74 - 01 DE                 - add esi,ebx
		unsigned char lpBuffer3[] = { 0x01,0xDE };
		SIZE_T nSize3 = sizeof(lpBuffer3);
		bResult = WriteProcessCodeMemory(Hprocess, (LPVOID)(base + 0x1BA74), lpBuffer3, nSize3);
		if (bResult) {
			printf("阳光反向增加开启成功！\n");
		}
	system("pause");

}