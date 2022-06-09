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

#include "stdafx.h"
#include "OptionTreeFileDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionTreeFileDlg::COptionTreeFileDlg()
{
	// Initialize variables
	m_strFilter = _T("");
	::ZeroMemory(&m_ofn, sizeof(m_ofn));
	::ZeroMemory(&m_szFile, sizeof(m_szFile));
	::ZeroMemory(&m_szFileTitle, sizeof(m_szFileTitle));
	::ZeroMemory(&m_szSelectedFolder, sizeof(m_szSelectedFolder));
}

COptionTreeFileDlg::COptionTreeFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd)
{
	// Set dialog
	SetDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd);
}

COptionTreeFileDlg::~COptionTreeFileDlg()
{
}

void COptionTreeFileDlg::SetDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd)
{
	// Declare variables
	CString strFilter;
	int nIndex = 0;

	// Store parameters into structure
	m_bOpenFileDialog = bOpenFileDialog;
	m_ofn.lpstrDefExt = lpszDefExt;
	if (lpszFileName != NULL)
	{
		_tcscpy(m_szFile, lpszFileName);
		m_ofn.lpstrFile = m_szFile;
		m_ofn.nMaxFile = MAX_PATH;
	}
	else
	{
		m_ofn.lpstrFile = m_szFile;
		m_ofn.nMaxFile = MAX_PATH;
	}
	m_ofn.lpstrFileTitle = m_szFileTitle;
	m_ofn.nMaxFileTitle = MAX_PATH;
	m_ofn.Flags = dwFlags | OFN_EXPLORER;
	if (lpszFilter != NULL)
	{
		m_strFilter = lpszFilter;
		LPTSTR pch = m_strFilter.GetBuffer(0);
		while ((pch = _tcschr(pch, '|')) != NULL)
		{
			*pch++ = '\0';
		}
		m_ofn.lpstrFilter = m_strFilter;
	}
	if (pParentWnd != NULL)
	{
		m_ofn.hwndOwner = pParentWnd->GetSafeHwnd();
	}
}

int COptionTreeFileDlg::DoModal()
{
	// Declare variables
	BOOL bRetValue;
	DWORD dwWinMajor;

	// Get OS version
	dwWinMajor = (DWORD)(LOBYTE(LOWORD(::GetVersion())));
	if (dwWinMajor >= 5)
	{
		m_ofn.lStructSize = sizeof(m_ofn);
	}
	else
	{
		m_ofn.lStructSize = sizeof(OPENFILENAME);
	}

	// Execute dialog
	if (m_bOpenFileDialog)
	{
		bRetValue = ::GetOpenFileName(&m_ofn);
	}
	else
	{
		bRetValue = ::GetSaveFileName(&m_ofn);
	}

	return (bRetValue ? IDOK : IDCANCEL);
}

CString COptionTreeFileDlg::GetPathName() const
{
	return m_ofn.lpstrFile;
}

CString COptionTreeFileDlg::GetFileName() const
{
	return m_ofn.lpstrFileTitle;
}

CString COptionTreeFileDlg::GetFileTitle() const
{
	TCHAR szTitle[MAX_PATH];

	// Validate
	if (m_ofn.lpstrFile == NULL)
	{
		return  _T("");
	}

	// Split path into components
	_tsplitpath(m_ofn.lpstrFile, NULL, NULL, szTitle, NULL);

	return szTitle;
}

CString COptionTreeFileDlg::GetFileExt() const
{
	// Declare variables
	TCHAR szExt[MAX_PATH];

	// Validate
	if (m_ofn.lpstrFile == NULL)
	{
		return  _T("");
	}

	// Split path into components
	_tsplitpath(m_ofn.lpstrFile, NULL, NULL, NULL, szExt);

	memmove(szExt, szExt + 1,strlen(szExt) -1);
	szExt[strlen(szExt) -1] = '\0';

	return szExt;
}

CString COptionTreeFileDlg::GetFileDir() const
{
	// Declare variables
	TCHAR szDrive[MAX_PATH];
	TCHAR szDir[MAX_PATH];

	// Validate
	if (m_ofn.lpstrFile == NULL)
	{
		return  _T("");
	}

	// Split path into components
	_tsplitpath(m_ofn.lpstrFile, szDrive, szDir, NULL, NULL);
	::lstrcat(szDrive, szDir);

	return szDrive;
}

CString COptionTreeFileDlg::GetFileDrive() const
{
	// Declare variables
	TCHAR szDrive[MAX_PATH];

	// Validate
	if (m_ofn.lpstrFile == NULL)
	{
		return  _T("");
	}

	// Split path into components
	_tsplitpath(m_ofn.lpstrFile, szDrive, NULL, NULL, NULL);

	return szDrive;
}

