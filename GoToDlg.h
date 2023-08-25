#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "HexEdit.h"


// CGoToDlg dialog

class CGoToDlg : public CDialog
{
	DECLARE_DYNAMIC(CGoToDlg)

public:
	CGoToDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGoToDlg();

	unsigned __int64 GoTo();

// Dialog Data
	enum { IDD = IDD_GO_TO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	
	
	CHexEdit m_editCtrl;
	afx_msg void OnEnUpdateEdit();
};
