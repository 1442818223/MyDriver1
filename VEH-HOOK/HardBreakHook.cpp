#include "hardbreakhook.hpp"
#include <TlHelp32.h>
// 检查线程是否已被挂钩
BOOL HardBreakHook::isHookThread(DWORD threadId)
{
	for (DWORD i = 0; i < 0x20; ++i) {
		if (removeThread[i] == threadId) {//等于自己的主线程
			return  FALSE;       //有的的话就不hook
		}
	}
	return TRUE;
}

// 设置异常处理函数
void  HardBreakHook::setExceptionHandlingFunction(PVECTORED_EXCEPTION_HANDLER func)
{
	exceptionHandle = AddVectoredContinueHandler(1,func);

	//函数的返回值是一个指向先前注册的处理程序的指针（PVOID 类型）。
	//如果成功注册处理程序，它会返回一个指针，你可以在后续调用 RemoveVectoredContinueHandler 时使用这个指针来注销处理程序。
	//如果函数失败，返回值将是 NULL。
}

// 安装Hook
void  HardBreakHook::hook()
{
	// 创建快照句柄，获取系统中所有进程的线程快照
	HANDLE 快照句柄 = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (快照句柄 != INVALID_HANDLE_VALUE) {
		THREADENTRY32 thread_entry32;
		thread_entry32.dwSize = sizeof(thread_entry32);
		HANDLE 线程句柄 = NULL;

		// 遍历所有线程
		if (Thread32First(快照句柄, &thread_entry32)) {
			do {
				// 判断线程所属的进程ID是否等于当前进程ID
				 // thread_entry32.th32OwnerProcessID通过线程拿进程id
				if (thread_entry32.th32OwnerProcessID == GetCurrentProcessId()) {
					// 判断是否是我们要hook的线程
					if (isHookThread(thread_entry32.th32ThreadID)) {//如果是我们的主进程线程ID就排除
						// 获得线程句柄
						线程句柄 = OpenThread(THREAD_ALL_ACCESS, FALSE, thread_entry32.th32ThreadID);
						if (线程句柄) {
							// 暂停线程
							SuspendThread(线程句柄);

							// 开始hook
							CONTEXT context;
							context.ContextFlags = CONTEXT_ALL;//表示你希望获取所有上下文信息，包括寄存器、程序计数器（PC）、堆栈指针等。
							GetThreadContext(线程句柄, &context);

							// 设置硬件断点寄存器
							context.Dr0 = dr0;
							context.Dr1 = dr1;
							context.Dr2 = dr2;
							context.Dr3 = dr3;
							context.Dr7 = dr7;

							// 设置线程上下文
							SetThreadContext(线程句柄, &context);

							// 恢复线程
							ResumeThread(线程句柄);

							// 关闭句柄
							CloseHandle(线程句柄);
						}
					}
				}
			} while (Thread32Next(快照句柄, &thread_entry32));
		}

		// 关闭快照句柄
		CloseHandle(快照句柄);
	}
}

// 卸载Hook
void  HardBreakHook::unHook()
{
	// 清空硬件断点寄存器
	dr0 = 0;
	dr1 = 0;
	dr2 = 0;
	dr3 = 0;
	dr7 = 0;

	// 调用hook函数，将清空后的寄存器值应用到目标线程
	hook();

	RemoveVectoredContinueHandler(exceptionHandle);
}
