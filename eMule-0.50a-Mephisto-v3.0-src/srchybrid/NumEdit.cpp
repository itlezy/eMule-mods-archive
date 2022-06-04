//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "NumEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CNumEdit

IMPLEMENT_DYNAMIC(CNumEdit, CEdit)
CNumEdit::CNumEdit()
{
}

CNumEdit::~CNumEdit()
{
}

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
	ON_WM_CHAR()
	ON_MESSAGE(WM_PASTE,OnPaste)
END_MESSAGE_MAP()

// CNumEdit message handlers

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	// Check new Char
	if((nChar <  VK_SPACE) || (nChar >= '0' && nChar <= '9')){
		// Foward char
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
	else if(nChar == '.' || nChar == ','){
		// Check if those chars have been already used
		CString buffer;
		GetWindowText(buffer);
		if(buffer.FindOneOf(_T(".,")) == -1){ //Xman unicode

			// Foward char
			CEdit::OnChar(nChar, nRepCnt, nFlags);
		}
	}
}

LONG CNumEdit::OnPaste(UINT /*wParam*/, LONG /*lParam*/)
{
	// To do.....	
	return 0L;
}

