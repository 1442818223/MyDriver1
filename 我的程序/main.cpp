#include<stdio.h>
#include<Windows.h>

void ͨ��_���������Ϣ(const char* pszFormat, ...)
{
	char szbufFormat[0x1000];
	char szbufFormat_Game[0x1100] = "";
	va_list argList;
	va_start(argList, pszFormat);
	vsprintf_s(szbufFormat, pszFormat, argList);
	strcat_s(szbufFormat_Game, "FPS ");// �������ͷ����
	strcat_s(szbufFormat_Game, szbufFormat);
	OutputDebugStringA(szbufFormat_Game);// ����ת��
	va_end(argList);
}
int main() {

	ͨ��_���������Ϣ("DEBUG: %s", "Hello, World!");
	

	const wchar_t* message = L"Hello, Debugger!1";
	OutputDebugStringW(message);

	system("pause");


	return 0;
}