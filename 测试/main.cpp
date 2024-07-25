#include <windows.h>
#include <iostream>

 
void rewrw() {


    printf("%s\n", __FUNCTION__);//__FUNCTION__当前在哪个函数就会转换为那个函数的字符串  编译器实现的

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

    // 将 MessageBoxA 地址填入 bufcode
    *(int*)&bufcode[9] = (ULONG_PTR)MessageBoxA;

    // 将机器码复制到分配的内存
    memcpy(mem, bufcode, sizeof(bufcode));

    // 打印分配的内存地址
    printf("%p\r\n", mem);
    system("pause");

    // 定义函数指针并调用它
    func=(FuncProc)mem;              //要让bufcode也能执行需要定义权限
    func();
    system("pause");

    // 打印内存中的第一个整数
    printf("mem = %d\r\n", *mem);
    
   
    
   
    system("pause");

    VirtualFree(mem, 0, MEM_RELEASE);

    return 0;
}
