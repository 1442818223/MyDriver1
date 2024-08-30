#include <windows.h>
#include <wincrypt.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

// ��������
BOOL ImportPFXCert(LPCWSTR szPFXFilePath, LPCWSTR szPassword, LPCWSTR szStoreName);
void SendKeyPress(HWND hwnd, WORD virtualKey);
void SendKeysToWindow(const std::wstring& windowTitle);
void KeyPressThread(const std::wstring& windowTitle);

// ����֤��
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

	// ��PFX�ļ�
	hFile = CreateFileW(szPFXFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		if (bFirstError) {
			MessageBoxW(NULL, L"�޷���PFX�ļ���", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// ��ȡPFX�ļ���С
	dwFileSize = GetFileSize(hFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE) {
		if (bFirstError) {
			MessageBoxW(NULL, L"��ȡPFX�ļ���Сʧ�ܡ�", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// �����ڴ��ȡPFX�ļ�
	PFXData.pbData = (BYTE*)malloc(dwFileSize);
	if (!PFXData.pbData) {
		if (bFirstError) {
			MessageBoxW(NULL, L"�ڴ����ʧ�ܡ�", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}
	PFXData.cbData = dwFileSize;

	if (!ReadFile(hFile, PFXData.pbData, dwFileSize, &dwBytesRead, NULL) || dwBytesRead != dwFileSize) {
		if (bFirstError) {
			MessageBoxW(NULL, L"��ȡPFX�ļ�ʧ�ܡ�", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// ��PFX�洢��
	hPFXStore = PFXImportCertStore(&PFXData, szPassword, CRYPT_EXPORTABLE);
	if (!hPFXStore) {
		if (bFirstError) {
			MessageBoxW(NULL, L"PFX����ʧ�ܡ�", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// ��Ŀ��֤��洢��
	hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER, szStoreName);
	if (!hStore) {
		if (bFirstError) {
			MessageBoxW(NULL, L"�޷���Ŀ��֤��洢����", L"Error", MB_OK | MB_ICONERROR);
			bFirstError = FALSE;
		}
		goto cleanup;
	}

	// ���֤�鵽Ŀ��洢��
	while ((pCertContext = CertEnumCertificatesInStore(hPFXStore, pCertContext)) != NULL) {
		if (!CertAddCertificateContextToStore(hStore, pCertContext, CERT_STORE_ADD_REPLACE_EXISTING, NULL)) {
			if (bFirstError) {
				MessageBoxW(NULL, L"��֤����ӵ��洢��ʧ�ܡ�", L"Error", MB_OK | MB_ICONERROR);
				bFirstError = FALSE;
			}
			goto cleanup;
		}
	}

	// ����ɹ�
	bResult = TRUE;
	if (bResult) {
		MessageBoxW(NULL, L"֤��ɹ����롣", L"Success", MB_OK | MB_ICONINFORMATION);
	}

cleanup:
	// ������Դ
	if (hStore) CertCloseStore(hStore, 0);
	if (hPFXStore) CertCloseStore(hPFXStore, 0);
	if (PFXData.pbData) free(PFXData.pbData);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	return bResult;
}

// ���Ͱ����¼�
void SendKeyPress(HWND hwnd, WORD virtualKey) {
	// �����ڴ���ǰ̨
	SetForegroundWindow(hwnd);
	BringWindowToTop(hwnd);
	SetActiveWindow(hwnd);
	SetFocus(hwnd);
	Sleep(1000);  // �ȴ����ڻ�ý���

	// ����һ��INPUT�ṹ�����飬���ڴ洢���º��ͷŰ�������Ϣ
	INPUT input[2] = { 0 };

	// ���ð��°�������Ϣ
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = virtualKey;  // �������
	input[0].ki.dwFlags = 0;  // �¼���ǣ�0��ʾ�����¼�

	// �����ͷŰ�������Ϣ
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = virtualKey;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;  // ���Ϊ�ͷŰ���

	// ����INPUT�¼������º��ͷţ�
	SendInput(1, &input[0], sizeof(INPUT));  // ���Ͱ����¼�
	Sleep(100);  // �ȴ�����ʱ��
	SendInput(1, &input[1], sizeof(INPUT));  // �����ͷ��¼�
}

// ���Ͱ�����ָ������
void SendKeysToWindow(const std::wstring& windowTitle) {
	HWND hwnd = FindWindow(NULL, windowTitle.c_str());
	if (hwnd == NULL) {
		std::wcout << L"δ�ҵ�����: " << windowTitle << std::endl;
		return;
	}

	// ���� Y ��
	SendKeyPress(hwnd, 'Y');
	Sleep(500);  // �ȴ�500����

	// ���ͻس���
	//SendKeyPress(hwnd, VK_RETURN);
	//Sleep(500);  // �ȴ�500����
}

// �̺߳��������Ͱ������
void KeyPressThread(const std::wstring& windowTitle) {
	int n = 4;
	while (n--) {
		// ���Ͱ���������
		SendKeysToWindow(windowTitle);
		// ÿ��3����һ��
		Sleep(300);  // ȷ���������ڲ��������
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

	// ����һ���߳������Ͱ������
	std::thread keyPressThread(KeyPressThread, L"��ȫ����");

	if (!ImportPFXCert(pfxFile, password, storeName)) {
		wprintf(L"֤�鵼��ʧ�ܡ�\n");
		keyPressThread.join();  // �ȴ��߳����
		return 1;
	}

	// �ȴ��߳����
	keyPressThread.join();

	wprintf(L"֤�鵼��ɹ���\n");
	return 0;
}
