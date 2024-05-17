#include <ntifs.h>   
#include<ntddk.h>
#include<wdm.h>

LARGE_INTEGER cookie;

NTSTATUS RegistryCallback(
	 PVOID CallbackContext, //�ص������Ĳ���
	 PVOID Argument1,  //���Ծ�����������
	 PVOID Argument2  //���Ծ�����������
) {
    NTSTATUS status = STATUS_SUCCESS;

	//DbgPrintEx(77, 0, "����һ:%p\n", CallbackContext);    ********************************************************
    REG_NOTIFY_CLASS  tempclass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;  //ע���������кܶ�
                                   //����ǧ��Ҫת��*(REG_NOTIFY_CLASS*)  ��ΪREG_NOTIFY_CLASS���ǻ�����������
                                                                          // ����������******************************

    UNICODE_STRING tempname = { 0 };
    tempname.Length = 0;
    tempname.MaximumLength = 1024 * sizeof(WCHAR);
    tempname.Buffer = (PWCH)ExAllocatePool(NonPagedPool, tempname.MaximumLength);
    RtlZeroMemory(tempname.Buffer, tempname.MaximumLength);
    RtlInitUnicodeString(&tempname, L"*ASDIOPXXAADCGR*");


    switch (tempclass) {
    case RegNtPreOpenKey: //����Ex��Ϊ�˼����ϰ汾
    case RegNtPreOpenKeyEx:
    case RegNtPreCreateKey:
    case RegNtPreCreateKeyEx: //��������Ҳ���Դ�
    {
        
        REG_CREATE_KEY_INFORMATION_V1 pkeinfo = *(REG_CREATE_KEY_INFORMATION_V1*)Argument2;
        __try {
            // ��ӡ������Ϣ
            DbgPrintEx(77, 0, "key info <%wZ>\n", pkeinfo.CompleteName);

            // �����������Ƿ�ƥ��
            if (FsRtlIsNameInExpression(&tempname, pkeinfo.CompleteName, TRUE, NULL)) {
                DbgPrintEx(77, 0, "Bad Create\n");
                status = STATUS_UNSUCCESSFUL;   //�ص���������STATUS_UNSUCCESSFUL�ͱ�ʾ��׽��������ע�������ط���ʧ��
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            DbgPrintEx(77, 0, "bad memory\n");
        }
        break;
    }
 
    default:
        break;
    }

    return status;
}

VOID  UnloadDriver(PDRIVER_OBJECT pDriver)
{
	DbgPrintEx(77, 0, "ж�سɹ�\n");


	// ʹ�ñ���� Cookie ע���ص�
	CmUnRegisterCallback(cookie);

	return;
}
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegpath)
{

	DbgPrintEx(77, 0, "���سɹ�\n");

	NTSTATUS status;
	
	// ע��ص�����
	status = CmRegisterCallback(RegistryCallback, //�ص�����
		(PVOID)0x123456, //�ص������Ĳ���
		&cookie); //ע����
	if (!NT_SUCCESS(status)) {
        DbgPrintEx(77, 0, "ע��ص�ʧ��: %08x\n", status);
		return status;
	}

	pDriver->DriverUnload = UnloadDriver;

	return 0;
}