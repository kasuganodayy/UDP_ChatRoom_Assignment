//////////////////////////////////////////////////
//  チャットソフト								//
//////////////////////////////////////////////////
#include	<Windows.h>
#include	<list>
#include	"resource.h"
#include	"UdpChatCore.h"	

// Globad Variables
bool debugFlag = false;

// プロトタイプ宣言
BOOL CALLBACK DialogProc(	HWND hwndDlg,		// ダイアログボックスのハンドル値
							UINT uMsg,			// メッセージＩＤ
							WPARAM wParam,		// 第一番目パラメータ
							LPARAM lParam);		// 第二番目パラメータ


/*-------------------------------------
	ＷＩＮＭＡＩＮ関数	
--------------------------------------*/
int APIENTRY WinMain(HINSTANCE 	hInstance, 		// アプリケーションインスタンス値
		     HINSTANCE 	hPrevInstance,			// 意味なし
		     LPSTR 	lpszArgs, 					// 起動時の引数文字列
		     int 	nWinMode)					// ウインドウ表示モード
{

	DialogBox	(hInstance,
					MAKEINTRESOURCE(IDD_MAINFORM),
					0,
					DialogProc);

	return 0;
}

/*-------------------------------------

	メッセージ処理関数（ウインドウ関数）
		この関数は、ＯＳから呼び出されてメッセージキューから
		メッセージの引渡しを受ける							

--------------------------------------*/
BOOL CALLBACK DialogProc(	HWND hwndDlg,		// handle to dialog box
							UINT uMsg,			// message
							WPARAM wParam,		// first message parameter
							LPARAM lParam)
{
	int		ccnt, ccip, ccpt;			// 文字数
	char	buffer[512];
	char	ip[256];
	char	port[32];


	switch(uMsg){

		case WM_INITDIALOG:
			//最初はチャット入力欄が暗い
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHATINPUT),false);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MESSAGELIST),false);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SEND),false);
			return true;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			// デバッグボタン
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

			// サーバ起動ボタン
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

			// ログインボタン
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

			// 送信ボタン
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

				// 終了ボタン
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
