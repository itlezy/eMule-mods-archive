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

// Original file: Written by Mighty Knife, EMule Morph Team

#include "stdafx.h"
#include "emule.h"
#include "resource.h"
#include "Preferences.h"
#include "MassRename.h"
#include "Partfile.h"
#include "Sharedfilelist.h"
#include "OtherFunctions.h"
#include "downloadqueue.h"
#include "knownfilelist.h"
#include "massrename.h"
#include "Log.h"
#include "UserMsgs.h"
#include "Neo/Ini2.h" // NEO: INI - [PlusIniClass] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define IDC_FILENAMEMASKEDIT 100

// CMassRenameEdit Edit control

BEGIN_MESSAGE_MAP(CMassRenameEdit, CEdit)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_MESSAGE(WM_PASTE,OnPaste)
	ON_MESSAGE(WM_UNDO,OnUndo)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CMassRenameEdit, CEdit)

void CMassRenameEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Get the current string before Edit and the selection - then default message processing
	GetSel (Start1,End1);
	GetWindowText (m_BeforeEdit);
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CMassRenameEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Get the current string before Edit and the selection - then default message processing	if (Start1!=-1) {
	GetSel (Start1,End1);
	GetWindowText (m_BeforeEdit);
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

LRESULT CMassRenameEdit::OnPaste(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Get the current string before Edit and the selection - then default message processing
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

////////////////////////////////
// CMassRenameDialog


IMPLEMENT_DYNAMIC(CMassRenameDialog, CDialog) // NEO: MLD - [ModelesDialogs]
CMassRenameDialog::CMassRenameDialog()
	: CModResizableDialog(CMassRenameDialog::IDD) // NEO: MLD - [ModelesDialogs]
{
	SimpleDlg = NULL;
	MassRenameEdit = NULL;
}

CMassRenameDialog::~CMassRenameDialog()
{
	m_FileList.RemoveAll();

	// Delete the CMassRenameEdit object we allocated in OnInitDialog
	if(MassRenameEdit)
		delete MassRenameEdit;
}

void CMassRenameDialog::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_OLDFILENAMESEDIT, OldFN);
//Lit>
//	DDX_Control(pDX, IDC_NEWFILENAMESEDITLEFT, NewFN);
	DDX_Control(pDX, IDC_NEWFILENAMESEDITLEFT, NewFNLeft);
	DDX_Control(pDX, IDC_LIT_NEWFILENAMESEDITRIGHT, NewFNRight);
//Lit<

	CModResizableDialog::DoDataExchange(pDX); // NEO: MLD - [ModelesDialogs]
}

BEGIN_MESSAGE_MAP(CMassRenameDialog, CModResizableDialog) // NEO: MLD - [ModelesDialogs]
	ON_EN_VSCROLL(IDC_OLDFILENAMESEDIT, OnEnVscrollOldfilenamesEdit)
	ON_EN_VSCROLL(IDC_NEWFILENAMESEDIT, OnEnVscrollNewfilenamesEdit)
	ON_BN_CLICKED(IDOK, OnBnClickedMassrenameOk)
	ON_EN_SETFOCUS(IDC_FILENAMEMASKEDIT, OnEnSetfocusFilenamemaskEdit)
	ON_EN_SETFOCUS(IDC_NEWFILENAMESEDIT, OnEnSetfocusRichEdit)
//Lit>
	ON_EN_SETFOCUS(IDC_LIT_NEWFILENAMESEDITRIGHT, OnEnSetfocusRichEdit)
	ON_EN_VSCROLL(IDC_LIT_NEWFILENAMESEDITRIGHT, OnEnVscrollNewfilenamesEditRight)
	ON_BN_CLICKED(IDC_LIT_MASSRENLEFT, OnBnClickedFilenameleft)
	ON_BN_CLICKED(IDC_LIT_MASSRENRIGHT, OnBnClickedFilenameright)
