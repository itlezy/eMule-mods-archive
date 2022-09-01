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
#pragma once

#include <vector>
#include <algorithm>
#include "KnownFile.h"
#include "ResizableLib/ResizableDialog.h"

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

class CMassRenameDialog : public CResizableDialog
{
	DECLARE_DYNAMIC(CMassRenameDialog)

public:
	CMassRenameDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMassRenameDialog();
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
// Dialog Data
	enum { IDD = IDD_MASSRENAME };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	bool m_DontTrackKeys;
	CRichEditCtrl* NFNLeft;
	CRichEditCtrl* NFNRight;
	CRichEditCtrl* OldFN;
	CWnd* m_LastFocusedEdit;  // Saves the (Edit-)Control that gained the focus at last
	int LastDelPos;   // Position where the last character was deleted; for UNDO
	int LastEditPos;  // Position where the last character was inserted at; for UNDO
	bool InDel;           // For Delete-tracking in UNDO
	bool LastEditWasUndo; // Remember if the last change was an UNDO
	CString UndoBuffer;
	void UpdateEditMask ();
	void Localize ();
public:
	// The caller of this dialog has to store pointers for all files in the
	// following list before calling DoModal
	CTypedPtrList<CPtrList, CKnownFile*> m_FileList;

	// The following list is the list of target filenames with path for the files
	// in m_FileList:
	std::vector<CString> m_NewFilenames;
	std::vector<CString> m_NewFilePaths;

	CMassRenameEdit* MassRenameEdit;
private:
public:
	afx_msg void OnEnVscrollOldfilenamesedit();
	afx_msg void OnEnVscrollNewfilenameseditLeft();
	afx_msg void OnEnVscrollNewfilenameseditRight();
	afx_msg void OnBnClickedMassrenameok();
	afx_msg void OnBnClickedMassrenamecancel();
	afx_msg void OnEnSetfocusFilenamemaskedit();
	afx_msg void OnEnSetfocusRichEdit();
	afx_msg void OnBnClickedFilenameleft();
	afx_msg void OnBnClickedFilenameright();
	afx_msg void OnBnClickedReset();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnEnChangeFilenamemaskedit();
	afx_msg void OnBnClickedButtonStrip(); //MORPH - Added by SiRoB, Clean MassRename
	afx_msg void OnBnClickedSimplecleanup();
	afx_msg void OnBnClickedInserttextcolumn();
	afx_msg void OnClose();
};

CString SimpleCleanupFilename (CString _filename);
