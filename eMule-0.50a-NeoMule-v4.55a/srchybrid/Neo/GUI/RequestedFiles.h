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

#include "ResizableLib/ResizablePage.h"
#include "RequestedFilesCtrl.h"

// NEO: RFL - [RequestFileList] -- Xanatos -->

///////////////////////////////////////////////////////////////////////////////
// CReqFilesPage

class CReqFilesPage : public CResizablePage
{ 
	DECLARE_DYNAMIC(CReqFilesPage) 

public: 
	CReqFilesPage(); 
	virtual ~CReqFilesPage(); 

	void SetClients(const CSimpleArray<CObject*>* paClients) { m_paClients = paClients; m_bDataChanged = true;}

// Dialog Data 
	enum { IDD = IDD_REQFILES }; 

protected: 
	CString m_strCaption;
	CRequestedFilesCtrl m_lstReqFiles;
	CRequestedFilesCtrl m_lstReqFilesUp;
	const CSimpleArray<CObject*>* m_paClients;
	bool m_bDataChanged;
	uint32 m_timer;

	void Localize(); 
	void RefreshData(bool deleteOld = true);

	virtual BOOL OnInitDialog(); 
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support 
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP() 
	afx_msg void OnBnClickedApply(); 
	//afx_msg void OnBnClickedRefresh(); 
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};

// NEO: RFL END <-- Xanatos --