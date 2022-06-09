// COptionTree
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgement.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 1999-2002 
// ComputerSmarts.net 
// mattrmiller@computersmarts.net

#ifndef OT_FILEDLG
#define OT_FILEDLG

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"

// Classes
class COptionTree;

// Structures
struct OT_OPENFILENAMEEX : public OPENFILENAME 
{ 
	void*	m_pvReserved;
	DWORD	m_dwReserved;
	DWORD	m_dwFlagsEx;
};

class COptionTreeFileDlg  
{
public:
	int SelectFolder(LPCTSTR lpszTitle = NULL, LPCTSTR lpszStartPath = NULL, UINT ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS, CWnd* pParentWnd = NULL);
	POSITION GetStartPosition() const;
	CString GetNextPathName(POSITION& pos) const;
	CString GetPathName() const;
	CString GetFileName() const;
	CString GetFileTitle() const;
	CString GetFileExt() const;
	CString GetFileDir() const;
	CString GetFileDrive() const;
	CString GetSelectedFolder() const;
	virtual int DoModal();
	void SetDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);
	COptionTreeFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);
	COptionTreeFileDlg();
	virtual ~COptionTreeFileDlg();

protected:
	static int __stdcall BrowseCtrlCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	OT_OPENFILENAMEEX	m_ofn;
	BOOL m_bOpenFileDialog;
	TCHAR m_szFile[MAX_PATH];
	TCHAR m_szFileTitle[MAX_PATH];
	TCHAR m_szSelectedFolder[MAX_PATH];
	CString m_strFilter;
};

#endif // !OT_FILEDLG
