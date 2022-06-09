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

class CPPgServer : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgServer)

public:
	CPPgServer();
	virtual ~CPPgServer();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }

	// Dialog Data
	enum { IDD = IDD_PPG_SERVER };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnBnClickedRemovedead();
	afx_msg void OnSetURLsForICC();
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
	CString	m_strSrvConnectTimeout;
	CString	m_strSrvKeepAliveTimeout;
	CString	m_strServerRetries;
	BOOL	m_bModified;
	BOOL	m_bRemoveDead;
	BOOL	m_bAutoServer;
	BOOL	m_bSmartId;
	BOOL	m_bAddSrvFromServer;
	BOOL	m_bAddSrvFromClients;
	BOOL	m_bAutoConnect;
	BOOL	m_bAutoConnectStatic;
	BOOL	m_bReconnect;
	BOOL	m_bScoreSystem;
	BOOL	m_bManualSrvHighPriority;
	BOOL	m_bRestartWaiting;
	BOOL	m_bUseAuxPort;
};
