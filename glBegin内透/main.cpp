#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<windows.h>
#include <iostream>
#include <psapi.h>
#include<iostream>

using namespace std;

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

					moduleCount = bytesRequired / sizeof(HMODULE); //ģ������
					moduleArray = (HMODULE*)moduleArrayBytes;

					if (EnumProcessModules(processHandle, moduleArray, bytesRequired, &bytesRequired))
					{
						baseAddress = (DWORD_PTR)moduleArray[0]; //��1���������߳�
					}

					LocalFree(moduleArrayBytes);
				}
			}
		}

		CloseHandle(processHandle);
	}

	return baseAddress;
}

PVOID GetProcessMoudleBase(HANDLE hProcess, char* moduleName)
{
	// ��������ģ��,
	HMODULE hModule[100] = { 0 };
	DWORD dwRet = 0;
	BOOL bRet = ::EnumProcessModules(hProcess, (HMODULE*)(hModule), sizeof(hModule), &dwRet);
	if (FALSE == bRet)
	{
		::CloseHandle(hProcess);
		return NULL;
	}
	char name[50] = { 0 };
	for (int i = 0; i < dwRet; i++)
	{
		GetModuleBaseNameA(hProcess, hModule[i], name, 50);//���ڻ�ȡָ��������ָ��ģ��Ļ�������

		if (!_stricmp(moduleName, name))
		{
			cout << name << "     Addr:" << hex << hModule[i] << endl;
			return hModule[i];
		}
	}

	::CloseHandle(hProcess);
	return NULL;
}
//ͨ����������ȡ������ַ(Ҫ�ĳ�64λֻ��Ҫ��IMAGE_OPTIONAL_HEADER�ĳ�IMAGE_OPTIONAL_HEADER64��������Ϊ64λ):
UINT64 GetFunAddrByName32(HANDLE hProcess, char* ModName, char* FunName)
{
	HANDLE hMod;
	PVOID BaseAddress = NULL;
	IMAGE_DOS_HEADER dosheader;     
	IMAGE_OPTIONAL_HEADER opthdr;//IMAGE_OPTIONAL_HEADER64
	IMAGE_EXPORT_DIRECTORY exports;
	USHORT index = 0;
	ULONG addr, i;
	char pFuncName[30] = { 0 };
	PULONG pAddressOfFunctions;
	PULONG pAddressOfNames;
	PUSHORT pAddressOfNameOrdinals;

	//��ȡģ���ַ
	BaseAddress = GetProcessMoudleBase(hProcess, ModName);
	if (!BaseAddress) return 0;

	//��ȡPEͷ
	hMod = BaseAddress;
	ReadProcessMemory(hProcess, hMod, &dosheader, sizeof(IMAGE_DOS_HEADER), 0);
	ReadProcessMemory(hProcess, (BYTE*)hMod + dosheader.e_lfanew + 24, &opthdr, sizeof(IMAGE_OPTIONAL_HEADER), 0);
	//ReadProcessMemory(hProcess, (BYTE*)hMod + dosheader.e_lfanew + 24, &opthdr, sizeof(IMAGE_OPTIONAL_HEADER64), 0);

	//���ҵ����� 
	ReadProcessMemory(hProcess, ((BYTE*)hMod + opthdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), &exports, sizeof(IMAGE_EXPORT_DIRECTORY), 0);

	pAddressOfFunctions = (ULONG*)((BYTE*)hMod + exports.AddressOfFunctions);
	pAddressOfNames = (ULONG*)((BYTE*)hMod + exports.AddressOfNames);
	pAddressOfNameOrdinals = (USHORT*)((BYTE*)hMod + exports.AddressOfNameOrdinals);

	//�ԱȺ����� 
	for (i = 0; i < exports.NumberOfNames; i++)
	{
		ReadProcessMemory(hProcess, pAddressOfNameOrdinals + i, &index, sizeof(USHORT), 0);
		ReadProcessMemory(hProcess, pAddressOfFunctions + index, &addr, sizeof(ULONG), 0);

		ULONG a = 0;
		ReadProcessMemory(hProcess, pAddressOfNames + i, &a, sizeof(ULONG), 0);
		ReadProcessMemory(hProcess, (BYTE*)hMod + a, pFuncName, 30, 0);
		ReadProcessMemory(hProcess, pAddressOfFunctions + index, &addr, sizeof(ULONG), 0);

		if (!_stricmp(pFuncName, FunName))
		{
			UINT64 funAddr = (UINT64)BaseAddress + addr;
			cout << pFuncName << "   " << hex << funAddr << endl;
			return funAddr;
		}
	}
	return 0;
}
//�޸Ľ��̴��������� ����: ���̾�� �޸Ĵ�����ʼ��ַ Ӳ����ָ�� �����ֽ���
BOOL WriteProcessCodeMemory(HANDLE hProcess, LPVOID lpStartAddress, LPCVOID lpBuffer, SIZE_T nSize) {
	DWORD dwOldProtect;
	//ȡ��ҳ����
	//if (!VirtualProtectEx(hProcess, lpStartAddress, nSize, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
	//	return FALSE;
	//}
	BOOL bResult = WriteProcessMemory(hProcess, lpStartAddress, lpBuffer, nSize, NULL);//д�����
	//VirtualProtectEx(hProcess, lpStartAddress, nSize, dwOldProtect, &dwOldProtect);//����ҳ����
	return bResult;
}

