//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2000 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

// KeyHelper.cpp: implementation of the CBCGKeyHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KeyHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGKeyHelper::CBCGKeyHelper(LPACCEL lpAccel) :
	m_lpAccel (lpAccel)
{
}
//*******************************************************************
CBCGKeyHelper::CBCGKeyHelper() :
	m_lpAccel (NULL)
{
}
//*******************************************************************
CBCGKeyHelper::~CBCGKeyHelper()
{
}
//*******************************************************************
void CBCGKeyHelper::Format (CString& str) const
{
	str.Empty ();

	if (m_lpAccel == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (m_lpAccel->fVirt & FCONTROL)
	{
		//AddVirtKeyStr (str, VK_CONTROL);
		str += _T("Ctrl+");
	}

	if (m_lpAccel->fVirt & FSHIFT)
	{
		//AddVirtKeyStr (str, VK_SHIFT);
		str += _T("Shift+");
	}

	if (m_lpAccel->fVirt & FALT)
	{
		//AddVirtKeyStr (str, VK_MENU);
		str += _T("Alt+");
	}

	if (m_lpAccel->fVirt & FVIRTKEY)
	{
		TCHAR keyname[64];
		UINT vkey = MapVirtualKey(m_lpAccel->key, 0)<<16;
		GetKeyNameText(vkey, keyname, sizeof(keyname));
		str += keyname;
	}
	else if (m_lpAccel->key != 27)	// Don't print esc
	{
		str += (char) m_lpAccel->key;
	}
}
//******************************************************************
void CBCGKeyHelper::AddVirtKeyStr (CString& str, UINT uiVirtKey, BOOL bLast) const
{
	//
	// This file was modified by Sven Ritter
	//

	#define BUFFER_LEN 50
	TCHAR szBuffer [BUFFER_LEN + 1];

	TRACE("KeyboardLayout: 0x%x\n", ::GetKeyboardLayout (0));

	UINT nScanCode = ::MapVirtualKeyEx (uiVirtKey, 0, 
		::GetKeyboardLayout (0)) <<16 | 0x1;
	
	if (uiVirtKey >= VK_PRIOR && uiVirtKey <= VK_HELP)
	{
		nScanCode |= 0x01000000;
	}
	
	::GetKeyNameText (nScanCode, szBuffer, BUFFER_LEN);
	
	CString strKey(szBuffer);
	strKey.MakeLower();
	
	//--------------------------------------
	// The first letter should be uppercase:
	//--------------------------------------
	for (int nCount = 0; nCount < strKey.GetLength(); nCount++)
	{
		TCHAR c = strKey[nCount];
		if (IsCharLower (c))
		{
			c = (TCHAR) toupper (c); // Convert single character JY 4-Dec-99
			strKey.SetAt (nCount, c);
			break;
		}
	}

	str += strKey;
	
	if (!bLast)
	{
		str += '+';
	}
}

