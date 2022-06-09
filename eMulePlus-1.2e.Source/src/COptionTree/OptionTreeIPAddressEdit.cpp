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
#include "OptionTreeIPAddressEdit.h"

// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeIPAddressEdit

COptionTreeIPAddressEdit::COptionTreeIPAddressEdit()
{
	// Initialize variables
	m_wndNotify = NULL;
}

COptionTreeIPAddressEdit::~COptionTreeIPAddressEdit()
{
}


BEGIN_MESSAGE_MAP(COptionTreeIPAddressEdit, CEdit)
	//{{AFX_MSG_MAP(COptionTreeIPAddressEdit)
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(EN_MAXTEXT, OnMaxText)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionTreeIPAddressEdit message handlers

void COptionTreeIPAddressEdit::OnKillFocus(CWnd* pNewWnd) 
{
	// Notify user
	if (IsWindow(m_wndNotify))
	{
		::SendMessage(m_wndNotify, OT_IPADDRESS_KILLFOCUS, (WPARAM) pNewWnd, NULL); 
	}

	CEdit::OnKillFocus(pNewWnd);	
}

void COptionTreeIPAddressEdit::SetNotifyWnd(HWND hWnd)
{
	// Save window
	m_wndNotify = hWnd;
}

void COptionTreeIPAddressEdit::OnMaxText() 
{
	// Notify user
	if (IsWindow(m_wndNotify))
	{
		::SendMessage(m_wndNotify, OT_IPADDRESS_NEXTEDIT, (WPARAM) this, NULL); 
	}	
}

void COptionTreeIPAddressEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// If return is pressed and not multiline, commit
	if (IsWindow(GetSafeHwnd()))
	{
		if (nChar == _T('.'))
		{
			// -- Notify user
			if (IsWindow(m_wndNotify))
			{
				::SendMessage(m_wndNotify, OT_IPADDRESS_NEXTEDIT, (WPARAM) this, NULL); 
			}
		}
	}
	
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}
