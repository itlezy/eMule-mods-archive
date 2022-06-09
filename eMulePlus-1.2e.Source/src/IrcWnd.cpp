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
#include "IrcWnd.h"
#include "emule.h"
#include "otherfunctions.h"
#include "opcodes.h"
#include "ColourPopup.h"
#include "TitleMenu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

static const COLORREF _acrColorPal[16] =
{
	RGB(255, 255, 255), RGB(0, 0, 0), RGB(0, 0, 127), RGB(0, 127, 0),
	RGB(255, 0, 0), RGB(127, 0, 0), RGB(156, 0, 156), RGB(252, 127, 0),
	RGB(255, 255, 0), RGB(0, 252, 0), RGB(0, 147, 147), RGB(0, 255, 255),
	RGB(0, 0, 252), RGB(255, 0, 255), RGB(127, 127, 127), RGB(210, 210, 210)
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//	CIrcWnd dialog
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CIrcWnd, CDialog)
CIrcWnd::CIrcWnd(CWnd* pParent /*=NULL*/)
		: CResizableDialog(CIrcWnd::IDD, pParent)
{
	m_pIrcMain = NULL;
	m_eConnectStatus = IRC_DISCONNECTED;
	m_bLoggedIn = false;
	m_pCurrentChannel = NULL;
	memset(&m_bSortAscendingChanList, true, sizeof(m_bSortAscendingChanList));
	serverChannelList.SetGeneralPurposeFind(true);
	m_ctlNickList.SetParent(this);
}

CIrcWnd::~CIrcWnd()
{
	if (!IsDisconnected())
		m_pIrcMain->Disconnect(true);

	POSITION	pos1, pos2;

	for (pos1 = channelLPtrList.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		ChannelList* cur_channel = (ChannelList*)channelLPtrList.GetNext(pos1);
		channelLPtrList.RemoveAt(pos2);
		delete cur_channel;
	}
	DeleteAllChannel();
	delete m_pIrcMain;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CIrcWnd::Localize()
{
	static const uint16 s_auResTbl[] =
	{
		IDS_IRC_NAME,
		IDS_UUSERS,
		IDS_DESCRIPTION
	};

	SetDlgItemText(IDC_BN_IRCCONNECT, GetResString((!IsDisconnected()) ? IDS_IRC_DISCONNECT : IDS_IRC_CONNECT));
	SetDlgItemText(IDC_CHATSEND, GetResString(IDS_IRC_SEND));
	SetDlgItemText(IDC_CLOSECHAT, GetResString(IDS_CW_CLOSE));

	TCITEM		tcitem;
	Channel		*pCurChannel;

	for (int i = 0; i < channelselect.GetItemCount();i++)
	{
		tcitem.mask = TCIF_PARAM;
		tcitem.lParam = -1;
		channelselect.GetItem(i, &tcitem);
		pCurChannel = (Channel*)tcitem.lParam;
		if (pCurChannel != NULL)
		{
			if (pCurChannel->type == 1)
			{
				GetResString(&pCurChannel->title, IDS_STATUS);
				tcitem.mask = TCIF_TEXT;
				tcitem.pszText = const_cast<LPTSTR>(pCurChannel->title.GetString());
				channelselect.SetItem(i, &tcitem);
			}
			else if (pCurChannel->type == 2)
			{
				GetResString(&pCurChannel->title, IDS_IRC_CHANNELLIST);
				tcitem.mask = TCIF_TEXT;
				tcitem.pszText = const_cast<LPTSTR>(pCurChannel->title.GetString());
				channelselect.SetItem(i, &tcitem);
			}
		}
	}
	m_ctlNickList.Localize();

	if (m_pCurrentChannel != NULL)
	{
		titleWindow.Reset();
		if (m_pCurrentChannel->type == 1)
			titleWindow.AppendText(GetResString(IDS_STATUS), RGB(0, 0, 128), CLR_DEFAULT, HTC_HAVENOLINK);
		else if(m_pCurrentChannel->type == 2)
			titleWindow.AppendText(GetResString(IDS_IRC_CHANNELLIST), RGB(0, 0, 128), CLR_DEFAULT, HTC_HAVENOLINK);
	}

	CHeaderCtrl	*pHeaderCtrl = serverChannelList.GetHeaderCtrl();
	CString		strRes;
	HDITEM		hdi;

	hdi.mask = HDI_TEXT;
	for (unsigned ui = 0; ui < ARRSIZE(s_auResTbl); ui++)
	{
		::GetResString(&strRes, static_cast<UINT>(s_auResTbl[ui]));
		hdi.pszText = const_cast<LPTSTR>(strRes.GetString());
		pHeaderCtrl->SetItem(static_cast<int>(ui), &hdi);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CIrcWnd::OnInitDialog()
{
	static const uint16 s_auColHdr2[][2] =
	{
		{ LVCFMT_LEFT, 203 },	// IRC2COL_NAME
		{ LVCFMT_RIGHT, 50 },	// IRC2COL_USERS
		{ LVCFMT_LEFT, 350 }	// IRC2COL_DESCRIPTION
	};

	CResizableDialog::OnInitDialog();

	CRect			rcRect;

	for (unsigned ui = 0; ui < ARRSIZE(s_auColHdr2); ui++)
		serverChannelList.InsertColumn(ui, _T(""), static_cast<int>(s_auColHdr2[ui][0]), static_cast<int>(s_auColHdr2[ui][1]));
	serverChannelList.LoadSettings(CPreferences::TABLE_IRC);

	m_hOldIcon = GetUserLstStatic()->SetIcon(GetUserLstIcon());
	m_ctlNickList.Init();

	Localize();
	m_pIrcMain = new CIrcMain();
	m_pIrcMain->SetIRCWnd(this);

	NewChannel(GetResString(IDS_STATUS), 1);
	NewChannel(GetResString(IDS_IRC_CHANNELLIST), 2);

	serverChannelList.GetWindowRect(&rcRect);
	ScreenToClient(&rcRect);
	rcRect.bottom = rcRect.top + 18;
	titleWindow.Create(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_AUTOHSCROLL | ES_READONLY | ES_NOHIDESEL, rcRect, this, (UINT)-1);
	titleWindow.ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
	titleWindow.m_dwFlags = HTC_ISLIMITED | HTC_ISDEFAULTLINKS;

	UpdateFont();
	m_pCurrentChannel = (Channel*)channelPtrList.GetTail();
	AddMessage(_T(""), _T(""), CLR_DEFAULT, GetResString(IDS_IRC_STATUSLOG));
	channelselect.SetCurSel(2);
	channelselect.SetCurFocus(2);
	OnTcnSelchangeTab2(NULL, NULL);
	channelselect.SetItemState(1, TCIS_HIGHLIGHTED, NULL);

	CheckDlgButton(IDC_UTF8_ENCODING, g_App.m_pPrefs->GetIrcMessageEncodingUTF8() ? BST_CHECKED : BST_UNCHECKED);

	AddAnchor(IDC_BN_IRCCONNECT, BOTTOM_LEFT);
	AddAnchor(IDC_CLOSECHAT, BOTTOM_LEFT);
	AddAnchor(IDC_CHATSEND, BOTTOM_RIGHT);
	AddAnchor(IDC_UTF8_ENCODING, BOTTOM_LEFT);
	AddAnchor(IDC_INPUTWINDOW, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_TEXTBOLD, BOTTOM_RIGHT);
	AddAnchor(IDC_TEXTUNDERLINE, BOTTOM_RIGHT);
	AddAnchor(IDC_TEXTCOLOR, BOTTOM_RIGHT);
	AddAnchor(IDC_NICKLIST, TOP_LEFT, BOTTOM_LEFT);
	AddAnchor(IDC_SERVERCHANNELLIST, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_TAB2, TOP_LEFT, TOP_RIGHT);

	m_bSortAscendingChanList[IRC2COL_USERS] = false;

	if (g_App.m_pPrefs->DoUseSort())
		SortInit(g_App.m_pPrefs->GetIrcSortCol());
	else
	{
		int		iSortCode = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_IRC);

		iSortCode |= g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_IRC) ? MLC_SORTASC : MLC_SORTDESC;
		SortInit(iSortCode);
	}

	GetDlgItem(IDC_CLOSECHAT)->EnableWindow(false);
	GetDlgItem(IDC_CHATSEND)->EnableWindow(false);

	m_ctrlTextBoldBtn.SetThemeHelper(&g_App.m_pMDlg->m_themeHelper);
	m_ctrlTextBoldBtn.SetIcon(IDI_TEXTBOLD);
	m_ctrlTextUnderlineBtn.SetThemeHelper(&g_App.m_pMDlg->m_themeHelper);
	m_ctrlTextUnderlineBtn.SetIcon(IDI_TEXTUNDERLINE);
	m_ctrlTextColorBtn.SetThemeHelper(&g_App.m_pMDlg->m_themeHelper);
	m_ctrlTextColorBtn.SetIcon(IDI_TEXTCOLOR);

	return true;
}

void CIrcWnd::OnDestroy()
{
	CResizableDialog::OnDestroy();

	DestroyIcon(GetUserLstStatic()->SetIcon(m_hOldIcon));
}

void CIrcWnd::UpdateFont()
{
	TCITEM		tcitem;
	Channel		*pChannel;
	int			i = 0;

	tcitem.mask = TCIF_PARAM;
	while (channelselect.GetItem(i++, &tcitem))
	{
		pChannel = (Channel*)tcitem.lParam;
		if (pChannel->log.m_hWnd != NULL)
			pChannel->log.SetFont(&g_App.m_pMDlg->m_fontDefault);
	}
	titleWindow.SetFont(&g_App.m_pMDlg->m_fontDefault);
	inputWindow.SetFont(&g_App.m_pMDlg->m_fontDefault);
}

void CIrcWnd::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

	if (titleWindow.m_hWnd)
	{
		CRect		rcRect;

		serverChannelList.GetWindowRect(&rcRect);
		ScreenToClient(&rcRect);
		titleWindow.SetWindowPos(NULL, rcRect.left, rcRect.top, rcRect.Width(), 18, SWP_NOZORDER);
	}

	if (m_pCurrentChannel != NULL && m_pCurrentChannel->log.m_hWnd)
	{
		CRect		rcRect;

		serverChannelList.GetWindowRect(&rcRect);
		ScreenToClient(&rcRect);

		if (m_pCurrentChannel->type == 4)
			rcRect.top += 18;

		m_pCurrentChannel->log.SetWindowPos(NULL, rcRect.left, rcRect.top, rcRect.Width(), rcRect.Height(), SWP_NOZORDER);
	}
}

int CIrcWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CResizableDialog::OnCreate(lpCreateStruct);
}

void CIrcWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NICKLIST, m_ctlNickList);
	DDX_Control(pDX, IDC_INPUTWINDOW, inputWindow);
	DDX_Control(pDX, IDC_TEXTBOLD, m_ctrlTextBoldBtn);
	DDX_Control(pDX, IDC_TEXTUNDERLINE, m_ctrlTextUnderlineBtn);
	DDX_Control(pDX, IDC_TEXTCOLOR, m_ctrlTextColorBtn);
	DDX_Control(pDX, IDC_SERVERCHANNELLIST, serverChannelList);
	DDX_Control(pDX, IDC_TAB2, channelselect);
}

BOOL CIrcWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);

	switch (wParam)
	{
		case IDC_BN_IRCCONNECT:
			OnBnClickedBnIrcconnect();
			return true;

		case IDC_UTF8_ENCODING:
			g_App.m_pPrefs->SetIrcMessageEncodingUTF8(IsDlgButtonChecked(IDC_UTF8_ENCODING) == BST_CHECKED);
			return true;

		case IDC_TEXTBOLD:
			OnBnClickedTextBold();
			return true;

		case IDC_TEXTUNDERLINE:
			OnBnClickedTextUnderline();
			return true;

		case IDC_TEXTCOLOR:
			OnBnClickedTextColor();
			return true;

		case IDC_CHATSEND:
			OnBnClickedChatsend();
			return true;

		case IDC_CLOSECHAT:
			OnBnClickedClosechat();
			return true;

		case Irc_Join:
			JoinChannels();
			return true;

		case Irc_Refresh:
			m_pIrcMain->SendString(_T("LIST"));
			return true;
	}
	return true;
}

BOOL CIrcWnd::PreTranslateMessage(MSG* pMsg)
{
	EMULE_TRY

	if (pMsg->message == WM_KEYDOWN && (pMsg->hwnd == inputWindow.m_hWnd))
	{
		if (pMsg->wParam == VK_RETURN)
		{
			OnBnClickedChatsend();
			return TRUE;
		}

		if (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN)
		{
			ScrollHistory(pMsg->wParam == VK_DOWN);
			return TRUE;
		}
	}

	return CResizableDialog::PreTranslateMessage(pMsg);

	EMULE_CATCH

	return false;
}

void CIrcWnd::OnBnClickedBnIrcconnect()
{
	if (IsDisconnected())
		m_pIrcMain->Connect();
	else
		m_pIrcMain->Disconnect();
	inputWindow.SetFocus();
}

void CIrcWnd::OnBnClickedClosechat(int nItem)
{
	TCITEM		tcitem;

	tcitem.mask = TCIF_PARAM;
	if (nItem == -1)
		nItem = channelselect.GetCurSel();

	if (nItem == -1)
		return;

	channelselect.GetItem(nItem, &tcitem);

	Channel		*pPartChannel = (Channel*)tcitem.lParam;

	if (pPartChannel != NULL)
	{
		if (pPartChannel->type == 4 && IsConnected())
		{
			CString		strPart = _T("PART ");

			strPart += pPartChannel->name;
			m_pIrcMain->SendString(strPart);
			return;
		}
		else if (pPartChannel->type == 5 || pPartChannel->type == 4)
		{
			RemoveChannel(pPartChannel->name);
			return;
		}
	}
}

