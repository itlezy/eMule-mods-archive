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

class CPPgNotify : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgNotify)

public:
	CPPgNotify();
	virtual ~CPPgNotify();	

	void Localize(void);

// Dialog Data
	enum { IDD = IDD_PPG_NOTIFY };

protected:
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified

	void UpdateControls();
	void ApplyNotifierSoundType();

	//virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange() { m_bModified = true;	SetModified(); } // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnBnClickedChangeNotification(); // X: [CI] - [Code Improvement]
	afx_msg void OnBnClickedOnChat();
	afx_msg void OnBnClickedBrowseAudioFile();
	afx_msg void OnBnClickedTestNotification();
};
