/*	CStatisticsTree Class Implementation File by Khaos
	Copyright (C) 2003

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	This file is a part of the KX mod, and more
	specifically, it is a part of my statistics
	add-on.

	The purpose of deriving a custom class from CTreeCtrl
	was to provide another level of customization and control.
	This allows us to easily code complicated parsing features
	and a context menu.
*/
#include "stdafx.h"
#include "emule.h"
#include "StatisticsTree.h"
#include "StatisticsDlg.h"
#include "emuledlg.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "Log.h"
#include "StringConversion.h"
#include "UserMsgs.h" //TK4 MOD

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CStatisticsTree, CTreeCtrl)

BEGIN_MESSAGE_MAP(CStatisticsTree, CTreeCtrl)
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemExpanded)
END_MESSAGE_MAP()

CStatisticsTree::CStatisticsTree()
{
}

CStatisticsTree::~CStatisticsTree()
{
	if (mnuContext) VERIFY( mnuContext.DestroyMenu() );
}

// This function is called from CStatisticsDlg::OnInitDialog in StatisticsDlg.cpp
void CStatisticsTree::Init()
{
	m_bExpandingAll = false;
}

// It is necessary to disrupt whatever behavior was preventing
// us from getting OnContextMenu to work.  This seems to be the
// magic fix...
void CStatisticsTree::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	UINT uHitFlags;
	HTREEITEM hItem = HitTest(point, &uHitFlags);
	if (hItem != NULL && (uHitFlags & TVHT_ONITEM))
	{
		Select(hItem, TVGN_CARET);
		SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
	}
	return;
}

void CStatisticsTree::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (point.x != -1 || point.y != -1) {
		CRect rcClient;
		GetClientRect(&rcClient);
		ClientToScreen(&rcClient);
		if (!rcClient.PtInRect(point)) {
			Default();
			return;
		}
	}

	DoMenu(point, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
}

void CStatisticsTree::OnLButtonUp( UINT nFlags, CPoint point )
{
	theApp.emuledlg->statisticswnd->ShowStatistics();
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

// This function saves the expanded tree items intelligently.  Instead
// of saving them every time we ShowStatistics, now they are only saved
// when a parent item is expanded or collapsed.
// m_bExpandingAll is TRUE when CollapseAll, ExpandAll or ApplyExpandedMask
// are executing.  This is to prevent us from saving the string a bajillion
// times whenever these functions are called.  CollapseAll and ExpandAll
// call GetExpandedMask() upon completion.
void CStatisticsTree::OnItemExpanded(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	if (!m_bExpandingAll)
		thePrefs.SetExpandedTreeItems(GetExpandedMask());
}

// Displays the command menu.  This function is overloaded
// because it is used both to display the context menu and also
// the menu that drops down from the button.
void CStatisticsTree::DoMenu()
{
	CPoint cursorPos;
	GetCursorPos(&cursorPos);
	DoMenu(cursorPos);
}

void CStatisticsTree::DoMenu(CPoint doWhere)
{
	DoMenu( doWhere, TPM_RIGHTALIGN | TPM_RIGHTBUTTON );
}

void CStatisticsTree::DoMenu(CPoint doWhere, UINT nFlags)
{
	CFileFind	findBackUp;
	CString		myBuffer;
	int			myFlags;

	myBuffer.Format(_T("%sstatbkup.ini"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	if (!findBackUp.FindFile(myBuffer)) myFlags = MF_GRAYED;
		else myFlags = MF_STRING;

	mnuContext.CreatePopupMenu();
	mnuContext.AddMenuTitle(GetResString(IDS_STATS_MNUTREETITLE), true);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_RESET, GetResString(IDS_STATS_BNRESET)/*, _T("DELETE")*/);
	mnuContext.AppendMenu(myFlags, MP_STATTREE_RESTORE, GetResString(IDS_STATS_BNRESTORE)/*, _T("RESTORE")*/);
	mnuContext.AppendMenu(MF_SEPARATOR);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_EXPANDMAIN, GetResString(IDS_STATS_MNUTREEEXPANDMAIN)/*, _T("EXPANDMAIN")*/);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_EXPANDALL, GetResString(IDS_STATS_MNUTREEEXPANDALL)/*, _T("EXPANDALL")*/);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COLLAPSEALL, GetResString(IDS_STATS_MNUTREECOLLAPSEALL)/*, _T("COLLAPSE")*/);
	mnuContext.AppendMenu(MF_SEPARATOR);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYSEL, GetResString(IDS_STATS_MNUTREECPYSEL)/*, _T("COPY")*/);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYVIS, GetResString(IDS_STATS_MNUTREECPYVIS)/*, _T("COPYVISIBLE")*/);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYALL, GetResString(IDS_STATS_MNUTREECPYALL)/*, _T("COPYSELECTED")*/);
	mnuContext.AppendMenu(MF_SEPARATOR);

	// ZZUL-TRA :: ClipStat :: Start
	mnuContext.AppendMenu(MF_STRING, MP_CLIPSTAT, GetResString(IDS_CLIPSTAT_COPY)/*, _T("CLIPSTAT")*/);
	mnuContext.AppendMenu(MF_SEPARATOR);
	// ZZUL-TRA :: ClipStat :: End

