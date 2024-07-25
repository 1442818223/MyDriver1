#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <thread>

HHOOK hHook = NULL;  // 钩子的句柄

// 异步播放蜂鸣声
void asyncBeep(int frequency, int duration) {    //Beep()会阻塞
	std::thread([frequency, duration]() {
		Beep(frequency, duration);
		}).detach();
}

// 鼠标钩子的回调函数
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;

		if (wParam == WM_LBUTTONDOWN) {
			// 鼠标左键按下时播放蜂鸣声
			asyncBeep(750, 300);  // 使用异步线程播放蜂鸣声
		}
		else if (wParam == WM_LBUTTONUP) {
			// 鼠标左键抬起时播放蜂鸣声
			asyncBeep(3000, 300);  // 使用异步线程播放蜂鸣声
		}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

int main() {
	// 注册低级鼠标钩子
	hHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
	if (!hHook) {
		printf("Failed to install hook!\n");
		return 1;
	}

	// 消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 取消钩子
	UnhookWindowsHookEx(hHook);
	return 0;
}
