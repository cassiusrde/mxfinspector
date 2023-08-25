// MXFInspector.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MXFInspector.h"
#include "MXFInspectorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMXFInspectorApp

BEGIN_MESSAGE_MAP(CMXFInspectorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMXFInspectorApp construction

CMXFInspectorApp::CMXFInspectorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CMXFInspectorApp object

CMXFInspectorApp theApp;


// CMXFInspectorApp initialization

BOOL CMXFInspectorApp::InitInstance()
{

	/*BOOL ret = FALSE;
	LARGE_INTEGER offset = {0}, numberOfBytes = {0};
	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = CreateFile(L"M:\\Avid MediaFiles\\MXF\\1\\msmMMOB.mdb", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	numberOfBytes.QuadPart = 2147483647;
	ret = LockFile(hFile, offset.LowPart, offset.HighPart, numberOfBytes.LowPart, numberOfBytes.HighPart);

	ret = UnlockFile(hFile, offset.LowPart, offset.HighPart, numberOfBytes.LowPart, numberOfBytes.HighPart);

	CloseHandle(hFile);*/


//TODO: call AfxInitRichEdit2() to initialize richedit2 library.
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	m_haccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));

	CMXFInspectorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


BOOL CMXFInspectorApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	if(m_haccel)
    {
        if (::TranslateAccelerator(m_pMainWnd->m_hWnd, m_haccel, lpMsg)) 
            return(TRUE);
    }

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}
