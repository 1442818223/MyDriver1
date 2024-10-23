#include "FarCall.h"
#include "Search.h"
#include "ExportFunc.h"
#include "tools.h"

typedef struct _FreeMemoryInfo 
{
	WORK_QUEUE_ITEM workitem;
	HANDLE pid;
	ULONG64 IsExecuteAddr;
	ULONG64 freeSize;

}FreeMemoryInfo,*PFreeMemoryInfo;

//NTSTATUS NTAPI NtGetNextThread(  //Win11是导出的
//	__in HANDLE ProcessHandle,
//	__in HANDLE ThreadHandle,   //获取的第一个线程就是主线程
//	__in ACCESS_MASK DesiredAccess,
//	__in ULONG HandleAttributes,
//	__in ULONG Flags,
//	__out PHANDLE NewThreadHandle
//)
//{
//
//	typedef NTSTATUS(NTAPI* ZwGetNextThreadProc)(      //用于枚举系统中某个进程的线程
//		__in HANDLE ProcessHandle,
//		__in HANDLE ThreadHandle,
//		__in ACCESS_MASK DesiredAccess,
//		__in ULONG HandleAttributes,
//		__in ULONG Flags,
//		__out PHANDLE NewThreadHandle
//		);
//	*static ZwGetNextThreadProc ZwGetNextThreadFunc = NULL;
//	if (!ZwGetNextThreadFunc)
//	{
//		UNICODE_STRING unName = { 0 };
//		RtlInitUnicodeString(&unName, L"ZwGetNextThread");
//		ZwGetNextThreadFunc = (ZwGetNextThreadProc)MmGetSystemRoutineAddress(&unName);
//		if (!ZwGetNextThreadFunc)
//		{
//			UNICODE_STRING unName = { 0 };
//			RtlInitUnicodeString(&unName, L"ZwGetNotificationResourceManager");
//			PUCHAR ZwGetNotificationResourceManagerAddr = (PUCHAR)MmGetSystemRoutineAddress(&unName);
//			ZwGetNotificationResourceManagerAddr -= 0x50;
//			for (int i = 0; i < 0x30; i++)
//			{
//				if (ZwGetNotificationResourceManagerAddr[i] == 0x48
//					&& ZwGetNotificationResourceManagerAddr[i + 1] == 0x8B
//					&& ZwGetNotificationResourceManagerAddr[i + 2] == 0xC4)
//				{
//					ZwGetNextThreadFunc = ZwGetNotificationResourceManagerAddr + i;
//					break;
//				}
//			}
//		}
//	}
//
//	if (ZwGetNextThreadFunc)
//	{
//		return ZwGetNextThreadFunc(ProcessHandle, ThreadHandle, DesiredAccess,
//			HandleAttributes, Flags, NewThreadHandle);
//	}*/
//
//	return STATUS_UNSUCCESSFUL;
//}


EXTERN_C NTSTATUS NTAPI ZwGetNextThread(      //用于枚举系统中某个进程的线程
	__in HANDLE ProcessHandle,
	__in HANDLE ThreadHandle,
	__in ACCESS_MASK DesiredAccess,
	__in ULONG HandleAttributes,
	__in ULONG Flags,
	__out PHANDLE NewThreadHandle
	);

PETHREAD NtGetProcessMainThread(PEPROCESS Process)
{
	PETHREAD ethread = NULL;
	
	KAPC_STATE kApcState = { 0 };
	
	KeStackAttachProcess(Process, &kApcState);
	
	HANDLE hThread = NULL;
	                                                     //NULL就是第一个线程,有值就是当前线程句柄的下一个
	NTSTATUS status = ZwGetNextThread(NtCurrentProcess(), NULL, THREAD_ALL_ACCESS,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, &hThread);

	if (NT_SUCCESS(status))
	{

		status = ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS,
			*PsThreadType, KernelMode, &ethread, NULL);
		NtClose(hThread);

		if (!NT_SUCCESS(status))
		{
			ethread = NULL;
		}
	}


	KeUnstackDetachProcess(&kApcState);
	return ethread;
}

