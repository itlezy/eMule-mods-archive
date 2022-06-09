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

#include "ColorButton.h"

class CPreferences;

class CPPgLists : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgLists)

public:
	CPPgLists();
	virtual ~CPPgLists();
	void SetPrefs(CPreferences* in_prefs) {	m_pPrefs = in_prefs; }

	enum { IDD = IDD_PPG_LISTS };

	virtual BOOL OnInitDialog();
	void Localize(void);
	virtual BOOL OnApply();
	afx_msg void OnSettingsChange()						{ SetModified(); }
	afx_msg LONG OnColorButtonSelChange(UINT lParam, LONG wParam)	{ NOPRM(lParam); NOPRM(wParam); SetModified(); return TRUE; }

protected:
	CPreferences *m_pPrefs;

	BOOL	m_bModified;
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}
	virtual void DoDataExchange(CDataExchange* pDX);
	void LoadSettings(void);
	DECLARE_MESSAGE_MAP()

protected:
	CString	m_strSmartFilterMaxQR;
	BOOL	m_bShowA4AF;
	BOOL	m_bShowA4AFCount;
	BOOL	m_bShowAvgDataRate;
	BOOL	m_bShowFileTypeIcons;
	BOOL	m_bShowTransferredOnCompleted;
	BOOL	m_bShowDownloadPercentage;
	BOOL	m_bShowPausedGray;
	BOOL	m_bShowFileStatusIcons;
	BOOL	m_bShowCountryFlag;
	BOOL	m_bRoundSizes;
	BOOL	m_bDisplayUploadParts;
	BOOL	m_bSmartFilterShowSourcesOQ;
	BOOL	m_bShowRatingIcons;

private:
	CColorButton m_FakeListColorButton;
};
