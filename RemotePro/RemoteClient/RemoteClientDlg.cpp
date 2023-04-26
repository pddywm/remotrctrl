#include "pch.h"
#include "Resource.h"
#include "RemoteClientDlg.h"
#include "ClientController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nport(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT1_port, m_nport);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}



BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CRemoteClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_NOTIFY(IPN_FIELDCHANGED,IDC_IPADDRESS_SERV,&CRemoteClientDlg::OnFieldchangedIpaddressServ)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	ON_COMMAND(IDC_EDIT1_port,&CRemoteClientDlg::OnChangeEdit1Port)
	ON_BN_CLICKED(IDC_BIN_STARE_WATCH, &CRemoteClientDlg::OnBnClickedBinStareWatch)
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_server_address = 0x7F000001;
	m_nport = "6467";
	UpdateData(false);
	CClientController::getInstance()->UpDataAddr(m_server_address);
	CClientController::getInstance()->UpDataPort(_ttoi(m_nport.GetBuffer()));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		


	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}





void CRemoteClientDlg::OnBnClickedButton1()
{
	int ret = CClientController::getInstance()->SendCommandPacket(1981);
	CString wtxt;
	if (ret == -1) {
		wtxt.Format("连接失败");
	}
	else {
		wtxt.Format("连接正常，接收命令%d", ret);
	}
	AfxMessageBox(wtxt);
}


void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	CClientController::getInstance()->SendCommandPacket(1);
	std::string drivers = CClientController::getInstance()->m_packet.strData;
	CString dr;
	m_Tree.DeleteAllItems();
	for (size_t i{}; i < drivers.size(); i++) {
		if (drivers[i] == ',') {
			dr += ":";
			HTREEITEM hTemp = m_Tree.InsertItem(dr);
			m_Tree.InsertItem(NULL,hTemp);
			dr.Empty();
			continue;
		}
		dr += drivers[i];
	}
	dr += ":";
	HTREEITEM hTemp=m_Tree.InsertItem(dr);
	m_Tree.InsertItem(NULL, hTemp);
}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree) {
	CString strRet, strTmp;
	do {
		strTmp = m_Tree.GetItemText(hTree);
		strRet = strTmp + "\\" + strRet;
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}


void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse,0);
	if (hTreeSelected == NULL)return;
	if (m_Tree.GetChildItem(hTreeSelected) == NULL)return;
	//删除子节点
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hTreeSelected);
		if (hSub != NULL)m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
	m_List.DeleteAllItems();
	//获取路径
	CString strPath = GetPath(hTreeSelected);
	
	CClientController::getInstance()->SendCommandPacket(2, false,(BYTE*)strPath.GetBuffer(),strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)(CClientController::getInstance()->m_packet.strData.c_str());
	while (pInfo->HasNext) {
		//fileinfo处理
		if (pInfo->IsDirectory) {
			if (CString(pInfo->szFileName) == "." ||
				CString(pInfo->szFileName) == "..") {
				int cmd = CClientController::getInstance()->GetInfoContinue();
				if (cmd < 0)break;
				pInfo = (PFILEINFO)(CClientController::getInstance()->m_packet.strData.c_str());
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected);
			m_Tree.InsertItem(NULL, hTemp);
		}
		else {
			m_List.InsertItem(0, pInfo->szFileName);
		}
		//接收包
		int cmd = CClientController::getInstance()->GetInfoContinue();
		if (cmd < 0)continue;
		pInfo = (PFILEINFO)(CClientController::getInstance()->m_packet.strData.c_str());
	};
	strPath.ReleaseBuffer();
}