int main() {

	HWND h = FindWindow(NULL, L"Counter-Strike");//��ȡ���ھ��
	DWORD processid;
	GetWindowThreadProcessId(h, &processid);//��ȡ����id

	HANDLE Hprocess = NULL;
	Hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processid);//�򿪽���

	char* Dllname = (char*)"opengl32.dll";   char* Funcname = (char*)"glBegin";
	UINT64 glBeginbase = GetFunAddrByName32(Hprocess, Dllname, Funcname); //��ȡ��Ľ��̵� ��ȡ�����̾���GetModuleHandleA
	printf("16����glBegin������ַΪ: %x\n", glBeginbase);

	  char* Funcname2 = (char*)"glDisable";
	  UINT64 glDisablebase = GetFunAddrByName32(Hprocess, Dllname, Funcname2);
	printf("16����glDisablebase������ַΪ: %x\n", glDisablebase);
	
	if (Hprocess == 0)
	{

		printf("�򿪽���ʧ�ܣ�\n");

	}
	else
	{


		unsigned char lpBuffer2[] = { 0x83,0x7c,0x24,0x08,0x05 };
		SIZE_T nSize2 = sizeof(lpBuffer2);
		BOOL bResult1 = WriteProcessCodeMemory(Hprocess, (LPVOID)(glBeginbase-0x50), lpBuffer2, nSize2);
		if (bResult1) {
			printf("cmp�ɹ���\n");
		}


		unsigned char lpBuffer3[] = { 0x75,0x0d,0x90,0x90,0x90};
		SIZE_T nSize3= sizeof(lpBuffer3);
		BOOL bResult2 = WriteProcessCodeMemory(Hprocess, (LPVOID)(glBeginbase - 0x50+5), lpBuffer3, nSize3);
		if (bResult2) {
			printf("jne�ɹ���\n");
		}


		unsigned char lpBuffer4[] = { 0x68,0x71,0x0b,0x00,0x00 };
		SIZE_T nSize4 = sizeof(lpBuffer4);
		BOOL bResult3 = WriteProcessCodeMemory(Hprocess, (LPVOID)(glBeginbase - 0x50 + 0xa), lpBuffer4, nSize4);
		if (bResult3) {
			printf("push b71�ɹ���\n");
		}


		UINT64 jl = glDisablebase - (glBeginbase - 0x50 + 0xf + 5); //Ӳ����Ҫ�����λ��
		unsigned char lpBuffer9[5];
        memcpy(lpBuffer9, &jl, sizeof(jl));
		printf("[0]: %x", lpBuffer9[0]);
		printf("[1]: %x", lpBuffer9[1]);

		unsigned char lpBuffer5[] = { 0xe8,lpBuffer9[0],lpBuffer9[1],0x00,0x00 };
		SIZE_T nSize5 = sizeof(lpBuffer5);
		BOOL bResult4 = WriteProcessCodeMemory(Hprocess, (LPVOID)(glBeginbase - 0x50 + 0xf), lpBuffer5, nSize5);
		if (bResult4) {
			printf("call glDisable�ɹ���\n");
		}

		unsigned char lpBuffer6[] = { 0x64,0xa1,0x18,0x00,0x00,0x00 };
		SIZE_T nSize6 = sizeof(lpBuffer6);
		BOOL bResult5 = WriteProcessCodeMemory(Hprocess, (LPVOID)(glBeginbase - 0x50 + 0x14), lpBuffer6, nSize6);
		if (bResult5) {
			printf("�޸����ƻ�����ɹ���\n");
		}

		//UINT64 jl2 = glBeginbase;
		//unsigned char lpBuffe11[8];
		//memcpy(lpBuffe11, &jl2, sizeof(jl2));
		//printf("[0]: %x", lpBuffe11[0]);
		//printf("[2]: %x", lpBuffe11[1]);
		//printf("[3]: %x", lpBuffe11[2]);
		//printf("[4]: %x", lpBuffe11[3]);
		//printf("[5]: %x", lpBuffe11[4]);
		//printf("[6]: %x", lpBuffe11[5]);
		//printf("[7]: %x", lpBuffe11[6]);
		//printf("[8]: %x", lpBuffe11[7]);
	
		//unsigned char lpBuffer7[] = { 0x90, 0xe9,lpBuffe11[0],lpBuffe11[1],lpBuffe11[2],lpBuffe11[3],0x90 };
		unsigned char lpBuffer7[] = {0xeb,0x3c,0x90,0x90 };
		SIZE_T nSize7 = sizeof(lpBuffer7);
		BOOL bResult6 = WriteProcessCodeMemory(Hprocess, (LPVOID)(glBeginbase - 0x50 + 0x1A), lpBuffer7, nSize7);
		if (bResult6) {
			printf("���سɹ���\n");
		}

		

		unsigned char lpBuffer[] = { 0xeb,0xa9,0x90,0x90,0x90,0x90};
		SIZE_T nSize = sizeof(lpBuffer);
		BOOL bResult = WriteProcessCodeMemory(Hprocess, (LPVOID)(glBeginbase+5), lpBuffer, nSize);
		if (bResult) {
			printf("��ת�ɹ���\n");
		}

		//__asm
		//{
		//	mov  eax, test //��test��ֵд��eax�Ĵ���
		//	dec  eax  //�Ĵ�����ֵ��һ
		//	mov  test, eax //��eax��ֵд��test������
		//}

	
	
	
	
	}

	system("pause");


}