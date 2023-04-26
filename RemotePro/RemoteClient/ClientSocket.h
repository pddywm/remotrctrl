#pragma once
#include "CCPacket.h"
#include <string>
#include <vector>

class CClientSocket
{
public:
	static CClientSocket* GetInstance();
	bool InitSocket(int IPAddr, int nPort);
	bool Send(const char* pData, size_t nSize);
	bool Send(CPacket& packet);
	int DealCommand();
	void CloseSocket();
private:
	CClientSocket();
	CClientSocket(CClientSocket& ss);
	~CClientSocket();
	BOOL InitSockEnv();
	class CHelper{
	public:
		CHelper();
		~CHelper();
	};
	static CHelper chelper;
	static CClientSocket* getinstance;
	std::vector<char> m_buffer;
	SOCKET Cli_sock;
};




