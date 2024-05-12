#include <Windows.h>
#include "resource.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

//��floatת��Ϊstd::wstring
std::wstring floatToWstring(float val)
{
    std::wostringstream woss;
    woss << val;
    return woss.str();
}

void ���ƾ���(HDC hdc, HBRUSH brush, int x, int y, int w, int h) {
    RECT rect = { x, y, x + w, y + h };
    FillRect(hdc, &rect, brush);
}

void ����(HDC hdc, HBRUSH brush, int x, int y, int w, int h, int ���) {
    ���ƾ���(hdc, brush, x, y, w, ���); // ����
    ���ƾ���(hdc, brush, x, y + ���, ���, h - ���); // ���
    ���ƾ���(hdc, brush, (x + w) - ���, y + ���, ���, h - ���); // �ұ�
    ���ƾ���(hdc, brush, x + ���, y + h - ���, w - ��� * 2, ���); // �ױ�
}

// ����һ���ṹ�壬���ڴ洢ÿһ������
struct RectData {
    float x, y, w, h;
    int d1, d2, d3, d4, d5;
    std::string type;
};

std::vector<RectData> rectDataVec; // �洢�ļ��е�����
HBRUSH brushRect; // ������仭ˢ

INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool initialized = false;

    switch (uMsg) {
    case WM_INITDIALOG:
    {
        SetTimer(hDlg, 1, 1000, NULL);  //����һ��1�봥��һ�εĶ�ʱ������ʱ����IDΪ1

        // ������ˢ
        brushRect = CreateSolidBrush(RGB(0, 0, 255));
        // ��ȡ��Ļ�ߴ�
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // ���ô���λ�úʹ�С��ʹ��ȫ��
        SetWindowPos(hDlg, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_SHOWWINDOW);
        initialized = true;
        break;
    }
    case WM_TIMER:
    {
        rectDataVec.clear();  // ��վɵľ�������
        std::string filename = "C:\\Users\\a0916\\Documents\\leidian9\\Pictures\\b.log"; // �ļ�·��
        std::ifstream file(filename); // ���ļ�
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

        InvalidateRect(hDlg, NULL, TRUE); // ˢ�������Ի����⽫����WM_PAINT���»���
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

        // ����ÿ������
        for (const auto& rectData : rectDataVec) {
            //std::wstring wstrVal = floatToWstring(rectData.x);
            //LPCWSTR lpcwstrVal = wstrVal.c_str();
            //MessageBox(hDlg, lpcwstrVal, L"Demo", MB_OK);
            ����(hdc, brushRect, rectData.x, rectData.y,
                rectData.w, rectData.h, 3);
        }

        EndPaint(hDlg, &ps);
        break;
    }
    case WM_CLOSE:
        KillTimer(hDlg, 1);  //ɾ����ʱ��
        DeleteObject(brushRect);  //ɾ����ˢ
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