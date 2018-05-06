// UdpSocket.h : Defines the entry point for the console application.
//
#pragma once

#ifndef __UDPSOCKET_H_
#define __UDPSOCKET_H_

int Udp_InitSocket(SOCKET &out_socket);
int Udp_BindSocket(SOCKET in_socket, int nPort);
int Udp_SetSockAddrsIn(const char * ptServerAddr, int nServerPort, struct sockaddr_in & serverAddrIn);
int Udp_SendData(SOCKET in_socket, struct sockaddr_in *pin_sockaddr, unsigned char * pSendData, unsigned long ulSendDataSize);
int Udp_RecvData(SOCKET in_socket, struct sockaddr_in *pin_sockaddr, unsigned char * pRecvData, unsigned long ulRecvDataSize);
int Udp_ExitSocket(SOCKET &in_socket);

#endif //__UDPSOCKET_H_