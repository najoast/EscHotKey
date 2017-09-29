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

const char* APP_NAME = "EscHotkey";

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
	//����ϵͳ�е���һ������   
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);   
}

// CAutoCloseWndDlg ��Ϣ�������

BOOL CAutoCloseWndDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//�õ������ļ�������·��
	// �����и��ӣ�����ǿ�������������Ŀ¼��C:\Windows\SysWow64,���Ի��Ҳ��������ļ�!
	// �����ʹ��ע�������ֵ����������ֱ��ȡ�����е�һ�����������ˣ������㡣

	LPSTR pszCmdLine = ::GetCommandLine();
// 	AfxMessageBox(pszCmdLine);
	if (strlen(pszCmdLine) < 2)
		exit(0);
	pszCmdLine++;//ȥ����һ������
	char* p = strstr(pszCmdLine, APP_NAME);
	if (p != NULL)
		p[0] = '\0';

	CString strConfig;
	strConfig.Format("%sconfig.ini", pszCmdLine);

// 	if (strstr(pszCmdLine, "regrun") != NULL)
// 	{
// 		//�ҵ�ϵͳ��������
// 		HKEY hKey;
// 		LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
// 		//��������Key 
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
	strcpy_s(m_nid.szTip, sizeof(m_nid.szTip), APP_NAME);
	::Shell_NotifyIcon(NIM_ADD,&m_nid);

	//SetDlgItemText(IDC_BUTTON1, m_bPause ? "Continue" : "Pause");
	//ShowWindow(SW_SHOWMINIMIZED);

	// ��Ӽ��̹���
	if (g_hHook == NULL)
	{
		g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,  AfxGetInstanceHandle(), NULL);
	}

	// ��ʼ���ؼ�״̬
	m_chkAutoRun.SetCheck(g_iniConfig.GetValueI("UserInfo", "AutoRun"));
	m_chkRunMin.SetCheck(g_iniConfig.GetValueI("UserInfo", "RunMin"));

	if (m_chkRunMin.GetCheck() == BST_CHECKED)
	{
		//this->ShowWindow(SW_HIDE);
		::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		
		//���̷���������Ϣ

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

//��������
void CAutoCloseWndDlg::AddStartupRun()
{
	HKEY   hKey; 
	char pFileName[MAX_PATH] = {0}; 
	//�õ����������ȫ·�� 
	DWORD dwRet = GetModuleFileName(NULL, pFileName, MAX_PATH);

	//CString strWriteToReg;
	//strWriteToReg.Format("\"%s\"", pFileName);//����˫���ţ����·�����пո�����⡣

	//�ҵ�ϵͳ�������� 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	//��������Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//���ע��
		lRet = RegSetValueEx(hKey, APP_NAME, 0,REG_SZ,(const BYTE*)(LPCSTR)pFileName, MAX_PATH);
		RegCloseKey(hKey); 
	}
}

//ȡ����������
void CAutoCloseWndDlg::DeleteStartupRun()
{
	HKEY   hKey;
	char pFileName[MAX_PATH] = {0}; 
	//�õ����������ȫ·�� 
	DWORD dwRet = GetModuleFileNameW(NULL, (LPWCH)pFileName, MAX_PATH); 
	//�ҵ�ϵͳ�������� 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	//��������Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//ɾ��ע��
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
	g_iniConfig.WriteFile();//todo: ����ע��
}

void CAutoCloseWndDlg::OnBnClickedRunMin()
{
	g_iniConfig.SetValueI("UserInfo", "RunMin", m_chkRunMin.GetCheck());
	g_iniConfig.WriteFile();//todo: ����ע��
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
