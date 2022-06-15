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
#include "DownloadQueue.h"
#include "KnownFileList.h"
#include "SharedFilesWnd.h"
#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#include <crypto51/crc.h>
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
#include "Neo/Ini2.h" // NEO: INI - [PlusIniClass] <-- Xanatos --

// Original file: Written by Mighty Knife, EMule Morph Team

// AddCRC32InputBox dialog

AddCRC32InputBox::AddCRC32InputBox(CWnd* pParent /*=NULL*/)
	: CModDialog(AddCRC32InputBox::IDD, pParent) // NEO: MLD - [ModelesDialogs]
{
}

AddCRC32InputBox::~AddCRC32InputBox()
{
	m_FileList.RemoveAll();
}

void AddCRC32InputBox::DoDataExchange(CDataExchange* pDX)
{
	CModDialog::DoDataExchange(pDX); // NEO: MLD - [ModelesDialogs]
}

BEGIN_MESSAGE_MAP(AddCRC32InputBox, CModDialog) // NEO: MLD - [ModelesDialogs]
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void AddCRC32InputBox::OnOK()
{	
	GetDlgItem(IDC_CRC32PREFIX)->GetWindowText (m_CRC32Prefix);
	GetDlgItem(IDC_CRC32SUFFIX)->GetWindowText (m_CRC32Suffix);
	m_DontAddCRC32 = IsDlgButtonChecked(IDC_DONTADDCRC)!=0;
	m_CRC32ForceUppercase = IsDlgButtonChecked(IDC_CRCFORCEUPPERCASE)!=0;
	m_CRC32ForceAdding = IsDlgButtonChecked(IDC_CRCFORCEADDING)!=0;

	CIni ini(thePrefs.GetConfigFile(), L"NeoPrefs");

	// Save the options in the preferences 
	ini.WriteBool (_T("DontAddCRC32"),m_DontAddCRC32);
	ini.WriteBool (_T("CRC32ForceUppercase"),m_CRC32ForceUppercase);
	ini.WriteBool (_T("CRC32ForceAdding"),m_CRC32ForceAdding);
	ini.WriteString (_T("CRC32Prefix"),CString ('\"')+m_CRC32Prefix+('\"'));
	ini.WriteString (_T("CRC32Suffix"),CString ('\"')+m_CRC32Suffix+('\"'));

	// For every chosen file create a worker thread and add it
	// to the file processing thread
	POSITION pos = m_FileList.GetHeadPosition();
	while (pos != NULL) {
		// first create a worker thread that calculates the CRC
		// if it's not already calculated...
		CKnownFile*  file = m_FileList.GetAt (pos);
		if(!theApp.knownfiles->IsFilePtrInList(file) && !theApp.downloadqueue->IsPartFile(file))
			continue;
		const uchar* FileHash = file->GetFileHash ();

		// But don't add the worker thread if the CRC32 should not
		// be calculated !
		if (!GetDontAddCRC32 ()) {
			CCRC32CalcWorker* workercrc = new CCRC32CalcWorker;
			workercrc->SetFileHashToProcess (FileHash);
			workercrc->SetFilePath (file->GetFilePath ());
			theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_FileProcessingThread.AddFileProcessingWorker (workercrc);
		}

		// Now add a worker thread that informs this window when
		// the calculation is completed.
		// The method OnCRC32RenameFilename will then rename the file
		CCRC32RenameWorker* worker = new CCRC32RenameWorker;
		worker->SetFileHashToProcess (FileHash);
		worker->SetFilePath (file->GetFilePath ());
		worker->SetFilenamePrefix (GetCRC32Prefix ());
		worker->SetFilenameSuffix (GetCRC32Suffix ());
		worker->SetDontAddCRCAndSuffix (GetDontAddCRC32 ());
		worker->SetCRC32ForceUppercase (GetCRC32ForceUppercase ());
		worker->SetCRC32ForceAdding (GetCRC32ForceAdding ());
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_FileProcessingThread.AddFileProcessingWorker (worker);

		// next file
		m_FileList.GetNext (pos);
	}
	// Start the file processing thread to process the files.
	if (!theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_FileProcessingThread.IsRunning ()) {
		// (Re-)start the thread, this will do the rest

		// If the thread object already exists, this will result in an
		// ASSERT - but that doesn't matter since we manually take care
		// that the thread does not run at this moment...
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_FileProcessingThread.CreateThread ();
	}

	CModDialog::OnOK(); // NEO: MLD - [ModelesDialogs]
}


void AddCRC32InputBox::OnCancel()
{
	CModDialog::OnCancel(); // NEO: MLD - [ModelesDialogs]
}

