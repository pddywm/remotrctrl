#include "pch.h"
#include "resource.h"
#include "CCommand.h"
#include <direct.h>
#include <atlimage.h>
#include <io.h>




CCommand::CCommand():threadid(0)
{
	struct {
		int nCmd;
		CMDFUNC func;
	}data[]{
		{1,&CCommand::MakeDriverInfo},
		{2,&CCommand::MakeDirectoryInfo},
		{3,&CCommand::RunFile},
		{4,&CCommand::DownLoadFile},
		{5,&CCommand::MouseEvent},
		{6,&CCommand::SendScreen},
		{7,&CCommand::LockMachine},
		{8,&CCommand::UnLockMachine},
		{9,&CCommand::DeleteLocalFile},
		{1981,&CCommand::TestConnect},
		{-1,NULL},
	};
	for (int i{}; data[i].nCmd != -1; i++) {
		m_mapFunction.insert(std::pair<int, CMDFUNC>(data[i].nCmd, data[i].func));
	}

}

CCommand::~CCommand()
{
}

int CCommand::ExcuteCommand(int nCmd)
{
	std::unordered_map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);
	if (it == m_mapFunction.end())return -1;
	return (this->*it->second)();
}

int CCommand::MakeDriverInfo()
{
	std::string result;
	for (int i{ 1 }; i <= 26; i++) {
		if (_chdrive(i) == 0) {
			if (result.size() > 0)result += ',';
			result += 'A' + i - 1;
		}
	}
	//Dump((BYTE*)pack.Data(), pack.nLength + 6);//����
	CPacket* pCPK = new CPacket(1, (const BYTE*)result.c_str(), result.size());
	plist->push_back(pCPK);
	return 0;
}

int CCommand::MakeDirectoryInfo()
{
	CPacket* pCPK;
	std::string strPath = packet.strData;
	//Dump((BYTE*)strPath.c_str(), strPath.size());
	if (_chdir(strPath.c_str()) != 0) {
		FILEINFO finfo;
		finfo.IsInvalid = true;
		finfo.IsDirectory = TRUE;
		finfo.HasNext = FALSE;
		memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
		pCPK = new CPacket(2, (BYTE*)&finfo, sizeof(finfo));
		plist->push_back(pCPK);
		OutputDebugString(_T("û��Ȩ�ޣ�����Ŀ¼"));
		return -2;
	}
	_finddata_t fdata;
	intptr_t hfind = 0;
	if ((hfind = _findfirst("*", &fdata)) == -1) {
		OutputDebugString(_T("û���ҵ��κ��ļ�"));
		return -3;
	}
	do {
		FILEINFO finfo;
		finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
		memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		pCPK = new CPacket(2, (BYTE*)&finfo, sizeof(finfo));
		plist->push_back(pCPK);
	} while (_findnext(hfind, &fdata) == 0);


	//����

	FILEINFO finfo;
	finfo.HasNext = FALSE;
	pCPK = new CPacket(2, (BYTE*)&finfo, sizeof(finfo));
	plist->push_back(pCPK);
	_findclose(hfind);
	return 0;
}

int CCommand::RunFile()
{
	std::string strPath = packet.strData;
	ShellExecute(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	CPacket* pCPK = new CPacket(3, NULL, 0);
	plist->push_back(pCPK);
	return 0;
}

int CCommand::DownLoadFile()
{
	std::string strPath = packet.strData;
	CPacket* pCPK;
	long long data{};
	FILE* pFile = NULL;
	errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
	if (err) {
		pCPK = new CPacket(4, (BYTE*)&data, 8);
		plist->push_back(pCPK);
		return -1;
	}
	if (pFile != NULL) {
		fseek(pFile, 0, SEEK_END);
		data = _ftelli64(pFile);
		pCPK = new CPacket(4, (BYTE*)&data, 8);
		plist->push_back(pCPK);
		fseek(pFile, 0, SEEK_SET);

		char buffer[1024] = "";
		size_t rlen{};
		do {
			rlen = fread(buffer, 1, 1024, pFile);
			pCPK = new CPacket(4, (BYTE*)buffer, rlen);
			plist->push_back(pCPK);
		} while (rlen >= 1024);
	}

	pCPK = new CPacket(4, NULL, 0);
	plist->push_back(pCPK);
	fclose(pFile);
	return 0;
}

int CCommand::MouseEvent()
{
	MOUSEEV mouse;
	memcpy(&mouse, packet.strData.c_str(), sizeof(MOUSEEV));
	DWORD nFlags = 0;
	switch (mouse.nButton) {
		case 1:nFlags = 1; break;//���
		case 2:nFlags = 2; break;//�Ҽ�
		case 4:nFlags = 4; break;//�м�
		case 8:nFlags = 8; break;//û�а���
		default:break;
	}
	if (nFlags != 8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
	switch (mouse.nAction) {
		case 0:nFlags |= 0x10; break;//����
		case 1:nFlags |= 0x20; break;//˫��
		case 2:nFlags |= 0x40; break;//����
		case 3:nFlags |= 0x80; break;//�ſ�
		default:break;
	}
	switch (nFlags) {
		case 0x21:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;//�������
		case 0x41:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			break;//�������
		case 0x81:
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;//����ſ�

		case 0x22:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo()); break;//�Ҽ�����
		case 0x42:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;//�Ҽ�����
		case 0x82:
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;//�Ҽ��ſ�

		case 0x24:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo()); //�м�˫��
		case 0x14:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;//�м�����
		case 0x44:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;//�м�����
		case 0x84:
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;//�м��ſ�
		case 0x08:
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;//����ƶ�
		default:break;
	}
	CPacket* pCPK = new CPacket(5, NULL, 0);
	plist->push_back(pCPK);
	return 0;
}

