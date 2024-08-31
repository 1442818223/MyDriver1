#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>


int main()
{

	UINT64 glDisablebase = 0x161523;
	unsigned char lpBuffer2[100];
	memcpy(lpBuffer2, &glDisablebase, sizeof(glDisablebase));

	for (int i = 0; i < 3; i++)
	{
		printf("%x ", lpBuffer2[i]);
	}
	printf("0x%x ", *(UINT64*)lpBuffer2);
	printf("0x%x ", *(unsigned char*)lpBuffer2);
	printf("\n");

	unsigned char lpBuffer[] = { 0xc6,0x45,0x48,1,lpBuffer2[2] };
	for (int i = 0; i < sizeof(lpBuffer); i++)
	{
		printf("%x ", lpBuffer[i]);
	}
	OutputDebugString(L"[db]输出调DD试信息123\n");


	 //char* x = (char*)"adsedsefs";  
	 //x[0] = *(char*)"1";   //字符串字面量通常存储在程序的只读数据区，这是为了保护它们不被意外修改。



	system("pause");
	return 0;
}