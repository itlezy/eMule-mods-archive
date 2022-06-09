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

#include "ScrollStatic.h"
#include "..\types.h"

class CUpDownClient;
class CCDGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(CCDGeneral)
		
public:
	CCDGeneral();
	~CCDGeneral();

// Dialog Data
	enum { IDD = IDD_CDPPG_GENERAL };

	virtual BOOL OnInitDialog();
	void Update();
	void Localize(void);

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CUpDownClient* m_pClient;

private:
	CEdit			m_ctrlUserName;
	CScrollStatic	m_ctrlServerName;
	CScrollStatic	m_ctrlClientSoftware;
	CStatic			m_ctrlClientIcon;

	HICON			m_hClientIcon;
	EnumClientTypes	m_eCurrClientSoft;
};
