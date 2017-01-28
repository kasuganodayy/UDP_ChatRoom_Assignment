#include "resource.h"
#include "WinSockWrappers.h"

#include "UdpChatCore.h"
#include "ServerMessageStruct.h"
#include "ClientMessageStruct.h"
#include <thread>
#include <time.h>

#include <list>
#include <map>
#include <exception>

using namespace std;


//--------------------------------
// ���C�u����
//--------------------------------
#pragma	comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

//--------------------------------
// �\���̐錾
//--------------------------------
typedef struct _ChatBuffer
{
	char* chat_string;
	int	buff_size;
}ChatBuffer, *LPChatBuffer;

using namespace std;

//--------------------------------
// �O���[�o���ϐ�
//--------------------------------
HWND		g_hwnd;
SOCKET		g_mysock;
sockaddr_in g_myaddr;
sockaddr_in g_toaddr;
int			g_myUserId;

thread		g_recvThread;
bool		g_mainloop;

bool		g_serverFlag = false;
char		g_sysTxt[256];
ChatMsgData g_sendbuff;

//���b�Z�[�W�ۑ��p�ϒ��z��
list<LPChatBuffer>		g_messages;

map <int, string>		g_userTable;		// UserID -> name table
map <int, sockaddr_in>	g_connectedTable;	// UserID -> sockaddr_in

string					g_logoutNameBuff;


//--------------------------------
// �v���g�^�C�v
//--------------------------------
void recvThread_Server(void);
void recvThread_Client(void);

void setMessageToBuffer(LPSrvMsg msg, LPChatBuffer buffer);
void setMessageToBuffer(LPChatMsgData msg, LPChatBuffer buffer);
void makeChatString(LPChatBuffer buffer, int userId, char* message);
void makeChatString(LPChatBuffer buffer, char* name, char* message);
void insertData(LPChatBuffer chat_buff);
void insertData(char* string);
void deleteChatData(LPChatBuffer buffer);
void requestLogin(char* name);


bool initUdpChatSystem(HWND hwnd, char* ip, char* port)
{
	g_serverFlag = (ip ? false : true);

	unsigned short portNo;

	portNo = strtol(port, NULL, 10);

	g_hwnd = hwnd;

	initWinSockSystem();
	g_mysock = createUdpSocket();

	if (g_mysock != INVALID_SOCKET)
	{
		if (g_serverFlag)
		{
			setAddressToSockAddrIn(&g_myaddr, INADDR_ANY, portNo);

			if (bindToSocket(g_mysock, &g_myaddr))
			{
				return true;
			}
		}
		else
		{
			setAddressToSockAddrIn(&g_toaddr, inet_addr(ip), portNo);

			return true;
		}

	}
		
	return false;
}

void destructUdpChatSystem()
{
	if (g_mainloop)
	{
		g_mainloop = false;

		if (g_serverFlag)
		{
			g_userTable.clear();
		}
		else
		{
			insertMyData("�ގ����܂����B");
		}
	}

	closeSocket(&g_mysock);

	if (g_recvThread.joinable())
	{
		g_recvThread.join();
	}
}

bool initWinSockSystem()
{
	if (g_mysock != NULL)
	{
		WinSockClose();
	}
	g_mysock = INVALID_SOCKET;
	g_mainloop = false;
	
	return WinSockInit();
}

void destructWinSockSystem()
{
	WinSockClose();
}

void setMessageToBuffer(LPSrvMsg msg, LPChatBuffer buffer)
{

	if (msg->msg.ChatBody.userId == NULL || msg->msg.ChatBody.message == NULL)
	{
		return;
	}

	makeChatString(buffer, msg->msg.ChatBody.userId, msg->msg.ChatBody.message);

}

void setMessageToBuffer(LPChatMsgData msg, LPChatBuffer buffer)
{
	if (msg->msg.ChatBody.name == NULL || msg->msg.ChatBody.message == NULL)
	{
		return;
	}

	makeChatString(buffer, msg->msg.ChatBody.name, msg->msg.ChatBody.message);
}

void makeChatString(LPChatBuffer buffer, int userId, char* message)
{
	int size = 0;
	char temp_buffer[512];

	size = sprintf_s(temp_buffer, "%d: %s \r\n", userId, message);
	temp_buffer[size] = NULL;

	buffer->buff_size = size + 1;
	buffer->chat_string = (char*)malloc( sizeof(char) * (size + 1) );

	strcpy_s(buffer->chat_string, size + 1, temp_buffer);

}

void makeChatString(LPChatBuffer buffer, char* name, char* message)
{
	int size = 0;
	char temp_buffer[512];

	size = sprintf_s(temp_buffer, "%s: %s \r\n", name, message);
	temp_buffer[size] = NULL;

	buffer->buff_size = size + 1;
	buffer->chat_string = (char*)malloc(sizeof(char) * (size + 1));

	strcpy_s(buffer->chat_string, size + 1, temp_buffer);
}

void deleteChatData(LPChatBuffer buffer)
{
	free(buffer->chat_string);
	free(buffer);
}

