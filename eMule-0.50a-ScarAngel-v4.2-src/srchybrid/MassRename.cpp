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

// Original file: Written by Mighty Knife, EMule Morph Team

#include "stdafx.h"
#include "emule.h"
#include "resource.h"
#include "Preferences.h"
#include "MassRename.h"
#include "OtherFunctions.h"
#include "SimpleCleanup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define IDC_FILENAMEMASKEDIT 100

// CMassRenameEdit edit control

BEGIN_MESSAGE_MAP(CMassRenameEdit, CEdit)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_MESSAGE(WM_PASTE,OnPaste)
	ON_MESSAGE(WM_UNDO,OnUndo)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CMassRenameEdit, CEdit)

IMPLEMENT_DYNAMIC(CMassRenameDialog, CDialog)
CMassRenameDialog::CMassRenameDialog(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMassRenameDialog::IDD, pParent)
{
}
void CMassRenameEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Get the current string before edit and the selection - then default message processing
	GetSel (Start1,End1);
	GetWindowText (m_BeforeEdit);
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CMassRenameEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Get the current string before edit and the selection - then default message processing	if (Start1!=-1) {
	GetSel (Start1,End1);
	GetWindowText (m_BeforeEdit);
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

LRESULT CMassRenameEdit::OnPaste(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Get the current string before edit and the selection - then default message processing
	GetSel (Start1,End1);
	GetWindowText (m_BeforeEdit);
	return Default();
}

LRESULT CMassRenameEdit::OnUndo(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Set Start1=End1=-5 to signal the main window that an UNDO is going on
	Start1=End1=-5;
	LRESULT res = Default();
	return res;
}

// CMassRenameDialog dialog

CMassRenameDialog::~CMassRenameDialog()
{
	// Delete the CMassRenameEdit object we allocated in OnInitDialog
	delete MassRenameEdit;
}

void CMassRenameDialog::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMassRenameDialog, CResizableDialog)
	ON_EN_VSCROLL(IDC_OLDFILENAMESEDIT, OnEnVscrollOldfilenamesedit)
	ON_EN_VSCROLL(IDC_NEWFILENAMESEDITLEFT, OnEnVscrollNewfilenameseditLeft)
	ON_EN_VSCROLL(IDC_NEWFILENAMESEDITRIGHT, OnEnVscrollNewfilenameseditRight)
	ON_BN_CLICKED(IDOK, OnBnClickedMassrenameok)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedMassrenamecancel)
	ON_EN_SETFOCUS(IDC_FILENAMEMASKEDIT, OnEnSetfocusFilenamemaskedit)
	ON_EN_SETFOCUS(IDC_NEWFILENAMESEDITLEFT, OnEnSetfocusRichEdit)
	ON_EN_SETFOCUS(IDC_NEWFILENAMESEDITRIGHT, OnEnSetfocusRichEdit)
	ON_BN_CLICKED(IDC_FILENAMELEFT, OnBnClickedFilenameleft)
	ON_BN_CLICKED(IDC_FILENAMERIGHT, OnBnClickedFilenameright)
	ON_BN_CLICKED(IDC_RESETBUTTON, OnBnClickedReset)
	ON_WM_SHOWWINDOW()
	ON_WM_CHAR()
	ON_EN_CHANGE(IDC_FILENAMEMASKEDIT, OnEnChangeFilenamemaskedit)
	ON_BN_CLICKED(IDC_BUTTONSTRIP, OnBnClickedButtonStrip) //MORPH - Added by SiRoB, Clean MassRename
	ON_BN_CLICKED(IDC_SIMPLECLEANUP, OnBnClickedSimplecleanup)
	ON_BN_CLICKED(IDC_INSERTTEXTCOLUMN, OnBnClickedInserttextcolumn)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CMassRenameDialog::OnOK() {
}

void CMassRenameDialog::OnCancel() {
}