int CCommand::SendScreen()
{
	CImage screen;
	HDC hScreen = GetDC(NULL);
	int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
	int nHeight = GetDeviceCaps(hScreen, VERTRES);
	screen.Create(nWidth, nHeight, nBitPerPixel);
	BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
	ReleaseDC(NULL, hScreen);
	//screen.Save(_T("test.png"), Gdiplus::ImageFormatPNG);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
	IStream* pStream = NULL;
	HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
	if (ret == S_OK) {
		screen.Save(pStream, Gdiplus::ImageFormatPNG);
		LARGE_INTEGER bg{};
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);
		PBYTE PData = (PBYTE)GlobalLock(hMem);
		SIZE_T nSize = GlobalSize(hMem);
		CPacket* pCPK = new CPacket(6, PData, nSize);
		plist->push_back(pCPK);
		//AfxMessageBox("���ͳɹ�");
		GlobalUnlock(hMem);
	}
	pStream->Release();
	GlobalFree(hMem);
	screen.ReleaseDC();
	return 0;
}

int CCommand::LockMach()
{
	dlg.Create(IDD_DIALOG_INFO, NULL);
	dlg.ShowWindow(SW_SHOW);
	//�ڱδ���
	CRect rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN) + 20;
	rect.bottom = GetSystemMetrics(SM_CXFULLSCREEN) + 20;
	dlg.MoveWindow(rect);
	//�����ö�
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	//�������
	ShowCursor(false);
	rect.left = 0;
	rect.top = 0;
	rect.right = 1;
	rect.bottom = 1;
	ClipCursor(rect);//���ƻ��Χ
	//����ϵͳ������
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
	//TODO:ʹ���������ʧЧ
	//::ShowWindow(::FindWindow(_T("Button"), NULL), SW_HIDE);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.wParam == 0x41) {
			break;
		}
	}
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
	dlg.DestroyWindow();
	ShowCursor(true);

	_endthreadex(0);
	return 0;
}

unsigned _stdcall CCommand::threadLockDlg(void* arg)
{
	CCommand* thiz = (CCommand*)arg;
	return thiz->LockMach();
}

int CCommand::LockMachine()
{
	if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
		_beginthreadex(NULL, 0, threadLockDlg, (void*)this, 0, &threadid);
	}
	CPacket* pCPK = new CPacket(7, NULL, 0);
	plist->push_back(pCPK);
	return 0;
}

int CCommand::UnLockMachine()
{
	PostThreadMessage(threadid, WM_KEYDOWN, 0x41, 0);
	CPacket* pCPK = new CPacket(8, NULL, 0);
	plist->push_back(pCPK);
	return 0;
}

int CCommand::DeleteLocalFile()
{
	std::string strPath = packet.strData;
	CPacket* pCPK = new CPacket(9, NULL, 0);
	DeleteFileA(strPath.c_str());
	plist->push_back(pCPK);
	return 0;
}

int CCommand::TestConnect()
{
	CPacket* pCPK = new CPacket(1981, NULL, 0);
	plist->push_back(pCPK);
	return 0;
}

int CCommand::RunCommand(CPacket& packet,std::list<CPacket*>* list, void* arg) {
	CCommand* thiz = (CCommand*)arg;
	thiz->plist = list;
	thiz->packet = packet;
	return thiz->ExcuteCommand(packet.sCmd);
}
