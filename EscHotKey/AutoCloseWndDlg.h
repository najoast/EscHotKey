// AutoCloseWndDlg.h : 头文件
//

#pragma once
#include "afxwin.h"



// CAutoCloseWndDlg 对话框
class CAutoCloseWndDlg : public CDialog
{
// 构造
public:
	CAutoCloseWndDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_AUTOCLOSEWND_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	NOTIFYICONDATA  m_nid;

	bool m_bPause;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	virtual BOOL DestroyWindow();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
	void AddStartupRun();
	void DeleteStartupRun();
public:
	afx_msg void OnBnClickedAutorun();
	afx_msg void OnBnClickedRunMin();
	CButton m_chkAutoRun;
	CButton m_chkRunMin;
	afx_msg void OnStnClickedHello();
	afx_msg void OnBnClickedBtnGithub();
	afx_msg void OnBnClickedBtnOpenConfig();
};