void CMassRenameDialog::Localize() {
	GetDlgItem (IDC_MR_STATIC1)->SetWindowText (GetResString (IDS_MASSRENAME1));
	GetDlgItem (IDC_MR_STATIC2)->SetWindowText (GetResString (IDS_MASSRENAME2));
	GetDlgItem (IDC_MR_STATIC3)->SetWindowText (GetResString (IDS_MASSRENAME3));
	GetDlgItem (IDC_FILENAMELEFT)->SetWindowText (GetResString (IDS_FILENAMELEFT));
	GetDlgItem (IDC_FILENAMERIGHT)->SetWindowText (GetResString (IDS_FILENAMERIGHT));

	GetDlgItem (IDC_BUTTONSTRIP)->SetWindowText (GetResString (IDS_CLEANUP));
	GetDlgItem (IDC_SIMPLECLEANUP)->SetWindowText (GetResString (IDS_SIMPLECLEANUP));
	GetDlgItem (IDC_INSERTTEXTCOLUMN)->SetWindowText (GetResString (IDS_INSERTTEXTCOLUMN));
	GetDlgItem (IDC_RESETBUTTON)->SetWindowText (GetResString (IDS_RESETFILENAMES));
	GetDlgItem (IDOK)->SetWindowText (GetResString(IDS_TREEOPTIONS_OK));
	GetDlgItem (IDCANCEL)->SetWindowText (GetResString (IDS_CANCEL));
}

BOOL CMassRenameDialog::OnInitDialog() {
	CResizableDialog::OnInitDialog();

	Localize ();

	InitWindowStyles(this);
	SetIcon(theApp.LoadIcon(_T("FILEMASSRENAME"),16,16),FALSE);
	SetWindowText(GetResString(IDS_MASSRENAME_TITLE));

	// Go through the list of files, collect all filenames to one string and
	// show it in the list of the old filenames
	POSITION pos = m_FileList.GetHeadPosition();
	CString FileListString;
	while (pos != NULL) {
		CKnownFile* file = m_FileList.GetAt (pos);
		if (FileListString=="") FileListString = file->GetFileName ();
		else FileListString = FileListString + _T("\r\n") + file->GetFileName ();
		m_FileList.GetNext (pos);
	}
	OldFN = (CRichEditCtrl*) GetDlgItem(IDC_OLDFILENAMESEDIT);
	NFNLeft = (CRichEditCtrl*) GetDlgItem(IDC_NEWFILENAMESEDITLEFT);
	NFNRight = (CRichEditCtrl*) GetDlgItem(IDC_NEWFILENAMESEDITRIGHT);
	
	// Don't allow formatted text, since we are only editing filenames...
	NFNLeft->SetTextMode (TM_PLAINTEXT | TM_MULTILEVELUNDO | TM_MULTICODEPAGE);
	NFNRight->SetTextMode (TM_PLAINTEXT | TM_MULTILEVELUNDO | TM_MULTICODEPAGE);
	OldFN->SetTextMode (TM_PLAINTEXT | TM_MULTILEVELUNDO | TM_MULTICODEPAGE);

	// Insert the starting text
	OldFN->SetWindowText (FileListString);
	NFNLeft->SetWindowText (FileListString);
	NFNRight->SetWindowText (FileListString);

	LastEditPos = -10;
	LastDelPos = -10;
	InDel = false;
	LastEditWasUndo = false;
	UndoBuffer = FileListString;

	// Show the left justify edit control
	NFNLeft->ModifyStyle (0,WS_VISIBLE);
	NFNRight->ModifyStyle (WS_VISIBLE,0);

	CheckDlgButton (IDC_FILENAMELEFT,BST_CHECKED);

	// Create a new CEdit to replace the IDC_FILENAMEMASKEDIT edit control
	CEdit* FNEdit = (CEdit*) GetDlgItem(IDC_FILENAMEMASKEDITTEMPLATE);
	/*int Style = FNEdit->GetStyle ();
	int ExStyle = FNEdit->GetExStyle ();*/
	// Hide the old control and show the new one instead
	FNEdit->ModifyStyle (WS_VISIBLE,0);
	MassRenameEdit = new CMassRenameEdit;
	MassRenameEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
      CRect (6,26,474,49), this, IDC_FILENAMEMASKEDIT);
	// Take the font of the old control
	MassRenameEdit->SetFont (FNEdit->GetFont ());

	m_DontTrackKeys = false;

	// activate windows messages for scrolling events in the filename window(s)
	NFNLeft->SetEventMask(NFNLeft->GetEventMask() | ENM_SCROLL);
	NFNRight->SetEventMask(NFNRight->GetEventMask() | ENM_SCROLL);
	OldFN->SetEventMask(OldFN->GetEventMask() | ENM_SCROLL);

    AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDC_MR_STATIC1, TOP_LEFT);
	AddAnchor(IDC_FILENAMEMASKEDITTEMPLATE , TOP_LEFT, TOP_CENTER);
	AddAnchor(IDC_MR_STATIC2, CSize(50,0)); //Original filenames
	AddAnchor(IDC_MR_STATIC3, TOP_LEFT);//  "New filenames:"
	AddAnchor(IDC_FILENAMELEFT, TOP_LEFT);
	AddAnchor(IDC_FILENAMERIGHT, TOP_LEFT);
	AddAnchor(IDC_RESETBUTTON, TOP_RIGHT);
	AddAnchor(IDC_NEWFILENAMESEDITLEFT, TOP_LEFT, CSize(50,100));
	AddAnchor(IDC_NEWFILENAMESEDITRIGHT, TOP_LEFT, CSize(50,100));
	AddAnchor(IDC_OLDFILENAMESEDIT, CSize(50,0), BOTTOM_RIGHT);
	
	AddAnchor(IDC_BUTTONSTRIP, BOTTOM_LEFT);
	AddAnchor(IDC_SIMPLECLEANUP, BOTTOM_LEFT);
	AddAnchor(IDC_INSERTTEXTCOLUMN, BOTTOM_LEFT);
	
	return TRUE;
}

