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
#include "DownloadQueue.h" // Enforce Ratio [Stulle] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CMuleStatusBarCtrl

IMPLEMENT_DYNAMIC(CMuleStatusBarCtrl, CStatusBarCtrl)

BEGIN_MESSAGE_MAP(CMuleStatusBarCtrl, CStatusBarCtrl)
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY_RANGE(TTN_GETDISPINFO, 0,SBarChatMsg, OnToolTipNotify) // Enforce Ratio [Stulle] - Stulle
END_MESSAGE_MAP()

CMuleStatusBarCtrl::CMuleStatusBarCtrl()
{
}

CMuleStatusBarCtrl::~CMuleStatusBarCtrl()
{
}

void CMuleStatusBarCtrl::Init(void)
{
	EnableToolTips();
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
	// ==> Enforce Ratio [Stulle] - Stulle
	case SBarUpDown:
		{
			if(!theApp.downloadqueue) // just in case!
				break;
			uint16 uLimitState = theApp.downloadqueue->GetLimitState();
			strText.Format(GetResString(IDS_DL_SES_RATIO)+_T(": %s"),(uLimitState>=DLR_SESLIM)?GetResString(IDS_YES):GetResString(IDS_NO));
			if(uLimitState<DLR_SOURCE && !(uLimitState&DLR_13RATIO || uLimitState&DLR_NAFC))
				strText.AppendFormat(_T("\n\r\x2022 ")+GetResString(IDS_DL_LIMIT_DEF)+_T(": %s (%s)"),GetResString(IDS_YES),(thePrefs.GetMaxDownload()==0xFFFF)?GetResString(IDS_PW_UNLIMITED):CastItoXBytes(thePrefs.GetMaxDownload(),true,true,1));
			else
				strText.AppendFormat(_T("\n\r\x2022 ")+GetResString(IDS_DL_LIMIT_DEF)+_T(": %s"),GetResString(IDS_NO));
			strText.AppendFormat(_T("\n\x2022 ")+GetResString(IDS_DL_UNL_NOUL)+_T(": %s"),(uLimitState&DLR_NOUL)?GetResString(IDS_YES):GetResString(IDS_NO));
			strText.AppendFormat(_T("\n\x2022 ")+GetResString(IDS_DL_UNL_13RATIO)+_T(": %s"),(uLimitState&DLR_13RATIO)?GetResString(IDS_YES):GetResString(IDS_NO));
			strText.AppendFormat(_T("\n\x2022 ")+GetResString(IDS_DL_NAFC_LIMIT)+_T(": %s"),(uLimitState&DLR_NAFC)?GetResString(IDS_YES):GetResString(IDS_NO));
			strText.AppendFormat(_T("\n\x2022 ")+GetResString(IDS_DL_RATIO_SRC)+_T(": %s"),(uLimitState&DLR_SESLIM)?GetResString(IDS_YES):GetResString(IDS_NO));
			strText.AppendFormat(_T("\n\x2022 ")+GetResString(IDS_DL_RATIO_ENF)+_T(": %s"),(uLimitState&DLR_ENFLIM)?GetResString(IDS_YES):GetResString(IDS_NO));
			strText.AppendFormat(_T("\n\x2022 ")+GetResString(IDS_DL_RATIO_FRIEND)+_T(": %s"),(uLimitState&DLR_FRILIM)?GetResString(IDS_YES):GetResString(IDS_NO)); // Multiple friendslots [ZZ] - Mephisto

			if(uLimitState >= DLR_SOURCE || uLimitState&DLR_NAFC)
				strText.AppendFormat(_T("\n\r\x2022 ")+GetResString(IDS_DL_RATIO_LIMIT)+_T(":%i"),theApp.downloadqueue->GetLimitRatio());
			break;
		}
	// <== Enforce Ratio [Stulle] - Stulle
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
				// ==> Enforce Ratio [Stulle] - Stulle
				/*
				pTI->lpszText = _tcsdup(strToolTipText); // gets freed by MFC
				*/
				pTI->lpszText = LPSTR_TEXTCALLBACK;
				// <== Enforce Ratio [Stulle] - Stulle
				GetRect(iPane, &pTI->rect);
				iHit = iPane;
			}
		}
	}
	return iHit;
}

// ==> Enforce Ratio [Stulle] - Stulle
static TCHAR pzToolTipText[512];
void CMuleStatusBarCtrl::OnToolTipNotify( UINT /*id*/, NMHDR * pNotifyStruct, LRESULT * /*result*/ )
{
	TOOLTIPTEXTW* pTI = (TOOLTIPTEXTW*)pNotifyStruct;
    _stprintf(pzToolTipText, GetPaneToolTipText( (EStatusBarPane)pNotifyStruct->idFrom ));
	::SendMessage(pNotifyStruct->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
	pTI->lpszText = pzToolTipText;
}
// <== Enforce Ratio [Stulle] - Stulle

// ==> Design Settings [eWombat/Stulle] - Max
void CMuleStatusBarCtrl::UpdateColor()
{
	COLORREF crTempColor_stb = thePrefs.GetStyleBackColor(window_styles, style_w_statusbar);

	if(crTempColor_stb == CLR_DEFAULT)
		crTempColor_stb = thePrefs.GetStyleBackColor(window_styles, style_w_default);

	SetBkColor(crTempColor_stb);
};
// <== Design Settings [eWombat/Stulle] - Max