// GoToDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MXFInspector.h"
#include "GoToDlg.h"


// CGoToDlg dialog

IMPLEMENT_DYNAMIC(CGoToDlg, CDialog)

CGoToDlg::CGoToDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGoToDlg::IDD, pParent)
{

}

CGoToDlg::~CGoToDlg()
{
}

void CGoToDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);	
	DDX_Control(pDX, IDC_EDIT, m_editCtrl);
}


BEGIN_MESSAGE_MAP(CGoToDlg, CDialog)	
	//ON_EN_UPDATE(IDC_EDIT, &CGoToDlg::OnEnUpdateEdit)
END_MESSAGE_MAP()

BOOL CGoToDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_editCtrl.SetFocus();
	m_editCtrl.SetSel(1, 1); // Coloca o foco no final do EditBox.

	return FALSE;
}

unsigned __int64 CGoToDlg::GoTo()
{
	return m_editCtrl.GetHex();
}
