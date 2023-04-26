#include "CCPacket.h"
#include "ClientSocket.h"
#include "pch.h"
#include "ClientController.h"

#define BUFFER_SIZE 4048000
CClientSocket* CClientSocket::getinstance = NULL;
CClientSocket::CHelper CClientSocket::chelper;

CClientSocket* CClientSocket::GetInstance() {
	if (!getinstance)getinstance = new CClientSocket;
	return getinstance;
}

CClientSocket::CClientSocket()
{
	if (!InitSockEnv()) {
		MessageBox(NULL, _T("初始化套接字错误"), _T("初始化错误"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_buffer.resize(BUFFER_SIZE);
	memset(m_buffer.data(), 0, BUFFER_SIZE);
}

BOOL CClientSocket::InitSockEnv() {
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data))return false;
	return true;
}

CClientSocket::~CClientSocket()
{
	//AfxMessageBox(L"被销毁");
	closesocket(Cli_sock);
	WSACleanup();
}

CClientSocket::CHelper::CHelper() {
	GetInstance();
}

CClientSocket::CHelper::~CHelper() {
	if (getinstance)delete getinstance;
	getinstance = NULL;
}
std::string GetErrorInfo(int wsaErrCode) {
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEPALI, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}

bool CClientSocket::InitSocket(int IPAddr,int nPort)
{
	Cli_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (Cli_sock == -1)return false;
	sockaddr_in Ser_addr;
	memset(&Ser_addr, 0, sizeof(sockaddr_in));
	Ser_addr.sin_family = AF_INET;
	Ser_addr.sin_addr.S_un.S_addr = htonl(IPAddr);
	Ser_addr.sin_port = htons(nPort);
	if (Ser_addr.sin_addr.s_addr == INADDR_NONE) {
		TRACE("指定Ip不存在");
		return false;
	}
	int ret = connect(Cli_sock, (const sockaddr*)&Ser_addr, sizeof(Ser_addr));
	if (ret == -1) {
		TRACE("连接失败，错误码%d", WSAGetLastError());
		return false;
		
	}
	return true;

}



int CClientSocket::DealCommand() {
	if (Cli_sock == -1)return -1;
	char* buffer = m_buffer.data();
	
	static int index{};
	while (1) {
		int len = recv(Cli_sock, buffer + index, BUFFER_SIZE - index, 0);
		if (len <= 0&&(index==0)) return -1;
		index += len;
		len = index;
		CClientController::getInstance()->m_packet = CPacket((BYTE*)buffer, len);
		if (len > 0) {
			memmove(buffer, buffer + len, index - len);
			index -= len;
			return CClientController::getInstance()->m_packet.sCmd;
		}
	}
	return -1;
}

bool CClientSocket::Send(const char* pData, size_t nSize) {
	if (Cli_sock == -1)return false;
	return send(Cli_sock, pData, nSize, 0) > 0;
}

bool CClientSocket::Send(CPacket& pack) {
	if (Cli_sock == -1)return false;
	return send(Cli_sock, pack.Data(), pack.nLength + 6, 0) > 0;
}

void CClientSocket::CloseSocket() {
	closesocket(Cli_sock);
}

