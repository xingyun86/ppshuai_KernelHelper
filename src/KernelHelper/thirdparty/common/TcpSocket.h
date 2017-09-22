// TcpSocket.h : Defines the entry point for the console application.
//


#include <windows.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#define DEFAULT_PORT "27015"

#define DEFAULT_SEND_TIMEOUT	10000 //发送默认超时为10s
#define DEFAULT_RECV_TIMEOUT	10000 //接收默认超时为10s

//检测是否为有效的IP地址
int inet_aton (const char *cp, struct in_addr *inaddr);
///////////////////////////////////////////////////////////////
//作用：将网络地址转化为IP
//参数：ipbuf是输出缓冲区, host是要转化的域名, maxlen是缓冲区大小
//返回值：返回-1是失败，0是成功
int get_ip_from_host(char *ipbuf, const char *host, int maxlen);

SOCKET Tcp_SockInitialize();

int Tcp_SetSockOptions(SOCKET connectSocket, DWORD dwSendTimeOut = DEFAULT_SEND_TIMEOUT,
						DWORD dwRecvTimeOut = DEFAULT_SEND_TIMEOUT, BOOL bLinger = FALSE);

void Tcp_SetSockAddrInf(const char * ptServerAddr, int nServerPort, struct sockaddr_in & serverAddrIn);

SOCKET Tcp_Connect(SOCKET connectSocket, struct sockaddr_in & serverAddrIn);

SOCKET Tcp_Init(const char * ptServerAddr, int nServerPort, DWORD dwSendTimeOut = DEFAULT_SEND_TIMEOUT,
					DWORD dwRecvTimeOut = DEFAULT_RECV_TIMEOUT, BOOL bLinger = FALSE);

int Tcp_Read(SOCKET s, void * data, int size);

int Tcp_Write(SOCKET s, const void * data, int size);

int Tcp_Send(SOCKET s, const char * data, int size);

int Tcp_Recv(SOCKET s, char * data, int size);

void Tcp_Exit(SOCKET s);

