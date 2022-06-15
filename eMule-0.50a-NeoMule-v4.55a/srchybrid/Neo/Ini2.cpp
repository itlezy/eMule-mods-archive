//this file is part of NeoMule
//Copyright (C)2007
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
#include "ini2.h"
#include "otherfunctions.h"
#include "StringConversion.h"

/***********************************************
* Construct And File Operations
*/

CIni::CIni(const CString &strFileName, const CString& strCategory)
{
	if(!strFileName.IsEmpty()){
		m_File = new CStdioFile;
		OpenFile(strFileName);
		ReadData();
	}else
		m_File = NULL;

	if(!strCategory.IsEmpty())
		SetSection(strCategory);
}

CIni::~CIni()
{
	SaveAndClose();

	delete m_File;
}

void CIni::OpenFile(const CString& strFileName)
{
	m_Write = false;
	if(m_File && m_File->m_hFile == CFile::hFileNull)
		m_File->Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite | CFile::shareDenyWrite);
}

void CIni::CloseFile()
{
	if(m_File && m_File->m_hFile!=CFile::hFileNull)
		m_File->Close();
}

void CIni::ReadData()
{
	if(m_File && m_File->m_hFile!=CFile::hFileNull)
	{
		m_strFileBuffer.Empty();
		CString strLine;
		while (m_File->ReadString(strLine))
		{
			if(strLine.IsEmpty())
				continue;

			m_strFileBuffer.Append(strLine);
			m_strFileBuffer.Append(_T("\r\n"));
		}
	}
}

void CIni::SaveAndClose()
{
	if(m_Write)
		if(m_File && m_File->m_hFile!=CFile::hFileNull)
		{
			m_File->SetLength(0);
			CString strBuffer = m_strFileBuffer;
			strBuffer.Replace(_T("\r\n"),_T("\n"));
			strBuffer.Append(_T("\n"));
			m_File->WriteString(strBuffer);
		}

	CloseFile();
}

/***********************************************
* Section/Category Operations
*/

void CIni::SetSection(const CString& strCategoryName)
{
	AddCategory(strCategoryName);

	m_strDefCategory = strCategoryName;
}

void CIni::AddCategory(const CString& strCategoryName)
{
	if(!CategoryExist(strCategoryName))
	{
		if(m_strFileBuffer.GetLength()!=0)
			m_strFileBuffer += _T("\r\n");

		m_strFileBuffer += _T('[');
		m_strFileBuffer += strCategoryName;
		m_strFileBuffer += _T(']');
	}
}

void CIni::DeleteCategory(const CString& strCategoryName)
{
	if(CategoryExist(strCategoryName))
	{
		CString catSearchString = _T("[");
		catSearchString += strCategoryName;
		catSearchString += _T(']');

		int nCatFirstCharPos = m_strFileBuffer.Find(catSearchString);

		// Check if the category is valid
		if(nCatFirstCharPos!=0)
		{
			catSearchString = _T("\r\n[");
			catSearchString += strCategoryName;
			catSearchString += _T(']');

			nCatFirstCharPos -= 2;
			nCatFirstCharPos = m_strFileBuffer.Find(catSearchString,nCatFirstCharPos);
			nCatFirstCharPos += 2;
		}

		int nCatLastCharPos = m_strFileBuffer.Find(_T("\r\n["), (nCatFirstCharPos + strCategoryName.GetLength() + 1));

		if (nCatLastCharPos < 0)
			nCatLastCharPos = m_strFileBuffer.GetLength();

		m_strFileBuffer.Delete(nCatFirstCharPos, (nCatLastCharPos - nCatFirstCharPos));
	}
}

bool CIni::CategoryExist(const CString& strCategoryName)
{
	CString catSearchString = _T("[");
	catSearchString += strCategoryName;
	catSearchString += _T(']');

	int nFindPos = m_strFileBuffer.Find(catSearchString);

	if (nFindPos < 0)
		return false;

	// Check if the category is valid
	if (nFindPos != 0)
	{
		catSearchString = _T("\r\n[");
		catSearchString += strCategoryName;
		catSearchString += _T(']');

		nFindPos -= 2;
		nFindPos = m_strFileBuffer.Find(catSearchString, nFindPos);

		if (nFindPos < 0)
			return false;
	}

	return true;
}

