#include "resource.h"
#include "WinSockWrappers.h"

#include "UdpChatCore.h"
#include "ServerMessageStruct.h"
#include "ClientMessageStruct.h"
#include <thread>
#include <time.h>

#include <list>
#include <map>

using namespace std;


//--------------------------------
// ライブラリ
//--------------------------------
#pragma	comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

//--------------------------------
// 構造体宣言
//--------------------------------
typedef struct _ChatBuffer
{
	char* chat_string;
	int	buff_size;
}ChatBuffer, *LPChatBuffer;

using namespace std;

//--------------------------------
// グローバル変数
//--------------------------------
HWND g_hwnd;
SOCKET g_mysock;
sockaddr_in g_myaddr;
sockaddr_in g_toaddr;

thread g_recvThread;

bool g_mainloop;

//メッセージ保存用可変長配列
list<LPChatBuffer> g_messages;

ChatMsgData g_sendbuff;

// UserID -> name table
map <int, string> userTable;


//--------------------------------
// プロトタイプ
//--------------------------------
void recvThread_Server(void);
void recvThread_Client(void);

void setMessageToBuffer(LPChatMsgData msg, LPChatBuffer buffer);
void makeChatString(LPChatBuffer buffer, int userId, char* message);
void insertData(LPChatBuffer chat_buff);
void deleteChatData(LPChatBuffer buffer);
void requestLogin(char* name);


bool initUdpChatSystem(HWND hwnd, char* ip, char* port)
{
	bool serverFlag = (ip ? false : true);

	unsigned short portNo;

	portNo = strtol(port, NULL, 10);

	g_hwnd = hwnd;

	initWinSockSystem();
	g_mysock = createUdpSocket();

	if (g_mysock != INVALID_SOCKET)
	{
		if (serverFlag)
		{
			setAddressToSockAddrIn(&g_myaddr, INADDR_ANY, portNo);
			setAddressToSockAddrIn(&g_toaddr, INADDR_ANY, portNo);

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
		//setAddressToSockAddrIn(&g_toaddr, INADDR_BROADCAST, portNo);

	}
		
	return false;
}

void destructUdpChatSystem()
{
	userTable.clear();

	if (g_mainloop)
	{
		g_mainloop = false;
		insertMyData("退室しました。");
	}

	closeSocket(&g_mysock);

	if (g_recvThread.joinable())
	{
		g_recvThread.join();
	}
}

bool initWinSockSystem()
{
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
	//ToDo: GUI更新処理
	// 受信したチャットデータをListBoxに追加。
	// ListBox等の細かい制御はSendMessageで行う。
	SendMessage(GetDlgItem(g_hwnd, IDC_MESSAGELIST), LB_INSERTSTRING, (WPARAM)0, (LPARAM)chat_buff->chat_string);
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
			// 受信メッセージ反映処理
			// Allocate memory for LPChatBuffer
			message_data = (LPChatBuffer)malloc(sizeof(ChatBuffer));

			// Allocate memory and set CharBuffer
			setMessageToBuffer(&recvbuff, message_data);

			insertData(message_data);
			deleteChatData(message_data);
			break;

		case SRVTYPE_LOGIN:
			if (strlen(recvbuff.msg.Login.name) > NAME_LENGTH)
			{
				// Reject login
				g_sendbuff.msg.AcceptBody.userId = -1;
				g_sendbuff.msg.Header.type = MSGTYPE_ACCEPT;
			}
			else
			{
				int tempID = rand();
				while (userTable.find(tempID) != userTable.end())
				{
					static short userGenCount = 0;

					if (userGenCount > 100)
					{
						g_sendbuff.msg.AcceptBody.userId = -1;
						g_sendbuff.msg.Header.type = MSGTYPE_ACCEPT;
						strcpy(g_sendbuff.msg.ChatBody.message, "UserID attempts over-count.");
						break;
					}

					tempID = rand();
					userGenCount++;
				}
			}
			break;

		case SRVTYPE_LOGOUT:
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
	// チャットの起動
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
	// Send chatBuffer as chatMode
	g_sendbuff.msg.Header.type = MSGTYPE_CHAT;
	// Copy content
	strcpy_s(g_sendbuff.msg.ChatBody.message, CHAT_LENGTH, chat);

	sendDataTo(g_mysock, g_sendbuff.data, sizeof(g_sendbuff.data), (sockaddr*)&g_toaddr);

}

void requestLogin(char* name)
{
	SrvMsg loginMsg;

	loginMsg.msg.Header.type = SRVTYPE_LOGIN;
	strcpy(loginMsg.msg.Login.name, name);

	sendDataTo(g_mysock, loginMsg.data, sizeof(loginMsg.data), (sockaddr*)&g_toaddr);
}