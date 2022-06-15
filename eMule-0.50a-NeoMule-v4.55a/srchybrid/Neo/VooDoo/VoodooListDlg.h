//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->

#include "VoodooListCtrl.h"
#include "Neo/Gui/ModeLess.h" // NEO: MLD - [ModelesDialogs] 

struct SVoodooClient;
class CVoodooDetailDialog; // NEO: MLD - [ModelesDialogs] 


class CVoodooListDlg : public CModResizableDialog // NEO: MLD - [ModelesDialogs] 
{
	DECLARE_DYNAMIC(CVoodooListDlg)

public:
	CVoodooListDlg();
	virtual ~CVoodooListDlg();

// Dialog Data
	enum { IDD = IDD_VOODOOLIST };

protected:
	CVoodooListCtrl m_VoodooList;
	CVoodooDetailDialog* m_VoodooDetailDialog; // NEO: MLD - [ModelesDialogs] 
	uint32 m_timer;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClick(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnBnClickedConnectTo();
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedDisconnect();
	afx_msg void OnBnClickedSearch();
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedEdit();
	afx_msg void OnBnClickedRemove();

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);

private:
	void	ReloadVoodooList();
	void	LoadVoodooItem(SVoodooClient* client);
};

#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
