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
// Ini2.cpp: Ini-File-Interface by bond006 <rene.landgrebe@gmx.de>
// ****************************************************************
// This new interface replaces the old interface which was too slow

#include "stdafx.h"
#include "ini2.h"
#include "otherfunctions.h"
#include "StringConversion.h"

CIni::CIni(const CString &strFileName, int iMode/*=0*/)
	: m_strDefCategory(_T("")), m_strFileBuffer(_T(""))
	, m_strDefCatBraces(_T("")), m_strDefCatBracesCR(_T(""))
	, m_strInternXchg(_T(""))
{
	OpenFile(strFileName, iMode);
	ReadData(iMode);
	m_strInternXchg.Preallocate(128);
}

CIni::~CIni()
{
	CloseFile();
}

void CIni::OpenFile(const CString &strFileName, int iMode)
{
	if (m_hFile == CFile::hFileNull)
		Open( strFileName, (iMode & INI_MODE_READONLY) ? (CFile::modeRead | CFile::shareDenyWrite) :
			(CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite | CFile::shareDenyWrite) );
}

void CIni::CloseFile()
{
	if (m_hFile != CFile::hFileNull)
		Close();
}

void CIni::ReadData(int iMode)
{
	if (m_hFile != CFile::hFileNull)
	{
		char tempBuffer[131072]; // change this value to support larger files than 128KB
		uint32 dwOffset = 0;
		ECodingFormat eCF = cfLocalCodePage;

		memzero(tempBuffer, sizeof(tempBuffer));

		uint32 dwReaded = Read(tempBuffer, sizeof(tempBuffer) - 1);

		if (dwReaded > 3 && HAS_BOM(&tempBuffer[0]))
		{
			eCF = cfUTF8withBOM;
			dwOffset = 3;
		//	Drop extra end of line as we explicitly add it while saving
			while (tempBuffer[dwOffset] == '\r' || tempBuffer[dwOffset] == '\n')
				dwOffset++;
		}

		if ((eCF == cfLocalCodePage) && ((iMode & INI_MODE_ANSIONLY) == 0))
		{
		//	Since original eMule does not convert all potential Unicode strings into UTF8
		//	(e.g. path, message filter) it could be that ini file contains string etries
		//	in mixed format (some in UTF8 while others in ANSI). So we need to verify the format
		//	for every entry in INI file. The verification is based on following assumptions:
		//	1) the entry name is ASCII characters
		//	2) value of the entry in ANSI or UTF8 format
			const char *const pcSep = "\n";
			char *pcToken = strtok(tempBuffer, pcSep);
			CString strToken;

			m_strFileBuffer.Preallocate(dwReaded);

			while (pcToken != NULL)
			{
				if (IsUTF8(pcToken))
					MB2Str(cfUTF8, &strToken, pcToken);
				else
					MB2Str(cfLocalCodePage, &strToken, pcToken);

				if (!strToken.IsEmpty())
					m_strFileBuffer.Append(strToken);
				m_strFileBuffer.AppendChar(_T('\n'));

				pcToken = strtok(NULL, pcSep);
			}
		}
		else
			MB2Str(eCF, &m_strFileBuffer, &tempBuffer[dwOffset], (dwReaded - dwOffset));

		// Cut off blank lines at the end of the file
		m_strFileBuffer.TrimRight(_T("\r\n"));
	}
}

void CIni::SaveAndClose()
{
	if (m_hFile != CFile::hFileNull)
	{
		SetLength(0);
		m_strFileBuffer.TrimRight(_T("\r\n"));

	//	For backward compatibility save the ini with BOM only if Unicode is used
		ECodingFormat eCF = cfLocalCodePage;
		static const char s_acBOMHeader[5] = {0xEFu, 0xBBu, 0xBFu, '\r', '\n'};  

		if (IsUTF8Required(m_strFileBuffer))
		{
			eCF = cfUTF8;
		//	Here end-of-line is added not to break standard Windows ini file parser (which is used by other clients)
			Write(s_acBOMHeader, ARRSIZE(s_acBOMHeader));
		}
		WriteStr2MB(eCF, m_strFileBuffer, *this);
	}

	CloseFile();
}

void CIni::AddCategory(const CString &strCategoryName)
{	//	strCategoryName is in "[Name]" format
	if (!CategoryExist(strCategoryName))
	{
		if (!m_strFileBuffer.IsEmpty())
			m_strFileBuffer += _T("\r\n");
		m_strFileBuffer += strCategoryName;
	}
}

