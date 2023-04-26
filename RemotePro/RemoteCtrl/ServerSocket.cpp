#include "pch.h"
#include "ServerSocket.h"

CServerSocket* CServerSocket::m_instance = NULL;
CServerSocket::CHelper CServerSocket::m_helper;

CServerSocket* CServerSocket::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new CServerSocket();
	}
	return m_instance;
}

CServerSocket::CServerSocket()
{
	if ((!InitSockEnv())||(!InitSocket())) {
		MessageBox(NULL, _T("网络初始化失败"), _T("初始化失败"), MB_OK | MB_ICONERROR);
		exit(0);
	}
}

CServerSocket::CHelper::CHelper()
{
	getInstance();
}

CServerSocket::CHelper::~CHelper()
{
	if (m_instance) {
		CServerSocket* tmp = m_instance;
		m_instance = NULL;
		delete tmp;
	}
}

CServerSocket::~CServerSocket()
{
	closesocket(serv_sock);
	WSACleanup();
}

bool CServerSocket::InitSockEnv()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data))return false;
	//创建套接字
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	return true;
}

bool CServerSocket::InitSocket()
{
	if (serv_sock == -1)return false;
	//绑定
	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(6467);
	if (bind(serv_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)return false;
	//监听
	if(listen(serv_sock, 1)==-1)return false;
	return true;
}

bool CServerSocket::AcceptClient(){
	sockaddr_in client_adr;
	int cli_sz = sizeof(client_adr);
	client = accept(serv_sock,(sockaddr*)&client_adr,&cli_sz);
	if (client == -1)return false;
	return true;
}
#define BUFFER_SIZE 4096

int CServerSocket::DealCommand() {
	if (client == -1)return -1;
	char* buffer = new char[BUFFER_SIZE];
	if (buffer == NULL) {
		TRACE("内存不足\n");
		return -2;
	}
	memset(buffer,0, BUFFER_SIZE);
	int index{};
	while (1) {
		int len = recv(client, buffer+index, BUFFER_SIZE -index , 0);
		if (len <= 0) {
			delete[] buffer;
			return -1;
		}
		index += len;
		len = index;
		m_packet = CPacket((const BYTE*)buffer, len);
		if (len > 0) {
			memmove(buffer, buffer + len, BUFFER_SIZE -len);
			index -= len;
			delete[] buffer;
			return 0;
		}
	}
	delete[] buffer;
	return -1;
}


bool CServerSocket::Send(CPacket& pack) {
	if (client == -1)return false;
	return send(client,pack.Data(), pack.nLength + 6, 0) > 0;
}


int CServerSocket::StartRun(SOCKCALL mycallback,void* arg)
{
	if (!AcceptClient())return -1;
	if(DealCommand()<0)return -2;
	mycallback(m_packet,&list,arg);
	while (!list.empty()) {
		CPacket* pCPK = list.front();
		Send(*pCPK);
		list.pop_front();
		delete pCPK;
	}
	CloseClient();
	return 0;
}


void CServerSocket::CloseClient()
{
	closesocket(client);
	client = INVALID_SOCKET;
}