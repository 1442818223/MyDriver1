#include <sapi.h>
#include <tchar.h>

int main() {
	// ��ʼ�� COM ��
	if (FAILED(::CoInitialize(NULL))) {
		return -1;
	}

	// ���� SAPI ��������
	ISpVoice* pVoice = NULL;
	if (FAILED(::CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice))) {
		::CoUninitialize();
		return -1;
	}

	// �������
	pVoice->Speak(L"�����ɹ�", 0, NULL);

	// �ͷ���Դ
	pVoice->Release();
	::CoUninitialize();

	return 0;
}
