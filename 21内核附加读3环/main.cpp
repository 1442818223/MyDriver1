#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>

#define _SYB_NAME L"\\\\.\\sysmblicname"
#define CTL_TALK CTL_CODE(FILE_DEVICE_UNKNOWN, 0x9000, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct DATA {
    HANDLE pid;                 // 要读写的进程ID
    unsigned __int64 address;   // 要读写的地址
    DWORD size;                 // 读写长度
    PVOID data;                 // 要读写的数据, 空指针 对应任何数据类型
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

    printf("输入操作的PID: ");
    if (scanf("%lu", &pid) != 1) {
        printf("Invalid input for PID\n");
        CloseHandle(hDevice);
        return 1;
    }
    printf("输入操作的16进制地址: ");
    if (scanf("%llx", &address) != 1) {
        printf("Invalid input for address\n");
        CloseHandle(hDevice);
        return 1;
    }

    data.pid = (HANDLE)pid;
    data.address = address;
    data.size = sizeof(DWORD);
    data.data = 0;

    DWORD outbuffer = 0; // 要返回什么数据类型就定义什么类型
    DWORD dwOut;

    printf("Sending DeviceIoControl...\n");
    if (DeviceIoControl(hDevice, CTL_TALK,
        &data, sizeof(Data),  //入
        &outbuffer, sizeof(outbuffer),  //出 
        &dwOut, NULL)) {
        printf("通信成功\n");
        printf("接收到的数据为: %d\n", outbuffer);
    }
    else {
        printf("错误码: %d\n", GetLastError());
    }

    CloseHandle(hDevice);
    printf("程序结束，按任意键退出...\n");
    getchar();  // 等待输入，以防止控制台窗口立即关闭
    getchar();  // 再次等待输入，以便查看程序输出
    return 0;
}
