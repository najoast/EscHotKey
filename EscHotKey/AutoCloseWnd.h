// AutoCloseWnd.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CAutoCloseWndApp:
// �йش����ʵ�֣������ AutoCloseWnd.cpp
//

class CAutoCloseWndApp : public CWinApp
{
public:
	CAutoCloseWndApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CAutoCloseWndApp theApp;