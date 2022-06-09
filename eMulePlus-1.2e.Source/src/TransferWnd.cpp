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
#include "updownclient.h"
#include "TransferWnd.h"
#include "otherfunctions.h"
#include "AddBuddy.h"
#include "CatDialog.h"
#include "IP2Country.h"
#include "TitleMenu.h"
#include "ServerList.h"
#include "server.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define TRANS_ROLLUP		120
#define TRANS_DOWNLOADLIST	121
#define TRANS_UPLOADLIST	122
#define TRANS_QUEUELIST		123
#define TRANS_INFOLIST		124
#define TRANS_DOWNLOADTABS	125
#define TRANS_TABWINDOW		126
#define TRANS_CLIENTLIST	127
#define TRANS_DUMMY			130


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CDummyForTabs : public CWnd
{
public:
	CDummyForTabs(){ m_pChild = NULL; }

	void SetChild(CWnd* pChild){ m_pChild = pChild;	}

	CWnd* GetChild(){ return m_pChild; }

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
	{
		EMULE_TRY
		NMHDR* pNMHDR = (NMHDR*)lParam;
		switch(pNMHDR->code)
		{
		case LVN_ITEMACTIVATE:
		case LVN_COLUMNCLICK:
		case NM_CLICK:
		case NM_DBLCLK:
			break;
		default:
			*pResult = GetParent()->GetParent()->SendMessage(WM_NOTIFY, wParam, lParam);
			return TRUE;
		}
		EMULE_CATCH2
		return CWnd::OnNotify(wParam, lParam, pResult);
	}


	afx_msg void OnSize(UINT nType, int cx, int cy)
	{
		NOPRM(nType); NOPRM(cx); NOPRM(cy);
		EMULE_TRY
		if(m_pChild)
		{
			CRect r;
			GetClientRect(&r);
			r -= CSize(2, 2);
			m_pChild->SetWindowPos(NULL, 0,0, r.Width(), r.Height(), SWP_NOMOVE|SWP_NOZORDER);
		}
		EMULE_CATCH2
	}

	DECLARE_DYNCREATE(CDummyForTabs)
	DECLARE_MESSAGE_MAP()

private:
	CWnd* m_pChild;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CCatTabs::DrawItem(CDC *pDC, int iItem)
{
#define COLORBARHEIGHT	2
#define TRIANGLESIZE	12
//#define TRIANGLESIZE	((rItem.bottom-rItem.top+1)/2)
	CTabs::DrawItem(pDC,iItem);

//	Draw the category color bar for user categories.
	if (iItem >= CCat::GetNumPredefinedCats() && iItem < CCat::GetNumCats())
	{
		CRect		rItem;
		CRect		rClient;
		COLORREF	cr;

		GetClientRect(rClient);
		GetItemRect(iItem, rItem);
#ifdef USE_BAR
#ifndef NEW_LOOK
		rItem.top = rClient.top;
		rItem.bottom = rItem.top + COLORBARHEIGHT;
		cr = CCat::GetCatColorByIndex(iItem);
		pDC->FillSolidRect(&rItem,cr);
#else
		if (iItem == GetCurSel())
		{
			rItem.top = rClient.top;
		}
		rItem.top++;
		rItem.bottom = rItem.top + COLORBARHEIGHT;
		cr = CCat::GetCatColorByIndex(iItem);

		CPen		*pOldPen,barPen;

		barPen.CreatePen(PS_SOLID,0,cr);
		pOldPen = pDC->SelectObject(&barPen);
		pDC->MoveTo(rItem.left+1,rItem.top);
		pDC->LineTo(rItem.right-1,rItem.top);
		pDC->MoveTo(rItem.left,rItem.top+1);
		pDC->LineTo(rItem.right,rItem.top+1);
		pDC->SelectObject(pOldPen);
//		pDC->FillSolidRect(&rItem,cr);
#endif NEW_LOOK

#else
		CBrush		*pOldBrush, newBrush;

		cr = CCat::GetCatColorByIndex(iItem);
		newBrush.CreateSolidBrush(cr);
		pOldBrush = pDC->SelectObject(&newBrush);
		rItem.top = rClient.top;
		pDC->BeginPath();
		pDC->MoveTo(rItem.left + 2, rItem.top + 1);
		pDC->LineTo(rItem.left + 2 + TRIANGLESIZE, rItem.top + 1);
		pDC->LineTo(rItem.left + 2, rItem.top + 1 + TRIANGLESIZE);
		pDC->EndPath();
		pDC->FillPath();
		pDC->SelectObject(pOldBrush);
#endif USE_BAR
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDummyForTabs, CWnd)

BEGIN_MESSAGE_MAP(CDummyForTabs, CWnd)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CTransferWnd, CResizableDialog)
	ON_WM_DESTROY()	// eklmn: bugfix(00): resource cleanup due to CResizableDialog
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()

//	ON_NOTIFY(TCN_SELCHANGE, TRANS_DOWNLOADTABS, OnTcnSelchangeDltab)
//	ON_NOTIFY(NM_RCLICK, TRANS_DOWNLOADTABS, OnNMRClickDltab)
//	ON_NOTIFY(NM_TABMOVED, TRANS_DOWNLOADTABS, OnTabMovement)
// ON_NOTIFY(LVN_BEGINDRAG, TRANS_DOWNLOADLIST, OnLvnBeginDrag)

	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

// CTransferWnd dialog

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CTransferWnd, CDialog)
CTransferWnd::CTransferWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CTransferWnd::IDD, pParent)
{
	m_bIsDragging = false;
	m_pwndDummyForDownloadList = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTransferWnd::~CTransferWnd()
{}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CTransferWnd::OnInitDialog()
{
	EMULE_TRY

	CResizableDialog::OnInitDialog();

	//	Create the Rollup control
	CRect		rRollupArea;

	GetClientRect(&rRollupArea);
	rRollupArea.DeflateRect(8, 5);

	m_ctlRollup.Create(NULL, NULL, WS_CHILD|WS_VISIBLE/*|WS_CLIPCHILDREN*/, rRollupArea, this, TRANS_ROLLUP);

	//	Create the Download-list control
	CWnd		*pParent = &m_ctlRollup;
	CRect		rItem = CRect(0, 0, 0, 0);
	DWORD		dwListStyle = WS_CHILD|WS_BORDER|WS_TABSTOP|LVS_REPORT|LVS_ALIGNLEFT|LVS_OWNERDRAWFIXED|LVS_SINGLESEL;
	DWORD		dwTabStyle = WS_CHILD|WS_VISIBLE|TCS_HOTTRACK|TCS_SINGLELINE|TCS_TOOLTIPS;

	m_pwndDummyForDownloadList = new CDummyForTabs();
	m_pwndDummyForDownloadList->Create(AfxRegisterWndClass(0), _T(""), WS_CHILD|WS_VISIBLE, rItem, pParent, TRANS_DUMMY);

	m_ctlDownloadList.Create(WS_VISIBLE | dwListStyle, rItem, m_pwndDummyForDownloadList, TRANS_DOWNLOADLIST);

	g_App.m_pDownloadList->SetDownloadListCtrl(&m_ctlDownloadList);
	m_ctlUploadList.Create(WS_VISIBLE | dwListStyle, rItem, pParent, TRANS_UPLOADLIST);
	m_ctlQueueList.Create(/*WS_VISIBLE |*/ dwListStyle, rItem, pParent, TRANS_QUEUELIST);
	m_ctlClientList.Create(/*WS_VISIBLE |*/ dwListStyle, rItem, pParent, TRANS_CLIENTLIST);

	g_App.m_pClientList->SetClientListCtrl(&m_ctlClientList);

	m_ctlInfoList.Create(WS_VISIBLE|WS_CHILD|WS_BORDER|WS_TABSTOP|LVS_REPORT|
							LVS_ALIGNLEFT|LVS_SINGLESEL, rItem, pParent, TRANS_INFOLIST);

	m_ctlDLTabs.Create(dwTabStyle, CRect(0, 0, 0, 20), pParent, TRANS_DOWNLOADTABS);
	m_ctlDLTabs.ModifyStyle(WS_CLIPSIBLINGS, 0);
	m_ctlDLTabs.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_ctlDLTabs.SetPadding(CSize(16, 0));

	m_pwndDummyForDownloadList->SetChild(&m_ctlDownloadList);

	AddBuddy(m_pwndDummyForDownloadList->m_hWnd, m_ctlDLTabs.m_hWnd, BDS_TOP);

	CString		strTmp;

	strTmp.Format(GetResString(IDS_RUP_DOWNINFO), g_App.m_pDownloadQueue->GetFileCount(),
						g_App.m_pDownloadQueue->GetActiveFileCount(),
						g_App.m_pDownloadQueue->GetPausedFileCount(),
						g_App.m_pDownloadQueue->GetStoppedFileCount(),
						g_App.m_pDownloadQueue->GetTransferringFiles(),
						static_cast<double>(g_App.m_pDownloadQueue->GetDataRate()) / 1024.0);
	m_ctlRollup.InsertItem(GetResString(IDS_RUP_DOWNLOADS), strTmp, m_pwndDummyForDownloadList, 0, TRUE);

	strTmp.Format(GetResString(IDS_RUP_UPINFO), g_App.m_pUploadQueue->GetWaitingUserCount(),
					  g_App.m_pUploadQueue->GetUploadQueueLength(),
					  g_App.m_pUploadQueue->GetBanCount(),
					  static_cast<double>(g_App.m_pUploadQueue->GetDataRate()) / 1024.0);
	m_ctlRollup.InsertItem(GetResString(IDS_RUP_UPLOADS), strTmp, &m_ctlUploadList, 1, TRUE);
	m_ctlRollup.InsertItem(GetResString(IDS_RUP_INFO), _T(""), &m_ctlInfoList, 2, FALSE);

	AddAnchor(m_ctlRollup.m_hWnd, TOP_LEFT, BOTTOM_RIGHT);

	InitRollupItemHeights();

	m_ctlDownloadList.Init();
	m_ctlUploadList.Init();
	m_ctlQueueList.Init();
	m_ctlClientList.Init();

	m_ctlDownloadList.SetOwner(this);
	m_ctlUploadList.SetOwner(this);
	m_ctlQueueList.SetOwner(this);
	m_ctlClientList.SetOwner(this);
	m_ctlDLTabs.SetOwner(this);

	m_nActiveWnd = MPW_UPLOADLIST;
	m_iTabRightClickIndex = -1;

	//	Initialize the Category tabs
	for (int ix = 0; ix < CCat::GetNumCats(); ix++)
		m_ctlDLTabs.InsertItem(ix,CCat::GetCatByIndex(ix)->GetTitle());

	// List controls & Category tabs tooltips
	m_ttip.Create(this);
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 15000);
	m_ttip.SetDelayTime(TTDT_INITIAL, g_App.m_pPrefs->GetToolTipDelay()*1000);
	m_ttip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
	m_ttip.SetBehaviour(PPTOOLTIP_MULTIPLE_SHOW);
	m_ttip.SetNotify(m_hWnd);
	m_ttip.AddTool(&m_ctlDownloadList, _T(""));
	m_ttip.AddTool(&m_ctlUploadList, _T(""));
	m_ttip.AddTool(&m_ctlQueueList, _T(""));
	m_ttip.AddTool(&m_ctlClientList, _T(""));
	m_ttip.AddTool(&m_ctlDLTabs, _T(""));

	EMULE_CATCH

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Resource cleanup due to CResizableDialog
void CTransferWnd::OnDestroy()
{
	if(m_pwndDummyForDownloadList)
	{
		m_pwndDummyForDownloadList->DestroyWindow();
		delete m_pwndDummyForDownloadList;
	}
	CResizableDialog::OnDestroy();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransferWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTransferWnd message handlers
BOOL CTransferWnd::PreTranslateMessage(MSG* pMsg)
{
	if (g_App.m_pPrefs->GetToolTipDelay() != 0)
	{
		switch (pMsg->message)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MOUSEMOVE:
			case WM_MOUSEWHEEL:
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				m_ttip.RelayEvent(pMsg);
		}
	}

	if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		if (pMsg->hwnd == m_ctlDLTabs.GetSafeHwnd())
		{
			POINT point;

			::GetCursorPos(&point);

			CPoint pt(point);

			if (GetTabUnderMouse(&pt) != -1)
			{
				OnDblClickDltab();
				return TRUE;
			}
		}
	}
	else if (pMsg->message == WM_MBUTTONUP)
	{
	//	If we are in Download list, display user or file details
		int iSel = GetItemUnderMouse(&m_ctlDownloadList);

		if (iSel != -1)
		{
			m_ctlDownloadList.ShowSelectedFileOrUserDetails();
		}
		else 
		{
			switch (m_nActiveWnd)
			{
				case MPW_UPLOADQUEUELIST:
					m_ctlQueueList.ShowSelectedUserDetails();
					break;

				case MPW_UPLOADLIST:
					m_ctlUploadList.ShowSelectedUserDetails();
					break;

				case MPW_UPLOADCLIENTLIST:
					m_ctlClientList.ShowSelectedUserDetails();
					break;
			}
		}
		return TRUE;
	}
	else if(pMsg->message == USRMSG_SWITCHUPLOADLIST)
		SwitchUploadList();

	return CResizableDialog::PreTranslateMessage(pMsg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CTransferWnd::GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY *pNotify)
{
	EMULE_TRY

	int						iControlId = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	if (iControlId == NULL)
		return;

	CString strInfo;

	switch (iControlId)
	{
		case TRANS_DOWNLOADTABS:
		{
			CPoint	ptLoc(*pNotify->pt);
			int		iIndex = GetTabUnderMouse(&ptLoc);

			if (iIndex < 0)
				return;

			uint32		dwCount = 0, dwTransferring = 0, dwPaused = 0, dwStopped = 0;
			double		dblSpeed = 0.0;
			uint64		qwSize = 0, qwCompletedSize = 0, qwRealSize = 0;
			CPartFile	*pPartFile;
			CString		strCatTitle;
			CCat		*pCat = CCat::GetCatByIndex(iIndex);

			if (iIndex != 0 || CCat::GetAllCatType() == CAT_ALL)
				strCatTitle = pCat->GetTitle();
			else
				strCatTitle = _T("[") + CCat::GetPredefinedCatTitle(CCat::GetAllCatType()) + _T("]");

			if (pCat->GetID() < CAT_PREDEFINED)
				strInfo.Format(_T("<cat=0x%06x>"), CCat::GetCatColorByIndex(iIndex));

			for (int i = 0; i < g_App.m_pDownloadQueue->GetFileCount(); i++)
			{
				pPartFile = g_App.m_pDownloadQueue->GetFileByIndex(i);

				if (pPartFile != NULL)
				{
					if (CCat::FileBelongsToGivenCat(pPartFile, CCat::GetCatIDByIndex(iIndex)))
					{
						dwCount++;
						if (pPartFile->GetTransferringSrcCount() > 0)
							dwTransferring++;
						if (pPartFile->IsPaused() && !pPartFile->IsStopped())
							dwPaused++;
						if (pPartFile->IsStopped())
							dwStopped++;
						dblSpeed += pPartFile->GetDataRate() / 1024.0f;
						qwSize += pPartFile->GetFileSize();
						qwCompletedSize += pPartFile->GetCompletedSize();
						qwRealSize += pPartFile->GetRealFileSize();
					}
				}
			}

			if (!pCat->GetComment().IsEmpty())
			{
				strCatTitle += _T(" (");
				strCatTitle += pCat->GetComment();
				strCatTitle += _T(")");
			}
			strCatTitle.Replace(_T("\n"), _T("<br>"));
			strCatTitle.Replace(_T("<"), _T("<<"));

			strInfo.AppendFormat(_T("<b>%s</b><br><hr=100%%><br><b>%s:</b><t>%u / %u<br><b>%s:</b><t>%u<br><b>%s:</b><t>%u<br><b>%s:</b><t>%.1f %s<br><b>%s:</b><t>%s / %s<br><b>%s:</b><t>%s<br><b>%s</b><t>%s<br><b>%s</b><t>%s"),
				strCatTitle,
				GetResString(IDS_DOWNLOADING), dwTransferring, (dwCount - dwPaused - dwStopped),
				GetResString(IDS_PAUSED), dwPaused, GetResString(IDS_STOPPED), dwStopped,
				GetResString(IDS_DL_SPEED), dblSpeed, GetResString(IDS_KBYTESEC),
				GetResString(IDS_DL_SIZE), CastItoXBytes(qwCompletedSize), CastItoXBytes(qwSize),
				GetResString(IDS_SIZE_ON_DISK), CastItoXBytes(qwRealSize),
				GetResString(IDS_PW_INCOMING), pCat->GetPath(),
				GetResString(IDS_PW_TEMP), pCat->GetTempPath());
			break;
		}
		case TRANS_DOWNLOADLIST:
		{
			if (m_ctlDownloadList.GetItemCount() < 1)
				return;

			int		iSel = GetItemUnderMouse(&m_ctlDownloadList);

			if (iSel < 0 || iSel == 65535)
				return;

			//	Build info text and display it
			CMuleCtrlItem		*pItem = reinterpret_cast<CMuleCtrlItem*>(m_ctlDownloadList.GetItemData(iSel));
			CPartFileDLItem		*pPartFileItem = dynamic_cast<CPartFileDLItem*>(pItem);
			CSourceDLItem		*pSourceItem = dynamic_cast<CSourceDLItem*>(pItem);

			if (pPartFileItem != NULL)
			{
				CPartFile		*pPartFile = pPartFileItem->GetFile();
				SHFILEINFO		shfi;

				strInfo = pPartFile->GetDownloadFileInfo4Tooltips();
				memzero(&shfi, sizeof(shfi));
				SHGetFileInfo(pPartFile->GetFileName(),	FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),	SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
				pNotify->ti->hIcon = shfi.hIcon;
			}
			else if (pSourceItem != NULL)
			{
#ifdef OLD_SOCKETS_ENABLED
				bool				bA4AF = pSourceItem->IsAskedForAnotherFile();
				CUpDownClient	    *pSource = pSourceItem->GetSource();
				CString				strTmp, strUserName = pSource->GetUserName();

				strUserName.Trim();
				strUserName.Replace(_T("\n"), _T("<br>"));
				strUserName.Replace(_T("<"), _T("<<"));

				if (g_App.m_pIP2Country->IsIP2Country())
					strTmp.Format(_T(" (<b>%s</b>)"), pSource->GetCountryName());

				strInfo.Format(_T("<t=1><b>%s</b><br><t=1>%s: %u%s<br><hr=100%%><br><b>%s:<t></b>%s:%u (<b>%s</b>)"),
					strUserName, GetResString(IDS_USERID), pSource->GetUserIDHybrid(), strTmp,
					GetResString(IDS_CLIENT), pSource->GetFullIP(), pSource->GetUserPort(), pSource->GetFullSoftVersionString());

				CServer *pCurSrv, *pSrv = g_App.m_pServerList->GetServerByIP(pSource->GetServerIP(), pSource->GetServerPort());

				if (pSrv != NULL)
				{
					strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s:%u"), GetResString(IDS_SERVER), ipstr(pSource->GetServerIP()), pSource->GetServerPort());

					strTmp = pSrv->GetListName();
					strTmp.Replace(_T("<"), _T("<<"));
					strTmp.Replace(_T("\n"), _T("<br>"));
					if (!strTmp.IsEmpty())
					{
						strInfo += _T(" (<b>");
						strInfo += strTmp;
						strInfo += _T("</b>)");
					}
				}
				
				if (pSource->Credits() != NULL && pSource->Credits()->GetUploadedTotal() != pSource->GetTransferredUp())
					strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s (%s)"), GetResString(IDS_STATS_DDATA), CastItoXBytes(pSource->GetTransferredUp()), CastItoXBytes(pSource->Credits()->GetUploadedTotal()));
				else if (pSource->GetTransferredUp() > 0)
					strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s"), GetResString(IDS_STATS_DDATA), CastItoXBytes(pSource->GetTransferredUp()));

				if (pSource->Credits() != NULL && pSource->Credits()->GetDownloadedTotal() != pSource->GetTransferredDown())
					strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s (%s)"), GetResString(IDS_STATS_UDATA), CastItoXBytes(pSource->GetTransferredDown()), CastItoXBytes(pSource->Credits()->GetDownloadedTotal()));
				else if (pSource->GetTransferredDown() > 0)
					strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s"), GetResString(IDS_STATS_UDATA), CastItoXBytes(pSource->GetTransferredDown()));

				strInfo.AppendFormat(_T("<br><b>%s:<t></b>%d"),	GetResString(IDS_TT_ASKEDCOUNT), pSource->GetAskedCountDown());

				pCurSrv = g_App.m_pServerConnect->GetCurrentServer();
				if ( (pSource->GetDownloadState() == DS_ONQUEUE) || ( (pSource->GetDownloadState() == DS_NONEEDEDPARTS) && 
					(!pSource->HasLowID() || ((pCurSrv != NULL) && (pCurSrv->GetIP() == pSource->GetServerIP()))) ) )
				{
					if (pSource->IsUDPFileReqPending())
						strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_NEXT_ASK), GetResString(IDS_ASKING));
					else
					{
						uint32 	dwFileReaskTime = pSource->GetNextFileReaskTime();
						uint32 	dwCurTick = ::GetTickCount();

						if (dwCurTick <= dwFileReaskTime)
							strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_NEXT_ASK), CastSecondsToHM((dwFileReaskTime - dwCurTick) / 1000));
					}
				}

				strInfo.AppendFormat(_T("<br><b>%s:<t></b>%d"),	GetResString(IDS_TT_AVAILABLEPARTS), pSource->GetAvailablePartCount());

				if (!bA4AF)
				{
					if (!pSource->IsClientFilenameEmpty())
						strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_TT_CLIENTSOURCENAME), pSource->GetClientFilename());
				}
				else	//	If Asked For Another File...
				{
					try
					{
						strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_ASKED4ANOTHERFILE), pSource->m_pReqPartFile->GetFileName());
					}
					catch(...)
					{
					}
				}
				//-For File Comment-//
				try
				{
					if (!bA4AF)
					{
						if (!pSource->IsFileCommentEmpty())
							strInfo.AppendFormat(_T("<br><hr=100%%><br><b>%s</b><t>%s<br>"), GetResString(IDS_CMT_READ), pSource->GetFileComment());
						else
							strInfo.AppendFormat(_T("<br><hr=100%%><br>%s<br>"), GetResString(IDS_CMT_NONE));

						if (pSource->GetFileRating() == PF_RATING_NONE)
							strInfo += GetRatingString(pSource->GetFileRating());
						else
							strInfo.AppendFormat(_T("<b>%s:</b><t>%s"), GetResString(IDS_TT_CMT_RATING), GetRatingString(pSource->GetFileRating()));
					}
				}
				catch(...)
				{
					//Information not received = not connected or connecting
					strInfo.AppendFormat(_T("<br><hr=100%%><br>%s"), GetResString(IDS_CMT_NOTCONNECTED));
				}

			// Set the tooltip icon
				pNotify->ti->hIcon =
					g_App.m_pMDlg->m_clientImgLists[CLIENT_IMGLST_PLAIN].ExtractIcon(pSource->GetClientIconIndex());
