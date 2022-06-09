//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "resource.h"
#include "otherfunctions.h"
#include "KeyboardShortcut.h"
#include "emule.h"

short GetCodeFromPressedKeys(MSG *pMsg)
{
	short nCode = MASK_ENABLED;

	if (GetAsyncKeyState(VK_MENU) < 0)
		nCode |= MASK_ALT;
	if (GetAsyncKeyState(VK_CONTROL) < 0)
		nCode |= MASK_CTRL;
	if (GetAsyncKeyState(VK_SHIFT) < 0)
		nCode |= MASK_SHIFT;
	if ((GetAsyncKeyState(VK_LWIN) < 0) || (GetAsyncKeyState(VK_RWIN) < 0))
		nCode |= MASK_WIN;
	nCode |= static_cast<uchar>(pMsg->wParam & MASK_KEY);

	return nCode;
}

CString GetStringFromShortcutCode(unsigned uiResID, int iIdx, int iMode)
{
	CString	String;
	int		iCode;

	if (uiResID != 0)
		::GetResString(&String, uiResID);

	iCode = g_App.m_pPrefs->GetShortcutCode(iIdx);
	if (((iCode & MASK_ENABLED) != 0) || ((iMode & SSP_RETNONE) != 0))
	{
		if (iMode & SSP_TAB_PREFIX)
			String += _T('\t');
		else if (iMode & SSP_SPACE_PREFIX)
			String += _T(": ");

		if ((iCode & MASK_ENABLED) != 0) 
		{
			if ((iCode & MASK_ALT) != 0)
				String += _T("Alt+");
			if ((iCode & MASK_CTRL) != 0)
				String += _T("Ctrl+");
			if ((iCode & MASK_SHIFT) != 0)
				String += _T("Shift+");
			if ((iCode & MASK_WIN) != 0)
				String +=_T("Win+");

			uchar byteKey = static_cast<uchar>(iCode & MASK_KEY);

			if ((byteKey >= VK_F1) && (byteKey <= VK_F12)) // function keys
				String.AppendFormat(_T("F%u"), byteKey - VK_F1 + 1);
			else if ((byteKey >= 'A') && (byteKey <= 'Z')) // letter keys
				String.AppendChar(byteKey);
			else if ((byteKey >= '0') && (byteKey <= '9')) // number keys
				String.AppendChar(byteKey);
			else
			{
				UINT	dwResStrId = 0;

				if (byteKey == VK_RETURN)	// Enter key
					dwResStrId = IDS_SHORTCUTS_ENTER_KEY;
				else if (byteKey == VK_SPACE) // Spacebar
					dwResStrId = IDS_SHORTCUTS_SPACE_KEY;
				else if (byteKey == VK_TAB) // Tab key
					dwResStrId = IDS_SHORTCUTS_TAB_KEY;
				else if (byteKey == VK_BACK) // Bakcspace key
					dwResStrId = IDS_SHORTCUTS_BACK_KEY;
				else if (byteKey == VK_INSERT) // Insert key
					dwResStrId = IDS_SHORTCUTS_INSERT_KEY;
				else if (byteKey == VK_DELETE) // Delete key
					dwResStrId = IDS_SHORTCUTS_DELETE_KEY;
				else if (byteKey == VK_HOME) // Home key
					dwResStrId = IDS_SHORTCUTS_HOME_KEY;
				else if (byteKey == VK_END) // End key
					dwResStrId = IDS_SHORTCUTS_END_KEY;
				else if (byteKey == VK_PRIOR) // Page Up key
					dwResStrId = IDS_SHORTCUTS_PAGEUP_KEY;
				else if (byteKey == VK_NEXT) // Page Down key
					dwResStrId = IDS_SHORTCUTS_PAGEDOWN_KEY;

				if (dwResStrId != 0)
					String += ::GetResString(dwResStrId);
				else	/*** Should never happen ***/
					String += _T("ERROR");
			}
		}
		else
			String += GetResString(IDS_SHORTCUTS_NONE);
	}

	return String;
}


CKeyboardShortcut::CKeyboardShortcut(void)
: m_bEnabled(false)
, m_bAlt(false)
, m_bCtrl(false)
, m_bShift(false)
, m_bWin(false)
, m_byteKey(VK_RETURN)
{
}

CKeyboardShortcut::CKeyboardShortcut(short nCode)
{
	SetCode(nCode);
}

CKeyboardShortcut::~CKeyboardShortcut(void)
{
}

void CKeyboardShortcut::SetCode(short nCode)
{
	m_bEnabled	= (nCode & MASK_ENABLED) ? true : false;
	m_bAlt		= (nCode & MASK_ALT) ? true : false;
	m_bCtrl		= (nCode & MASK_CTRL) ? true : false;
	m_bShift	= (nCode & MASK_SHIFT) ? true : false;
	m_bWin		= (nCode & MASK_WIN) ? true : false;
	m_byteKey	= static_cast<uchar>(nCode & MASK_KEY);
}
