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
#include "OptionTreeSpinnerEdit.h"

// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeSpinnerEdit

COptionTreeSpinnerEdit::COptionTreeSpinnerEdit()
{
	// Initialize variables
	m_otSpinnerButton = NULL;
}

COptionTreeSpinnerEdit::~COptionTreeSpinnerEdit()
{
}


BEGIN_MESSAGE_MAP(COptionTreeSpinnerEdit, CEdit)
	//{{AFX_MSG_MAP(COptionTreeSpinnerEdit)
	ON_WM_KEYDOWN()
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_CONTROL_REFLECT(EN_CHANGE, OnTextChange)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionTreeSpinnerEdit message handlers

void COptionTreeSpinnerEdit::SetOwnerSpinner(COptionTreeSpinnerButton *otSpinnerButton)
{
	// Save pointer
	m_otSpinnerButton = otSpinnerButton;
}

void COptionTreeSpinnerEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Find what key was presed
	if (nChar == VK_UP)
	{
		m_otSpinnerButton->SendMessage(OT_NOTIFY_UP, 0, 0);
	}
	else if (nChar == VK_DOWN)
	{
		m_otSpinnerButton->SendMessage(OT_NOTIFY_DOWN, 0, 0);
	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL COptionTreeSpinnerEdit::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// User edit
	if(m_otSpinnerButton != NULL)
	{
		// -- No user edit
		if (m_otSpinnerButton->GetOption(OT_EDIT_USEREDIT) == FALSE)
		{
			// -- -- No paste or cut
			if (LOWORD(wParam) == WM_PASTE || LOWORD(wParam) == WM_CUT)
			{
				return FALSE;
			}
		}
		// -- User edit
		else
		{
			switch (LOWORD(wParam))
			{
				case EM_UNDO:
				case WM_CUT:
				case WM_COPY:
				case WM_CLEAR:
				case WM_PASTE:
					return SendMessage(LOWORD(wParam));

				case OT_MES_NSELECTALL:
					return SendMessage(EM_SETSEL, 0, -1);

				default:
					return CEdit::OnCommand(wParam, lParam);
			}
		}
	}

	return FALSE;
}

int COptionTreeSpinnerEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// Create edit
	if (CEdit::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	
	
	return 0;
}

void COptionTreeSpinnerEdit::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    // Declare variables
	CMenu mMenu;
	BOOL bReadOnly;
	DWORD dwFlags;
	int nSelStart, nSelEnd;
	int nLen;

	// Make sure we have options
	if (m_otSpinnerButton == NULL)
	{
		return;
	}

    // Crete menu
	mMenu.CreatePopupMenu();

    // Get read only
	bReadOnly = GetStyle() & ES_READONLY;
	
	// Get selection
	GetSel(nSelStart, nSelEnd);

	// Get window length
	nLen = GetWindowTextLength();

	// Insert Undo
	if (m_otSpinnerButton->GetOption(OT_EDIT_USEREDIT) == TRUE)
	{
		dwFlags = CanUndo() && !bReadOnly ? 0 : MF_GRAYED | MF_DISABLED;
	}
	else
	{
		dwFlags = MF_GRAYED | MF_DISABLED;
	}
    mMenu.InsertMenu(0, MF_BYPOSITION | dwFlags, EM_UNDO, OT_MES_UNDO);
	
	// Insert seperator
    mMenu.InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);

    // Insert copy
	if (nSelStart == nSelEnd)
	{
		dwFlags = MF_GRAYED | MF_DISABLED;
	}
	else
	{
		dwFlags = 0;
	}
    mMenu.InsertMenu(2, MF_BYPOSITION | dwFlags, WM_COPY, OT_MES_COPY);

	// Insert cut and clear
	if (m_otSpinnerButton->GetOption(OT_EDIT_USEREDIT) == TRUE)
	{
		if (nSelStart == nSelEnd || bReadOnly == TRUE)
		{
			dwFlags = MF_GRAYED | MF_DISABLED;
		}
		else
		{
			dwFlags = 0;
		}
	}
	else
	{
		dwFlags = MF_GRAYED | MF_DISABLED;
	}
    mMenu.InsertMenu(2, MF_BYPOSITION | dwFlags, WM_CUT, OT_MES_CUT);
    mMenu.InsertMenu(4, MF_BYPOSITION | dwFlags, WM_CLEAR, OT_MES_DELETE);

    // Insert paste
	if (m_otSpinnerButton->GetOption(OT_EDIT_USEREDIT) == TRUE)
	{
		if (IsClipboardFormatAvailable(CF_TEXT) == FALSE || bReadOnly == TRUE)
		{
			dwFlags = MF_GRAYED | MF_DISABLED;
		}
		else
		{
			dwFlags = 0;
		}
	}
	else
	{
		dwFlags = MF_GRAYED | MF_DISABLED;
	}
    mMenu.InsertMenu(4, MF_BYPOSITION | dwFlags, WM_PASTE, OT_MES_PASTE);

	// Insert seperator
    mMenu.InsertMenu(6, MF_BYPOSITION | MF_SEPARATOR);

    // Insert select all
    mMenu.InsertMenu(7, MF_BYPOSITION | dwFlags, OT_MES_NSELECTALL, OT_MES_SELECTALL);

    // Show menu
	mMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);	
}

void COptionTreeSpinnerEdit::OnKillFocus(CWnd* pNewWnd) 
{
	// Make sure we have options
	if (m_otSpinnerButton != NULL)
	{
		// -- If new focus is not parent then sent message to item to commit
		if (pNewWnd->GetSafeHwnd() != m_otSpinnerButton->GetSafeHwnd())
		{
			::PostMessage(m_otSpinnerButton->GetSafeHwnd(), OT_NOTIFY_COMMITCHANGES, NULL, NULL);
		}			
	}
	
	CEdit::OnKillFocus(pNewWnd);	
}

void COptionTreeSpinnerEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);	
}

void COptionTreeSpinnerEdit::OnTextChange() 
{
	// Force spinner button redraw
	::PostMessage(m_otSpinnerButton->GetSafeHwnd(), OT_NOTIFY_FORCEREDRAW, NULL, NULL);	
}

BOOL COptionTreeSpinnerEdit::PreTranslateMessage(MSG* pMsg) 
{

	return CEdit::PreTranslateMessage(pMsg);
}

void COptionTreeSpinnerEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Disallow user edit
	if(m_otSpinnerButton != NULL)
	{
		// -- No user edit
		if (m_otSpinnerButton->GetOption(OT_EDIT_USEREDIT) == FALSE)
		{
			return;
		}
	}
			
		
	
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
