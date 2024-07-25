#include <sapi.h>
#include <tchar.h>

int main() {
	// 初始化 COM 库
	if (FAILED(::CoInitialize(NULL))) {
		return -1;
	}

	// 创建 SAPI 语音对象
	ISpVoice* pVoice = NULL;
	if (FAILED(::CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice))) {
		::CoUninitialize();
		return -1;
	}

	// 语音输出
	pVoice->Speak(L"开启成功", 0, NULL);

	// 释放资源
	pVoice->Release();
	::CoUninitialize();

	return 0;
}
