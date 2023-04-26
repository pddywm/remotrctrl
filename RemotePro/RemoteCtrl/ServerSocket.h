#pragma once
#include "pch.h"
#include "framework.h"
#include <list>
#include "CCPacket.h"

typedef int (*SOCKCALL)(CPacket& packet,std::list<CPacket*>* plist, void* args);

class CServerSocket
{
public:
	static CServerSocket* getInstance();//获取单例指针
	int StartRun(SOCKCALL mycallback,void* arg);//开始运行
private://单例设计
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
	static CServerSocket* m_instance;//仅仅是申明未定义
	SOCKET serv_sock;
	SOCKET client;

private:
	bool InitSockEnv();//网络环境初始化
	bool InitSocket();//网络初始化
	bool AcceptClient();//接送数据
	int DealCommand();//处理接收数据转
	bool Send(CPacket& packet);//发送数据
	void CloseClient();//关闭客户端套接字
	std::list<CPacket*> list;//待发送缓冲区
	CPacket m_packet;//接收数据
};
