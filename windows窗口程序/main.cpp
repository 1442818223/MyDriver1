//ʹ�� ��Դ�ļ� �� ��װ�˻�ͼ���̵�DialogBox���� �����Ի���
#include<Windows.h>
#include"resource.h"

INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	BOOL bRet = TRUE;
	switch (uMsg)
	{

		//������Ϣ�ɷ�	 
	case WM_COMMAND:
		switch (LOWORD(wParam)) { // ť����Ϣ�Ĵ���(�����Ϣ�ĵ�λ�־��ǰ�ťID)
		case IDC_BUTTON1:
			MessageBox(hDlg, L"Hello Button 1", L"Demo", MB_OK); break;
		case IDC_BUTTON2:
			MessageBox(hDlg, L"Hello Button 2", L"Demo", MB_OK); break;
		default:
			bRet = FALSE;
			break;
		}

	case WM_LBUTTONDOWN:
		MessageBox(hDlg, L"Hello����!", L"Demo", MB_OK);
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	default:
		bRet = FALSE;   //��������Ϣתȥwindows����
		break;
	}

	return bRet;      //��������Ϣתȥwindows����
}


int APIENTRY WinMain(HINSTANCE hinstance,//�ǵ�ǰ����ʵ���ľ��
	HINSTANCE hPrevInstance,//ʼ��Ϊ NULL,����֧�ֶ��ʵ����Ӧ�ó���
	LPSTR lpCmdLine,// �������в�����������������л�ȡ��������ʱ���ݵĶ�����Ϣ
	int nCmdshow)//��ͨ�����ڿ��ƴ����������������С�����������ʾ��
{

	DialogBox(hinstance, MAKEINTRESOURCE(IDD_MIAN), NULL, MainDlgProc); //ֻ�ô����ڹ���,������ȥ��������
	//ʹ�ú��ʶ��Դ�ര��


}