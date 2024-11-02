#pragma once
#include<ntifs.h>
#include<ntddk.h>
PEPROCESS FindProcessByName(CHAR* ProcessName);
MODE SetPreviousMode(PETHREAD Thread, MODE Mode);