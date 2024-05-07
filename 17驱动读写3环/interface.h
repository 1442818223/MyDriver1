#pragma once
#pragma once
#include <windows.h>
#include <stdio.h>
#include <winioctl.h>
#define READCODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define WRITECODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_ALL_ACCESS)

typedef struct DATA
{
	DWORD pid;//要读写的进程ID
	unsigned __int64 address;//要读写的地址
	DWORD size;//读写长度
	PVOID data;//要读写的数据,  要读出来的数据   MDLgua挂靠的读写啊 几乎 记住了 
}Data;


class tearInterface {
public:
	HANDLE driver;

	tearInterface(LPCSTR RegistryPath)
	{
		driver = CreateFileA(RegistryPath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (driver == INVALID_HANDLE_VALUE) {
			printf("驱动链接失败");
			system("pause");
			exit(0);
		}
		printf("驱动链接OK 驱动开发者--泪心");
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
