#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>

#define _SYB_NAME L"\\\\.\\sysmblicname"
#define CTL_TALK CTL_CODE(FILE_DEVICE_UNKNOWN, 0x9000, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct DATA {
    HANDLE pid;                 // Ҫ��д�Ľ���ID
    unsigned __int64 address;   // Ҫ��д�ĵ�ַ
    DWORD size;                 // ��д����
    PVOID data;                 // Ҫ��д������, ��ָ�� ��Ӧ�κ���������
} Data, * PDATA;

int main() {
    HANDLE hDevice = CreateFile(_SYB_NAME,
        FILE_ALL_ACCESS,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Error: %x\n", GetLastError());
        return 1;
    }

    Data data = { 0 };
    unsigned long pid;
    unsigned long long address;

    printf("���������PID: ");
    if (scanf("%lu", &pid) != 1) {
        printf("Invalid input for PID\n");
        CloseHandle(hDevice);
        return 1;
    }
    printf("���������16���Ƶ�ַ: ");
    if (scanf("%llx", &address) != 1) {
        printf("Invalid input for address\n");
        CloseHandle(hDevice);
        return 1;
    }

    data.pid = (HANDLE)pid;
    data.address = address;
    data.size = sizeof(DWORD);
    data.data = 0;

    DWORD outbuffer = 0; // Ҫ����ʲô�������;Ͷ���ʲô����
    DWORD dwOut;

    printf("Sending DeviceIoControl...\n");
    if (DeviceIoControl(hDevice, CTL_TALK,
        &data, sizeof(Data),  //��
        &outbuffer, sizeof(outbuffer),  //�� 
        &dwOut, NULL)) {
        printf("ͨ�ųɹ�\n");
        printf("���յ�������Ϊ: %d\n", outbuffer);
    }
    else {
        printf("������: %d\n", GetLastError());
    }

    CloseHandle(hDevice);
    printf("�����������������˳�...\n");
    getchar();  // �ȴ����룬�Է�ֹ����̨���������ر�
    getchar();  // �ٴεȴ����룬�Ա�鿴�������
    return 0;
}
