#include <windows.h>
#include <iostream>

void SendKeyPress(HWND hwnd, WORD virtualKey) {
	// �����ڴ���ǰ̨
	SetForegroundWindow(hwnd);

	// ����һ��INPUT�ṹ�����飬���ڴ洢���º��ͷŰ�������Ϣ
	INPUT input[2] = { 0 };

	// ���ð��°�������Ϣ
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = virtualKey;  // �������

	// �����ͷŰ�������Ϣ
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = virtualKey;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;  // ���Ϊ�ͷŰ���

	// ����INPUT�¼������º��ͷţ�
	SendInput(2, input, sizeof(INPUT));
}

void SendKeysToWindow(LPCWSTR windowTitle) {
	// �ҵ����ھ��
	HWND hwnd = FindWindow(NULL, windowTitle);
	if (hwnd == NULL) {
		std::wcout << L"δ�ҵ�����: " << windowTitle << std::endl;
		return;
	}

	// ���� Y ��
	SendKeyPress(hwnd, 'Y');
	Sleep(500);  // �ȴ�500����

	// ���ͻس���
	SendKeyPress(hwnd, VK_RETURN);
	Sleep(500);  // �ȴ�500����
}

int main() {

	int n = 3;
	while (n--) {
		SendKeysToWindow(L"��ȫ����");
		// �ȴ�һ��ʱ����ټ��
		Sleep(500);  // ÿ��1����һ��
	}

	return 0;
}
