#include<stdio.h>
#include<Windows.h>

void 通用_输出调试信息(const char* pszFormat, ...)
{
	char szbufFormat[0x1000];
	char szbufFormat_Game[0x1100] = "";
	va_list argList;
	va_start(argList, pszFormat);
	vsprintf_s(szbufFormat, pszFormat, argList);
	strcat_s(szbufFormat_Game, "FPS ");// 加上输出头特征
	strcat_s(szbufFormat_Game, szbufFormat);
	OutputDebugStringA(szbufFormat_Game);// 编码转换
	va_end(argList);
}
int main() {

	通用_输出调试信息("DEBUG: %s", "Hello, World!");
	

	const wchar_t* message = L"Hello, Debugger!1";
	OutputDebugStringW(message);

	system("pause");


	return 0;
}