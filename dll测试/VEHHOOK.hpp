#pragma once
#include<windows.h>
#include <TlHelp32.h>
#include <stdio.h>

class HardBreakHook {
public:
	PVOID exceptionHandle = nullptr;   //�쳣������                        
	uintptr_t dr0 = 0;
	uintptr_t dr1 = 0;
	uintptr_t dr2 = 0;
	uintptr_t dr3 = 0;
	uintptr_t dr7 = 0;
	DWORD removeThread[0x20] = { 0 };
	PVECTORED_EXCEPTION_HANDLER exceptionHandleFunction = nullptr;

	// Ĭ�Ϲ��캯��
	HardBreakHook() {}

	// ���ι��캯��
	HardBreakHook(uintptr_t d0, uintptr_t d1, uintptr_t d2, uintptr_t d3, uintptr_t d7,
		PVECTORED_EXCEPTION_HANDLER exceptionHandleFunc, DWORD* remove)
		: dr0(d0), dr1(d1), dr2(d2), dr3(d3), dr7(d7), exceptionHandleFunction(exceptionHandleFunc) {
		memcpy(removeThread, remove, sizeof(remove));
		exceptionHandle = AddVectoredExceptionHandler(1, exceptionHandleFunction);//1���������쳣��������ǰ��
		hook();
	}

	// ����߳��Ƿ��ѱ��ҹ�
	BOOL isHookThread(DWORD threadId);

	// �����쳣������
	void setExceptionHandlingFunction(PVECTORED_EXCEPTION_HANDLER func);

	// ��װHook
	void hook();

	// ж��Hook
	void unHook();
};