BOOL AddCRC32InputBox::OnInitDialog(){
	CModDialog::OnInitDialog(); // NEO: MLD - [ModelesDialogs]
	InitWindowStyles(this);
	SetIcon(theApp.LoadIcon(_T("FILECRC32"),16,16),FALSE);
	SetWindowText(GetResString(IDS_X_CRC32_TITLE));

	CIni ini(thePrefs.GetConfigFile(), L"NeoPrefs");

	// Save the options in the preferences 
	m_DontAddCRC32 = ini.GetBool (_T("DontAddCRC32"),false);
	m_CRC32ForceUppercase = ini.GetBool (_T("CRC32ForceUppercase"),false);
	m_CRC32ForceAdding = ini.GetBool (_T("CRC32ForceAdding"),false);
	m_CRC32Prefix = ini.GetString (_T("CRC32Prefix"),CString(_T("\" [\"")).Trim (_T("\"")));
	m_CRC32Suffix = ini.GetString (_T("CRC32Suffix"),CString(_T("\"]\"")).Trim (_T("\"")));

	CheckDlgButton(IDC_DONTADDCRC,m_DontAddCRC32 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CRCFORCEUPPERCASE,m_CRC32ForceUppercase ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CRCFORCEADDING,m_CRC32ForceAdding ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_CRC32PREFIX)->SetWindowText (m_CRC32Prefix);
	GetDlgItem(IDC_CRC32SUFFIX)->SetWindowText (m_CRC32Suffix);
     
	GetDlgItem(IDC_IBLABEL)->SetWindowText(GetResString(IDS_X_CRC_PREFIX));
    GetDlgItem(IDC_STATIC)->SetWindowText(GetResString(IDS_X_CRC_SUFFIX));
	GetDlgItem(IDC_CRCFORCEADDING)->SetWindowText(GetResString(IDS_X_CRC_FORCEADDING));
	GetDlgItem(IDC_CRCFORCEUPPERCASE)->SetWindowText(GetResString(IDS_X_CRC_FORCEUPPERCASE));
	GetDlgItem(IDC_DONTADDCRC)->SetWindowText(GetResString(IDS_X_CRC_DONTADDCRC));
    
	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));

	//LoadWindowRect(_T("AddCRC32InputBox"), FALSE);
	return TRUE;
}

void AddCRC32InputBox::OnDestroy() 
{
	//SaveWindowRect(_T("AddCRC32InputBox"), FALSE);

	CModDialog::OnDestroy(); // NEO: MLD - [ModelesDialogs]
}


IMPLEMENT_DYNCREATE(CCRC32RenameWorker, CFileProcessingWorker)

void CCRC32RenameWorker::Run () {
	// NEO: SSH - [SlugFillerSafeHash]
	// BEGIN SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return;
	// END SLUGFILLER: SafeHash
	// NEO: SSH END

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
	// NEO: SSH - [SlugFillerSafeHash]
	// BEGIN SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return;
	// END SLUGFILLER: SafeHash
	// NEO: SSH END

	// This method calculates the CRC32 checksum of the file and stores it to the
	// CKnownFile object. The CRC won't be calculated if it exists already.
	CKnownFile* f = ValidateKnownFile (m_fileHashToProcess);
	if (f==NULL) {
		// File doesn't exist in the list; deleted and reloaded the shared files list in
		// the meantime ?
		// Let's hope the creator of this Worker thread has set the filename so we can
		// display it...
		if (m_FilePath == "") {
			AddLogLine (false, GetResString(IDS_X_LOG_CRC32_WRN1));
		} else {
			AddLogLine (false, GetResString(IDS_X_LOG_CRC32_WRN2),m_FilePath);
		}
		return;         
	}
	if (f->IsCRC32Calculated ()) {     
		// We already have the CRC
		AddLogLine (false, GetResString(IDS_X_LOG_CRC32_SKIP1),
						   f->GetFileName ());
		UnlockSharedFilesList ();
		return;
	}
	if (f->IsPartFile ()) {     
		// We can't add a CRC suffix to files which are not complete
		AddLogLine (false, GetResString(IDS_X_LOG_CRC32_SKIP2),
						   f->GetFileName ());
		UnlockSharedFilesList ();
		return;
	}
	AddLogLine (false, GetResString(IDS_X_LOG_CRC32_CALC),f->GetFileName ());
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
			AddLogLine (false, GetResString(IDS_X_LOG_CRC32_ADORT));
			return;
		}

		// Calculation successfully completed. Update the CRC in the CKnownFile object.
		byte FinalCRC32 [4];
		CRC32Calculator.TruncatedFinal (FinalCRC32,4); // Get the CRC
		sCRC32.Format (_T("%02X%02X%02X%02X"),(int) FinalCRC32 [3],
										  (int) FinalCRC32 [2],
										  (int) FinalCRC32 [1],
										  (int) FinalCRC32 [0]);
		AddLogLine (false, GetResString(IDS_X_LOG_CRC32_COMPLETED),Filename,sCRC32);

		// relock the list, get the file pointer
		f = ValidateKnownFile (m_fileHashToProcess);
		// store the CRC
		if (f != NULL) {
			memcpy(f->GetCalculatedCRC32rw(),FinalCRC32,4);
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
		AddLogLine (false, GetResString(IDS_X_LOG_CRC32_WRN3),
						   Filename);
	}

}
