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

class CPPgNotify : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgNotify)

public:
	CPPgNotify();
	virtual ~CPPgNotify();
	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	void Localize(void);

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

// Dialog Data
	enum { IDD = IDD_PPG_NOTIFY };

	afx_msg void OnBnClickedCbTbnUsesound();
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnBnClickedCbTbnOnchat();
	afx_msg void OnBnClickedBtnBrowseWav();

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()

private:
	void LoadSettings(void);

protected:
	CPreferences *m_pPrefs;
	BOOL	m_bModified;
	BOOL	m_bUseSound;
	BOOL	m_bOnLog;
	BOOL	m_bOnChat;
	BOOL	m_bOnChatMessage;
	BOOL	m_bOnDownloadAdded;
	BOOL	m_bOnDownloadFinished;
	BOOL	m_bUseScheduler;
	BOOL	m_bOnWebServer;
	BOOL	m_bOnImportant;
	BOOL	m_bOnServerError;
	CString	m_strWavFileName;

private:
	CComboBox m_DtimeCombo;
	CComboBox m_FsizeCombo;
	CEdit m_WavFileNameEdit;
	CButton m_WavFileNameButton;
};
