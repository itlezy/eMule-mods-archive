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
#include "otherfunctions.h"
#include "TitleMenu.h"
#include "StatisticsDlg.h"
#include "UploadQueue.h"
#include <time.h>

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
}

// This function is called from CStatisticsDlg::OnInitDialog in StatisticsDlg.cpp
void CStatisticsTree::Init()
{
	m_bExpandingAll = false;
	ModifyStyle(0,TVS_NOTOOLTIPS | TVS_DISABLEDRAGDROP);
#ifdef _UNICODE
//	Win9x: Explicitly set to Unicode to receive Unicode notifications.
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);
#endif
}

// It is necessary to disrupt whatever behavior was preventing
// us from getting OnContextMenu to work.  This seems to be the
// magic fix...
void CStatisticsTree::OnRButtonDown(UINT nFlags, CPoint point)
{
	NOPRM(nFlags); NOPRM(point);
}

void CStatisticsTree::OnContextMenu(CWnd* pWnd, CPoint point)
{
	NOPRM(pWnd);
	DoMenu(point, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
}

void CStatisticsTree::OnLButtonUp( UINT nFlags, CPoint point )
{
	g_App.m_pMDlg->m_dlgStatistics.ShowStatistics();
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

// This function saves the expanded tree items intelligently.  Instead
// of saving them every time we ShowStatistics, now they are only saved
// when a parent item is expanded or collapsed.
// m_bExpandingAll is TRUE when CollapseAll, ExpandAll or ApplyExpandedMask
// are executing.  This is to prevent us from saving the string a bajillion
// times whenever these functions are called.  CollapseAll and ExpandAll
// call GetExpandedMask() upon completion.
void CStatisticsTree::OnItemExpanded(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR); NOPRM(pResult);
	if (!m_bExpandingAll)
		g_App.m_pPrefs->SetExpandedTreeItems(GetExpandedMask());
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
	DoMenu(doWhere, TPM_RIGHTALIGN | TPM_RIGHTBUTTON);
}

void CStatisticsTree::DoMenu(CPoint doWhere, UINT nFlags)
{
	CFileFind	findBackUp;
	CString		strBuffer(g_App.m_pPrefs->GetConfigDir());
	CTitleMenu	mnuContext;
	CMenu		mnuHTML;

	strBuffer += _T("statbkup.ini");

	int			iFlags = MF_STRING | (!findBackUp.FindFile(strBuffer) ? MF_GRAYED : 0);

	mnuContext.CreatePopupMenu();
	mnuContext.AddMenuTitle(GetResString(IDS_STATS_MNUTREETITLE));

	mnuContext.AppendMenu(MF_STRING | MF_POPUP, MP_STATTREE_RESET, GetResString(IDS_STATS_BNRESET));
	mnuContext.AppendMenu(iFlags, MP_STATTREE_RESTORE, GetResString(IDS_STATS_BNRESTORE));
	mnuContext.AppendMenu(MF_SEPARATOR);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_EXPANDMAIN, GetResString(IDS_STATS_MNUTREEEXPANDMAIN));
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_EXPANDALL, GetResString(IDS_STATS_MNUTREEEXPANDALL));
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COLLAPSEALL, GetResString(IDS_STATS_MNUTREECOLLAPSEALL));
	mnuContext.AppendMenu(MF_SEPARATOR);
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYSEL, GetResString(IDS_STATS_MNUTREECPYSEL));
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYVIS, GetResString(IDS_STATS_MNUTREECPYVIS));
	mnuContext.AppendMenu(MF_STRING, MP_STATTREE_COPYALL, GetResString(IDS_STATS_MNUTREECPYALL));
	mnuContext.AppendMenu(MF_SEPARATOR);

	mnuHTML.CreateMenu();
	mnuHTML.AppendMenu(MF_STRING, MP_STATTREE_HTMLCOPYSEL, GetResString(IDS_STATS_MNUTREECPYSEL));
	mnuHTML.AppendMenu(MF_STRING, MP_STATTREE_HTMLCOPYVIS, GetResString(IDS_STATS_MNUTREECPYVIS));
	mnuHTML.AppendMenu(MF_STRING, MP_STATTREE_HTMLCOPYALL, GetResString(IDS_STATS_MNUTREECPYALL));
	mnuHTML.AppendMenu(MF_SEPARATOR);
	mnuHTML.AppendMenu(MF_STRING, MP_STATTREE_HTMLEXPVIS, GetResString(IDS_STATS_EXPORTVIS));
	mnuHTML.AppendMenu(MF_STRING, MP_STATTREE_HTMLEXPORT, GetResString(IDS_STATS_EXPORT2HTML));
	mnuContext.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)mnuHTML.m_hMenu, GetResString(IDS_STATS_MNUTREEHTML));

	mnuContext.TrackPopupMenuEx(nFlags, doWhere.x, doWhere.y, this, NULL);

