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

#include "..\resource.h"
#include "..\MuleListCtrl.h"

enum EnumFDSrcNameListColumns
{
	FDSRCCOL_FILENAME = 0,
	FDSRCCOL_SOURCES,

	FDSRCCOL_NUMCOLUMNS
};

struct FCtrlItem_Struct
{
	CString	strFileName;
	uint32	iCount;
};

class CPartFile;

class CFDSrcNames : public CPropertyPage
{
	DECLARE_DYNCREATE(CFDSrcNames)

public:
	CFDSrcNames();
	~CFDSrcNames();

	void Localize();
	void Update();

	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRename();
	afx_msg void OnBnClickedCleanup();
	afx_msg void OnBnClickedFdsTakeover();
	afx_msg void OnDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	CPartFile* m_pFile;

	enum { IDD = IDD_FDPPG_SOURCENAMES };
	CButton m_ctrlTakeOver;
	CButton m_ctrlCleanUp;
	CButton m_ctrlRename;
	CEdit m_ctrlFilename;

protected:
	DECLARE_MESSAGE_MAP()

	static int CALLBACK CompareListNameItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	CMuleListCtrl	pmyListCtrl;

private:
	bool			m_bSortAscending[FDSRCCOL_NUMCOLUMNS];

	void FillSourcenameList();
};
