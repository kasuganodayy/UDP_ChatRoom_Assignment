#include "WinSockWrappers.h"
#include "wsock32error.h"

/*----------------------------------------------------------------------
	WinSock2.2 ������
		����
			�Ȃ�
		�߂�l
			bool true:���� false:���s
------------------------------------------------------------------------*/
bool WinSockInit()
{

	WSADATA		m_wd;						// �v�r�`�r�s�`�q�s�t�o�p
	WORD		requiredversion;			// 
	int			sts;						// �߂�l
	int			errcode;					// �\�P�b�g�̃G���[�R�[�h

	// ���̃v���O�������v������o�[�W����
	requiredversion = MAKEWORD(2,2);

	// �v�h�m�r�n�b�j������
	sts = WSAStartup(MAKEWORD(2,2),&m_wd);
	if(sts == SOCKET_ERROR)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
		return false;
	}

	// �o�[�W�����`�F�b�N
	if(m_wd.wVersion != requiredversion)
	{
		MessageBox(NULL,"VERSION ERROR!","",MB_OK);
		return false;
	}

	return true;

}

/*----------------------------------------------------------------------
	WinSock�I���֐�
		����
			�Ȃ�
		�߂�l
			�Ȃ�
------------------------------------------------------------------------*/
void WinSockClose()
{

	// �v�h�m�r�n�b�j�̌㏈��
	WSACleanup();

}

/*----------------------------------------------------------------------
	�t�c�o�������֐�
		����
			�Ȃ�
		�߂�l
			SOCKET
------------------------------------------------------------------------*/
SOCKET createUdpSocket()
{
	SOCKET		sock;
	int			errcode;					// �\�P�b�g�̃G���[�R�[�h

	// �\�P�b�g�쐬(UDP�v���g�R���p�̃\�P�b�g�쐬)
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock == INVALID_SOCKET){
		errcode = WSAGetLastError();
		errcom(errcode);
	}

	return sock;
}

/*----------------------------------------------------------------------
	sendTo���b�p
		����
			SOCKET*	sock
			const char* data
			int data_len
			sockaddr* to_addr
		�߂�l
			bool ���s����false
------------------------------------------------------------------------*/
bool sendDataTo(SOCKET sock, const char* data, int data_len, sockaddr* to_addr)
{
	// ���M
	int sts = sendto(sock, data, data_len, 0, to_addr ,sizeof(sockaddr));
	
	if(sts == SOCKET_ERROR)
	{
		errcom( WSAGetLastError() );
		return false;
	}
	return true;
}

/*----------------------------------------------------------------------
	recvFrom���b�p
		����
			SOCKET*	sock
			char* recvbuff
			int buff_len
			sockaddr* from_addr
			int addr_len
		�߂�l
			int SOCKET_ERROR�@�܂��͎�M�o�C�g��
------------------------------------------------------------------------*/
int recvDataFrom(SOCKET sock, char* recvbuff, int buff_len, sockaddr* from_addr)
{
	int addr_len = sizeof(sockaddr);

	int err;
	int sts = recvfrom(sock,					// �\�P�b�g�ԍ�
					recvbuff,					// ��M�o�b�t�@
					buff_len,					// ��M�o�b�t�@�o�C�g��
					0,	
					from_addr,					// ���M���A�h���X
					&addr_len);					// ��T�����̃o�C�g�����Z�b�g
	
	if(sts == SOCKET_ERROR)
	{
		err = WSAGetLastError();

		if (err != WSAEINTR)
		{
			errcom( err );
			return SOCKET_ERROR;
		}
	}

	return sts;
}

void setAddressToSockAddrIn(sockaddr_in* p_sockaddrin, unsigned long addr, unsigned short port_no)
{
	p_sockaddrin->sin_port = htons(port_no);
	p_sockaddrin->sin_family= AF_INET;
	p_sockaddrin->sin_addr.s_addr = addr;
}

/*----------------------------------------------------------------------
	�o�C���h
		����
			SOCKET*	sock
			sockaddr_in* p_sockaddrin
		�߂�l
			bool
------------------------------------------------------------------------*/
bool bindToSocket(SOCKET sock, sockaddr_in* p_sockaddrin)
{
	// �����̃\�P�b�g�ɂh�o�A�h���X�A�|�[�g�ԍ������蓖�Ă�
	if( bind(sock, (sockaddr*)p_sockaddrin, sizeof(sockaddr)) ==SOCKET_ERROR)
	{
		errcom( WSAGetLastError() );
		return false;
	}

	return true;
}

/*----------------------------------------------------------------------
	���~�e�b�h�u���[�h�L���X�g���[�h
		����
			SOCKET*	sock
			bool flg
		�߂�l
			bool
------------------------------------------------------------------------*/
bool setLimitedBroadCastMode(SOCKET sock, bool flg)
{
	// ���~�e�b�h�u���[�h�L���X�gON/OFF
	int sts = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&flg, sizeof(bool));
	
	if(sts == SOCKET_ERROR)
	{
		errcom( WSAGetLastError() );
		return false;
	}

	return true;
}

/*----------------------------------------------------------------------
	SOCKET�I���֐�
		����
			SOCKET*	sock
		�߂�l
			�Ȃ�
------------------------------------------------------------------------*/
void closeSocket(SOCKET* sock)
{
	if (*sock != INVALID_SOCKET)
	{
		closesocket(*sock);
		*sock = INVALID_SOCKET;
	}
}
