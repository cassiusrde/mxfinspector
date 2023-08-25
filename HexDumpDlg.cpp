// HexDumpDlg.cpp : implementation file
//

#include "stdafx.h"

#include "MXFInspector.h"
#include "HexDumpDlg.h"

#include <sstream>


// CHexDumpDlg dialog

IMPLEMENT_DYNAMIC(CHexDumpDlg, CDialog)

CHexDumpDlg::CHexDumpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHexDumpDlg::IDD, pParent)
{
	CHexEditBase::RegisterClass();
	m_size = 0;
}

CHexDumpDlg::~CHexDumpDlg()
{
}

void CHexDumpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);		
	DDX_Control(pDX, IDC_EDIT_DUMP, m_editHex);
}

BEGIN_MESSAGE_MAP(CHexDumpDlg, CDialog)		
	ON_BN_CLICKED(IDOK, &CHexDumpDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CHexDumpDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CHexDumpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();			
	
	// via DDX connected	
	m_editHex.SetNotUsedCol(RGB(255,255,255));

	m_editHex.SetAddressSize(8, false);
	m_editHex.SetAdrCol(RGB(255,255,255), RGB(125,125,125));
	m_editHex.SetShowAddress(true, false);

	m_editHex.SetBytesPerRow(16, false, false);
	m_editHex.SetHexCol(RGB(255,255,255), RGB(0,0,0));	
	m_editHex.SetReadonly(true);
	if( m_size )
		m_editHex.SetData(&m_buffer[0], m_size);

	return FALSE;
}

void CHexDumpDlg::SetMemory(const unsigned char *buffer, int len)
{
	m_buffer.clear();
	m_buffer.resize(len);
	std::copy(buffer, buffer + len, m_buffer.begin());
	m_size = len;
}

void CHexDumpDlg::OnBnClickedOk()
{
	OnOK();
}

void CHexDumpDlg::OnBnClickedCancel()
{
	OnCancel();
}
