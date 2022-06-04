//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "Sockets.h"
#include "Server.h"
#include "ServerList.h"
#include "kademlia/kademlia/kademlia.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CMuleStatusBarCtrl

IMPLEMENT_DYNAMIC(CMuleStatusBarCtrl, CStatusBarCtrl)

BEGIN_MESSAGE_MAP(CMuleStatusBarCtrl, CStatusBarCtrl)
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY_RANGE(TTN_GETDISPINFO, 0, SBarConnected, OnToolTipNotify)//status tooltip
END_MESSAGE_MAP()

CMuleStatusBarCtrl::CMuleStatusBarCtrl()
{
}

CMuleStatusBarCtrl::~CMuleStatusBarCtrl()
{
}
void CMuleStatusBarCtrl::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	int iPane = GetPaneAtPosition(point);
	switch (iPane)
	{
		case SBarLog:
			AfxMessageBox(_T("eMule ") + GetResString(IDS_SV_LOG) + _T("\n\n") + GetText(SBarLog));
			break;
		
		case SBarKadCount:
			{
			CString strURL;
		    strURL = thePrefs.GetNodesDatUpdateURL();
				CString strConfirm;
				strConfirm.Format(GetResString(IDS_CONFIRMNODESDOWNLOAD), strURL);
				if(strURL.GetLength() != 0 && AfxMessageBox(strConfirm, MB_YESNO | MB_ICONQUESTION, 0) == IDYES)
		    theApp.emuledlg->UpdateNodesDatFromURL(strURL);
			break;
			}
		
		case SBarUpDown:
			theApp.emuledlg->SetActiveDialog(theApp.emuledlg->statisticswnd);
			break;
		case SBarConnected:
			if (theApp.serverconnect->IsConnected() || Kademlia::CKademlia::IsConnected())
			theApp.emuledlg->ShowPreferences(IDD_PPG_CONNECTION);
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
//status tooltip +
CString CMuleStatusBarCtrl::GetPaneToolTipText(EStatusBarPane iPane) const
{
	CString strText;
	switch (iPane)
	{
	case SBarConnected:
		if (theApp.serverconnect->IsConnected() && !Kademlia::CKademlia::IsConnected())
	    {
		    if (theApp.serverconnect->IsLowID())
		    {
			strText.Format(GetResString(IDS_STATUS_TIP));
			strText.AppendFormat(GetResString(IDS_STATUS_TIP_2));
		    }
			else
			{
			strText.Format(GetResString(IDS_STATUS_TIP_3));
			strText.AppendFormat(GetResString(IDS_STATUS_TIP_4));
		    }
	    }
	    else if (!theApp.serverconnect->IsConnected() && Kademlia::CKademlia::IsConnected())
	    {
		    if (Kademlia::CKademlia::IsFirewalled())
			{
			strText.Format(GetResString(IDS_STATUS_TIP));
			strText.AppendFormat(GetResString(IDS_STATUS_TIP_2));
		    }
			else
			{
			strText.Format(GetResString(IDS_STATUS_TIP_3));
			strText.AppendFormat(GetResString(IDS_STATUS_TIP_4));
		    }
	     }
	     else if (theApp.serverconnect->IsConnected() && Kademlia::CKademlia::IsConnected())
	     {
	        if (theApp.serverconnect->IsLowID() && Kademlia::CKademlia::IsFirewalled())
		    {
			strText.Format(GetResString(IDS_STATUS_TIP));
			strText.AppendFormat(GetResString(IDS_STATUS_TIP_2));
	        }
		    else if (theApp.serverconnect->IsLowID())
		{
			strText.Format(GetResString(IDS_STATUS_TIP));
			strText.AppendFormat(GetResString(IDS_STATUS_TIP_2));
	        }
		    else if (Kademlia::CKademlia::IsFirewalled())
		{
		    strText.Format(GetResString(IDS_STATUS_TIP));
			strText.AppendFormat(GetResString(IDS_STATUS_TIP_2));
		}
			else
			{
            strText.Format(GetResString(IDS_STATUS_TIP_3));
			strText.AppendFormat(GetResString(IDS_STATUS_TIP_4));
			}
		}
		break;
	}
	return strText;
}
//status tooltip -
INT_PTR CMuleStatusBarCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	INT_PTR iHit = CWnd::OnToolHitTest(point, pTI);
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
				//status tooltip
				//pTI->lpszText = _tcsdup(strToolTipText); // gets freed by MFC
				pTI->lpszText = LPSTR_TEXTCALLBACK;
				//status tooltip
				GetRect(iPane, &pTI->rect);
				iHit = iPane;
			}
		}
	}
	return iHit;
}
//status tooltip
static TCHAR pzToolTipText[512];
void CMuleStatusBarCtrl::OnToolTipNotify( UINT /*id*/, NMHDR * pNotifyStruct, LRESULT * /*result*/ )
{
	TOOLTIPTEXTW* pTI = (TOOLTIPTEXTW*)pNotifyStruct;
    _stprintf(pzToolTipText, GetPaneToolTipText( (EStatusBarPane)pNotifyStruct->idFrom ));
	::SendMessage(pNotifyStruct->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
	pTI->lpszText = pzToolTipText;
}
//status tooltip