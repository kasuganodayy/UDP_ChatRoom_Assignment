#ifndef __UDP_CHAT_MESSAGE_STRUCT_2014__
#define __UDP_CHAT_MESSAGE_STRUCT_2014__

#include "Constants.h"

/////////////////////
//サーバサイド クライアントからの送信メッセージ　サーバはこれを受信し、ChatMsgDataを送信する。
////////////////////
#define SRVTYPE_LOGIN 0		//ログイン要求受信
#define SRVTYPE_LOGOUT 1	//ログアウト要求受信
#define SRVTYPE_CHAT 2		//チャット文受信

#define SRVTYPE_ASK 101		//サーバ状態チェック要求　未実装
#define SRVTYPE_ENUM 102	//サーバ列挙要求　未実装

typedef struct _SRVHEADER	//サーバメッセージヘッダ
{
	short type;
}SRVHEADER, *LPSRVHEADER;

typedef struct _SRVBODY_LOGIN	//ログイン要求。ソケットアドレスの重複チェックはしています。
{
	char name[NAME_LENGTH];
}SRVBODY_LOGIN, *LPSRVBODY_LOGIN;

typedef struct _SRVBODY_LOGOUT	//ログアウト要求。こちらはuserIdなので注意。
{
	short userId;
}SRVBODY_LOGOUT, *LPSRVBODY_LOGOUT;

typedef struct _SRVBODY_CHAT	//チャットメッセージ受信。メッセージには「名前が含まれない」
{
	int userId;
	char message[CHAT_LENGTH];
}SRVBODY_CHAT, *LPSRVBODY_CHAT;

typedef struct _SRVMESSAGE	//サーバメッセージ構造体
{
	SRVHEADER Header;

	union
	{
		SRVBODY_LOGIN Login;
		SRVBODY_LOGOUT Logout;
		SRVBODY_CHAT ChatBody;
	};
}SRVMESSAGE, *LPSRVMESSAGE;

typedef struct _SrvMsg		//char*とのunion用
{
	union
	{
		char data[sizeof(_SRVMESSAGE)];
		SRVMESSAGE msg;
	};
}SrvMsg, *LPSrvMsg;


#endif