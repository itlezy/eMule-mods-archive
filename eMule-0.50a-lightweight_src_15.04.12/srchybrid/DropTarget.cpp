//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "emuledlg.h"
#include "DropTarget.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define	PREFIXA	"ed2k://"

BOOL IsUrlSchemeSupportedA(LPCSTR pszUrl)
{
	/*static const struct SCHEME
	{
		LPCSTR pszPrefix;
		size_t iLen;
	} _aSchemes[] = 
	{
#define SCHEME_ENTRY(prefix)	{ prefix, _countof(prefix)-1 }
		SCHEME_ENTRY("ed2k://")
#undef SCHEME_ENTRY
	};

	for (size_t i = 0; i < ARRSIZE(_aSchemes); i++)
	{
		if (strncmp(pszUrl, _aSchemes[i].pszPrefix, _aSchemes[i].iLen) == 0)
			return TRUE;
	}*/
	if (strncmp(pszUrl, PREFIXA, _countof(PREFIXA)-1) == 0)
		return TRUE;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// CMainFrameDropTarget

CMainFrameDropTarget::CMainFrameDropTarget()
{
	m_bDropDataValid = FALSE;
}

HRESULT CMainFrameDropTarget::PasteText(CLIPFORMAT cfData, COleDataObject& data)
{
	HRESULT hrPasteResult = E_FAIL;
	HANDLE hMem;
	if ((hMem = data.GetGlobalData(cfData)) != NULL)
	{
		LPCSTR pszUrlA;
		if ((pszUrlA = (LPCSTR)GlobalLock(hMem)) != NULL)
		{
			// skip white space
			while (isspace((unsigned char)*pszUrlA))
				pszUrlA++;
			
			hrPasteResult = S_FALSE; // default: nothing was pasted
			if (_strnicmp(pszUrlA, "ed2k://|", 8) == 0)
			{
				CString strData(pszUrlA);
				int iPos = 0;
				CString str = strData.Tokenize(_T("\r\n"), iPos);
				while (!str.IsEmpty())
				{
					theApp.emuledlg->ProcessED2KLink(str);
					hrPasteResult = S_OK;
					str = strData.Tokenize(_T("\r\n"), iPos);
				}
			}

			GlobalUnlock(hMem);
		}
		GlobalFree(hMem);
	}
	return hrPasteResult;
}

BOOL CMainFrameDropTarget::IsSupportedDropData(COleDataObject* pDataObject)
{
	//************************************************************************
	//*** THIS FUNCTION HAS TO BE AS FAST AS POSSIBLE!!!
	//************************************************************************

	if (!pDataObject->IsDataAvailable(CF_TEXT))
		// Unknown data format
		return FALSE;

	//
	// Check text data
	//
	BOOL bResult = FALSE;

	HANDLE hMem;
	if ((hMem = pDataObject->GetGlobalData(CF_TEXT)) != NULL)
	{
		LPCSTR lpszUrl;
		if ((lpszUrl = (LPCSTR)GlobalLock(hMem)) != NULL)
		{
			// skip white space
			while (isspace((unsigned char)*lpszUrl))
				lpszUrl++;
			bResult = IsUrlSchemeSupportedA(lpszUrl);
			GlobalUnlock(hMem);
		}
		GlobalFree(hMem);
	}
	return bResult;
}

DROPEFFECT CMainFrameDropTarget::OnDragEnter(CWnd*, COleDataObject* pDataObject, DWORD, CPoint)
{
	m_bDropDataValid = IsSupportedDropData(pDataObject);
	return m_bDropDataValid ? DROPEFFECT_COPY : DROPEFFECT_NONE;
}

DROPEFFECT CMainFrameDropTarget::OnDragOver(CWnd*, COleDataObject*, DWORD, CPoint)
{
	return m_bDropDataValid ? DROPEFFECT_COPY : DROPEFFECT_NONE;
}

BOOL CMainFrameDropTarget::OnDrop(CWnd*, COleDataObject* pDataObject, DROPEFFECT /*dropEffect*/, CPoint /*point*/)
{
	if (m_bDropDataValid)
	{
		if (pDataObject->IsDataAvailable(CF_TEXT))
		{
			PasteText(CF_TEXT, *pDataObject);
		}
		return TRUE;
	}
	return FALSE;
}

void CMainFrameDropTarget::OnDragLeave(CWnd*)
{
	// Do *NOT* set 'm_bDropDataValid=FALSE'!
	// 'OnDragLeave' may be called from MFC when scrolling! In that case it's
	// not really a "leave".
	//m_bDropDataValid = FALSE;
}
