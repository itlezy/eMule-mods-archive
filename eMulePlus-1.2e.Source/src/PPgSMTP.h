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

class CPPgSMTP : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgSMTP)

public:
	CPPgSMTP();
	virtual ~CPPgSMTP();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	enum { IDD = IDD_PPG_SMTP };
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	void Localize(void);
	afx_msg void OnDataChange()		{ SetModified(); }
	afx_msg void OnEnChangeSMTPAuthenticated();

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
	BOOL	m_bModified;
	CString	m_strSMTPServer;
	CString	m_strSMTPName;
	CString	m_strSMTPFrom;
	CString	m_strSMTPTo;
	CString	m_strSMTPUserName;
	CString	m_strSMTPPassword;
	BOOL	m_bSMTPAuthenticated;
	BOOL	m_bSMTPInfoEnabled;
	BOOL	m_bSMTPWarningEnabled;
	BOOL	m_bSMTPMsgInSubjEnabled;
};
