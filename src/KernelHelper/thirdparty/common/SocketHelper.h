#pragma once

#ifndef __SOCKETHELPER_H__
#define __SOCKETHELPER_H__

class CTcpSocketHandler
{
public:
	CTcpSocketHandler();
	~CTcpSocketHandler();

public:
#if 0
	static CTcpSocketHandler& GetInstance()
	{
		static CTcpSocketHandler instance;
		return instance;
	}
#endif

	int Init(const char *cServerAddr, int nServerPort, bool bAutoReConnect = false);
	
	static void RecvDataFromApp(void * p);
	int StartRecvDataThread();
	int RequestCommand(const char * pCommand, int nCommandLength);
	//std::wstring GetWebUrl(char * pUrl, std::wstring & wstrNickName);
	int Exit();

	void SetRecvFlag(bool bRecvFlag) {
		m_bRecvFlag = bRecvFlag;
	}

	bool GetRecvFlag() {
		return m_bRecvFlag;
	}
	void SetClientSocket(SOCKET s) {
		m_clientSocket = s;
	}
	SOCKET GetClientSocket() {
		return m_clientSocket;
	}
	void SetServerAddr(std::string strServerAddr) {
		m_strServerAddr = strServerAddr;
	}
	std::string GetServerAddr() {
		return m_strServerAddr;
	}
	void SetServerPort(int nServerPort) {
		m_nServerPort = nServerPort;
	}
	int GetServerPort() {
		return m_nServerPort;
	}
	void SetAutoReConnect(bool bAutoReConnect) {
		m_bAutoReConnect = bAutoReConnect;
	}
	bool GetAutoReConnect() {
		return m_bAutoReConnect;
	}
	void SetReConnecting(bool bReConnecting) {
		m_bReConnecting = bReConnecting;
	}
	bool GetReConnecting() {
		return m_bReConnecting;
	}

	struct sockaddr_in m_serverSockAddrIn;
	void ClientSockInitialize();//初始化
	void ClientSetSockOptions();//设置选项
	void ClientSetSockAddrsIn();//设置参数
	void ClientExecuteConnect();//执行连接

private:
	void ClientInit();

	std::string m_strServerAddr;
	int m_nServerPort;
	SOCKET m_clientSocket;
	SOCKET m_clientReConnectSocket;
	bool m_bRecvFlag;//false-关闭，true-运行
	bool m_bAutoReConnect;//是否自动重连，true-自动重连，false-不自动重连
	bool m_bReConnecting;//是否正在重连，true-是，false-否
};

class CUdpSocketHandler{
public:
	typedef struct tagSocketPacket{
		int ssize;//结构体大小
		SOCKET s;
		struct sockaddr_in si;//接入的连接信息
		int port;//端口
		void * data;//数据指针
		int size;//数据大小
	}SOCKETPACKET, *PSOCKETPACKET;

	SOCKET udp_start(int nPort);

	void udp_setsockaddrsin()//设置参数
	{

	}
	static DWORD WINAPI udp_recvdata_thread(void *p);
	static DWORD WINAPI udp_senddata_thread(void *p);

};
#endif // __SOCKETHELPER_H__