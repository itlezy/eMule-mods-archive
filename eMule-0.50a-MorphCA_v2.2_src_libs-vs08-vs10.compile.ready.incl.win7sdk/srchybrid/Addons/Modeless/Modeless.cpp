//this file is part of NeoMule
//Copyright (C)2007 David Xanatos ( XanatosDavid@googlemail.com / http://neomule.sourceforge.net )
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
#include "Modeless.h"
#include "emule.h"
#include "emuleDlg.h"
#include "UserMsgs.h"

CWnd* GetEmuleDlg() {return theApp.emuledlg;}

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// NEO: MLD - [ModelesDialogs] -- Xanatos -->

#if _MFC_VER>=0x0800

// MFC for Visual C++ 8.0 (MFC 8.x)
#undef BEGIN_MESSAGE_MAP
#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	PTM_WARNING_DISABLE \
	template<>const AFX_MSGMAP* theClass::GetMessageMap() const \
		{ return GetThisMessageMap(); } \
	template<>const AFX_MSGMAP* PASCAL theClass::GetThisMessageMap() \
	{ \
		typedef theClass ThisClass;						   \
		typedef baseClass TheBaseClass;					   \
		static const AFX_MSGMAP_ENTRY _messageEntries[] =  \
		{
#endif
BEGIN_MESSAGE_MAP(CModDialog, CDialog)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CModWin<CResizableDialog>, CResizableDialog)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CModSht<CTreePropSheet>, CTreePropSheet)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CModSht<CResizableSheet>, CResizableSheet)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()
/*
BEGIN_MESSAGE_MAP(CModSht<CListViewWalkerPreferenceSheet>, CListViewWalkerPreferenceSheet)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()
*/
BEGIN_MESSAGE_MAP(CModSht<CListViewWalkerPropertySheet>, CListViewWalkerPropertySheet)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()
/*
BEGIN_MESSAGE_MAP(CModWlkSht<CListViewWalkerPreferenceSheet>, CListViewWalkerPreferenceSheet)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()
*/
BEGIN_MESSAGE_MAP(CModWlkSht<CListViewWalkerPropertySheet>, CListViewWalkerPropertySheet)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

// NEO: MLD END <-- Xanatos --