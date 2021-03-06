//this file is part of eMule
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

#include <vector>
#include <algorithm>
#include "KnownFile.h"
#include "SimpleCleanup.h"
#include "Neo/GUI/Modeless.h" // NEO: MLD - [ModelesDialogs]

class CMassRenameEdit : public CEdit
{
	DECLARE_DYNAMIC(CMassRenameEdit)
protected:
	DECLARE_MESSAGE_MAP()
public:
	int Start1, End1;
	CString m_BeforeEdit;

	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg LRESULT OnPaste(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUndo(WPARAM wParam, LPARAM lParam);
};

class CMassRenameDialog : public CModResizableDialog // NEO: MLD - [ModelesDialogs]
{
	DECLARE_DYNAMIC(CMassRenameDialog)

public:
	CMassRenameDialog();   // standard constructor
	virtual ~CMassRenameDialog();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();

	// The caller of this dialog has to store pointers for all files in the
	// following list before calling DoModal
	CTypedPtrList<CPtrList, CKnownFile*> m_FileList;

// Dialog Data
	enum { IDD = IDD_MASSRENAME };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

	CString UndoBuffer;
	bool m_DontTrackKeys;
	int LastDelPos;				// Position where the last character was deleted; for UNDO
	int LastEditPos;			// Position where the last character was inserted at; for UNDO
	bool InDel;					// For Delete-tracking in UNDO
	bool LastEditWasUndo;		// Remember if the last change was an UNDO

//Lit>
	//CRichEditCtrl NewFN;
	CRichEditCtrl NewFNLeft;
	CRichEditCtrl NewFNRight;
//Lit<
	CRichEditCtrl OldFN;
	CWnd* m_LastFocusedEdit;	// Saves the (Edit-)Control that gained the focus at last

	void UpdateEditMask ();
	void Localize ();

	afx_msg void OnEnVscrollOldfilenamesEdit();
	afx_msg void OnEnVscrollNewfilenamesEdit();
	afx_msg void OnBnClickedMassrenameOk();
	afx_msg void OnEnSetfocusFilenamemaskEdit();
	afx_msg void OnEnSetfocusRichEdit();
	afx_msg void OnBnClickedFilenameleft();
	afx_msg void OnBnClickedFilenameright();
	afx_msg void OnBnClickedReset();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnEnChangeFilenamemaskEdit();
	afx_msg void OnBnClickedButtonStrip(); //MORPH - Added by SiRoB, Clean MassRename
	afx_msg void OnBnClickedSimplecleanup();
	afx_msg void OnBnClickedInserttextcolumn();
	afx_msg void OnClose();
//Lit>
	afx_msg void OnEnVscrollNewfilenamesEditRight();
//Lit<

	afx_msg LRESULT SimpleCleanupNotify(WPARAM wParam, LPARAM lParam);

private:
	CMassRenameEdit* MassRenameEdit;
	CSimpleCleanupDialog* SimpleDlg;
};

CString SimpleCleanupFilename (CString _filename);
