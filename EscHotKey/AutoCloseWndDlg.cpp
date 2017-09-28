// AutoCloseWndDlg.cpp : 实现文件
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
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CAutoCloseWndDlg 对话框

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

HHOOK       g_hHook = NULL;         //钩子句柄
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam,LPARAM lParam)  
{   
	//判断是否是有效按键  
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
	//传给系统中的下一个钩子   
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);   
}

// CAutoCloseWndDlg 消息处理程序

BOOL CAutoCloseWndDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDR_MAINFRAME; //图标的ID
	m_nid.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	m_nid.uCallbackMessage = WM_SYSTEMTRAY; //图标对应的消息的ID
	m_nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//wcscpy_s(m_nid.szTip,10, L"自动关闭断言");
	strcpy_s(m_nid.szTip, sizeof(m_nid.szTip), "AutoCloseAssert");
	::Shell_NotifyIcon(NIM_ADD,&m_nid);

	SetDlgItemText(IDC_BUTTON1, m_bPause ? "Continue" : "Pause");
	//ShowWindow(SW_SHOWMINIMIZED);

	// 添加键盘钩子
	if (g_hHook == NULL)
	{
		g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,  AfxGetInstanceHandle(), NULL);
	}
	
	if (!g_iniConfig.ReadFile("config.ini"))
	{
		AfxMessageBox("Load config.ini failed!");
		exit(0);
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAutoCloseWndDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CAutoCloseWndDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


LRESULT CAutoCloseWndDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
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
	// TODO: 在此添加专用代码和/或调用基类
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = 0;
	::Shell_NotifyIcon(NIM_DELETE, &m_nid);

	return CDialog::DestroyWindow();
}


void CAutoCloseWndDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	CDialog::OnClose();
}



void CAutoCloseWndDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnTimer(nIDEvent);
}

void CAutoCloseWndDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bPause = !m_bPause;
	SetDlgItemText(IDC_BUTTON1, m_bPause ? "Continue" : "Pause");
}
