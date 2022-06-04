//this file is part of eMule
//Copyright (C)2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "emule.h"
#include "ModVersion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT CemuleApp::m_nMVersionMjr = MOD_VERSION_MJR;
const UINT CemuleApp::m_nMVersionMin = MOD_VERSION_MIN;
const UINT CemuleApp::m_nMVersionBld = MOD_VERSION_BUILD;
const TCHAR CemuleApp::m_szMVersionLong[] = MOD_VERSION_LONG;
const TCHAR CemuleApp::m_szMVersion[] = MOD_VERSION;