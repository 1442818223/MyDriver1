#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<windows.h>
#include <iostream>
#include <psapi.h>

DWORD_PTR GetProcessBaseAddress(DWORD processID)   //GetModuleHandleAֻ�ܻ�ȡ�����̵ľ��(ǿת���ǻ���ַ)
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
//�޸Ľ��̴��������� ����: ���̾�� �޸Ĵ�����ʼ��ַ Ӳ����ָ�� �����ֽ���
BOOL WriteProcessCodeMemory(HANDLE hProcess, LPVOID lpStartAddress, LPCVOID lpBuffer, SIZE_T nSize) {
	DWORD dwOldProtect;
	//ȡ��ҳ����
	if (!VirtualProtectEx(hProcess, lpStartAddress, nSize, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
		return FALSE;
	}
	BOOL bResult = WriteProcessMemory(hProcess, lpStartAddress, lpBuffer, nSize, NULL);//д�����
	VirtualProtectEx(hProcess, lpStartAddress, nSize, dwOldProtect, &dwOldProtect);//����ҳ����
	return bResult;
}
int main()
{
	HWND h = FindWindow(NULL, L"ֲ���ս��ʬ���İ�");//��ȡ���ھ��
	DWORD processid;
	GetWindowThreadProcessId(h, &processid);//��ȡ����id

	HANDLE Hprocess = NULL;
	Hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processid);//�򿪽���
	
	DWORD_PTR base = GetProcessBaseAddress(processid); //��ȡ��Ľ��̵� ��ȡ�����̾���GetModuleHandleA
	printf("16���ƻ�ַΪ: %x\n", base);
	
		if (Hprocess == 0)
		{

			printf("�򿪽���ʧ�ܣ�\n");

		}
		else
		{
			//[[base+2A9EC0]+768]+5560]
			DWORD_PTR temp = 0;//��ʱ����1�����ڵ���ƫ��
			ReadProcessMemory(Hprocess, (LPCVOID)(base + 0x2A9EC0), &temp, sizeof(DWORD_PTR), NULL);
			printf("��һ�ζ�:%x\n", temp);

			DWORD_PTR temp2 = 0;
			ReadProcessMemory(Hprocess, (LPCVOID)(temp + 0x768), &temp2, sizeof(DWORD_PTR), NULL);
			printf("�ڶ��ζ�:%x\n", temp2);

			printf("�򿪽��̳ɹ���\n");
			printf("�����޸�Ϊ��");
			int n;
			scanf("%d", &n);

			DWORD a = WriteProcessMemory(Hprocess, (LPVOID)(temp2 + 0x5560), &n, 4, 0);//ע�����
			if (a == 0)
				printf("�޸�ʧ�ܣ�");
			else
				printf("�޸ĳɹ���");

		}
		//PlantsVsZombies.exe + 88E73 - C6 45 48 01 - mov byte ptr[ebp + 48], 01

		//005227B5 - C7 87 C8000000 00000000 - mov[edi + 000000C8], 00000000 ���ֲ���
		//PlantsVsZombies.exe + 1227B5 - C7 87 C8000000 00000000 - mov[edi + 000000C8], 00000000


		unsigned char lpBuffer[] = {0xc6,0x45,0x48,1};
		SIZE_T nSize=sizeof(lpBuffer);
		BOOL bResult = WriteProcessCodeMemory(Hprocess, (LPVOID)(base + 0x88e73), lpBuffer, nSize);
		if (bResult) {
			printf("��CD�޸ĳɹ���\n");
		}

		unsigned char lpBuffer2[] = { 0xc7,0x87 ,0xC8,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		SIZE_T nSize2 = sizeof(lpBuffer2);
	    bResult = WriteProcessCodeMemory(Hprocess, (LPVOID)(base + 0x1227B5), lpBuffer2, nSize2);
		if (bResult) {
			printf("С��ʬ��ɱ�����ɹ���\n");
		}

		//PlantsVsZombies.exe+1BA74 - 01 DE                 - add esi,ebx
		unsigned char lpBuffer3[] = { 0x01,0xDE };
		SIZE_T nSize3 = sizeof(lpBuffer3);
		bResult = WriteProcessCodeMemory(Hprocess, (LPVOID)(base + 0x1BA74), lpBuffer3, nSize3);
		if (bResult) {
			printf("���ⷴ�����ӿ����ɹ���\n");
		}
	system("pause");

}