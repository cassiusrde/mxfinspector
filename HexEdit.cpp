// HexEdit.cpp : implementation file
//

#include "stdafx.h"
#include "HexEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHexEdit class

IMPLEMENT_DYNAMIC(CHexEdit, CEdit)

BEGIN_MESSAGE_MAP(CHexEdit, CEdit)
//{{AFX_MSG_MAP(CHexEdit)
ON_WM_CHAR()
ON_WM_KEYDOWN()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CHexEdit::CHexEdit()
{	
	m_strLiteral = _T("");
	m_strMaskLiteral = _T("0000000000000000");
}

void CHexEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(!CheckChar(nChar)) 
		return;

	int startPos, endPos;
	GetSel(startPos, endPos);

	if (isprint(nChar))
	{	
		m_strLiteral.Insert(startPos, nChar);		
	}
	else if (nChar == VK_BACK)
	{
		//Confere se a string não é vazia.
		if (!m_strLiteral.IsEmpty())			
			m_strLiteral.Delete(endPos-1, 1);
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);	
}

BOOL CHexEdit::CheckChar(UINT nChar)
{
	// Controle de caracteres.
	if( nChar == VK_BACK )
		return TRUE;
			
	// Faz um 'unselect' dos ítens(se houver).
	int startPos, endPos;
	GetSel(startPos, endPos);
	SetSel(-1, 0);
	SetSel(startPos, startPos);
	
	// Confere a tecla com a máscara.
	GetSel(startPos, endPos);
	
	// Confere se a string não é maior que a máscara.
	if (m_strLiteral.GetLength() >= m_strMaskLiteral.GetLength())
	{
		MessageBeep((UINT)-1);
		return FALSE;
	}

	// Verifica se o caracter é válido.
	if (!isxdigit(nChar)) 
	{
		MessageBeep((UINT)-1);
		return FALSE;
	}

	return TRUE;	
}

void CHexEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
	case VK_DELETE:
	case VK_INSERT: 
		return;
	}
	
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}


unsigned __int64 CHexEdit::GetHex()
{
	return _wcstoui64(m_strLiteral, NULL, 16);
}