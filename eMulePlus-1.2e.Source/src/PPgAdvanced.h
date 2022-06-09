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

class CPPgAdvanced : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgAdvanced)

public:
	CPPgAdvanced();
	virtual ~CPPgAdvanced();
	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

// Dialog Data
	enum { IDD = IDD_PPG_ADVANCED };

	afx_msg void OnSettingsChange()					{SetModified();}
	afx_msg void OnBnClickedComEnabled();
	afx_msg void OnBnClickedSLSEnabled();
	afx_msg void OnSetCleanupFilter();
	afx_msg void OnBnClickedBanEnabled();
	void Localize(void);

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	void LoadSettings(void);

protected:
	CPreferences *m_pPrefs;
	CString	m_strBanTimes;
	CString	m_strMinRequestTime;
	CString	m_strBanTimeInMins;
	CString	m_strComString;
	CString	m_strSlsMaxSources;
	CString	m_strOutdated;
	BOOL	m_bModified;
	BOOL	m_bBanMessage;
	BOOL	m_bBanEnabled;
	BOOL	m_bNoBanEnabled;
	BOOL	m_bComEnabled;
	BOOL	m_bSlsEnabled;
	BOOL	m_bFNameCleanup;
	BOOL	m_bFNameCleanupTag;
	BOOL	m_bCountermeasures;
	BOOL	m_bAutoNewVerChk;
};
