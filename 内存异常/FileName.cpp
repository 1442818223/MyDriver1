#include <windows.h>
#include <stdio.h>

// ȫ��ָ��洢�쳣������
PVOID vehHandle = NULL;
ULONG_PTR HookAddr = 0xDEADBEEF; // ��Ҫ����ĵ�ַ

// �Զ����쳣������
LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
		// �ж��Ƿ�������Ҫ��صĵ�ַ
		ULONG_PTR accessAddress = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
		if (accessAddress == HookAddr) {
			printf("[HOOK] ���񵽶�д�쳣����ַ��0x%08X\n", HookAddr);
			system("pause");
			// �����ڴ˴������Զ��崦��
			// �����޸�ֵ��������������
	// ��ʱ�޸��ڴ汣�����Ա����ٴδ����쳣
			DWORD oldProtect;
			VirtualProtect((LPVOID)HookAddr, sizeof(ULONG32), PAGE_EXECUTE_READWRITE, &oldProtect);

			// �޸ĵ�ַ��ֵ
			//*(ULONG32*)HookAddr = 9000;

			// �ָ��ڴ汣��
			VirtualProtect((LPVOID)HookAddr, sizeof(ULONG32), oldProtect, &oldProtect);

			// ���������쳣��ָ�������һ��д��ָ�ռ����4�ֽڳ���
#ifdef _WIN64
			ExceptionInfo->ContextRecord->Rip +=6;  // 64λϵͳ
#else
			ExceptionInfo->ContextRecord->Eip += 4;  // 32λϵͳ
#endif

			// ���� CONTINUE_EXECUTION ����ִ��
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	// �����������Ҫ������쳣������ϵͳ����
	return EXCEPTION_CONTINUE_SEARCH;
}

void SetHook() {
	// ����������쳣�������
	vehHandle = AddVectoredExceptionHandler(1, VectoredHandler);
	if (vehHandle != NULL) {
		printf("�ѳɹ���� VEH �쳣�������\n");
		system("pause");
	}
	else {
		printf("��� VEH �쳣�������ʧ�ܣ�\n");
		system("pause");
	}
}

void RemoveHook() {
	// ɾ���쳣�������
	if (vehHandle != NULL) {
		RemoveVectoredExceptionHandler(vehHandle);
		printf("�ѳɹ��Ƴ� VEH �쳣�������\n");
		system("pause");
	}
}

int main() {
	SetHook();

	// ��������Ҫģ���д�쳣�Ĵ����
	// ���Զ�ȡ��д�� HookAddr
	__try {
		*(ULONG32*)HookAddr = 0x12345678;  // ���������쳣 //�������ʾ6�ֽ�
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		printf("�����쳣����ͨ�� VEH ����\n");
		system("pause");
	}
	system("pause");
	// �Ƴ� hook
	RemoveHook();

	return 0;
}
