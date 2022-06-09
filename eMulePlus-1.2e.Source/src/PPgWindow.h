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

#include "FontPreviewCombo.h"

class CPreferences;

class CPPgWindow : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgWindow)

public:
	CPPgWindow();
	virtual ~CPPgWindow();
	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	void Localize(void);

// Dialog Data
	enum { IDD = IDD_PPG_WINDOW };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnSettingsChange()						{ SetModified(); }

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}
	virtual void DoDataExchange(CDataExchange* pDX);
	void LoadSettings(void);
	DECLARE_MESSAGE_MAP()

protected:
	CPreferences *m_pPrefs;
	BOOL	m_bModified;
	BOOL	m_bStartMin;
	BOOL	m_bMinTray;
	BOOL	m_bCloseToTray;
	BOOL	m_bPromptOnExit;
	BOOL	m_bPromptOnDisconnect;
	BOOL	m_bPromptOnFriendDel;
	BOOL	m_bBringToForeground;
	BOOL	m_bShowRateOnTitle;
	BOOL	m_bShowSpeedMeterOnToolbar;
	BOOL	m_bKeepSearchHistory;

private:
	CFontPreviewCombo	fontPreviewCombo;
	CComboBox			fontSizeCombo;
};
