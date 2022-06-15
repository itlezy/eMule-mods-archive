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

/***************************************************************************************
* NOTE: Some of the functions in this class are tooken from the ini2 of eMule Plus, 
*		the rest are functions from the officiel emule ini2 with small modifications.
*
*/

class CStdioFile;

#pragma once
#include "types.h"

class CIni
{
public:
	// Construct And File Operations
	CIni(const CString& strFileName = _T(""), const CString& strCategory = _T(""));
	~CIni();

	void		CloseWithoutSave(){CloseFile();}
	void		SaveAndClose();

	// this ini can also be operated without a file over the buffer
	void SetBuffer(const CString& Buffer)		{m_strFileBuffer = Buffer;}
	const CString &GetBuffer()					{return m_strFileBuffer;}

	// Section/Category Operations
	void		SetSection(const CString& strSection);
	void		AddCategory(const CString& strCategoryName);
	void		DeleteCategory(const CString& strCategoryName);
	bool		CategoryExist(const CString& strCategoryName);

	// Entry/Value Operations
	void		DeleteKey(const CString& strEntryName)				{DeleteEntry(m_strDefCategory, strEntryName);}
	void		DeleteEntry(const CString& strCategoryName, const CString& strEntryName);
	bool		EntryExist(const CString &strCategoryName, const TCHAR *pcEntryName);
	bool		ValueExist(const CString& strCategoryName, const CString& strEntryName);

	// All the rest
	CString		GetString(LPCTSTR lpszEntry,	LPCTSTR		lpszDefault = NULL,				LPCTSTR lpszSection = NULL);
	CString		GetStringUTF8(LPCTSTR lpszEntry,LPCTSTR		lpszDefault = NULL,				LPCTSTR lpszSection = NULL);
	CString		GetStringLong(LPCTSTR lpszEntry,LPCTSTR		lpszDefault = NULL,				LPCTSTR lpszSection = NULL);
	double		GetDouble(LPCTSTR lpszEntry,	double		fDefault = 0.0,					LPCTSTR lpszSection = NULL);
	float		GetFloat(LPCTSTR lpszEntry,		float		fDefault = 0.0F,				LPCTSTR lpszSection = NULL);
	int			GetInt(LPCTSTR lpszEntry,		int			nDefault = 0,					LPCTSTR lpszSection = NULL);
	ULONGLONG	GetUInt64(LPCTSTR lpszEntry,	ULONGLONG	nDefault = 0,					LPCTSTR lpszSection = NULL);
	WORD		GetWORD(LPCTSTR lpszEntry,		WORD		nDefault = 0,					LPCTSTR lpszSection = NULL);
	bool		GetBool(LPCTSTR lpszEntry,		bool		bDefault = false,				LPCTSTR lpszSection = NULL);
	CPoint		GetPoint(LPCTSTR lpszEntry,		CPoint		ptDefault = CPoint(0,0),		LPCTSTR lpszSection = NULL);
	CRect		GetRect(LPCTSTR lpszEntry,		CRect		rectDefault = CRect(0,0,0,0),	LPCTSTR lpszSection = NULL);
	COLORREF	GetColRef(LPCTSTR lpszEntry,	COLORREF	crDefault = RGB(128,128,128),	LPCTSTR lpszSection = NULL);
	bool		GetBinary(LPCTSTR lpszEntry,	BYTE** ppData, UINT* pBytes,				LPCTSTR lpszSection = NULL);

	void		WriteString(LPCTSTR strEntry,	LPCTSTR		s,								LPCTSTR lpszSection = NULL);
	void		WriteStringUTF8(LPCTSTR strEntry,LPCTSTR    s,								LPCTSTR lpszSection = NULL);
	void		WriteDouble(LPCTSTR lpszEntry,	double		f,								LPCTSTR lpszSection = NULL);
	void		WriteFloat(LPCTSTR lpszEntry,	float		f,								LPCTSTR lpszSection = NULL);
	void		WriteInt(LPCTSTR lpszEntry,		int			n,								LPCTSTR lpszSection = NULL);
	void		WriteUInt64(LPCTSTR lpszEntry,	ULONGLONG	n,								LPCTSTR lpszSection = NULL);
	void		WriteWORD(LPCTSTR lpszEntry,	WORD		n,								LPCTSTR lpszSection = NULL);
	void		WriteBool(LPCTSTR lpszEntry,	bool		b,								LPCTSTR lpszSection = NULL);
	void		WritePoint(LPCTSTR lpszEntry,	CPoint		pt,								LPCTSTR lpszSection = NULL);
	void		WriteRect(LPCTSTR lpszEntry,	CRect		rect,							LPCTSTR lpszSection = NULL);
	void		WriteColRef(LPCTSTR lpszEntry,	COLORREF	cr,								LPCTSTR lpszSection = NULL);
	bool		WriteBinary(LPCTSTR lpszEntry,	LPBYTE pData, UINT nBytes,					LPCTSTR lpszSection = NULL);

