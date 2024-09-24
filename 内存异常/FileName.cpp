#include <windows.h>
#include <stdio.h>

// 全局指针存储异常处理函数
PVOID vehHandle = NULL;
ULONG_PTR HookAddr = 0xDEADBEEF; // 需要捕获的地址

// 自定义异常处理函数
LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
		// 判断是否是我们要监控的地址
		ULONG_PTR accessAddress = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
		if (accessAddress == HookAddr) {
			printf("[HOOK] 捕获到读写异常，地址：0x%08X\n", HookAddr);
			system("pause");
			// 可以在此处进行自定义处理
			// 比如修改值或者做其他操作
	// 临时修改内存保护，以避免再次触发异常
			DWORD oldProtect;
			VirtualProtect((LPVOID)HookAddr, sizeof(ULONG32), PAGE_EXECUTE_READWRITE, &oldProtect);

			// 修改地址的值
			//*(ULONG32*)HookAddr = 9000;

			// 恢复内存保护
			VirtualProtect((LPVOID)HookAddr, sizeof(ULONG32), oldProtect, &oldProtect);

			// 跳过导致异常的指令，假设是一个写入指令，占用了4字节长度
#ifdef _WIN64
			ExceptionInfo->ContextRecord->Rip +=6;  // 64位系统
#else
			ExceptionInfo->ContextRecord->Eip += 4;  // 32位系统
#endif

			// 返回 CONTINUE_EXECUTION 继续执行
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	// 如果不是我们要处理的异常，交由系统处理
	return EXCEPTION_CONTINUE_SEARCH;
}

void SetHook() {
	// 添加向量化异常处理程序
	vehHandle = AddVectoredExceptionHandler(1, VectoredHandler);
	if (vehHandle != NULL) {
		printf("已成功添加 VEH 异常处理程序！\n");
		system("pause");
	}
	else {
		printf("添加 VEH 异常处理程序失败！\n");
		system("pause");
	}
}

void RemoveHook() {
	// 删除异常处理程序
	if (vehHandle != NULL) {
		RemoveVectoredExceptionHandler(vehHandle);
		printf("已成功移除 VEH 异常处理程序！\n");
		system("pause");
	}
}

int main() {
	SetHook();

	// 这里是需要模拟读写异常的代码块
	// 尝试读取或写入 HookAddr
	__try {
		*(ULONG32*)HookAddr = 0x12345678;  // 触发访问异常 //反汇编显示6字节
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		printf("发生异常，已通过 VEH 处理！\n");
		system("pause");
	}
	system("pause");
	// 移除 hook
	RemoveHook();

	return 0;
}
