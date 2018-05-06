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
	void ClientSockInitialize();//��ʼ��
	void ClientSetSockOptions();//����ѡ��
	void ClientSetSockAddrsIn();//���ò���
	void ClientExecuteConnect();//ִ������

private:
	void ClientInit();

	std::string m_strServerAddr;
	int m_nServerPort;
	SOCKET m_clientSocket;
	SOCKET m_clientReConnectSocket;
	bool m_bRecvFlag;//false-�رգ�true-����
	bool m_bAutoReConnect;//�Ƿ��Զ�������true-�Զ�������false-���Զ�����
	bool m_bReConnecting;//�Ƿ�����������true-�ǣ�false-��
};

class CUdpSocketHandler{
public:
	typedef struct tagSocketPacket{
		int ssize;//�ṹ���С
		SOCKET s;
		struct sockaddr_in si;//�����������Ϣ
		int port;//�˿�
		void * data;//����ָ��
		int size;//���ݴ�С
	}SOCKETPACKET, *PSOCKETPACKET;

	SOCKET udp_start(int nPort);

	void udp_setsockaddrsin()//���ò���
	{

	}
	static DWORD WINAPI udp_recvdata_thread(void *p);
	static DWORD WINAPI udp_senddata_thread(void *p);

};
#endif // __SOCKETHELPER_H__