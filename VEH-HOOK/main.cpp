#include <stdio.h>
#include "HardBreakHook.hpp"

HardBreakHook g_hbHook;



LONG NTAPI exceptionHandle(struct _EXCEPTION_POINTERS* �쳣��Ϣ) {
	// �ж��Ƿ��ǵ����쳣��Ӳ���ϵ㴥�����쳣��
	if (�쳣��Ϣ->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {
		// �ж��Ƿ��������趨�Ķϵ��ַ�������쳣
		if ((uintptr_t)�쳣��Ϣ->ExceptionRecord->ExceptionAddress == HookAddr) {
			// �������������⴦������ϣ�����쳣�����߼�
			// ...





			//��ԭ����
			//1,��ԭ�����߼�дһ��
			//2,����������ĳ���

			// �����ǵ��쳣��������Ϻ��������
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	// �����������Ҫ������쳣������ϵͳ���������������
	return EXCEPTION_CONTINUE_SEARCH;
}



int main() {


	g_hbHook.dr0 = HookAddr;
	g_hbHook.dr7 = 0x55;
	g_hbHook.removeThread[0] = GetCurrentThreadId();  //���̲߳�����VEH HOOK ��ֹ���� �ų����Լ����߳�
	g_hbHook.setExceptionHandlingFunction(exceptionHandle);
	g_hbHook.hook();


	//g_hbHook.unHook();
	return 0;
}