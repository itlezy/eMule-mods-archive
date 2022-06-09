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
#include "..\emule.h"
#include "..\resource.h"
#include "filedetails.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CFileDetails, CPropertySheet)

CFileDetails::CFileDetails(UINT nIDCaption, CPartFile* pFile, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(GetResString(nIDCaption), pParentWnd, iSelectPage)
{
	Init(pFile);
}

CFileDetails::CFileDetails(LPCTSTR pszCaption, CPartFile* pFile, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	Init(pFile);
}

CFileDetails::~CFileDetails()
{
}

void CFileDetails::Init(CPartFile* pFile)
{
	EMULE_TRY

	m_nTimer = 0;

	m_wndGeneral.m_pFile = pFile;
	m_wndTransfer.m_pFile = pFile;
	m_wndSourceNames.m_pFile = pFile;
	m_wndAviInfo.m_pFile = pFile;
	m_wndParts.m_pFile = pFile;

	m_psh.dwFlags = (m_psh.dwFlags | PSH_NOAPPLYNOW) & ~PSH_HASHELP;

	GetResString(&m_strGeneralTitle, IDS_PW_GENERAL);
	m_wndGeneral.m_psp.dwFlags = (m_wndGeneral.m_psp.dwFlags | PSP_USETITLE) & ~PSP_HASHELP;
	m_wndGeneral.m_pPSP->pszTitle = m_strGeneralTitle;

	GetResString(&m_strTransferTitle, IDS_TRANSFER_NOUN);
	m_wndTransfer.m_psp.dwFlags = (m_wndTransfer.m_psp.dwFlags | PSP_USETITLE) & ~PSP_HASHELP;
	m_wndTransfer.m_pPSP->pszTitle = m_strTransferTitle;

	GetResString(&m_strSourceNamesTitle, IDS_INFLST_FILE_SOURCENAMES);
	m_wndSourceNames.m_psp.dwFlags = (m_wndSourceNames.m_psp.dwFlags | PSP_USETITLE) & ~PSP_HASHELP;
	m_wndSourceNames.m_pPSP->pszTitle = m_strSourceNamesTitle;

	GetResString(&m_strPartsTitle, IDS_UP_PARTS);
	m_wndParts.m_psp.dwFlags = (m_wndParts.m_psp.dwFlags | PSP_USETITLE) & ~PSP_HASHELP;
	m_wndParts.m_pPSP->pszTitle = m_strPartsTitle;

	AddPage(&m_wndGeneral);
	AddPage(&m_wndTransfer);
	AddPage(&m_wndSourceNames);

	if (pFile != NULL && pFile->IsAviMovie())
	{
		GetResString(&m_strAviInfoTitle, IDS_FD_AVIINFO);
		m_wndAviInfo.m_psp.dwFlags = (m_wndAviInfo.m_psp.dwFlags | PSP_USETITLE) & ~PSP_HASHELP;
		m_wndAviInfo.m_pPSP->pszTitle = m_strAviInfoTitle;

		AddPage(&m_wndAviInfo);
	}

	AddPage(&m_wndParts);

	EMULE_CATCH
}

BEGIN_MESSAGE_MAP(CFileDetails, CPropertySheet)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileDetails message handlers

BOOL CFileDetails::OnInitDialog()
{
	CPropertySheet::OnInitDialog();
	
	m_nTimer = SetTimer(301, 15000, NULL);

	return true;
}

void CFileDetails::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == m_nTimer)
	{
		m_wndGeneral.Update();
		m_wndTransfer.Update();
		m_wndSourceNames.Update();
		m_wndParts.Update();
	}

	CPropertySheet::OnTimer(nIDEvent);
}

void CFileDetails::OnDestroy()
{
	if (m_nTimer != 0)
	{
		KillTimer(m_nTimer);
	}

	CPropertySheet::OnDestroy();
}
