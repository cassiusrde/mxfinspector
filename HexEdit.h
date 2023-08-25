#if !defined(AFX_DELAYEDIT_H__88227B96_24F0_4BFF_AEE6_BB18993D3BA0__INCLUDED_)
#define AFX_DELAYEDIT_H__88227B96_24F0_4BFF_AEE6_BB18993D3BA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HexEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHexEdit window

class CHexEdit : public CEdit
{
	DECLARE_DYNAMIC(CHexEdit)
		// Construction
public:
	CHexEdit();

	unsigned __int64 GetHex();
	
	// Attributes
public:	
	CString m_strLiteral;		
	CString m_strMaskLiteral;
	
	// Operations
	BOOL CheckChar(UINT nChar);
	void SendChar(UINT nChar);
	
public:
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDelayEdit)
	//}}AFX_VIRTUAL	
	// Generated message map functions
protected:
	//{{AFX_MSG(CDelayEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);	
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG	
	DECLARE_MESSAGE_MAP()

	
};

#endif 
