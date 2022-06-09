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

class CPPgSources : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgSources)

public:
	CPPgSources();
	virtual ~CPPgSources();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }

// Dialog Data
	enum { IDD = IDD_PPG_SOURCES };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnSettingsChange()				{ SetModified(); }
	afx_msg void OnSourcesChange();
	afx_msg void OnBnClickedAutoSources();
	afx_msg void OnBnClickedDisableXS();
	afx_msg void OnBnClickedDisableXSUpTo();
	void Localize(void);

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
	BOOL	m_bModified;
	BOOL	m_bDisableXS;
	BOOL	m_bEnableXSUpTo;
	BOOL	m_bAutoSrcEnabled;
	CString	m_strXsUpTo;
	CString	m_strMaxSrcPerFile;
	uint32	m_dwMinAutoSrcPerFile;
	uint32	m_dwMaxAutoSrcPerFile;
	uint32	m_dwMaxAutoSrcTotal;
	uint32	m_dwMaxAutoXchgSources;
};
