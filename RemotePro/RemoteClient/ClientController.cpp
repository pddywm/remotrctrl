#include "pch.h"
#include "ClientController.h"


CClientController* CClientController::m_instance = NULL;
CClientController::CHelper CClientController::helper;
std::unordered_map<UINT, CClientController::MSGFUNC> CClientController::m_mapFunc;

CClientController* CClientController::getInstance()
{
	if (!m_instance) {
		m_instance = new CClientController;
		struct {
			UINT nMsg;
			MSGFUNC func;
		}data[]{
			{WM_SEND_PACK,&CClientController::OnSendPack},
			{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatcher},
			{-1,NULL}
		};
		for (int i{}; data[i].nMsg != -1; i++)
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(data[i].nMsg, data[i].func));
	}
	return m_instance;
}

void CClientController::threadFunc() {
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE) {
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::unordered_map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(pmsg->msg.message);
			if (it != m_mapFunc.end()) {
				pmsg->result = (this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
			}
			else {
				pmsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else {
			std::unordered_map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end())
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
		}
	}
}

unsigned  CClientController::threadEntry(void* arg) {
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}

CClientController::CClientController() :
	m_statusDlg(&m_remoteDlg), m_watchDlg(&m_remoteDlg)
{
	m_hThread = INVALID_HANDLE_VALUE;
	m_nThreadID = -1;
	m_isFull = false;
	pSocket = CClientSocket::GetInstance();
	hMutex = CreateMutex(NULL, FALSE, "socket");
}


CClientController::~CClientController()
{
	CloseHandle(hMutex);
}

CClientController::CHelper::CHelper() {

}

CClientController::CHelper::~CHelper() {
	if (m_instance) {
		CClientController* tmp = m_instance;
		m_instance = nullptr;
		delete tmp;
	}
}


int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}


int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, threadEntry, this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DIALOG1, &m_remoteDlg);
	return 0;
}


LRESULT CClientController::SendMessage(MSG msg) {
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)return -2;
	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)hEvent);
	WaitForSingleObject(hEvent, -1);
	return info.result;
}


LRESULT CClientController::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam) { return 0; }
LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam) { return 0; }
LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam) {
	return m_statusDlg.ShowWindow(SW_SHOW);
}
LRESULT CClientController::OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam) {
	return m_watchDlg.DoModal();
}


void  CClientController::UpDataPort(DWORD port) {
	m_nport = port;
}

void CClientController::UpDataAddr(DWORD addr) {
	m_addr = addr;
}


int CClientController::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nlen) {
	WaitForSingleObject(hMutex, INFINITE);
	pSocket->CloseSocket();
	pSocket->InitSocket(m_addr, m_nport);
	CPacket pack(nCmd, pData, nlen);
	pSocket->Send(pack);
	pSocket->DealCommand();
	if (bAutoClose)
		pSocket->CloseSocket();
	ReleaseMutex(hMutex);
	return m_packet.sCmd;
}

int CClientController::GetInfoContinue() {
	return pSocket->DealCommand();
}

void CClientController::DownLoadFile(const CString& filename, const CString& path) {
	CFileDialog dlg(FALSE, NULL , filename,
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, NULL, &m_remoteDlg);
	if (dlg.DoModal() == IDOK) {
		LocalPath = dlg.GetPathName();
		RemotePath = path;
		_beginthread(DownLoadThread, 0, this);
		m_remoteDlg.BeginWaitCursor();
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_remoteDlg);
		m_statusDlg.SetActiveWindow();
	}
}
void CClientController::DownLoadEntry() {
	FILE* pFile = fopen(LocalPath, "wb+");
	if (pFile == NULL) {
		AfxMessageBox("本地没有权限保存该文件或者文件无法创建");
		return;
	}
	int ret = SendCommandPacket(4,FALSE,(BYTE*)RemotePath.GetBuffer(),RemotePath.GetLength());
	if (ret != 4)return;
	long long nLength = *(long long*)m_packet.strData.c_str();
	if (nLength == 0) {
		AfxMessageBox("文件长度为0或无法读取文件");
		return;
	}
	long long nCount = 0;
	//文件写入
	while (nCount <= nLength) {
		ret = GetInfoContinue();
		if (ret < 0) {
			CString wtxt;
			wtxt.Format("文件传输失败,错误码%d", WSAGetLastError());
			AfxMessageBox(wtxt);
			break;
		}
		if ((m_packet.sCmd == 4) && (m_packet.strData.size() == 0))break;
		fwrite(m_packet.strData.c_str(),
			1, m_packet.strData.size(), pFile);
		nCount += m_packet.strData.size();
	}
	fclose(pFile);
	m_remoteDlg.EndWaitCursor();
	m_statusDlg.ShowWindow(HIDE_WINDOW);
}


void CClientController::DownLoadThread(void* arg) {
	CClientController* thiz = (CClientController*)arg;
	thiz->DownLoadEntry();
	_endthread();
}


void CClientController::BeginWatch() {
	_beginthread(WatchThread, 0, this);
	m_watchDlg.DoModal();
}


void CClientController::WatchThread(void* arg) {
	CClientController* thiz = (CClientController*)arg;
	thiz->WatchEntry();
}

void CClientController::WatchEntry() {
	//Sleep(50);
	while(1) {
		if (m_isFull == false) {
			int ret = SendCommandPacket(6);
			if (ret > 0) {
				if (m_packet.sCmd == 6) {
					if (m_isFull == false) {
						BYTE* pData = (BYTE*)m_packet.strData.c_str();
						//存入image
						HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
						if (hMem == NULL) {
							Sleep(1);
							continue;
						}
						IStream* pStream = NULL;
						HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
						if (hRet == S_OK) {
							ULONG length = 0;
							pStream->Write(pData, m_packet.strData.size(), &length);
							LARGE_INTEGER bg = {};
							pStream->Seek(bg, STREAM_SEEK_SET, NULL);
							m_image.Load(pStream);
							m_isFull = true;
						}
					}
				}
			}
			else {
				Sleep(1);
			}
		}
	}
	_endthread();
}

