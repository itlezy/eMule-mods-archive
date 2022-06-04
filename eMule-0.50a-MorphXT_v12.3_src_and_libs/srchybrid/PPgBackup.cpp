// PPgBackup.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPgBackup.h"
#include "XMessageBox.h"
#include "string.h"
#include "Preferences.h"
#include "OtherFunctions.h"

// Mighty Knife: Save current settings before backup
#include "emuledlg.h"
// [end] Mighty Knife

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// CPPgBackup dialog

IMPLEMENT_DYNAMIC(CPPgBackup, CPropertyPage)
CPPgBackup::CPPgBackup()
: CPropertyPage(CPPgBackup::IDD)
{
}

CPPgBackup::~CPPgBackup()
{
}

void CPPgBackup::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}



BOOL CPPgBackup::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	Localize();
	LoadSettings();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



BEGIN_MESSAGE_MAP(CPPgBackup, CPropertyPage)
	ON_BN_CLICKED(IDC_BACKUPNOW, OnBnClickedBackupnow)
	ON_BN_CLICKED(IDC_DAT, OnBnClickedDat)
	ON_BN_CLICKED(IDC_MET, OnBnClickedMet)
	ON_BN_CLICKED(IDC_INI, OnBnClickedIni)
	ON_BN_CLICKED(IDC_PART, OnBnClickedPart)
	ON_BN_CLICKED(IDC_PARTMET, OnBnClickedPartMet)
	ON_BN_CLICKED(IDC_SELECTALL, OnBnClickedSelectall)
	ON_BN_CLICKED(IDC_AUTOBACKUP, OnBnClickedAutobackup)
	ON_BN_CLICKED(IDC_AUTOBACKUP2, OnBnClickedAutobackup2) //EastShare - Added by Pretender, Double Backup
END_MESSAGE_MAP()

void CPPgBackup::OnBnClickedDat()
{
	if (IsDlgButtonChecked(IDC_DAT)!=0) {
		GetDlgItem(IDC_BACKUPNOW)->EnableWindow(IsDlgButtonChecked(IDC_DAT));
	} else {
		if (IsDlgButtonChecked(IDC_DAT)==0 && IsDlgButtonChecked(IDC_MET)==0 && IsDlgButtonChecked(IDC_INI)==0 && IsDlgButtonChecked(IDC_PART)==0 && IsDlgButtonChecked(IDC_PARTMET)==0)
			GetDlgItem(IDC_BACKUPNOW)->EnableWindow(false);
	}

}

void CPPgBackup::OnBnClickedMet()
{
	if (IsDlgButtonChecked(IDC_MET)!=0) {
		GetDlgItem(IDC_BACKUPNOW)->EnableWindow(IsDlgButtonChecked(IDC_MET));
	} else {
		if (IsDlgButtonChecked(IDC_DAT)==0 && IsDlgButtonChecked(IDC_MET)==0 && IsDlgButtonChecked(IDC_INI)==0 && IsDlgButtonChecked(IDC_PART)==0 && IsDlgButtonChecked(IDC_PARTMET)==0)
			GetDlgItem(IDC_BACKUPNOW)->EnableWindow(false);
	}

}

void CPPgBackup::OnBnClickedIni()
{
	if (IsDlgButtonChecked(IDC_INI)!=0) {
		GetDlgItem(IDC_BACKUPNOW)->EnableWindow(IsDlgButtonChecked(IDC_INI));
	} else {
		if (IsDlgButtonChecked(IDC_DAT)==0 && IsDlgButtonChecked(IDC_MET)==0 && IsDlgButtonChecked(IDC_INI)==0 && IsDlgButtonChecked(IDC_PART)==0 && IsDlgButtonChecked(IDC_PARTMET)==0)
			GetDlgItem(IDC_BACKUPNOW)->EnableWindow(false);
	}

}

void CPPgBackup::OnBnClickedPart()
{
	if (IsDlgButtonChecked(IDC_PART)!=0) {
		GetDlgItem(IDC_BACKUPNOW)->EnableWindow(IsDlgButtonChecked(IDC_PART));
	} else {
		if (IsDlgButtonChecked(IDC_DAT)==0 && IsDlgButtonChecked(IDC_MET)==0 && IsDlgButtonChecked(IDC_INI)==0 && IsDlgButtonChecked(IDC_PART)==0 && IsDlgButtonChecked(IDC_PARTMET)==0)
			GetDlgItem(IDC_BACKUPNOW)->EnableWindow(false);
	}

}