void CIni::SetDefaultCategory(const CString& strCategoryName)
{
	m_strDefCategory = strCategoryName;
	m_strDefCatBraces.Format(_T("[%s]"), m_strDefCategory);
	m_strDefCatBracesCR.Format(_T("\r\n[%s]"), m_strDefCategory);

	AddCategory(m_strDefCatBraces);
}

void CIni::SetDefaultCategory(const TCHAR *pcCategoryName)
{
	m_strDefCategory = pcCategoryName;
	m_strDefCatBraces.Format(_T("[%s]"), m_strDefCategory);
	m_strDefCatBracesCR.Format(_T("\r\n[%s]"), m_strDefCategory);

	AddCategory(m_strDefCatBraces);
}

void CIni::DeleteCategory(const CString& strCategoryName)
{
	CString catSearchString = _T("[");
	catSearchString += strCategoryName;
	catSearchString += _T(']');

	if (CategoryExist(catSearchString))
	{
		int nCatFirstCharPos = m_strFileBuffer.Find(catSearchString);

		// Check if the category is valid
		if (nCatFirstCharPos != 0)
		{
			catSearchString = _T("\r\n[");
			catSearchString += strCategoryName;
			catSearchString += _T(']');

			nCatFirstCharPos -= 2;
			nCatFirstCharPos = m_strFileBuffer.Find(catSearchString, nCatFirstCharPos);
			nCatFirstCharPos += 2;
		}

		int nCatLastCharPos = m_strFileBuffer.Find(_T("\r\n["), (nCatFirstCharPos + strCategoryName.GetLength() + 1));

		if (nCatLastCharPos < 0)
			nCatLastCharPos = m_strFileBuffer.GetLength();
		else
			nCatLastCharPos += 2;

		m_strFileBuffer.Delete(nCatFirstCharPos, (nCatLastCharPos - nCatFirstCharPos));
	}
}

void CIni::DeleteEntry(const CString& strCategoryName, const CString &strEntryName)
{
	CString catSearchString = _T("[");
	catSearchString += strCategoryName;
	catSearchString += _T(']');

	if (EntryExist(catSearchString, strEntryName))
	{
		int nEntryFirstCharPos, nEntryLastCharPos;
		int nCatFirstCharPos = m_strFileBuffer.Find(catSearchString);

		// Check if the category is valid
		if (nCatFirstCharPos != 0)
		{
			catSearchString = _T("\r\n[");
			catSearchString += strCategoryName;
			catSearchString += _T(']');

			nCatFirstCharPos -= 2;
			nCatFirstCharPos = m_strFileBuffer.Find(catSearchString, nCatFirstCharPos);
			nCatFirstCharPos += 2;
		}

		CString entrySearchString = _T("\r\n");
		entrySearchString += strEntryName;
		entrySearchString += _T('=');

		nEntryFirstCharPos = m_strFileBuffer.Find(entrySearchString, (nCatFirstCharPos + strCategoryName.GetLength() + 1));
		nEntryFirstCharPos += 2;

		nEntryLastCharPos = m_strFileBuffer.Find(_T("\r\n"), nEntryFirstCharPos);

		if (nEntryLastCharPos < 0)
			nEntryLastCharPos = m_strFileBuffer.GetLength();
		else
			nEntryLastCharPos += 2; // remove entry including "\r\n"

		m_strFileBuffer.Delete(nEntryFirstCharPos, (nEntryLastCharPos - nEntryFirstCharPos));
	}
}

__declspec(noinline) const CString& CIni::GetString(const TCHAR *pcEntryName, const TCHAR *pcValue)
{
	GetValue(pcEntryName, &m_strInternXchg);
	if (m_strInternXchg.IsEmpty() && !EntryExist(m_strDefCatBraces, pcEntryName))
		m_strInternXchg = pcValue;
	return m_strInternXchg;
}

__declspec(noinline) void CIni::GetString(CString *pstrOut, const TCHAR *pcEntryName, const TCHAR *pcValue)
{
	GetValue(pcEntryName, pstrOut);
	if (pstrOut->IsEmpty() && !EntryExist(m_strDefCatBraces, pcEntryName))
		*pstrOut = pcValue;
}

__declspec(noinline) double CIni::GetDouble(const TCHAR *pcEntryName, double defValue)
{
	GetValue(pcEntryName, &m_strInternXchg);
	if (m_strInternXchg.IsEmpty() && !EntryExist(m_strDefCatBraces, pcEntryName))
		return defValue;
	else
		return _tstof(m_strInternXchg);
}

