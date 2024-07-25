#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

// 函数声明
BOOL ImportPFXCert(LPCWSTR szPFXFilePath, LPCWSTR szPassword, LPCWSTR szStoreName);
void SendKeyPress(HWND hwnd, WORD virtualKey);
void SendKeysToWindow(const std::wstring& windowTitle);
void KeyPressThread(const std::wstring& windowTitle);

// 导入证书
BOOL ImportPFXCert(LPCWSTR szPFXFilePath, LPCWSTR szPassword, LPCWSTR szStoreName) {
	HCERTSTORE hStore = NULL;
	HCERTSTORE hPFXStore = NULL;
	CRYPT_DATA_BLOB PFXData = { 0 };
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0;
	DWORD dwBytesRead = 0;
	PCCERT_CONTEXT pCertContext = NULL;
	BOOL bResult = FALSE;
	BOOL bFirstError = TRUE;

	// 打开PFX文件
	hFile = CreateFileW(szPFXFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		if (bFirstError) {
			MessageBoxW(NULL, L"无法打开PFX文件。", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// 获取PFX文件大小
	dwFileSize = GetFileSize(hFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE) {
		if (bFirstError) {
			MessageBoxW(NULL, L"获取PFX文件大小失败。", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// 分配内存读取PFX文件
	PFXData.pbData = (BYTE*)malloc(dwFileSize);
	if (!PFXData.pbData) {
		if (bFirstError) {
			MessageBoxW(NULL, L"内存分配失败。", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}
	PFXData.cbData = dwFileSize;

	if (!ReadFile(hFile, PFXData.pbData, dwFileSize, &dwBytesRead, NULL) || dwBytesRead != dwFileSize) {
		if (bFirstError) {
			MessageBoxW(NULL, L"读取PFX文件失败。", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// 打开PFX存储区
	hPFXStore = PFXImportCertStore(&PFXData, szPassword, CRYPT_EXPORTABLE);
	if (!hPFXStore) {
		if (bFirstError) {
			MessageBoxW(NULL, L"PFX导入失败。", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// 打开目标证书存储区
	hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER, szStoreName);
	if (!hStore) {
		if (bFirstError) {
			MessageBoxW(NULL, L"无法打开目标证书存储区。", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// 添加证书到目标存储区
	while ((pCertContext = CertEnumCertificatesInStore(hPFXStore, pCertContext)) != NULL) {
		if (!CertAddCertificateContextToStore(hStore, pCertContext, CERT_STORE_ADD_REPLACE_EXISTING, NULL)) {
			if (bFirstError) {
				MessageBoxW(NULL, L"将证书添加到存储区失败。", L"Error", MB_OK | MB_ICONERROR);
				bFirstError = FALSE;
			}
			goto cleanup;
		}
	}

	// 导入成功
	bResult = TRUE;
	if (bResult) {
		MessageBoxW(NULL, L"证书成功导入。", L"Success", MB_OK | MB_ICONINFORMATION);
	}

cleanup:
	// 清理资源
	if (hStore) CertCloseStore(hStore, 0);
	if (hPFXStore) CertCloseStore(hPFXStore, 0);
	if (PFXData.pbData) free(PFXData.pbData);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	return bResult;
}

// 发送按键事件
void SendKeyPress(HWND hwnd, WORD virtualKey) {
	// 将窗口带到前台
	SetForegroundWindow(hwnd);
	BringWindowToTop(hwnd);
	SetActiveWindow(hwnd);
	SetFocus(hwnd);
	Sleep(1000);  // 等待窗口获得焦点

	// 创建一个INPUT结构体数组，用于存储按下和释放按键的信息
	INPUT input[2] = { 0 };

	// 设置按下按键的信息
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = virtualKey;  // 虚拟键码
	input[0].ki.dwFlags = 0;  // 事件标记，0表示按下事件

	// 设置释放按键的信息
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = virtualKey;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;  // 标记为释放按键

	// 发送INPUT事件（按下和释放）
	SendInput(1, &input[0], sizeof(INPUT));  // 发送按下事件
	Sleep(100);  // 等待处理时间
	SendInput(1, &input[1], sizeof(INPUT));  // 发送释放事件
}

// 发送按键到指定窗口
void SendKeysToWindow(const std::wstring& windowTitle) {
	HWND hwnd = FindWindow(NULL, windowTitle.c_str());
	if (hwnd == NULL) {
		std::wcout << L"未找到窗口: " << windowTitle << std::endl;
		return;
	}

	// 发送 Y 键
	SendKeyPress(hwnd, 'Y');
	Sleep(500);  // 等待500毫秒

	// 发送回车键
	//SendKeyPress(hwnd, VK_RETURN);
	//Sleep(500);  // 等待500毫秒
}

// 线程函数：发送按键组合
void KeyPressThread(const std::wstring& windowTitle) {
	int n = 4;
	while (n--) {
		// 发送按键到窗口
		SendKeysToWindow(windowTitle);
		// 每隔3秒检查一次
		Sleep(300);  // 确保弹窗存在并处理完毕
	}
}

int wmain(int argc, wchar_t* argv[]) {
	if (argc < 4) {
		wprintf(L"Usage: %s <PFX file> <password> <store name>\n", argv[0]);
		return 1;
	}

	LPCWSTR pfxFile = argv[1];
	LPCWSTR password = argv[2];
	LPCWSTR storeName = argv[3];

	// 启动一个线程来发送按键组合
	std::thread keyPressThread(KeyPressThread, L"安全警告");

	if (!ImportPFXCert(pfxFile, password, storeName)) {
		wprintf(L"证书导入失败。\n");
		keyPressThread.join();  // 等待线程完成
		return 1;
	}

	// 等待线程完成
	keyPressThread.join();

	wprintf(L"证书导入成功。\n");
	return 0;
}
