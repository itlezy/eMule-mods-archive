//this file is part of eMule
//Copyright (C)2003-2008 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#pragma once

class CPPgAcK : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgAcK)

public:
	CPPgAcK();
	virtual ~CPPgAcK();
	void Localize(void);
	// Dialog Data
	enum { IDD = IDD_PPG_AcK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    BOOL OnApply(void);
	void LoadSettings(void);
	BOOL OnInitDialog(void);
	int	m_iFilter; //>>> taz::ASF

DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ SetModified(); }
//>>> taz::ASF
	afx_msg void OnBnClickedASF_ON();
	afx_msg void OnBnClickedSERVERANALYZER_ON();
//<<< taz::ASF
//>>> taz::AcK filters [Aenarion/Xanatos]
	afx_msg void OnClickButton();
public:
	afx_msg void OnBnClickedChangePassff();
//<<< taz::AcK filters [Aenarion/Xanatos]
};
