#pragma once
#include<ntifs.h>
#include<windef.h>

#define READCODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define WRITECODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_ALL_ACCESS)

#define DEVICENAME L"\\device\\tearWrite_driver"
#define SYMBOLNAME L"\\??\\tearWrite_driver"

typedef struct DATA
{
	DWORD pid;//要读写的进程ID
	unsigned __int64 address;//要读写的地址
	DWORD size;//读写长度
	PVOID data;//要读写的数据,  空指针 对应任何数据类型
}Data;

void DriverUnload(PDRIVER_OBJECT driver);
NTSTATUS CreateDevice(PDRIVER_OBJECT driver);
NTSTATUS DriverIrpCtl(PDEVICE_OBJECT device, PIRP pirp);
BOOL ReadMemory(Data* data);
BOOL WriteMemory(Data* data);
