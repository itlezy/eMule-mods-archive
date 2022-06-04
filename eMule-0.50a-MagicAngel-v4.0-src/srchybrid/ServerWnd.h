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
#include "ResizableLib\ResizableDialog.h"
#include "ServerListCtrl.h"
#include "IconStatic.h"
#include "RichEditCtrlX.h"
#include "ClosableTabCtrl.h"
#include "SplitterControl.h"
//MORPH START - Added by SiRoB, XML News [O²]
#include "types.h" // Added by N_OxYdE: XML News
//MORPH END    - Added by SiRoB, XML News [O²]

class CHTRichEditCtrl;
class CCustomAutoComplete;

// Mighty Knife: News feeds
// grrr, why is this d*mn "pug" implementation completely contained in a
// .h-file and not splitted in a .h/.cpp pair... 
// therefore we mustn't include the .h-file here, otherwise there will be linker
// errors :(
namespace pug {
	class xml_node;
}
// [end] Mighty Knife

class CServerWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CServerWnd)

public:
	CServerWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerWnd();
	void Localize();
	bool UpdateServerMetFromURL(CString strURL);
	void ToggleDebugWindow();
	void UpdateMyInfo();
	void UpdateLogTabSelection();
	void SaveAllSettings();
	BOOL SaveServerMetStrings();
	void ShowNetworkInfo();
	void UpdateControlsState();
	void ResetHistory();
	void PasteServerFromClipboard();
	bool AddServer(uint16 uPort, CString strAddress, CString strName = _T(""), bool bShowErrorMB = true);
	CString GetMyInfoString();

	//MORPH START - Added by SiRoB, XML News [O²]
	void ListFeeds();
	void DownloadFeed();
	void DownloadAllFeeds(); //Commander - Added: Update All Feeds at once
	void ParseNewsNode(pug::xml_node _node, CString _xmlbuffer);
	void ParseNewsFile(LPCTSTR strTempFilename);
	void OnFeedListSelChange();
	CArray<CString> aFeedUrls;
	CArray<CString> aXMLUrls;
	CArray<CString> aXMLNames;
	//MORPH END   - Added by SiRoB, XML News [O²]

// Dialog Data
	enum { IDD = IDD_SERVER };

	enum ELogPaneItems
	{
		PaneServerInfo	= 0, // those are CTabCtrl item indices
		PaneLog			= 1,
		PaneVerboseLog	= 2,
		//MORPH START - Added by SiRoB, XML News [O²]
		PaneNews 		= 3,
		//MORPH END   - Added by SiRoB, XML News [O²]
		//MORPH START - Added by SiRoB, Morph Log
		PaneMorphLog	= 4
		//MORPH END   - Added by SiRoB, Morph Log
	};

	CServerListCtrl serverlistctrl;
	CHTRichEditCtrl* servermsgbox;
	CHTRichEditCtrl* logbox;
	CHTRichEditCtrl* debuglog;
	CClosableTabCtrl StatusSelector;
	CSplitterControl m_wndSplitter;

	//MORPH START - Added by SiRoB, XML News [O²]
	CHTRichEditCtrl* newsmsgbox;
	CComboBox m_feedlist;
	afx_msg void OnEnLinkNewsBox(NMHDR *pNMHDR, LRESULT *pResult);
	//MORPH END - Added by SiRoB, XML News [O²]
	//MORPH START - Added by SiRoB, Morph Log
	CHTRichEditCtrl* morphlog;
	//MORPH END   - Added by SiRoB, Morph Log

	// Mighty Knife: Context menu for editing news feeds
	CMenu m_FeedsMenu;
	void ReadXMLList (CStringList& _names, CStringList& _urls);
	void WriteXMLList (CStringList& _names, CStringList& _urls);
	// [end] Mighty Knife

private:
	void	DoResize(int delta);
	void	UpdateSplitterRange();
	void	DoSplitResize(int delta);
	void	ShowSplitWindow(bool bReDraw = false);
	void	InitSplitter();
	void	ReattachAnchors();

	CIconStatic m_ctrlNewServerFrm;
	CIconStatic m_ctrlUpdateServerFrm;
	CIconStatic m_ctrlMyInfoFrm;
	CImageList m_imlLogPanes;
	HICON icon_srvlist;
	bool	debug;
	CRichEditCtrlX m_MyInfo;
	CHARFORMAT m_cfDef;
	CHARFORMAT m_cfBold;
	CCustomAutoComplete* m_pacServerMetURL;
	CString m_strClickNewVersion;
	//MORPH START - Added by SiRoB, New Version Check
	CString m_strMorphNewVersion;
	//MORPH END   - Added by SiRoB, New Version Check

protected:
	void SetAllIcons();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam); //MORPH

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedAddserver();
	afx_msg void OnBnClickedUpdateServerMetFromUrl();
	afx_msg void OnBnClickedResetLog();
	afx_msg void OnBnConnect();
	afx_msg void OnTcnSelchangeTab3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnLinkServerBox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg void OnDDClicked();
	afx_msg void OnSvrTextChange();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnStnDblclickServlstIco();
	afx_msg void OnSplitterMoved(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedFeedchange(); //MORPH
};