#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "windows.h"



int main()
{
	UINT64 glDisablebase = 0x161523;
	unsigned char lpBuffer2[100];
	memcpy(lpBuffer2, &glDisablebase, sizeof(glDisablebase));

	for (int i = 0; i < sizeof(glDisablebase); i++)
	{
		printf("%x ", lpBuffer2[i]);
	}
	printf("\n");
	unsigned char lpBuffer[] = { 0xc6,0x45,0x48,1,lpBuffer2[2] };
	for (int i = 0; i < sizeof(lpBuffer); i++)
	{
		printf("%x ", lpBuffer[i]);
	}
	OutputDebugString(L"输出调DD试信息123\n");

	system("pause");
	return 0;
}