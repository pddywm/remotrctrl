#include "pch.h"
#include "afxdialogex.h"
#include "CWatchDialog.h"
#include "ClientController.h"

// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG2, pParent)
{

}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON1, &CWatchDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CWatchDialog::OnBnClickedButton2)
END_MESSAGE_MAP()


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point)
{
	CRect clientRect;
	m_picture.GetWindowRect(&clientRect);
	point.y -= 36;
	int width0 = clientRect.Width();
	int height0 = clientRect.Height()-36;
	point.x = point.x * 1920 / width0;
	point.y = point.y * 1080 / height0;
	return point;
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetTimer(0, 50, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control

}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0) {
		CClientController* p = CClientController::getInstance();
		if (p->m_isFull) {
			CRect rect;
			m_picture.GetWindowRect(rect);
			p->m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0,
				rect.Width(),rect.Height()-18, SRCCOPY);
			p->m_image.Destroy();
			p->m_isFull = false;
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	UserPoint2RemoteScreenPoint(point);
	MouseEvent event;
	event.ptXY = point;
	event.nButton = 1;
	event.nAction = 1;
	CClientController::getInstance()->SendCommandPacket(5,true,(BYTE*)&event,sizeof(MouseEvent));
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//AfxMessageBox("左键弹起");
	UserPoint2RemoteScreenPoint(point);
	MouseEvent event;
	event.ptXY = point;
	event.nButton = 1;//左键
	event.nAction = 3;//弹起
	CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(MouseEvent));
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UserPoint2RemoteScreenPoint(point);
	MouseEvent event;
	event.ptXY = point;
	event.nButton = 2;//右键
	event.nAction = 1;//双击
	CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(MouseEvent));
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UserPoint2RemoteScreenPoint(point);
	MouseEvent event;
	event.ptXY = point;
	event.nButton = 2;
	event.nAction = 3;
	CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(MouseEvent));
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UserPoint2RemoteScreenPoint(point);
	MouseEvent event;
	event.ptXY = point;
	event.nButton = 4;
	event.nAction = 1;
	CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(MouseEvent));
	CDialog::OnMButtonDblClk(nFlags, point);
}


void CWatchDialog::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UserPoint2RemoteScreenPoint(point);
	MouseEvent event;
	event.ptXY = point;
	event.nButton = 4;
	event.nAction = 3;
	CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(MouseEvent));
	CDialog::OnMButtonUp(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UserPoint2RemoteScreenPoint(point);
	MouseEvent event;
	event.ptXY = point;
	event.nButton = 1;//左键
	event.nAction = 2;//按下
	CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(MouseEvent));
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UserPoint2RemoteScreenPoint(point);
	MouseEvent event;
	event.ptXY = point;
	event.nButton = 4;
	event.nAction = 2;
	CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(MouseEvent));
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UserPoint2RemoteScreenPoint(point);
	MouseEvent event;
	event.ptXY = point;
	event.nButton = 2;
	event.nAction = 2;
	CClientController::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(MouseEvent));
	CDialog::OnRButtonDown(nFlags, point);
}


//锁机
void CWatchDialog::OnBnClickedButton1()
{
	CClientController::getInstance()->SendCommandPacket(7);
}

//解锁
void CWatchDialog::OnBnClickedButton2()
{
	CClientController::getInstance()->SendCommandPacket(8);
}
