#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <thread>

HHOOK hHook = NULL;  // ���ӵľ��

// �첽���ŷ�����
void asyncBeep(int frequency, int duration) {    //Beep()������
	std::thread([frequency, duration]() {
		Beep(frequency, duration);
		}).detach();
}

// ��깳�ӵĻص�����
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;

		if (wParam == WM_LBUTTONDOWN) {
			// ����������ʱ���ŷ�����
			asyncBeep(750, 300);  // ʹ���첽�̲߳��ŷ�����
		}
		else if (wParam == WM_LBUTTONUP) {
			// ������̧��ʱ���ŷ�����
			asyncBeep(3000, 300);  // ʹ���첽�̲߳��ŷ�����
		}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

int main() {
	// ע��ͼ���깳��
	hHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
	if (!hHook) {
		printf("Failed to install hook!\n");
		return 1;
	}

	// ��Ϣѭ��
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// ȡ������
	UnhookWindowsHookEx(hHook);
	return 0;
}
