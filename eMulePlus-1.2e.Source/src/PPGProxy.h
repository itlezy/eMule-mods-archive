//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

class CPreferences;

class CPPgProxy : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgProxy)

public:
	CPPgProxy();
	virtual ~CPPgProxy();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	void SetPrefs(CPreferences *pPrefs)		{ m_pPrefs = pPrefs; }
	void Localize(void);

	// Dialog Data
	enum { IDD = IDD_PPG_PROXY };

	afx_msg void OnBnClickedEnableproxy();
	afx_msg void OnBnClickedEnableauth();
	afx_msg void OnCbnSelchangeProxytype();
	afx_msg void OnSettingsChange()			{ SetModified(); }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void LoadSettings();

	CPreferences	*m_pPrefs;
	ProxySettings	m_proxy;

	CComboBox	m_ProxyTypeCombo;

protected:
	CString		m_strProxyPort;
	CString		m_strProxyUser;
	CString		m_strProxyPassword;
	CString		m_strProxyName;
	BOOL		m_bEnableProxy;
	BOOL		m_bEnableAuth;
};
