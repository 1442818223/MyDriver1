#include <stdio.h>
#include <windows.h>
#define  _SYB_NAME	 L"\\\\.\\sysmblicname"
#define CTL_TALK  CTL_CODE(FILE_DEVICE_UNKNOWN,0x9000,METHOD_BUFFERED,FILE_ANY_ACCESS)

int main()
{
	HANDLE hDevice = CreateFile(_SYB_NAME,
		FILE_ALL_ACCESS,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	BOOL flag;

	if (hDevice==INVALID_HANDLE_VALUE)
	{
		printf("%x", GetLastError());
	}
	else
	{
		char inbuf[260] = { 0 };
		char outbuf[260] = { 0 };

		memcpy(inbuf, "demo", strlen("demo"));

		DWORD retlen = 0;

		printf("�����ɹ�\n");


		if (DeviceIoControl(hDevice, CTL_TALK, 
			inbuf,//ָ���������ݻ�������ָ�룬�����ݽ������ݸ��豸��
			strlen(inbuf),
			outbuf,//ָ��������ݻ�������ָ�룬�����ݽ����豸���ء�
			sizeof(outbuf),
			&retlen,//ʵ�ʴ�����ֽ���
			NULL))
		{
			printf("ͨ�ųɹ�\n");
			printf("���յ�������Ϊ%d\n", outbuf);

		}
		else
		{
			printf("������:%d\n", GetLastError());
		}
	}
	getchar();
}