void insertMyData(char* my_message)
{
	sendChatMessage(my_message);
}

void insertData(LPChatBuffer chat_buff)
{
	SendMessage(GetDlgItem(g_hwnd, IDC_MESSAGELIST), LB_INSERTSTRING, (WPARAM)0, (LPARAM)chat_buff->chat_string);
}

void insertData(char* string)
{
	SendMessage(GetDlgItem(g_hwnd, IDC_MESSAGELIST), LB_INSERTSTRING, (WPARAM)0, (LPARAM)string);
}

void recvThread_Server(void)
{
	SrvMsg recvbuff;
	LPChatBuffer message_data;
	sockaddr_in fromaddr;

	message_data = NULL;

	while(g_mainloop)
	{
		if (recvDataFrom(g_mysock, recvbuff.data, sizeof(recvbuff.data), (sockaddr*)&fromaddr) == SOCKET_ERROR)
			break;

		switch(recvbuff.msg.Header.type)
		{
		case SRVTYPE_CHAT:
			// ��M���b�Z�[�W���f����
			// Allocate memory for LPChatBuffer
			message_data = (LPChatBuffer)malloc(sizeof(ChatBuffer));

			// Allocate memory and set CharBuffer
			setMessageToBuffer(&recvbuff, message_data);

			insertData(message_data);
			deleteChatData(message_data);

			// Send msg to all connected users
			if(!g_connectedTable.empty())
			{
				g_sendbuff.msg.Header.type = MSGTYPE_CHAT;
				try
				{
					strcpy(g_sendbuff.msg.ChatBody.name, g_userTable.at(recvbuff.msg.ChatBody.userId).c_str());
				}
				catch (exception ex)
				{
					if (!strcmp(ex.what(), "invalid map<K, T> key"))
					{
						strcpy(g_sendbuff.msg.ChatBody.name, g_logoutNameBuff.c_str());
					}
				}

				strcpy(g_sendbuff.msg.ChatBody.message, recvbuff.msg.ChatBody.message);

				for (map<int, sockaddr_in>::iterator i = g_connectedTable.begin();
					i != g_connectedTable.end();
					++i)
				{
					sendDataTo(g_mysock, g_sendbuff.data, sizeof(g_sendbuff.data), (sockaddr*)&(i->second));
				}
			}
			break;

		case SRVTYPE_LOGIN:
			g_sendbuff.msg.Header.type = MSGTYPE_ACCEPT;

			if (strlen(recvbuff.msg.Login.name) > NAME_LENGTH)
			{
				// Reject login
				g_sendbuff.msg.AcceptBody.userId = -1;
			}
			else
			{
				int tempID = rand();
				while (g_userTable.find(tempID) != g_userTable.end())
				{
					static short userGenCount = 0;

					if (userGenCount > 100)
					{
						tempID = -1;
						strcpy(g_sendbuff.msg.ChatBody.message, "UserID attempts over-count.");
						break;
					}

					tempID = rand();
					userGenCount++;
				}

				if (tempID != -1)
				{
					// Accept Login
					// Insert to userTable
					g_userTable.insert(pair<int, string>(tempID, string(recvbuff.msg.Login.name)));
					// Insert to connectAddr list
					g_connectedTable.insert(pair<int, sockaddr_in>(tempID, fromaddr));

					sprintf_s(g_sysTxt, "SYSTEM: User %d logged in with name \"%s\".", tempID, recvbuff.msg.Login.name);
					insertData(g_sysTxt);
				}

				g_sendbuff.msg.AcceptBody.userId = tempID;

				sendDataTo(g_mysock, g_sendbuff.data, sizeof(g_sendbuff.data), (sockaddr*)&fromaddr);
			}
			break;

		case SRVTYPE_LOGOUT:
			if (recvbuff.msg.Logout.userId != -1)
			{
				g_logoutNameBuff = g_userTable.at(recvbuff.msg.Logout.userId);

				g_userTable.erase(recvbuff.msg.Logout.userId);
				g_connectedTable.erase(recvbuff.msg.Logout.userId);

				g_sendbuff.msg.Header.type = MSGTYPE_LOGOUT_ACCEPT;
				sendDataTo(g_mysock, g_sendbuff.data, sizeof(g_sendbuff.data), (sockaddr*)&fromaddr);
			}
			break;

		default:
			break;
		}
	}
}

