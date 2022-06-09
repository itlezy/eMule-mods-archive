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
// FileDetails.h : header file
#pragma once

#include "FDGeneral.h"
#include "FDTransfer.h"
#include "FDSrcNames.h"
#include "FDParts.h"
#include "FDAviInfo.h"

class CPartFile;

/////////////////////////////////////////////////////////////////////////////
// CFileDetails
class CFileDetails : public CPropertySheet
{
	DECLARE_DYNAMIC(CFileDetails)

public:
	CFileDetails(UINT nIDCaption, CPartFile* pFile, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CFileDetails(LPCTSTR pszCaption, CPartFile* pFile, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CFileDetails();
	virtual BOOL OnInitDialog();

	void Init(CPartFile* pFile);

protected:
	CFDGeneral	m_wndGeneral;
	CFDTransfer	m_wndTransfer;
	CFDSrcNames	m_wndSourceNames;
	CFDAviInfo	m_wndAviInfo;
	CFDParts	m_wndParts;

	UINT		m_nTimer;

	CString		m_strGeneralTitle;
	CString		m_strTransferTitle;
	CString		m_strSourceNamesTitle;
	CString		m_strAviInfoTitle;
	CString		m_strPartsTitle;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
};