	void		SerGetString(	bool bGet, CString&		s,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	LPCTSTR strDefault = NULL);
	void		SerGetDouble(	bool bGet, double&		f,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	double fDefault = 0.0);
	void		SerGetFloat(	bool bGet, float&		f,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	float fDefault = 0.0);
	void		SerGetInt(		bool bGet, int&			n,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	int nDefault = 0);
	void		SerGetDWORD(	bool bGet, DWORD&		n,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	DWORD nDefault = 0);
	void		SerGetBool(		bool bGet, bool&		b,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	bool bDefault = false);
	void		SerGetPoint(	bool bGet, CPoint&		pt,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CPoint ptDefault = CPoint(0,0));
	void		SerGetRect(		bool bGet, CRect&		rc,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CRect rectDefault = CRect(0,0,0,0));
	void		SerGetColRef(	bool bGet, COLORREF&	cr,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	COLORREF crDefault = RGB(128,128,128));

	void		SerGet(	bool bGet, CString&	 s,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	LPCTSTR lpszDefault = NULL);
	void		SerGet(	bool bGet, double&	 f,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	double fDefault = 0.0);
	void		SerGet(	bool bGet, float&	 f,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	float fDefault = 0.0F);
	void		SerGet(	bool bGet, int&		 n,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	int nDefault = 0);
	void		SerGet(	bool bGet, short&	 n,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	int nDefault = 0);
	void		SerGet(	bool bGet, DWORD&	 n,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	DWORD nDefault = 0);
	void		SerGet(	bool bGet, WORD&	 n,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	DWORD nDefault = 0);
//	void		SerGet(	bool bGet, bool&	 b,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	bool bDefault = false);
	void		SerGet(	bool bGet, CPoint&	 pt, LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CPoint ptDefault = CPoint(0,0));
	void		SerGet(	bool bGet, CRect&	 rc, LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CRect rectDefault = CRect(0,0,0,0));
//	void		SerGet(	bool bGet, COLORREF& cr, LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	COLORREF crDefault = RGB(128,128,128));
   
//ARRAYs
	void		SerGet(	bool bGet, CString*	s,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, LPCTSTR lpszDefault = NULL);
	void		SerGet(	bool bGet, double*	f,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, double fDefault = 0.0);
	void		SerGet(	bool bGet, float*	f,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, float fDefault = 0.0F);
	void		SerGet(	bool bGet, BYTE*	n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, BYTE nDefault = 0);
	void		SerGet(	bool bGet, int*		n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, int nDefault = 0);
	void		SerGet(	bool bGet, short*	n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, int nDefault = 0);
	void		SerGet(	bool bGet, DWORD*	n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, DWORD nDefault = 0);
	void		SerGet(	bool bGet, WORD*	n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, DWORD nDefault = 0);
	void		SerGet(	bool bGet, CPoint*	pt,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, CPoint ptDefault = CPoint(0,0));
	void		SerGet(	bool bGet, CRect*	rc,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, CRect rectDefault = CRect(0,0,0,0));

	int			Parse(const CString&, int nOffset, CString &strOut);

private:
	// File Operations
	void		OpenFile(const CString& strFileName);
	void		CloseFile();
	void		ReadData();

	// Value Operations
	void		WriteValue(const TCHAR *pcEntryName, const CString &strValue);
	CString		GetValue(const TCHAR *pcEntryName, bool* ptrSuccess = NULL);

	// Private Variables
	CString		m_strDefCategory;
	CString		m_strFileBuffer;
	CStdioFile*	m_File;
	bool		m_Write;
};
