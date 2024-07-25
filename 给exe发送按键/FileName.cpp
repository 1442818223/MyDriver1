#include <windows.h>
#include <iostream>

void SendKeyPress(HWND hwnd, WORD virtualKey) {
	// 将窗口带到前台
	SetForegroundWindow(hwnd);

	// 创建一个INPUT结构体数组，用于存储按下和释放按键的信息
	INPUT input[2] = { 0 };

	// 设置按下按键的信息
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = virtualKey;  // 虚拟键码

	// 设置释放按键的信息
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = virtualKey;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;  // 标记为释放按键

	// 发送INPUT事件（按下和释放）
	SendInput(2, input, sizeof(INPUT));
}

void SendKeysToWindow(LPCWSTR windowTitle) {
	// 找到窗口句柄
	HWND hwnd = FindWindow(NULL, windowTitle);
	if (hwnd == NULL) {
		std::wcout << L"未找到窗口: " << windowTitle << std::endl;
		return;
	}

	// 发送 Y 键
	SendKeyPress(hwnd, 'Y');
	Sleep(500);  // 等待500毫秒

	// 发送回车键
	SendKeyPress(hwnd, VK_RETURN);
	Sleep(500);  // 等待500毫秒
}

int main() {

	int n = 3;
	while (n--) {
		SendKeysToWindow(L"安全警告");
		// 等待一段时间后再检查
		Sleep(500);  // 每隔1秒检查一次
	}

	return 0;
}