void CPPgBackup::OnBnClickedPartMet()
{
	if (IsDlgButtonChecked(IDC_PARTMET)!=0) {
		GetDlgItem(IDC_BACKUPNOW)->EnableWindow(IsDlgButtonChecked(IDC_PARTMET));
	} else {
		if (IsDlgButtonChecked(IDC_DAT)==0 && IsDlgButtonChecked(IDC_MET)==0 && IsDlgButtonChecked(IDC_INI)==0 && IsDlgButtonChecked(IDC_PART)==0 && IsDlgButtonChecked(IDC_PARTMET)==0)
			GetDlgItem(IDC_BACKUPNOW)->EnableWindow(false);
	}

}


void CPPgBackup::OnBnClickedBackupnow()
{
	TCHAR buffer[200];
	y2All = FALSE;
	n2All = FALSE;
	if (IsDlgButtonChecked(IDC_DAT)!=0)
	{
		Backup(_T("*.dat"), true);
		CheckDlgButton(IDC_DAT,BST_UNCHECKED);
	}

	if (IsDlgButtonChecked(IDC_MET)!=0)
	{
		Backup(_T("*.met"), true);
		CheckDlgButton(IDC_MET,BST_UNCHECKED);
	}

	if (IsDlgButtonChecked(IDC_INI)!=0)
	{
		// Mighty Knife: Save current settings before backup
		theApp.emuledlg->SaveSettings (false);
		// [end] Mighty Knife
		Backup(_T("*.ini"), true);
		CheckDlgButton(IDC_INI,BST_UNCHECKED);
	}

	if (IsDlgButtonChecked(IDC_PARTMET)!=0)
	{
		Backup2(_T("*.part.met"));
		CheckDlgButton(IDC_PARTMET,BST_UNCHECKED);
	}

	if (IsDlgButtonChecked(IDC_PART)!=0)
	{
		_stprintf(buffer,GetResString(IDS_BACKUP_PART  ) );		//"Because of their size, backing up *.part files may take a few minutes.\nAre you sure you want to do this?"
		if(MessageBox(buffer,GetResString(IDS_BACKUP_PART_W),MB_ICONQUESTION|MB_YESNO)== IDYES)
			Backup2(_T("*.part"));
		CheckDlgButton(IDC_PART,BST_UNCHECKED);

	}

	MessageBox(_GetResString(IDS_BACKUP_COMPLETE), _T("BackUp complete."), MB_OK); //"File(s) Copied Successfully."
	y2All = FALSE;
	n2All = FALSE;
}



void CPPgBackup::Backup(LPCTSTR extensionToBack, BOOL conFirm)  
{
	WIN32_FIND_DATA FileData; 
	HANDLE hSearch; 
	TCHAR buffer[200];
	//CString szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR));
	CString szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	TCHAR szNewPath[MAX_PATH]; 

	SetCurrentDirectory(szDirPath);
	BOOL error = FALSE;
	BOOL OverWrite = TRUE;
	szDirPath +="Backup\\";

	BOOL fFinished = FALSE; 

	// Create a new directory if one does not exist
	if(!PathFileExists(szDirPath))
		CreateDirectory(szDirPath, NULL);

	// Start searching for files in the current directory. 

	hSearch = FindFirstFile(extensionToBack, &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		error = TRUE;
	} 

	// Copy each file to the new directory 
	CString str;
	while (!fFinished && !error) 
	{ 
		lstrcpy(szNewPath, szDirPath); 
		lstrcat(szNewPath, FileData.cFileName); 

		if(PathFileExists(szNewPath))
		{
			if (conFirm)
			{
				if (y2All == FALSE && n2All == FALSE)
				{
					_stprintf(buffer, GetResString(IDS_OVERWRITE1), FileData.cFileName);
					int rc = ::XMessageBox(m_hWnd,buffer,GetResString(IDS_OVERWRITE2),MB_YESNO|MB_YESTOALL|MB_NOTOALL|MB_ICONQUESTION);
					if (rc == IDYES)
						OverWrite = TRUE;
					else if (rc == IDYESTOALL)
					{
						OverWrite = TRUE;
						y2All = TRUE;
					}
					else if (rc == IDNOTOALL)
					{
						OverWrite = FALSE;
						n2All = TRUE;
					}
					else 
						OverWrite = FALSE;
				} else if(y2All == TRUE)
					OverWrite = TRUE;
				else //if(n2All == TRUE)
					OverWrite = FALSE;
			} 
			else
				OverWrite = TRUE;
		}	
		if(OverWrite)
			CopyFile(FileData.cFileName, szNewPath, FALSE);

		if (!FindNextFile(hSearch, &FileData)) 
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{ 
				//MessageBox("File Copied Successfully.", "BackUp complete", MB_OK); 
				fFinished = TRUE; 

			} 
			else 
			{ 
				error = TRUE;
			} 
		}

	} 


	// Close the search handle. 
	if (!FindClose(hSearch)) 
	{ 
		error = TRUE;
	} 
	if (error)
		MessageBox(GetResString(IDS_BACKUPERROR	),_T("Error"),MB_OK);
}


