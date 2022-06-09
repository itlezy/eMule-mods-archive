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

class CPPgBackup : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgBackup)

public:
	CPPgBackup();
	virtual ~CPPgBackup();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs;}

	// Dialog Data
	enum { IDD = IDD_PPG_BACKUP };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	void Localize(void);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBackupnow();
	afx_msg void OnBnClickedCommon();
	afx_msg void OnBnClickedPart();
	afx_msg void OnBnClickedSelectall();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedScheduledBackupCheck();
	afx_msg void OnSettingsChange()						{ SetModified(); }

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	void LoadSettings();
	void CheckBackupNowButton();
	BOOL SelectDir(const TCHAR *pcInDir, TCHAR *outdir, const CString &titletext);

protected:
	CPreferences *m_pPrefs;
	BOOL	m_bModified;
	BOOL	m_bDatFiles;
	BOOL	m_bMetFiles;
	BOOL	m_bIniFiles;
	BOOL	m_bPartFiles;
	BOOL	m_bPartMetFiles;
	BOOL	m_bTxtsrcFiles;
	BOOL	m_bAutoBackup;
	BOOL	m_bOverwriteFiles;
	BOOL	m_bScheduledBackup;
	CString	m_strBackupDir;
	CString	m_strScheduledBackupInterval;
};
