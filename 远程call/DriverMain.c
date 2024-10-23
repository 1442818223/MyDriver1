#include <ntifs.h>

#include "FarCall.h"
#include "FarCall.h"
#include "Search.h"
#include "ExportFunc.h"
#include "tools.h"



VOID DriverUnload(PDRIVER_OBJECT pDriver)
{


}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	//DbgBreakPoint();
	//ULONG_PTR moudleBase = GetModuleR3(1332, "kernel32.dll", NULL);
	
	
	HANDLE pid = 7640;
	//x64
	char code[] =
	{  
		             0x31, 0xC9,                                                      //xor rcx, rcx
		             0x31, 0xD2,                                                     //xor rdx, rdx
		             0x4D, 0x31, 0xC0,                                               // xor r8, r8
		             0x4D, 0x31, 0xC9,                                               //xor r9, r9
		             0x48, 0xB8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,     // mov rax, 0x1122334455667788
		             0x48, 0x81, 0xEC, 0xA8, 0x00, 0x00, 0x00,                       //sub rsp, 0xA8  这里写a0会没效果 卸载时还会蓝屏 应为栈要对齐 下面还多了一个call相当于push 8  所以上是a8而不是a0
		             0xFF, 0xD0,                                                      //call rax
		             0x48, 0x81, 0xC4, 0xA8, 0x00, 0x00, 0x00,                        //add rsp, 0xA8
		             0xC3 
	};                                                          //ret
	*(PULONG64)&code[12] = 0x7FFC4D3EA9D0;    //    MessageBoxA
	 
	//x32
	//char code[] =
	//{  
 //     0x6A , 0x00 ,
	//  0x6A , 0x00 ,
	//  0x6A , 0x00 ,
	//  0x6A , 0x00 ,   
	//  0x83, 0xEC, 0x44,
	//  0xFF , 0x15 , 0x40 , 0x20 , 0x55 , 0x00,
	//  0x83, 0xC4, 0x44
	//};                                                          //ret
	//*(PULONG64)&code[10] = 0x740780B0;    //    MessageBoxA
	


	RemoteCall(pid, code,sizeof(code));

	//DbgBreakPoint();
	//1.直接读写 memcpy   直接切CR3 不附加了  调用附加函数

	//隐藏内存的时候 不会蓝屏
	//2.MmCopyVirtualMemory  ReadProcessMemory

	//3.MDL

	//4.直接映射物理页  第一个效率慢，第二个换页的时候 不是很精确，必须做跨页处理
	// 效率慢的原因 每层地址都自己转化 2 9 9 12   cr3 PDE PTE  10 10 12 2  9 9 9 9 12 4
	

	//5.APC读写 

	//6.队列读写   DXF system->线程下 父进程

	//DXF WIN7 0E异常 验证进程的白名单   PTE

	//CF  替换PML4 中 PDPTE

	//X64 CF 注入的  CR3 VAD  SXG // 

	//VT

	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}