__declspec(noinline) int CIni::GetInt(const TCHAR *pcEntryName, int defValue)
{
	GetValue(pcEntryName, &m_strInternXchg);
	if (m_strInternXchg.IsEmpty() && !EntryExist(m_strDefCatBraces, pcEntryName))
		return defValue;
	else
		return _tstoi(m_strInternXchg);
}

uint32 CIni::GetUInt32(const TCHAR *pcEntryName, uint32 defValue)
{
	return static_cast<uint32>(GetInt(pcEntryName, defValue));
}

uint16 CIni::GetUInt16(const TCHAR *pcEntryName, int iDefVal)
{
	return static_cast<uint16>(GetInt(pcEntryName, iDefVal));
}

__declspec(noinline) uint64 CIni::GetUInt64(const TCHAR *pcEntryName, uint64 defValue)
{
	GetValue(pcEntryName, &m_strInternXchg);
	if (m_strInternXchg.IsEmpty() && !EntryExist(m_strDefCatBraces, pcEntryName))
		return defValue;
	else
		return _tstoi64(m_strInternXchg);
}

WORD CIni::GetWORD(const TCHAR *pcEntryName, WORD defValue)
{
	return static_cast<WORD>(GetUInt32(pcEntryName, defValue));
}

__declspec(noinline) bool CIni::GetBool(const TCHAR *pcEntryName, bool defValue)
{
	GetValue(pcEntryName, &m_strInternXchg);
	if (m_strInternXchg.IsEmpty() && !EntryExist(m_strDefCatBraces, pcEntryName))
		return defValue;
	else
		return (_tstoi(m_strInternXchg) != 0);
}

void CIni::SetString(const TCHAR *pcEntryName, const CString& strValue)
{
	SetValue(pcEntryName, strValue);
}

void CIni::SetDouble(const TCHAR *pcEntryName, double dValue)
{
	m_strInternXchg.Format(_T("%g"), dValue);
	SetValue(pcEntryName, m_strInternXchg);
}

void CIni::SetInt(const TCHAR *pcEntryName, int iValue)
{
	m_strInternXchg.Format(_T("%d"), iValue);
	SetValue(pcEntryName, m_strInternXchg);
}

void CIni::SetUInt32(const TCHAR *pcEntryName, uint32 dwValue)
{
	m_strInternXchg.Format(_T("%u"), dwValue);
	SetValue(pcEntryName, m_strInternXchg);
}

void CIni::SetUInt64(const TCHAR *pcEntryName, uint64 qwValue)
{
	m_strInternXchg.Format(_T("%I64u"), qwValue);
	SetValue(pcEntryName, m_strInternXchg);
}

void CIni::SetWORD(const TCHAR *pcEntryName, WORD wValue)
{
	SetUInt32(pcEntryName, wValue);
}

void CIni::SetBool(const TCHAR *pcEntryName, bool bValue)
{
	m_strInternXchg = (bValue) ? _T("1") : _T("0");
	SetValue(pcEntryName, m_strInternXchg);
}

void CIni::GetArray(CString *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	CString temp = GetString(pcEntryName, _T(""));

	int nPos = 0;
	int nLastPos = 0;
	int nEscapePos = 0;

	// bugfix: using escape characters for "\" and "," in strings - 08/25/03
	if (temp.Find(_T('\\')) >= 0)
	{
		for (unsigned ui = 0; ui < uiElements; ui++)
		{
			nLastPos = nPos;
			nPos = temp.Find(_T(','), nLastPos);
			nEscapePos = temp.Find(_T('\\'), nLastPos);

			if(nPos==(-2))
				pToArray[ui] = _T("");

			if(nEscapePos==(-1) && nPos!=(-2))
			{
				if(nPos == (-1))
				{
					pToArray[ui] = temp.Mid(nLastPos, (temp.GetLength() - nLastPos));
					nPos = -2;
				}
				else
				{
					pToArray[ui] = (nPos >= 0) ? temp.Mid(nLastPos, (nPos - nLastPos)) : _T("");
					nPos++;
				}
			}
			else if(nEscapePos!=(-1) && nPos!=(-2))
			{
				if(nPos < nEscapePos)
				{
					if(nPos == (-1))
					{
						pToArray[ui] = temp.Mid(nLastPos, (temp.GetLength() - nLastPos));
						nPos = -2;
					}
					else
					{
						pToArray[ui] = (nPos >= 0) ? temp.Mid(nLastPos, (nPos - nLastPos)) : _T("");
						nPos++;
					}
				}
				else
				{
					int saveBegPos = nLastPos;

					while(nPos > nEscapePos && nEscapePos != (-1))
					{
						nLastPos = nEscapePos + 2;
						nPos = temp.Find(_T(','), nLastPos);
						nEscapePos = temp.Find(_T('\\'), nLastPos);
					}

					if(nPos == (-1))
					{
						pToArray[ui] = temp.Mid(saveBegPos,(temp.GetLength() - saveBegPos));
						nPos = -2;
					}
					else
					{
						pToArray[ui] = (nPos >= 0) ? temp.Mid(saveBegPos,(nPos - saveBegPos)) : _T("");
						nPos++;
					}
				}
			}

			pToArray[ui].Replace(_T("\\,"), _T(","));
			pToArray[ui].Replace(_T("\\\\"), _T("\\"));
		}
	}
	else
	{
		for (unsigned ui = 0; ui < uiElements; ui++)
			pToArray[ui] = (nPos >= 0) ? temp.Tokenize(_T(","), nPos) : _T("");
	}
}