POSITION COptionTreeFileDlg::GetStartPosition() const
{
	return (POSITION)m_ofn.lpstrFile;
}

CString COptionTreeFileDlg::GetNextPathName(POSITION& pos) const
{
	// Declare variables
	BOOL bExplorer = m_ofn.Flags & OFN_EXPLORER;
	TCHAR chDelimiter;
	LPTSTR lpsz;
	CString strPath;
	LPTSTR lpszPath;
	LPTSTR lpszFileName;
	CString strFileName;

	// Explorer?
	if (bExplorer)
		chDelimiter = _T('\0');
	else
		chDelimiter = _T(' ');

	// Get it
	lpsz = (LPTSTR)pos;
	if (lpsz == m_ofn.lpstrFile) 
	{
		if ((m_ofn.Flags & OFN_ALLOWMULTISELECT) == FALSE)
		{
			pos = NULL;
			return m_ofn.lpstrFile;
		}

		// -- find char pos after first Delimiter
		while(*lpsz != chDelimiter && *lpsz != _T('\0'))
			lpsz = _tcsinc(lpsz);
		lpsz = _tcsinc(lpsz);

		// -- If single selection then return only selection
		if (*lpsz == 0)
		{
			pos = NULL;
			return m_ofn.lpstrFile;
		}
	}
	strPath = m_ofn.lpstrFile;
	if (!bExplorer)
	{
		lpszPath = m_ofn.lpstrFile;
		while(*lpszPath != chDelimiter)
		{
			lpszPath = _tcsinc(lpszPath);
		}
		strPath = strPath.Left(lpszPath - m_ofn.lpstrFile);
	}
	lpszFileName = lpsz;
	strFileName = lpsz;

	// Find char pos at next Delimiter
	while(*lpsz != chDelimiter && *lpsz != _T('\0'))
	{
		lpsz = _tcsinc(lpsz);
	}

	if (!bExplorer && *lpsz == _T('\0'))
	{
		pos = NULL;
	}
	else
	{
		if (!bExplorer)
		{
			strFileName = strFileName.Left(lpsz - lpszFileName);
		}

		// -- If double terminated then done
		lpsz = _tcsinc(lpsz);
		if (*lpsz == _T('\0')) 
		{
			pos = NULL;
		}
		else
		{
			pos = (POSITION)lpsz;
		}
	}

	// -- Only add '\\' if it is needed
	if (!strPath.IsEmpty())
	{
		// -- check for last back-slash or forward slash (handles DBCS)
		LPCTSTR lpsz = _tcsrchr(strPath, _T('\\'));
		if (lpsz == NULL)
		{
			lpsz = _tcsrchr(strPath, _T('/'));
		}
		// -- if it is also the last character, then we don't need an extra
		if (lpsz != NULL &&	(lpsz - (LPCTSTR)strPath) == strPath.GetLength()-1)
		{
			ASSERT(*lpsz == _T('\\') || *lpsz == _T('/'));
			return strPath + strFileName;
		}
	}

	return strPath + _T('\\') + strFileName;
}

int COptionTreeFileDlg::SelectFolder(LPCTSTR lpszTitle, LPCTSTR lpszStartPath, UINT ulFlags, CWnd* pParentWnd)
{
	// Declare variables
	LPMALLOC pMalloc;
	BROWSEINFO bi;
	LPITEMIDLIST pidl;
	int	nRetValue = IDCANCEL;
	::ZeroMemory(&bi, sizeof(bi));

	// Gets the Shell's default allocator
	if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
		// -- Get help on BROWSEINFO struct - it's got all the bit settings.
		if (pParentWnd != NULL)
		{
			bi.hwndOwner = pParentWnd->GetSafeHwnd();
		}
		bi.pidlRoot = NULL;
		bi.pszDisplayName = m_szSelectedFolder;
		bi.lpszTitle = lpszTitle;
		bi.ulFlags = ulFlags;
		bi.lpfn = BrowseCtrlCallback;
		bi.lParam = (LPARAM)lpszStartPath;
		
		// -- This next call issues the dialog box.
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, m_szSelectedFolder))
			{ 
				// -- -- At this point pszBuffer contains the selected path
				nRetValue = IDOK;
			} 

			// -- -- Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pidl);
		}

		// -- Release the shell's allocator.
		pMalloc->Release();
	}

	return nRetValue;
}

int __stdcall COptionTreeFileDlg::BrowseCtrlCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED && lpData != NULL)
	{
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

CString COptionTreeFileDlg::GetSelectedFolder() const
{
	return m_szSelectedFolder;
}