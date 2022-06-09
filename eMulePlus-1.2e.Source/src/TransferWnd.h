//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#include "UploadListCtrl.h"
#include "DownloadListCtrl.h"
#include "QueueListCtrl.h"
#include "ClientListctrl.h"
#include "InfoListCtrl.h"
#include "RollupCtrl.h"
#include "ResizableLib\ResizableDialog.h"
#include "ToolTips\PPToolTip.h"
#include "TabCtrl.hpp"
#include "Tabs.h"
#include "Loggable.h"

enum EnumExpandType
{
	COLLAPSE_ONLY = 0,
	EXPAND_ONLY = 1,
	EXPAND_COLLAPSE = 2
};

#define USRMSG_CLEARCOMPLETED	WM_USER + 5


class CDummyForTabs;

class CCatTabs : public CTabs
{
	void DrawItem(CDC *pDC, int iItem);
};

enum EnumMiddlePanelWindows
{
	MPW_UPLOADLIST = 0,
	MPW_UPLOADQUEUELIST,
	MPW_UPLOADCLIENTLIST
};

// CTransferWnd dialog

class CTransferWnd : public CResizableDialog, public CLoggable
{
	DECLARE_DYNAMIC(CTransferWnd)

public:
					CTransferWnd(CWnd* pParent = NULL);   // standard constructor
	virtual			~CTransferWnd();
	void			Localize();
// Dialog Data
	enum
	{
		IDD = IDD_TRANSFER
	};

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void UpdateDownloadHeader();
	void UpdateUploadHeader();
	void UpdateInfoHeader();
	void InitRollupItemHeights();
	void SaveRollupItemHeights();
	void SwitchUploadList();
	afx_msg void OnDestroy();	// eklmn: bugfix(00): resource cleanup due to CResizableDialog

	void EditCatTabLabel(int index,CString newlabel);
	void UpdateCatTabTitles();
	void UpdateKnown();
	void UpdateQueueFilter();

	afx_msg void	OnTcnSelchangeDltab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnNMRClickDltab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnLvnBeginDrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnTabMovement(NMHDR *pNMHDR, LRESULT *pResult);
	
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);

	int				AddCategory(CString newtitle, CString newincoming, CString newtemp, CString newcomment, CString newautocat, bool bAddTab=true);
	int				AddPredefinedCategory(EnumCategories ePredefinedCatID, bool bAddTab=true);

protected:
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL	OnInitDialog();
	virtual BOOL	OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL	OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

private:
	int				m_nTabDropIndex;
	bool			m_bIsDragging;
	CImageList	   *m_pDragImage;
	int				m_iTabRightClickIndex;

private:
	void		OnDblClickDltab();
	int			GetTabUnderMouse(CPoint* point);
	CString		GetTabStatistic(byte tab); 
	void		GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY *pNotify);

public:
	CCatTabs				m_ctlDLTabs;
	
	CDummyForTabs			*m_pwndDummyForDownloadList;

	CDownloadListCtrl		m_ctlDownloadList;
	CQueueListCtrl			m_ctlQueueList;
	CClientListCtrl			m_ctlClientList;
	EnumMiddlePanelWindows	m_nActiveWnd;
	
	CUploadListCtrl			m_ctlUploadList;
	CInfoListCtrl			m_ctlInfoList;
	CRollupCtrl				m_ctlRollup;
	CPPToolTip				m_ttip;
};