void CIni::GetArray(double *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	CString temp = GetString(pcEntryName, _T(""));

	int iPos = 0;

//	Different delimiter is used as comma is a decimal-point in some languages
	for (unsigned ui = 0; ui < uiElements; ui++)
		pToArray[ui] = (iPos >= 0) ? _tstof(temp.Tokenize(_T("|"), iPos)) : 0.;
}

void CIni::GetArray(int *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	CString temp = GetString(pcEntryName, _T(""));

	int iPos = 0;

	for (unsigned ui = 0; ui < uiElements; ui++)
		pToArray[ui] = (iPos >= 0) ? _tstoi(temp.Tokenize(_T(","), iPos)) : 0;
}

void CIni::GetArray(WORD *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	CString temp = GetString(pcEntryName, _T(""));

	int iPos = 0;

	for (unsigned ui = 0; ui < uiElements; ui++)
		pToArray[ui] = (iPos >= 0) ? (WORD)_tstoi(temp.Tokenize(_T(","), iPos)) : 0;
}

void CIni::GetArray(bool *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	CString temp = GetString(pcEntryName, _T(""));

	int iPos = 0;

	for (unsigned ui = 0; ui < uiElements; ui++)
		pToArray[ui] = (((iPos >= 0) ? _tstoi(temp.Tokenize(_T(","), iPos)) : 0) != 0);
}

void CIni::SetArray(CString *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	m_strInternXchg.Truncate(0);
	for (unsigned ui = 0; ui < uiElements; ui++)
	{
		// bugfix: using escape characters for "\" and "," in strings - 08/25/03
		pToArray[ui].Replace(_T("\\"), _T("\\\\"));
		pToArray[ui].Replace(_T(","), _T("\\,"));

		m_strInternXchg += pToArray[ui];
		m_strInternXchg += _T(",");
	}
	m_strInternXchg.TrimRight(_T(','));

	SetString(pcEntryName, m_strInternXchg);
}

void CIni::SetArray(const double *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	m_strInternXchg.Truncate(0);

//	Different delimiter is used as comma is a decimal-point in some languages
	for (unsigned ui = 0; ui < uiElements; ui++)
		m_strInternXchg.AppendFormat(_T("%g|"), pToArray[ui]);
	m_strInternXchg.TrimRight(_T('|'));

	SetString(pcEntryName, m_strInternXchg);
}

void CIni::SetArray(const int *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	m_strInternXchg.Truncate(0);

	for (unsigned ui = 0; ui < uiElements; ui++)
		m_strInternXchg.AppendFormat(_T("%d,"), pToArray[ui]);
	m_strInternXchg.TrimRight(_T(','));

	SetString(pcEntryName, m_strInternXchg);
}

void CIni::SetArray(const WORD *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	m_strInternXchg.Truncate(0);

	for (unsigned ui = 0; ui < uiElements; ui++)
		m_strInternXchg.AppendFormat(_T("%u,"), pToArray[ui]);
	m_strInternXchg.TrimRight(_T(','));

	SetString(pcEntryName, m_strInternXchg);
}

