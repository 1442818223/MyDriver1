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
	if (!hDevice)
	{
		printf("%x", GetLastError());
	}
	else
	{
		//int x = 500;
		//DWORD p = 0;
		////x对应要请求驱动读取的地方
		//ReadFile(hDevice, &x, 4, &p, NULL); //程序一加载 驱动就会拿到当前栈进行修改
		//printf("%d\n", x);
		

		DWORD bread = 0;//真正写了多长
		WriteFile(hDevice, "This Message come from R3.", strlen("This Message come from R3."), &bread, NULL);


	
		CloseHandle(hDevice);

		//	char inbuf[260] = { 0 };
		//	char outbuf[260] = { 0 };

		//	memcpy(inbuf, "demo", strlen("demo"));

		//	DWORD retlen = 0;

		//	printf("创建成功\n");


		//	if (DeviceIoControl(hDevice, CTL_TALK,
		//		inbuf,//指向输入数据缓冲区的指针，该数据将被传递给设备。
		//		strlen(inbuf),
		//		outbuf,//指向输出数据缓冲区的指针，该数据将从设备返回。
		//		sizeof(outbuf),
		//		&retlen,//实际传输的字节数
		//		NULL))
		//	{
		//		printf("通信成功\n");
		//		printf("接收到的数据为%d\n", outbuf);

		//	}
		//	else
		//	{
		//		printf("错误码:%d\n", GetLastError());
		//	}
		//}

	}
	getchar();
}