//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "SharedFilesCtrl.h"
#include "./Addons/AudioListCtrl.h" //>>> WiZaRd::SharedFiles Redesign
#include "ProgressCtrlX.h"
#include "IconStatic.h"
#include "SharedDirsTreeCtrl.h"
#include "SplitterControl.h"
#include "EditDelayed.h" //>>> WiZaRd::ShareFilter
#include "HistoryListCtrl.h" //Xman [MoNKi: -Downloaded History-]

class CSharedFilesWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CSharedFilesWnd)

public:
	CSharedFilesWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSharedFilesWnd();

	void Localize();
	void ShowSelectedFilesSummary(bool bHistory =false); //Xman [MoNKi: -Downloaded History-]
	void Reload();

// Dialog Data
	enum { IDD = IDD_FILES };

	CSharedFilesCtrl sharedfilesctrl;

private:
	CProgressCtrlX pop_bar;
	CProgressCtrlX pop_baraccept;
	CProgressCtrlX pop_bartrans;
	CFont bold;
	CIconStatic m_ctrlStatisticsFrm;
	CSharedDirsTreeCtrl m_ctlSharedDirTree;
	HICON icon_files;
	CSplitterControl m_wndSplitter;

protected:
	void SetAllIcons();
	void DoResize(int delta);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedPreview(); //>>> WiZaRd::MediaPlayer
	afx_msg void OnBnClickedReloadSharedFiles();
	//boizaum open shared folder
	afx_msg void OnBnClickedOpenincomingfolder();
	
	afx_msg void OnLvnItemActivateSharedFiles(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSharedFiles(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg void OnStnDblClickFilesIco();
	afx_msg void OnTvnSelChangedSharedDirsTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow( BOOL bShow,UINT nStatus  ); //Xman [MoNKi: -Downloaded History-]
	//Xman [MoNKi: -Downloaded History-]
public:
	CHistoryListCtrl historylistctrl;
protected:
	afx_msg void OnLvnItemActivateHistorylist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickHistorylist(NMHDR *pNMHDR, LRESULT *pResult);
	//Xman end

//>>> WiZaRd::SharedFiles Redesign
private:
	bool			m_bShareVisible;
public:
	CAudioListCtrl	 audiolistctrl;
	void			ToggleShareAudio(const bool bShare);
//<<< WiZaRd::SharedFiles Redesign
//>>> WiZaRd::ShareFilter
public:
	CStringArray	m_astrFilter;
	UINT			GetFilterColumn() const	{return m_nFilterColumn;}
protected:
	CHeaderCtrl		m_ctlSharedListHeader;
	UINT			m_nFilterColumn;
	CEditDelayed	m_ctlShareFilter;
	afx_msg LRESULT OnChangeFilter(WPARAM wParam, LPARAM lParam);
//<<< WiZaRd::ShareFilter
};