void CIrcWnd::OnTcnSelchangeTab2(NMHDR *pNMHDR, LRESULT *pResult)
{
	int		iCurSel;
	TCITEM	tcitem;
	NOPRM(pNMHDR);

	m_ctlNickList.DeleteAllItems();

	tcitem.mask = TCIF_PARAM;

	if ((iCurSel = channelselect.GetCurSel()) == -1)
		return;

	if (!channelselect.GetItem(iCurSel, &tcitem))
		return;

	Channel		*pChannel = (Channel*)tcitem.lParam;

	if (pChannel == NULL)
		return;

	m_pCurrentChannel = pChannel;

	int			i;
	bool		bWasChanged = (channelselect.GetItemState(iCurSel, TCIS_HIGHLIGHTED) & TCIS_HIGHLIGHTED) ? true : false;

	m_ctlNickList.UpdateNickCount();
	channelselect.SetItemState(iCurSel, TCIS_HIGHLIGHTED, NULL);
	titleWindow.ShowWindow(SW_HIDE);
	if (m_pCurrentChannel->type == 1)
		GetDlgItem(IDC_CLOSECHAT)->EnableWindow(false);
	else if (m_pCurrentChannel->type == 2)
	{
		serverChannelList.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CLOSECHAT)->EnableWindow(false);
		tcitem.mask = TCIF_PARAM;
		i = 0;
		while (channelselect.GetItem(i++, &tcitem))
		{
			pChannel = (Channel*)tcitem.lParam;
			if (pChannel != m_pCurrentChannel && pChannel->log.m_hWnd != NULL)
				pChannel->log.ShowWindow(SW_HIDE);
		}
		return;
	}
	else
		GetDlgItem(IDC_CLOSECHAT)->EnableWindow(true);
	SetActivity(m_pCurrentChannel->name);

	CRect		rcChannel;

	serverChannelList.GetWindowRect(&rcChannel);
	ScreenToClient(&rcChannel);
	if (m_pCurrentChannel->type == 4)
	{
		rcChannel.top += 18;
		titleWindow.ShowWindow(SW_SHOW);
		SetTitle(m_pCurrentChannel->name, m_pCurrentChannel->title);
	}
	m_pCurrentChannel->log.SetWindowPos(NULL, rcChannel.left, rcChannel.top, rcChannel.Width(), rcChannel.Height(), SWP_NOZORDER);
	m_pCurrentChannel->log.SetRedraw(false);
	m_pCurrentChannel->log.ShowWindow(SW_SHOW);
	if ((m_pCurrentChannel->log.m_dwFlags & HTC_ISAUTOSCROLL) != 0 && bWasChanged)
		m_pCurrentChannel->log.ScrollToLastLine();
	m_pCurrentChannel->log.SetRedraw(true);

	tcitem.mask = TCIF_PARAM;
	i = 0;
	while (channelselect.GetItem(i++, &tcitem))
	{
		pChannel = (Channel*)tcitem.lParam;
		if (pChannel != m_pCurrentChannel && pChannel->log.m_hWnd != NULL)
			pChannel->log.ShowWindow(SW_HIDE);
	}
	serverChannelList.ShowWindow(SW_HIDE);
	m_ctlNickList.RefreshNickList(m_pCurrentChannel->name);
	m_pCurrentChannel->log.SetTitle(m_pCurrentChannel->name);
	inputWindow.SetFocus();
	if (pResult != NULL)
		*pResult = 0;
}

BEGIN_MESSAGE_MAP(CIrcWnd, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_SERVERCHANNELLIST, OnNMDblclkserverChannelList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVERCHANNELLIST, OnColumnClickChanL)
	ON_NOTIFY(NM_RCLICK, IDC_SERVERCHANNELLIST, OnNMRclickChanL)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB2, OnTcnSelchangeTab2)
	ON_MESSAGE(WM_CLOSETAB, OnCloseTab)
	ON_MESSAGE(CPN_SELENDOK, OnColorSelEndOK)
	ON_MESSAGE(CPN_SELENDCANCEL, OnColorSelEndCancel)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//	Channel List
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void CIrcWnd::ResetServerChannelList()
{
	POSITION		pos1, pos2;
	ChannelList		*pCurChannel;

	for (pos1 = channelLPtrList.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		pCurChannel = (ChannelList*)channelLPtrList.GetNext(pos1);
		channelLPtrList.RemoveAt(pos2);
		delete pCurChannel;
	}
	serverChannelList.DeleteAllItems();
}

