//this file is part of eMule
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

// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->

#pragma once

enum structTypes { IOM_VST_FILE, IOM_VST_DIR, IOM_VST_SUBDIR, IOM_VST_NEW };

struct VirtMapStruct {
	CString mapFrom;
	CString mapTo;
	CString fileID;
	structTypes type;
};

class CPPgVirtual : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgVirtual)

public:
	CPPgVirtual();
	~CPPgVirtual();

// Dialog Data
	enum { IDD = IDD_PPG_VIRTUAL };

	void Localize(void);

protected:
	CComboBox m_typesel;
	CListCtrl m_list;
	CList<VirtMapStruct *> structList;

	void FillList();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMClkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnNMRightClkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSettingsChange() {SetModified();}
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
};

// NEO: VSF END <-- Xanatos --