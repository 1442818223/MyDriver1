#include <windows.h>
#include <iostream>

int main() {
    std::cout << "Press Enter to show the MessageBox..." << std::endl;

    while (true) {
        std::cin.get(); // �ȴ��û����»س���

        MessageBoxA(NULL, "Hello, World!", "Test MessageBox", MB_OK);
        std::cout << "MessageBox displayed. Press Enter to show again or Ctrl+C to exit..." << std::endl;
    }

    return 0;
}
