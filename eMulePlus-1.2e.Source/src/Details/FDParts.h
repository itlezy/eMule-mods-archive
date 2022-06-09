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

#include "..\MuleListCtrl.h"

class CPartFile;

enum EnumFDPartsStatus
{
	FDPARTS_STAT_DOWNLOADING = 0,
	FDPARTS_STAT_RECOVERING,
	FDPARTS_STAT_CORRUPTED,
	FDPARTS_STAT_NONE,
	FDPARTS_STAT_NOTAVAILABLE,
	FDPARTS_STAT_COMPLETE,

	FDPARTS_STAT_COUNT
};

typedef struct
{
	unsigned	uiCompleteSz;
	unsigned	uiSrcNum1;
	unsigned	uiSrcNum2;
	unsigned	uiStatus;
} PartInfoType;

class CFDParts : public CPropertyPage
{
	DECLARE_DYNCREATE(CFDParts)

// Construction
public:
	CFDParts();
	~CFDParts();

	void Update();

	CPartFile* m_pFile;

	enum { IDD = IDD_FDPPG_PARTS };

	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFilterChange();
	afx_msg void OnDestroy();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

private:
	CMuleListCtrl	m_PartsListCtrl;
	bool			m_bSortAscending[FDPARTSCOL_NUMCOLUMNS + 1];
	static std::vector<PartInfoType>	s_aPartInfo;
	CBitmap			m_bmpDownloadStatus;
	CBitmap			m_bmpUploadStatus;
	CBitmap			m_bmpCompletionStatus;

	void Localize();
	void SortInit(int iSortCode);
};