/***********************************************
* Entry/Value Operations
*/


void CIni::DeleteEntry(const CString& strCategoryName, const CString &strEntryName)
{
	if(EntryExist(strCategoryName,strEntryName))
	{
		CString catSearchString = _T("[");
		catSearchString += strCategoryName;
		catSearchString += _T(']');

		int nEntryFirstCharPos;
		int nEntryLastCharPos;
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

bool CIni::EntryExist(const CString &strCategoryName, const TCHAR *pcEntryName)
{
	CString catSearchString = _T("[");
	catSearchString += strCategoryName;
	catSearchString += _T(']');

	int nEntryFirstCharPos;
	int nCatFirstCharPos;
	int nCatLastCharPos;

	if ((nCatFirstCharPos = m_strFileBuffer.Find(catSearchString)) < 0)
		// Category does not exist
		return false;

	// Check if the category is valid
	if (nCatFirstCharPos != 0)
	{
		catSearchString = _T("\r\n[");
		catSearchString += strCategoryName;
		catSearchString += _T(']');

		nCatFirstCharPos -= 2;
		nCatFirstCharPos = m_strFileBuffer.Find(catSearchString, nCatFirstCharPos);

		if (nCatFirstCharPos < 0)
			return false;

		nCatFirstCharPos += 2;
	}

	nCatLastCharPos = m_strFileBuffer.Find(_T("\r\n["), (nCatFirstCharPos + strCategoryName.GetLength() + 1));

	if (nCatLastCharPos < 0)
		// No following category exists - end of category is end of file
		nCatLastCharPos = m_strFileBuffer.GetLength();
	else
		nCatLastCharPos += 2;

	CString entrySearchString = _T("\r\n");
	entrySearchString += pcEntryName;
	entrySearchString += _T('=');

	nEntryFirstCharPos = m_strFileBuffer.Find(entrySearchString, (nCatFirstCharPos + strCategoryName.GetLength() + 1));
	
	if (nEntryFirstCharPos >= 0)
		nEntryFirstCharPos += 2;
	else
		return false;

	if (nEntryFirstCharPos < nCatFirstCharPos || nEntryFirstCharPos > nCatLastCharPos)
		// The entry we're searching for does not exist in the given category
		return false;

	return true;
}

void CIni::WriteValue(const TCHAR *pcEntryName, const CString &strValue)
{
	m_Write = true;

	CString	strEntryName = pcEntryName;

	if (EntryExist(m_strDefCategory, pcEntryName))
	{
		CString catSearchString = _T("[");
		catSearchString += m_strDefCategory;
		catSearchString += _T(']');

		int nValueFirstCharPos;
		int nEntryFirstCharPos;
		int nEntryLastCharPos;
		int nCatFirstCharPos = m_strFileBuffer.Find(catSearchString);

		// Check if the category is valid
		if (nCatFirstCharPos != 0)
		{
			catSearchString = _T("\r\n[");
			catSearchString += m_strDefCategory;
			catSearchString += _T(']');

			nCatFirstCharPos -= 2;
			nCatFirstCharPos = m_strFileBuffer.Find(catSearchString,nCatFirstCharPos);
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
	else
	{
		if(CategoryExist(m_strDefCategory))
		{
			CString catSearchString = _T("[");
			catSearchString += m_strDefCategory;
			catSearchString += _T(']');

			int nCatLastCharPos, nCatFirstCharPos = m_strFileBuffer.Find(catSearchString);

			// Check if the category is valid
			if (nCatFirstCharPos != 0)
			{
				catSearchString = _T("\r\n[");
				catSearchString += m_strDefCategory;
				catSearchString += _T(']');

				nCatFirstCharPos -= 2;
				nCatFirstCharPos = m_strFileBuffer.Find(catSearchString,nCatFirstCharPos);
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
}


/***********************************************
* Value Operations
*/

CString CIni::GetValue(const TCHAR *pcEntryName, bool* ptrSuccess)
{
	CString	strEntryName = pcEntryName;

	if (ValueExist(m_strDefCategory, strEntryName))
	{
		CString catSearchString = _T("[");
		catSearchString += m_strDefCategory;
		catSearchString += _T(']');

		int nValueFirstCharPos;
		int nEntryFirstCharPos;
		int nEntryLastCharPos;
		int nCatFirstCharPos = m_strFileBuffer.Find(catSearchString);

		// Check if the category is valid
		if (nCatFirstCharPos != 0)
		{
			catSearchString = _T("\r\n[");
			catSearchString += m_strDefCategory;
			catSearchString += _T(']');

			nCatFirstCharPos -= 2;
			nCatFirstCharPos = m_strFileBuffer.Find(catSearchString,nCatFirstCharPos);
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

		if(ptrSuccess)
			*ptrSuccess = true;

		return m_strFileBuffer.Mid(nValueFirstCharPos, (nEntryLastCharPos - nValueFirstCharPos));
	}
	else
		return _T("");
}

bool CIni::ValueExist(const CString& strCategoryName, const CString& strEntryName)
{
	CString catSearchString = _T("[");
	catSearchString += strCategoryName;
	catSearchString += _T(']');

	int nEntryFirstCharPos;
	int nCatFirstCharPos;
	int nCatLastCharPos;

	if ((nCatFirstCharPos = m_strFileBuffer.Find(catSearchString)) < 0)
		return false;	// Category does not exist

	// Check if the category is valid
	if (nCatFirstCharPos != 0)
	{
		catSearchString = _T("\r\n[");
		catSearchString += strCategoryName;
		catSearchString += _T(']');

		nCatFirstCharPos -= 2;
		nCatFirstCharPos = m_strFileBuffer.Find(catSearchString, nCatFirstCharPos);

		if (nCatFirstCharPos < 0)
			return false;

		nCatFirstCharPos += 2;
	}

	nCatLastCharPos = m_strFileBuffer.Find(_T("\r\n["), (nCatFirstCharPos + strCategoryName.GetLength() + 1));

	if (nCatLastCharPos < 0)
		// No following category exists - end of category is end of file
		nCatLastCharPos = m_strFileBuffer.GetLength();
	else
		nCatLastCharPos += 2;

	CString entrySearchString = _T("\r\n");
	entrySearchString += strEntryName;
	entrySearchString += _T('=');

	nEntryFirstCharPos = m_strFileBuffer.Find(entrySearchString, (nCatFirstCharPos + strCategoryName.GetLength() + 1));

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

/************************************************************************************************
* All the rest
*/

CString CIni::GetString(LPCTSTR strEntry, LPCTSTR strDefault/*=NULL*/, LPCTSTR strSection/* = NULL*/)
{
	if(strSection)
		SetSection(strSection);

	bool bSuccess = false;
	CString strTemp = GetValue(strEntry,&bSuccess);

	if (!bSuccess){
		CString strDef = _T("");
		if(strDefault)
			strDef = strDefault;
		WriteValue(strEntry,strDef);
		return strDef;
	}else
		return strTemp;
}
CString CIni::GetStringLong(LPCTSTR strEntry, LPCTSTR strDefault/*=NULL*/, LPCTSTR strSection/* = NULL*/)
{
	return GetString(strEntry, strDefault, strSection);
}

CString CIni::GetStringUTF8(LPCTSTR strEntry, LPCTSTR strDefault/*=NULL*/, LPCTSTR strSection/* = NULL*/)
{
	return GetString(strEntry, strDefault, strSection);
}

double CIni::GetDouble(LPCTSTR strEntry, double fDefault/* = 0.0*/, LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, _countof(strDefault), _T("%g"), fDefault);
	strDefault[_countof(strDefault) - 1] = _T('\0');
	CString m_chBuffer = GetString(strEntry,strDefault,strSection);
	return _tstof(m_chBuffer);
}

float CIni::GetFloat(LPCTSTR strEntry,float fDefault/* = 0.0*/, LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, _countof(strDefault), _T("%g"), fDefault);
	strDefault[_countof(strDefault) - 1] = _T('\0');
	CString m_chBuffer = GetString(strEntry,strDefault,strSection);
	return (float)_tstof(m_chBuffer);
}

int CIni::GetInt(LPCTSTR strEntry,int nDefault/* = 0*/,LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, _countof(strDefault), _T("%d"), nDefault);
	strDefault[_countof(strDefault) - 1] = _T('\0');
	CString m_chBuffer = GetString(strEntry,strDefault,strSection);
	return _tstoi(m_chBuffer);
}

ULONGLONG CIni::GetUInt64(LPCTSTR strEntry,ULONGLONG nDefault/* = 0*/,LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, _countof(strDefault), _T("%I64u"), nDefault);
	strDefault[_countof(strDefault) - 1] = _T('\0');
	CString m_chBuffer = GetString(strEntry,strDefault,strSection);
	ULONGLONG nResult;
	if (_stscanf(m_chBuffer, _T("%I64u"), &nResult) != 1)
		return nDefault;
	return nResult;
}

WORD CIni::GetWORD(LPCTSTR strEntry,WORD nDefault/* = 0*/,LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, _countof(strDefault), _T("%u"), nDefault);
	strDefault[_countof(strDefault) - 1] = _T('\0');
	CString m_chBuffer = GetString(strEntry,strDefault,strSection);
	return (WORD)_tstoi(m_chBuffer);
}

bool CIni::GetBool(LPCTSTR strEntry,bool bDefault/* = false*/,LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, _countof(strDefault), _T("%d"), bDefault);
	strDefault[_countof(strDefault) - 1] = _T('\0');
	CString m_chBuffer = GetString(strEntry,strDefault,strSection);
	return ( _tstoi(m_chBuffer) != 0 );
}

CPoint CIni::GetPoint(LPCTSTR strEntry,	CPoint ptDefault, LPCTSTR strSection)
{
	CPoint ptReturn=ptDefault;

	CString strDefault;
	strDefault.Format(_T("(%d,%d)"),ptDefault.x, ptDefault.y);

	CString strPoint = GetString(strEntry,strDefault, strSection);
	if (_stscanf(strPoint,_T("(%d,%d)"), &ptReturn.x, &ptReturn.y) != 2)
		return ptDefault;

	return ptReturn;
}

CRect CIni::GetRect(LPCTSTR strEntry, CRect rectDefault, LPCTSTR strSection)
{
	CRect rectReturn=rectDefault;

	CString strDefault;
	//old version :strDefault.Format("(%d,%d,%d,%d)",rectDefault.top,rectDefault.left,rectDefault.bottom,rectDefault.right);
	strDefault.Format(_T("%d,%d,%d,%d"),rectDefault.left,rectDefault.top,rectDefault.right,rectDefault.bottom);

	CString strRect = GetString(strEntry,strDefault,strSection);

	//new Version found
	if( 4==_stscanf(strRect,_T("%d,%d,%d,%d"),&rectDefault.left,&rectDefault.top,&rectDefault.right,&rectDefault.bottom))
		return rectReturn;
	//old Version found
	if (_stscanf(strRect,_T("(%d,%d,%d,%d)"), &rectReturn.top,&rectReturn.left,&rectReturn.bottom,&rectReturn.right) != 4)
		return rectDefault;
	return rectReturn;
}

COLORREF CIni::GetColRef(LPCTSTR strEntry, COLORREF crDefault, LPCTSTR strSection)
{
	int temp[3]={	GetRValue(crDefault),
					GetGValue(crDefault),
					GetBValue(crDefault)};

	CString strDefault;
	strDefault.Format(_T("RGB(%hd,%hd,%hd)"),temp[0],temp[1],temp[2]);

	CString strColRef = GetString(strEntry,strDefault,strSection);
	if (_stscanf(strColRef,_T("RGB(%d,%d,%d)"), temp, temp+1, temp+2) != 3)
		return crDefault;

	return RGB(temp[0],temp[1],temp[2]);
}
	
void CIni::WriteString(LPCTSTR strEntry, LPCTSTR str, LPCTSTR strSection/* = NULL*/)
{
	if(strSection)
		SetSection(strSection);
	WriteValue(strEntry,str);
}

void CIni::WriteStringUTF8(LPCTSTR strEntry, LPCTSTR psz, LPCTSTR strSection/* = NULL*/)
{
	WriteString(strEntry, psz, strSection);
}

void CIni::WriteDouble(LPCTSTR strEntry,double f, LPCTSTR strSection/*= NULL*/)
{
	TCHAR strBuffer[MAX_PATH];
	_sntprintf(strBuffer, _countof(strBuffer), _T("%g"), f);
	strBuffer[_countof(strBuffer) - 1] = _T('\0');
	WriteString(strEntry,strBuffer,strSection);
}

void CIni::WriteFloat(LPCTSTR strEntry,float f, LPCTSTR strSection/* = NULL*/)
{
	TCHAR strBuffer[MAX_PATH];
	_sntprintf(strBuffer, _countof(strBuffer), _T("%g"), f);
	strBuffer[_countof(strBuffer) - 1] = _T('\0');
	WriteString(strEntry,strBuffer,strSection);
}

void CIni::WriteInt(LPCTSTR strEntry,int n, LPCTSTR strSection/* = NULL*/)
{
	TCHAR strBuffer[MAX_PATH];
	_itot(n, strBuffer, 10);
	WriteString(strEntry,strBuffer,strSection);
}

void CIni::WriteUInt64(LPCTSTR strEntry,ULONGLONG n, LPCTSTR strSection/* = NULL*/)
{
	TCHAR strBuffer[MAX_PATH];
	_ui64tot(n, strBuffer, 10);
	WriteString(strEntry,strBuffer,strSection);
}

void CIni::WriteWORD(LPCTSTR strEntry,WORD n, LPCTSTR strSection/* = NULL*/)
{
	TCHAR strBuffer[MAX_PATH];
	_ultot(n, strBuffer, 10);
	WriteString(strEntry,strBuffer,strSection);
}

void CIni::WriteBool(LPCTSTR strEntry,bool b, LPCTSTR strSection/* = NULL*/)
{
	TCHAR strBuffer[MAX_PATH];
	_sntprintf(strBuffer, _countof(strBuffer), _T("%d"), (int)b);
	strBuffer[_countof(strBuffer) - 1] = _T('\0');
	WriteString(strEntry,strBuffer,strSection);
}

void CIni::WritePoint(LPCTSTR strEntry,CPoint pt, LPCTSTR strSection)
{
	CString strBuffer;
	strBuffer.Format(_T("(%d,%d)"),pt.x,pt.y);
	WriteString(strEntry,strBuffer,strSection);
}

void CIni::WriteRect(LPCTSTR strEntry,CRect rect, LPCTSTR strSection)
{
	CString strBuffer;
	strBuffer.Format(_T("(%d,%d,%d,%d)"),rect.top,rect.left,rect.bottom,rect.right);
	WriteString(strEntry,strBuffer,strSection);
}

void CIni::WriteColRef(LPCTSTR strEntry,COLORREF cr, LPCTSTR strSection)
{
	CString strBuffer;
	strBuffer.Format(_T("RGB(%d,%d,%d)"),GetRValue(cr), GetGValue(cr), GetBValue(cr));
	WriteString(strEntry,strBuffer,strSection);
}

void CIni::SerGetString(	bool bGet,CString &	str,LPCTSTR strEntry,LPCTSTR strSection,LPCTSTR strDefault)
{
	if(bGet)
		str = GetString(strEntry,strDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteString(strEntry,str, strSection/* = NULL*/);
}
void CIni::SerGetDouble(	bool bGet,double&	f,	LPCTSTR strEntry,LPCTSTR strSection/* = NULL*/,double fDefault/* = 0.0*/)
{
	if(bGet)
		f = GetDouble(strEntry,fDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteDouble(strEntry,f, strSection/* = NULL*/);
}
void CIni::SerGetFloat(		bool bGet,float	&	f,	LPCTSTR strEntry, LPCTSTR strSection/* = NULL*/,float fDefault/* = 0.0*/)
{
	if(bGet)
		f = GetFloat(strEntry,fDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteFloat(strEntry,f, strSection/* = NULL*/);
}
void CIni::SerGetInt(		bool bGet,int	&	n,	LPCTSTR strEntry,LPCTSTR strSection/* = NULL*/,int nDefault/* = 0*/)
{
	if(bGet)
		n = GetInt(strEntry,nDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteInt(strEntry,n, strSection/* = NULL*/);
}
void CIni::SerGetDWORD(		bool bGet,DWORD	&	n,	LPCTSTR strEntry,LPCTSTR strSection/* = NULL*/,DWORD nDefault/* = 0*/)
{
	if(bGet)
		n = (DWORD)GetInt(strEntry,nDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteInt(strEntry,n, strSection/* = NULL*/);
}
void CIni::SerGetBool(		bool bGet,bool	&	b,	LPCTSTR strEntry,LPCTSTR strSection/* = NULL*/,bool bDefault/* = false*/)
{
	if(bGet)
		b = GetBool(strEntry,bDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteBool(strEntry,b, strSection/* = NULL*/);
}

void CIni::SerGetPoint(	bool bGet,CPoint	& pt,	LPCTSTR strEntry,	LPCTSTR strSection,	CPoint ptDefault)
{
	if(bGet)
		pt = GetPoint(strEntry,ptDefault,strSection);
	else
		WritePoint(strEntry,pt, strSection);
}
void CIni::SerGetRect(		bool bGet,CRect		& rect,	LPCTSTR strEntry,	LPCTSTR strSection,	CRect rectDefault)
{
	if(bGet)
		rect = GetRect(strEntry,rectDefault,strSection);
	else
		WriteRect(strEntry,rect, strSection);
}
void CIni::SerGetColRef(	bool bGet,COLORREF	& cr,	LPCTSTR strEntry,	LPCTSTR strSection,	COLORREF crDefault)
{
	if(bGet)
		cr = GetColRef(strEntry,crDefault,strSection);
	else
		WriteColRef(strEntry,cr, strSection);
}
// Überladene Methoden //////////////////////////////////////////////////////////////////////////////////////////////////77
// Einfache Typen /////////////////////////////////////////////////////////////////////////////////////////////////////////
void		CIni::SerGet(	bool bGet,CString	& str,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	LPCTSTR strDefault/*= NULL*/)
{
   SerGetString(bGet,str,strEntry,strSection,strDefault);
}
void		CIni::SerGet(	bool bGet,double	& f,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	double fDefault/* = 0.0*/)
{
   SerGetDouble(bGet,f,strEntry,strSection,fDefault);
}
void		CIni::SerGet(	bool bGet,float		& f,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	float fDefault/* = 0.0*/)
{
   SerGetFloat(bGet,f,strEntry,strSection,fDefault);
}
void		CIni::SerGet(	bool bGet,int		& n,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	int nDefault/* = 0*/)
{
   SerGetInt(bGet,n,strEntry,strSection,nDefault);
}
void		CIni::SerGet(	bool bGet,short		& n,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	int nDefault/* = 0*/)
{
   int nTemp = n;
   SerGetInt(bGet,nTemp,strEntry,strSection,nDefault);
   n = (short)nTemp;
}
void		CIni::SerGet(	bool bGet,DWORD		& n,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	DWORD nDefault/* = 0*/)
{
   SerGetDWORD(bGet,n,strEntry,strSection,nDefault);
}
void		CIni::SerGet(	bool bGet,WORD		& n,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	DWORD nDefault/* = 0*/)
{
   DWORD dwTemp = n;
   SerGetDWORD(bGet,dwTemp,strEntry,strSection,nDefault);
   n = (WORD)dwTemp;
}
void		CIni::SerGet(	bool bGet,CPoint	& pt,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	CPoint ptDefault/* = CPoint(0,0)*/)
{
   SerGetPoint(bGet,pt,strEntry,strSection,ptDefault);
}
void		CIni::SerGet(	bool bGet,CRect		& rect,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	CRect rectDefault/* = CRect(0,0,0,0)*/)
{
   SerGetRect(bGet,rect,strEntry,strSection,rectDefault);
}

void CIni::SerGet(bool bGet, CString *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, LPCTSTR Default/*=NULL*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, ar[i]);
				if(ar[i].GetLength() == 0)
					ar[i] = Default;
			}

		} else {
			strBuffer = ar[0];
			for(int i = 1; i < nCount; i++) {
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(ar[i]);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}

void CIni::SerGet(bool bGet, double *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, double Default/* = 0.0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = _tstof(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%g"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%g"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, float *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, float Default/* = 0.0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = (float)_tstof(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%g"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%g"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, int *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, int Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = _tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, unsigned char *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, unsigned char Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = (unsigned char)_tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, short *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, int Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = (short)Default;
				else
					ar[i] = (short)_tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, DWORD *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, DWORD Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = (DWORD)_tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, WORD *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, DWORD Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = (WORD)Default;
				else
					ar[i] = (WORD)_tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void		CIni::SerGet(	bool bGet,CPoint	* ar,	   int nCount, LPCTSTR strEntry,	LPCTSTR strSection/*=NULL*/,	CPoint Default/* = CPoint(0,0)*/)
{
   CString strBuffer;
   for( int i=0 ; i<nCount ; i++)
   {
      strBuffer.Format(_T("_%i"),i);
      strBuffer = strEntry + strBuffer;
      SerGet(bGet,ar[i],strBuffer,strSection,Default);
   }
}
void		CIni::SerGet(	bool bGet,CRect	* ar,	   int nCount, LPCTSTR strEntry,	LPCTSTR strSection/*=NULL*/,	CRect Default/* = CRect(0,0,0,0)*/)
{
   CString strBuffer;
   for( int i=0 ; i<nCount ; i++)
   {
      strBuffer.Format(_T("_%i"),i);
      strBuffer = strEntry + strBuffer;
      SerGet(bGet,ar[i],strBuffer,strSection,Default);
   }
}

int			CIni::Parse(const CString& strIn, int nOffset, CString& strOut) {

	strOut.Empty();
	int nLength = strIn.GetLength();

	if(nOffset < nLength) {
		if(nOffset != 0 && strIn[nOffset] == _T(','))
			nOffset++;

		while(nOffset < nLength) {
			if(!_istspace((_TUCHAR)strIn[nOffset]))
				break;

			nOffset++;
		}

		while(nOffset < nLength) {
			strOut += strIn[nOffset];

			if(strIn[++nOffset] == _T(','))
				break;
		}

		strOut.Trim();
	}
	return nOffset;
}

bool CIni::GetBinary(LPCTSTR lpszEntry, BYTE** ppData, UINT* pBytes, LPCTSTR pszSection)
{
	*ppData = NULL;
	*pBytes = 0;

	CString str = GetString(lpszEntry, NULL, pszSection);
	if (str.IsEmpty())
		return false;
	ASSERT(str.GetLength()%2 == 0);
	INT_PTR nLen = str.GetLength();
	*pBytes = UINT(nLen)/2;
	*ppData = new BYTE[*pBytes];
	for (int i=0;i<nLen;i+=2)
	{
		(*ppData)[i/2] = (BYTE)(((str[i+1] - 'A') << 4) + (str[i] - 'A'));
	}
	return true;
}

bool CIni::WriteBinary(LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes, LPCTSTR pszSection)
{
	// convert to string and write out
	LPTSTR lpsz = new TCHAR[nBytes*2+1];
	UINT i;
	for (i = 0; i < nBytes; i++)
	{
		lpsz[i*2] = (TCHAR)((pData[i] & 0x0F) + 'A'); //low nibble
		lpsz[i*2+1] = (TCHAR)(((pData[i] >> 4) & 0x0F) + 'A'); //high nibble
	}
	lpsz[i*2] = 0;


	WriteString(lpszEntry, lpsz, pszSection);
	delete[] lpsz;
	return true;
}