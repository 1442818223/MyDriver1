#include <windows.h>
#include <iostream>

 
void rewrw() {


    printf("%s\n", __FUNCTION__);//__FUNCTION__��ǰ���ĸ������ͻ�ת��Ϊ�Ǹ��������ַ���  ������ʵ�ֵ�

    OutputDebugStringA("__FUNCTION__\n");
    return;
}

typedef void(__stdcall* FuncProc)();

int main(int argc, char* argv[]) {

    OutputDebugStringA("[av]dwadaw\n");
    rewrw();


    int e = 1;
    int starttimer = GetTickCount();
    while (e--) {
        Beep(1000, 500);
        Sleep(1000);
    }
    int endtimer = GetTickCount();
    printf("%d\n", endtimer-starttimer);

    int* mem = (int*)VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    FuncProc func = NULL;
    if (!mem) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 1;
    }
    memset(mem, 0, 0x1000);


    char bufcode[] = {
        0x6A, 0x00, // push 0
        0x6A, 0x00, // push 0
        0x6A, 0x00, // push 0
        0x6A, 0x00, // push 0
        0xB8, 0, 0, 0, 0, // mov eax, MessageBoxA address (to be filled)
        0xFF, 0xD0, // call eax
        0xC3       // ret
    };

    // �� MessageBoxA ��ַ���� bufcode
    *(int*)&bufcode[9] = (ULONG_PTR)MessageBoxA;

    // �������븴�Ƶ�������ڴ�
    memcpy(mem, bufcode, sizeof(bufcode));

    // ��ӡ������ڴ��ַ
    printf("%p\r\n", mem);
    system("pause");

    // ���庯��ָ�벢������
    func=(FuncProc)mem;              //Ҫ��bufcodeҲ��ִ����Ҫ����Ȩ��
    func();
    system("pause");

    // ��ӡ�ڴ��еĵ�һ������
    printf("mem = %d\r\n", *mem);
    
   
    
   
    system("pause");

    VirtualFree(mem, 0, MEM_RELEASE);

    return 0;
}
