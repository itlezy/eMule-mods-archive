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

class CPPgAutoDL : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgAutoDL)

public:
	CPPgAutoDL();
	virtual ~CPPgAutoDL();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs;}
	void Localize(void);

// Dialog Data
	enum { IDD = IDD_PPG_AUTODL };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnSettingsChange()			{ SetModified(); }
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndlabeleditList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedSet();
	afx_msg void OnBnClickedNew();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedAutoDL();

protected:
	BOOL	m_bModified;
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
	CPreferences	*m_pPrefs;
	CListCtrl		m_ctrlList;
	long			m_lInterval;
	BOOL			m_bAutoDLEnabled;
};
