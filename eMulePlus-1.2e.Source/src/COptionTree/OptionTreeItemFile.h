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

#ifndef OT_ITEMFILE
#define OT_ITEMFILE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemFile.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"
#include "OptionTreeFileDlg.h"

class COptionTreeItemFile : public COptionTreeItem
{
public:
	COptionTreeItemFile();
	virtual ~COptionTreeItemFile();
	virtual void OnMove();
	virtual void OnRefresh();
	virtual void OnCommit();
	virtual void OnActivate();
	virtual void CleanDestroyWindow();
	virtual void OnDeSelect();
	virtual void OnSelect();
	virtual void DrawAttribute(CDC *pDC, const RECT &rcRect);

	CString GetSelectedFolder();
	CString GetFileDrive();
	CString GetFileDir();
	CString GetFileExt();
	CString GetFileTitle();
	CString GetFileName();
	CString GetPathName();
	CString GetNextPathName(POSITION& pos);
	POSITION GetStartPosition();
	CString GetDialogTitle();
	void SetDialogTitle(CString strTitle);
	CString GetFilter();
	void SetFilter(CString strFilter);
	CString GetDefaultExtention();
	void SertDefaultExtention(CString strExt);
	DWORD GetDialogFlags();
	void SetDialogFlags(DWORD dwFlags);
	void AddFileName(CString strFile);
	BOOL CreateFileItem(CString strFile, CString strDefExt, CString strFilter, DWORD dwOptions, DWORD dwDlgFlags);

protected:
	CString GetFileDrive(CString strFile);
	CString GetFileDirectory(CString strFile);
	CString GetFileExtention(CString strFile);
	BOOL GetOption(DWORD dwOption);
	void SetOption(DWORD dwOption, BOOL bSet);	
	BOOL GetDialogFlag(DWORD dwOption);
	void SetDialogFlag(DWORD dwOption, BOOL bSet);	
	DWORD m_dwOptions;
	DWORD m_dwDlgFlags;
	long m_lDefaultHeight;
	COptionTreeFileDlg m_dlgFile;
	CStringArray m_strFileNames;
	CString m_strDefExt;
	CString m_strFilter;
	CString m_strDlgTitle;
};

#endif // !OT_ITEMFILE
