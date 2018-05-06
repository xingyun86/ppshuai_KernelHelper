// TcpSocket.h : Defines the entry point for the console application.
//
#pragma once

#ifndef __TCPSOCKET_H_
#define __TCPSOCKET_H_

#define DEFAULT_PORT "27015"

#define DEFAULT_SEND_TIMEOUT	10000 //发送默认超时为10s
#define DEFAULT_RECV_TIMEOUT	10000 //接收默认超时为10s


//检测是否为有效的IP地址
__inline static int inet_aton(const char *cp, struct in_addr *inaddr)
{
	int dots = 0;  //2
	register u_long addr = 0;
	register u_long val = 0, base = 10;
	do
	{
		register char c = *cp;
		switch (c)
		{
		case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			val = (val * base) + (c - '0');
			break;
		case '.':
			if (++dots > 3)
				return 0;
		case '\0':
			if (val > 255)
				return 0;
			addr = addr << 8 | val;
			val = 0;
			break;
		default:
			return 0;
		}
	} while (*cp++);
	if (dots < 3)
		addr <<= 8 * (3 - dots);
	if (inaddr)
		inaddr->s_addr = htonl(addr);
	return 1;
}

///////////////////////////////////////////////////////////////
//作用：将网络地址转化为IP
//参数：ipbuf是输出缓冲区, host是要转化的域名, maxlen是缓冲区大小
//返回值：返回-1是失败，0是成功
__inline static int get_ip_from_host(char *ipbuf, const char *host, int maxlen)
{
	WSADATA wsaData = { 0 };
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) //调用Windows Sockets DLL
	{
#if defined(DEBUG) || defined(_DEBUG)
		printf("winsock cannot initialize!%s\n", strerror(errno));
#endif
		WSACleanup();
		return 0;
	}
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;

	if (inet_aton(host, &sa.sin_addr) == 0)
	{
		struct hostent *h;
		h = gethostbyname(host);
		if (h == NULL)
			return -1;
		memcpy(&sa.sin_addr, h->h_addr, sizeof(struct in_addr));
	}
	strncpy(ipbuf, inet_ntoa(sa.sin_addr), maxlen);

	WSACleanup();

	return 0;
}

SOCKET Tcp_SockInitialize();

int Tcp_SetSockOptions(SOCKET connectSocket, DWORD dwSendTimeOut = DEFAULT_SEND_TIMEOUT,
						DWORD dwRecvTimeOut = DEFAULT_SEND_TIMEOUT, BOOL bLinger = FALSE);

int Tcp_SetSockAddrsIn(const char * ptServerAddr, int nServerPort, struct sockaddr_in & serverAddrIn);

SOCKET Tcp_Connect(SOCKET connectSocket, struct sockaddr_in & serverAddrIn);

SOCKET Tcp_Init(const char * ptServerAddr, int nServerPort, DWORD dwSendTimeOut = DEFAULT_SEND_TIMEOUT,
					DWORD dwRecvTimeOut = DEFAULT_RECV_TIMEOUT, BOOL bLinger = FALSE);

int Tcp_Read(SOCKET s, void * data, int size);

int Tcp_Write(SOCKET s, const void * data, int size);

int Tcp_Send(SOCKET s, const char * data, int size);

int Tcp_Recv(SOCKET s, char * data, int size);

void Tcp_Exit(SOCKET s);

#endif //__TCPSOCKET_H_
