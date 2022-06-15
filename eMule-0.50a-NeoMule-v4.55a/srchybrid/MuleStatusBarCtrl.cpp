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
#include "stdafx.h"
#include "emule.h"
#include "MuleStatusBarCtrl.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "ServerWnd.h"
#include "StatisticsDlg.h"
#include "ChatWnd.h"
#include "Sockets.h"
#include "Server.h"
#include "ServerList.h"
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "math.h"
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CMuleStatusBarCtrl

IMPLEMENT_DYNAMIC(CMuleStatusBarCtrl, CStatusBarCtrl)

BEGIN_MESSAGE_MAP(CMuleStatusBarCtrl, CStatusBarCtrl)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

CMuleStatusBarCtrl::CMuleStatusBarCtrl()
{
}

CMuleStatusBarCtrl::~CMuleStatusBarCtrl()
{
}

void CMuleStatusBarCtrl::Init(void)
{
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	EnableToolTips();
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
}

void CMuleStatusBarCtrl::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	int iPane = GetPaneAtPosition(point);
	switch (iPane)
	{
		case SBarLog:
			AfxMessageBox(_T("eMule ") + GetResString(IDS_SV_LOG) + _T("\n\n") + GetText(SBarLog));
			break;

		case SBarUsers:
			theApp.emuledlg->serverwnd->ShowNetworkInfo();
			break;
		
		case SBarUpDown:
			theApp.emuledlg->SetActiveDialog(theApp.emuledlg->statisticswnd);
			break;
		
		case SBarConnected:
			theApp.emuledlg->serverwnd->ShowNetworkInfo();
			break;

		case SBarChatMsg:
			theApp.emuledlg->SetActiveDialog(theApp.emuledlg->chatwnd);
			break;
	}
}

int CMuleStatusBarCtrl::GetPaneAtPosition(CPoint& point) const
{
	CRect rect;
	int nParts = GetParts(0, NULL);
	for (int i = 0; i < nParts; i++)
	{
		GetRect(i, rect);
		if (rect.PtInRect(point))
			return i;
	}
	return -1;
}

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
HICON CMuleStatusBarCtrl::GetTipInfo(CString &info)
{
	CPoint pt;
	::GetCursorPos(&pt);
	ScreenToClient(&pt);
	info.Empty();
	switch (GetPaneAtPosition(pt))
	{
		case SB_LOG:
			info = GetText(SB_LOG);
			if(info.GetLength() > 100)
			{
				int m = (int)ceil((float)info.GetLength() / 100);
				for(int i = 0; i < m; i++)
				{
					int n = (info.GetLength() / m) * (i + 1);
					while (info[n] != _T(' ') && n < info.GetLength()) n++;
					if(n < info.GetLength())
						info.SetAt(n, _T('\n'));
					else
						break;
				}
			}
			break;
		case SB_UP_SPEED:
			theApp.uploadqueue->GetTransferTipInfo(info);
			break;
		case SB_DN_SPEED:
			theApp.downloadqueue->GetTransferTipInfo(info);
			break;
		case SB_SERVER:
		{
			CServer* server = theApp.serverconnect ? theApp.serverconnect->GetCurrentServer() : NULL;
			if(server && (server = theApp.serverlist->GetServerByAddress(server->GetAddress(), server->GetPort())) != NULL)
				return server->GetServerTooltipInfo(info);

			break;
		}
		case SB_MSG:
			if(theApp.emuledlg->IsTrayIconToFlash())
				info = GetResString(IDS_X_SBMSG);
			else
				info = GetResString(IDS_X_NOMSG);

			break;
	}
	return (HICON)NULL;
}

#else

CString CMuleStatusBarCtrl::GetPaneToolTipText(EStatusBarPane iPane) const
{
	CString strText;
	switch (iPane)
	{
	case SBarConnected:
		if (theApp.serverconnect && theApp.serverconnect->IsConnected())
		{
			CServer* cur_server = theApp.serverconnect->GetCurrentServer();
			CServer* srv = cur_server ? theApp.serverlist->GetServerByAddress(cur_server->GetAddress(), cur_server->GetPort()) : NULL;
			if (srv)
			{
				// Can't add more info than just the server name, unfortunately the MFC tooltip which 
				// we use here does not show more than one(!) line of text.
				strText = _T("eD2K ") + GetResString(IDS_SERVER) + _T(": ") + srv->GetListName();
				strText.AppendFormat(_T("  (%s %s)"), GetFormatedUInt(srv->GetUsers()), GetResString(IDS_UUSERS));
			}
		}
		break;
	}
	return strText;
}

int CMuleStatusBarCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	int iHit = CWnd::OnToolHitTest(point, pTI);
	if (iHit == -1 && pTI != NULL && pTI->cbSize >= sizeof(AFX_OLDTOOLINFO))
	{
		int iPane = GetPaneAtPosition(point);
		if (iPane != -1)
		{
			CString strToolTipText = GetPaneToolTipText((EStatusBarPane)iPane);
			if (!strToolTipText.IsEmpty())
			{
				pTI->hwnd = m_hWnd;
				pTI->uId = (UINT_PTR)iPane;
				pTI->uFlags &= ~TTF_IDISHWND;
				pTI->uFlags |= TTF_NOTBUTTON|TTF_ALWAYSTIP;
				pTI->lpszText = _tcsdup(strToolTipText); // gets freed by MFC
				GetRect(iPane, &pTI->rect);
				iHit = iPane;
			}
		}
	}
	return iHit;
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --