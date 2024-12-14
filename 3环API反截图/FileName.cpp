#define _CRT_SECURE_NO_WARNINGS

#include<Windows.h>
#include<stdio.h>
#include "detours.h"
#include "detver.h"

#pragma comment(lib,"detours.lib")
// ����ԭʼ������ָ��
BOOL(WINAPI* oldBitBlt)(
	_In_ HDC hdc, _In_ int x, _In_ int y, _In_ int cx, _In_ int cy, _In_opt_ HDC hdcSrc, _In_ int x1, _In_ int y1, _In_ DWORD rop
	) = BitBlt;

// �����Լ������غ���
BOOL WINAPI MyBitBlt(
	_In_ HDC hdc, _In_ int x, _In_ int y, _In_ int cx, _In_ int cy, _In_opt_ HDC hdcSrc, _In_ int x1, _In_ int y1, _In_ DWORD rop
) {

	printf("MyBitBlt called: hdc=%p, x=%d, y=%d, cx=%d, cy=%d, hdcSrc=%p, x1=%d, y1=%d, rop=0x%X\n",
		hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	// ��� hdcSrc ��Ϊ 0��˵�������ѱ�����
	if ((x != 0 && y != 0) || hdcSrc != 0) {
	
		// �����в�����ֵΪ 0��ʹ��ͼʧЧ
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
	// ����ԭʼ�����Ľ��
	return oldBitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
}
// ��װ����
void Hook() {
	// ��ʼ Detour ����
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	// �����Ӹ��ӵ�ԭʼ������
	DetourAttach(&(PVOID&)oldBitBlt, MyBitBlt);

	// �ύ����
	DetourTransactionCommit();

}
// ж�ع���
void Unhook() {
	

	// ��ʼ Detour ����
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	// �����Ӵ�ԭʼ�����Ϸ���
	DetourDetach(&(PVOID&)oldBitBlt, MyBitBlt);

	// �ύ����
  DetourTransactionCommit();

}
int  main()
{
	Hook();
	// ���Ե��� BitBlt
	HDC hdc = GetDC(NULL);
	if (hdc) {
		BitBlt(hdc, 10, 10, 100, 100, NULL, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hdc);
	}
	system("pause");
	Unhook();
	return 0;
}