//Lit<
	ON_BN_CLICKED(IDC_RESETBUTTON, OnBnClickedReset)
	ON_WM_SHOWWINDOW()
	ON_WM_CHAR()
	ON_EN_CHANGE(IDC_FILENAMEMASKEDIT, OnEnChangeFilenamemaskEdit)
	ON_BN_CLICKED(IDC_BUTTONSTRIP, OnBnClickedButtonStrip) //MORPH - Added by SiRoB, Clean MassRename
	ON_BN_CLICKED(IDC_SIMPLECLEANUP, OnBnClickedSimplecleanup)
	ON_BN_CLICKED(IDC_INSERTTEXTCOLUMN, OnBnClickedInserttextcolumn)
	ON_WM_CLOSE()
	ON_MESSAGE(UM_SIMPLE_CLEANUP_NOTIFY, SimpleCleanupNotify)
END_MESSAGE_MAP()


void CMassRenameDialog::Localize()
{
	GetDlgItem (IDC_MR_STATIC1)->SetWindowText (GetResString (IDS_X_MASSRENAME1));
	GetDlgItem (IDC_MR_STATIC2)->SetWindowText (GetResString (IDS_X_MASSRENAME2));
	GetDlgItem (IDC_MR_STATIC3)->SetWindowText (GetResString (IDS_X_MASSRENAME3));

	GetDlgItem (IDC_BUTTONSTRIP)->SetWindowText (GetResString (IDS_CLEANUP));
	GetDlgItem (IDC_SIMPLECLEANUP)->SetWindowText (GetResString (IDS_X_SIMPLECLEANUP));
	GetDlgItem (IDC_INSERTTEXTCOLUMN)->SetWindowText (GetResString (IDS_X_INSERTTEXTCOLUMN));
	GetDlgItem (IDC_RESETBUTTON)->SetWindowText (GetResString (IDS_X_RESETFILENAMES));
	GetDlgItem (IDCANCEL)->SetWindowText (GetResString (IDS_CANCEL));
}

BOOL CMassRenameDialog::OnInitDialog()
{
	CModResizableDialog::OnInitDialog(); // NEO: MLD - [ModelesDialogs]

	Localize();

	InitWindowStyles(this);
	SetIcon(theApp.LoadIcon(_T("FILEMASSRENAME"),16,16),FALSE);
	SetWindowText(GetResString(IDS_X_MASSRENAME_TITLE));

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

	
	// Don't allow formatted text, since we are only Editing filenames...
//Lit>
//	NewFN.SetTextMode (TM_PLAINTEXT | TM_MULTILEVELUNDO | TM_MULTICODEPAGE);
	NewFNLeft.SetTextMode (TM_PLAINTEXT | TM_MULTILEVELUNDO | TM_MULTICODEPAGE);
	NewFNRight.SetTextMode (TM_PLAINTEXT | TM_MULTILEVELUNDO | TM_MULTICODEPAGE);
//Lit<
	OldFN.SetTextMode (TM_PLAINTEXT | TM_MULTILEVELUNDO | TM_MULTICODEPAGE);

	// Insert the starting text
	OldFN.SetWindowText (FileListString);
//Lit>
//	NewFN.SetWindowText (FileListString);
	NewFNLeft.SetWindowText (FileListString);
	NewFNRight.SetWindowText (FileListString);
//Lit<

	LastEditPos = -10;
	LastDelPos = -10;
	InDel = false;
	LastEditWasUndo = false;
	UndoBuffer = FileListString;

	// Show the left justify Edit control
//Lit>
//	NewFN.ModifyStyle (0,WS_VISIBLE);
	NewFNLeft.ModifyStyle (0,WS_VISIBLE);
	NewFNRight.ModifyStyle (WS_VISIBLE,0);
//Lit<

//Lit>
	CheckDlgButton (IDC_LIT_MASSRENLEFT,BST_CHECKED);
//Lit<

	// Create a new CEdit to replace the IDC_FILENAMEMASKEDIT Edit control
	CEdit* FNEdit = (CEdit*) GetDlgItem(IDC_FILENAMEMASKEDITTEMPLATE);
	/*int Style = FNEdit->GetStyle ();
	int ExStyle = FNEdit->GetExStyle ();*/
	// Hide the old control and show the new one instead
	FNEdit->ModifyStyle (WS_VISIBLE,0);
	MassRenameEdit = new CMassRenameEdit;
	MassRenameEdit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,CRect (6,26,474,49), this, IDC_FILENAMEMASKEDIT);
	// Take the font of the old control
	MassRenameEdit->SetFont (FNEdit->GetFont ());

	m_DontTrackKeys = false;

	// activate windows messages for scrolling events in the filename window(s)
