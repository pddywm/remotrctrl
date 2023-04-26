#pragma once
#include "resource.h"
#include "CCPacket.h"
#include "CStatus.h"
#include "CWatchDialog.h"
#include "RemoteClientDlg.h"
#include "ClientSocket.h"
#include <unordered_map>

#define WM_SEND_PACK (WM_USER+1)//���Ͱ�����
#define WM_SEND_DATA (WM_USER+2)//��������
#define WM_SHOW_STATUS (WM_USER+3)//չʾ�ȴ��Ի���
#define WM_SHOW_WATCH (WM_USER+4)//չʾԶ�̶Ի���
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ�����Ϣ����


class CClientController
{
public:
	static CClientController* getInstance();//��ȡ����
	int InitController();//��ʼ�����Ʋ�
	int Invoke(CWnd*& pMainWnd);//����������
	LRESULT SendMessage(MSG msg);//������Ϣ
	int GetInfoContinue();//������ȡ��Ϣ
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nlen = 0);
	void DownLoadFile(const CString& filename, const CString& strpath);//�����ļ�
	void BeginWatch();//��ʼ����
private:
	static void DownLoadThread(void* arg);//�����߳�
	void DownLoadEntry();//�ļ�����
	static void WatchThread(void* arg);//�����߳�
	void WatchEntry();//Զ�̼���---���������������ת��image
private://�������
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
		(UINT nMsg, WPARAM wParam, LPARAM lParam);//��Ϣ������ָ��
	LRESULT OnSendPack(UINT nMsg,WPARAM wParam,LPARAM lParam);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::unordered_map<UINT, MSGFUNC> m_mapFunc;//��Ϣӳ��
	static unsigned __stdcall threadEntry(void* arg);//��Ϣѭ���߳�
	void threadFunc();//�����̼߳���Ϣѭ��
	HANDLE m_hThread;
	unsigned int m_nThreadID;

public:
	void UpDataPort(DWORD port);
	void UpDataAddr(DWORD addr);
	CPacket m_packet;//�������յ�������
	bool m_isFull;//�Ƿ���ͼ�񻺴�
	CImage m_image;//ͼ�񻺴�
private://model��
	CWatchDialog m_watchDlg;//���Ӵ���
	CRemoteClientDlg m_remoteDlg;//������
	CStatus  m_statusDlg;//�ȴ�����
	DWORD m_nport;
	DWORD m_addr;
	CClientSocket* pSocket;//�����ʵ��
	CString RemotePath;//Զ������·��
	CString LocalPath;//��������·��
	HANDLE hMutex;
};

