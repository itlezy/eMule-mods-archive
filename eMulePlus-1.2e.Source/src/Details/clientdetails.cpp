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
//
// FileDetails.cpp : implementation file

#include "stdafx.h"
#include "clientdetails.h"
#include "..\otherfunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientDetails

IMPLEMENT_DYNAMIC(CClientDetails, CPropertySheet)

CClientDetails::CClientDetails(UINT uiIDCaption, CUpDownClient* pClient, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(GetResString(uiIDCaption), pParentWnd, iSelectPage)
{
	Init(pClient);
}

CClientDetails::CClientDetails(LPCTSTR pszCaption, CUpDownClient* pClient, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	Init(pClient);
}

CClientDetails::~CClientDetails()
{
}


BEGIN_MESSAGE_MAP(CClientDetails, CPropertySheet)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClientDetails message handlers

void CClientDetails::Init(CUpDownClient* pClient)
{
	m_pClient = pClient;
	m_nTimer = 0;

	m_proppageGeneral.m_pClient = m_pClient;
	m_wndTransfer.m_pClient = m_pClient;
	m_wndScores.m_pClient = m_pClient;

	m_psh.dwFlags = (m_psh.dwFlags | PSH_NOAPPLYNOW) & ~PSH_HASHELP;

	GetResString(&m_strGeneralTitle, IDS_PW_GENERAL);
	m_proppageGeneral.m_psp.dwFlags = (m_proppageGeneral.m_psp.dwFlags | PSP_USETITLE) & ~PSP_HASHELP;
	m_proppageGeneral.m_pPSP->pszTitle = m_strGeneralTitle;

	GetResString(&m_strTransferTitle, IDS_TRANSFER_NOUN);
	m_wndTransfer.m_psp.dwFlags = (m_wndTransfer.m_psp.dwFlags | PSP_USETITLE) & ~PSP_HASHELP;
	m_wndTransfer.m_pPSP->pszTitle = m_strTransferTitle;

	GetResString(&m_strScoresTitle, IDS_CD_SCORES);
	m_wndScores.m_psp.dwFlags = (m_wndScores.m_psp.dwFlags | PSP_USETITLE) & ~PSP_HASHELP;
	m_wndScores.m_pPSP->pszTitle = m_strScoresTitle;

	AddPage(&m_proppageGeneral);
	AddPage(&m_wndTransfer);
	AddPage(&m_wndScores);
}

BOOL CClientDetails::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	m_nTimer = SetTimer(301, 10000, NULL);

	return bResult;
}

void CClientDetails::OnTimer(UINT nIDEvent)
{
	if(nIDEvent == m_nTimer)
	{
		m_proppageGeneral.Update();
		m_wndTransfer.Update();
		m_wndScores.Update();
	}

	CPropertySheet::OnTimer(nIDEvent);
}
