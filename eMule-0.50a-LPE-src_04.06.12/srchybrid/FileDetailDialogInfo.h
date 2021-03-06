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
#pragma once
#include "ResizableLib/ResizablePage.h"

class CFileDetailDialogInfo : public CResizablePage
{
	DECLARE_DYNAMIC(CFileDetailDialogInfo)

public:
	CFileDetailDialogInfo();   // standard constructor
	virtual ~CFileDetailDialogInfo();

	void SetFiles(const CSimpleArray<void*>* paFiles) { m_paFiles = paFiles; m_bDataChanged = true; }

	// Dialog Data
	enum { IDD = IDD_FILEDETAILS_INFO };

protected:
	CString m_strCaption;
	const CSimpleArray<void*>* m_paFiles;
	bool m_bDataChanged;
	UINT_PTR m_timer;
	static LPCTSTR sm_pszNotAvail;

	void Localize();
	void RefreshData();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};