//线程挂起
NTSTATUS PsSuspendThread(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL)
{
	typedef NTSTATUS (NTAPI *PsSuspendThreadProc)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL);

	static PsSuspendThreadProc PsSuspendThreadFunc = NULL;

	if (!PsSuspendThreadFunc)
	{     //                                                                                /////////////////////////// //llu 是一种后缀，表示这个常量是一个无符号长整数 (unsigned long long) 类型                
		PsSuspendThreadFunc = (PsSuspendThreadProc)searchCode("ntoskrnl.exe", "PAGE", "4883EC304C8BF2488BF9836424200065488B342588010000488974247066FF8EE4010000904C8DB948050000", -0x11llu);
	}

	if (PsSuspendThreadFunc)
	{
		return PsSuspendThreadFunc(Thread, PreviousSuspendCount);
	}

	return STATUS_NOT_IMPLEMENTED;
}

//线程恢复   
NTSTATUS PsResumeThread(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL)
{                   //NTAPI兼容x86  x64无视
	typedef NTSTATUS(NTAPI *PsResumeThreadProc)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL);

	static PsResumeThreadProc PsResumeThreadFunc = NULL;

	if (!PsResumeThreadFunc)
	{
		PsResumeThreadFunc = (PsResumeThreadProc)searchCode("ntoskrnl.exe", "PAGE", "4883EC2841B801000000E8****4883C428C3CC***********488BC44889580848896810488970184889782041564883EC2065488B2C25880100004C8DB158040000488BF1498BCE66FF8DE4010000", 0);
	}

	if (PsResumeThreadFunc)
	{
		return PsResumeThreadFunc(Thread, PreviousSuspendCount);
	}

	return STATUS_NOT_IMPLEMENTED;
}


VOID ExFreeMemoryWorkItem(_In_ PVOID Parameter)
{
	PFreeMemoryInfo item = (PFreeMemoryInfo)Parameter;

	PEPROCESS Process = NULL;

	NTSTATUS st = PsLookupProcessByProcessId(item->pid, &Process);

	if (!NT_SUCCESS(st))
	{
		return ;
	}

	if (PsGetProcessExitStatus(Process) != 0x103)
	{
		ObDereferenceObject(Process);
		return ;
	}

	ULONG64 exeValue = 0;
	SIZE_T pro = 0;
	BOOLEAN isSuccess = FALSE;

	int count = 0;

	while (1)
	{
		if (count > 10000) break;
		NTSTATUS status = MmCopyVirtualMemory(Process, item->IsExecuteAddr, IoGetCurrentProcess(),
                                              &exeValue, 8, KernelMode, &pro);
		if (NT_SUCCESS(status) && exeValue == 1) //看看远程call是否调用成功
		{
			isSuccess = TRUE;
			break;
		}

		KernelSleep(10, FALSE);//10毫秒
		count++;
	}

	
	KAPC_STATE kApcState = { 0 };
	KeStackAttachProcess(Process, &kApcState);

	if (isSuccess)
	{
		PVOID BaseAddr = (PVOID)(item->IsExecuteAddr - 0x500);//释放BaseAddr
		ZwFreeVirtualMemory(NtCurrentProcess(), &BaseAddr, &item->freeSize, MEM_RELEASE);
	}

	KeUnstackDetachProcess(&kApcState);

	ExFreePool(item);
	ObDereferenceObject(Process);
}

