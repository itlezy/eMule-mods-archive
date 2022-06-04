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
#include "stdafx.h"
#include "resource.h"
#include "AddCRC32TagDialog.h"
#include "OtherFunctions.h"
#include "emule.h"
#include "emuleDlg.h"
#include "SharedFilesWnd.h"
#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#include <cryptopp/crc.h>
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#include "log.h"
#include "UserMsgs.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// Original file: Written by Mighty Knife, EMule Morph Team

// AddCRC32InputBox dialog

IMPLEMENT_DYNAMIC(AddCRC32InputBox, CDialog)

AddCRC32InputBox::AddCRC32InputBox(CWnd* pParent /*=NULL*/)
	: CDialog(AddCRC32InputBox::IDD, pParent)
{
}

AddCRC32InputBox::~AddCRC32InputBox()
{
}

void AddCRC32InputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(AddCRC32InputBox, CDialog)
END_MESSAGE_MAP()

void AddCRC32InputBox::OnOK()
{	
	GetDlgItem(IDC_CRC32PREFIX)->GetWindowText (m_CRC32Prefix);
	thePrefs.SetCRC32Prefix (m_CRC32Prefix);

	GetDlgItem(IDC_CRC32SUFFIX)->GetWindowText (m_CRC32Suffix);
	thePrefs.SetCRC32Suffix (m_CRC32Suffix);
	m_DontAddCRC32 = IsDlgButtonChecked(IDC_DONTADDCRC)!=0;
	thePrefs.SetDontAddCRCToFilename (m_DontAddCRC32);

	m_CRC32ForceUppercase = IsDlgButtonChecked(IDC_CRCFORCEUPPERCASE)!=0;
	thePrefs.SetCRC32ForceUppercase (m_CRC32ForceUppercase);

	m_CRC32ForceAdding = IsDlgButtonChecked(IDC_CRCFORCEADDING)!=0;
	thePrefs.SetCRC32ForceAdding (m_CRC32ForceAdding);

	CDialog::OnOK();
}


void AddCRC32InputBox::OnCancel()
{
	CDialog::OnCancel();
}

BOOL AddCRC32InputBox::OnInitDialog(){
	CDialog::OnInitDialog();
	InitWindowStyles(this);
	SetIcon(theApp.LoadIcon(_T("FILECRC32"),16,16),FALSE);
	SetWindowText(GetResString(IDS_CRC32_TITLE));

	CheckDlgButton(IDC_DONTADDCRC,thePrefs.GetDontAddCRCToFilename() ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CRCFORCEUPPERCASE,thePrefs.GetCRC32ForceUppercase() ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CRCFORCEADDING,thePrefs.GetCRC32ForceAdding() ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_CRC32PREFIX)->SetWindowText (thePrefs.GetCRC32Prefix ());
	GetDlgItem(IDC_CRC32SUFFIX)->SetWindowText (thePrefs.GetCRC32Suffix ());
     
	GetDlgItem(IDC_IBLABEL)->SetWindowText(GetResString(IDS_CRC_PREFIX));
    GetDlgItem(IDC_STATIC)->SetWindowText(GetResString(IDS_CRC_SUFFIX));
	GetDlgItem(IDC_CRCFORCEADDING)->SetWindowText(GetResString(IDS_CRC_FORCEADDING));
	GetDlgItem(IDC_CRCFORCEUPPERCASE)->SetWindowText(GetResString(IDS_CRC_FORCEUPPERCASE));
	GetDlgItem(IDC_DONTADDCRC)->SetWindowText(GetResString(IDS_CRC_DONTADDCRC));
    
	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));

	return TRUE;
}

IMPLEMENT_DYNCREATE(CCRC32RenameWorker, CFileProcessingWorker)

void CCRC32RenameWorker::Run () {
	// Inform the shared files window that the file can be renamed.
	// It's saver to rename the file in the main thread than in this thread
	// to avoid address conflicts in the pointer lists of all the files...
	// They are not being thread save...
	// We send the address of this thread in the LPARAM parameter to the
	// shared files window so it can access all parameters needed to rename the file.
	::SendMessage (theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_hWnd,UM_CRC32_RENAMEFILE,
		  		   0, (LPARAM) this);
}

