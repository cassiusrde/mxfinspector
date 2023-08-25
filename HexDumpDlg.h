#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include <vector>

#include "hexeditbase.h"

// CGoToDlg dialog

class CHexDumpDlg : public CDialog
{
	DECLARE_DYNAMIC(CHexDumpDlg)

public:
	CHexDumpDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHexDumpDlg();	

	void SetMemory(const unsigned char *buffer, int len);

// Dialog Data
	enum { IDD = IDD_DUMP_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()	
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
private:
	CHexEditBase				m_editHex;
	int							m_size;
	std::vector<unsigned char>	m_buffer;
};
