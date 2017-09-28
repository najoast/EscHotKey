// AutoCloseWndDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "AutoCloseWnd.h"
#include "AutoCloseWndDlg.h"
#include "IniFile.h"

// #include "psapi.h"
// #pragma comment(lib, "Psapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CIniFile g_iniConfig;
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAutoCloseWndDlg �Ի���

#define WM_SYSTEMTRAY WM_USER+1


CAutoCloseWndDlg::CAutoCloseWndDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAutoCloseWndDlg::IDD, pParent)
	, m_bPause(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutoCloseWndDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAutoCloseWndDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CAutoCloseWndDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

enum 
{
	ESC_MIN_WINDOW = 1,
	ESC_CLOSE_WINDOW,
	ESC_HIDE_WINDOW,
	ESC_EXIT_PROCESS,
};

HHOOK       g_hHook = NULL;         //���Ӿ��
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam,LPARAM lParam)  
{   
	//�ж��Ƿ�����Ч����  
	if (nCode >= HC_ACTION && wParam==WM_KEYDOWN)   
	{   
		//BOOL bEsc = GetAsyncKeyState(VK_ESCAPE)>>((sizeof(SHORT) *8)-1);     
		KBDLLHOOKSTRUCT* pStruct = (KBDLLHOOKSTRUCT*)lParam;    
 
		if (pStruct->vkCode == VK_ESCAPE)
		{
			//AfxMessageBox("ESC key down!!");
			TCHAR szName[MAX_PATH] = {0};
			HWND hwnd = ::GetForegroundWindow();
			if(hwnd)
			{
				DWORD dwId = 0;
				GetWindowThreadProcessId(hwnd, &dwId);
				HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE,
					0, dwId);
				if(handle)
				{
					TCHAR szFileName[MAX_PATH] = {0};
					DWORD dwLen = MAX_PATH;
					//GetProcessImageFileName(handle, szFileName, MAX_PATH);
					QueryFullProcessImageName(handle, 0, szFileName, &dwLen);
					int err = GetLastError();
					//AfxMessageBox(szFileName);

					std::string strFullName(szFileName);
					int nPos = strFullName.find_last_of('\\');
					if (nPos != std::string::npos)
					{
						std::string strFileName = strFullName.substr(nPos+1);
						int nValue = g_iniConfig.GetValueI("esc", strFileName.c_str());
						switch (nValue)
						{
						case ESC_MIN_WINDOW:
							::ShowWindow(hwnd, SW_MINIMIZE);
							break;
						case ESC_CLOSE_WINDOW:
							::CloseWindow(hwnd);
							break;
						case ESC_HIDE_WINDOW:
							::ShowWindow(hwnd, SW_HIDE);
							break;
						case ESC_EXIT_PROCESS:
							::TerminateProcess(handle, 0);
							break;
						}
					}
				}
			}

		}
		//return TRUE;    
	}   
	//����ϵͳ�е���һ������   
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);   
}

// CAutoCloseWndDlg ��Ϣ�������

BOOL CAutoCloseWndDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDR_MAINFRAME; //ͼ���ID
	m_nid.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	m_nid.uCallbackMessage = WM_SYSTEMTRAY; //ͼ���Ӧ����Ϣ��ID
	m_nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//wcscpy_s(m_nid.szTip,10, L"�Զ��رն���");
	strcpy_s(m_nid.szTip, sizeof(m_nid.szTip), "AutoCloseAssert");
	::Shell_NotifyIcon(NIM_ADD,&m_nid);

	SetDlgItemText(IDC_BUTTON1, m_bPause ? "Continue" : "Pause");
	//ShowWindow(SW_SHOWMINIMIZED);

	// ��Ӽ��̹���
	if (g_hHook == NULL)
	{
		g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,  AfxGetInstanceHandle(), NULL);
	}
	
	if (!g_iniConfig.ReadFile("config.ini"))
	{
		AfxMessageBox("Load config.ini failed!");
		exit(0);
	}

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CAutoCloseWndDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CAutoCloseWndDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CAutoCloseWndDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


LRESULT CAutoCloseWndDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: �ڴ����ר�ô����/����û���
	switch (message)
	{
	case WM_SYSCOMMAND:
		{
			if(wParam == SC_MINIMIZE)
				return ShowWindow(SW_HIDE);
		}
		break;
	case WM_SYSTEMTRAY:
		{
			if(lParam == WM_LBUTTONDOWN)
				return ShowWindow(SW_SHOW);
		}
		break;
	default:
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);	
}

BOOL CAutoCloseWndDlg::DestroyWindow()
{
	// TODO: �ڴ����ר�ô����/����û���
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = 0;
	::Shell_NotifyIcon(NIM_DELETE, &m_nid);

	return CDialog::DestroyWindow();
}


void CAutoCloseWndDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	
	CDialog::OnClose();
}



void CAutoCloseWndDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialog::OnTimer(nIDEvent);
}

void CAutoCloseWndDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_bPause = !m_bPause;
	SetDlgItemText(IDC_BUTTON1, m_bPause ? "Continue" : "Pause");
}