//Lit>
	//NewFN.SetEventMask(NewFN.GetEventMask() | ENM_SCROLL);
	NewFNLeft.SetEventMask(NewFNLeft.GetEventMask() | ENM_SCROLL);
	NewFNRight.SetEventMask(NewFNRight.GetEventMask() | ENM_SCROLL);
//Lit<
	OldFN.SetEventMask(OldFN.GetEventMask() | ENM_SCROLL);

	// add windows resizing support
	AddAnchor(IDC_FILENAMEMASKEDITTEMPLATE, TOP_LEFT, TOP_RIGHT);

	AddAnchor(IDC_NEWFILENAMESEDIT, TOP_LEFT, BOTTOM_CENTER);
//Lit>
	AddAnchor(IDC_LIT_NEWFILENAMESEDITRIGHT, TOP_LEFT, BOTTOM_CENTER);
//Lit<
	AddAnchor(IDC_OLDFILENAMESEDIT, TOP_CENTER, BOTTOM_RIGHT);
	AddAnchor(IDC_MR_STATIC2, TOP_CENTER);
	AddAnchor(IDC_FILENAMEMASKEDIT, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_RESETBUTTON, TOP_RIGHT);

	AddAnchor(IDC_BUTTONSTRIP, BOTTOM_LEFT);
	AddAnchor(IDC_SIMPLECLEANUP, BOTTOM_LEFT);
	AddAnchor(IDC_INSERTTEXTCOLUMN, BOTTOM_LEFT);

	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);

	EnableSaveRestore(_T("MassRename"));

	return TRUE;
}

void CMassRenameDialog::OnClose()
{
	// NEO: MLD - [ModelesDialogs]
	if(SimpleDlg)
		delete SimpleDlg;
	// NEO: MLD END
	CModResizableDialog::OnClose(); // NEO: MLD - [ModelesDialogs]
}

void CMassRenameDialog::OnEnVscrollOldfilenamesEdit()
{
	// Scroll the "new filename" windows to the correct position
//Lit>
//	int VDiff = OldFN.GetFirstVisibleLine ()-NewFN.GetFirstVisibleLine();
//	if (VDiff != 0) NewFN.LineScroll (VDiff,0);
	int VDiff = OldFN.GetFirstVisibleLine ()-NewFNLeft.GetFirstVisibleLine();
	if (VDiff != 0) NewFNLeft.LineScroll (VDiff,0);
	VDiff = OldFN.GetFirstVisibleLine ()-NewFNRight.GetFirstVisibleLine();
	if (VDiff != 0) NewFNRight.LineScroll (VDiff,0);
//Lit<
}

void CMassRenameDialog::OnEnVscrollNewfilenamesEdit()
{
	// Scroll the "old filename" windows to the correct position
//Lit>
//	int VDiff = NewFN.GetFirstVisibleLine()-OldFN.GetFirstVisibleLine ();
//	if (VDiff != 0) OldFN.LineScroll (VDiff,0);
	int VDiff = NewFNLeft.GetFirstVisibleLine()-OldFN.GetFirstVisibleLine ();
	if (VDiff != 0) OldFN.LineScroll (VDiff,0);
//Lit<
}

//Lit>
void CMassRenameDialog::OnEnVscrollNewfilenamesEditRight()
{
	// Scroll the "old filename" windows to the correct position
	int VDiff = NewFNRight.GetFirstVisibleLine()-OldFN.GetFirstVisibleLine ();
	if (VDiff != 0) OldFN.LineScroll (VDiff,0);
}
//Lit<