void CMassRenameDialog::OnClose()
{
	CResizableDialog::OnClose();
}

void CMassRenameDialog::OnEnVscrollOldfilenamesedit()
{
	// Scroll the "new filename" windows to the correct position
	int VDiff = OldFN->GetFirstVisibleLine ()-NFNLeft->GetFirstVisibleLine();
	if (VDiff != 0) NFNLeft->LineScroll (VDiff,0);
	VDiff = OldFN->GetFirstVisibleLine ()-NFNRight->GetFirstVisibleLine();
	if (VDiff != 0) NFNRight->LineScroll (VDiff,0);
}

void CMassRenameDialog::OnEnVscrollNewfilenameseditLeft()
{
	// Scroll the "old filename" windows to the correct position
	int VDiff = NFNLeft->GetFirstVisibleLine()-OldFN->GetFirstVisibleLine ();
	if (VDiff != 0) OldFN->LineScroll (VDiff,0);
}

void CMassRenameDialog::OnEnVscrollNewfilenameseditRight()
{
	// Scroll the "old filename" windows to the correct position
	int VDiff = NFNRight->GetFirstVisibleLine()-OldFN->GetFirstVisibleLine ();
	if (VDiff != 0) OldFN->LineScroll (VDiff,0);
}

//typedef vector<bool> fnvector;

