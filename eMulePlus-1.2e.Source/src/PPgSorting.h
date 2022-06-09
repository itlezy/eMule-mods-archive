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

#include "ArrowCombo.h"

class CPreferences;

class CPPgSorting : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgSorting)

public:
	CPPgSorting();
	virtual ~CPPgSorting();
	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }

	enum { IDD = IDD_PPG_SORTING };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	void Localize(void);
	afx_msg void OnBnClickedUseSort();
	afx_msg void OnBnClickedSortSources2Box();
	afx_msg void OnBnClickedPausedStoppedLast()		{ SetModified(); }
	afx_msg void OnCbnSelchangeSortServers()		{ SetModified(); }
	afx_msg void OnCbnSelchangeSortFiles()			{ SetModified(); }
	afx_msg void OnCbnSelchangeSortDownloads()		{ SetModified(); }
	afx_msg void OnCbnSelchangeSortSources1()		{ SetModified(); }
	afx_msg void OnCbnSelchangeSortSources2()		{ SetModified(); }
	afx_msg void OnCbnSelchangeSortUploads()		{ SetModified(); }
	afx_msg void OnCbnSelchangeSortQueue()			{ SetModified(); }
	afx_msg void OnCbnSelchangeSortSearch()			{ SetModified(); }
	afx_msg void OnCbnSelchangeSortIrc()			{ SetModified(); }
	afx_msg void OnCbnSelchangeSortClientList()		{ SetModified(); }

protected:
	CPreferences *m_pPrefs;
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
	void LoadCombo(void);

protected:
	BOOL m_bUseSorting;
	BOOL m_bUseSourcesSorting;
	BOOL m_bPausedStoppedLast;

private:
	CArrowCombo m_ServerColsCombo;
	CArrowCombo m_FileColsCombo;
	CArrowCombo m_DownloadColsCombo;
	CArrowCombo m_UploadColsCombo;
	CArrowCombo m_QueueColsCombo;
	CArrowCombo m_SearchColsCombo;
	CArrowCombo m_IrcColsCombo;
	CArrowCombo m_ClientListColsCombo;
	CArrowCombo m_SourceCols1Combo;
	CArrowCombo m_SourceCols2Combo;
};
