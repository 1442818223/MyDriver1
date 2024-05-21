#include <windows.h>
#include <stdio.h>

int main() {
	/*
	HANDLE CreateSemaphoreA(
	LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,// 默认安全属性
	LONG lInitialCount,// 初始计数 (信号量对象的初始计数值。该值必须大于或等于零，并且小于或等于 lMaximumCount。当信号量的计数值大于零时，其状态为有信号；当其计数值为零时，其状态为无信号。)
	LONG lMaximumCount,// 最大计数
	LPCSTR lpName// 命名信号量
    );
	*/

	
	HANDLE hSemaphoreA = CreateSemaphoreA(NULL, 2, 10, NULL);     //参数二设置了2之后后面的操作每次都会增加或者减少2   0或1都是1
	if (hSemaphoreA == NULL) {
		printf("CreateSemaphoreA error: %d\n", GetLastError());
		return 1;
	}


	/*
  BOOL ReleaseSemaphore(
  HANDLE hSemaphore,  // 信号量句柄
  LONG lReleaseCount, // 增加计数的数量
  LPLONG lpPreviousCount // 指向接收 以前 计数值的变量的指针（可以为 NULL）  (该变量接收信号量的前一个计数值。如果此参数为 NULL，则不返回前一个计数值。)
);
	*/
	LONG pre = 0;
	LONG prel = 0;

	// 释放信号量，增加计数
	if (!ReleaseSemaphore(hSemaphoreA, 2, &pre)) {         //参数二只是一个计数器,参数2乘以CreateSemaphoreA的参数二
		printf("ReleaseSemaphore error: %d\n", GetLastError());
		CloseHandle(hSemaphoreA);
		return 1;
	}

	if (!ReleaseSemaphore(hSemaphoreA, 1, &prel)) {       
		printf("ReleaseSemaphore error: %d\n", GetLastError());
		CloseHandle(hSemaphoreA);
		return 1;
	}

	// 打印结果
	printf("Semaphore Handle: %p, Previous Count 1: %ld, Previous Count 2: %ld\n",
		hSemaphoreA, pre, prel);

	// 关闭信号量句柄
	CloseHandle(hSemaphoreA);
	


	return 0;
}