void CPPgBackup::Backup2(LPCTSTR extensionToBack)  
{

	WIN32_FIND_DATA FileData;   
	HANDLE hSearch;   
	TCHAR buffer[200];  
	BOOL error = FALSE;  



	for (int i=0;i<thePrefs.tempdir.GetCount();i++) {
		CString szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
		CString szTempPath = CString(thePrefs.GetTempDir(i));  
		TCHAR szNewPath[MAX_PATH]; 

		BOOL fFinished = FALSE;     
		BOOL OverWrite = TRUE;  
		szDirPath +="Backup\\";

		if(!PathFileExists(szDirPath))  
			CreateDirectory(szDirPath, NULL);  

		if (i==0)
			szDirPath+="Temp\\";  
		else { // i> 0 
			CString tempno;
			tempno.Format(L"Temp%d\\",i)  ;
			szDirPath+=tempno;
	    }

		if(!PathFileExists(szDirPath))  
			CreateDirectory(szDirPath, NULL);  

		// Start searching for files in the current directory.   
		SetCurrentDirectory(szTempPath);  

		hSearch = FindFirstFile(extensionToBack, &FileData);   

		if (hSearch == INVALID_HANDLE_VALUE)   
		{   
			error = TRUE;
		}   

		// Copy each file to the new directory   
		while (!fFinished && !error)   
		{   
			lstrcpy(szNewPath, szDirPath);   
			lstrcat(szNewPath, FileData.cFileName);   

			//MessageBox(szNewPath,"New Path",MB_OK);  
			if(PathFileExists(szNewPath))  
			{  
				if (y2All == FALSE && n2All == FALSE)
				{
					_stprintf(buffer, GetResString(IDS_OVERWRITE1), FileData.cFileName);
					int rc = ::XMessageBox(m_hWnd,buffer,GetResString(IDS_OVERWRITE2),MB_YESNO|MB_YESTOALL|MB_NOTOALL|MB_ICONQUESTION);
					if (rc == IDYES)
						OverWrite = TRUE;
					else if (rc == IDYESTOALL)
					{
						OverWrite = TRUE;
						y2All = TRUE;
					}
					else if (rc == IDNOTOALL)
					{
						OverWrite = FALSE;
						n2All = TRUE;
					}
					else 
						OverWrite = FALSE;
				} else if(y2All == TRUE)
					OverWrite = TRUE;
				else //if(n2All == TRUE)
					OverWrite = FALSE;
			}  

			if(OverWrite)  
				CopyFile(FileData.cFileName, szNewPath, FALSE);  

			if (!FindNextFile(hSearch, &FileData))   
			{  
				if (GetLastError() == ERROR_NO_MORE_FILES)   
				{   

					fFinished = TRUE;   
				}   
				else   
				{   
					error = TRUE;  
				}   
			}  
		}   

		// Close the search handle.   
		if (!FindClose(hSearch))   
		{   
			error = TRUE;  
		}   

		SetCurrentDirectory(CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)));  
	}

		if (error)  
			MessageBox(_T("Error encountered during backup"),_T("Error"),MB_OK);  

	} 

void CPPgBackup::OnBnClickedSelectall()
{
	CheckDlgButton(IDC_DAT,BST_CHECKED);
	CheckDlgButton(IDC_MET,BST_CHECKED);
	CheckDlgButton(IDC_INI,BST_CHECKED);
	CheckDlgButton(IDC_PART,BST_CHECKED);
	CheckDlgButton(IDC_PARTMET,BST_CHECKED);
	GetDlgItem(IDC_BACKUPNOW)->EnableWindow(true);
}