void CIni::SetArray(const bool *pToArray, unsigned uiElements, const TCHAR *pcEntryName)
{
	m_strInternXchg.Truncate(0);

	for (unsigned ui = 0; ui < uiElements; ui++)
		m_strInternXchg.AppendFormat(_T("%d,"), pToArray[ui]);
	m_strInternXchg.TrimRight(_T(','));

	SetString(pcEntryName, m_strInternXchg);
}

void CIni::SetValue(const TCHAR *pcEntryName, const CString &strValue)
{
	CString	strEntryName(pcEntryName);

	if (EntryExist(m_strDefCatBraces, pcEntryName))
	{
		int nValueFirstCharPos, nEntryFirstCharPos, nEntryLastCharPos;
		int nCatFirstCharPos = m_strFileBuffer.Find(m_strDefCatBraces);

		// Check if the category is valid
		if (nCatFirstCharPos != 0)
		{
			nCatFirstCharPos -= 2;
			nCatFirstCharPos = m_strFileBuffer.Find(m_strDefCatBracesCR, nCatFirstCharPos);
			nCatFirstCharPos += 2;
		}

		CString entrySearchString = _T("\r\n");
		entrySearchString += strEntryName;
		entrySearchString += _T('=');

		nEntryFirstCharPos = m_strFileBuffer.Find(entrySearchString, (nCatFirstCharPos + m_strDefCategory.GetLength() + 1));
		nEntryFirstCharPos += 2;
		nEntryLastCharPos = m_strFileBuffer.Find(_T("\r\n"), nEntryFirstCharPos);

		if (nEntryLastCharPos < 0)
			nEntryLastCharPos = m_strFileBuffer.GetLength();

		nValueFirstCharPos = nEntryFirstCharPos + strEntryName.GetLength() + 1;

		// Delete an existing value and insert a new value
		m_strFileBuffer.Delete(nValueFirstCharPos, (nEntryLastCharPos - nValueFirstCharPos));
		m_strFileBuffer.Insert(nValueFirstCharPos, strValue);
	}
	else if (CategoryExist(m_strDefCatBraces))
	{
		int nCatLastCharPos, nCatFirstCharPos = m_strFileBuffer.Find(m_strDefCatBraces);

		// Check if the category is valid
		if (nCatFirstCharPos != 0)
		{
			nCatFirstCharPos -= 2;
			nCatFirstCharPos = m_strFileBuffer.Find(m_strDefCatBracesCR, nCatFirstCharPos);
			nCatFirstCharPos += 2;
		}

		nCatLastCharPos = m_strFileBuffer.Find(_T("\r\n["), (nCatFirstCharPos + m_strDefCategory.GetLength() + 1));

		CString entryString;

		strEntryName += _T('=');
		strEntryName += strValue;

		if (nCatLastCharPos < 0)
		{
			nCatLastCharPos = m_strFileBuffer.GetLength();
			entryString = _T("\r\n");
			entryString += strEntryName;
		}
		else
		{
			nCatLastCharPos += 2;
			entryString = strEntryName;
			entryString += _T("\r\n");
		}

		m_strFileBuffer.Insert(nCatLastCharPos, entryString);
	}
}

void CIni::GetValue(const TCHAR *pcEntryName, CString *pstrOut)
{
	CString	strEntryName(pcEntryName);

	if (ValueExist(m_strDefCatBraces, strEntryName))
	{
		int nValueFirstCharPos, nEntryFirstCharPos, nEntryLastCharPos;
		int nCatFirstCharPos = m_strFileBuffer.Find(m_strDefCatBraces);

		// Check if the category is valid
		if (nCatFirstCharPos != 0)
		{
			nCatFirstCharPos -= 2;
			nCatFirstCharPos = m_strFileBuffer.Find(m_strDefCatBracesCR, nCatFirstCharPos);
			nCatFirstCharPos += 2;
		}

		CString entrySearchString = _T("\r\n");
		entrySearchString += strEntryName;
		entrySearchString += _T('=');

		nEntryFirstCharPos = m_strFileBuffer.Find(entrySearchString, (nCatFirstCharPos + m_strDefCategory.GetLength() + 1));
		nEntryFirstCharPos += 2;
		nEntryLastCharPos = m_strFileBuffer.Find(_T("\r\n"), nEntryFirstCharPos);

		if (nEntryLastCharPos < 0)
			nEntryLastCharPos = m_strFileBuffer.GetLength();

		nValueFirstCharPos = nEntryFirstCharPos + strEntryName.GetLength() + 1;

		*pstrOut = m_strFileBuffer.Mid(nValueFirstCharPos, (nEntryLastCharPos - nValueFirstCharPos));
	}
	else
		*pstrOut = _T("");
}

