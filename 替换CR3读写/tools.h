#pragma once
#include<ntifs.h>
#include<ntddk.h>
PEPROCESS FindProcessByName(CHAR* ProcessName);
ULONG FatchPid(CHAR* ProcessName);
MODE SetPreviousMode(PETHREAD Thread, MODE Mode);

PVOID BeGetSystemRoutineAddress(IN CHAR* functionToResolve);