#include "WinSockWrappers.h"
#include "wsock32error.h"

/*----------------------------------------------------------------------
	WinSock2.2 初期化
		引数
			なし
		戻り値
			bool true:成功 false:失敗
------------------------------------------------------------------------*/
bool WinSockInit()
{

	WSADATA		m_wd;						// ＷＳＡＳＴＡＲＴＵＰ用
	WORD		requiredversion;			// 
	int			sts;						// 戻り値
	int			errcode;					// ソケットのエラーコード

	// このプログラムが要求するバージョン
	requiredversion = MAKEWORD(2,2);

	// ＷＩＮＳＯＣＫ初期化
	sts = WSAStartup(MAKEWORD(2,2),&m_wd);
	if(sts == SOCKET_ERROR)
	{
		errcode = WSAGetLastError();
		errcom(errcode);
		return false;
	}

	// バージョンチェック
	if(m_wd.wVersion != requiredversion)
	{
		MessageBox(NULL,"VERSION ERROR!","",MB_OK);
		return false;
	}

	return true;

}

/*----------------------------------------------------------------------
	WinSock終了関数
		引数
			なし
		戻り値
			なし
------------------------------------------------------------------------*/
void WinSockClose()
{

	// ＷＩＮＳＯＣＫの後処理
	WSACleanup();

}

/*----------------------------------------------------------------------
	ＵＤＰ初期化関数
		引数
			なし
		戻り値
			SOCKET
------------------------------------------------------------------------*/
SOCKET createUdpSocket()
{
	SOCKET		sock;
	int			errcode;					// ソケットのエラーコード

	// ソケット作成(UDPプロトコル用のソケット作成)
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock == INVALID_SOCKET){
		errcode = WSAGetLastError();
		errcom(errcode);
	}

	return sock;
}

/*----------------------------------------------------------------------
	sendToラッパ
		引数
			SOCKET*	sock
			const char* data
			int data_len
			sockaddr* to_addr
		戻り値
			bool 失敗時はfalse
------------------------------------------------------------------------*/
bool sendDataTo(SOCKET sock, const char* data, int data_len, sockaddr* to_addr)
{
	// 送信
	int sts = sendto(sock, data, data_len, 0, to_addr ,sizeof(sockaddr));
	
	if(sts == SOCKET_ERROR)
	{
		errcom( WSAGetLastError() );
		return false;
	}
	return true;
}

/*----------------------------------------------------------------------
	recvFromラッパ
		引数
			SOCKET*	sock
			char* recvbuff
			int buff_len
			sockaddr* from_addr
			int addr_len
		戻り値
			int SOCKET_ERROR　または受信バイト数
------------------------------------------------------------------------*/
int recvDataFrom(SOCKET sock, char* recvbuff, int buff_len, sockaddr* from_addr)
{
	int addr_len = sizeof(sockaddr);

	int err;
	int sts = recvfrom(sock,					// ソケット番号
					recvbuff,					// 受信バッファ
					buff_len,					// 受信バッファバイト長
					0,	
					from_addr,					// 送信元アドレス
					&addr_len);					// 第５引数のバイト長をセット
	
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
	バインド
		引数
			SOCKET*	sock
			sockaddr_in* p_sockaddrin
		戻り値
			bool
------------------------------------------------------------------------*/
bool bindToSocket(SOCKET sock, sockaddr_in* p_sockaddrin)
{
	// 自分のソケットにＩＰアドレス、ポート番号を割り当てる
	if( bind(sock, (sockaddr*)p_sockaddrin, sizeof(sockaddr)) ==SOCKET_ERROR)
	{
		errcom( WSAGetLastError() );
		return false;
	}

	return true;
}

/*----------------------------------------------------------------------
	リミテッドブロードキャストモード
		引数
			SOCKET*	sock
			bool flg
		戻り値
			bool
------------------------------------------------------------------------*/
bool setLimitedBroadCastMode(SOCKET sock, bool flg)
{
	// リミテッドブロードキャストON/OFF
	int sts = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&flg, sizeof(bool));
	
	if(sts == SOCKET_ERROR)
	{
		errcom( WSAGetLastError() );
		return false;
	}

	return true;
}

/*----------------------------------------------------------------------
	SOCKET終了関数
		引数
			SOCKET*	sock
		戻り値
			なし
------------------------------------------------------------------------*/
void closeSocket(SOCKET* sock)
{
	if (*sock != INVALID_SOCKET)
	{
		closesocket(*sock);
		*sock = INVALID_SOCKET;
	}
}
