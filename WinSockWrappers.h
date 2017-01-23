#ifndef __WIN_SOCK_WRAPPERS__
#define __WIN_SOCK_WRAPPERS__

#include <winsock2.h>

bool WinSockInit();
void WinSockClose();
SOCKET createUdpSocket();
void closeSocket(SOCKET* sock);

bool sendDataTo(SOCKET sock, const char* data, int data_len, sockaddr* to_addr);
int recvDataFrom(SOCKET sock, char* recvbuff, int buff_len, sockaddr* from_addr);

void setAddressToSockAddrIn(sockaddr_in* p_sockaddrin, unsigned long addr, unsigned short port_no);
bool bindToSocket(SOCKET sock, sockaddr_in* p_sockaddrin);
bool setLimitedBroadCastMode(SOCKET sock, bool flg);

#endif