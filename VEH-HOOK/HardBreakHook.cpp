#include "hardbreakhook.hpp"
#include <TlHelp32.h>
// ����߳��Ƿ��ѱ��ҹ�
BOOL HardBreakHook::isHookThread(DWORD threadId)
{
	for (DWORD i = 0; i < 0x20; ++i) {
		if (removeThread[i] == threadId) {//�����Լ������߳�
			return  FALSE;       //�еĵĻ��Ͳ�hook
		}
	}
	return TRUE;
}

// �����쳣������
void  HardBreakHook::setExceptionHandlingFunction(PVECTORED_EXCEPTION_HANDLER func)
{
	exceptionHandle = AddVectoredContinueHandler(1,func);

	//�����ķ���ֵ��һ��ָ����ǰע��Ĵ�������ָ�루PVOID ���ͣ���
	//����ɹ�ע�ᴦ��������᷵��һ��ָ�룬������ں������� RemoveVectoredContinueHandler ʱʹ�����ָ����ע���������
	//�������ʧ�ܣ�����ֵ���� NULL��
}

// ��װHook
void  HardBreakHook::hook()
{
	// �������վ������ȡϵͳ�����н��̵��߳̿���
	HANDLE ���վ�� = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (���վ�� != INVALID_HANDLE_VALUE) {
		THREADENTRY32 thread_entry32;
		thread_entry32.dwSize = sizeof(thread_entry32);
		HANDLE �߳̾�� = NULL;

		// ���������߳�
		if (Thread32First(���վ��, &thread_entry32)) {
			do {
				// �ж��߳������Ľ���ID�Ƿ���ڵ�ǰ����ID
				 // thread_entry32.th32OwnerProcessIDͨ���߳��ý���id
				if (thread_entry32.th32OwnerProcessID == GetCurrentProcessId()) {
					// �ж��Ƿ�������Ҫhook���߳�
					if (isHookThread(thread_entry32.th32ThreadID)) {//��������ǵ��������߳�ID���ų�
						// ����߳̾��
						�߳̾�� = OpenThread(THREAD_ALL_ACCESS, FALSE, thread_entry32.th32ThreadID);
						if (�߳̾��) {
							// ��ͣ�߳�
							SuspendThread(�߳̾��);

							// ��ʼhook
							CONTEXT context;
							context.ContextFlags = CONTEXT_ALL;//��ʾ��ϣ����ȡ������������Ϣ�������Ĵ����������������PC������ջָ��ȡ�
							GetThreadContext(�߳̾��, &context);

							// ����Ӳ���ϵ�Ĵ���
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