#endif //OLD_SOCKETS_ENABLED
			}
			break;
		}
		case TRANS_UPLOADLIST:
		{
			if (m_ctlUploadList.GetItemCount() < 1)
				return;

			int		iSel = GetItemUnderMouse(&m_ctlUploadList);

			if (iSel < 0 || iSel == 65535)
				return;

			CUpDownClient *pSource = (CUpDownClient*)m_ctlUploadList.GetItemData(iSel);

			pNotify->ti->hIcon = pSource->GetClientInfo4Tooltips(strInfo, true);
			break;
		}
		case TRANS_QUEUELIST:
		{
			if (m_ctlQueueList.GetItemCount() < 1)
				return;

			int		iSel = GetItemUnderMouse(&m_ctlQueueList);

			if (iSel < 0 || iSel == 65535)
				return;

			CUpDownClient *pSource = (CUpDownClient*)m_ctlQueueList.GetItemData(iSel);

			pNotify->ti->hIcon = pSource->GetClientInfo4Tooltips(strInfo);
			break;
		}
		case TRANS_CLIENTLIST:
		{
			if (m_ctlClientList.GetItemCount() < 1)
				return;

			int		iSel = GetItemUnderMouse(&m_ctlClientList);

			if (iSel < 0 || iSel == 65535)
				return;

			CUpDownClient *pSource = (CUpDownClient*)m_ctlClientList.GetItemData(iSel);

			pNotify->ti->hIcon = pSource->GetClientInfo4Tooltips(strInfo);
			break;
		}
	}

	pNotify->ti->sTooltip = strInfo;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SwitchUploadList() toggles the visibility of the UploadList with the UploadQueueList
