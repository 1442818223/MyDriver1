#include <windows.h>
#include <stdio.h>

// ���� SEH �ṹ��
typedef struct ExceptionSehInfo {
	struct ExceptionSehInfo* Next;
	PVOID Handler;
} ExceptionSehInfo;
 
// �Զ��� SEH �������
EXCEPTION_DISPOSITION SehHandler(EXCEPTION_RECORD* pException, ExceptionSehInfo* pFrame, CONTEXT* pContext, ExceptionSehInfo* pDispatchContext) {
	MessageBoxA(NULL, "SehHandler Invoked", "Exception", MB_OK);

	// ���� EIP ���������ܵ�����ָ���������㣩
	pContext->Eip += 2;

	// ���߲���ϵͳ����ִ��
	return ExceptionContinueExecution;
}

void testxxxx() {

	//__try  catch 

	ExceptionSehInfo node = { 0 };   // ��ʼ�� SEH �ڵ�
	ExceptionSehInfo* pre = NULL;

	// �� SEH �ڵ����ӵ� SEH ������
	__asm {
		mov eax, fs: [0]        // ��ȡ SEH �����ͷ
		mov pre, eax           // ����ԭ���� SEH ��ͷ
	}

	node.Next = pre;            // ���õ�ǰ�ڵ����һ���ڵ�Ϊԭ��������ͷ
	node.Handler = SehHandler;  // �����Զ����쳣�������

	// ����ǰ�ڵ����ӵ� SEH ������
	__asm {
		lea eax, node          // ��ȡ��ǰ�ڵ�ĵ�ַ
		mov fs : [0] , eax        // ����ǰ�ڵ�����Ϊ�µ� SE,H ����ͷ
	}

	// �����������쳣
	int i = 0;
	int x = i / 0;              // ���������쳣

	printf("This won't be printed\n");

	// �ָ�ԭ���� SEH ����
	__asm {
		mov eax, pre           // �ָ�ԭ���� SEH ��ͷ
		mov fs : [0] , eax        // ����Ϊ SEH �����ͷ
	}
}

int main() {
	testxxxx();
	system("pause");
	return 0;
}
