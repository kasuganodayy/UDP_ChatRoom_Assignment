#pragma once

#include "Constants.h"

/////////////////////
//�N���C�A���g�T�C�h �T�[�o����̎�M���b�Z�[�W�@�N���C�A���g�͂������M���ASrvMsg�𑗐M����B
////////////////////
#define MSGTYPE_CHAT	0		//�`���b�g���b�Z�[�W
#define MSGTYPE_LOGIN	1		//���O�C���ʒm
#define MSGTYPE_LOGOUT	2		//���O�A�E�g�ʒm
#define MSGTYPE_ACCEPT  3		//���O�C��OK
#define MSGTYPE_SRVENUM  4		//�T�[�o�񋓉����i�������j
#define MSGTYPE_SRVSTOP 255		//�T�[�o��~���b�Z�[�W

typedef struct _MSGHEADER		//�N���C�A���g���@��M�@���b�Z�[�W�w�b�_
{
	short type;
	unsigned long timestamp;	//�\�[�g�p�̃^�C���X�^���v�ł��B
}MSGHEADER, *LPMSGHEADER;

typedef struct _MSGBODY_CHAT		//�`���b�g���b�Z�[�W�i�T�[�o���瑗���Ă���j
{
	char name[NAME_LENGTH];		//���̃��[�U���̓T�[�o�����m��Ȃ��̂ŃT�[�o����̕ԐM����
	char message[CHAT_LENGTH];
}MSGBODY_CHAT, *LPMSGBODY_CHAT;

typedef struct _MSGBODY_LOGINOUT	//���O�C�����A�E�g���b�Z�[�W�B�����[�U�̒ʒm�ɂȂ��Ă���B
{
	char name[NAME_LENGTH];
}MSGBODY_LOGINOUT, *LPMSGBODY_LOGINOUT;

typedef struct _MSGBODY_ACCEPT		//���O�C��OK���b�Z�[�W�B���[�UID�����炤�B
{
	short userId;			//-1���Ԃ��Ă��Ă���ꍇ�̓��O�C�����s
}MSGBODY_ACCEPT, *LPMSGBODY_ACCEPT;

typedef struct _MSGBODY_SRVENUM		//�T�[�o�񋓃��[�h�擾���b�Z�[�W�B���͖������B
{
	unsigned long address;
	unsigned short port;
}MSGBODY_SRVENUM, *LPMSGBODY_SRVENUM;

typedef struct _MSGDATA			//�N���C�A���g��M���b�Z�[�W�\����
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

typedef struct _ChatMsgData		//char*��union�B
{
	union
	{
		MSGDATA msg;
		char	data[sizeof(MSGDATA)];
	};
}ChatMsgData, *LPChatMsgData;