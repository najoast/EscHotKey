// AutoCloseWndDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"



// CAutoCloseWndDlg �Ի���
class CAutoCloseWndDlg : public CDialog
{
// ����
public:
	CAutoCloseWndDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_AUTOCLOSEWND_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	NOTIFYICONDATA  m_nid;

	bool m_bPause;

	// ���ɵ���Ϣӳ�亯��
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
