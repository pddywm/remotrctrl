#include "pch.h"
#include "Server.h"
#pragma warning(disable:4407)



template<COperator op>
AcceptOverlapped<op>::AcceptOverlapped(CServer* psrv)
{
	m_worker = ThreadWorker(this, (FUNCTYPE)&AcceptOverlapped<op>::AcceptWorker);
	m_operator = EAccept;
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024);
	m_server = psrv;
}

template<COperator op>
int AcceptOverlapped<op>::AcceptWorker()
{
	INT lLength = 0;
	INT rLength = 0;
	if (m_client->GetBufferSize() > 0)
	{
		sockaddr* plocal = NULL;
		sockaddr* premote = NULL;
		GetAcceptExSockaddrs(*m_client, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (sockaddr**)&plocal, &lLength, (sockaddr**)&premote, &rLength);
		memcpy(m_client->GetLocalAddr(), plocal, sizeof(sockaddr_in));
		memcpy(m_client->GetRemoteddr(), premote, sizeof(sockaddr_in));
		m_server->BindNewSocket(*m_client);
		int ret = WSARecv((SOCKET)*m_client, m_client->RecvWSABuffer(), 1, *m_client, &m_client->flags(), *m_client,NULL);
		
		//if ((ret == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
		//{
			//TRACE("ret = %d error = %d\r\n", ret, WSAGetLastError());
		//}
		//if (!m_server->NewAccept())
		//{
			//return -2;
		//}
	}
	return -1;
}

template<COperator op>
RecvOverlapped<op>::RecvOverlapped()
{
	m_operator = op;
	m_worker = ThreadWorker(this, (FUNCTYPE)&RecvOverlapped<op>::RecvWorker);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024 * 256);
}

template<COperator op>
int RecvOverlapped<op>::RecvWorker()
{
	int ret = m_client->Recv();
	return ret;
}


template<COperator op>
SendOverlapped<op>::SendOverlapped()
{
	m_operator = op;
	m_worker = ThreadWorker(this, (FUNCTYPE)&SendOverlapped<op>::SendWorker);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024 * 256);
}

template<COperator op>
int SendOverlapped<op>::SendWorker()
{
	return -1;
}

template<COperator op>
ErrorOverlapped<op>::ErrorOverlapped():m_operator(EError), m_worker(this, &ErrorOverlapped::ErrorWorker)
{
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024);
}



CClient::CClient(CServer* psrv) : m_isbusy(false), m_flags(0),
m_accept(new ACCEPTOVERLAPPED(psrv)),
m_recv(new RECVOVERLAPPED()),
m_send(new SENDOVERLAPPED()),          
m_vecSend(this, (SENDCALLBACK)&CClient::SendData)
{
	m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_buffer.resize(1024);
	memset(&m_laddr, 0, sizeof(m_laddr));
	memset(&m_raddr, 0, sizeof(m_raddr));
}

CClient::~CClient(){}

void CClient::SetOverlapped(CClient* ptr)
{
	m_accept->m_client = ptr;
	m_recv->m_client = ptr;
	m_send->m_client = ptr;
}


CClient::operator SOCKET() {
	return m_sock;
}
CClient::operator PVOID() {
	return &m_buffer[0];
}
CClient::operator LPDWORD() {
	return &m_received;
}

CClient::operator LPOVERLAPPED()
{
	return &m_accept->m_overlapped;
}


LPWSABUF CClient::RecvWSABuffer()
{
	return &m_recv->m_wsabuffer;
}

LPWSABUF CClient::SendWSABuffer()
{
	return &m_send->m_wsabuffer;
}

sockaddr_in* CClient::GetLocalAddr() {
	return &m_laddr;
}

sockaddr_in* CClient::GetRemoteddr() {
	return &m_raddr;
}

size_t CClient::GetBufferSize() {
	return m_buffer.size();
}

int CClient::Recv()
{
	int ret = recv(m_sock, m_buffer.data() + m_used, m_buffer.size() - m_used, 0);
	if (ret <= 0)
	{
		return -1;
	}
	m_used += (size_t)ret;
	return 0;
}

int CClient::Send(void* buffer, size_t nSize)
{
	std::vector<char> data(nSize);
	memcpy(data.data(), buffer, nSize);
	if (m_vecSend.PushBack(data))
		return 0;
	return -1;
}

int CClient::SendData(std::vector<char>& data)
{
	return -1;
}

DWORD& CClient::flags() {
	return m_flags;
}

CServer::CServer(const std::string& ip, short port) :m_pool(10)
{
	m_hIOCP = INVALID_HANDLE_VALUE;
	m_sock = INVALID_SOCKET;
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr.s_addr);
}

CServer::~CServer(){}


bool CServer::StartService()
{
	CreateSocket();
	if (bind(m_sock, (sockaddr*)&m_addr, sizeof(m_addr)) == -1)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return false;
	}
	if (listen(m_sock, 3) == -1)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return false;
	}
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
	if (m_hIOCP == NULL)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		m_hIOCP = INVALID_HANDLE_VALUE;
		return false;
	}
	CreateIoCompletionPort((HANDLE)m_sock, m_hIOCP, (ULONG_PTR)this, 0);
	m_pool.Invoke();
	m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&CServer::threadIocp));
	if (!NewAccept()) return false;
	return true;
}

bool CServer::NewAccept()
{
	CClient* pClient = new CClient(this);
	pClient->SetOverlapped(pClient);
	m_client.insert(std::pair<SOCKET, CClient*>(*pClient, pClient));
	if (!AcceptEx(m_sock, *pClient, *pClient, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, *pClient, *pClient))
	{
		TRACE("%d\r\n", WSAGetLastError());
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			m_hIOCP = INVALID_HANDLE_VALUE;
			return false;
		}
	}
	return true;
}

void CServer::CreateSocket()
{
	m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	int opt = 1;
	setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
}

int CServer::threadIocp()
{
	while (1) {
		DWORD tranferred = 0;
		ULONG_PTR CompletionKey = 0;
		OVERLAPPED* lpOverlapped = NULL;
		GetQueuedCompletionStatus(m_hIOCP, &tranferred, &CompletionKey, &lpOverlapped, INFINITE);
		if (tranferred >= 0 && CompletionKey != 0)
		{
			COverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, COverlapped, m_overlapped);
			switch (pOverlapped->m_operator)
			{
			case EAccept:
			{
				ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
			}
			break;
			case ERecv:
			{
				RECVOVERLAPPED* pOver = (RECVOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
			}
			break;
			case ESend:
			{
				SENDOVERLAPPED* pOver = (SENDOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
			}
			break;
			case EError:
			{
				ERROROVERLAPPED* pOver = (ERROROVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
			}
			break;
			}
		}
	}
	return 1;
}


void CServer::BindNewSocket(SOCKET s)
{
	CreateIoCompletionPort((HANDLE)s, m_hIOCP, (ULONG_PTR)this, 0);
}



