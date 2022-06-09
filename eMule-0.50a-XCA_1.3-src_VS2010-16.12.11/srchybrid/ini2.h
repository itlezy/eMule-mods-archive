// X-Ray :: eMulePlusIniClass :: Start
// Ini.h: Schnittstelle für die Klasse CIni.

// Autor: Michael Schikora
// Mail:  schiko@schikos.de
//
// If you found this code useful,
// please let me know
//
// How to use:
//
class CStdioFile;

#pragma once

class CIni
{
public:
	// Construct And File Operations
	CIni(const CString& rstrFileName, const CString& rstrSection);
	~CIni();

	void		SetNotModified(){ m_bWrite = false;}
	void		Save();

	// this ini can also be operated without a file over the buffer
	void SetBuffer(const CString& Buffer)		{m_strFileBuffer = Buffer;}
	const CString &GetBuffer() const			{return m_strFileBuffer;}

	// Section Operations
	void		SetSection(const CString& rstrSection){m_strSection = rstrSection;}
	int			AddSection(const CString& strSectionName);
	void		DeleteSection(const CString& strSectionName);
	int			FindSection(const CString& strSectionName) const;

	// Entry/Value Operations
	void		DeleteKey(const CString& strEntryName)				{DeleteEntry(m_strSection, strEntryName);}
	void		DeleteEntry(const CString& strSectionName, const CString& strEntryName);
	int			FindEntry(const CString &strSectionName, const TCHAR *pcEntryName) const;

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
	bool		GetBinary(LPCTSTR lpszEntry,	BYTE** ppData, size_t* pBytes,				LPCTSTR lpszSection = NULL);

	void		WriteString(LPCTSTR lpszEntry,	LPCTSTR		s,								LPCTSTR lpszSection = NULL);
	void		WriteStringUTF8(LPCTSTR lpszEntry,LPCTSTR   s,								LPCTSTR lpszSection = NULL);
	void		WriteDouble(LPCTSTR lpszEntry,	double		f,								LPCTSTR lpszSection = NULL);
	void		WriteFloat(LPCTSTR lpszEntry,	float		f,								LPCTSTR lpszSection = NULL);
	void		WriteInt(LPCTSTR lpszEntry,		int			n,								LPCTSTR lpszSection = NULL);
	void		WriteUInt64(LPCTSTR lpszEntry,	ULONGLONG	n,								LPCTSTR lpszSection = NULL);
	void		WriteWORD(LPCTSTR lpszEntry,	WORD		n,								LPCTSTR lpszSection = NULL);
	void		WriteBool(LPCTSTR lpszEntry,	bool		b,								LPCTSTR lpszSection = NULL);
	void		WritePoint(LPCTSTR lpszEntry,	CPoint		pt,								LPCTSTR lpszSection = NULL);
	void		WriteRect(LPCTSTR lpszEntry,	CRect		rect,							LPCTSTR lpszSection = NULL);
	void		WriteColRef(LPCTSTR lpszEntry,	COLORREF	cr,								LPCTSTR lpszSection = NULL);
	void		WriteBinary(LPCTSTR lpszEntry,	LPBYTE pData, size_t nBytes,				LPCTSTR lpszSection = NULL);

	void		SerGetString(	bool bGet, CString&		s,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	LPCTSTR lpszDefault = NULL);
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
	void		SerGet(	bool bGet, CPoint&	 pt, LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CPoint ptDefault = CPoint(0,0));
	void		SerGet(	bool bGet, CRect&	 rc, LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CRect rectDefault = CRect(0,0,0,0));
   
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
	void		OpenFile();
	void		CloseFile();
	void		ReadData();

	// Value Operations
	void		WriteValue(const TCHAR *pcEntryName, const CString &strValue);
	CString		GetValue(const TCHAR *pcEntryName, bool* ptrSuccess = NULL) const;

	// Private Variables
	CString		strFileName;
	CString		m_strSection;
	CString		m_strFileBuffer;
	CStdioFile*	m_File;
	bool		m_bWrite;
};
// X-Ray :: eMulePlusIniClass :: End
