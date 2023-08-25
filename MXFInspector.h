// MXFInspector.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMXFInspectorApp:
// See MXFInspector.cpp for the implementation of this class
//

class CMXFInspectorApp : public CWinApp
{
public:
	CMXFInspectorApp();

	HACCEL m_haccel;

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
};

extern CMXFInspectorApp theApp;