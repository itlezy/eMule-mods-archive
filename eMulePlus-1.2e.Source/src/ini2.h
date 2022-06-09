//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// Ini2.h: Ini-File-Interface by bond006 <rene.landgrebe@gmx.de>
// ****************************************************************
// This new interface replaces the old interface which was too slow
#pragma once

#include "types.h"

#define INI_MODE_READONLY		0x01
#define INI_MODE_ANSIONLY		0x02

class CIni : public CFile
{
public:
	CIni(const CString &strFileName, int iMode = 0);
	~CIni();

	void		CloseWithoutSave(){CloseFile();}
	void		SaveAndClose();

	void		AddCategory(const CString& strCategoryName);
	void		SetDefaultCategory(const CString& strCategoryName);
	void		SetDefaultCategory(const TCHAR *pcCategoryName);
	void		DeleteCategory(const CString& strCategoryName);
	void		DeleteEntry(const CString& strCategoryName, const CString& strEntryName);

	__declspec(noinline) const CString&	GetString(const TCHAR *pcEntryName, const TCHAR *pcValue);
	__declspec(noinline) void		GetString(CString *pstrOut, const TCHAR *pcEntryName, const TCHAR *pcValue);
	__declspec(noinline) double		GetDouble(const TCHAR *pcEntryName, double defValue);
	__declspec(noinline) int		GetInt(const TCHAR *pcEntryName, int defValue);
	WORD		GetWORD(const TCHAR *pcEntryName, WORD defValue);
	uint32		GetUInt32(const TCHAR *pcEntryName, uint32 defValue);
	uint16		GetUInt16(const TCHAR *pcEntryName, int iDefVal);
	__declspec(noinline) uint64		GetUInt64(const TCHAR *pcEntryName, uint64 defValue);
	__declspec(noinline) bool		GetBool(const TCHAR *pcEntryName, bool defValue);
	void		SetString(const TCHAR *pcEntryName, const CString& strValue);
	void		SetDouble(const TCHAR *pcEntryName, double dValue);
	void		SetInt(const TCHAR *pcEntryName, int iValue);
	void		SetWORD(const TCHAR *pcEntryName, WORD wValue);
	void		SetUInt32(const TCHAR *pcEntryName, uint32 dwValue);
	void		SetUInt64(const TCHAR *pcEntryName, uint64 qwValue);
	void		SetBool(const TCHAR *pcEntryName, bool bValue);

	void		GetArray(CString *pString, unsigned uiElements, const TCHAR *pcEntryName);
	void		GetArray(double *pDouble, unsigned uiElements, const TCHAR *pcEntryName);
	void		GetArray(int *pInt, unsigned uiElements, const TCHAR *pcEntryName);
	void		GetArray(WORD *pWord, unsigned uiElements, const TCHAR *pcEntryName);
	void		GetArray(bool *pBool, unsigned uiElements, const TCHAR *pcEntryName);
	void		SetArray(CString *pString, unsigned uiElements, const TCHAR *pcEntryName);
	void		SetArray(const double *pDouble, unsigned uiElements, const TCHAR *pcEntryName);
	void		SetArray(const int *pInt, unsigned uiElements, const TCHAR *pcEntryName);
	void		SetArray(const WORD *pWord, unsigned uiElements, const TCHAR *pcEntryName);
	void		SetArray(const bool *pBool, unsigned uiElements, const TCHAR *pcEntryName);

	bool		CategoryExist(const CString& strCategoryName);
	bool		EntryExist(const CString &strCategoryName, const TCHAR *pcEntryName);
	bool		ValueExist(const CString& strCategoryName, const CString& strEntryName);

private:
	void		OpenFile(const CString &strFileName, int iMode);
	void		CloseFile();
	void		ReadData(int iMode);

	void		SetValue(const TCHAR *pcEntryName, const CString &strValue);
	void		GetValue(const TCHAR *pcEntryName, CString *pstrOut);

	CString m_strDefCategory;
	CString m_strFileBuffer;
//	Used as internal class exchange buffer to avoid numerous memory allocations
//	based on the fact that all requests are done consecutively from one thread
	CString m_strInternXchg;
	CString m_strDefCatBraces;		// default category in format "[Name]"
	CString m_strDefCatBracesCR;	// default category in format "\r\n[Name]"
};
