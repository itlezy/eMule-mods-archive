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
#pragma once

#include "..\resource.h"
#include "CDGeneral.h"
#include "CDTransfer.h"
#include "CDScores.h"

class CClientDetails : public CPropertySheet
{
	DECLARE_DYNAMIC(CClientDetails)

public:
	CClientDetails(UINT uiIDCaption, CUpDownClient *pClient, CWnd *pParentWnd = NULL, UINT iSelectPage = 0);
	CClientDetails(LPCTSTR pszCaption, CUpDownClient* pClient, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CClientDetails();

	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);

protected:
	void Init(CUpDownClient* pClient);
	DECLARE_MESSAGE_MAP()

protected:
	CCDGeneral	m_proppageGeneral;
	CCDTransfer	m_wndTransfer;
	CCDScores	m_wndScores;

	CUpDownClient*	m_pClient;
	UINT		m_nTimer;

	CString		m_strGeneralTitle;
	CString		m_strTransferTitle;
	CString		m_strScoresTitle;
};