IMPLEMENT_DYNCREATE(CCRC32CalcWorker, CFileProcessingWorker)

void CCRC32CalcWorker::Run () {
	// This method calculates the CRC32 checksum of the file and stores it to the
	// CKnownFile object. The CRC won't be calculated if it exists already.
	CKnownFile* f = ValidateKnownFile (m_fileHashToProcess);
	if (f==NULL) {
		// File doesn't exist in the list; deleted and reloaded the shared files list in
		// the meantime ?
		// Let's hope the creator of this Worker thread has set the filename so we can
		// display it...
		if (m_FilePath == "") {
			AddLogLine (false, GetResString(IDS_LOG_CRC32_WRN1));
		} else {
			AddLogLine (false, GetResString(IDS_LOG_CRC32_WRN2),m_FilePath);
		}
		return;         
	}
	if (f->IsCRC32Calculated ()) {     
		// We already have the CRC
		AddLogLine (false, GetResString(IDS_LOG_CRC32_SKIP1),
						   f->GetFileName ());
		UnlockSharedFilesList ();
		return;
	}
	if (f->IsPartFile ()) {     
		// We can't add a CRC suffix to files which are not complete
		AddLogLine (false, GetResString(IDS_LOG_CRC32_SKIP2),
						   f->GetFileName ());
		UnlockSharedFilesList ();
		return;
	}
	AddLogLine (false, GetResString(IDS_LOG_CRC32_CALC),f->GetFileName ());
	CString Filename = f->GetFileName ();
	// Release the lock while computing...
	UnlockSharedFilesList ();
	CString sCRC32;
	
	// Calculate the CRC...
	CFile file;
	if (file.Open(f->GetFilePath (),CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone)){
		// to be implemented...
		CryptoPP::CRC32 CRC32Calculator;
		byte* buffer = (byte*) malloc (65000);
		int cnt;
		do {
			if (!theApp.emuledlg->IsRunning())	// in case of shutdown while still hashing
				break;
			if (m_pOwner->IsTerminating ())     // Stop the calculation if aborted
				break;
			cnt = file.Read (buffer, 65000);
			if (cnt > 0) {
				// Update the CRC32 in the calculation object
				CRC32Calculator.Update (buffer, cnt);
			}
		} while (cnt == 65000);

		free (buffer);
		file.Close ();

		if (!theApp.emuledlg->IsRunning()) {
			// Abort and get back immediately
			return;
		}
		if (m_pOwner->IsTerminating ()) {
			// Calculation aborted; this will stop all calculations, so we can tell
			// the user its completly stopped.
			AddLogLine (false, GetResString(IDS_LOG_CRC32_ADORT));
			return;
		}

		// Calculation successfully completed. Update the CRC in the CKnownFile object.
		byte FinalCRC32 [4];
		CRC32Calculator.TruncatedFinal (FinalCRC32,4); // Get the CRC
		sCRC32.Format (_T("%02X%02X%02X%02X"),(int) FinalCRC32 [3],
										  (int) FinalCRC32 [2],
										  (int) FinalCRC32 [1],
										  (int) FinalCRC32 [0]);
		AddLogLine (false, GetResString(IDS_LOG_CRC32_COMPLETED),Filename,sCRC32);

		// relock the list, get the file pointer
		f = ValidateKnownFile (m_fileHashToProcess);
		// store the CRC
		if (f != NULL) {
			f->SetLastCalculatedCRC32 (sCRC32);
		} // No error message if file does not exist...

		// Release the lock of the list again
		UnlockSharedFilesList ();
		// Inform the file window that the CRC calculation was successful.
		// The window should then show the CRC32 in the corresponding column.
		// We tell the file window the file hash by lParam - not the pointer to the 
		// CKnownFile object because this could lead to a deadlock if the list
		// is blocked and the user pressed the Reload button at the time when the message
		// is pending !
		::SendMessage (theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_hWnd,UM_CRC32_UPDATEFILE,
					0, (LPARAM) m_fileHashToProcess);
	} else {
		// File cannot be accessed
		AddLogLine (false, GetResString(IDS_LOG_CRC32_WRN3),
						   Filename);
	}

}