void recvThread_Client(void)
{
	ChatMsgData recvBuff;
	LPChatBuffer message_data;
	sockaddr_in fromaddr;

	message_data = NULL;

	while (g_mainloop)
	{
		if (recvDataFrom(g_mysock, recvBuff.data, sizeof(recvBuff.data), (sockaddr*)&fromaddr) == SOCKET_ERROR)
			break;

		switch (recvBuff.msg.Header.type)
		{
		case MSGTYPE_CHAT:
			message_data = (LPChatBuffer)malloc(sizeof(ChatBuffer));

			// Allocate memory and set CharBuffer
			setMessageToBuffer(&recvBuff, message_data);

			insertData(message_data);
			deleteChatData(message_data);
			break;

		case MSGTYPE_ACCEPT:
			g_myUserId = recvBuff.msg.AcceptBody.userId;

			if ( g_myUserId == -1)
			{
				// Login rejected
				g_mainloop = false;

				sprintf_s(g_sysTxt, "ERROR: %s", recvBuff.msg.ChatBody.message);
				insertData(g_sysTxt);

				if (g_recvThread.joinable())
					g_recvThread.join();
			}
			else
			{
				EnableWindow(GetDlgItem(g_hwnd, IDC_SRVSTART), false);
				EnableWindow(GetDlgItem(g_hwnd, IDSTART), false);
				EnableWindow(GetDlgItem(g_hwnd, IDC_SRVMODEPORT), false);
				EnableWindow(GetDlgItem(g_hwnd, IDC_SERVERIP), false);
				EnableWindow(GetDlgItem(g_hwnd, IDC_SERVERPORT), false);
				EnableWindow(GetDlgItem(g_hwnd, IDC_NAME), false);
				EnableWindow(GetDlgItem(g_hwnd, IDC_CHATINPUT), true);
				EnableWindow(GetDlgItem(g_hwnd, IDC_MESSAGELIST), true);
				EnableWindow(GetDlgItem(g_hwnd, IDC_SEND), true);

				sprintf_s(g_sysTxt, "System: Logged in with UserID \" %d \" ", g_myUserId);
				insertData(g_sysTxt);
			}
			break;

		case MSGTYPE_LOGOUT_ACCEPT:
			break;

		case MSGTYPE_SRVSTOP:
			MessageBox(g_hwnd, "Server has shut down.", "Error", MB_OK);
			g_mainloop = false;

			EnableWindow(GetDlgItem(g_hwnd, IDC_SRVSTART), true);
			EnableWindow(GetDlgItem(g_hwnd, IDSTART), true);
			EnableWindow(GetDlgItem(g_hwnd, IDC_SRVMODEPORT), true);
			EnableWindow(GetDlgItem(g_hwnd, IDC_SERVERIP), true);
			EnableWindow(GetDlgItem(g_hwnd, IDC_SERVERPORT), true);
			EnableWindow(GetDlgItem(g_hwnd, IDC_NAME), true);
			EnableWindow(GetDlgItem(g_hwnd, IDC_CHATINPUT), false);
			EnableWindow(GetDlgItem(g_hwnd, IDC_MESSAGELIST), false);
			EnableWindow(GetDlgItem(g_hwnd, IDC_SEND), false);
			break;
		}
	}
}


bool startUdpChat_Server(void)
{
	g_mainloop = true;
	g_recvThread = thread(recvThread_Server);

	return true;
}

bool startUdpChat_Client(char* name)
{
	if (g_recvThread.joinable())
	{
		g_recvThread.join();
	}

	// �`���b�g�̋N��
	if (strlen(name) > NAME_LENGTH)
	{
		return false;
	}

	// Send Login Request
	requestLogin(name);

	// Copy name to sendbuff's Header
	//strcpy_s(g_sendbuff.msg.ChatBody.name, NAME_LENGTH, name);
	//insertMyData("Connected."); // Comment on Join

	// Start recvThread
	g_mainloop = true;
	g_recvThread = thread(recvThread_Client); // std:: not needed as using namespace std;
	return true;
}

void sendChatMessage(char* chat)
{
	SrvMsg sendbuff;
	// Send chatBuffer as chatMode
	sendbuff.msg.Header.type = SRVTYPE_CHAT;
	sendbuff.msg.ChatBody.userId = g_myUserId;
	// Copy content
	strcpy_s(sendbuff.msg.ChatBody.message, CHAT_LENGTH, chat);
	// Send content
	sendDataTo(g_mysock, sendbuff.data, sizeof(sendbuff.data), (sockaddr*)&g_toaddr);
}

void requestLogin(char* name)
{
	SrvMsg loginMsg;

	loginMsg.msg.Header.type = SRVTYPE_LOGIN;
	strcpy(loginMsg.msg.Login.name, name);

	sendDataTo(g_mysock, loginMsg.data, sizeof(loginMsg.data), (sockaddr*)&g_toaddr);
}

void endUdpChat(void)
{
	if (g_mysock != NULL)
	{
		if (g_serverFlag)
		{
			// Send msg to all connected users
			if (!g_connectedTable.empty())
			{
				g_sendbuff.msg.Header.type = MSGTYPE_SRVSTOP;

				for (map<int, sockaddr_in>::iterator i = g_connectedTable.begin();
					i != g_connectedTable.end();
					++i)
				{
					sendDataTo(g_mysock, g_sendbuff.data, sizeof(g_sendbuff.data), (sockaddr*)&(i->second));
				}
			}
		}
		else
		{
			SrvMsg logoutMsg;

			logoutMsg.msg.Header.type = SRVTYPE_LOGOUT;
			logoutMsg.msg.Logout.userId = g_myUserId;

			sendDataTo(g_mysock, logoutMsg.data, sizeof(logoutMsg.data), (sockaddr*)&g_toaddr);
		}
	}

	destructUdpChatSystem();
	destructWinSockSystem();
}