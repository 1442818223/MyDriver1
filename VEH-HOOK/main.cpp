#include <stdio.h>
#include "HardBreakHook.hpp"

HardBreakHook g_hbHook;



LONG NTAPI exceptionHandle(struct _EXCEPTION_POINTERS* 异常信息) {
	// 判断是否是单步异常（硬件断点触发的异常）
	if (异常信息->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {
		// 判断是否是我们设定的断点地址触发的异常
		if ((uintptr_t)异常信息->ExceptionRecord->ExceptionAddress == HookAddr) {
			// 可以在这里任意处理我们希望的异常处理逻辑
			// ...





			//还原处理
			//1,把原来的逻辑写一下
			//2,跳过这句代码的长度

			// 是我们的异常，处理完毕后继续运行
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}

	// 如果不是我们要处理的异常，交由系统或其他处理程序处理
	return EXCEPTION_CONTINUE_SEARCH;
}



int main() {


	g_hbHook.dr0 = HookAddr;
	g_hbHook.dr7 = 0x55;
	g_hbHook.removeThread[0] = GetCurrentThreadId();  //主线程不能用VEH HOOK 防止死锁 排除掉自己的线程
	g_hbHook.setExceptionHandlingFunction(exceptionHandle);
	g_hbHook.hook();


	//g_hbHook.unHook();
	return 0;
}