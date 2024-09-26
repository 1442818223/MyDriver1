// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <windows.h>
DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
	MessageBoxA(NULL, "Hello from the thread!", "Notification", MB_OK);
	return 0;
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

		// 创建线程
	//	CreateThread(NULL, 0, ThreadFunction, NULL, 0, NULL);
		
        MessageBoxA(NULL, "Hello from the thread!", "Notification", MB_OK);
        
        break; // 确保这里有 break


    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

