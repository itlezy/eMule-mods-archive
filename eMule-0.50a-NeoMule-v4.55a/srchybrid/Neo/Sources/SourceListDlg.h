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

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->

#include "MuleListCtrl.h"
#include "ListCtrlItemWalk.h"
#include "Neo/Gui/ModeLess.h" // NEO: MLD - [ModelesDialogs] 

class CSourceDetailDialog;

class CSourceListCtrl : public CMuleListCtrl, public CListCtrlItemWalk
{
	DECLARE_DYNAMIC(CSourceListCtrl)
public:
	CSourceListCtrl();
	virtual ~CSourceListCtrl();
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

class CSourceListDlg : public CModResizableDialog // NEO: MLD - [ModelesDialogs] 
{
	DECLARE_DYNAMIC(CSourceListDlg)

public:
	CSourceListDlg();
	virtual ~CSourceListDlg();

// Dialog Data
	enum { IDD = IDD_SOURCELIST };

protected:
	CSourceListCtrl m_SourceList;
	CSourceDetailDialog* m_SourceDetailDialog; // NEO: MLD - [ModelesDialogs] 

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();

private:
	void	ReloadSourceList();
};

#endif // NEO_CD // NEO: NCD END <-- Xanatos --