void CPPgBackup::OnBnClickedAutobackup()
{
	SetModified();
}

//EastShare - Added by Pretender, Double Backup
void CPPgBackup::OnBnClickedAutobackup2()
{
	SetModified();
}
//EastShare - Added by Pretender, Double Backup

void CPPgBackup::LoadSettings(void)
{
	if(m_hWnd)
	{
		if(thePrefs.GetAutoBackup())
			CheckDlgButton(IDC_AUTOBACKUP,1);
		else
			CheckDlgButton(IDC_AUTOBACKUP,0);
		//EastShare Start - Added by Pretender, Double Backup
		if(thePrefs.GetAutoBackup2())
			CheckDlgButton(IDC_AUTOBACKUP2,1);
		else
			CheckDlgButton(IDC_AUTOBACKUP2,0);
		//EastShare End - Added by Pretender, Double Backup
	}
}

BOOL CPPgBackup::OnApply()
{
	thePrefs.SetAutoBackup(IsDlgButtonChecked(IDC_AUTOBACKUP)!=0);
	
	//EastShare Start - Added by Pretender, Double Backup
	thePrefs.SetAutoBackup2(IsDlgButtonChecked(IDC_AUTOBACKUP2)!=0); 

	LoadSettings();
	SetModified();
	//EastShare End - Added by Pretender, Double Backup

	return CPropertyPage::OnApply();
}

//eastshare start - added by linekin, backup backup
void CPPgBackup::Backup3()
{
	WIN32_FIND_DATA FileData; 
	HANDLE hSearch; 
	CString szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR))+_T("Backup\\");
	if(!PathFileExists(szDirPath)) return;
	TCHAR szNewPath[MAX_PATH]; 

	SetCurrentDirectory(szDirPath);
	BOOL error = FALSE;
	szDirPath = CString(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR))+_T("Backup2\\");

	BOOL fFinished = FALSE; 

	// Create a new directory if one does not exist
	if(!PathFileExists(szDirPath))
		CreateDirectory(szDirPath, NULL);

	// Start searching for files in the current directory. 

	hSearch = FindFirstFile(_T("*.*"), &FileData); 
	if (hSearch == INVALID_HANDLE_VALUE) 
	{ 
		error = TRUE;
	} 

	// Copy each file to the new directory 
	while (!fFinished && !error) 
	{ 
		lstrcpy(szNewPath, szDirPath); 
		lstrcat(szNewPath, FileData.cFileName); 

		MoveFileEx(FileData.cFileName, szNewPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);

		if (!FindNextFile(hSearch, &FileData)) 
		{
			if (GetLastError() == ERROR_NO_MORE_FILES) 
			{ 
				//MessageBox("File Copied Successfully.", "BackUp complete", MB_OK); 
				fFinished = TRUE; 

			} 
			else 
			{ 
				error = TRUE;
			} 
		}

	} 


	// Close the search handle. 
	if (!FindClose(hSearch)) 
	{ 
		error = TRUE;
	} 
	if (error)
		MessageBox(GetResString(IDS_BACKUPERROR),_T("Error_"),MB_OK);
}

void CPPgBackup::Localize(void)
{
	if(m_hWnd)
	{
		GetDlgItem(IDC_BACKUP_FILEFRAME)->SetWindowText( GetResString(IDS_BACKUP_FILEFRAME) );
		GetDlgItem(IDC_SELECTALL)->SetWindowText( GetResString(IDS_BACKUP_SELECTALL) );
		GetDlgItem(IDC_BACKUP_AUTO)->SetWindowText( GetResString(IDS_BACKUP_AUTOFRAME) );
		GetDlgItem(IDC_AUTOBACKUP)->SetWindowText( GetResString(IDS_BACKUP_AUTOBACKUP));
		GetDlgItem(IDC_BACKUP_NOTE)->SetWindowText( GetResString(IDS_BACKUP_NOTE));
		GetDlgItem(IDC_BACKUPNOW)->SetWindowText( GetResString(IDS_BACKUP_BACKUPNOW) );
		GetDlgItem(IDC_BACKUP_MESSAGE)->SetWindowText( GetResString(IDS_BACKUP_MESSAGE) );
		GetDlgItem(IDC_AUTOBACKUP2)->SetWindowText( GetResString(IDS_BACKUP_AUTOBACKUP2));
	}
}
//eastshare end