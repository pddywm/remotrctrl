#pragma once
#include "afxdialogex.h"


// CStatus 对话框

class CStatus : public CDialogEx
{
	DECLARE_DYNAMIC(CStatus)

public:
	CStatus(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CStatus();
	void CloseCwnd();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