//TK4 MOD +
	if(thePrefs.GetSplitterbarPositionStat() < 97) 
		myFlags = MF_GRAYED;
	else                                          
		myFlags = MF_STRING;
    mnuContext.AppendMenu(myFlags, MP_HIDE_SCOPES, GetResString(IDS_REST_SCOPES));
//TK4 MOD -
	GetPopupMenuPos(*this, doWhere);
	mnuContext.TrackPopupMenu(nFlags, doWhere.x, doWhere.y, this);

	VERIFY( mnuContext.DestroyMenu() );
}

// Process context menu items...
BOOL CStatisticsTree::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	switch (wParam) {
		// ZZUL-TRA :: ClipStat :: Start
		case MP_CLIPSTAT:
			theApp.CopyTextToClipboard(theApp.emuledlg->statisticswnd->CreateModFeedback());
			break;
		// ZZUL-TRA :: ClipStat :: End
		case MP_STATTREE_RESET:
			{
				if(AfxMessageBox(GetResString(IDS_STATS_MBRESET_TXT), MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
					break;

				thePrefs.ResetCumulativeStatistics();
				AddLogLine(false, GetResString(IDS_STATS_NFORESET));
				theApp.emuledlg->statisticswnd->ShowStatistics();

				CString myBuffer; 
				myBuffer.Format(GetResString(IDS_STATS_LASTRESETSTATIC), thePrefs.GetStatsLastResetStr(false));
				GetParent()->GetDlgItem(IDC_STATIC_LASTRESET)->SetWindowText(myBuffer);

				break;
			}
		case MP_STATTREE_RESTORE:
			{
				if (AfxMessageBox(GetResString(IDS_STATS_MBRESTORE_TXT), MB_YESNO | MB_ICONQUESTION) == IDNO)
					break;

				if(!thePrefs.LoadStats(1))
					LogError(LOG_STATUSBAR, GetResString(IDS_ERR_NOSTATBKUP));
				else {
					AddLogLine(false, GetResString(IDS_STATS_NFOLOADEDBKUP));
					CString myBuffer;
					myBuffer.Format(GetResString(IDS_STATS_LASTRESETSTATIC), thePrefs.GetStatsLastResetStr(false));
					GetParent()->GetDlgItem(IDC_STATIC_LASTRESET)->SetWindowText(myBuffer);
				}

				break;
			}
		case MP_STATTREE_EXPANDMAIN:
			{
				SetRedraw(false);
				ExpandAll(true);
				goto lblSaveExpanded;
			}
		case MP_STATTREE_EXPANDALL:
			{
				SetRedraw(false);
				ExpandAll();
				goto lblSaveExpanded;
			}
		case MP_STATTREE_COLLAPSEALL:
			{
				SetRedraw(false);
				CollapseAll();
lblSaveExpanded:
				thePrefs.SetExpandedTreeItems(GetExpandedMask());
				SetRedraw(true);
				break;
			}
		case MP_STATTREE_COPYSEL:
		case MP_STATTREE_COPYVIS:
		case MP_STATTREE_COPYALL:
			{
				CopyText(wParam);
				break;
			}
		
	      //TK4 MOD +
         	case MP_HIDE_SCOPES:
		     {
			 	theApp.emuledlg->statisticswnd->SendMessage(UM_RESTORE_SCOPES,MP_HIDE_SCOPES,0);
				break;
			 }
		//TK4 MOD -
	}

	return true;
}

// If the item is bold it returns true, otherwise
// false.  Very straightforward.
// EX: if(IsBold(myTreeItem)) AfxMessageBox("It's bold.");
BOOL CStatisticsTree::IsBold(HTREEITEM theItem)
{
	UINT stateBold = GetItemState(theItem, TVIS_BOLD);
	return (stateBold & TVIS_BOLD);
}

// If the item is expanded it returns true, otherwise
// false.  Very straightforward.
// EX: if(IsExpanded(myTreeItem)) AfxMessageBox("It's expanded.");
BOOL CStatisticsTree::IsExpanded(HTREEITEM theItem)
{
	UINT stateExpanded = GetItemState(theItem, TVIS_EXPANDED);
	return (stateExpanded & TVIS_EXPANDED);
}

// This is a generic function to check if a state is valid or not.
// It accepts a tree item handle and a state/statemask/whatever.
// It then retrieves the state UINT value and does a bitand
// with the original input.  This should translate into a
// boolean result that tells us whether the checked state is
// true or not.  This is currently unused, but may come in handy
// for states other than bold and expanded.
// EX:  if(CheckState(myTreeItem, TVIS_BOLD)) AfxMessageBox("It's bold.");
BOOL CStatisticsTree::CheckState(HTREEITEM hItem, UINT state)
{
	UINT stateGeneric = GetItemState(hItem, state);
	return (stateGeneric & state);
}

// Returns the entire text label of an HTREEITEM.  This
// is an overloaded function.
// EX: CString itemText = GetItemText(myTreeItem);
CString CStatisticsTree::GetItemText(HTREEITEM theItem)
{
	if (theItem == NULL)
		return _T("");

	TVITEM item;
	TCHAR szText[1024]; 
	item.mask = TVIF_TEXT | TVIF_HANDLE;
	item.hItem = theItem;
	item.pszText = szText;
	item.cchTextMax = _countof(szText);

	if (GetItem(&item)) {
		szText[_countof(szText) - 1] = _T('\0');
		return szText;
	}

	return _T("");
}

// This seperates the title from the value in a tree item that has
// a title to the left of a colon, and a value to the right, with
// a space seperating the value from the colon. ": "
// int getPart can be GET_TITLE (0) or GET_VALUE (1)
// EXAMPLE:
// HTREEITEM hMyItem = treeCtrl.InsertItem("Title: 5", hMyParent);
// CString strTitle = treeCtrl.GetItemText(hMyItem, GET_TITLE);
// CString strValue = treeCtrl.GetItemText(hMyItem, GET_VALUE);
// AfxMessageBox("The title is: " + strTitle + "\nThe value is: " + strValue);
CString CStatisticsTree::GetItemText(HTREEITEM theItem, int getPart)
{
	if (theItem == NULL)
		return _T("");

	CString fullText, returnText;
	fullText = GetItemText(theItem);

	if (fullText.IsEmpty())
		return _T("");

	int posSeparator = fullText.Find(_T(": "));
	
	if (posSeparator < 1) {
		returnText = getPart == GET_TITLE ? fullText : _T("");
		return returnText;
	}

	if (getPart == GET_TITLE)
		returnText = fullText.Left(posSeparator);
	else if (getPart == GET_VALUE)
		returnText = fullText.Mid(posSeparator + 2);
	else
		returnText = _T("");

	return returnText;
}

CString CStatisticsTree::GetText(bool onlyVisible, HTREEITEM theItem, int theItemLevel, bool firstItem)
{
	bool bPrintHeader = firstItem;
	HTREEITEM hCurrent;
	if (theItem == NULL)
	{
		hCurrent = GetRootItem(); // Copy All Vis or Copy All
	}
	else
	{
		if (bPrintHeader && (!ItemHasChildren(theItem) || !IsExpanded(theItem)))
			bPrintHeader = false;
		hCurrent = theItem;
	}

	CString	strBuffer;
	if (bPrintHeader)
		strBuffer.Format(_T("eMule v%s %s [%s]\r\n\r\n"), theApp.m_strCurVersionLong, GetResString(IDS_SF_STATISTICS) ,thePrefs.GetUserNick());

	while (hCurrent != NULL)
	{
		for (int i = 0; i < theItemLevel; i++)
			strBuffer += _T("   ");
		strBuffer += GetItemText(hCurrent);
		if (bPrintHeader || !firstItem)
			strBuffer += _T("\r\n");
		if (ItemHasChildren(hCurrent) && (!onlyVisible || IsExpanded(hCurrent)))
			strBuffer += GetText(onlyVisible, GetChildItem(hCurrent), theItemLevel+1, false);
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
		if (firstItem && theItem != NULL)
			break; // Copy Selected Branch was used, so we don't want to copy all branches at this level.  Only the one that was selected.
	}
	return strBuffer;
}

// Doh-nuts.
bool CStatisticsTree::CopyText(int copyMode)
{
	switch (copyMode) {
		case MP_STATTREE_COPYSEL:
			{
				HTREEITEM selectedItem = GetSelectedItem();
				if (selectedItem != NULL) {
					CString theText = GetText(true, selectedItem);
					if (theText.IsEmpty())
						return false;
					theApp.CopyTextToClipboard(theText);
					return true;
				}
				return false;
			}
		case MP_STATTREE_COPYVIS:
			{
				CString theText = GetText();
				if (theText.IsEmpty())
					return false;
				theApp.CopyTextToClipboard(theText);
				return true;
			}
		case MP_STATTREE_COPYALL:
			{
				CString theText = GetText(false);
				if (theText.IsEmpty())
					return false;
				theApp.CopyTextToClipboard(theText);
				return true;
			}
	}

	return false;
}

// This function generates the HTML output for ExportHTML.  The reason this was made separate
// from GetHTML is because it uses style sheets.  This lets the user easily customize the look
// of the HTML file after it is saved, just by changing a value here and there.
// Styled ID Tags:	pghdr	= This is used for the header that gives the eMule build and date.
//					sec		= Sections, ie Transfer, Connection, Session, Cumulative
//					item	= Items, ie UL:DL Ratio, Peak Connections, Downloaded Data
//					bdy		= The BODY tag.  Used to control the background color.
CString CStatisticsTree::GetHTMLForExport(HTREEITEM theItem, int theItemLevel, bool firstItem)
{
	static int s_iHtmlId;
	if (theItem==NULL && theItemLevel==0 && firstItem)
		s_iHtmlId = 0;

	CString		strBuffer, strItem, strImage, strChild, strTab;
	int			nImage=0, nSelectedImage=0;
	HTREEITEM	hCurrent;

	CString strDivStart, strDiv, strDivA, strDivEnd, strJ, strName;

	strBuffer.Empty();

	if (firstItem) hCurrent = GetRootItem();
	else hCurrent = theItem;

	while (hCurrent != NULL)
	{
		strItem.Empty();
		if (ItemHasChildren(hCurrent))
		{
			s_iHtmlId++;
			strJ.Format(_T("%d"),s_iHtmlId);
			if (IsExpanded(hCurrent))
			{
				strChild = _T("visible");
				strDiv = _T("<div id=\"T") + strJ + _T("\" style=\"margin-left:18px\">");
			}
			else
			{
				strChild = _T("hidden");
				strDiv = _T("<div id=\"T") + strJ + _T("\" style=\"margin-left:18px; visibility:hidden; position:absolute\">");
			}
			strDivStart = _T("<a href=\"javascript:togglevisible('") + strJ + _T("')\">");
			strDivEnd = _T("</div>");
			strDivA = _T("</a>");
			strName = _T("name=\"I") + strJ + _T("\"");
		}
		else
		{
			strChild = _T("space");
			strDiv=_T("");
			strDivStart=_T("");
			strDivEnd=_T("");
			strName=_T("");
		}
		strBuffer += _T("\n");
		for (int i = 0; i < theItemLevel; i++)
			strBuffer += _T("\t");

		strItem += strDivStart;
		strItem += _T("<img ") + strName + _T("src=\"stats_") + strChild + _T(".gif\" align=\"middle\">&nbsp;");
		strItem += strDivA;

		if (GetItemImage(hCurrent, nImage, nSelectedImage))
			strImage.Format(_T("%u"),nImage);
		else
			strImage.Format(_T("%u"),0);

		strItem += _T("<img src=\"stats_") + strImage + _T(".gif\" align=\"middle\">&nbsp;");

		if (IsBold(hCurrent))
			strItem += _T("<b>") + GetItemText(hCurrent) + _T("</b>");
		else
			strItem += GetItemText(hCurrent);

		if (theItemLevel==0) strBuffer.Append(_T("\n"));
		strBuffer += strItem + _T("<br />");

		if (ItemHasChildren(hCurrent))
		{
			strTab = _T("\n");
			for (int i = 0; i < theItemLevel; i++)
				strTab += _T("\t");
			strBuffer += strTab + strDiv;
			strBuffer += strTab + _T("\t") + GetHTMLForExport(GetChildItem(hCurrent), theItemLevel+1, false);
			strBuffer += strTab + strDivEnd;
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}
	return strBuffer;
}

// Expand all the tree sections.  Recursive.
// Can also expand only bold items (Main Sections)
void CStatisticsTree::ExpandAll(bool onlyBold, HTREEITEM theItem)
{
	HTREEITEM hCurrent;

	if (theItem == NULL) {
		if (onlyBold) CollapseAll();
		hCurrent = GetRootItem();
		m_bExpandingAll = true;
	}
	else
		hCurrent = theItem;

	while (hCurrent != NULL)
	{
		if (ItemHasChildren(hCurrent) && (!onlyBold || IsBold(hCurrent))) {
			Expand(hCurrent, TVE_EXPAND);
			ExpandAll(onlyBold, GetChildItem(hCurrent));
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}

	if (theItem == NULL) m_bExpandingAll = false;
}

// Collapse all the tree sections.  This is recursive
// so that we can collapse submenus.  SetRedraw should
// be FALSE while this is executing.
void CStatisticsTree::CollapseAll(HTREEITEM theItem)
{
	HTREEITEM hCurrent;

	if (theItem == NULL) {
		hCurrent = GetRootItem();
		m_bExpandingAll = true;
	}
	else
		hCurrent = theItem;

	while (hCurrent != NULL)
	{
		if (ItemHasChildren(hCurrent))
			CollapseAll(GetChildItem(hCurrent));
		Expand(hCurrent, TVE_COLLAPSE);
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}

	if (theItem == NULL) m_bExpandingAll = false;
}

// This returns a string of 1's and 0's indicating
// which parent items are expanded.  Only saves the
// bold items.
CString CStatisticsTree::GetExpandedMask(HTREEITEM theItem)
{
	HTREEITEM	hCurrent;
	CString		tempMask;

	tempMask.Empty();

	if (theItem == NULL)
		hCurrent = GetRootItem();
	else
		hCurrent = theItem;

	while (hCurrent != NULL)
	{
		if (ItemHasChildren(hCurrent) && IsBold(hCurrent)) {
			if (IsExpanded(hCurrent))
				tempMask += _T("1");
			if (!IsExpanded(hCurrent))
				tempMask += _T("0");
			tempMask += GetExpandedMask(GetChildItem(hCurrent));
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}
	return tempMask;
}

// This takes a string and uses it to set the expanded or
// collapsed state of the tree items.
int CStatisticsTree::ApplyExpandedMask(CString theMask, HTREEITEM theItem, int theStringIndex)
{
	HTREEITEM	hCurrent;

	if (theItem == NULL) {
		hCurrent = GetRootItem();
		SetRedraw(false);
		ExpandAll(true);
		m_bExpandingAll = true;
	}
	else
		hCurrent = theItem;

	while (hCurrent != NULL && theStringIndex < theMask.GetLength())
	{
		if (ItemHasChildren(hCurrent) && IsBold(hCurrent)) {
			if (theMask.GetAt(theStringIndex) == '0') Expand(hCurrent, TVE_COLLAPSE);
			theStringIndex++;
			theStringIndex = ApplyExpandedMask(theMask, GetChildItem(hCurrent), theStringIndex);			
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}
	if (theItem == NULL) {
		SetRedraw(true);
		m_bExpandingAll = true;
	}
	return theStringIndex;
}