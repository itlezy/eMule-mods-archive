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

class CPPgHTTPD : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgHTTPD)

public:
	CPPgHTTPD();
	virtual ~CPPgHTTPD();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	enum { IDD = IDD_PPG_HTTPD };
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	void Localize(void);
	afx_msg void OnDataChange()		{SetModified();}
	afx_msg void OnEnChangeHTTPDEnabled();
	afx_msg void OnReloadTemplates();
	afx_msg void OnBnClickedTmplbrowse();
	afx_msg void OnEnChangeMMEnabled();
	afx_msg void OnEnChangeHTTPDGuestEnabled();
	afx_msg void OnEnChangeIntruderDetection();

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
	CString	m_strWSPort;
	CString	m_strWSPasswd;
	CString	m_strWSGuestPasswd;
	CString	m_strTmplPath;
	CString	m_strMMPort;
	CString	m_strMMPasswd;
	CString	m_strTempDisableLogin;
	CString	m_strLoginAttemptsAllowed;
	BOOL	m_bModified;
	BOOL	m_bWSEnabled;
	BOOL	m_bWSGuestEnabled;
	BOOL	m_bWSIntruderDetectEnabled;
	BOOL	m_bMMEnabled;
};
