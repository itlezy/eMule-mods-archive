// MsgViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MsgViewer.h"
#include "MsgViewerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgViewerApp

BEGIN_MESSAGE_MAP(CMsgViewerApp, CWinApp)
	//{{AFX_MSG_MAP(CMsgViewerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgViewerApp construction

CMsgViewerApp::CMsgViewerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMsgViewerApp object

CMsgViewerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMsgViewerApp initialization

BOOL CMsgViewerApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	SetShellAssociations();

	CMsgViewerDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CMsgViewerApp::SetShellAssociations()
{
	HKEY hKey = NULL;
	DWORD dwDispos = 0;
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, _T(".AppShare"), 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, 
		&dwDispos) != ERROR_SUCCESS)
		return; // failed

	BOOL bAlreadySet = (REG_OPENED_EXISTING_KEY == dwDispos);
	const TCHAR szLink[] = _T("AppShareFile");

	if (REG_OPENED_EXISTING_KEY != dwDispos)
		VERIFY(RegSetValueEx(hKey, NULL, 0, REG_SZ, (PBYTE) szLink, sizeof(szLink)) == ERROR_SUCCESS);

	VERIFY(RegCloseKey(hKey) == ERROR_SUCCESS);


	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, szLink, 0, NULL, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, 
		NULL, &hKey, &dwDispos) != ERROR_SUCCESS)
		return; // failed

	// set description
	const TCHAR szDescription[] = _T(" AppShare binary log");
	VERIFY(RegSetValueEx(hKey, NULL, 0, REG_SZ, (PBYTE) szDescription, sizeof(szDescription)) == ERROR_SUCCESS);

	// get the current executable file path
	TCHAR szFileName[MAX_PATH];
	VERIFY(GetModuleFileName(NULL, szFileName, MAX_PATH));

	// create shell subkey
	HKEY hSubKey = NULL;
	if (RegCreateKeyEx(hKey, _T("Shell\\&Open with MsgViewer\\Command"), 0, NULL, 0, 
		KEY_SET_VALUE, NULL, &hSubKey, &dwDispos) == ERROR_SUCCESS)
	{
		CString strCommand;
		strCommand.Format(_T("\"%s\" %%1"), szFileName);

		VERIFY(RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (PBYTE) (PCTSTR) strCommand, (strCommand.GetLength() + 1) * sizeof(TCHAR)) == ERROR_SUCCESS);

		VERIFY(RegCloseKey(hSubKey) == ERROR_SUCCESS);
	}

	// default icon
	if (RegCreateKeyEx(hKey, _T("DefaultIcon"), 0, NULL, 0, 
		KEY_SET_VALUE, NULL, &hSubKey, &dwDispos) == ERROR_SUCCESS)
	{
		CString strCommand;
		strCommand.Format(_T("%s,%d"), szFileName, 1);
		VERIFY(RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (PBYTE) (PCTSTR) strCommand, (strCommand.GetLength() + 1) * sizeof(TCHAR)) == ERROR_SUCCESS);

		VERIFY(RegCloseKey(hSubKey) == ERROR_SUCCESS);
	}


	VERIFY(RegCloseKey(hKey) == ERROR_SUCCESS);

	if (!bAlreadySet)
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}


