#include <windows.h>
#include <iostream>

int main() {
    std::cout << "Press Enter to show the MessageBox..." << std::endl;

    while (true) {
        std::cin.get(); // 等待用户按下回车键

        MessageBoxA(NULL, "Hello, World!", "Test MessageBox", MB_OK);
        std::cout << "MessageBox displayed. Press Enter to show again or Ctrl+C to exit..." << std::endl;
    }

    return 0;
}