BOOLEAN RemoteCall(HANDLE pid, PVOID ShellCode, ULONG shellcodeSize)
{
	PEPROCESS Process = NULL;

	NTSTATUS st = PsLookupProcessByProcessId(pid, &Process);

	if (!NT_SUCCESS(st))
	{
		return FALSE;
	}

	if (PsGetProcessExitStatus(Process) != 0x103)
	{
		ObDereferenceObject(Process);
		return FALSE;
	}
	
	
	PETHREAD pThread = NtGetProcessMainThread(Process);

	PVOID wow64 = PsGetProcessWow64Process(Process);
	
	BOOLEAN isWow64 = wow64 ? TRUE : FALSE;  //判断是不是x64进程

	ObDereferenceObject(Process);

	if (!pThread)
	{
		return FALSE;
	}
	
	PUCHAR kShellcode =  ExAllocatePool(PagedPool, shellcodeSize); //0环申请空间
	memcpy(kShellcode, ShellCode, shellcodeSize);

	KAPC_STATE kApcState = {0};
	KeStackAttachProcess(Process,&kApcState);


	PUCHAR BaseAddr = NULL;
	SIZE_T size = shellcodeSize + PAGE_SIZE;
	do 
	{     //3环申请空间                                                //size大小
		st = ZwAllocateVirtualMemory(NtCurrentProcess(), &BaseAddr, 0, &size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (!NT_SUCCESS(st))
		{
			break;
		}
		
		memset(BaseAddr, 0, size);

		PUCHAR ms = BaseAddr + PAGE_SIZE;  //开头一个页要写保存上下文环境

		memcpy(ms, kShellcode, shellcodeSize);

		st = PsSuspendThread(pThread, NULL);
		
		if (NT_SUCCESS(st))
		{
			if (isWow64)//32位call
			{
				/*
								60              pushad
								B8 78563412     mov eax,12345678
								83EC 40         sub esp,40
								FFD0            call eax                                 ; 测试.<ModuleEntryPoint>
								83C4 40         add esp,40
								B8 78563412     mov eax,12345678
								C700 01000000   mov dword ptr ds:[eax],1
								61              popad
								FF25 00000000   jmp dword ptr ds:[0]
								0000            add byte ptr ds:[eax],al
								0000            add byte ptr ds:[eax],al
								0000            add byte ptr ds:[eax],al
								0000            add byte ptr ds:[eax],al
								0000            add byte ptr ds:[eax],al




				*/

				char bufcode[] = 
				{
					0x60,                                 // pushad
					0xB8, 0x78, 0x56, 0x34, 0x12,         // mov eax,shellcode的位置
					0x83, 0xEC, 0x40,                     // sub esp,40
					0xFF, 0xD0,                           //call eax
					0x83, 0xC4, 0x40,                     // add esp,40
					0xB8, 0x78, 0x56, 0x34, 0x12,         // mov eax,(BaseAddr + 0x500) 
					0xC7, 0x00,	0x01, 0x00, 0x00,0x00,    //mov dword ptr ds:[eax],1
					0x61,                                 //popad
					0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,   //jmp 回去
					0x00, 0x00,
					0x00, 0x00,
					0x00, 0x00,
					0x00, 0x00,
				};

				//x86进程
				PUCHAR teb64 = (PUCHAR)PsGetThreadTeb(pThread);
				//获取wowcontext
				SIZE_T retProc = NULL;    //防缺页
				MmCopyVirtualMemory(Process, (PULONG64)(teb64 + 0x1488), Process, (PULONG64)(teb64 + 0x1488), 8, UserMode, &retProc);

				PUCHAR WowContext = (PUCHAR)*(PULONG64)(teb64 + 0x1488);//0x1480+8 是第二项所以+8
				*(PULONG)&bufcode[2] = ms;
				*(PULONG)&bufcode[15] = (ULONG)(ms + 0x500);
				*(PULONG)&bufcode[32] = *(PULONG)(WowContext + 0xbc); //0x1488+0xb8+4   //返回原来eip地址

				memcpy(BaseAddr, bufcode,sizeof(bufcode));
				//修改
				*(PULONG)(WowContext + 0xbc) = BaseAddr;
			}
			else
			{

				/*
					                                                                  
							50											push  rax                                                                  
							51                                          push  rcx                                                                  
							52                                          push  rdx                                                                  
							53                                          push  rbx                                                                  
							55                                          push  rbp                                                                  
							56                                          push  rsi                                                                  
							57                                          push  rdi                                                                  
							41 50                                       push  r8                                                                   
							41 51                                       push  r9                                                                   
							41 52                                       push  r10                                                                  
							41 53                                       push  r11                                                                  
							41 54                                       push  r12                                                                  
							41 55                                       push  r13                                                                  
							41 56                                       push  r14                                                                  
							41 57                                       push  r15                                                                  
							48 B8 99 89 67 45 23 01 00 00               mov  rax,0x0000012345678999                                                
							48 81 EC A8 00 00 00                        sub  rsp,0x00000000000000A0                                                
							FF D0                                       call  rax                                                                  
							48 81 C4 A8 00 00 00                        add  rsp,0x00000000000000A0                                                
							41 5F                                       pop  r15                                                                   
							41 5E                                       pop  r14                                                                   
							41 5D                                       pop  r13                                                                   
							41 5C                                       pop  r12                                                                   
							41 5B                                       pop  r11                                                                   
							41 5A                                       pop  r10                                                                   
							41 59                                       pop  r9                                                                    
							41 58                                       pop  r8                                                                    
							5F                                          pop  rdi                                                                   
							5E                                          pop  rsi                                                                   
							5D                                          pop  rbp                                                                   
							5B                                          pop  rbx                                                                   
							5A                                          pop  rdx                                                                   
							59                                          pop  rcx                                                                   
							48 B8 89 67 45 23 01 00 00 00               mov  rax,0x0000000123456789                                                
							48 C7 00 01 00 00 00                        mov  qword ptr ds:[rax],0x0000000000000001                                 
							58                                          pop  rax                                                                   
							FF 25 00 00 00 00                           jmp  qword ptr ds:[PCHunter64.00000001403ABA27]                            
							00 00                                       add  byte ptr ds:[rax],al                                                  
							00 00                                       add  byte ptr ds:[rax],al                                                  
							00 00                                       add  byte ptr ds:[rax],al                                                  
							00 00                                       add  byte ptr ds:[rax],al                                                  
                                                      
					                                                

				*/
				char bufcode[] =
				{
					0x50, //push  rax
					0x51, //push  rcx   
					0x52, //push  rdx
					0x53, //push  rbx												//
					0x55, 															//
					0x56, 															//
					0x57, 															//
					0x41, 0x50, 													//
					0x41, 0x51, 													//
					0x41, 0x52, 													//
					0x41, 0x53, 													//
					0x41, 0x54, 													//
					0x41, 0x55, 													//
					0x41, 0x56, 													//
					0x41, 0x57, 													//
					0x48, 0xB8, 0x99, 0x89, 0x67, 0x45, 0x23, 0x01, 0x00,0x00, 		// mov  rax,shellcode位置
					0x48, 0x81, 0xEC, 0xA0, 0x00, 0x00, 0x00, 						// sub  rsp,0x00000000000000A0 
					0xFF, 0xD0, 													// call  rax  
					0x48, 0x81, 0xC4, 0xA0, 0x00, 0x00, 0x00, 						//add  rsp,0x00000000000000A0      //Rsp对齐16字节(规定) 
					0x41, 0x5F, 													//
					0x41, 0x5E,														//
					0x41, 0x5D, 													//
					0x41, 0x5C, 													//
					0x41, 0x5B, 													//
					0x41, 0x5A, 													//
					0x41, 0x59, 													//
					0x41, 0x58, 													//
					0x5F, 															//
					0x5E, 															//
					0x5D, 															//
					0x5B, 															//
					0x5A,															//
					0x59, 															//pop  rcx   
					0x48, 0xB8, 0x89, 0x67, 0x45, 0x23, 0x01, 0x00, 0x00, 0x00, 	//  mov  rax,(BaseAddr + 0x500)  
					0x48, 0xC7, 0x00, 0x01, 0x00, 0x00, 0x00,  				        //  mov  qword ptr ds:[rax],0x0000000000000001  
					0x58,                                                           // pop rax 
					0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,                             //   jmp  回去    
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,					//
				};
				//x64进程
				
				ULONG64 initStackAddr = *(PULONG64)((PUCHAR)pThread + 0x28);//_KTHREAD的Initialstack 栈底部
				PKTRAP_FRAME ptrap = (PKTRAP_FRAME)(initStackAddr - sizeof(KTRAP_FRAME)); //栈顶
				*(PULONG64)&bufcode[25] = (ULONG64)ms;//shellcode的位置
				*(PULONG64)&bufcode[73] = (ULONG64)BaseAddr + 0x500; //往BaseAddr + 0x500里面放了个1(测试验证)
				*(PULONG64)&bufcode[95] = ptrap->Rip;//跳进要注入的线程rip  (rip改回去,复原)
				
				memcpy(BaseAddr, bufcode, sizeof(bufcode));//bufcode很小,占不到ms那个位置
				
				/*
				  ---------------------------------------------------
				 |          |                                        |
				 | bufcode  |        用户shellcode                   |
			     |          |                                        |
				  ---------------------------------------------------
				*/
				


				ptrap->Rip = BaseAddr; //改变注入线程的rip  (上面都是构造)
				
			}

			PFreeMemoryInfo item = (PFreeMemoryInfo)ExAllocatePool(NonPagedPool, sizeof(FreeMemoryInfo));
			
			item->IsExecuteAddr = BaseAddr + 0x500;   //这个地址的1是判断是否执行成功的标志
			
			item->pid = pid;
			
			item->freeSize = size;
			
			ExInitializeWorkItem(&item->workitem, ExFreeMemoryWorkItem, item);
			
			ExQueueWorkItem(&item->workitem, DelayedWorkQueue);  //DelayedWorkQueue用延时队列插入系统进程


			PsResumeThread(pThread, NULL);

		}
	} while (0);
	
	
	KeUnstackDetachProcess(&kApcState);

	ObDereferenceObject(pThread);
	ExFreePool(kShellcode);
	return TRUE;
}