void CTransferWnd::SwitchUploadList()
{
	EMULE_TRY

	switch (m_nActiveWnd)
	{
		case MPW_UPLOADLIST:
		{
			m_nActiveWnd = MPW_UPLOADQUEUELIST;
			m_ctlRollup.SetText(1, GetResString(IDS_ONQUEUE), true);
			m_ctlUploadList.ShowWindow(SW_HIDE);
			m_ctlClientList.ShowWindow(SW_HIDE);
			m_ctlRollup.SetItemClient(1, &m_ctlQueueList);
			m_ctlRollup.Invalidate();
			UpdateQueueFilter();
			break;
		}
		case MPW_UPLOADQUEUELIST:
		{
			m_nActiveWnd = MPW_UPLOADCLIENTLIST;
			UpdateKnown();
			m_ctlUploadList.ShowWindow(SW_HIDE);
			m_ctlQueueList.ShowWindow(SW_HIDE);
			m_ctlRollup.SetItemClient(1, &m_ctlClientList);
			m_ctlRollup.Invalidate();
			break;
		}
		case MPW_UPLOADCLIENTLIST:
		{
			m_nActiveWnd = MPW_UPLOADLIST;
			m_ctlRollup.SetText(1, GetResString(IDS_RUP_UPLOADS), true);
			m_ctlQueueList.ShowWindow(SW_HIDE);
			m_ctlClientList.ShowWindow(SW_HIDE);
			m_ctlRollup.SetItemClient(1, &m_ctlUploadList);
			m_ctlRollup.Invalidate();
			break;
		}
		default:
			break;
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Update Known Clients on header when total number of known clients changed
void CTransferWnd::UpdateKnown()
{
	if(m_nActiveWnd == MPW_UPLOADCLIENTLIST)
	{
		CString	strBuf = GetResString(IDS_CLIENTLIST);

		strBuf.AppendFormat(_T(" (%u)"), m_ctlClientList.GetItemCount());
		m_ctlRollup.SetText(1, strBuf, true);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Update Applied Filter on header when filter changes (triggered in QueueListCtrl)
void CTransferWnd::UpdateQueueFilter()
{
	static const uint16 auResStr[] = {
		IDS_BANNED,		//CLI_FILTER_BANNED
		IDS_FRIENDS,	//CLI_FILTER_FRIEND
		IDS_WITHCREDITS	//CLI_FILTER_CREDIT
	};

	if (m_nActiveWnd == MPW_UPLOADQUEUELIST)
	{
		CString	strBuffer = GetResString(IDS_ONQUEUE);

		if (static_cast<unsigned>(m_ctlQueueList.m_iClientFilter - CLI_FILTER_BANNED) < ARRSIZE(auResStr))
			strBuffer.AppendFormat(_T(" (%s)"), GetResString(auResStr[m_ctlQueueList.m_iClientFilter - CLI_FILTER_BANNED]));

		m_ctlRollup.SetText(1, strBuffer, true);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Localize() loads localized text for the UI resources
void CTransferWnd::Localize()
{
	EMULE_TRY

	switch(m_nActiveWnd)
	{
		case MPW_UPLOADLIST:
			m_ctlRollup.SetText(1,GetResString(IDS_RUP_UPLOADS), true);
			break;
		case MPW_UPLOADQUEUELIST:
			UpdateQueueFilter();
			break;
		case MPW_UPLOADCLIENTLIST:
			UpdateKnown();
			break;
	}

//	Localize the rollup control
	m_ctlRollup.SetText(0, GetResString(IDS_RUP_DOWNLOADS), true);
	m_ctlRollup.SetText(2, GetResString(IDS_RUP_INFO), true);
	UpdateInfoHeader();

	m_ctlDownloadList.Localize();
	m_ctlUploadList.Localize();
	m_ctlQueueList.Localize();
	m_ctlClientList.Localize();
	m_ctlInfoList.Localize();

	CString  strCatTitle;

	for (byte i = 0; i < CCat::GetNumPredefinedCats(); i++)
	{
		if (i == 0)
		{
			_EnumCategories eCatID = CCat::GetAllCatType();

			strCatTitle = CCat::GetPredefinedCatTitle(eCatID);
			if (eCatID != CAT_ALL)
			{
				strCatTitle = _T('[') + strCatTitle;
				strCatTitle += _T(']');
			}
			CCat::GetCatByIndex(i)->SetTitle(GetResString(IDS_CAT_ALL));
		}
		else
		{
			strCatTitle = CCat::GetPredefinedCatTitle(CCat::GetCatIDByIndex(i));
			CCat::GetCatByIndex(i)->SetTitle(strCatTitle);
		}

		EditCatTabLabel(i, strCatTitle);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	UpdateDownloadHeader() updates the information displayed in the right side of the DownloadList rollup bar
void CTransferWnd::UpdateDownloadHeader()
{
	EMULE_TRY

	CString strBuffer;

	strBuffer.Format( GetResString(IDS_RUP_DOWNINFO),
					g_App.m_pDownloadQueue->GetFileCount(),
					g_App.m_pDownloadQueue->GetActiveFileCount(),
					g_App.m_pDownloadQueue->GetPausedFileCount(),
					g_App.m_pDownloadQueue->GetStoppedFileCount(),
					g_App.m_pDownloadQueue->GetTransferringFiles(),
					static_cast<double>(g_App.m_pDownloadQueue->GetDataRate())/1024.0 );
	m_ctlRollup.SetText(0, strBuffer);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	UpdateUploadHeader() updates the information displayed in the right side of the UploadList rollup bar
void CTransferWnd::UpdateUploadHeader()
{
	EMULE_TRY

	CString strBuffer;

	strBuffer.Format( GetResString(IDS_RUP_UPINFO),
					  g_App.m_pUploadQueue->GetWaitingUserCount(),
						g_App.m_pUploadQueue->GetUploadQueueLength(),
							g_App.m_pUploadQueue->GetBanCount(),
								static_cast<double>(g_App.m_pUploadQueue->GetDataRate())/1024.0);
	m_ctlRollup.SetText(1, strBuffer);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	UpdateInfoHeader() updates the information displayed in the right side of the InfoList rollup bar
void CTransferWnd::UpdateInfoHeader()
{
	EMULE_TRY

	CString strInfoHeader;

	switch (m_ctlInfoList.GetType())
	{
		case INFOLISTTYPE_SOURCE:
			strInfoHeader.Format(GetResString(IDS_RUP_INFOUSER), m_ctlInfoList.GetName());
			break;
		case INFOLISTTYPE_FILE:
			strInfoHeader.Format(GetResString(IDS_RUP_INFOFILE), m_ctlInfoList.GetName());
			break;
	}

	m_ctlRollup.SetText(2, strInfoHeader);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransferWnd::InitRollupItemHeights()
{
	static const double s_adDefItemHeights[3][8] =
	{
		{ 0, 100, 0, 70, 0, 70, 0, 50 },
		{ 0, 0, 100, 30, 0, 0, 70, 25 },
		{ 0, 0, 0, 0, 100, 30, 30, 25 }
	};

	EMULE_TRY

	double	(*padItemHeights)[8] = (double(*)[8])g_App.m_pPrefs->GetRollupPosPtr();
	double	(*padHeights)[8];
	bool	*pbState, bZero, bUseDefault = false;

	for (int exp = 0; exp < 8; exp++)
	{
		double dTotHeight = 0.0;

		for (int i = 0; i < 3; i++)
		{
			bZero = (padItemHeights[i][exp] == 0);
		//	Protect against wrong values
			if (!bZero && (padItemHeights[i][exp] < 7))
			{
				bUseDefault = true;
				break;
			}
		//	Total up the percentages of expanded sub-panes, collapsed ones have to be zero
			if ((exp & (1 << i)) != 0)
				dTotHeight += padItemHeights[i][exp];
			else if (!bZero)
			{
				bUseDefault = true;
				break;
			}
		}
		if (bUseDefault)
			break;

	//	If the percentages don't total up to 100 (except 0 when all are disabled)...
		if ((dTotHeight <= 99 || dTotHeight >= 101) && (dTotHeight != 0 || exp != 0))
		{
			bUseDefault = true;
			break;
		}
	}

//	Restore the user's last expansion state
	if (!bUseDefault)
	{
		pbState = g_App.m_pPrefs->GetRollupStatePtr();
		for (int i = 0; i < 3; i++)
			m_ctlRollup.ExpandItem(i, pbState[i]);
	}

	padHeights = (bUseDefault) ? (double (*)[8])s_adDefItemHeights : padItemHeights;

	m_ctlRollup.SetItemHeights(0, padHeights[0], sizeof(*padHeights));
	m_ctlRollup.SetItemHeights(1, padHeights[1], sizeof(*padHeights));
	m_ctlRollup.SetItemHeights(2, padHeights[2], sizeof(*padHeights));

	m_ctlInfoList.SetColumn(g_App.m_pPrefs->GetDetailColumnWidth());

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransferWnd::SaveRollupItemHeights()
{
	EMULE_TRY

//	Don't try to save if objects weren't created
	if (m_ctlRollup.GetCount() >= 3)
	{
		double	(*padItemHeights)[8] = (double(*)[8])g_App.m_pPrefs->GetRollupPosPtr();
		double	f1, f2, f3;

		for (int exp = 0; exp < 8; exp++)
		{
			f1 = m_ctlRollup.GetItem(0)->adSizes[exp];
			f2 = m_ctlRollup.GetItem(1)->adSizes[exp];
			f3 = m_ctlRollup.GetItem(2)->adSizes[exp];

			// get rid off some rounding errors
			if(f3)
				f3 = f1 ? (f2 ? (100 - f1 - f2) : (100 - f1)) : (100 - f2);
			else if(f2)
				f2 = 100 - f1;

			padItemHeights[0][exp] = f1;
			padItemHeights[1][exp] = f2;
			padItemHeights[2][exp] = f3;
		}
			
		bool	*pbState = g_App.m_pPrefs->GetRollupStatePtr();

		pbState[0] = m_ctlRollup.GetItem(0)->pHeader->IsExpanded();
		pbState[1] = m_ctlRollup.GetItem(1)->pHeader->IsExpanded();
		pbState[2] = m_ctlRollup.GetItem(2)->pHeader->IsExpanded();

		g_App.m_pPrefs->SetDetailColumnWidth(m_ctlInfoList.GetColumn());
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CTransferWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam >= MP_CAT_SET_0 && wParam <= MP_CAT_SET_LAST)
	{
		EnumCategories		eCatID = static_cast<_EnumCategories>(static_cast<int>(CAT_PREDEFINED) + (wParam - MP_CAT_SET_0));

		CCat::SetAllCatType(eCatID);
		m_ctlDLTabs.SetCurSel(0);
		m_ctlDownloadList.ChangeCategoryByIndex(0);
		if (eCatID == CAT_ALL)
			EditCatTabLabel(0, CCat::GetPredefinedCatTitle(CCat::GetAllCatType()));
		else
			EditCatTabLabel(0, _T("[")+CCat::GetPredefinedCatTitle(CCat::GetAllCatType())+_T("]"));
		return TRUE;
	}
	if (wParam >= MP_SHOWPREDEFINEDCAT_0 && wParam <= MP_SHOWPREDEFINEDCAT_LAST)
	{
		int					iCatPos = wParam - MP_SHOWPREDEFINEDCAT_0;
		EnumCategories		eCatID = static_cast<_EnumCategories>(iCatPos + CAT_PREDEFINED);
		int					iCatIndex = CCat::GetCatIndexByID(eCatID);

	//	If the predefined cat isn't in...
		if (iCatIndex == -1)
		{
			AddPredefinedCategory(eCatID);
			g_App.m_pPrefs->SaveCats();
		}
		else
		{
			CCat::RemoveCatByIndex(iCatIndex);
			m_ctlDLTabs.DeleteItem(iCatIndex);
			m_ctlDLTabs.SetCurSel(0);
			m_ctlDownloadList.ChangeCategoryByID(CAT_ALL);
			g_App.m_pPrefs->SaveCats();
		}
		UpdateCatTabTitles();
		return TRUE;
	}

	switch (wParam)
	{
		case USRMSG_SWITCHUPLOADLIST:
			SwitchUploadList();
			break;

		case USRMSG_CLEARCOMPLETED:
			g_App.m_pDownloadList->ClearCompleted(CAT_NONE);
			break;

		case MP_CAT_ADD:
		{
			int newindex = AddCategory(_T("?"), g_App.m_pPrefs->GetIncomingDir(), g_App.m_pPrefs->GetTempDir(), _T(""), _T(""), true);
			CCatDialog dialog(newindex);

			if (dialog.DoModal() == IDCANCEL)
			{
				CCat::RemoveCatByIndex(newindex);
				m_ctlDLTabs.DeleteItem(newindex);
				m_ctlDLTabs.SetCurSel(0);
				m_ctlDownloadList.ChangeCategoryByIndex(0);
				g_App.m_pPrefs->SaveCats();
				if (CCat::GetNumCats() == 1)
				{
					CCat::SetAllCatType(CAT_ALL);
				}
				g_App.m_pMDlg->m_dlgSearch.UpdateCatTabs();
			}
			else
			{
				g_App.m_pMDlg->m_dlgSearch.UpdateCatTabs();
				EditCatTabLabel(newindex, CCat::GetCatByIndex(newindex)->GetTitle());
				g_App.m_pPrefs->SaveCats();
			}
			break;
		}

		case MP_CAT_EDIT:
		{
			CCatDialog		dialog(m_iTabRightClickIndex);

			dialog.DoModal();

			CString csName = CCat::GetCatByIndex(m_iTabRightClickIndex)->GetTitle();

			EditCatTabLabel(m_iTabRightClickIndex, csName);
			g_App.m_pMDlg->m_dlgSearch.UpdateCatTabs();
			g_App.m_pPrefs->SaveCats();
			break;
		}
		case MP_CAT_REMOVE:
		{
			g_App.m_pDownloadQueue->ResetCatParts(m_iTabRightClickIndex);
			CCat::RemoveCatByIndex(m_iTabRightClickIndex);
			m_ctlDLTabs.DeleteItem(m_iTabRightClickIndex);
			m_ctlDLTabs.SetCurSel(0);
			m_ctlDownloadList.ChangeCategoryByID(CAT_ALL);
			g_App.m_pPrefs->SaveCats();
			if (CCat::GetNumCats() == 1)
			{
				CCat::SetAllCatType(CAT_ALL);
			}
			g_App.m_pMDlg->m_dlgSearch.UpdateCatTabs();
			break;
		}
		case MP_PRIOLOW:
			g_App.m_pDownloadQueue->SetCatPrio(m_iTabRightClickIndex,PR_LOW);
			break;

		case MP_PRIONORMAL:
			g_App.m_pDownloadQueue->SetCatPrio(m_iTabRightClickIndex,PR_NORMAL);
			break;

		case MP_PRIOHIGH:
			g_App.m_pDownloadQueue->SetCatPrio(m_iTabRightClickIndex,PR_HIGH);
			break;

		case MP_PRIOAUTO:
			g_App.m_pDownloadQueue->SetCatPrio(m_iTabRightClickIndex,PR_AUTO);
			break;

		case MP_PAUSE:
			g_App.m_pDownloadQueue->SetCatStatus(m_iTabRightClickIndex,MP_PAUSE);
			break;

		case MP_STOP:
			g_App.m_pDownloadQueue->SetCatStatus(m_iTabRightClickIndex,MP_STOP);
			break;

		case MP_CANCEL:
			if (AfxMessageBox(GetResString(IDS_Q_CANCELDL),MB_ICONQUESTION|MB_YESNO) == IDYES)
				g_App.m_pDownloadQueue->SetCatStatus(m_iTabRightClickIndex, MP_CANCEL);
			break;

		case MP_RESUME:
			g_App.m_pDownloadQueue->SetCatStatus(m_iTabRightClickIndex,MP_RESUME);
			break;

		case MP_RESUMENEXT:
			g_App.m_pDownloadQueue->StartNextFile(CCat::GetCatIDByIndex(m_iTabRightClickIndex));
			break;

		default:
			break;
	}

	return CResizableDialog::OnCommand(wParam, lParam);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetTabUnderMouse() returns the index of the category tab at point 'point' or -1 if there is none.
int CTransferWnd::GetTabUnderMouse(CPoint* point)
{
	TCHITTESTINFO hitinfo;
	CRect rect;

	m_ctlDLTabs.GetWindowRect(&rect);
	point->Offset(0-rect.left,0-rect.top);
	hitinfo.pt = *point;

	if(m_ctlDLTabs.GetItemRect( 0, &rect ))
	{
		if(hitinfo.pt.y< rect.top+30 && hitinfo.pt.y >rect.top-30)
		{
			hitinfo.pt.y = rect.top;
		}
	}

	// Find the destination tab...
	unsigned int		nTab = m_ctlDLTabs.HitTest(&hitinfo);

	if (hitinfo.flags != TCHT_NOWHERE)
		return nTab;
	else
		return -1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	OnTcnSelchangeDltab() is the message handler for selection changes in the category tab
void CTransferWnd::OnTcnSelchangeDltab(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);

	m_ctlDownloadList.ChangeCategoryByIndex(m_ctlDLTabs.GetCurSel());
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	OnNMRClickDltab() is the right-click message handler for the tab control.
//	Popup menu for the category tabs
void CTransferWnd::OnNMRClickDltab(NMHDR *pNMHDR, LRESULT *pResult)
{
	static const uint16 s_auViewRes[] = {
		IDS_CAT_ALL,
		IDS_CAT_UNCATEGORIZED,
		0,
		IDS_CAT_INCOMPLETE,
		IDS_COMPLETE,
		IDS_WAITING,
		IDS_DOWNLOADING,
		IDS_ERRORLIKE,
		IDS_PAUSED,
		IDS_STOPPED,
		IDS_STALLED,
		IDS_ST_ACTIVE,
		IDS_ST_INACTIVE,
		0,
		IDS_VIDEO,
		IDS_AUDIO,
		IDS_SEARCH_ARC,
		IDS_SEARCH_CDIMG
	};
	CTitleMenu	menu;
	POINT		point;
	NOPRM(pNMHDR);

	::GetCursorPos(&point);

	CPoint		pt(point);
	CMenu		menuView, menuCat, menuPriority;

	menuPriority.CreateMenu();
	menuPriority.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	menuPriority.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	menuPriority.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	menuPriority.AppendMenu(MF_STRING, MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));

	menu.CreatePopupMenu();
	menu.AddMenuTitle(GetResString(IDS_CAT));

	m_iTabRightClickIndex = GetTabUnderMouse(&pt);

	if (m_iTabRightClickIndex == 0)
	{
		unsigned	ui, uiMenuID;
		CString		strRes;

		menuView.CreateMenu();
		for (uiMenuID = MP_CAT_SET_0, ui = 0; ui < ARRSIZE(s_auViewRes); ui++)
		{
			if (s_auViewRes[ui] != 0)
			{
				::GetResString(&strRes, s_auViewRes[ui]);
				menuView.AppendMenu(MF_STRING, uiMenuID++, strRes);
			}
			else
				menuView.AppendMenu(MF_SEPARATOR);
		}
		menuView.CheckMenuItem(MP_CAT_SET_0 + CCat::GetAllCatType() - CAT_PREDEFINED, MF_CHECKED | MF_BYCOMMAND);

		menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)menuView.m_hMenu, GetResString(IDS_CHANGECATVIEW));
	}

	int			iNumPredefinedCats = CCat::GetNumPredefinedCats();
	UINT		dwPredefinedCatActionFlag = (m_iTabRightClickIndex < iNumPredefinedCats) ? MF_GRAYED : MF_STRING;
	UINT		dwIsAllOrUserFlag = ((m_iTabRightClickIndex < iNumPredefinedCats) && !(m_iTabRightClickIndex == 0 && CCat::GetAllCatType() == CAT_ALL))? MF_GRAYED : MF_STRING;

	if (m_iTabRightClickIndex < iNumPredefinedCats)
	{
		menuCat.CreateMenu();
		for (int i = 0; i < CAT_TOTALPREDEFINEDCATS; i++)
		{
		//	Check to see if the cat has a tab and check the menu item if so
			int		catIndex = CCat::GetCatIndexByID(static_cast<_EnumCategories>(static_cast<int>(CAT_PREDEFINED) + i));
			UINT	checkFlag = (catIndex == -1) ? MF_UNCHECKED : MF_CHECKED;

			if (i == 0)
				checkFlag |= MF_GRAYED;

		//	Add the 'i'th predefined category title
			menuCat.AppendMenu( MF_STRING | checkFlag,
								  MP_SHOWPREDEFINEDCAT_0 + i,
								  CCat::GetPredefinedCatTitle(static_cast<_EnumCategories>(static_cast<int>(CAT_PREDEFINED) + i)) );

		//	Add a seperator after the last of the status categories.
			if (CAT_PREDEFINED + i == CAT_LASTSTATUSCAT)
			{
				menuCat.AppendMenu(MF_SEPARATOR);
			}
		}
		menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)menuCat.m_hMenu, GetResString(IDS_CAT_SHOWSTATUSTAB));
		menu.AppendMenu(MF_SEPARATOR);
	}

	menu.AppendMenu(MF_STRING,MP_CAT_ADD,GetResString(IDS_CAT_ADD));
	menu.AppendMenu(dwPredefinedCatActionFlag, MP_CAT_EDIT, GetResString(IDS_CAT_EDIT));
	menu.AppendMenu(dwPredefinedCatActionFlag, MP_CAT_REMOVE, GetResString(IDS_CAT_REMOVE));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(dwIsAllOrUserFlag|MF_POPUP, (UINT_PTR)menuPriority.m_hMenu, GetResString(IDS_PRIORITY));

	menu.AppendMenu(MF_STRING,MP_CANCEL,GetResString(IDS_MAIN_BTN_CANCEL));
	menu.AppendMenu(MF_STRING,MP_STOP, GetResString(IDS_STOP_VERB));
	menu.AppendMenu(MF_STRING,MP_PAUSE, GetResString(IDS_PAUSE_VERB));
	menu.AppendMenu(MF_STRING,MP_RESUME, GetResString(IDS_RESUME));
	menu.AppendMenu(MF_STRING,MP_RESUMENEXT, GetResString(IDS_DL_RESUMENEXT));

	menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

//	Menu objects are destroyed in their destructor

	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransferWnd::OnLvnBeginDrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iSel = m_ctlDownloadList.GetSelectionMark();

	if (iSel == -1)
		return;

	CMuleCtrlItem	   *pItem = reinterpret_cast<CMuleCtrlItem*>(m_ctlDownloadList.GetItemData(iSel));
	CPartFileDLItem	   *pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);

	if (pFileItem == NULL)
		return;

	m_bIsDragging = true;

	POINT pt;

	::GetCursorPos(&pt);

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	m_pDragImage = m_ctlDownloadList.CreateDragImage(m_ctlDownloadList.GetSelectionMark(),&pt);
    m_pDragImage->BeginDrag(0, CPoint(0,0));
    m_pDragImage->DragEnter(GetDesktopWindow(), pNMLV->ptAction);
    SetCapture();
	m_nTabDropIndex = -1;

	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransferWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if(!(nFlags & MK_LBUTTON))
		m_bIsDragging = false;

	if(m_bIsDragging)
	{
		CPoint pt(point);           //get our current mouse coordinates

		ClientToScreen(&pt);        //convert to screen coordinates

		m_nTabDropIndex = GetTabUnderMouse(&pt);
		m_ctlDLTabs.SetCurSel(m_nTabDropIndex);
		m_ctlDLTabs.Invalidate();

		::GetCursorPos(&pt);
		pt.y -= 10;

		m_pDragImage->DragMove(pt); //move the drag image to those coordinates
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransferWnd::OnLButtonUp(UINT uiFlags, CPoint point)
{
	NOPRM(uiFlags); NOPRM(point);
	if (m_bIsDragging)
	{
		ReleaseCapture();
		m_bIsDragging = false;

		m_pDragImage->DragLeave(GetDesktopWindow());
		m_pDragImage->EndDrag();
		delete m_pDragImage;

	//	If the cursor is over a tab and the current tab is a predefined cat
		if ( m_nTabDropIndex >= CCat::GetNumPredefinedCats()
		 && ( m_ctlDownloadList.GetCurTabIndex() < CCat::GetNumPredefinedCats()
		   || ( m_ctlDownloadList.GetCurTabIndex() >= CCat::GetNumPredefinedCats()
		     && m_nTabDropIndex != m_ctlDownloadList.GetCurTabIndex()) ) )
		{
			CPartFile	   *pPartFile;
			int				index = -1;
			POSITION		pos = m_ctlDownloadList.GetFirstSelectedItemPosition();

			while (pos != NULL)
			{
				index = m_ctlDownloadList.GetNextSelectedItem(pos);
				if (index > -1)
				{
					CMuleCtrlItem	*pItem = reinterpret_cast<CMuleCtrlItem*>(m_ctlDownloadList.GetItemData(index));
					CPartFileDLItem	*pFileItem = dynamic_cast<CPartFileDLItem*>(pItem);

					if (pFileItem != NULL)
					{
						pPartFile = pFileItem->GetFile();
						pPartFile->SetCatID(CCat::GetCatIDByIndex(m_nTabDropIndex));
					}
				}
			}

			m_ctlDLTabs.SetCurSel(m_ctlDownloadList.GetCurTabIndex());

			//if (m_ctlDLTabs.GetCurSel() > CCat::GetNumPredefinedCats())
			m_ctlDownloadList.ChangeCategoryByIndex(m_ctlDLTabs.GetCurSel());

			UpdateCatTabTitles();
		}
		else
		{
			m_ctlDLTabs.SetCurSel(m_ctlDownloadList.GetCurTabIndex());
		}
		m_ctlDownloadList.Invalidate();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransferWnd::OnDblClickDltab()
{
	CPoint	point;

	::GetCursorPos(&point);

	int		iTab = GetTabUnderMouse(&point);

	if (iTab >= CCat::GetNumPredefinedCats())
	{
		m_iTabRightClickIndex = iTab;
		OnCommand(MP_CAT_EDIT, 0);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	UpdateCatTabTitles() updates the tab text to reflect the current category titles and, incidentally,
//		updates the file count information displayed on the tab.
void CTransferWnd::UpdateCatTabTitles()
{
	if (!g_App.m_pMDlg->IsRunning())
		return;

	CString		strCatTitle;

//	For each tab...
	for (int i = 0; i < m_ctlDLTabs.GetItemCount(); i++)
	{
		if (i == 0)
		{
			EnumCategories		eCatID = CCat::GetAllCatType();

			strCatTitle = CCat::GetPredefinedCatTitle(eCatID);
			if (eCatID != CAT_ALL)
			{
				strCatTitle = _T('[') + strCatTitle;
				strCatTitle += _T(']');
			}
		}
		else
		{
			strCatTitle = CCat::GetCatByIndex(i)->GetTitle();
		}
	//	If it's the "All" tab change the title to reflect the current All category selection.
	//	If not, change the title to reflect the current category title
		EditCatTabLabel(i, strCatTitle);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	EditCatTabLabel() changes the label of tab 'index' to 'strNewLabel' (with additional file count info if the
//		"Show Category Tab Info" preference is set)
void CTransferWnd::EditCatTabLabel(int index, CString strNewLabel)
{
	TCITEM tabitem;

	tabitem.mask = TCIF_PARAM;
	m_ctlDLTabs.GetItem(index,&tabitem);
	tabitem.mask = TCIF_TEXT;

	CDownloadList::PartFileVector  *pvecPartFiles;
	uint32							dwTotalCount = 0;
	uint32							dwDownloading = 0;

//	Count the number of files in the DownloadQueue belonging to the
//	'index'th category and the number of those currently downloading
	if ((pvecPartFiles = g_App.m_pDownloadList->GetFiles()) != NULL)
	{
		for(unsigned int i = 0; i < pvecPartFiles->size(); i++)
		{
			CPartFile		*pPartFile = (*pvecPartFiles)[i];

			if (pPartFile == NULL)
				continue;

			if (CCat::FileBelongsToGivenCat(pPartFile,CCat::GetCatIDByIndex(index)))
			{
				dwTotalCount++;
				if(pPartFile->GetTransferringSrcCount() > 0)
					dwDownloading++;
			}
		}
	}

	delete pvecPartFiles;
	pvecPartFiles = NULL;

//	Append the appropriate information to the tab label

//	KuSh: what about not displaying dwDownloading if null ?
	if (!CCat::GetCatByIndex(index)->IsPredefined()/* && dwDownloading > 0*/)
	{
		strNewLabel.AppendFormat(_T(" %u/%u"), dwDownloading, dwTotalCount);
	}
	else
	{
	//	We don't really need a download count on predefined cats
		strNewLabel.AppendFormat(_T(" %u"), dwTotalCount);
	}

	tabitem.pszText = strNewLabel.LockBuffer();
	m_ctlDLTabs.SetItem(index,&tabitem);
	strNewLabel.UnlockBuffer();
	m_ctlDLTabs.Invalidate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CTransferWnd::AddCategory( CString strNewTitle, CString strNewIncoming, CString strNewTemp, 
							   CString strNewComment, CString strNewAutoCatExt, bool bAddTab)
{
	CCat		*pNewCat = new CCat(strNewTitle, strNewIncoming, strNewTemp, strNewComment, strNewAutoCatExt);

	int		iIndex = CCat::AddCat(pNewCat);

	if (bAddTab)
		m_ctlDLTabs.InsertItem(iIndex,strNewTitle);

	m_ctlDLTabs.Invalidate();

	return iIndex;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CTransferWnd::AddPredefinedCategory(EnumCategories ePredefinedCatID, bool bAddTab/*=true*/)
{
	CCat	*pNewCat = new CCat(ePredefinedCatID);

	int		iIndex = CCat::AddPredefinedCat(pNewCat);

	if (bAddTab)
		m_ctlDLTabs.InsertItem(iIndex, pNewCat->GetTitle());

	m_ctlDLTabs.Invalidate();

	return iIndex;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTransferWnd::OnTabMovement(NMHDR *pNMHDR, LRESULT *pResult)
{
	UINT	from = m_ctlDLTabs.GetLastMovementSource();
	UINT	to = m_ctlDLTabs.GetLastMovementDestination();
	byte	iNumPredefinedCats = CCat::GetNumPredefinedCats();
	NOPRM(pNMHDR); NOPRM(pResult);

	if (from < iNumPredefinedCats || to < iNumPredefinedCats || from == to-1)
		return;
//
//	Do the reorder
//

//	Rearrange the cat-map
	if (!CCat::MoveCat(from, to))
		return;

//	Rearrange the tab control itself
	m_ctlDLTabs.ReorderTab(from, to);

	UpdateCatTabTitles();
	g_App.m_pMDlg->m_dlgSearch.UpdateCatTabs();

	if (to > from)
	{
		--to;
	}
	m_ctlDLTabs.SetCurSel(to);
	m_ctlDownloadList.ChangeCategoryByIndex(to);
	m_ctlDLTabs.Invalidate();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CTransferWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;

	if (pNMHDR->hwndFrom == m_ctlDLTabs.GetSafeHwnd())
	{
		switch(pNMHDR->code)
		{
			case NM_RCLICK:
				OnNMRClickDltab(pNMHDR, pResult);
				break;
			case NM_DBLCLK:
			case WM_LBUTTONDBLCLK:
				OnDblClickDltab();
				break;
			case TCN_SELCHANGE:
				OnTcnSelchangeDltab(pNMHDR, pResult);
				break;
			case NM_TABMOVED:
				OnTabMovement(pNMHDR, pResult);
				break;
		}
	}
	else if(pNMHDR->hwndFrom == m_ctlDownloadList.GetSafeHwnd())
	{
		switch(pNMHDR->code)
		{
			case LVN_BEGINDRAG:
				OnLvnBeginDrag(pNMHDR, pResult);
				break;
		}
	}
	else
	{
		switch(pNMHDR->code)
		{
			case UDM_TOOLTIP_DISPLAY:
			{
				NM_PPTOOLTIP_DISPLAY *pNotify = (NM_PPTOOLTIP_DISPLAY*)lParam;

				GetInfo4ToolTip(pNotify);
				return TRUE;
			}
			case UDM_TOOLTIP_POP:
			{
				m_ttip.Pop();
				return TRUE;
			}
		}
	}

	return CResizableDialog::OnNotify(wParam, lParam, pResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
