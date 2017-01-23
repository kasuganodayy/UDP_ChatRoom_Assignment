//////////////////////////////////////////////////
//  �`���b�g�\�t�g								//
//////////////////////////////////////////////////
#include	<Windows.h>
#include	<list>
#include	"resource.h"
#include	"UdpChatCore.h"	

// Globad Variables
bool debugFlag = false;

// �v���g�^�C�v�錾
BOOL CALLBACK DialogProc(	HWND hwndDlg,		// �_�C�A���O�{�b�N�X�̃n���h���l
							UINT uMsg,			// ���b�Z�[�W�h�c
							WPARAM wParam,		// ���Ԗڃp�����[�^
							LPARAM lParam);		// ���Ԗڃp�����[�^


/*-------------------------------------
	�v�h�m�l�`�h�m�֐�	
--------------------------------------*/
int APIENTRY WinMain(HINSTANCE 	hInstance, 		// �A�v���P�[�V�����C���X�^���X�l
		     HINSTANCE 	hPrevInstance,			// �Ӗ��Ȃ�
		     LPSTR 	lpszArgs, 					// �N�����̈���������
		     int 	nWinMode)					// �E�C���h�E�\�����[�h
{

	DialogBox	(hInstance,
					MAKEINTRESOURCE(IDD_MAINFORM),
					0,
					DialogProc);

	return 0;
}

/*-------------------------------------

	���b�Z�[�W�����֐��i�E�C���h�E�֐��j
		���̊֐��́A�n�r����Ăяo����ă��b�Z�[�W�L���[����
		���b�Z�[�W�̈��n�����󂯂�							

--------------------------------------*/
BOOL CALLBACK DialogProc(	HWND hwndDlg,		// handle to dialog box
							UINT uMsg,			// message
							WPARAM wParam,		// first message parameter
							LPARAM lParam)
{
	int		ccnt, ccip, ccpt;			// ������
	char	buffer[512];
	char	ip[256];
	char	port[32];


	switch(uMsg){

		case WM_INITDIALOG:
			//�ŏ��̓`���b�g���͗����Â�
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHATINPUT),false);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MESSAGELIST),false);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SEND),false);
			return true;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			// �f�o�b�O�{�^��
			case IDC_DEBUG:
			{
				debugFlag = !debugFlag;
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHATINPUT), debugFlag);
				EnableWindow(GetDlgItem(hwndDlg, IDC_MESSAGELIST), debugFlag);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SEND), debugFlag);

				char dbgTxt[512] = { "DEBUG: WinSock Error may occur." };
				SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGELIST), LB_INSERTSTRING, (WPARAM)0, (LPARAM)dbgTxt);
			}

				break;

			// �T�[�o�N���{�^��
			case IDC_SRVSTART:
				ccpt = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SRVMODEPORT));
				GetWindowText(GetDlgItem(hwndDlg, IDC_SRVMODEPORT), port, ccpt + 1);

				if (debugFlag)
				{
					char dbgTxt[256];
					sprintf_s(dbgTxt, "DEBUG: Starting ServerMode...Port: %s", port);
					SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGELIST), LB_INSERTSTRING, (WPARAM)0, (LPARAM)dbgTxt);
				}

				if(initUdpChatSystem(hwndDlg,nullptr,  port))
				{
					if (startUdpChat_Server())
					{
						// Disable SRVSTART button
						EnableWindow(GetDlgItem(hwndDlg, IDC_SRVSTART), false);

						char txt[256];

						gethostname(txt, (int)sizeof(txt));
						HOSTENT* lpHost = gethostbyname(txt);
						IN_ADDR inaddr;
						memcpy(&inaddr, lpHost->h_addr_list[0], 4);

						sprintf_s(txt, "Started Server on IP: %s / PORT: %s", inet_ntoa(inaddr), port);
						SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGELIST), LB_INSERTSTRING, (WPARAM)0, (LPARAM)txt);
					}
				}
				break;

			// ���O�C���{�^��
			case IDSTART:
				ccip = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SERVERIP));
				ccpt = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SERVERPORT));
				ccnt = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_NAME));

				GetWindowText(GetDlgItem(hwndDlg, IDC_SERVERIP), ip, ccip + 1);
				GetWindowText(GetDlgItem(hwndDlg, IDC_SERVERPORT), port, ccpt + 1);
				GetWindowText(GetDlgItem(hwndDlg, IDC_NAME), buffer, ccnt + 1);

				if (debugFlag)
				{
					char dbgTxt[1024];
					sprintf_s(dbgTxt, "DEBUG: Logging into IP: %s / PORT: %s...with name: \"%s\"", ip, port, buffer);
					SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGELIST), LB_INSERTSTRING, (WPARAM)0, (LPARAM)dbgTxt);
				}

				if (initUdpChatSystem(hwndDlg, ip, port))
				{
					startUdpChat_Client(buffer);
				}
				break;

			// ���M�{�^��
			case IDC_SEND:
				ccnt = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHATINPUT));
				GetWindowText(GetDlgItem(hwndDlg, IDC_CHATINPUT), buffer, ccnt + 1);

				if (debugFlag)
				{
					char dbgTxt[1024];
					sprintf_s(dbgTxt, "DEBUG: %s", buffer);
					SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGELIST), LB_INSERTSTRING, (WPARAM)0, (LPARAM)dbgTxt);
				}

				sendChatMessage(buffer);
				break;

				// �I���{�^��
			case IDEXIT:
				if (BN_CLICKED == HIWORD(wParam))
				{
					EndDialog(hwndDlg, IDEXIT);
					destructUdpChatSystem();
					destructWinSockSystem();
				}
				break;
			}
			return true;

		case WM_CLOSE:
			EndDialog(hwndDlg,IDEXIT);
			destructUdpChatSystem();
			destructWinSockSystem();
			return true;

	}
	return false;
}
