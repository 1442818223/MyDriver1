#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main() {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ��ȡ��ǰ����Ŀ¼
	char currentPath[MAX_PATH];
	if (GetCurrentDirectoryA(MAX_PATH, currentPath) == 0) {
		printf("��ȡ��ǰĿ¼ʧ��\n");
		return 1;
	}

	// �����������������·��
	char driverPath[MAX_PATH];
	snprintf(driverPath, sizeof(driverPath), "%s\\12356.sys", currentPath); // �������������ļ���Ϊ 12356.sys
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ֹͣ���������
	const char* stopCmd = "sc stop MyDriver";  // �޸�Ϊ MyDriver
	// ɾ�����������
	const char* deleteCmd = "sc delete MyDriver"; // �޸�Ϊ MyDriver

	// ִ��ֹͣ��������
	int resultStop = system(stopCmd);
	if (resultStop != 0) {
		printf("ֹͣ����ʧ��\n");
	}

	// ִ��ɾ����������
	int resultDelete = system(deleteCmd);
	if (resultDelete != 0) {
		printf("ɾ������ʧ��\n");
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ������������
	char createCmd[512];
	snprintf(createCmd, sizeof(createCmd), "sc create MyDriver binPath= \"%s\" type= kernel start= demand", driverPath);
	int resultCreate = system(createCmd);
	if (resultCreate != 0) {
		printf("��������ʧ��\n");
		return resultCreate;
	}

	// ������������
	const char* startCmd = "sc start MyDriver";
	int resultStart = system(startCmd);
	if (resultStart != 0) {
		printf("��������ʧ��\n");
		return resultStart;
	}

	printf("���������ѳɹ����ز�������\n");

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HKEY hKey;
	LONG result;

	// �򿪷����ע�����
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SYSTEM\\CurrentControlSet\\Services\\MyDriver"),
		0,
		KEY_SET_VALUE,
		&hKey);

	if (result != ERROR_SUCCESS) {
		printf("�޷���ע����������룺%ld\n", result);
		return 1;
	}

	// ���� Start ֵΪ 1
	DWORD startValue = 1; // 1 ��ʾ�Զ�����
	result = RegSetValueEx(hKey,
		TEXT("Start"),
		0,
		REG_DWORD,
		(const BYTE*)&startValue,
		sizeof(startValue));

	if (result != ERROR_SUCCESS) {
		printf("�޷����� Start ֵ��������룺%ld\n", result);
		RegCloseKey(hKey);
		return 1;
	}

	printf("Start ֵ�ѳɹ�����Ϊ 1��\n");

	// �ر�ע�����
	RegCloseKey(hKey);


	system("pause");
	return 0;
}
/*
@echo off

rem �л����ű����ڵ�Ŀ¼
cd /d "%~dp0"

rem �������Ƿ����
sc query MyDriver >nul 2>&1
if %errorlevel%==0 (
	echo ������������Ѵ��ڣ�����ֹͣ��ɾ������...

	rem ֹͣ����������ڣ�
	sc stop MyDriver

	rem ɾ���Ѵ��ڵķ���������ڣ�
	sc delete MyDriver
) else (
	echo ����������񲻴��ڣ�����ж�ء�
)

rem ���������������
sc create MyDriver binPath= "%~dp012356.sys" type= kernel start= demand

rem ��������
sc start MyDriver

pause
*/