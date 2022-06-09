//this file is part of NeoMule
//Copyright (C)2007 David Xanatos ( XanatosDavid@googlemail.com / http://neomule.sourceforge.net )
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

#include "ResizableLib/ResizableSheet.h"
#include "ResizableLib/ResizableDialog.h"
#include "../TreePropSheet.h"
#include "../ListViewWalkerPropertySheet.h"
//#include "ListViewWalkerPreferenceSheet.h"

CWnd* GetEmuleDlg();

// NEO: MLD - [ModelesDialogs] -- Xanatos -->

template<class WINDOW>
class CModWnd : public WINDOW {

public:
	CModWnd() {
		m_bDeleteOnClose = TRUE;
		m_bActive = FALSE;
	}

	void OpenDialog(int nCmdShow = SW_SHOW, UINT bDeleteOnClose = TRUE) {
		m_bDeleteOnClose = bDeleteOnClose;
		if (!m_bActive){
			m_bActive = TRUE;
			CallCreate();
		}
		if(nCmdShow != SW_HIDE){
			ShowWindow(nCmdShow);
			SetFocus();
		}
	}
	virtual void CloseDialog() {
		if (m_bActive)
			DestroyWindow();
	}
	bool IsDialogOpen() { 
		return (m_bActive == TRUE); 
	}

protected:
	virtual void CallCreate() = 0;
	virtual BOOL OnInitDialog(){
		UINT old_nFlags = m_nFlags;
		m_nFlags |= WF_CONTINUEMODAL;
		BOOL bResult = WINDOW::OnInitDialog();
		m_nFlags = old_nFlags;
		return bResult;
	}
	afx_msg void OnOK()	{
		UpdateData();
		DestroyWindow();
	}
	afx_msg void OnCancel()	{
		DestroyWindow();
	}
	afx_msg void PostNcDestroy() {
		m_bActive = FALSE;
		if (m_bDeleteOnClose)
			delete this;
	}

	BOOL			m_bActive;
	BOOL			m_bDeleteOnClose;
};

template<class DIALOG>
class CModWin : public CModWnd<DIALOG> {

public:
	CModWin(UINT nIDTemplate, CWnd* /*pParent*/ = NULL):CModWnd<DIALOG>(){
		m_nIDTemplate = nIDTemplate;
	}

protected:
	virtual void CallCreate() {
		Create(m_nIDTemplate, GetEmuleDlg());
	}
	DECLARE_MESSAGE_MAP()

	UINT			m_nIDTemplate;
};

template<class SHEET>
class CModSht : public CModWnd<SHEET> {

public:
	CModSht():CModWnd<SHEET>() {}

protected:
	virtual void CallCreate() {
		Create(GetEmuleDlg(), WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME | DS_CONTEXTHELP | WS_VISIBLE | WS_MINIMIZEBOX);
	}
	virtual BOOL OnInitDialog(){
		// Modeless property sheets don't have Ok and Cancel buttons by default
		// This little trick fulls the property sheet into thinking it's modal
		// during it's init, so that it doesn't disable the Ok and Cancel buttons
		BOOL old_bModeless = m_bModeless;
		m_bModeless = FALSE;
		BOOL bResult = CModWnd<SHEET>::OnInitDialog();
		m_bModeless = old_bModeless;
		return bResult;
	}
	afx_msg void OnOK()	{
		SendMessage(WM_COMMAND, ID_APPLY_NOW);
		CModWnd<SHEET>::OnOK();
	}
	DECLARE_MESSAGE_MAP()
};

template<class WLK_SHEET>
class CModWlkSht : public CModSht<WLK_SHEET> {

public:
	CModWlkSht(CListCtrlItemWalk* pListCtrl):CModSht<WLK_SHEET>() {
		m_pListCtrl = pListCtrl;
	}

	void DropControl() {
		m_pListCtrl = NULL; 
		m_bDeleteOnClose = TRUE; 
		if(!IsDialogOpen()) 
			delete this;
	}
protected:
	DECLARE_MESSAGE_MAP()
};

typedef CModWin<CDialog> CModDialog;
typedef CModWin<CResizableDialog> CModResizableDialog;
typedef CModSht<CTreePropSheet> CModTreePropSheet;
//typedef CModSht<CResizableSheet> CModResizableSheet;
//typedef CModWlkSht<CListViewWalkerPreferenceSheet> CModListViewWalkerPreferenceSheet;
//typedef CModWlkSht<CListViewWalkerPropertySheet> CModListViewWalkerPropertySheet;

// NEO: MLD END <-- Xanatos --