void CMassRenameDialog::OnBnClickedMassrenameOk()
{
//Lit>
	bool RightJustify = !IsDlgButtonChecked (IDC_LIT_MASSRENLEFT);
	CRichEditCtrl &NewFN = RightJustify ? NewFNRight : NewFNLeft;
//Lit<
	// The following list is the list of target filenames with path for the files
	// in m_FileList:
	std::vector<CString> NewFilenames;
	std::vector<CString> NewFilePaths;

	// First check for duplicate and invalid filenames - otherwise the dialog
	// won't close !
	
	// Create a sorted list, add all filenames with path uppercase. If there are
	// duplicate filenames, that's not correct. Also if there's an empty filename
	// or if there are not enough filenames in the window, this is not correct.

	if (NewFN.GetLineCount () < m_FileList.GetCount()) {
		AfxMessageBox (GetResString (IDS_X_NOTENOUGHFILENAMES),MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	NewFilenames.clear(); // Clear the list of target filenames and rebuild it

	// Create new strings and insert them into a sList
	std::vector<CString> sList;
	POSITION fpos = m_FileList.GetHeadPosition();
	int i = 0;
	while (fpos != NULL) {
		CString FName;
		int newcount;
		newcount = NewFN.GetLine (i,FName.GetBuffer (MAX_PATH),MAX_PATH);
		FName.ReleaseBuffer (newcount);
		FName.Trim ('\r');
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
		NewFilenames.push_back (FName);
		NewFilePaths.push_back (path);

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
			er.Format (GetResString (IDS_X_IDENTICALFILENAMES),i+1);
			AfxMessageBox (er,MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}

	// The user has successfully entered new filenames. Now we have
	// to rename all the files...
	POSITION pos = m_FileList.GetHeadPosition();
	for(int i=0;pos != NULL;) {
		CString newname = NewFilenames.at (i);
		CString newpath = NewFilePaths.at (i);
		CKnownFile* file = m_FileList.GetAt (pos);
		if(!theApp.knownfiles->IsFilePtrInList(file) && !theApp.downloadqueue->IsPartFile(file))
			continue;
		// .part files could be renamed by simply changing the filename
		// in the CKnownFile object.
		bool bPartFile = file->IsPartFile();
		bool bShareFile = theApp.sharedfiles->IsFilePtrInList(file); // NEO: AKF - [AllKnownFiles]
		if (!bPartFile && bShareFile && (_trename(file->GetFilePath(), newpath) != 0)){  // NEO: AKF - [AllKnownFiles]
			// Use the "Format"-Syntax of AddLogLine here instead of
			// CString.Format+AddLogLine, because if "%"-characters are
			// in the string they would be misinterpreted as control sequences!
			ModLog(GetResString(IDS_X_FAILED_TO_RENAME), file->GetFilePath(), newpath, _tcserror(errno));
		} else {
			if (!bPartFile) {
				// Use the "Format"-Syntax of AddLogLine here instead of
				// CString.Format+AddLogLine, because if "%"-characters are
				// in the string they would be misinterpreted as control sequences!
				ModLog(GetResString(IDS_X_SUCCESED_TO_RENAME), file->GetFilePath(), newpath);
				file->SetFileName(newname);
				if(bShareFile) // NEO: AKF - [AllKnownFiles]
					file->SetFilePath(newpath);
				//UpdateFile(file);
			} else {
				// Use the "Format"-Syntax of AddLogLine here instead of
				// CString.Format+AddLogLine, because if "%"-characters are
				// in the string they would be misinterpreted as control sequences!
				ModLog(GetResString(IDS_X_SUCCESED_TO_RENAME), file->GetFileName(), newname);
				file->SetFileName(newname, true); 
				((CPartFile*) file)->UpdateDisplayedInfo();
				((CPartFile*) file)->SavePartFile(); 
				//UpdateFile(file);
			}
		}

		// Next item
		m_FileList.GetNext (pos);
		i++;
	}

	// Everything is ok, the caller can take NewFilenames to rename the files.
	OnOK(); // NEO: MLD - [ModelesDialogs]
}

// Copy the first line of the New-filenames Edit field to the Mask Edit-field
void CMassRenameDialog::UpdateEditMask()
{
	m_DontTrackKeys = true;
	// Take the first line of the "New filenames" Edit control as a mask for all filenames
	CString FirstLine;
//Lit>
	//NewFN.GetWindowText (FirstLine);
	NewFNLeft.GetWindowText (FirstLine);
	bool RightJustify = !IsDlgButtonChecked (IDC_LIT_MASSRENLEFT);
	if (RightJustify) 
		NewFNRight.GetWindowText (FirstLine);
//Lit<
	int i = FirstLine.Find ('\r');
	if (i != -1) {
		FirstLine = FirstLine.Left (i);
	}
	GetDlgItem (IDC_FILENAMEMASKEDIT)->SetWindowText (FirstLine);
	m_DontTrackKeys = false;
}

void CMassRenameDialog::OnEnSetfocusFilenamemaskEdit()
{
	UpdateEditMask ();
	
	m_LastFocusedEdit = GetDlgItem (IDC_FILENAMEMASKEDIT);
}

// Remember the Edit control that gained the focus at last
void CMassRenameDialog::OnEnSetfocusRichEdit()
{
	m_LastFocusedEdit = GetFocus ();
}

void CMassRenameDialog::OnBnClickedReset()
{
	CString txt;
	OldFN.GetWindowText (txt);
//Lit>
//	NewFN.SetWindowText (txt);
	NewFNLeft.SetWindowText (txt);
	NewFNRight.SetWindowText (txt);
//Lit<
	UndoBuffer = txt;
	InDel = false;
	LastEditWasUndo = false;
	LastDelPos = -10;
	OnEnSetfocusFilenamemaskEdit();
}

void CMassRenameDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CModResizableDialog::OnShowWindow(bShow, nStatus); // NEO: MLD - [ModelesDialogs]

	if (bShow) GetDlgItem(IDC_FILENAMEMASKEDIT)->SetFocus ();
}


void CMassRenameDialog::OnEnChangeFilenamemaskEdit()
{
	if (m_DontTrackKeys) return;
	
	CString AfterEdit;
	int Start1,End1,Start2,End2;
	int StartR1,EndR1,StartR2,EndR2;
	
	MassRenameEdit->GetSel (Start2,End2);
	MassRenameEdit->GetWindowText (AfterEdit);
	Start1 = MassRenameEdit->Start1;
	End1 = MassRenameEdit->End1;
//Lit>
	bool RightJustify = !IsDlgButtonChecked (IDC_LIT_MASSRENLEFT);

	CRichEditCtrl &NewFN = RightJustify ? NewFNRight : NewFNLeft;
//Lit<

	int BeforeEditLen = MassRenameEdit->m_BeforeEdit.GetLength ();

	StartR1 = BeforeEditLen-Start1;
	StartR2 = BeforeEditLen-Start2;
	EndR1 = BeforeEditLen-End1;
	EndR2 = BeforeEditLen-End2;

	CString allFNText;
	NewFN.GetWindowText (allFNText);

	if (Start1==-5) {
		// There's an UNDO goin on...
//Lit>
//		NewFN.SetWindowText (UndoBuffer);
		NewFNLeft.SetWindowText (UndoBuffer);
		NewFNRight.SetWindowText (UndoBuffer);
//Lit<
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
		for (int i=0; i < NewFN.GetLineCount (); i++) {
			int lstart = NewFN.LineIndex (i);
			int llen = NewFN.LineLength (lstart);
			bool nochange=false;  // Don't Edit if position is not in the current line
//lit>			
//			NewFN.SetSel (lstart+Start2,lstart+Start2+1);
//			if (lstart+Start2 >= lstart+llen) nochange = true;
			if (!RightJustify) {
				NewFN.SetSel (lstart+Start2,lstart+Start2+1);
				if (lstart+Start2 >= lstart+llen) nochange = true;
			} else {
				NewFN.SetSel (lstart+llen-StartR2,lstart+llen-StartR2+1);
				if (llen-StartR2 < 0) nochange = true;
			}
//Lit<
			if (!nochange) NewFN.ReplaceSel (_T(""));
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
		for (int i=0; i < NewFN.GetLineCount (); i++) {
			int lstart = NewFN.LineIndex (i);
			int llen = NewFN.LineLength (lstart);
			bool nochange=false;  // Don't Edit if position is not in the current line
//Lit>
//			NewFN.SetSel (lstart+Start1,lstart+min(End1,llen));
//			if (lstart+Start1 > lstart+llen) nochange = true;
			if (!RightJustify) {
				NewFN.SetSel (lstart+Start1,lstart+min(End1,llen));
				if (lstart+Start1 > lstart+llen) nochange = true;
			} else {
				NewFN.SetSel (lstart+max(llen-StartR1,0),lstart+llen-EndR1);
				if (llen-StartR1 < 0) nochange = true;
			}
//Lit<
			if (!nochange) NewFN.ReplaceSel (NewChars);
		}
	}
}

void CMassRenameDialog::OnBnClickedButtonStrip()
{
	CString filenames;

//Lit>
	bool RightJustify = !IsDlgButtonChecked (IDC_LIT_MASSRENLEFT);
	CRichEditCtrl &NewFN = RightJustify ? NewFNRight : NewFNLeft;
//Lit<

	// Now process through each line and cleanup that filename
	for (int i=0; i < NewFN.GetLineCount (); i++) {
		// Get the filename
		CString filename;
		int newcount;
		newcount = NewFN.GetLine (i,filename.GetBuffer (MAX_PATH+1),MAX_PATH);
		filename.ReleaseBuffer(newcount);
		// Clean it up
		filename = CleanupFilename (filename);
		// and add it to the current list of filenames
		if (filenames != "") filenames += "\r\n";
		filenames += filename;
	}

	// at the end save the list of filenames to the RichEdit controls
//Lit>
//	NewFN.SetWindowText( filenames );
	NewFNLeft.SetWindowText( filenames );
	NewFNRight.SetWindowText( filenames );
//Lit<
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
	if(!SimpleDlg)
		SimpleDlg = new CSimpleCleanupDialog(this);
	if(!SimpleDlg->IsDialogOpen()){
		CIni ini(thePrefs.GetConfigFile(), L"NeoPrefs");
		int SimpleCleanupOptions = ini.GetInt (_T("SimpleCleanupOptions"),3);
		CString SimpleCleanupSearch = ini.GetString (_T("SimpleCleanupSearch"));
		CString SimpleCleanupReplace = ini.GetString (_T("SimpleCleanupReplace"));
		// Format of the preferences string for character replacement:
		//      "str";"str";"str";...;"str"
		// Every "str" in SimpleCleanupSearchChars corresponds to a "str"
		// in SimpleCleanupReplaceChars at the same position.
		CString SimpleCleanupSearchChars = ini.GetString (_T("SimpleCleanupSearchChars"), _T("\"\xE4\";\"\xF6\";\"\xFC\";\"\xC4\";\"\xD6\";\"\xDC\";\"\xDF\""));/*ISO 8859-4*/
		CString SimpleCleanupReplaceChars = ini.GetString (_T("SimpleCleanupReplaceChars"), _T("\"ae\";\"oe\";\"ue\";\"Ae\";\"Oe\";\"Ue\";\"ss\""));

		SimpleDlg->SetConfig (SimpleCleanupOptions,	SimpleCleanupSearch, SimpleCleanupReplace, SimpleCleanupSearchChars, SimpleCleanupReplaceChars);
		SimpleDlg->OpenDialog(FALSE);
	}
}

LRESULT CMassRenameDialog::SimpleCleanupNotify(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	ASSERT(SimpleDlg);

	// Get the config how to perform the cleanup
	int options;
	CString source, dest;
	CString sourcechar, destchar;
	SimpleDlg->GetConfig (options,source,dest,sourcechar,destchar);

	CIni ini(thePrefs.GetConfigFile(), L"NeoPrefs");

	// Save the options in the preferences 
	ini.WriteInt (_T("SimpleCleanupOptions"),options);
	// Enclose the strings with '"' before writing them to the file.
	// These will be filtered if the string is read again
	ini.WriteString (_T("SimpleCleanupSearch"),CString ('\"')+source+('\"'));
	ini.WriteString (_T("SimpleCleanupReplace"),CString ('\"')+dest+('\"'));
	ini.WriteString (_T("SimpleCleanupSearchChars"),CString ('\"')+sourcechar+('\"'));
	ini.WriteString (_T("SimpleCleanupReplaceChars"),CString ('\"')+destchar+('\"'));

	CString filenames;

//Lit>
	bool RightJustify = !IsDlgButtonChecked (IDC_LIT_MASSRENLEFT);
	CRichEditCtrl &NewFN = RightJustify ? NewFNRight : NewFNLeft;
//Lit<

	// Now process through each line and cleanup that filename
	for (int i=0; i < NewFN.GetLineCount (); i++) {
		// Get the filename
		CString filename;
		int newcount;
		newcount = NewFN.GetLine (i,filename.GetBuffer (MAX_PATH+1),MAX_PATH);
		filename.ReleaseBuffer(newcount);
		// Clean it up
		filename = SimpleCleanupFilename (filename.SpanExcluding (_T("\r\n")),
											options,source,dest,
											sourcechar,destchar);
		// and add it to the current list of filenames
		if (filenames != "") filenames += "\r\n";
		filenames += filename;
	}

	// at the end save the list of filenames to the RichEdit controls
//Lit>	
//	NewFN.SetWindowText( filenames );
	NewFNLeft.SetWindowText( filenames );
	NewFNRight.SetWindowText( filenames );
//Lit<

	return 0;
}

void CMassRenameDialog::OnBnClickedInserttextcolumn()
{
//Lit>
	bool RightJustify = !IsDlgButtonChecked (IDC_LIT_MASSRENLEFT);
	CRichEditCtrl &NewFN = RightJustify ? NewFNRight : NewFNLeft;
//Lit<
	// Prepare the UNDO-Buffer; for future implementation since the Windows Undo handling
	// is very crazy...
	LastEditWasUndo = true;
	CString allFNText;
	NewFN.GetWindowText (allFNText);
	UndoBuffer = allFNText;
	InDel = false;

	// Now determine where we have to insert the clipboard data.
	// If the "EditMask" Edit line is selected, start from the top.
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
		NewFN.GetSel (StartXL, EndXL);
		StartLine = NewFN.LineFromChar (StartXL);
		// It's not allowed to select multple lines here!
		if (NewFN.LineFromChar (EndXL) != StartLine) EndXL = StartXL;
		// Correct the Index positions concernung the start position of the current line
		StartXL -= NewFN.LineIndex ();
		EndXL -= NewFN.LineIndex ();

		CurrentLineLength = NewFN.LineLength();

		StartX = StartXL;
		EndX = EndXL;
	}

//Lit>
	if (RightJustify) {
		StartX = CurrentLineLength - StartX;
		EndX = CurrentLineLength - EndX;
		// From now on these coordinates have to be treated inverted!
	}
//Lit<

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
		for (int i=0; i < NewFN.GetLineCount (); i++) {
			// Get the filename
			CString filename;
			int newcount;
			newcount = NewFN.GetLine (i,filename.GetBuffer (MAX_PATH+1),MAX_PATH);
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
//Lit>
/*
				// Delete the marked characters
				filename.Delete (StartX, EndX-StartX);
				// Insert this line into the filename
				filename.Insert (StartX,NextChars);
*/
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
//Lit<
				// Move the clipboard cursor position to the next line; skip "\n"
				clstart = clend+1;
			}

			// and add it to the current list of filenames
			if (filenames != "") filenames += "\r\n";
			filenames += filename;

		}

		// at the end save the list of filenames to the RichEdit controls
//Lit>
//		NewFN.SetWindowText( filenames );
		NewFNLeft.SetWindowText( filenames );
		NewFNRight.SetWindowText( filenames );
//Lit<

		// update the MaskEdit-control
		UpdateEditMask ();
	}
}

//Lit>
void CMassRenameDialog::OnBnClickedFilenameleft()
{
	// Show the left justify edit control
	NewFNLeft.ModifyStyle (0,WS_VISIBLE);
	NewFNRight.ModifyStyle (WS_VISIBLE,0);
	CString txt;
	NewFNRight.GetWindowText (txt);
	NewFNLeft.SetWindowText (txt);

	Invalidate();
}

void CMassRenameDialog::OnBnClickedFilenameright()
{
	// Show the right justify edit control
	NewFNLeft.ModifyStyle (WS_VISIBLE,0);
	NewFNRight.ModifyStyle (0,WS_VISIBLE);
	CString txt;
	NewFNLeft.GetWindowText (txt);
	NewFNRight.SetWindowText (txt);

	Invalidate();
}
//Lit<
