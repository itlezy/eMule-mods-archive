// MiniVersion.h  Version 1.1
//
// Author:  Hans Dietrich
//          hdietrich2@hotmail.com
//
// This software is released into the public domain.
// You are free to use it in any way you like, except
// that you may not sell this source code.
//
// This software is provided "as is" with no expressed
// or implied warranty.  I accept no liability for any
// damage or loss of business that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _WINDOWS_

class CMiniVersion
{
// constructors
public:
	CMiniVersion(LPCTSTR lpszPath = NULL);
	BOOL Init();
	void Release();

// operations
public:

// attributes
public:
	// fixed info
	BOOL GetFileVersion(WORD *pwVersion);
	BOOL GetProductVersion(WORD* pwVersion);
	BOOL GetFileFlags(DWORD& rdwFlags);
	BOOL GetFileOS(DWORD& rdwOS);
	BOOL GetFileType(DWORD& rdwType);
	BOOL GetFileSubtype(DWORD& rdwType);	

	// string info
	BOOL GetCompanyName(LPTSTR lpszCompanyName, int nSize);
	BOOL GetProductName(LPTSTR lpszProductName, int nSize);
	BOOL GetFileDescription(LPTSTR lpszFileDescription, int nSize);
	BOOL GetLegalCopyright(LPTSTR lpszLegalCopyright, int nSize);
	BOOL GetFileVersionString(LPTSTR lpszFileVersion, int nSize);
	BOOL GetProductVersionString(LPTSTR lpszProductVersion, int nSize);

// implementation
protected:
	BOOL GetFixedInfo(VS_FIXEDFILEINFO& rFixedInfo);
	BOOL GetStringInfo(LPCTSTR lpszKey, LPTSTR lpszValue);

	BYTE*		m_pData;
	DWORD		m_dwHandle;
	WORD		m_wFileVersion[4];
	WORD		m_wProductVersion[4];
	DWORD		m_dwFileFlags;
	DWORD		m_dwFileOS;
	DWORD		m_dwFileType;
	DWORD		m_dwFileSubtype;

	TCHAR		m_szPath[MAX_PATH*2];
	TCHAR		m_szCompanyName[MAX_PATH*2];
	TCHAR		m_szProductName[MAX_PATH*2];
	TCHAR		m_szFileDescription[MAX_PATH*2];
	TCHAR		m_szLegalCopyright[MAX_PATH*2];
	TCHAR		m_szFileVersion[MAX_PATH*2];
	TCHAR		m_szProductVersion[MAX_PATH*2];
};

#endif // _WINDOWS_
