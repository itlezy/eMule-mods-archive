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

class CPreferences;

class CPPgPartTraffic : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgPartTraffic)

public:
	CPPgPartTraffic();
	virtual ~CPPgPartTraffic();

	void SetPrefs(CPreferences* in_prefs) {	m_pPrefs = in_prefs;}
	void Localize(void);

	enum { IDD = IDD_PPG_PARTTRAFFIC };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnBnClickedPtUseit();
	afx_msg void OnSettingsChange()			{ SetModified(); }

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void LoadSettings(void);

protected:
	CPreferences *m_pPrefs;
	int		m_iUploadBarStyle;
	int		m_iUploadBarColor;
	BOOL	m_bModified;
	BOOL	m_bDisplayPartTraffic;
};
