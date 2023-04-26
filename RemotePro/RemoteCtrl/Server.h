#pragma once
#include "Thread.h"
#include "SendQueue.h"
#include <map>
#include <MSWSock.h>
#include <ws2tcpip.h>


enum COperator
{
	ENone,
	EAccept,
	ERecv,
	ESend,
	EError
};

class CClient;
class CServer;

class COverlapped
{
public:
	OVERLAPPED m_overlapped;
	DWORD m_operator;//����
	std::vector<char>m_buffer;//������
	ThreadWorker m_worker;//������
	CServer* m_server;//����������
	CClient* m_client;//��Ӧ�Ŀͻ���
	WSABUF m_wsabuffer;
};


template<COperator>
class AcceptOverlapped :public COverlapped, ThreadFuncBase
{
public:
	AcceptOverlapped(CServer* psrv);
	int AcceptWorker();
};


typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;


template<COperator>
class RecvOverlapped :public COverlapped, ThreadFuncBase
{
public:
	RecvOverlapped();
	int RecvWorker();

};

typedef RecvOverlapped<ERecv> RECVOVERLAPPED;


template<COperator>
class SendOverlapped :public COverlapped, ThreadFuncBase
{
public:
	SendOverlapped();
	int SendWorker();
};
typedef SendOverlapped<ESend> SENDOVERLAPPED;


template<COperator>
class ErrorOverlapped :public COverlapped, ThreadFuncBase
{
public:
	ErrorOverlapped();
	int ErrorWorker();
};
typedef ErrorOverlapped<EError> ERROROVERLAPPED;




class CClient : public ThreadFuncBase
{
public:
	CClient(CServer* psrv);
	~CClient();
	void SetOverlapped(CClient* ptr);
	operator SOCKET();
	operator PVOID();
	operator LPOVERLAPPED();
	operator LPDWORD();
	LPWSABUF RecvWSABuffer();
	LPWSABUF SendWSABuffer();
	sockaddr_in* GetLocalAddr();
	sockaddr_in* GetRemoteddr();
	size_t GetBufferSize();
	DWORD& flags();
	int Recv();
	int Send(void* buffer, size_t nSize);
	int SendData(std::vector<char>& data);
private:
	SOCKET m_sock;
	DWORD m_received;
	DWORD m_flags;
	std::shared_ptr<ACCEPTOVERLAPPED> m_accept;
	std::shared_ptr<RECVOVERLAPPED> m_recv;
	std::shared_ptr<SENDOVERLAPPED> m_send;
	std::vector<char>m_buffer;
	size_t m_used;//�Ѿ�ʹ�õĻ�������С
	sockaddr_in m_laddr;//���ص�ַ
	sockaddr_in m_raddr;//Զ�̵�ַ
	bool m_isbusy;
	CSendQueue<std::vector<char>> m_vecSend;
};


class CServer : public ThreadFuncBase
{
public:
	CServer(const std::string& ip = "0.0.0.0", short port = 9527);
	~CServer();
	bool StartService();
	bool NewAccept();
	void BindNewSocket(SOCKET s);
private:
	void CreateSocket();
	int threadIocp();
	CThreadPool m_pool;
	HANDLE m_hIOCP;
	SOCKET m_sock;
	sockaddr_in m_addr;
	std::map<SOCKET, CClient*>m_client;
};