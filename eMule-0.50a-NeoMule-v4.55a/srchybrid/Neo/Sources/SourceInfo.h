//this file is part of eMule
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

#include "Neo/Gui/ModeLess.h" // NEO: MLD - [ModelesDialogs]  
#include "ResizableLib/ResizablePage.h"
#include "MuleListCtrl.h"

class CKnownSource;
class CUpDownClient;
struct IPTableStruct;
struct SeenFileStruct;
///////////////////////////////////////////////////////////////////////////////
// CSourceDetailPage

class CSourceDetailPage : public CResizablePage
{
	DECLARE_DYNAMIC(CSourceDetailPage)

public:
	CSourceDetailPage();   // standard constructor
	virtual ~CSourceDetailPage();

	void SetClients(const CSimpleArray<CObject*>* paClients) { m_paClients = paClients; m_bDataChanged = true; }

	enum { IDD = IDD_SOURCEINFOWND };

protected:
	const CSimpleArray<CObject*>* m_paClients;
	bool m_bDataChanged;
	CMuleListCtrl m_IPTablesList;
	CMuleListCtrl m_SeenFilesList; // NEO: SFL - [SourceFileList]

	void Localize();
	void RefreshData();

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnBnClickedAnalyze(); 
	afx_msg void OnDestroy();

private:
	CKnownSource*	Source;
	CUpDownClient*	Client;

	void	UpdateAnalisis();

	void	AddIPtable(IPTableStruct* IPTable);
	void	AddSeenFile(SeenFileStruct* SeenFile); // NEO: SFL - [SourceFileList]
};

///////////////////////////////////////////////////////////////////////////////
// CSourceDetailDialog

class CSourceDetailDialog : public  CModListViewWalkerPropertySheet // NEO: MLD - [ModelesDialogs] 
{
	DECLARE_DYNAMIC(CSourceDetailDialog)

public:
	CSourceDetailDialog(CKnownSource* pSource,CListCtrlItemWalk* pListCtrl);
	virtual ~CSourceDetailDialog();

protected:
	CSourceDetailPage m_wndSource;

	void Construct();

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
};
#endif // NEO_CD // NEO: NCD END <-- Xanatos --