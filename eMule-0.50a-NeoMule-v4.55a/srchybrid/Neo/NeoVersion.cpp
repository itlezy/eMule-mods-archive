//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

#include "StdAfx.h"
#include "emule.h"
#include "Functions.h"
#include "NeoVersion.h"

// NEO: NV - [NeoVersion] -- Xanatos -->
CString CemuleApp::GetAppTitle(bool bE){
	/* Xanatos:
	* The Proper Name is "Neo Mule" not "Neo eMule", 
	* but the Online Signaure need "eMule" in the Window Title,
	* so we left "eMule" in the Title of the Main Window,
	* to prevent troubles...
	*/

#ifdef NEO_MODER
 #ifdef MOD_EXT
	static CString ModName = StrLine(_T("%s (%s) v%s by: %s"), MOD_NAME, MOD_EXT, theApp.m_strNeoVersionLong, NEO_MODER);
	static CString ModTitle = StrLine(_T("%s (%s) v%s by: %s"), MOD_TITLE, MOD_EXT, theApp.m_strNeoVersionLong, NEO_MODER);
 #else
	static CString ModName = StrLine(_T("%s v%s by: %s"), MOD_NAME, theApp.m_strNeoVersionLong, NEO_MODER);
	static CString ModTitle = StrLine(_T("%s v%s by: %s"), MOD_TITLE, theApp.m_strNeoVersionLong, NEO_MODER);
 #endif
#else
 #ifdef MOD_EXT
	static CString ModName = StrLine(_T("%s (%s) v%s"), MOD_NAME, MOD_EXT, theApp.m_strNeoVersionLong);
	static CString ModTitle = StrLine(_T("%s (%s) v%s"), MOD_TITLE, MOD_EXT, theApp.m_strNeoVersionLong);
 #else
	static CString ModName = StrLine(_T("%s v%s"), MOD_NAME, theApp.m_strNeoVersionLong);
	static CString ModTitle = StrLine(_T("%s v%s"), MOD_TITLE, theApp.m_strNeoVersionLong);
 #endif
#endif

	return bE ? ModTitle : ModName;
}
// NEO: NV END <-- Xanatos --
