#pragma once
#include "pch.h"
#include "framework.h"
#include <list>
#include "CCPacket.h"

typedef int (*SOCKCALL)(CPacket& packet,std::list<CPacket*>* plist, void* args);

class CServerSocket
{
public:
	static CServerSocket* getInstance();//��ȡ����ָ��
	int StartRun(SOCKCALL mycallback,void* arg);//��ʼ����
private://�������
	CServerSocket();
	~CServerSocket();
	CServerSocket(const CServerSocket& ss);
	CServerSocket& operator=(const CServerSocket& ss) {};
	class CHelper {
	public:
		CHelper();
		~CHelper();
	};
	static CHelper m_helper;
	static CServerSocket* m_instance;//����������δ����
	SOCKET serv_sock;
	SOCKET client;

private:
	bool InitSockEnv();//���绷����ʼ��
	bool InitSocket();//�����ʼ��
	bool AcceptClient();//��������
	int DealCommand();//�����������ת
	bool Send(CPacket& packet);//��������
	void CloseClient();//�رտͻ����׽���
	std::list<CPacket*> list;//�����ͻ�����
	CPacket m_packet;//��������
};
