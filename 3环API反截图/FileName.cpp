#define _CRT_SECURE_NO_WARNINGS

#include<Windows.h>
#include<stdio.h>
#include "detours.h"
#include "detver.h"

#pragma comment(lib,"detours.lib")
// 保存原始函数的指针
BOOL(WINAPI* oldBitBlt)(
	_In_ HDC hdc, _In_ int x, _In_ int y, _In_ int cx, _In_ int cy, _In_opt_ HDC hdcSrc, _In_ int x1, _In_ int y1, _In_ DWORD rop
	) = BitBlt;

// 定义自己的拦截函数
BOOL WINAPI MyBitBlt(
	_In_ HDC hdc, _In_ int x, _In_ int y, _In_ int cx, _In_ int cy, _In_opt_ HDC hdcSrc, _In_ int x1, _In_ int y1, _In_ DWORD rop
) {

	printf("MyBitBlt called: hdc=%p, x=%d, y=%d, cx=%d, cy=%d, hdcSrc=%p, x1=%d, y1=%d, rop=0x%X\n",
		hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	// 如果 hdcSrc 不为 0，说明函数已被调用
	if ((x != 0 && y != 0) || hdcSrc != 0) {
	
		// 将所有参数赋值为 0，使截图失效
		hdc = 0;
		x = 0;
		y = 0;
		cx = 0;
		cy = 0;
		hdcSrc = 0;
		x1 = 0;
		y1 = 0;
		rop = 0;
	}
	// 返回原始函数的结果
	return oldBitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
}
// 安装钩子
void Hook() {
	// 开始 Detour 事务
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	// 将钩子附加到原始函数上
	DetourAttach(&(PVOID&)oldBitBlt, MyBitBlt);

	// 提交事务
	DetourTransactionCommit();

}
// 卸载钩子
void Unhook() {
	

	// 开始 Detour 事务
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	// 将钩子从原始函数上分离
	DetourDetach(&(PVOID&)oldBitBlt, MyBitBlt);

	// 提交事务
  DetourTransactionCommit();

}
int  main()
{
	Hook();
	// 测试调用 BitBlt
	HDC hdc = GetDC(NULL);
	if (hdc) {
		BitBlt(hdc, 10, 10, 100, 100, NULL, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdc);
	}
	system("pause");
	Unhook();
	return 0;
}