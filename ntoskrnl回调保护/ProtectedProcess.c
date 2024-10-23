#include "ProtectedProcess.h"
#include "ExportFunc.h"
#include "Search.h"

extern POBJECT_TYPE * IoDriverObjectType;

HANDLE regHandle = NULL;
HANDLE gProtectId;

BOOLEAN SetProtectPid(HANDLE pid)
{
	gProtectId = pid;
}

OB_PREOP_CALLBACK_STATUS PreCallback(
	_In_ PVOID RegistrationContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation
)
{
	PEPROCESS Process = OperationInformation->Object;
	if (!Process) return OB_PREOP_SUCCESS;              //��ѯʧ�ܾ��Թ�

	HANDLE currentPid = PsGetCurrentProcessId();

	HANDLE targetPid = PsGetProcessId(Process);

	if (currentPid == gProtectId) return OB_PREOP_SUCCESS;   //Ҫ�Ǳ�����Թ�

	if (targetPid != gProtectId) return OB_PREOP_SUCCESS; //����Ҫ�����ľ��Թ�




	//ʣ�µľ���Ҫ������
	//DbgPrintEx(77, 0, "[db]:PreCallback\r\n");
	//OperationInformation->Object
	// �����������Ƿ�Ϊ�������
	if (OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE)
	{
		OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
		OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess = 0;
	}
	else // ���������Ƶ����
	{
		// ������ķ���Ȩ����Ϊ 0����ֹ������Ʋ���
		OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = 0;
		OperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess = 0;
	}

	return OB_PREOP_SUCCESS;
}

//ͨ������������豸�������ƻ�ȡ�� PDRIVER_OBJECT �����ָ��
PDRIVER_OBJECT GetDriverObjectByName(PWCH ObjName)
{
	
	UNICODE_STRING driverName1 = { 0 };
	
	RtlInitUnicodeString(&driverName1, ObjName);
	
	PDRIVER_OBJECT Driver = NULL;
	
	// ʹ�� ObReferenceObjectByName ������ȡ��������
	NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, 0, 0, *IoDriverObjectType, KernelMode, NULL, &Driver);
	
	if (NT_SUCCESS(status))
	{
		ObDereferenceObject(Driver);
	}
	
	return Driver;
}



VOID DestoryObRegister()
{
	if (regHandle) ObUnRegisterCallbacks(regHandle);
}

NTSTATUS InitObRegister()
{
	                                                //�豸������
	PDRIVER_OBJECT pDirver = GetDriverObjectByName(L"\\Driver\\WMIxWDM");

	if (!pDirver) return STATUS_UNSUCCESSFUL;

	ULONG64 jmpRcx =  searchCode("ntoskrnl.exe", ".text", "FFE1", 0); //FFE1:jmp rax

	if (!jmpRcx) return STATUS_UNSUCCESSFUL;

	OB_OPERATION_REGISTRATION obOp = { 0 };

	obOp.ObjectType = PsProcessType;
	obOp.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	obOp.PreOperation = jmpRcx;  //ǰ�ص�   [�������ڲ�������]  �������ĺô��������غ�����ַ ������ʶ��Ϊ�����ֱ��ִ��
	//obOp.PostOperation = PostCallBack;

	OB_CALLBACK_REGISTRATION obCallRegster = { 0 };
	obCallRegster.Version = ObGetFilterVersion();
	obCallRegster.OperationRegistrationCount = 1;
	obCallRegster.RegistrationContext = PreCallback; //����
	obCallRegster.OperationRegistration = &obOp;

	RtlInitUnicodeString(&obCallRegster.Altitude, L"456340");

	//PKLDR_DATA_TABLE_ENTRY ldr = (PKLDR_DATA_TABLE_ENTRY)pDirver->DriverSection;  //Ϊ��,û��pDirver�����Ļ���
	//ldr->Flags |= 0x20;

	//��ʼ�򲹶�(�������0x20Ȩ�޽��в���)
	RTL_OSVERSIONINFOEXW version = { 0 };
	RtlGetVersion(&version);
	PUCHAR findFunc = NULL;
	//WIN7
	if (version.dwBuildNumber == 7600 || version.dwBuildNumber == 7601)
	{
		PUCHAR func = (PUCHAR)ObRegisterCallbacks;
		for (int i = 0; i < 0x500; i++)
		{
			if (func[i] == 0x74 && func[i + 2] == 0xe8 && func[i + 7] == 0x3b && func[i + 8] == 0xc3)
			{
				LARGE_INTEGER larger;
				larger.QuadPart = func + i + 7;
				larger.LowPart += *(PULONG)(func + i + 3);
				findFunc = larger.QuadPart;
				break;
			}
		}
	}
	else
	{
		PUCHAR func = (PUCHAR)ObRegisterCallbacks;
		for (int i = 0; i < 0x500; i++)
		{
			if (func[i] == 0xBA && func[i + 5] == 0xe8 && func[i + 10] == 0x85 && func[i + 11] == 0xc0)
			{
				LARGE_INTEGER larger;
				larger.QuadPart = func + i + 10;
				larger.LowPart += *(PULONG)(func + i + 6);
				findFunc = larger.QuadPart;
				break;
			}
		}
	}

	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (findFunc)
	{
		PHYSICAL_ADDRESS phy = MmGetPhysicalAddress(findFunc);//��ȡ ָ�������ַ ��Ӧ �������ַ(�����ֵ)
		PVOID mem = MmMapIoSpace(phy, 10, MmNonCached);//�������ڴ��ַӳ�䵽�����ַ�ռ���
		if (mem)
		{
			UCHAR bufCode[10] = { 0 };
			UCHAR patch[] = { 0xb0,0x1,0xc3 };  //�ҵ�Ȩ�޼�⺯��ͷ��ֱ�ӷ���1
			memcpy(bufCode, mem, 10);
			memcpy(mem, patch, sizeof(patch));
			status = ObRegisterCallbacks(&obCallRegster, &regHandle);
			memcpy(mem, bufCode, 10); //������֮�����Ǿ͸����޸���ȥ
		}
	}
	


	return status;
}