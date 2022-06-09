// COptionTree
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgement.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 1999-2002 
// ComputerSmarts.net 
// mattrmiller@computersmarts.net

#include "stdafx.h"
#include "OptionTreeItemStatic.h"
#include "OptionTree.h"
#include "OptionTreeDef.h"
#include "..\otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionTreeItemStatic::COptionTreeItemStatic() : m_strStaticText(_T(""))
{
	// Initialize variables
	m_dwOptions = NULL;

	// Set item type
	SetItemType(OT_ITEM_STATIC);
}

COptionTreeItemStatic::~COptionTreeItemStatic()
{

}

void COptionTreeItemStatic::DrawAttribute(CDC *pDC, const RECT &rcRect)
{
	// Make sure option is not NULL
	if (m_otOption == NULL)
	{
		return;
	}

	// Declare variables
	CFont *pOldFont;
	COLORREF crOld;
	int nOldBack;
	CRect rcText;
	COLORREF crOldBack;

	// Select font
	pOldFont = pDC->SelectObject(m_otOption->GetNormalFont());
	
	// Set text color
	if (IsReadOnly() == TRUE || m_otOption->IsWindowEnabled() == FALSE)
	{
		crOld = pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}
	else
	{
		crOld = pDC->SetTextColor(GetTextColor());
	}

	// Set background mode
	nOldBack = pDC->SetBkMode(TRANSPARENT);

	// Set background color
	crOldBack = pDC->SetBkColor(GetBackgroundColor());	

	// Get rectangle
	rcText = rcRect;

	// Draw text
	pDC->DrawText(m_strStaticText, rcText, IsRightToLeftLanguage() ?
		DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_RTLREADING :
		DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);

	// Restore GDI ojects
	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(crOld);
	pDC->SetBkMode(nOldBack);
	pDC->SetBkColor(crOldBack);
}	

CString COptionTreeItemStatic::GetStaticText()
{
	// Return text
	return m_strStaticText;
}

void COptionTreeItemStatic::SetStaticText(CString strStaticText)
{
	// Set variables
	m_strStaticText = strStaticText;
}

void COptionTreeItemStatic::CleanDestroyWindow()
{
	// Do nothing here
}

void COptionTreeItemStatic::OnCommit()
{
	// Do nothing here
}

void COptionTreeItemStatic::OnRefresh()
{
	// Do nothing here
}

void COptionTreeItemStatic::OnMove()
{
	// Do nothing here
}

void COptionTreeItemStatic::OnActivate()
{
	// Do nothing here
}

void COptionTreeItemStatic::OnDeSelect()
{
	// Do nothing here
}

void COptionTreeItemStatic::OnSelect()
{
	// Do nothing here
}

BOOL COptionTreeItemStatic::GetOption(DWORD dwOption)
{
	// Return option
	return (m_dwOptions & dwOption) ? TRUE : FALSE;
}

void COptionTreeItemStatic::SetOption(DWORD dwOption, BOOL bSet)
{
	// Set option
	if (bSet == TRUE)
	{
		m_dwOptions |= dwOption;
	}
	else
	{
		m_dwOptions &= ~dwOption;
	}	
}

BOOL COptionTreeItemStatic::CreateStaticItem(DWORD dwOptions)
{
	// Make sure options is not NULL
	if (m_otOption == NULL)
	{
		return FALSE;
	}

	// Save options
	m_dwOptions = dwOptions;

	// Always return TRUE
	return TRUE;
}
