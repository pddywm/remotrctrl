#pragma once

#include "afxdialogex.h"
#include <string>


typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//是否有效
	char szFileName[256];//文件名
	BOOL HasNext;//是否还有后续0没有1有
	BOOL IsDirectory;//是否未目录
}FILEINFO, * PFILEINFO;


class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButton1();//连接测试
	afx_msg void OnBnClickedBtnFileinfo();//获取磁盘信息
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);//双击获取指定目录信息
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);//单击获取指定目录信息
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);//右键弹出菜单
	afx_msg void OnDownloadFile();//下载文件
	afx_msg void OnDeleteFile();//删除文件
	afx_msg void OnRunFile();//运行文件
	afx_msg void OnBnClickedBinStareWatch();//监视
	afx_msg void OnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);//用户改变ip
	afx_msg void OnChangeEdit1Port();//用户改变port
	DECLARE_MESSAGE_MAP()
private:
	DWORD m_server_address;//ip控件
	CString m_nport;//port控件
	CTreeCtrl m_Tree;//树控件
	CListCtrl m_List;//列表控件
	static void threadEntryFoeWatchData(void* arg);
	void threadWatchData();
	CString GetPath(HTREEITEM hTree);
};
