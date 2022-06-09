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
#include "stdafx.h"
#include "emule.h"
#include "MuleStatusBarCtrl.h"
#include "otherfunctions.h"
#include "emuleDlg.h"
#include "ServerWnd.h"
#include "StatisticsDlg.h"
#include "ChatWnd.h"
#include <math.h>
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "server.h"
#include "ServerList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CMuleStatusBarCtrl

IMPLEMENT_DYNAMIC(CMuleStatusBarCtrl, CStatusBarCtrl)
CMuleStatusBarCtrl::CMuleStatusBarCtrl()
{
}

CMuleStatusBarCtrl::~CMuleStatusBarCtrl()
{
}

BEGIN_MESSAGE_MAP(CMuleStatusBarCtrl, CStatusBarCtrl)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


void CMuleStatusBarCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	NOPRM(nFlags);
	switch (GetPaneAtPosition(point))
	{
		case -1:
			return;
		case SB_MESSAGETEXT:
			if (g_App.m_pMDlg->m_wndServer.m_ctrlBoxSwitcher.GetCurSel() != 0)
			{
				g_App.m_pMDlg->m_wndServer.m_ctrlBoxSwitcher.SetCurSel(0);

				LRESULT pDummy;

				g_App.m_pMDlg->m_wndServer.OnTcnSelchangeTab1(NULL, &pDummy);
			}
		case SB_SERVER:
			g_App.m_pMDlg->SetActiveDialog(&g_App.m_pMDlg->m_wndServer);
			break;
		case SB_SESSIONTIME:
			g_App.m_pMDlg->SetActiveDialog(&g_App.m_pMDlg->m_dlgStatistics);
			break;
		case SB_MESSAGESTATUS:
			g_App.m_pMDlg->SetActiveDialog(&g_App.m_pMDlg->m_wndChat);
			break;
		case SB_UPLOADRATE:
		case SB_DOWNLOADRATE:
			g_App.m_pMDlg->SetActiveDialog(&g_App.m_pMDlg->m_wndTransfer);
			break;
	}
}

int CMuleStatusBarCtrl::GetPaneAtPosition(CPoint& point)
{
	CRect rect;
	int iParts = GetParts(0, NULL);

	for (int i = 0; i<iParts; i++)
	{
		GetRect(i, rect);
		if (rect.PtInRect(point))
			return i;
	}
	return -1;
}
