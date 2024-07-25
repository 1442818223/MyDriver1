#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <thread>
#include <vector>
#include <random>
#include <numeric>

HHOOK hHook = NULL;  // 钩子的句柄

bool running = false;
POINT lockedPosition;

// 模拟鼠标点击
void clickMouse() {
	INPUT Inputs[2] = { 0 };

	Inputs[0].type = INPUT_MOUSE;
	Inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	Inputs[1].type = INPUT_MOUSE;
	Inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(1, &Inputs[0], sizeof(INPUT));
	Sleep(150);
	SendInput(1, &Inputs[1], sizeof(INPUT));
}

// 生成随机时间间隔的函数
std::vector<int> generateRandomIntervals(int count, int totalTime) {
	std::vector<int> intervals(count);
	std::random_device rd;
	std::mt19937 gen(rd());

	int remainingTime = totalTime;
	for (int i = 0; i < count - 1; ++i) {
		std::uniform_int_distribution<> dis(1, remainingTime - (count - i - 1));
		intervals[i] = dis(gen);
		remainingTime -= intervals[i];
	}
	intervals[count - 1] = remainingTime;

	for (int i = 0; i < count; ++i) {
		printf("Interval %d: %d ms\n", i + 1, intervals[i]);
	}

	return intervals;
}

// 开始连续点击
void startContinuousClick(int x, int y) {
	if (!running) {
		printf("Starting continuous click at locked position (%d, %d)\n", x, y);
		running = true;
		lockedPosition.x = x;
		lockedPosition.y = y;
		std::thread([&]() {
			std::vector<int> intervals = generateRandomIntervals(4, 1000);
			while (running) {
				for (int i = 0; i < intervals.size() && running; ++i) {
					clickMouse();
					Sleep(intervals[i]);
				}
				intervals = generateRandomIntervals(4, 1000);
			}
			}).detach();
	}
}

// 停止连续点击
void stopContinuousClick() {
	if (running) {
		printf("Stopping continuous click\n");
		running = false;
	}
}

// 键盘钩子的回调函数
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;

		// 处理按下的键
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			bool ctrlPressed = GetAsyncKeyState(VK_CONTROL) & 0x8000;

			if (ctrlPressed) {
				switch (pKeyBoard->vkCode) {
				case VK_F1:  // Ctrl+F1
					startContinuousClick(348, 118);
					break;
				case VK_F2:  // Ctrl+F2
					stopContinuousClick();
					break;
				}
			}
		}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

int main() {
	// 注册低级键盘钩子
	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
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
