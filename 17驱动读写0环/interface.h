#pragma once
#include<ntifs.h>
#include<windef.h>

#define READCODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define WRITECODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_ALL_ACCESS)

#define DEVICENAME L"\\device\\tearWrite_driver"
#define SYMBOLNAME L"\\??\\tearWrite_driver"

typedef struct DATA
{
	DWORD pid;//Ҫ��д�Ľ���ID
	unsigned __int64 address;//Ҫ��д�ĵ�ַ
	DWORD size;//��д����
	PVOID data;//Ҫ��д������,  ��ָ�� ��Ӧ�κ���������
}Data;

void DriverUnload(PDRIVER_OBJECT driver);
NTSTATUS CreateDevice(PDRIVER_OBJECT driver);
NTSTATUS DriverIrpCtl(PDEVICE_OBJECT device, PIRP pirp);
BOOL ReadMemory(Data* data);
BOOL WriteMemory(Data* data);
