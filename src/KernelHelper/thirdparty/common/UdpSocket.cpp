// UdpSocket.cpp : Defines the entry point for the console application.
//
#include "CommonHelper.h"
#include "UdpSocket.h"

#include <process.h>
#include <winsock2.h>

#define LOCALHOST_IP_ADDR "127.0.0.1"
#define BROADCAST_IP_ADDR "255.255.255.255"
#define DEFAULT_SERVER_PORT 12721	//服务端口号

__inline static int Udp_InitSocket(SOCKET &out_socket)
{
	int nResult = 0;
	int nValue = 1L;
	//加载套接字库
	WORD wVersionRequested = 0;
	WSADATA wsaData = { 0 };

	wVersionRequested = MAKEWORD(2, 2);

	nResult = WSAStartup(wVersionRequested, &wsaData);
	if (nResult != 0) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		nResult = (-1);
		goto __LEAVE_EXIT;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		nResult = (-1);
		goto __LEAVE_EXIT;
	}

	//创建套接字
	out_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (out_socket == INVALID_SOCKET)
	{
		_tprintf(_T("socket() called failed! The error code is: %d\n"), WSAGetLastError());
		nResult = (-1);
		goto __LEAVE_EXIT;
	}
	else
	{
		_tprintf(_T("socket() called succesful!\n"));
		nResult = 0;
	}

	setsockopt(out_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&nValue, sizeof(nValue));
	setsockopt(out_socket, SOL_SOCKET, SO_BROADCAST, (const char *)&nValue, sizeof(nValue));
	
__LEAVE_EXIT:
	if (nResult)
	{
		WSACleanup();
	}
	return nResult;
}
__inline static int Udp_BindSocket(SOCKET in_socket, int nPort)
{
	int nResult = 0;

	//服务器端
	struct sockaddr_in in_sockaddr;
	in_sockaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	in_sockaddr.sin_family = AF_INET;
	in_sockaddr.sin_port = htons(nPort);

	//绑定套接字
	nResult = bind(in_socket, (struct sockaddr*)&in_sockaddr, sizeof(struct sockaddr));
	if (nResult == SOCKET_ERROR)
	{
#if defined(DEBUG) || defined(_DEBUG)
		_tprintf(_T("bind() called failed! The error code is: %d\n"), WSAGetLastError());
#endif
		nResult = (-1);
	}
	else
	{
#if defined(DEBUG) || defined(_DEBUG)
		_tprintf(_T("bind() called successful!\n"));
#endif
		nResult = 0;
	}

	return nResult;
}
__inline static int Udp_SetSockAddrsIn(const char * ptServerAddr, int nServerPort, struct sockaddr_in & serverAddrIn)
{
	serverAddrIn.sin_family = AF_INET;
	serverAddrIn.sin_port = htons(nServerPort);
	serverAddrIn.sin_addr.S_un.S_addr = inet_addr(ptServerAddr);
	return 0;
}
__inline static int Udp_SendData(SOCKET in_socket, struct sockaddr_in *pin_sockaddr, unsigned char * pSendData, unsigned long ulSendDataSize)
{
	int nResult = 0;

	//发送数据
	nResult = sendto(in_socket, (const char *)pSendData, ulSendDataSize, 0, (struct sockaddr*)pin_sockaddr, sizeof(struct sockaddr));
	if (nResult == SOCKET_ERROR)
	{
		_tprintf(_T("sendto() called failed! The error code is: %d\n"), WSAGetLastError());
		nResult = (-1);
	}
	else
	{
		_tprintf(_T("sendto() called successful!\n"));
		nResult = 0;
	}

	return nResult;
}
__inline static int Udp_RecvData(SOCKET in_socket, struct sockaddr_in *pin_sockaddr, unsigned char * pRecvData, unsigned long ulRecvDataSize)
{
	int nResult = 0;

	//等待并接收数据
	int nFromSize = sizeof(struct sockaddr);
	nResult = recvfrom(in_socket, (char *)pRecvData, ulRecvDataSize, 0, (struct sockaddr*)pin_sockaddr, &nFromSize);

#if defined(DEBUG) || defined(_DEBUG)
	//打印接收到的数据
	printf("receive data:%s from client [%s:%d]\n", pRecvData, inet_ntoa(pin_sockaddr->sin_addr), pin_sockaddr->sin_port);
#endif

	return nResult;
}
__inline static int Udp_ExitSocket(SOCKET &in_socket)
{
	int nResult = 0;
	if (in_socket != INVALID_SOCKET)
	{
		//关闭套接字
		closesocket(in_socket);
	}

	//终止套接字库的使用
	WSACleanup();

	return  nResult;
}