void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL)return;
	if (m_Tree.GetChildItem(hTreeSelected) == NULL)return;
	//删除子节点
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hTreeSelected);
		if (hSub != NULL)m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
	m_List.DeleteAllItems();
	//获取路径
	CString strPath = GetPath(hTreeSelected);

	CClientController::getInstance()->SendCommandPacket(2, false, (BYTE*)strPath.GetBuffer(), strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)(CClientController::getInstance()->m_packet.strData.c_str());
	while (pInfo->HasNext) {
		//fileinfo处理
		if (pInfo->IsDirectory) {
			if (CString(pInfo->szFileName) == "." ||
				CString(pInfo->szFileName) == "..") {
				int cmd = CClientController::getInstance()->GetInfoContinue();
				if (cmd < 0)break;
				pInfo = (PFILEINFO)(CClientController::getInstance()->m_packet.strData.c_str());
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected);
			m_Tree.InsertItem(NULL, hTemp);
		}
		else {
			m_List.InsertItem(0, pInfo->szFileName);
		}
		//接收包
		int cmd = CClientController::getInstance()->GetInfoContinue();
		if (cmd < 0)continue;
		pInfo = (PFILEINFO)(CClientController::getInstance()->m_packet.strData.c_str());
	};
	strPath.ReleaseBuffer();
}


void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	//弹出菜单
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	CMenu menu;
	menu.LoadMenu(IDR_MENU1);
	CMenu* pPupup = menu.GetSubMenu(0);
	if (pPupup != NULL) {
		pPupup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,ptMouse.x,ptMouse.y,this);
	}
}


void CRemoteClientDlg::OnDownloadFile()
{
	int nListSelected = m_List.GetSelectionMark();
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	strPath += m_List.GetItemText(nListSelected, 0);
	CString filename = m_List.GetItemText(nListSelected, 0);
	CClientController::getInstance()->DownLoadFile(filename,strPath);
}





void CRemoteClientDlg::OnDeleteFile()
{
	int nListSelected = m_List.GetSelectionMark();
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strFile = GetPath(hSelected);
	strFile += m_List.GetItemText(nListSelected, 0);
	int ret = CClientController::getInstance()->SendCommandPacket(9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	//重新刷新
	if (hSelected == NULL)return;
	if (m_Tree.GetChildItem(hSelected) == NULL)return;
	//删除子节点
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hSelected);
		if (hSub != NULL)m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
	m_List.DeleteAllItems();
	//获取路径
	CString strPath = GetPath(hSelected);

	CClientController::getInstance()->SendCommandPacket(2, false, (BYTE*)strPath.GetBuffer(), strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)(CClientController::getInstance()->m_packet.strData.c_str());
	while (pInfo->HasNext) {
		//fileinfo处理
		if (pInfo->IsDirectory) {
			if (CString(pInfo->szFileName) == "." ||
				CString(pInfo->szFileName) == "..") {
				int cmd = CClientController::getInstance()->GetInfoContinue();
				if (cmd < 0)break;
				pInfo = (PFILEINFO)(CClientController::getInstance()->m_packet.strData.c_str());
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hSelected);
			m_Tree.InsertItem(NULL, hTemp);
		}
		else {
			m_List.InsertItem(0, pInfo->szFileName);
		}
		//接收包
		int cmd = CClientController::getInstance()->GetInfoContinue();
		if (cmd < 0)continue;
		pInfo = (PFILEINFO)(CClientController::getInstance()->m_packet.strData.c_str());
	};

	strPath.ReleaseBuffer();
}


void CRemoteClientDlg::OnRunFile()
{
	int nListSelected = m_List.GetSelectionMark();
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strFile = GetPath(hSelected);
	strFile += m_List.GetItemText(nListSelected, 0);
	CClientController::getInstance()->SendCommandPacket(3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
}


void CRemoteClientDlg::OnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	UpdateData();
	CClientController::getInstance()->UpDataAddr(m_server_address);
	*pResult = 0;
}


void CRemoteClientDlg::OnChangeEdit1Port()
{
	UpdateData();
	DWORD port = _ttoi(m_nport.GetBuffer());
	CClientController::getInstance()->UpDataPort(port);
}

void CRemoteClientDlg::OnBnClickedBinStareWatch() {
	CClientController::getInstance()->BeginWatch();
}
