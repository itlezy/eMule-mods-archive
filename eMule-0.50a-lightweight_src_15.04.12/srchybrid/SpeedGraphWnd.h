//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "neo/modeless.h"

class CSpeedGraph
{
public:
	CSpeedGraph(); 
	virtual ~CSpeedGraph();
	void Init(uint_ptr trafficEntries);
	void InitArray();
	void Set_TrafficValue(uint32 value);
	uint32	m_ui_MaxAmount;
	uint32* TrafficStats;
private:
	uint_ptr	TrafficEntries;
};

// CSpeedGraphWnd dialog
class CMainFrameDropTarget;

class CSpeedGraphWnd : public CModDialog
{
	//DECLARE_DYNCREATE(CSpeedGraphWnd)

public:
	CSpeedGraphWnd();   // standard constructor
	virtual ~CSpeedGraphWnd();
	enum { IDD = IDD_SPEEDGRAPH };
	// X-Ray :: Speedgraph :: Start 
	void Update_TrafficGraph();
	void SetUSpeedMeterRange(uint32 iValue){m_co_UpTrafficGraph.m_ui_MaxAmount = iValue*1024;}
	void SetDSpeedMeterRange(uint32 iValue){m_co_DownTrafficGraph.m_ui_MaxAmount = iValue*1024;}
	// X-Ray :: Speedgraph :: End

protected:	
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	void DrawWindow();
	CSpeedGraph m_co_UpTrafficGraph;
	CSpeedGraph m_co_DownTrafficGraph;
	CMainFrameDropTarget* m_pDropTarget;
	int	lastvalidi;
	int	lastvalido;
	//COLORREF ColorText;
};