//	Menu objects are destroyed in their destructor
}

// Process context menu items...
BOOL CStatisticsTree::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);
	switch (wParam)
	{
		case MP_STATTREE_RESET:
			if(AfxMessageBox(GetResString(IDS_STATS_MBRESET_TXT), MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
				break;

			g_App.m_pPrefs->ResetStatistics();
			g_App.m_pMDlg->AddLogLine(0, IDS_STATS_NFORESET);
			g_App.m_pMDlg->m_dlgStatistics.ShowStatistics();
			break;

		case MP_STATTREE_RESTORE:
			if (AfxMessageBox(GetResString(IDS_STATS_MBRESTORE_TXT), MB_YESNO | MB_ICONQUESTION) == IDNO)
				break;

			if (!g_App.m_pPrefs->LoadStats(1))
				g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR, IDS_ERR_NOSTATBKUP);
			else
				g_App.m_pMDlg->AddLogLine(0, IDS_STATS_NFOLOADEDBKUP);
			break;

		case MP_STATTREE_EXPANDMAIN:
			SetRedraw(false);
			ExpandAll(true);
			goto lblSaveExpanded;

		case MP_STATTREE_EXPANDALL:
			SetRedraw(false);
			ExpandAll();
			goto lblSaveExpanded;

		case MP_STATTREE_COLLAPSEALL:
			SetRedraw(false);
			CollapseAll();
lblSaveExpanded:
			g_App.m_pPrefs->SetExpandedTreeItems(GetExpandedMask());
			SetRedraw(true);
			break;

		case MP_STATTREE_COPYSEL:
		case MP_STATTREE_COPYVIS:
		case MP_STATTREE_COPYALL:
			CopyText(wParam);
			break;

		case MP_STATTREE_HTMLCOPYSEL:
		case MP_STATTREE_HTMLCOPYVIS:
		case MP_STATTREE_HTMLCOPYALL:
			CopyHTML(wParam);
			break;

		case MP_STATTREE_HTMLEXPORT:
		case MP_STATTREE_HTMLEXPVIS:
			ExportHTML(wParam==MP_STATTREE_HTMLEXPVIS);
			break;
	}

	return true;
}

// If the item is bold it returns true, otherwise
// false.  Very straightforward.
// EX: if(IsBold(myTreeItem)) MessageBox("It's bold.");
bool CStatisticsTree::IsBold(HTREEITEM theItem)
{
	UINT stateBold = GetItemState(theItem, TVIS_BOLD);
	return (stateBold & TVIS_BOLD) ? true : false;
}

// If the item is expanded it returns true, otherwise
// false.  Very straightforward.
// EX: if(IsExpanded(myTreeItem)) MessageBox("It's expanded.");
bool CStatisticsTree::IsExpanded(HTREEITEM theItem)
{
	UINT stateExpanded = GetItemState(theItem, TVIS_EXPANDED);
	return (stateExpanded & TVIS_EXPANDED) ? true : false;
}

// This is a generic function to check if a state is valid or not.
// It accepts a tree item handle and a state/statemask/whatever.
// It then retrieves the state UINT value and does a bitand
// with the original input.  This should translate into a
// boolean result that tells us whether the checked state is
// true or not.  This is currently unused, but may come in handy
// for states other than bold and expanded.
// EX:  if(CheckState(myTreeItem, TVIS_BOLD)) MessageBox("It's bold.");
bool CStatisticsTree::CheckState(HTREEITEM hItem, UINT state)
{
	UINT stateGeneric = GetItemState(hItem, state);
	return (stateGeneric & state) ? true : false;
}

