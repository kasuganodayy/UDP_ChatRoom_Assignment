#pragma once

#include "Constants.h"

/////////////////////
//クライアントサイド サーバからの受信メッセージ　クライアントはこれを受信し、SrvMsgを送信する。
////////////////////
#define MSGTYPE_CHAT	0		//チャットメッセージ
#define MSGTYPE_LOGIN	1		//ログイン通知
#define MSGTYPE_LOGOUT	2		//ログアウト通知
#define MSGTYPE_ACCEPT  3		//ログインOK
#define MSGTYPE_SRVENUM  4		//サーバ列挙応答（未実装）
#define MSGTYPE_SRVSTOP 255		//サーバ停止メッセージ

typedef struct _MSGHEADER		//クライアント側　受信　メッセージヘッダ
{
	short type;
	unsigned long timestamp;	//ソート用のタイムスタンプです。
}MSGHEADER, *LPMSGHEADER;

typedef struct _MSGBODY_CHAT		//チャットメッセージ（サーバから送られてくる）
{
	char name[NAME_LENGTH];		//他のユーザ名はサーバしか知らないのでサーバからの返信頼り
	char message[CHAT_LENGTH];
}MSGBODY_CHAT, *LPMSGBODY_CHAT;

typedef struct _MSGBODY_LOGINOUT	//ログイン＆アウトメッセージ。他ユーザの通知になっている。
{
	char name[NAME_LENGTH];
}MSGBODY_LOGINOUT, *LPMSGBODY_LOGINOUT;

typedef struct _MSGBODY_ACCEPT		//ログインOKメッセージ。ユーザIDをもらう。
{
	short userId;			//-1が返ってきている場合はログイン失敗
}MSGBODY_ACCEPT, *LPMSGBODY_ACCEPT;

typedef struct _MSGBODY_SRVENUM		//サーバ列挙モード取得メッセージ。実は未実装。
{
	unsigned long address;
	unsigned short port;
}MSGBODY_SRVENUM, *LPMSGBODY_SRVENUM;

typedef struct _MSGDATA			//クライアント受信メッセージ構造体
{
	MSGHEADER Header;
	union
	{
		MSGBODY_CHAT ChatBody;
		MSGBODY_LOGINOUT LoginoutBody;
		MSGBODY_ACCEPT AcceptBody;
		MSGBODY_SRVENUM SrvenumBody;
	};
}MSGDATA, *LPMSGDATA;

typedef struct _ChatMsgData		//char*とunion。
{
	union
	{
		MSGDATA msg;
		char	data[sizeof(MSGDATA)];
	};
}ChatMsgData, *LPChatMsgData;