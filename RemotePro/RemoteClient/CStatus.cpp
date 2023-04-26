// CStatus.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "CStatus.h"


// CStatus 对话框

IMPLEMENT_DYNAMIC(CStatus, CDialogEx)

CStatus::CStatus(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CStatus::~CStatus()
{
}

void CStatus::CloseCwnd()
{
	OnOK();
}

void CStatus::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CStatus, CDialogEx)
END_MESSAGE_MAP()


// CStatus 消息处理程序
