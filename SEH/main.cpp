#include <windows.h>
#include <stdio.h>

// 定义 SEH 结构体
typedef struct ExceptionSehInfo {
	struct ExceptionSehInfo* Next;
	PVOID Handler;
} ExceptionSehInfo;
 
// 自定义 SEH 处理程序
EXCEPTION_DISPOSITION SehHandler(EXCEPTION_RECORD* pException, ExceptionSehInfo* pFrame, CONTEXT* pContext, ExceptionSehInfo* pDispatchContext) {
	MessageBoxA(NULL, "SehHandler Invoked", "Exception", MB_OK);

	// 调整 EIP 以跳过可能的问题指令（例如除以零）
	pContext->Eip += 2;

	// 告诉操作系统继续执行
	return ExceptionContinueExecution;
}

void testxxxx() {

	//__try  catch 

	ExceptionSehInfo node = { 0 };   // 初始化 SEH 节点
	ExceptionSehInfo* pre = NULL;

	// 将 SEH 节点链接到 SEH 链表中
	__asm {
		mov eax, fs: [0]        // 获取 SEH 链表的头
		mov pre, eax           // 保存原来的 SEH 链头
	}

	node.Next = pre;            // 设置当前节点的下一个节点为原来的链表头
	node.Handler = SehHandler;  // 设置自定义异常处理程序

	// 将当前节点链接到 SEH 链表中
	__asm {
		lea eax, node          // 获取当前节点的地址
		mov fs : [0] , eax        // 将当前节点设置为新的 SE,H 链表头
	}

	// 触发除以零异常
	int i = 0;
	int x = i / 0;              // 故意制造异常

	printf("This won't be printed\n");

	// 恢复原来的 SEH 链表
	__asm {
		mov eax, pre           // 恢复原来的 SEH 链头
		mov fs : [0] , eax        // 设置为 SEH 链表的头
	}
}

int main() {
	testxxxx();
	system("pause");
	return 0;
}
