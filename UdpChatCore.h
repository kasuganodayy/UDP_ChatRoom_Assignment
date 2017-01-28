#ifndef __UDPCHATCORE_2014__
#define __UDPCHATCORE_2014__
extern bool debugFlag;
extern bool g_serverFlag;

bool initWinSockSystem();
bool initUdpChatSystem(HWND hwnd, char* ip, char* port);

void destructUdpChatSystem();
void destructWinSockSystem();

bool startUdpChat_Server(void);
bool startUdpChat_Client(char* name);
void sendChatMessage(char* chat);

void endUdpChat(void);

void insertMyData(char* my_message);

#endif
