#pragma once
typedef struct _WIN1X_MM_AVL_TABLE
{
	/*+0x000*/    union
	{
		/*+0x000*/    struct _WIN1X_MM_AVL_NODE* BalancedRoot;
		/*+0x000*/    void* NodeHint;
		/*+0x000*/    unsigned __int64 NumberGenericTableElements;
	};
} WIN1X_MM_AVL_TABLE, * WIN1X_PMM_AVL_TABLE;
typedef struct _HIDE_MEMORY_BUFFER
{
	PEPROCESS pProcess;
	PMDL pMdl;
	PVOID pVadShort;
	ULONGLONG StartingVpn;
	ULONGLONG EndingVpn;
} HIDE_MEMORY_BUFFER, * PHIDE_MEMORY_BUFFER;


typedef struct _WIN1X_MM_AVL_NODE
{
	/*+0x000*/    union
	{
		struct _WIN1X_MM_AVL_NODE* Children[2];
		struct
		{
			struct _WIN1X_MM_AVL_NODE* LeftChild;
			struct _WIN1X_MM_AVL_NODE* RightChild;
		};
	};
	/*+0x010*/    union
	{
		UCHAR Red : 1;
		UCHAR Balance : 2;
		ULONG_PTR ParentValue;
	};
} WIN1X_MM_AVL_NODE, * WIN1X_PMM_AVL_NODE;
typedef struct _WIN1X_MMVAD_SHORT
{
	/*+0x000*/    union
	{
		/*+0x000*/    struct _WIN1X_MM_AVL_NODE VadNode;
		/*+0x000*/    struct _WIN1X_MMVAD_SHORT* NextVad;
	};
	/*+0x018*/    ULONG StartingVpn;
	/*+0x01c*/    ULONG EndingVpn;
	/*+0x020*/    UCHAR StartingVpnHigh;
	/*+0x021*/    UCHAR EndingVpnHigh;
	/*+0x022*/    UCHAR CommitChargeHigh;
	/*+0x023*/    UCHAR SpareNT64VadUChar;
	/*+0x024*/    LONG ReferenceCount;
	/*+0x028*/    struct _EX_PUSH_LOCK* PushLock;
	/*+0x030*/    ULONG VadFlags;
	/*+0x034*/    ULONG VadFlags1;
	/*+0x038*/    struct _MI_VAD_EVENT_BLOCK* EventList;
} WIN1X_MMVAD_SHORT, * WIN1X_PMMVAD_SHORT;