// Returns the entire text label of an HTREEITEM.  This
// is an overloaded function.
// EX: CString itemText = GetItemText(myTreeItem);
CString CStatisticsTree::GetItemText(HTREEITEM hti)
{
	if (hti == NULL)
		return _T("");

	TVITEM	tvi;
	TCHAR	acText[1024];

	tvi.mask = TVIF_TEXT | TVIF_HANDLE;
	tvi.hItem = hti;
	tvi.pszText = acText;
	tvi.cchTextMax = ARRSIZE(acText);

	if (GetItem(&tvi))
		return tvi.pszText;

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
// MessageBox("The title is: " + strTitle + "\nThe value is: " + strValue);
CString CStatisticsTree::GetItemText(HTREEITEM hti, int iPart)
{
	if (hti == NULL)
		return _T("");

	CString	strFullText = GetItemText(hti);

	if (strFullText.IsEmpty())
		return _T("");

	int iSeparatorPos = strFullText.Find(_T(": "));

	if (iSeparatorPos < 1)
		return (iPart == GET_TITLE) ? strFullText : _T("");

	CString	strReturn;

	if (iPart == GET_TITLE)
		strReturn = strFullText.Left(iSeparatorPos);
	else if (iPart == GET_VALUE)
		strReturn = strFullText.Mid(iSeparatorPos + 2);

	return strReturn;
}

// This is the primary function for generating HTML output of the statistics tree.
// It is recursive.
CString CStatisticsTree::GetHTML(bool onlyVisible, HTREEITEM theItem, int theItemLevel, bool firstItem)
{
	CString		strBuffer, strItem;
	HTREEITEM	hCurrent;

	if (firstItem)
		strBuffer.Format(_T("<font face=\"Verdana,Courier New,Helvetica\" size=\"2\">\r\n<b>") CLIENT_NAME_WITH_VER _T(" %s [%s]</b>\r\n<br /><br />\r\n"), GetResString(IDS_STATISTICS), g_App.m_pPrefs->GetUserNick());

	if (theItem == NULL) {
		if (!onlyVisible) g_App.m_pMDlg->m_dlgStatistics.ShowStatistics(true);
		hCurrent = GetRootItem(); // Copy All Vis or Copy All
	}
	else if (firstItem) {
		if (ItemHasChildren(theItem)) hCurrent = theItem; // Copy Branch issued for item with children, use item.
		else hCurrent = GetParentItem(theItem); // Copy Branch issued for item with no children, use parent.
	}
	else hCurrent = theItem; // This function has been recursed.

	while (hCurrent != NULL)
	{
		if (IsBold(hCurrent)) strItem = _T("<b>") + GetItemText(hCurrent) + _T("</b>");
		else strItem = GetItemText(hCurrent);
		for (int i = 0; i < theItemLevel; i++) strBuffer += "&nbsp;&nbsp;&nbsp;";
		if (theItemLevel==0) strBuffer.Append(_T("\n"));
		strBuffer += strItem + _T("<br />");
		if (ItemHasChildren(hCurrent) && (!onlyVisible || IsExpanded(hCurrent)))
			strBuffer += (CString) GetHTML(onlyVisible, GetChildItem(hCurrent), theItemLevel+1, false);
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
		if (firstItem && theItem != NULL) break; // Copy Selected Branch was used, so we don't want to copy all branches at this level.  Only the one that was selected.
	}
	if (firstItem) strBuffer += "</font>";
	return strBuffer;
}

// Takes the HTML output generated by GetHTML
// and puts it on the clipboard.  Simplenuff.
bool CStatisticsTree::CopyHTML(int copyMode)
{
	switch (copyMode) {
		case MP_STATTREE_HTMLCOPYSEL:
			{
				HTREEITEM selectedItem = GetSelectedItem();
				if (selectedItem != NULL) {
					CString theHTML = GetHTML(true, selectedItem);
					if (theHTML.IsEmpty())
						return false;
					g_App.CopyTextToClipboard(theHTML);
					return true;
				}
				return false;
			}
		case MP_STATTREE_HTMLCOPYVIS:
			{
				CString theHTML = GetHTML();
				if (theHTML.IsEmpty())
					return false;
				g_App.CopyTextToClipboard(theHTML);
				return true;
			}
		case MP_STATTREE_HTMLCOPYALL:
			{
				CString theHTML = GetHTML(false);
				if (theHTML.IsEmpty())
					return false;
				g_App.CopyTextToClipboard(theHTML);
				return true;
			}
	}

	return false;
}

// The plaintext alterego of GetHTML.  Simplenuff.
// Oh yeah, the example/code this is based on was originally written by the enkeyDEV
// crew.  This was the inspiration for GetHTML.
CString CStatisticsTree::GetText(bool onlyVisible, HTREEITEM theItem, int theItemLevel, bool firstItem)
{
	CString		strBuffer;
	HTREEITEM	hCurrent;

	if (firstItem)
		strBuffer.Format(CLIENT_NAME_WITH_VER _T(" Statistics [%s]\r\n\r\n"), g_App.m_pPrefs->GetUserNick());

	if (theItem == NULL) hCurrent = GetRootItem(); // Copy All Vis or Copy All
	else if (firstItem) {
		if (ItemHasChildren(theItem)) hCurrent = theItem; // Copy Branch issued for item with children, use item.
		else hCurrent = GetParentItem(theItem); // Copy Branch issued for item with no children, use parent.
	}
	else hCurrent = theItem; // This function has been recursed.

	while (hCurrent != NULL)
	{
		for (int i = 0; i < theItemLevel; i++)
			strBuffer += "   ";
		strBuffer += GetItemText(hCurrent);
		strBuffer += "\r\n";
		if (ItemHasChildren(hCurrent) && (!onlyVisible || IsExpanded(hCurrent)))
			strBuffer += (CString) GetText(onlyVisible, GetChildItem(hCurrent), theItemLevel+1, false);
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
		if (firstItem && theItem != NULL) break; // Copy Selected Branch was used, so we don't want to copy all branches at this level.  Only the one that was selected.
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
					g_App.CopyTextToClipboard(theText);
					return true;
				}
				return false;
			}
		case MP_STATTREE_COPYVIS:
			{
				CString theText = GetText();
				if (theText.IsEmpty())
					return false;
				g_App.CopyTextToClipboard(theText);
				return true;
			}
		case MP_STATTREE_COPYALL:
			{
				CString theText = GetText(false);
				if (theText.IsEmpty())
					return false;
				g_App.CopyTextToClipboard(theText);
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
CString CStatisticsTree::GetHTMLForExport(bool onlyVisible, HTREEITEM theItem, int theItemLevel, bool firstItem)
{
	static int j = 0;
	CString		strBuffer, strItem, strImage, strChild, strTab;
	int			nImage=0, nSelectedImage=0;
	HTREEITEM	hCurrent;
	CString		strDivStart, strDiv, strJ, strName;
	const TCHAR	*pcDivA, *pcDivEnd;

	hCurrent = (firstItem) ? GetRootItem() : theItem;

	while (hCurrent != NULL)
	{
		strItem.Empty();
		if (ItemHasChildren(hCurrent))
		{
			j++;
			strJ.Format(_T("%d"),j);
			strDiv = _T("<div id=\"T");
			strDiv += strJ;
			if (IsExpanded(hCurrent))
			{
				strChild = _T("visible");
				strDiv += _T("\" style=\"margin-left:18px\">");
			}
			else
			{
				strChild = _T("hidden");
				strDiv += _T("\" style=\"margin-left:18px; visibility:hidden; position:absolute\">");
			}
			strDivStart = _T("<a href=\"javascript:togglevisible('") + strJ + _T("')\">");
			pcDivEnd = _T("</div>");
			pcDivA = _T("</a>");
			strName = _T("name=\"I") + strJ + _T("\"");
		}
		else
		{
			strChild = _T("space");
			strDiv = _T("");
			strDivStart = _T("");
			pcDivEnd = _T("");
			pcDivA = _T("");
			strName = _T("");
		}
		strBuffer += _T("\n");
		for (int i = 0; i < theItemLevel; i++)
			strBuffer += _T("\t");

		strItem += strDivStart;
		strItem += _T("<img ") + strName + _T("src=\"stats_") + strChild + _T(".gif\" align=\"middle\">&nbsp;");
		strItem += pcDivA;

		if (GetItemImage(hCurrent, nImage, nSelectedImage))
			strImage.Format(_T("%u"),nImage);
		else
			strImage.Format(_T("%u"),0);

		strItem += _T("<img src=\"stats_") + strImage + _T(".gif\" align=\"middle\">&nbsp;");

		if (IsBold(hCurrent))
			strItem += _T("<b>") + GetItemText(hCurrent) + _T("</b>");
		else
			strItem += GetItemText(hCurrent);

		if (theItemLevel == 0)
			strBuffer += _T('\n');
		strBuffer += strItem;
		strBuffer += _T("<br />");

		if (ItemHasChildren(hCurrent))
		{
			strTab = _T("\n");
			for (int i = 0; i < theItemLevel; i++)
				strTab += _T('\t');
			strBuffer += strTab;
			strBuffer += strDiv;
			strBuffer += strTab + _T('\t') + GetHTMLForExport(onlyVisible, GetChildItem(hCurrent), theItemLevel + 1, false);
			strBuffer += strTab;
			strBuffer += pcDivEnd;
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}
	return strBuffer;
}

// Get a file name from the user, obtain the generated HTML and then save it in that file.
void CStatisticsTree::ExportHTML(bool onlyvisible)
{
	EMULE_TRY

	CFile htmlFile;

	TCHAR szDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szDir);

	CFileDialog saveAsDlg (false, _T("html"), _T("*.html"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER, _T("HTML Files (*.html)|*.html|All Files (*.*)|*.*||"), this, 0);
	if (saveAsDlg.DoModal() == IDOK)
	{
		CString		strHTML;

		strHTML.Format( _T("<html>\r\n<header>\r\n<title>") CLIENT_NAME_WITH_VER _T(" %s [%s]</title>\r\n")
			_T("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">")
			_T("<style type=\"text/css\">\r\n")
			_T("#pghdr { color: #000F80; font: bold 12pt/14pt Verdana, Courier New, Helvetica; }\r\n")
			_T("#pghdr2 { color: #000F80; font: bold 10pt/12pt Verdana, Courier New, Helvetica; }\r\n")
			_T("img { border: 0px; }\r\n")
			_T("a { text-decoration: none; }\r\n")
			_T("#sec { color: #000000; font: bold 9pt/11pt Verdana, Courier New, Helvetica; }\r\n")
			_T("#item { color: #000000; font: normal 8pt/10pt Verdana, Courier New, Helvetica; }\r\n")
			_T("#bdy { color: #000000; font: normal 8pt/10pt Verdana, Courier New, Helvetica; background-color: #FFFFFF; }\r\n</style>\r\n</header>\r\n")
			_T("<script language=\"JavaScript1.2\" type=\"text/javascript\">\r\n")
			_T("function obj(menu)\r\n")
			_T("{\r\n")
			_T("return (navigator.appName == \"Microsoft Internet Explorer\")?this[menu]:document.getElementById(menu);\r\n")
			_T("}\r\n")
			_T("function togglevisible(treepart)\r\n")
			_T("{\r\n")
			_T("if (this.obj(\"T\"+treepart).style.visibility == \"hidden\")\r\n")
			_T("{\r\n")
			_T("this.obj(\"T\"+treepart).style.position=\"\";\r\n")
			_T("this.obj(\"T\"+treepart).style.visibility=\"\";\r\n")
			_T("document[\"I\"+treepart].src=\"stats_visible.gif\";\r\n")
			_T("}\r\n")
			_T("else\r\n")
			_T("{\r\n")
			_T("this.obj(\"T\"+treepart).style.position=\"absolute\";\r\n")
			_T("this.obj(\"T\"+treepart).style.visibility=\"hidden\";\r\n")
			_T("document[\"I\"+treepart].src=\"stats_hidden.gif\";\r\n")
			_T("}\r\n")
			_T("}\r\n")
			_T("</script>\r\n")
			_T("<body id=\"bdy\">\r\n")

			_T("<span id=\"pghdr\"><b>") CLIENT_NAME_WITH_VER _T(" %s</b></span><br /><span id=\"pghdr2\">%s %s</span>\r\n<br /><br />\r\n")
			_T("%s</body></html>"),
			GetResString(IDS_STATISTICS), g_App.m_pPrefs->GetUserNick(),
			GetResString(IDS_STATISTICS), GetResString(IDS_CD_UNAME), g_App.m_pPrefs->GetUserNick(),
			GetHTMLForExport(onlyvisible) );

		htmlFile.Open(saveAsDlg.GetPathName(), CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);
		WriteStr2MB(cfUTF8, strHTML, htmlFile);
		htmlFile.Close();

		static const TCHAR *const s_apcFileNames[] = {
			_T("stats_0.gif"), _T("stats_1.gif"), _T("stats_2.gif"), _T("stats_3.gif"), _T("stats_4.gif"),
			_T("stats_5.gif"), _T("stats_6.gif"), _T("stats_7.gif"), _T("stats_8.gif"), _T("stats_9.gif"),
			_T("stats_10.gif"), _T("stats_11.gif"), _T("stats_12.gif"), _T("stats_13.gif"),
			_T("stats_14.gif"), _T("stats_15.gif"), _T("stats_16.gif"),
			_T("stats_hidden.gif"), _T("stats_space.gif"), _T("stats_visible.gif")
		};
		CString		strDst = saveAsDlg.GetPathName().Left(saveAsDlg.GetPathName().GetLength() - saveAsDlg.GetFileName().GetLength());// EC - what if directory name == filename? this should fix this
		CString		strSrc = g_App.m_pPrefs->GetAppDir();

		strSrc += _T("\\WebServer\\");
		for (unsigned ui = 0; ui < ARRSIZE(s_apcFileNames); ui++)
			::CopyFile(strSrc + s_apcFileNames[ui], strDst + s_apcFileNames[ui], false);
	}

	SetCurrentDirectory(szDir);

	EMULE_CATCH
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

// This recursive function returns a string of 1's and 0's indicating
// which parent items are expanded.  Only saves the
// bold items.
CString& CStatisticsTree::GetExpandedMask(HTREEITEM theItem)
{
	HTREEITEM	hCurrent;

	if (theItem == NULL)
	{
		m_strExpandedMask.Truncate(0);
		hCurrent = GetRootItem();
	}
	else
		hCurrent = theItem;

	while (hCurrent != NULL)
	{
		if (ItemHasChildren(hCurrent) && IsBold(hCurrent))
		{
			if (IsExpanded(hCurrent))
				m_strExpandedMask += _T('1');
			if (!IsExpanded(hCurrent))
				m_strExpandedMask += _T('0');

			GetExpandedMask(GetChildItem(hCurrent));
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}
	return m_strExpandedMask;
}

// This takes a string and uses it to set the expanded or
// collapsed state of the tree items
int CStatisticsTree::ApplyExpandedMask(const CString &strMask, HTREEITEM theItem/*=NULL*/, int theStringIndex/*=0*/)
{
	HTREEITEM	hCurrent;

	if (theItem == NULL)
	{
		hCurrent = GetRootItem();
		SetRedraw(false);
		ExpandAll(true);
		m_bExpandingAll = true;
	}
	else
		hCurrent = theItem;

	while (hCurrent != NULL && theStringIndex < strMask.GetLength())
	{
		if (ItemHasChildren(hCurrent) && IsBold(hCurrent))
		{
			if (strMask.GetAt(theStringIndex) == _T('0'))
				Expand(hCurrent, TVE_COLLAPSE);
			theStringIndex++;
			theStringIndex = ApplyExpandedMask(strMask, GetChildItem(hCurrent), theStringIndex);
		}
		hCurrent = GetNextItem(hCurrent, TVGN_NEXT);
	}
	if (theItem == NULL)
	{
		SetRedraw(true);
		m_bExpandingAll = false;
	}
	return theStringIndex;
}

void CStatisticsTree::DeleteChildItems (HTREEITEM parentItem)
{
	if (ItemHasChildren(parentItem))
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = GetChildItem(parentItem);
		while (hChildItem != NULL)
		{
			hNextItem = GetNextItem(hChildItem, TVGN_NEXT);
			DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}
}
