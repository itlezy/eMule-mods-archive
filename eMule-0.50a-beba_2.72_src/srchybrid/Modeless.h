//this file is part of eMule beba
//Copyright (C)2005-2009 Tuxman ( der_tuxman@arcor.de / http://tuxproject.de)
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

#include "ListViewWalkerPropertySheet.h"
#include "TreePropSheet.h"
#include "ResizableLib/ResizableDialog.h"

// CModelessDialog

class CModelessDialog : public CDialog {
	DECLARE_DYNAMIC(CModelessDialog)

public:
	CModelessDialog(UINT nIDTemplate, CWnd* pParent = NULL, BOOL bDeleteOnClose = TRUE);
	void OpenDialog();
	void CloseDialog();
	bool IsDialogOpen();

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void PostNcDestroy();
	DECLARE_MESSAGE_MAP()

private:
	UINT			m_nIDTemplate;
	CWnd*			m_pParent;
	BOOL			m_bActive;
	BOOL			m_bDeleteOnClose;
};

// CModelessResizableDialog

class CModelessResizableDialog : public CResizableDialog {
	//DECLARE_DYNAMIC(CModelessResizableDialog)

public:
	CModelessResizableDialog(UINT nIDTemplate, CWnd* pParent = NULL, BOOL bDeleteOnClose = TRUE);
	void OpenDialog();
	void CloseDialog();
	bool IsDialogOpen();

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void PostNcDestroy();
	DECLARE_MESSAGE_MAP()

private:
	UINT			m_nIDTemplate;
	CWnd*			m_pParent;
	BOOL			m_bActive;
	BOOL			m_bDeleteOnClose;
};

// CModelessPropertySheet

class CModelessTreePropSheet : public CTreePropSheet {
	DECLARE_DYNAMIC(CModelessTreePropSheet)

public:
	CModelessTreePropSheet(bool bDeleteOnClose = FALSE);
	void OpenDialog();
	void CloseDialog();
	bool IsDialogOpen();

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void PostNcDestroy();
	DECLARE_MESSAGE_MAP()

private:
	bool			m_bActive;
	bool			m_bDeleteOnClose;
};

// CModelessPropertySheet

class CModelessPropertySheet : public CListViewWalkerPropertySheet {
	friend class CModelessPropertySheetInterface;
	DECLARE_DYNAMIC(CModelessPropertySheet)

public:
	CModelessPropertySheet(CListCtrlItemWalk* pListCtrl, bool bDeleteOnClose = TRUE);
	virtual ~CModelessPropertySheet();
	void OpenDialog();
	void CloseDialog();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void PostNcDestroy();
	DECLARE_MESSAGE_MAP()

private:
	void RemoveData(CObject* toremove);
	bool m_bActive;
	bool m_bDeleteOnClose;
	CSimpleArray<CModelessPropertySheetInterface*> m_interfaces;
};

// CModelessPropertySheetInterface

class CModelessPropertySheetInterface : public CObject {
	friend class CModelessPropertySheet;
	DECLARE_DYNAMIC(CModelessPropertySheetInterface)

public:
	CModelessPropertySheetInterface(CObject* owner);
	virtual ~CModelessPropertySheetInterface();
	bool IsDialogOpen() const;

protected:
	void OpenPropertySheet(const CSimpleArray<CModelessPropertySheetInterface*>* paOthers, ...);
	virtual CModelessPropertySheet* CreatePropertySheet(va_list) = 0;
	CObject* GetOwner() const;
	int GetPropertySheetCount() const;
	CModelessPropertySheet* GetPropertySheet(int i) const;

private:
	CObject* m_owner;
	CSimpleArray<CModelessPropertySheet*> m_propertySheets;
};
