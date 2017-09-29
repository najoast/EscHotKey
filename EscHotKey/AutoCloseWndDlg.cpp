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

const char* APP_NAME = "EscHotkey";

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
	DDX_Control(pDX, IDC_AUTORUN, m_chkAutoRun);
	DDX_Control(pDX, IDC_RUN_MIN, m_chkRunMin);
}

BEGIN_MESSAGE_MAP(CAutoCloseWndDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_AUTORUN, &CAutoCloseWndDlg::OnBnClickedAutorun)
	ON_BN_CLICKED(IDC_RUN_MIN, &CAutoCloseWndDlg::OnBnClickedRunMin)
	ON_STN_CLICKED(IDC_HELLO, &CAutoCloseWndDlg::OnStnClickedHello)
	ON_BN_CLICKED(IDC_BTN_GITHUB, &CAutoCloseWndDlg::OnBnClickedBtnGithub)
	ON_BN_CLICKED(IDC_BTN_OPEN_CONFIG, &CAutoCloseWndDlg::OnBnClickedBtnOpenConfig)
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
						int nValue = g_iniConfig.GetValueI("ESC", strFileName.c_str());
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

	//得到配置文件的完整路径
	// 这里有个坑：如果是开机启动，工作目录是C:\Windows\SysWow64,所以会找不到配置文件!
	// 最初想使用注册表里存的值，后来发现直接取命令行第一个参数就行了，更方便。

	LPSTR pszCmdLine = ::GetCommandLine();
// 	AfxMessageBox(pszCmdLine);
	if (strlen(pszCmdLine) < 2)
		exit(0);
	pszCmdLine++;//去掉第一个引号
	char* p = strstr(pszCmdLine, APP_NAME);
	if (p != NULL)
		p[0] = '\0';

	CString strConfig;
	strConfig.Format("%sconfig.ini", pszCmdLine);

// 	if (strstr(pszCmdLine, "regrun") != NULL)
// 	{
// 		//找到系统的启动项
// 		HKEY hKey;
// 		LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
// 		//打开启动项Key 
// 		long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey);
// 		if(lRet== ERROR_SUCCESS)
// 		{
// 			if (::RegQueryValueEx( hKey, _T(APP_NAME), 0, NULL, (BYTE*)szPath, &dwLen ) == ERROR_SUCCESS)
// 			{
// 				AfxMessageBox(szPath);
// 				char* p = strstr(szPath, APP_NAME);
// 				if (p != NULL)
// 					p[0] = '\0';
// 
// 				strcat(szPath, "\\config.ini");
// 			}
// 
// 			RegCloseKey(hKey);
// 		}
// 	}

	if (!g_iniConfig.ReadFile(strConfig))
	{
		AfxMessageBox("Load config.ini failed!");
		exit(0);
	}

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
	strcpy_s(m_nid.szTip, sizeof(m_nid.szTip), APP_NAME);
	::Shell_NotifyIcon(NIM_ADD,&m_nid);

	//SetDlgItemText(IDC_BUTTON1, m_bPause ? "Continue" : "Pause");
	//ShowWindow(SW_SHOWMINIMIZED);

	// 添加键盘钩子
	if (g_hHook == NULL)
	{
		g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,  AfxGetInstanceHandle(), NULL);
	}

	// 初始化控件状态
	m_chkAutoRun.SetCheck(g_iniConfig.GetValueI("UserInfo", "AutoRun"));
	m_chkRunMin.SetCheck(g_iniConfig.GetValueI("UserInfo", "RunMin"));

	if (m_chkRunMin.GetCheck() == BST_CHECKED)
	{
		//this->ShowWindow(SW_HIDE);
		::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		
		//托盘发送提醒消息

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

//开机启动
void CAutoCloseWndDlg::AddStartupRun()
{
	HKEY   hKey; 
	char pFileName[MAX_PATH] = {0}; 
	//得到程序自身的全路径 
	DWORD dwRet = GetModuleFileName(NULL, pFileName, MAX_PATH);

	//CString strWriteToReg;
	//strWriteToReg.Format("\"%s\"", pFileName);//加上双引号，解决路径中有空格的问题。

	//找到系统的启动项 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	//打开启动项Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//添加注册
		lRet = RegSetValueEx(hKey, APP_NAME, 0,REG_SZ,(const BYTE*)(LPCSTR)pFileName, MAX_PATH);
		RegCloseKey(hKey); 
	}
}

//取消开机启动
void CAutoCloseWndDlg::DeleteStartupRun()
{
	HKEY   hKey;
	char pFileName[MAX_PATH] = {0}; 
	//得到程序自身的全路径 
	DWORD dwRet = GetModuleFileNameW(NULL, (LPWCH)pFileName, MAX_PATH); 
	//找到系统的启动项 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	//打开启动项Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//删除注册
		RegDeleteValue(hKey,_T(APP_NAME));
		RegCloseKey(hKey);
	}
}
void CAutoCloseWndDlg::OnBnClickedAutorun()
{
	int nChecked = m_chkAutoRun.GetCheck();
	if (BST_CHECKED == nChecked)
		this->AddStartupRun();
	else
		this->DeleteStartupRun();

	g_iniConfig.SetValueI("UserInfo", "AutoRun", nChecked);
	g_iniConfig.WriteFile();//todo: 保留注释
}

void CAutoCloseWndDlg::OnBnClickedRunMin()
{
	g_iniConfig.SetValueI("UserInfo", "RunMin", m_chkRunMin.GetCheck());
	g_iniConfig.WriteFile();//todo: 保留注释
}

void CAutoCloseWndDlg::OnStnClickedHello()
{
	
}

void CAutoCloseWndDlg::OnBnClickedBtnGithub()
{
	::ShellExecute(NULL, "Open", "https://github.com/najoast/EscHotkey", NULL, NULL, SW_SHOWNORMAL);
}

void CAutoCloseWndDlg::OnBnClickedBtnOpenConfig()
{
	char szPath[MAX_PATH] = "";
	if (::GetCurrentDirectory(MAX_PATH, szPath) > 0)
		::ShellExecute(NULL, "Open", "C:\\windows\\explorer.exe", szPath, NULL, SW_SHOWNORMAL);
}
