#ifndef __UDP_CHAT_MESSAGE_STRUCT_2014__
#define __UDP_CHAT_MESSAGE_STRUCT_2014__

#include "Constants.h"

/////////////////////
//�T�[�o�T�C�h �N���C�A���g����̑��M���b�Z�[�W�@�T�[�o�͂������M���AChatMsgData�𑗐M����B
////////////////////
#define SRVTYPE_LOGIN 0		//���O�C���v����M
#define SRVTYPE_LOGOUT 1	//���O�A�E�g�v����M
#define SRVTYPE_CHAT 2		//�`���b�g����M

#define SRVTYPE_ASK 101		//�T�[�o��ԃ`�F�b�N�v���@������
#define SRVTYPE_ENUM 102	//�T�[�o�񋓗v���@������

typedef struct _SRVHEADER	//�T�[�o���b�Z�[�W�w�b�_
{
	short type;
}SRVHEADER, *LPSRVHEADER;

typedef struct _SRVBODY_LOGIN	//���O�C���v���B�\�P�b�g�A�h���X�̏d���`�F�b�N�͂��Ă��܂��B
{
	char name[NAME_LENGTH];
}SRVBODY_LOGIN, *LPSRVBODY_LOGIN;

typedef struct _SRVBODY_LOGOUT	//���O�A�E�g�v���B�������userId�Ȃ̂Œ��ӁB
{
	short userId;
}SRVBODY_LOGOUT, *LPSRVBODY_LOGOUT;

typedef struct _SRVBODY_CHAT	//�`���b�g���b�Z�[�W��M�B���b�Z�[�W�ɂ́u���O���܂܂�Ȃ��v
{
	int userId;
	char message[CHAT_LENGTH];
}SRVBODY_CHAT, *LPSRVBODY_CHAT;

typedef struct _SRVMESSAGE	//�T�[�o���b�Z�[�W�\����
{
	SRVHEADER Header;

	union
	{
		SRVBODY_LOGIN Login;
		SRVBODY_LOGOUT Logout;
		SRVBODY_CHAT ChatBody;
	};
}SRVMESSAGE, *LPSRVMESSAGE;

typedef struct _SrvMsg		//char*�Ƃ�union�p
{
	union
	{
		char data[sizeof(_SRVMESSAGE)];
		SRVMESSAGE msg;
	};
}SrvMsg, *LPSrvMsg;


#endif