void CIrcWnd::AddChannelToList(CString name, CString user, CString description)
{
	CString		ntemp = name;
	CString		dtemp = description;
	int			iUserTest = _tstoi(user);

	if ((g_App.m_pPrefs->GetIRCChanNameFilter() || g_App.m_pPrefs->GetIRCChannelUserFilter()) && g_App.m_pPrefs->GetIRCUseChanFilter())
	{
		if (iUserTest < g_App.m_pPrefs->GetIRCChannelUserFilter())
			return;
		if (dtemp.MakeLower().Find(g_App.m_pPrefs->GetIRCChanNameFilter().MakeLower()) == -1 && ntemp.MakeLower().Find(g_App.m_pPrefs->GetIRCChanNameFilter().MakeLower()) == -1)
			return;
	}
	ChannelList		*pToAdd = new ChannelList;

	pToAdd->name = name;
	pToAdd->users = user;
	pToAdd->desc = description;

	int			iIndex = 0, iLen, iColorIndex;
	CString		strTemp;
	TCHAR		cCurChar;

	while (pToAdd->desc.GetLength() > iIndex)
	{
		cCurChar = pToAdd->desc[iIndex];
		switch (cCurChar)
		{
			case 0x03:
			{
				pToAdd->desc.Delete(iIndex);
				if (pToAdd->desc.GetLength() > iIndex)
				{
					if (pToAdd->desc[iIndex] >= _T('0') && pToAdd->desc[iIndex] <= _T('9'))
					{
						if (pToAdd->desc.GetLength() > iIndex)
						{
							iLen = 1;
							if (pToAdd->desc[iIndex + 1] >= _T('0') && pToAdd->desc[iIndex + 1] <= _T('9'))
								iLen++;
							iColorIndex = _tstoi(pToAdd->desc.Mid(iIndex, iLen));
							pToAdd->desc.Delete(iIndex, iLen);
							if (iColorIndex < 16)
							{
								strTemp.Format(_T("\001%06x"), _acrColorPal[iColorIndex]);
								pToAdd->desc.Insert(iIndex, strTemp);
								iIndex += strTemp.GetLength();
							}
							if (pToAdd->desc.GetLength() > iIndex && pToAdd->desc[iIndex] == _T(',') && pToAdd->desc[iIndex + 1] >= _T('0') && pToAdd->desc[iIndex + 1] <= _T('9'))
							{
								iLen = 1;
								if (pToAdd->desc.GetLength() > iIndex + 1 && pToAdd->desc[iIndex + 2] >= _T('0') && pToAdd->desc[iIndex + 2] <= _T('9'))
									iLen++;
								iColorIndex = _tstoi(pToAdd->desc.Mid(iIndex + 1, iLen));
								pToAdd->desc.Delete(iIndex, iLen + 1);
								if (iColorIndex < 16)
								{
									strTemp.Format(_T("\002%06x"), _acrColorPal[iColorIndex]);
									pToAdd->desc.Insert(iIndex, strTemp);
									iIndex += strTemp.GetLength();
								}
							}
						}
					}
					else
					{
						pToAdd->desc.Insert(iIndex, _T('\x03'));
						iIndex++;
					}
				}
				break;
			}
			case 0x0F:
			{
				pToAdd->desc.SetAt(iIndex, _T('\x03'));
				iIndex++;
				break;
			}
			default:
			{
				if ((TBYTE)cCurChar < _T(' '))
					pToAdd->desc.Delete(iIndex);
				else
					iIndex++;
			}
		}
	}

	channelLPtrList.AddTail(pToAdd);

	int	iItems = serverChannelList.InsertItem(LVIF_TEXT | LVIF_PARAM | LVIF_STATE,
		serverChannelList.GetItemCount(), pToAdd->name, LVIS_OVERLAYMASK, LVIS_OVERLAYMASK, 0, (LPARAM)pToAdd);

	serverChannelList.SetItemText(iItems, IRC2COL_USERS, pToAdd->users);
	serverChannelList.SetItemText(iItems, IRC2COL_DESCRIPTION, pToAdd->desc);
	if (channelselect.GetCurSel() != 1)
		channelselect.SetItemState(1, TCIS_HIGHLIGHTED, TCIS_HIGHLIGHTED);
}

int CIrcWnd::SortProcChanL(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	ChannelList		*pItem1 = (ChannelList*)lParam1;
	ChannelList		*pItem2 = (ChannelList*)lParam2;

	switch (lParamSort)
	{
		case IRC2COL_NAME:
			return CString(pItem1->name).CompareNoCase(pItem2->name);
		case IRC2COL_NAME + MLC_SORTDESC:
			return CString(pItem2->name).CompareNoCase(pItem1->name);
		case IRC2COL_USERS:
			return _tstoi(pItem1->users) - _tstoi(pItem2->users);
		case IRC2COL_USERS + MLC_SORTDESC:
			return _tstoi(pItem2->users) - _tstoi(pItem1->users);
		case IRC2COL_DESCRIPTION:
			return CString(pItem1->desc).CompareNoCase(pItem2->desc);
		case IRC2COL_DESCRIPTION + MLC_SORTDESC:
			return CString(pItem2->desc).CompareNoCase(pItem1->desc);
		default:
			return 0;
	}
}

void CIrcWnd::OnColumnClickChanL(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW		*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int				iSubItem = pNMListView->iSubItem;
	bool			bSortOrder = m_bSortAscendingChanList[iSubItem];

// Reverse sorting direction for the same column and keep the same if column was changed
	if ((serverChannelList.GetSortParam() & MLC_COLUMNMASK) == static_cast<uint32>(iSubItem))
		m_bSortAscendingChanList[iSubItem] = bSortOrder = !bSortOrder;

	serverChannelList.SetSortArrow(iSubItem, bSortOrder);
	serverChannelList.SortItems(SortProcChanL, iSubItem + ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));
	g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_IRC, iSubItem);
	g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_IRC, bSortOrder);
	*pResult = 0;
}

void CIrcWnd::SortInit(int iSortCode)
{
//	Get the sort column
	int		iSortColumn = (iSortCode & MLC_COLUMNMASK);
//	Get the sort order
	bool	bSortAscending = (iSortCode & MLC_SORTDESC) == 0;

	serverChannelList.SetSortArrow(iSortColumn, bSortAscending);
	serverChannelList.SortItems(SortProcChanL, iSortCode);
	m_bSortAscendingChanList[iSortColumn] = bSortAscending;
}

void CIrcWnd::OnNMRclickChanL(NMHDR *pNMHDR, LRESULT *pResult)
{
	CTitleMenu	menuChanL;
	POINT		point;
	NOPRM(pNMHDR);

	::GetCursorPos(&point);

	menuChanL.CreatePopupMenu();
	menuChanL.AddMenuTitle(GetResString(IDS_IRC_CHANNEL));
	menuChanL.AppendMenu(MF_STRING | ((serverChannelList.GetSelectionMark() != -1) ? MF_ENABLED : MF_GRAYED), Irc_Join, GetResString(IDS_IRC_JOIN));
	menuChanL.AppendMenu(MF_SEPARATOR);
	menuChanL.AppendMenu(MF_STRING | ((IsConnected()) ? MF_ENABLED : MF_GRAYED), Irc_Refresh, GetResString(IDS_CMT_REFRESH));
	menuChanL.SetDefaultItem(Irc_Join);
	menuChanL.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

//	Menu objects are destroyed in their destructor
	*pResult = 0;
}

