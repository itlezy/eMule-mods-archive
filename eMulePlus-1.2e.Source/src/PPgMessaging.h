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

class CPPgMessaging : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgMessaging)

public:
	CPPgMessaging();
	virtual ~CPPgMessaging();
	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	void Localize(void);

	enum { IDD = IDD_PPG_MESSAGING };

	afx_msg void OnSettingsChange()		{ SetModified(); }
	afx_msg void OnBnClickedCbImPutmeaway();

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

private:
	void LoadSettings(void);

	DECLARE_MESSAGE_MAP()

protected:
	CPreferences	*m_pPrefs;
	int		m_iAcceptMessages;
	BOOL	m_bModified;
	BOOL	m_bPutMeInAwayState;
	CString	m_strAwayMessage;
	CString	m_strMsgFilter;
	CString	m_strCommentFilter;
};