void CMassRenameDialog::OnBnClickedMassrenameok()
{
	// First check for duplicate and invalid filenames - otherwise the dialog
	// won't close !
	
	// Create a sorted list, add all filenames with path uppercase. If there are
	// duplicate filenames, that's not correct. Also if there's an empty filename
	// or if there are not enough filenames in the window, this is not correct.

	bool RightJustify = !IsDlgButtonChecked (IDC_FILENAMELEFT);
	CRichEditCtrl* NFNEdit = NFNLeft;
	if (RightJustify) {
		NFNEdit = NFNRight;
	}

	if (NFNEdit->GetLineCount () < m_FileList.GetCount()) {
		AfxMessageBox (GetResString (IDS_NOTENOUGHFILENAMES),
					   MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	m_NewFilenames.clear(); // Clear the list of target filenames and rebuild it

	// Create new strings and insert them into a sList
	std::vector<CString> sList;
	POSITION fpos = m_FileList.GetHeadPosition();
	int i = 0;
	while (fpos != NULL) {
		CString FName; int newcount;
		newcount=NFNEdit->GetLine (i,FName.GetBuffer (MAX_PATH+1),MAX_PATH);
		FName.ReleaseBuffer (newcount);
		FName.Trim (_T("\r"));
		FName.Trim (' ');
		if ((FName=="") || (FName==".") || (FName=="..") || (FName.FindOneOf (_T(":\\?*")) >= 0)){
			CString er;
			er.Format (_T("Invalid filename in line %d. Rename not possible."),i+1);
			AfxMessageBox (er,MB_OK|MB_ICONEXCLAMATION);
			return;
		}
		CString path;
		CKnownFile* file = m_FileList.GetAt (fpos);
		PathCombine (path.GetBuffer (MAX_PATH),file->GetPath(),FName);

		// Add to the list of new filenames. This will be the result for the
		// caller of this dialog if all checks are ok.
		m_NewFilenames.push_back (FName);
		m_NewFilePaths.push_back (path);

		path.ReleaseBuffer ();
		path.MakeUpper ();
		sList.push_back (path);

		m_FileList.GetNext (fpos);
		i++;
	}

	// Sort the stringlist to check for duplicate items
	std::sort (sList.begin(),sList.end());

	// Check for duplicate filenames
	for (int i=1; i < (int) sList.size(); i++) {
		if (sList.at (i-1) == sList.at (i)) {
			CString er;
			er.Format (GetResString (IDS_IDENTICALFILENAMES),i+1);
			AfxMessageBox (er,MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}

	// Everything is ok, the caller can take m_NewFilenames to rename the files.
	EndDialog (IDOK);
}

void CMassRenameDialog::OnBnClickedMassrenamecancel()
{
	EndDialog (IDCANCEL);
}

// Copy the first line of the New-filenames edit field to the Mask edit-field
void CMassRenameDialog::UpdateEditMask()
{
	m_DontTrackKeys = true;
	// Take the first line of the "New filenames" edit control as a mask for all filenames
	CString FirstLine;
	NFNLeft->GetWindowText (FirstLine);
	bool RightJustify = !IsDlgButtonChecked (IDC_FILENAMELEFT);
	if (RightJustify) 
		NFNRight->GetWindowText (FirstLine);
	int i = FirstLine.Find ('\r');
	if (i != -1) {
		FirstLine = FirstLine.Left (i);
	}
	GetDlgItem (IDC_FILENAMEMASKEDIT)->SetWindowText (FirstLine);
	m_DontTrackKeys = false;
}

void CMassRenameDialog::OnEnSetfocusFilenamemaskedit()
{
	UpdateEditMask ();
	
	m_LastFocusedEdit = GetDlgItem (IDC_FILENAMEMASKEDIT);
}

// Remember the Edit control that gained the focus at last
void CMassRenameDialog::OnEnSetfocusRichEdit()
{
	m_LastFocusedEdit = GetFocus ();
}

void CMassRenameDialog::OnBnClickedFilenameleft()
{
	// Show the left justify edit control
	NFNLeft->ModifyStyle (0,WS_VISIBLE);
	NFNRight->ModifyStyle (WS_VISIBLE,0);
	CString txt;
	NFNRight->GetWindowText (txt);
	NFNLeft->SetWindowText (txt);

	Invalidate();
}

void CMassRenameDialog::OnBnClickedFilenameright()
{
	// Show the right justify edit control
	NFNLeft->ModifyStyle (WS_VISIBLE,0);
	NFNRight->ModifyStyle (0,WS_VISIBLE);
	CString txt;
	NFNLeft->GetWindowText (txt);
	NFNRight->SetWindowText (txt);

	Invalidate();
}

void CMassRenameDialog::OnBnClickedReset()
{
	CString txt;
	OldFN->GetWindowText (txt);
	NFNLeft->SetWindowText (txt);
	NFNRight->SetWindowText (txt);
	UndoBuffer = txt;
	InDel = false;
	LastEditWasUndo = false;
	LastDelPos = -10;
	OnEnSetfocusFilenamemaskedit();
}

void CMassRenameDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CResizableDialog::OnShowWindow(bShow, nStatus);

	if (bShow) GetDlgItem(IDC_FILENAMEMASKEDIT)->SetFocus ();
}


void CMassRenameDialog::OnEnChangeFilenamemaskedit()
{
	if (m_DontTrackKeys) return;
	
	CString AfterEdit;
	int Start1,End1,Start2,End2;
	int StartR1,EndR1,StartR2,EndR2;
	
	MassRenameEdit->GetSel (Start2,End2);
	MassRenameEdit->GetWindowText (AfterEdit);
	Start1 = MassRenameEdit->Start1;
	End1 = MassRenameEdit->End1;
	bool RightJustify = !IsDlgButtonChecked (IDC_FILENAMELEFT);

	int BeforeEditLen = MassRenameEdit->m_BeforeEdit.GetLength ();

	StartR1 = BeforeEditLen-Start1;
	StartR2 = BeforeEditLen-Start2;
	EndR1 = BeforeEditLen-End1;
	EndR2 = BeforeEditLen-End2;

	CRichEditCtrl* NFNEdit = NFNLeft;
	if (RightJustify) {
		NFNEdit = NFNRight;
	}

	CString allFNText;
	NFNEdit->GetWindowText (allFNText);

	if (Start1==-5) {
		// There's an UNDO goin on...
		NFNLeft->SetWindowText (UndoBuffer);
		NFNRight->SetWindowText (UndoBuffer);
		UndoBuffer = allFNText;
		LastEditWasUndo = true;
		return; // Cancel here
	}

	if (LastEditWasUndo) {
		UndoBuffer = allFNText;
		LastEditWasUndo = false;
	}

	if ((MassRenameEdit->Start1==End1) && (Start2==End2) && (Start2 <= Start1)) {
		if (!InDel) {
			// We start deleting characters, so save the old content for Undo
			UndoBuffer = allFNText;
			LastDelPos = Start2;
			InDel = true;
		} else {
			// We have already deleted some characters. Now we have to figure out
			// if the new character to delete is on the same block of characters
			// like the others. Otherwise the user started to delete characters on another
			// position and we have to renew our UNDO buffer.
			if (!((Start2 == LastDelPos) || (Start2==(LastDelPos-1))))
				UndoBuffer = allFNText;  // Group deletitions for UNDO
			LastDelPos = Start2;
		}
		// DEL or ENTF pressed; remove 1 character on position Start2
		for (int i=0; i < NFNEdit->GetLineCount (); i++) {
			int lstart = NFNEdit->LineIndex (i);
			int llen = NFNEdit->LineLength (lstart);
			bool nochange=false;  // Don't edit if position is not in the current line
			if (!RightJustify) {
				NFNEdit->SetSel (lstart+Start2,lstart+Start2+1);
				if (lstart+Start2 >= lstart+llen) nochange = true;
			} else {
				NFNEdit->SetSel (lstart+llen-StartR2,lstart+llen-StartR2+1);
				if (llen-StartR2 < 0) nochange = true;
			}
			if (!nochange) NFNEdit->ReplaceSel (_T(""));
		}
	} else {
		CString NewChars = AfterEdit.Mid (Start1,End2-Start1);
		if (NewChars=="") {
			// Oh, that's dangerous. The user replaced the string by "" which is
			// a delete action, not an insert action !
			// In this case the Insert-routine has actually to be a Delete-routine !
			if (!InDel)
				UndoBuffer = allFNText;
			InDel = true;
			LastDelPos = Start1;
		} else if (LastEditWasUndo) { // Save the old content for Undo
			UndoBuffer = allFNText;
			LastEditPos = Start1;
			InDel = false;
			LastEditWasUndo = false;
		} else if (InDel) {
			if (!(Start1 == LastDelPos))
				UndoBuffer = allFNText;
			LastEditPos = Start1;
			InDel = false;
		} else {
			if (!(Start1 == (LastEditPos+1)))
				UndoBuffer = allFNText;
			LastEditPos = Start1;
		}
		// Remove some characters Start1..End1 and replace them by Start1..End2
		for (int i=0; i < NFNEdit->GetLineCount (); i++) {
			int lstart = NFNEdit->LineIndex (i);
			int llen = NFNEdit->LineLength (lstart);
			bool nochange=false;  // Don't edit if position is not in the current line
			if (!RightJustify) {
				NFNEdit->SetSel (lstart+Start1,lstart+min(End1,llen));
				if (lstart+Start1 > lstart+llen) nochange = true;
			} else {
				NFNEdit->SetSel (lstart+max(llen-StartR1,0),lstart+llen-EndR1);
				if (llen-StartR1 < 0) nochange = true;
			}
			if (!nochange) NFNEdit->ReplaceSel (NewChars);
		}
	}
}

void CMassRenameDialog::OnBnClickedButtonStrip()
{
	// Choose the current active RichEdit - the one which shows the filenames
	// left justified or right justified
	CRichEditCtrl* NFNEdit = NFNLeft;
	bool RightJustify = !IsDlgButtonChecked (IDC_FILENAMELEFT);
	if (RightJustify) {
		NFNEdit = NFNRight;
	}

	CString filenames;

	// Now process through each line and cleanup that filename
	for (int i=0; i < NFNEdit->GetLineCount (); i++) {
		// Get the filename
		CString filename;int newcount;
		newcount=NFNEdit->GetLine (i,filename.GetBuffer (MAX_PATH+1),MAX_PATH);
		filename.ReleaseBuffer(newcount);
		// Clean it up
		filename = CleanupFilename (filename);
		// and add it to the current list of filenames
		if (filenames != "") filenames += "\r\n";
		filenames += filename;
	}

	// at the end save the list of filenames to the Richedit controls
	NFNLeft->SetWindowText( filenames );
	NFNRight->SetWindowText( filenames );
}

// A "simple" version for cleaning up filenames - only removes ".", "_", "\r" and "\n".
CString SimpleCleanupFilename (CString _filename) {
	// The last "." must not be replaced - it's the separator for the extension!
	int lastdot = _filename.ReverseFind ('.');
	for (int i=0; i < _filename.GetLength(); i++) {
		switch (_filename.GetAt (i)) {
			case '.':
			case '_': {
				if (i != lastdot) _filename.SetAt (i,' ');
			}
		}
	}
	// Strip "\r" and "\n"
	return _filename.SpanExcluding (_T("\r\n"));
}


void CMassRenameDialog::OnBnClickedSimplecleanup()
{
	//Xman changed: use the simple method
	/* // Simple cleanup [MorphXT] - Stulle
	// Choose the current active RichEdit - the one which shows the filenames
	// left justified or right justified
	CRichEditCtrl* NFNEdit = NFNLeft;
	bool RightJustify = !IsDlgButtonChecked (IDC_FILENAMELEFT);
	if (RightJustify) {
		NFNEdit = NFNRight;
	}

	CString filenames;

	// Now process through each line and cleanup that filename
	for (int i=0; i < NFNEdit->GetLineCount (); i++) {
		// Get the filename
		CString filename;int newcount;
		newcount= NFNEdit->GetLine (i,filename.GetBuffer (MAX_PATH+1),MAX_PATH);
		filename.ReleaseBuffer(newcount);
		// Clean it up
		filename = SimpleCleanupFilename (filename);
		// and add it to the current list of filenames
		if (filenames != "") filenames += "\r\n";
		filenames += filename;
	}

	// at the end save the list of filenames to the Richedit controls
	NFNLeft->SetWindowText( filenames );
	NFNRight->SetWindowText( filenames );
	*/ // Simple cleanup [MorphXT] - Stulle

//	/* Simple cleanup [MorphXT] - Stulle
	CSimpleCleanupDialog sclean;
	sclean.SetConfig (thePrefs.GetSimpleCleanupOptions (),
		thePrefs.GetSimpleCleanupSearch (),
		thePrefs.GetSimpleCleanupReplace (),
		thePrefs.GetSimpleCleanupSearchChars (),
		thePrefs.GetSimpleCleanupReplaceChars ());
	if (sclean.DoModal()==IDOK) {
		// Get the config how to perform the cleanup
		int options;
		CString source, dest;
		CString sourcechar, destchar;
		sclean.GetConfig (options,source,dest,sourcechar,destchar);

		// Save the options in the preferences 
		thePrefs.SetSimpleCleanupOptions (options);
		thePrefs.SetSimpleCleanupSearch (source);
		thePrefs.SetSimpleCleanupReplace (dest);
		thePrefs.SetSimpleCleanupSearchChars (sourcechar);
		thePrefs.SetSimpleCleanupReplaceChars (destchar);

		// Choose the current active RichEdit - the one which shows the filenames
		// left justified or right justified
		CRichEditCtrl* NFNEdit = NFNLeft;
		bool RightJustify = !IsDlgButtonChecked (IDC_FILENAMELEFT);
		if (RightJustify) {
			NFNEdit = NFNRight;
		}

		CString filenames;

		// Now process through each line and cleanup that filename
		for (int i=0; i < NFNEdit->GetLineCount (); i++) {
			// Get the filename
			CString filename;int newcount;
			newcount=NFNEdit->GetLine (i,filename.GetBuffer (MAX_PATH+1),MAX_PATH);
			filename.ReleaseBuffer(newcount);
			// Clean it up
			filename = SimpleCleanupFilename (filename.SpanExcluding (_T("\r\n")),
											  options,source,dest,
											  sourcechar,destchar);
			// and add it to the current list of filenames
			if (filenames != "") filenames += "\r\n";
			filenames += filename;
		}

		// at the end save the list of filenames to the Richedit controls
		NFNLeft->SetWindowText( filenames );
		NFNRight->SetWindowText( filenames );

	}
//	*/ Simple cleanup [MorphXT] - Stulle
}

void CMassRenameDialog::OnBnClickedInserttextcolumn()
{
	// Choose the current active RichEdit - the one which shows the filenames
	// left justified or right justified
	CRichEditCtrl* NFNEdit = NFNLeft;
	bool RightJustify = !IsDlgButtonChecked (IDC_FILENAMELEFT);
	if (RightJustify) {
		NFNEdit = NFNRight;
	}

	// Prepare the UNDO-Buffer; for future implementation since the Windows Undo handling
	// is very crazy...
	LastEditWasUndo = true;
	CString allFNText;
	NFNEdit->GetWindowText (allFNText);
	UndoBuffer = allFNText;
	InDel = false;

	// Now determine where we have to insert the clipboard data.
	// If the "EditMask" edit line is selected, start from the top.
	// If the cursor is placed in the RichEdit with all the filenames, we insert
	// the data starting at the current cursor position.
	// StartX receives the X-Position in the current line where to start inserting,
	// EndX receives the end position of the current selection. If there's text
	// selected in the current line, this will be deleted in every line!
	int StartX=0, EndX = 0;
	int StartLine = 0, CurrentLineLength = 0;
	MassRenameEdit->GetSel (StartX, EndX);
	CurrentLineLength = MassRenameEdit->LineLength();

	// Determine the last focused Edit control with the help of m_LastFocusedEdit.
	// This trick has to be used because windows even changes the focus to the button
	// when hitting it with the mouse.
	if (m_LastFocusedEdit != MassRenameEdit) {
		// Get the cursor position from the RichEdit-control
		long StartXL=0, EndXL=0;
		NFNEdit->GetSel (StartXL, EndXL);
		StartLine = NFNEdit->LineFromChar (StartXL);
		// It's not allowed to select multple lines here!
		if (NFNEdit->LineFromChar (EndXL) != StartLine) EndXL = StartXL;
		// Correct the Index positions concernung the start position of the current line
		StartXL -= NFNEdit->LineIndex ();
		EndXL -= NFNEdit->LineIndex ();

		CurrentLineLength = NFNEdit->LineLength();

		StartX = StartXL;
		EndX = EndXL;
	}

	// If we are in "Right-justify" mode, invert start and end coordinates
	if (RightJustify) {
		StartX = CurrentLineLength - StartX;
		EndX = CurrentLineLength - EndX;
		// From now on these coordinates have to be treated inverted!
	}

	// Get the content of the clipboard
	CString ClipboardData;
	OpenClipboard ();
	HGLOBAL hglb = ::GetClipboardData( CF_TEXT );
    if (hglb != NULL) { 
        LPSTR str;
		str = (LPSTR) GlobalLock(hglb); 
        if (str != NULL) { 
			// Get the data
			ClipboardData = str;
            GlobalUnlock(hglb); 
        } 
    } 
    CloseClipboard(); 

	int clstart=0;
	int clend=0;

	// Remove any CR characters
	ClipboardData.Remove ('\r');

	// Only proceed if there's something to insert
	if (!ClipboardData.IsEmpty ()) {

		CString filenames;

		// Now process through each line and cleanup that filename
		for (int i=0; i < NFNEdit->GetLineCount (); i++) {
			// Get the filename
			CString filename;int newcount;
			newcount=NFNEdit->GetLine (i,filename.GetBuffer (MAX_PATH+1),MAX_PATH);
			filename.ReleaseBuffer(newcount);

			// Remove "\r\n" from the current filename if necessary
			filename = filename.SpanExcluding (_T("\r\n"));

			// We only proceed those lines that are larger than our StartLine - all
			// other filenames have to be copied.
			// Apart of that if there's no mor data in the clipboard buffer, we are 
			// finished and only have to copy all the other lines.
			if ((i >= StartLine) && (clstart <= ClipboardData.GetLength ())) {

				// Delete the marked characters from the filename (if anything is marked) and
				// insert the next part of the clipboard at the current cursor position
				clend = ClipboardData.Find (_T("\n"),clstart);
				if (clend==-1) clend = ClipboardData.GetLength ();
				// Extract the characters to be inserted. Make sure not to copy the
				// trailing "\n"!
				CString NextChars = ClipboardData.Mid (clstart,clend-clstart);
				CurrentLineLength = filename.GetLength ();
				if (!RightJustify) {
					// Delete the marked characters
					filename.Delete (StartX, EndX-StartX);
					// Insert this line into the filename
					filename.Insert (StartX,NextChars);
				} else {
					// Insert this line into the filename
					filename.Insert (CurrentLineLength - StartX, NextChars);
					// Delete the marked characters; the length of the 
					// string has changed!
					filename.Delete (filename.GetLength () - StartX, StartX-EndX);
				}
				// Move the clipboard cursor position to the next line; skip "\n"
				clstart = clend+1;
			}

			// and add it to the current list of filenames
			if (filenames != "") filenames += "\r\n";
			filenames += filename;

		}

		// at the end save the list of filenames to the Richedit controls
		NFNLeft->SetWindowText( filenames );
		NFNRight->SetWindowText( filenames );

		// update the MaskEdit-control
		UpdateEditMask ();
	}

}

