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

#pragma once

class CPPgMessages : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgMessages)

public:
	CPPgMessages();
	virtual ~CPPgMessages();

// Dialog Data
	enum { IDD = IDD_PPG_MESSAGES };

	void Localize(void);

protected:
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified

	void LoadSettings(void);

	//virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ m_bModified = true;	SetModified(); } // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnSpamFilterChange();
	afx_msg void OnDestroy();
};