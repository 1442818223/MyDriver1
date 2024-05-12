#include <Windows.h>
#include "resource.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

//将float转换为std::wstring
std::wstring floatToWstring(float val)
{
    std::wostringstream woss;
    woss << val;
    return woss.str();
}

void 绘制矩形(HDC hdc, HBRUSH brush, int x, int y, int w, int h) {
    RECT rect = { x, y, x + w, y + h };
    FillRect(hdc, &rect, brush);
}

void 画框(HDC hdc, HBRUSH brush, int x, int y, int w, int h, int 厚度) {
    绘制矩形(hdc, brush, x, y, w, 厚度); // 顶边
    绘制矩形(hdc, brush, x, y + 厚度, 厚度, h - 厚度); // 左边
    绘制矩形(hdc, brush, (x + w) - 厚度, y + 厚度, 厚度, h - 厚度); // 右边
    绘制矩形(hdc, brush, x + 厚度, y + h - 厚度, w - 厚度 * 2, 厚度); // 底边
}

// 定义一个结构体，用于存储每一行数据
struct RectData {
    float x, y, w, h;
    int d1, d2, d3, d4, d5;
    std::string type;
};

std::vector<RectData> rectDataVec; // 存储文件中的数据
HBRUSH brushRect; // 矩形填充画刷

INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool initialized = false;

    switch (uMsg) {
    case WM_INITDIALOG:
    {
        SetTimer(hDlg, 1, 1000, NULL);  //设置一个1秒触发一次的定时器，定时器的ID为1

        // 创建画刷
        brushRect = CreateSolidBrush(RGB(0, 0, 255));
        // 获取屏幕尺寸
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // 设置窗口位置和大小，使其全屏
        SetWindowPos(hDlg, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_SHOWWINDOW);
        initialized = true;
        break;
    }
    case WM_TIMER:
    {
        rectDataVec.clear();  // 清空旧的矩形数据
        std::string filename = "C:\\Users\\a0916\\Documents\\leidian9\\Pictures\\b.log"; // 文件路径
        std::ifstream file(filename); // 打开文件
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                RectData rectData;
                std::string token;
                std::vector<std::string> tokens;
                while (std::getline(iss, token, ',')) {
                    tokens.push_back(token);
                }
                if (tokens.size() == 10) {
                    rectData.x = std::stof(tokens[0])/100-100;
                    rectData.y = std::stof(tokens[1]) /100-100;
                    rectData.w = std::stof(tokens[2]) ;
                    rectData.h = std::stof(tokens[3]) ;
                    rectData.d1 = std::stoi(tokens[4]);
                    rectData.d2 = std::stoi(tokens[5]);
                    rectData.d3 = std::stoi(tokens[6]);
                    rectData.d4 = std::stoi(tokens[7]);
                    rectData.d5 = std::stoi(tokens[8]);
                    rectData.type = tokens[9];
                    rectDataVec.push_back(rectData);
                }
                else {
                    std::cerr << "Failed to parse line: " << line << std::endl;
                }
            }
            file.close();
        }

        InvalidateRect(hDlg, NULL, TRUE); // 刷新整个对话框，这将触发WM_PAINT重新绘制
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
       /* case IDC_BUTTON1:
            MessageBox(hDlg, L"Hello Button 1", L"Demo", MB_OK);
            break;
        case IDC_BUTTON2:
            MessageBox(hDlg, L"Hello Button 2", L"Demo", MB_OK);
            break;*/
        default:
            return FALSE;
        }
        break;
    case WM_PAINT:
    {
        if (!initialized) break;

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);

        // 绘制每个矩形
        for (const auto& rectData : rectDataVec) {
            //std::wstring wstrVal = floatToWstring(rectData.x);
            //LPCWSTR lpcwstrVal = wstrVal.c_str();
            //MessageBox(hDlg, lpcwstrVal, L"Demo", MB_OK);
            画框(hdc, brushRect, rectData.x, rectData.y,
                rectData.w, rectData.h, 3);
        }

        EndPaint(hDlg, &ps);
        break;
    }
    case WM_CLOSE:
        KillTimer(hDlg, 1);  //删除定时器
        DeleteObject(brushRect);  //删除画刷
        EndDialog(hDlg, 0);
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, MainDlgProc);
}