#pragma once
#pragma once
#include <windows.h>
#include <stdio.h>
#include <winioctl.h>
#define READCODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define WRITECODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_ALL_ACCESS)

typedef struct DATA
{
	DWORD pid;//Ҫ��д�Ľ���ID
	unsigned __int64 address;//Ҫ��д�ĵ�ַ
	DWORD size;//��д����
	PVOID data;//Ҫ��д������,  Ҫ������������   MDLgua�ҿ��Ķ�д�� ���� ��ס�� 
}Data;


class tearInterface {
public:
	HANDLE driver;

	tearInterface(LPCSTR RegistryPath)
	{
		driver = CreateFileA(RegistryPath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (driver == INVALID_HANDLE_VALUE) {
			printf("��������ʧ��");
			system("pause");
			exit(0);
		}
		printf("��������OK ����������--����");
	}

	template<typename  type>
	type ReadMemoryByTear(HANDLE ProcessId, unsigned __int64 readAddress, SIZE_T size) {
		Data read;
		read.address = readAddress;
		read.size = size;
		read.pid = (DWORD)ProcessId;
		//	read.data;
		if (DeviceIoControl(driver, READCODE, &read, sizeof(read), &read, sizeof(read), 0, 0))
			return (type)read.data;
	}

	template<typename  WriteValue>
	bool WriteMemoryByTear(HANDLE Processid, unsigned __int64 addr, SIZE_T siz, WriteValue value)
	{
		Data write;
		write.address = addr;
		write.pid = (DWORD)Processid;
		write.size = sizeof(WriteValue);
		if (DeviceIoControl(driver, WRITECODE, &write, sizeof(write), &write, sizeof(write), 0, 0))
			return true;


	}





};

static  tearInterface driver("\\\\.\\tearWrite_driver");