bool CIni::CategoryExist(const CString &strCategoryName)
{	//	strCategoryName is in "[Name]" format
	int iFindPos = m_strFileBuffer.Find(strCategoryName);

	if (iFindPos < 0)
		return false;

	// Check if the category is valid
	if (iFindPos != 0)
	{
		CString	catSearchString(_T("\r\n"));

		catSearchString += strCategoryName;
		iFindPos -= 2;
		iFindPos = m_strFileBuffer.Find(catSearchString, iFindPos);

		if (iFindPos < 0)
			return false;
	}

	return true;
}

bool CIni::EntryExist(const CString &strCategoryName, const TCHAR *pcEntryName)
{	//	strCategoryName is in "[Name]" format
	int nEntryFirstCharPos, nCatFirstCharPos, nCatLastCharPos;

	if ((nCatFirstCharPos = m_strFileBuffer.Find(strCategoryName)) < 0)
		return false;	// Category does not exist

	// Check if the category is valid
	if (nCatFirstCharPos != 0)
	{
		CString	catSearchString(_T("\r\n"));

		catSearchString += strCategoryName;
		nCatFirstCharPos -= 2;
		nCatFirstCharPos = m_strFileBuffer.Find(catSearchString, nCatFirstCharPos);

		if (nCatFirstCharPos < 0)
			return false;

		nCatFirstCharPos += 2;
	}

	nCatLastCharPos = m_strFileBuffer.Find(_T("\r\n["), (nCatFirstCharPos + strCategoryName.GetLength() - 1));

	if (nCatLastCharPos < 0)
		// No following category exists - end of category is end of file
		nCatLastCharPos = m_strFileBuffer.GetLength();
	else
		nCatLastCharPos += 2;

	CString entrySearchString = _T("\r\n");
	entrySearchString += pcEntryName;
	entrySearchString += _T('=');

	nEntryFirstCharPos = m_strFileBuffer.Find(entrySearchString, (nCatFirstCharPos + strCategoryName.GetLength() - 1));
	
	if (nEntryFirstCharPos >= 0)
		nEntryFirstCharPos += 2;
	else
		return false;

	if (nEntryFirstCharPos < nCatFirstCharPos || nEntryFirstCharPos > nCatLastCharPos)
		// The entry we're searching for does not exist in the given category
		return false;

	return true;
}

bool CIni::ValueExist(const CString &strCategoryName, const CString &strEntryName)
{	//	strCategoryName is in "[Name]" format
	int nEntryFirstCharPos, nCatFirstCharPos, nCatLastCharPos;

	if ((nCatFirstCharPos = m_strFileBuffer.Find(strCategoryName)) < 0)
		return false;	// Category does not exist

	// Check if the category is valid
	if (nCatFirstCharPos != 0)
	{
		CString	catSearchString(_T("\r\n"));

		catSearchString += strCategoryName;
		nCatFirstCharPos -= 2;
		nCatFirstCharPos = m_strFileBuffer.Find(catSearchString, nCatFirstCharPos);

		if (nCatFirstCharPos < 0)
			return false;

		nCatFirstCharPos += 2;
	}

	nCatLastCharPos = m_strFileBuffer.Find(_T("\r\n["), (nCatFirstCharPos + strCategoryName.GetLength() - 1));

	if (nCatLastCharPos < 0)
		// No following category exists - end of category is end of file
		nCatLastCharPos = m_strFileBuffer.GetLength();
	else
		nCatLastCharPos += 2;

	CString entrySearchString = _T("\r\n");
	entrySearchString += strEntryName;
	entrySearchString += _T('=');

	nEntryFirstCharPos = m_strFileBuffer.Find(entrySearchString, (nCatFirstCharPos + strCategoryName.GetLength() - 1));

	if (nEntryFirstCharPos >= 0)
		nEntryFirstCharPos += 2;
	else
		return false;

	if(nEntryFirstCharPos < nCatFirstCharPos || nEntryFirstCharPos > nCatLastCharPos)
		// The entry we're searching for does not exist in the given category
		return false;

	int nEndOfLine = m_strFileBuffer.Find(_T("\r\n"), nEntryFirstCharPos);

	if (nEndOfLine < 0)
		nEndOfLine = m_strFileBuffer.GetLength();

	if ((nEntryFirstCharPos + strEntryName.GetLength() + 1) == nEndOfLine)
		return false;

	return true;
}
