#pragma once
#include "resource.h"
#include "CCPacket.h"
#include "CStatus.h"
#include "CWatchDialog.h"
#include "RemoteClientDlg.h"
#include "ClientSocket.h"
#include <unordered_map>

#define WM_SEND_PACK (WM_USER+1)//发送包数据
#define WM_SEND_DATA (WM_USER+2)//发送数据
#define WM_SHOW_STATUS (WM_USER+3)//展示等待对话框
#define WM_SHOW_WATCH (WM_USER+4)//展示远程对话框
#define WM_SEND_MESSAGE (WM_USER+0x1000)//自定义消息处理


class CClientController
{
public:
	static CClientController* getInstance();//获取单例
	int InitController();//初始化控制层
	int Invoke(CWnd*& pMainWnd);//启动主窗口
	LRESULT SendMessage(MSG msg);//发送消息
	int GetInfoContinue();//继续获取信息
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nlen = 0);
	void DownLoadFile(const CString& filename, const CString& strpath);//下载文件
	void BeginWatch();//开始监视
private:
	static void DownLoadThread(void* arg);//下载线程
	void DownLoadEntry();//文件下载
	static void WatchThread(void* arg);//监视线程
	void WatchEntry();//远程监视---发送命令，接收数据转换image
private://单例设计
	CClientController();
	~CClientController();
	class CHelper{
	public:
		CHelper();
		~CHelper();
	};
	static CHelper helper;
	static CClientController* m_instance;
private:
	typedef struct MsgInfo {
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)
		(UINT nMsg, WPARAM wParam, LPARAM lParam);//消息处理函数指针
	LRESULT OnSendPack(UINT nMsg,WPARAM wParam,LPARAM lParam);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::unordered_map<UINT, MSGFUNC> m_mapFunc;//消息映射
	static unsigned __stdcall threadEntry(void* arg);//消息循环线程
	void threadFunc();//用于线程间消息循环
	HANDLE m_hThread;
	unsigned int m_nThreadID;

public:
	void UpDataPort(DWORD port);
	void UpDataAddr(DWORD addr);
	CPacket m_packet;//网络层接收到的数据
	bool m_isFull;//是否有图像缓存
	CImage m_image;//图像缓存
private://model层
	CWatchDialog m_watchDlg;//监视窗口
	CRemoteClientDlg m_remoteDlg;//主窗口
	CStatus  m_statusDlg;//等待窗口
	DWORD m_nport;
	DWORD m_addr;
	CClientSocket* pSocket;//网络层实例
	CString RemotePath;//远程下载路径
	CString LocalPath;//本地下载路径
	HANDLE hMutex;
};

