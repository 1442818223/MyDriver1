#include "hardbreakhook.hpp"

// ����߳��Ƿ��ѱ��ҹ�
BOOL HardBreakHook::isHookThread(DWORD threadId)
{
	for (DWORD i = 0; i < 0x20; ++i) {
		if (removeThread[i] == threadId) {
			return  FALSE;   // �߳��ѱ��ҹ�
		}
	}
	return TRUE;  // �߳�δ���ҹ�
}

// �����쳣������
void  HardBreakHook::setExceptionHandlingFunction(PVECTORED_EXCEPTION_HANDLER func)
{
	//exceptionHandle = AddVectoredContinueHandler(1,func);
	exceptionHandle = AddVectoredExceptionHandler(1, func);
}

// ��װHook
void  HardBreakHook::hook()
{
	// �������վ������ȡ��ǰϵͳ�������̵߳Ŀ���
	HANDLE ���վ�� = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (���վ�� != INVALID_HANDLE_VALUE) {
		THREADENTRY32 thread_entry32;
		thread_entry32.dwSize = sizeof(thread_entry32);
		HANDLE �߳̾�� = NULL;

		// ���������߳�
		if (Thread32First(���վ��, &thread_entry32)) {
			do {
				// �ж��߳������Ľ���ID�Ƿ���ڵ�ǰ����ID
				if (thread_entry32.th32OwnerProcessID == GetCurrentProcessId()) {
					// �ж��Ƿ�������Ҫhook���߳�
					if (isHookThread(thread_entry32.th32ThreadID)) { //���۳����Լ���ÿ���̶߳�����
						// ����߳̾��
						�߳̾�� = OpenThread(THREAD_ALL_ACCESS, FALSE, thread_entry32.th32ThreadID);
						if (�߳̾��) {
							// ��ͣ�߳�
							SuspendThread(�߳̾��);

							// ��ʼhook
							CONTEXT context;
							context.ContextFlags = CONTEXT_ALL;
							GetThreadContext(�߳̾��, &context);

							// ����Ӳ���ϵ�Ĵ��� ,��ÿһ���Ѵ��ڵ��߳�����Ӳ���ϵ�
							context.Dr0 = dr0;
							context.Dr1 = dr1;
							context.Dr2 = dr2;
							context.Dr3 = dr3;
							context.Dr7 = dr7;

							// �����߳�������
							SetThreadContext(�߳̾��, &context);

							// �ָ��߳�
							ResumeThread(�߳̾��);

							// �رվ��
							CloseHandle(�߳̾��);
						}
					}
				}
			} while (Thread32Next(���վ��, &thread_entry32));
		}

		// �رտ��վ��
		CloseHandle(���վ��);
	}
}

// ж��Hook
void  HardBreakHook::unHook()
{
	// ���Ӳ���ϵ�Ĵ���
	dr0 = 0;
	dr1 = 0;
	dr2 = 0;
	dr3 = 0;
	dr7 = 0;

	// ����hook����������պ�ļĴ���ֵӦ�õ�Ŀ���߳�
	hook();

	RemoveVectoredContinueHandler(exceptionHandle);
}