void CIrcWnd::OnNMDblclkserverChannelList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	JoinChannels();
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//	Nick List
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void CIrcWnd::ChangeAllNick(const CString &strOldNick, const CString &strNewNick)
{
	Channel	*pCurChannel = FindChannelByName(strOldNick);

	if (pCurChannel != NULL)
	{
		pCurChannel->name = strNewNick;

		TCITEM	tcitem;
		int		i;

		tcitem.mask = TCIF_PARAM;
		tcitem.lParam = -1;
		for (i = 0; i < channelselect.GetItemCount();i++)
		{
			channelselect.GetItem(i, &tcitem);
			if (((Channel*)tcitem.lParam) == pCurChannel)
				break;
		}
		if (((Channel*)tcitem.lParam) != pCurChannel)
			return;
		tcitem.mask = TCIF_TEXT;
		tcitem.pszText = const_cast<LPTSTR>(strNewNick.GetString());
		channelselect.SetItem(i, &tcitem);
	}
	for (POSITION pos = channelPtrList.GetHeadPosition(); pos != NULL;)
	{
		pCurChannel = (Channel*)channelPtrList.GetNext(pos);
		if (m_ctlNickList.ChangeNick(pCurChannel->name, strOldNick, strNewNick) && !g_App.m_pPrefs->GetIrcIgnoreInfoMessage())
			AddMessageF(pCurChannel->name, _T(""), RGB(0, 147, 0), GetResString(IDS_IRC_NOWKNOWNAS), strOldNick, strNewNick);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//	Messages
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void CIrcWnd::AddMessage(const CString &strChannel, const TCHAR *pcNick, COLORREF crTextColor, const CString &strInputLine)
{
	Channel		*pUpdateChannel = NULL;
	int			iLen;

	if (!strChannel.IsEmpty())
	{
		pUpdateChannel = FindChannelByName(strChannel);
		if (pUpdateChannel == NULL)
			pUpdateChannel = NewChannel(strChannel, (strChannel.GetString()[0] == _T('#')) ? 4 : 5);
	}
	else
		pUpdateChannel = (Channel*)channelPtrList.GetHead();

	if (pUpdateChannel == NULL)
		return;

	CString		strTemp, strLine(strInputLine);

	if (strChannel.IsEmpty())
	{
		strTemp = m_pIrcMain->GetNick();
		iLen = strTemp.GetLength();
		if (iLen > 0 && (_tcsncmp(strLine, strTemp, iLen) == 0))
		{
			iLen++;
			if ((strLine.GetLength() >= iLen) && (strLine.GetString()[iLen] == _T(':')))
				iLen++;
			strLine.Delete(0, iLen);
		}
	}

	if (strLine.IsEmpty())
		return;

	COLORREF	crForegroundColor = crTextColor, crBackgroundColor = CLR_DEFAULT;
	int			iIndex = 0, iLastIndex = 0, iColorIndex;
	DWORD		dwFlags = 0;
	TCHAR		cCurChar;
	bool		bStripColor = false;

	if (g_App.m_pPrefs->GetIRCAddTimestamp())
	{
		COleDateTime		currentTime(COleDateTime::GetCurrentTime());

		pUpdateChannel->log.AppendText(currentTime.Format(_T("%X: ")), RGB(0, 0, 128), CLR_DEFAULT, HTC_HAVENOLINK);
	}

	if (!strChannel.IsEmpty() && (*pcNick != _T('\0')))
	{
		bStripColor = g_App.m_pPrefs->GetIrcStripColor();
		strTemp.Format(_T("<%s>"), pcNick);
		pUpdateChannel->log.AppendText(strTemp, (m_pIrcMain->GetNick() == pcNick) ? RGB(128, 0, 128) : RGB(51, 51, 153));
	}

	while (strLine.GetLength() > iIndex)
	{
		cCurChar = strLine[iIndex];
		switch (cCurChar)
		{
			case 0x02:	//toggle bold
				if (iIndex > iLastIndex)
					pUpdateChannel->log.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
				iIndex++;
				iLastIndex = iIndex;
				dwFlags ^= HTC_BOLD;
				break;

			case 0x03:	//color
				if (iIndex > iLastIndex)
					pUpdateChannel->log.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
				iIndex++;

				if (strLine.GetLength() > iIndex && strLine[iIndex] >= _T('0') && strLine[iIndex] <= _T('9'))
				{
					iLen = 1;

					if (strLine.GetLength() > iIndex + 1 && strLine[iIndex + 1] >= _T('0') && strLine[iIndex + 1] <= _T('9'))
						iLen++;

					if (!bStripColor)
					{
						iColorIndex = _tstoi(strLine.Mid(iIndex, iLen));
						crForegroundColor = (iColorIndex < ARRSIZE(_acrColorPal)) ? _acrColorPal[iColorIndex] : crTextColor;
					}
					iIndex += iLen;

					if (strLine.GetLength() > iIndex + 1 && strLine[iIndex] == _T(',') && strLine[iIndex + 1] >= _T('0') && strLine[iIndex + 1] <= _T('9'))
					{
						iLen = 2;

						if (strLine.GetLength() > iIndex + 2 && strLine[iIndex + 2] >= _T('0') && strLine[iIndex + 2] <= _T('9'))
							iLen++;

						if (!bStripColor)
						{
							iColorIndex = _tstoi(strLine.Mid(iIndex + 1, iLen));
							crBackgroundColor = (iColorIndex < ARRSIZE(_acrColorPal)) ? _acrColorPal[iColorIndex]: CLR_DEFAULT;
						}
						iIndex += iLen;
					}
				}
				else
				{
					crForegroundColor = crTextColor;
					crBackgroundColor = CLR_DEFAULT;
				}
				iLastIndex = iIndex;
				break;

			case 0x0F:	//normal
				if (crForegroundColor != crTextColor || crBackgroundColor != CLR_DEFAULT || dwFlags != 0)
				{
					if (iIndex > iLastIndex)
						pUpdateChannel->log.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
					iIndex++;
					iLastIndex = iIndex;
					crForegroundColor = crTextColor;
					crBackgroundColor = CLR_DEFAULT;
					dwFlags = 0;
				}
				else
					strLine.Delete(iIndex);
				break;

			case 0x1F:	//toggle underline
				if (iIndex > iLastIndex)
					pUpdateChannel->log.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
				iIndex++;
				iLastIndex = iIndex;
				dwFlags ^= HTC_UNDERLINE;
				break;

			default:
				if (cCurChar < _T(' '))
					strLine.Delete(iIndex);
				else
					iIndex++;
		}
	}
	if (iLastIndex < iIndex)
		pUpdateChannel->log.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
	pUpdateChannel->log.AppendText(_T("\n"), CSTRLEN(_T("\n")), CLR_DEFAULT, CLR_DEFAULT, HTC_HAVENOLINK);

	if (m_pCurrentChannel != pUpdateChannel)
		SetActivity(pUpdateChannel->name);
}

void CIrcWnd::AddMessageF(const CString &strChannel, const TCHAR *pcNick, COLORREF crTextColor, const TCHAR *pcLine, ...)
{
	va_list		argptr;
	CString		strTmp;

	va_start(argptr, pcLine);
	strTmp.FormatV(pcLine, argptr);
	va_end(argptr);
	AddMessage(strChannel, pcNick, crTextColor, strTmp);
}

void CIrcWnd::SetConnectStatus(EnumConnectStatus eConnectStatus)
{
	switch (eConnectStatus)
	{
		case IRC_CONNECTING:
		case IRC_CONNECTED:
			SetDlgItemText(IDC_BN_IRCCONNECT, GetResString(IDS_IRC_DISCONNECT));
			AddMessage(_T(""), _T(""), CLR_DEFAULT, GetResString((eConnectStatus == IRC_CONNECTED) ? IDS_CONNECTED : IDS_CONNECTING));
			break;
		case IRC_DISCONNECTED:
		{
			SetDlgItemText(IDC_BN_IRCCONNECT, GetResString(IDS_IRC_CONNECT));
			AddMessage(_T(""), _T(""), CLR_DEFAULT, GetResString(IDS_DISCONNECTED));
			m_bLoggedIn = false;
			ResetServerChannelList();

			Channel	*pToDel;

			while (channelPtrList.GetCount() > 2)
			{
				pToDel = (Channel*)channelPtrList.GetTail();
				RemoveChannel(pToDel->name);
			}
			GetDlgItem(IDC_CLOSECHAT)->EnableWindow(false);
			GetDlgItem(IDC_CHATSEND)->EnableWindow(false);
			break;
		}
	}
	m_eConnectStatus = eConnectStatus;
}

void CIrcWnd::NoticeMessage(CString source, CString message)
{
	if (source.IsEmpty())
	{
		CString		strLine = message, strChannel;

		if (m_pCurrentChannel->type == 4 || m_pCurrentChannel->type == 5)
		{
			CString		strTemp = m_pIrcMain->GetNick();
			int			iLen = strTemp.GetLength();

			if (iLen > 0 && strLine.Left(iLen) == strTemp)
			{
				if (strLine.Mid(iLen + 1, 1) == _T(":"))
					iLen ++;
				strLine = strLine.Right(strLine.GetLength() - (iLen + 1));
			}
			strChannel = m_pCurrentChannel->name;
		}
		AddMessage(strChannel, _T(""), RGB(255, 80, 80), strLine);
		return;
	}

	bool		bFlag = false;
	Channel		*pCurChannel = FindChannelByName(source);
	Nick		*pCurNick;

	if (pCurChannel != NULL)
	{
		AddMessageF(source, _T(""), RGB(153, 51, 102), _T("-%s- %s"), source, message);
		bFlag = true;
	}

	for (POSITION pos = channelPtrList.GetHeadPosition(); pos != NULL;)
	{
		pCurChannel = (Channel*)channelPtrList.GetNext(pos);
		pCurNick = m_ctlNickList.FindNickByName(pCurChannel->name, source);
		if (pCurNick != NULL)
		{
			AddMessageF(pCurChannel->name, _T(""), RGB(153, 51, 102), _T("-%s- %s"), source, message);
			bFlag = true;
		}
	}

	if (!bFlag)
		AddMessageF((m_pCurrentChannel->type == 4 || m_pCurrentChannel->type == 5) ? m_pCurrentChannel->name : _T(""), _T(""), RGB(153, 51, 102), _T("-%s- %s"), source, message);
}

void CIrcWnd::SetTitle(CString channel, CString title)
{
	Channel		*pCurChannel = FindChannelByName(channel);

	if (pCurChannel == NULL)
		return;

	pCurChannel->title = title;

	if (pCurChannel != m_pCurrentChannel || m_pCurrentChannel->type != 4)
		return;

	titleWindow.Reset();

	CString		strLine = pCurChannel->title;

	if (strLine.IsEmpty())
		return;

	COLORREF	crForegroundColor = CLR_DEFAULT, crBackgroundColor = CLR_DEFAULT;
	int			iIndex = 0, iLastIndex = 0, iColorIndex, iLen;
	DWORD		dwFlags = 0;
	TCHAR		cCurChar;

	while (strLine.GetLength() > iIndex)
	{
		cCurChar = strLine[iIndex];
		switch (cCurChar)
		{
			case 0x02:
			{
				if (iIndex > iLastIndex)
					titleWindow.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
				iIndex++;
				iLastIndex = iIndex;
				dwFlags ^= HTC_BOLD;
				break;
			}
			case 0x03:
			{
				if (iIndex > iLastIndex)
					titleWindow.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
				iIndex++;

				if (strLine.GetLength() > iIndex && strLine[iIndex] >= _T('0') && strLine[iIndex] <= _T('9'))
				{
					iLen = 1;

					if (strLine.GetLength() > iIndex + 1 && strLine[iIndex + 1] >= _T('0') && strLine[iIndex + 1] <= _T('9'))
						iLen++;

					iColorIndex = _tstoi(strLine.Mid(iIndex, iLen));
					crForegroundColor = (iColorIndex < 16) ? _acrColorPal[iColorIndex] : CLR_DEFAULT;
					iIndex += iLen;

					if (strLine.GetLength() > iIndex + 1 && strLine[iIndex] == _T(',') && strLine[iIndex + 1] >= _T('0') && strLine[iIndex + 1] <= _T('9'))
					{
						iLen = 1;

						if (strLine.GetLength() > iIndex + 2 && strLine[iIndex + 2] >= _T('0') && strLine[iIndex + 2] <= _T('9'))
							iLen++;

						iColorIndex = _tstoi(strLine.Mid(iIndex + 1, iLen));
						crBackgroundColor = (iColorIndex < 16) ? _acrColorPal[iColorIndex]: CLR_DEFAULT;
						iIndex += 1 + iLen;
					}
				}
				else
				{
					crForegroundColor = CLR_DEFAULT;
					crBackgroundColor = CLR_DEFAULT;
				}
				iLastIndex = iIndex;
				break;
			}
			case 0x0F:
			{
				if (iIndex > iLastIndex)
					titleWindow.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
				iIndex++;
				iLastIndex = iIndex;
				crForegroundColor = CLR_DEFAULT;
				crBackgroundColor = CLR_DEFAULT;
				dwFlags = 0;
				break;
			}
			case 0x1F:
			{
				if (iIndex > iLastIndex)
					titleWindow.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
				iIndex++;
				iLastIndex = iIndex;
				dwFlags ^= HTC_UNDERLINE;
				break;
			}
			default:
			{
				if ((TBYTE)cCurChar < _T(' '))
					strLine.Delete(iIndex);
				else
					iIndex++;
			}
		}
	}
	if (iLastIndex < iIndex)
		titleWindow.AppendText(strLine.Mid(iLastIndex, iIndex - iLastIndex), crForegroundColor, crBackgroundColor, dwFlags);
}

void CIrcWnd::SetActivity(CString channel)
{
	Channel		*pRefresh = FindChannelByName(channel);

	if (pRefresh == NULL)
	{
		pRefresh = (Channel*)channelPtrList.GetHead();
		if (pRefresh == NULL)
			return;
	}
	TCITEM		tcitem;
	int			i;

	tcitem.mask = TCIF_PARAM;
	tcitem.lParam = -1;
	for (i = 0; i < channelselect.GetItemCount();i++)
	{
		channelselect.GetItem(i, &tcitem);
		if (((Channel*)tcitem.lParam) == pRefresh)
			break;
	}
	if (((Channel*)tcitem.lParam) != pRefresh)
		return;
	if (channelselect.GetCurSel() != i)
		channelselect.SetItemState(i, TCIS_HIGHLIGHTED, TCIS_HIGHLIGHTED);
}

void CIrcWnd::OnBnClickedTextBold()
{
	CString		strSend;
	int			iStartChar, iEndChar;

	inputWindow.GetSel(iStartChar, iEndChar);
	inputWindow.GetWindowText(strSend);

	if (iEndChar > iStartChar && iEndChar <= strSend.GetLength())
		strSend.Insert(iEndChar, _T('\x02'));

	if (iStartChar <= strSend.GetLength())
	{
		strSend.Insert(iStartChar, _T('\x02'));
		iStartChar++;
		iEndChar++;
	}

	inputWindow.SetWindowText(strSend);
	inputWindow.SetSel(iStartChar, iEndChar);
	inputWindow.SetFocus();
}

void CIrcWnd::OnBnClickedTextUnderline()
{
	CString		strSend;
	int			iStartChar, iEndChar;

	inputWindow.GetSel(iStartChar, iEndChar);
	inputWindow.GetWindowText(strSend);

	if (iEndChar > iStartChar && iEndChar <= strSend.GetLength())
		strSend.Insert(iEndChar, _T('\x1F'));

	if (iStartChar <= strSend.GetLength())
	{
		strSend.Insert(iStartChar, _T('\x1F'));
		iStartChar++;
		iEndChar++;
	}

	inputWindow.SetWindowText(strSend);
	inputWindow.SetSel(iStartChar, iEndChar);
	inputWindow.SetFocus();
}

void CIrcWnd::OnBnClickedTextColor()
{
	CRect		rColorButton;

	m_ctrlTextColorBtn.GetWindowRect(&rColorButton);

	new CColourPopup(CPoint(rColorButton.right - 149, rColorButton.top - 68), CLR_DEFAULT, this,
		GetResString(IDS_DEFAULT), NULL, true);

	inputWindow.SetFocus();
}

LONG CIrcWnd::OnColorSelEndOK(WPARAM wParam, LPARAM lParam)
{
	CWnd			*pParent = GetParent();
	NOPRM(wParam);

	if (pParent != NULL)
	{
		pParent->SendMessage(CPN_CLOSEUP, lParam, (WPARAM)GetDlgCtrlID());
		pParent->SendMessage(CPN_SELENDOK, lParam, (WPARAM)GetDlgCtrlID());
	}

	CString		strSend, strColor = _T("\003");
	int			iStartChar, iEndChar;

	inputWindow.GetSel(iStartChar, iEndChar);
	inputWindow.GetWindowText(strSend);

	if (iEndChar > iStartChar && iEndChar <= strSend.GetLength())
		strSend.Insert(iEndChar, strColor);

	if (iStartChar <= strSend.GetLength())
	{
		if ((int)lParam >= 0 && (int)lParam < 16)
			strColor.AppendFormat(_T("%i"), (int)lParam);
		strSend.Insert(iStartChar, strColor);
		iStartChar += strColor.GetLength();
		iEndChar += strColor.GetLength();
	}

	inputWindow.SetWindowText(strSend);
	inputWindow.SetSel(iStartChar, iEndChar);

	return true;
}

LONG CIrcWnd::OnColorSelEndCancel(WPARAM wParam, LPARAM /*lParam*/)
{
	CWnd		*pParent = GetParent();

	if (pParent != NULL)
	{
		pParent->SendMessage(CPN_CLOSEUP, wParam, (WPARAM)GetDlgCtrlID());
		pParent->SendMessage(CPN_SELENDCANCEL, wParam, (WPARAM)GetDlgCtrlID());
	}

	return true;
}

void CIrcWnd::OnBnClickedChatsend()
{
	inputWindow.SetFocus();

	if (!IsConnected() || m_pCurrentChannel == NULL || m_pCurrentChannel->log.m_hWnd == NULL)
		return;

	CString		strSend;

	inputWindow.GetWindowText(strSend);
	inputWindow.SetWindowText(_T(""));

	if (strSend.IsEmpty())
		return;

	if (m_pCurrentChannel->history.GetCount() == g_App.m_pPrefs->GetMaxChatHistoryLines())
		m_pCurrentChannel->history.RemoveAt(0);
	m_pCurrentChannel->history.Add(strSend);
	m_pCurrentChannel->history_pos = static_cast<uint16>(m_pCurrentChannel->history.GetCount());

	CString		strTemp;
	int			iIndex;

	if ((GetKeyState(VK_CONTROL) & 0x8000) == 0 && strSend[0] == _T('/'))
	{
		strSend.TrimLeft(_T("/ "));
		iIndex = strSend.Find(_T(" "));

		if (iIndex <= 0)
		{
			m_pIrcMain->SendString(strSend);
			return;
		}

		strTemp = strSend.Mid(iIndex);
		strTemp.Trim();

		if (strTemp.IsEmpty())
			return;

		strSend.Truncate(iIndex);
		strSend.MakeLower();

		if (strSend == _T("hop"))
		{
			if (m_pCurrentChannel->type != 4)
				return;

			m_pIrcMain->SendString(_T("PART ") + m_pCurrentChannel->name);
			m_pIrcMain->SendString(_T("JOIN ") + strTemp);
			return;
		}
		else if (strSend == _T("msg") || strSend == _T("privmsg"))
		{
			iIndex = strTemp.Find(_T(" "));

			if (iIndex <= 0)
				return;

			strSend = strTemp.Mid(iIndex);
			strSend.Trim();
			strTemp.Truncate(iIndex);
			strTemp.Trim();

			if (strSend.IsEmpty() || strTemp.IsEmpty())
				return;

			if (strTemp.CompareNoCase(_T("nickserv")) == 0)
				strTemp = _T("ns ");
			else if (strTemp.CompareNoCase(_T("chanserv")) == 0)
				strTemp = _T("cs ");
			else
			{
				strTemp.Insert(0, _T("PRIVMSG "));
				strTemp += _T(" :");
			}
			strTemp += strSend;
			strSend.Insert(0, _T("* >> "));
			AddMessage((m_pCurrentChannel->type == 4 || m_pCurrentChannel->type == 5) ? m_pCurrentChannel->name : _T(""), _T(""), RGB(153, 51, 102), strSend);
			m_pIrcMain->SendString(strTemp);
			return;
		}
		else if (strSend == _T("topic"))
		{
			iIndex = strTemp.Find(_T(" "));

			if ((iIndex <= 0 || strTemp[iIndex] != _T('#')) && m_pCurrentChannel->type == 4)
			{
				strSend = strTemp;
				strTemp = m_pCurrentChannel->name;
			}
			else if (iIndex > 0)
			{
				strSend = strTemp.Mid(iIndex);
				strSend.Trim();
				strTemp.Truncate(iIndex);
				strTemp.Trim();
			}
			else
				return;

			if (strSend.IsEmpty() || strTemp.IsEmpty() || strTemp[0] != _T('#'))
				return;

			strTemp.Insert(0, _T("TOPIC "));
			strTemp += _T(" :");
			strTemp += strSend;
			m_pIrcMain->SendString(strTemp);
			return;
		}
		else if (strSend == _T("me"))
		{
			if (m_pCurrentChannel->type != 4 && m_pCurrentChannel->type != 5)
				return;

			strSend.Format(_T("PRIVMSG %s :\001ACTION %s\001"), m_pCurrentChannel->name, strTemp);
			AddMessageF(m_pCurrentChannel->name, _T(""), RGB(0, 147, 0), _T("* %s %s"), m_pIrcMain->GetNick(), strTemp);
			m_pIrcMain->SendString(strSend);
			return;
		}
		else
		{
			strSend += _T(" ");
			strSend += strTemp;
			m_pIrcMain->SendString(strSend);
			return;
		}
	}

	if (m_pCurrentChannel->type < 4)
	{
		m_pIrcMain->SendString(strSend);
		return;
	}

	strTemp.Format(_T("PRIVMSG %s :%s"), m_pCurrentChannel->name, strSend);
	m_pIrcMain->SendString(strTemp);
	AddMessage(m_pCurrentChannel->name, m_pIrcMain->GetNick(), CLR_DEFAULT, strSend);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//	Channels
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

Channel* CIrcWnd::FindChannelByName(const CString &strName)
{
	Channel	*pCurChannel;
	CString	strSearch(strName);

	strSearch.Trim();
	for (POSITION pos = channelPtrList.GetHeadPosition(); pos != NULL;)
	{
		pCurChannel = (Channel*)channelPtrList.GetNext(pos);
		if (pCurChannel->name.CompareNoCase(strSearch) == 0 && (pCurChannel->type == 4 || pCurChannel->type == 5))
			return pCurChannel;
	}
	return NULL;
}

Channel* CIrcWnd::NewChannel(const CString &strName, byte byteType)
{
	Channel		*pToAdd = new Channel;
	TCITEM		tcitem;

	pToAdd->name = strName;
	pToAdd->title = strName;
	pToAdd->type = byteType;
	pToAdd->history_pos = 0;
	if (byteType != 2)
	{
		CRect		rcChannel;

		serverChannelList.GetWindowRect(&rcChannel);
		ScreenToClient(&rcChannel);
		if (pToAdd->type == 4)
			rcChannel.top += 18;
		pToAdd->log.Create(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL, rcChannel, this, (UINT)-1);
		pToAdd->log.ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
		pToAdd->log.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
		pToAdd->log.SetFont(&g_App.m_pMDlg->m_fontDefault);
		pToAdd->log.SetTitle(strName);
		pToAdd->log.m_dwFlags = HTC_ISAUTOSCROLL | HTC_ISWORDWRAP | HTC_ISDEFAULTLINKS;
		pToAdd->log.SetTargetDevice(NULL, 0);
	}
	channelPtrList.AddTail(pToAdd);

	tcitem.mask = TCIF_PARAM | TCIF_TEXT;
	if (byteType >= 4)
		tcitem.mask |= TCIF_IMAGE;
	tcitem.lParam = (LPARAM)pToAdd;
	tcitem.pszText = const_cast<LPTSTR>(strName.GetString());
	tcitem.iImage = 0;
	uint32 pos = channelselect.GetItemCount();

	channelselect.InsertItem(pos, &tcitem);
	if (byteType >= 4)
	{
		channelselect.SetCurSel(pos);
		channelselect.SetCurFocus(pos);
		OnTcnSelchangeTab2(NULL, NULL);
		GetDlgItem(IDC_CLOSECHAT)->EnableWindow(true);
		GetDlgItem(IDC_CHATSEND)->EnableWindow(true);
	}

	return pToAdd;
}

void CIrcWnd::RemoveChannel(CString channel)
{
	Channel		*pToDel = FindChannelByName(channel);

	if (pToDel == NULL)
		return;

	TCITEM		tcitem;
	int			i;

	tcitem.mask = TCIF_PARAM;
	tcitem.lParam = -1;

	for (i = 0; i < channelselect.GetItemCount();i++)
	{
		channelselect.GetItem(i, &tcitem);
		if (((Channel*)tcitem.lParam) == pToDel)
			break;
	}

	if (((Channel*)tcitem.lParam) != pToDel)
		return;

	channelselect.DeleteItem(i);

	if (pToDel == m_pCurrentChannel)
	{
		m_ctlNickList.DeleteAllItems();
		if (channelselect.GetItemCount() > 2 && i > 1)
		{
			if (i == 2)
				i++;
			channelselect.SetCurSel(i - 1);
			channelselect.SetCurFocus(i - 1);
			OnTcnSelchangeTab2(NULL, NULL);
		}
		else
		{
			channelselect.SetCurSel(0);
			channelselect.SetCurFocus(0);
			OnTcnSelchangeTab2(NULL, NULL);
			GetDlgItem(IDC_CLOSECHAT)->EnableWindow(false);
			GetDlgItem(IDC_CHATSEND)->EnableWindow(false);
		}
	}
	m_ctlNickList.DeleteAllNick(pToDel->name);
	channelPtrList.RemoveAt(channelPtrList.Find(pToDel));
	delete pToDel;
}

void CIrcWnd::DeleteAllChannel()
{
	POSITION	pos1, pos2;
	Channel		*pChannel;

	for (pos1 = channelPtrList.GetHeadPosition();(pos2 = pos1) != NULL;)
	{
		pChannel = (Channel*)channelPtrList.GetNext(pos1);
		m_ctlNickList.DeleteAllNick(pChannel->name);
		channelPtrList.RemoveAt(pos2);
		delete pChannel;
	}
}

void CIrcWnd::JoinChannels()
{
	if (!IsConnected())
		return;

	int			iIndex = -1;
	POSITION	pos = serverChannelList.GetFirstSelectedItemPosition();

	while (pos != NULL)
	{
		iIndex = serverChannelList.GetNextSelectedItem(pos);
		if (iIndex > -1)
			m_pIrcMain->SendString(_T("JOIN ") + serverChannelList.GetItemText(iIndex, 0));
	}
}

LRESULT CIrcWnd::OnCloseTab(WPARAM wparam, LPARAM lparam)
{
	NOPRM(lparam);
	OnBnClickedClosechat((int)wparam);
	return true;
}

void CIrcWnd::SendString(const CString &strSend)
{
	if (IsConnected())
		m_pIrcMain->SendString(strSend);
}

void CIrcWnd::ScrollHistory(bool down)
{
	if ((m_pCurrentChannel->history_pos == 0 && !down) || (m_pCurrentChannel->history_pos == m_pCurrentChannel->history.GetCount() && down))
		return;

	if (down)
		++m_pCurrentChannel->history_pos;
	else
		--m_pCurrentChannel->history_pos;

	CString		strBuffer;

	if (m_pCurrentChannel->history_pos != m_pCurrentChannel->history.GetCount())
		strBuffer = m_pCurrentChannel->history.GetAt(m_pCurrentChannel->history_pos);

	inputWindow.SetWindowText(strBuffer);
	inputWindow.SetSel(strBuffer.GetLength(), strBuffer.GetLength());
}
