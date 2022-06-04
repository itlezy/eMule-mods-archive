// X-Ray :: eMulePlusIniClass :: Start
#include "stdafx.h"
#include <io.h>
#include <share.h>
#include "Ini2.h"
#include "StringConversion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***********************************************
* Construct And File Operations
*/

CIni::CIni(const CString& rstrFileName, const CString& rstrSection) : strFileName(rstrFileName), m_bWrite(false)
{
	if(!rstrFileName.IsEmpty())
{
		if(!_taccess(rstrFileName, 0))
	{
		m_strFileBuffer.Empty();
			FILE* pFile = _tfsopen(rstrFileName, _T("rS"), _SH_DENYWR);
			DWORD wBOM = 0;
			if (fread(&wBOM, sizeof(wBOM), 1, pFile) == 1)
			{
				fseek(pFile, (wBOM & 0xFFFFFF) == 0xBFBBEF ? 3 : 0, SEEK_SET);
				char acRaw[1024];
		CString strLine;
				while (fgets(acRaw, _countof(acRaw), pFile) != NULL)
		{
					size_t uRawSize = strlen(acRaw);
					if (acRaw[uRawSize - 1] == _T('\r'))
						--uRawSize;
					if (uRawSize <= 1)
				continue;
					WCHAR awc[_countof(acRaw)];
					sint_ptr iChars = utf8towc(acRaw, uRawSize, awc, _countof(acRaw));
					if (iChars < 0)
					{
						strLine = acRaw;
			m_strFileBuffer.Append(strLine);
			m_strFileBuffer.AppendChar(_T('\n'));
		}
					else if (iChars > 1)
					{
						awc[iChars - 1] = 0;
						m_strFileBuffer.Append(awc);
						m_strFileBuffer.AppendChar(_T('\n'));
					}
				}
			}
			fclose(pFile);
		}
	}

	if(!rstrSection.IsEmpty())
		SetSection(rstrSection);
}

CIni::~CIni()
{
	Save();
}

void CIni::Save()
{
	if(m_bWrite)
{
		if(!strFileName.IsEmpty())
{
			FILE* pFile = _tfsopen(strFileName, _T("wt"), _SH_DENYWR);
			fseek(pFile, 0, SEEK_SET);
			fputs(wc2utf8(m_strFileBuffer), pFile);
			::SetEndOfFile((HANDLE)_get_osfhandle(_fileno(pFile)));
			fclose(pFile);
			m_bWrite = false;
		}
	}
}

/***********************************************
* Section Operations
*/
int CIni::AddSection(const CString& strSectionName)
{
	ASSERT(FindSection(strSectionName) == -1);
	int nSecFirstCharPos = m_strFileBuffer.GetLength();
	if(nSecFirstCharPos != 0){
		++nSecFirstCharPos;
		m_strFileBuffer += _T('\n');
	}

	m_strFileBuffer += _T('[');
	m_strFileBuffer += strSectionName;
	m_strFileBuffer += _T(']');
	return nSecFirstCharPos;
}

void CIni::DeleteSection(const CString& strSectionName)
{
	int nSecFirstCharPos = FindSection(strSectionName);
	if(nSecFirstCharPos == -1)
		nSecFirstCharPos = AddSection(strSectionName);
	int nSecLastCharPos = m_strFileBuffer.Find(_T("\n["), (nSecFirstCharPos + strSectionName.GetLength() + 2));

	if (nSecLastCharPos < 0)
		nSecLastCharPos = m_strFileBuffer.GetLength();

	m_strFileBuffer.Delete(nSecFirstCharPos, (nSecLastCharPos - nSecFirstCharPos));
}

int CIni::FindSection(const CString& strSectionName) const
{
	CString secSearchString = _T("[");
	secSearchString += strSectionName;
	secSearchString += _T(']');

	int nFindPos = m_strFileBuffer.Find(secSearchString);

	// Check if the section is valid
	if (nFindPos > 0)
	{
		secSearchString = _T("\n[");
		secSearchString += strSectionName;
		secSearchString += _T(']');

		--nFindPos;
		nFindPos = m_strFileBuffer.Find(secSearchString, nFindPos);

		if (nFindPos >= 0)
			return nFindPos + 1;
	}

	if(nFindPos < 0)
		return -1;

	return nFindPos;
}

/***********************************************
* Entry/Value Operations
*/


void CIni::DeleteEntry(const CString& strSectionName, const CString &strEntryName)
{
	int nEntryFirstCharPos = FindEntry(strSectionName, strEntryName);
	if(nEntryFirstCharPos >= 0)
	{
		int nEntryLastCharPos = m_strFileBuffer.Find(_T('\n'), nEntryFirstCharPos);

		if (nEntryLastCharPos < 0)
			nEntryLastCharPos = m_strFileBuffer.GetLength();
		else
			++nEntryLastCharPos; // remove entry including "\n"

		m_strFileBuffer.Delete(nEntryFirstCharPos, (nEntryLastCharPos - nEntryFirstCharPos));
	}
}

int CIni::FindEntry(const CString &strSectionName, const TCHAR *pcEntryName) const
{
	int nSecFirstCharPos = FindSection(strSectionName);
	if(nSecFirstCharPos == -1)
		return -1;
	int nSecLastCharPos = m_strFileBuffer.Find(_T("\n["), (nSecFirstCharPos + strSectionName.GetLength() + 2));
	if (nSecLastCharPos < 0)
		// No following section exists - end of section is end of file
		nSecLastCharPos = m_strFileBuffer.GetLength();
	else
		++nSecLastCharPos;

	CString entrySearchString = _T("\n");
	entrySearchString += pcEntryName;
	entrySearchString += _T('=');

	int nEntryFirstCharPos = m_strFileBuffer.Find(entrySearchString, (nSecFirstCharPos + strSectionName.GetLength() + 1));
	
	if (nEntryFirstCharPos < 0){
#ifdef _DEBUG
		CString lowcase(m_strFileBuffer);
		ASSERT((lowcase.MakeLower().Find(entrySearchString.MakeLower(), (nSecFirstCharPos + strSectionName.GetLength() + 1))) < 0);
#endif
		return -1;
	}

	++nEntryFirstCharPos;

	if (/*nEntryFirstCharPos < nSecFirstCharPos || */nEntryFirstCharPos > nSecLastCharPos)
		// The entry we're searching for does not exist in the given section
		return -1;

	return nEntryFirstCharPos;
}

void CIni::WriteValue(const TCHAR *pcEntryName, const CString &strValue)
{
	CString	strEntryName = pcEntryName;
	m_bWrite = true;

	int nEntryFirstCharPos = FindEntry(m_strSection, pcEntryName);
	if (nEntryFirstCharPos >= 0)
	{
		int nEntryLastCharPos = m_strFileBuffer.Find(_T('\n'), nEntryFirstCharPos);

		if (nEntryLastCharPos < 0)
			nEntryLastCharPos = m_strFileBuffer.GetLength();

		int nValueFirstCharPos = nEntryFirstCharPos + strEntryName.GetLength() + 1;

		// Delete an existing value and insert a new value
		m_strFileBuffer.Delete(nValueFirstCharPos, (nEntryLastCharPos - nValueFirstCharPos));
		m_strFileBuffer.Insert(nValueFirstCharPos, strValue);
	}
	else{
		int nSecFirstCharPos = FindSection(m_strSection);
		if(nSecFirstCharPos == -1)
			nSecFirstCharPos = AddSection(m_strSection);
		int nSecLastCharPos = m_strFileBuffer.Find(_T("\n["), (nSecFirstCharPos + m_strSection.GetLength() + 2));

		CString entryString;

		strEntryName += _T('=');
		strEntryName += strValue;

		if (nSecLastCharPos < 0)
		{
			nSecLastCharPos = m_strFileBuffer.GetLength();
			entryString = _T('\n');
			entryString += strEntryName;
		}
		else
		{
			++nSecLastCharPos;
			entryString = strEntryName;
			entryString += _T('\n');
		}

		m_strFileBuffer.Insert(nSecLastCharPos, entryString);
	}
}


/***********************************************
* Value Operations
*/

CString CIni::GetValue(const TCHAR *pcEntryName, bool* ptrSuccess) const
{
	CString	strEntryName = pcEntryName;

	int nEntryFirstCharPos = FindEntry(m_strSection, strEntryName);
	if (nEntryFirstCharPos >= 0)
	{
		int nEntryLastCharPos = m_strFileBuffer.Find(_T('\n'), nEntryFirstCharPos);

		if (nEntryLastCharPos < 0)
			nEntryLastCharPos = m_strFileBuffer.GetLength();

		int nValueFirstCharPos = nEntryFirstCharPos + strEntryName.GetLength() + 1;

		if(ptrSuccess)
			*ptrSuccess = true;

		return m_strFileBuffer.Mid(nValueFirstCharPos, (nEntryLastCharPos - nValueFirstCharPos));
	}
	return _T("");
}

/************************************************************************************************
* All the rest
*/

CString CIni::GetString(LPCTSTR lpszEntry, LPCTSTR lpszDefault, LPCTSTR lpszSection)
{
	if(lpszSection)
		SetSection(lpszSection);

	bool bSuccess = false;
	CString strTemp = GetValue(lpszEntry,&bSuccess);

	if (!bSuccess && lpszDefault){
		//CString strDef = lpszDefault;
		//WriteValue(lpszEntry,strDef);
		return lpszDefault;
	}
	return strTemp;
}

CString CIni::GetStringLong(LPCTSTR lpszEntry, LPCTSTR lpszDefault, LPCTSTR lpszSection)
{
	return GetString(lpszEntry, lpszDefault, lpszSection);
}

CString CIni::GetStringUTF8(LPCTSTR lpszEntry, LPCTSTR lpszDefault, LPCTSTR lpszSection)
{
	return GetString(lpszEntry, lpszDefault, lpszSection);
}

double CIni::GetDouble(LPCTSTR lpszEntry, double fDefault, LPCTSTR lpszSection)
{
	CString m_chBuffer = GetString(lpszEntry, NULL, lpszSection);
	if(m_chBuffer.IsEmpty())
		return fDefault;
	return _tstof(m_chBuffer);
}

float CIni::GetFloat(LPCTSTR lpszEntry, float fDefault, LPCTSTR lpszSection)
{
	CString m_chBuffer = GetString(lpszEntry, NULL, lpszSection);
	if(m_chBuffer.IsEmpty())
		return fDefault;
	return (float)_tstof(m_chBuffer);
}

int CIni::GetInt(LPCTSTR lpszEntry, int nDefault, LPCTSTR lpszSection)
{
	CString m_chBuffer = GetString(lpszEntry, NULL, lpszSection);
	if(m_chBuffer.IsEmpty())
		return nDefault;
	return _tstoi(m_chBuffer);
}

ULONGLONG CIni::GetUInt64(LPCTSTR lpszEntry, ULONGLONG nDefault, LPCTSTR lpszSection)
{
	CString m_chBuffer = GetString(lpszEntry, NULL, lpszSection);
	if(m_chBuffer.IsEmpty())
		return nDefault;
	ULONGLONG nResult;
	if (_stscanf_s(m_chBuffer, _T("%I64u"), &nResult) != 1)
		return nDefault;
	return nResult;
}

WORD CIni::GetWORD(LPCTSTR lpszEntry, WORD nDefault, LPCTSTR lpszSection)
{
	CString m_chBuffer = GetString(lpszEntry, NULL, lpszSection);
	if(m_chBuffer.IsEmpty())
		return nDefault;
	return (WORD)_tstoi(m_chBuffer);
}

bool CIni::GetBool(LPCTSTR lpszEntry, bool bDefault, LPCTSTR lpszSection)
{
	CString m_chBuffer = GetString(lpszEntry, NULL, lpszSection);
	if(m_chBuffer.IsEmpty())
		return bDefault;
	return _tstoi(m_chBuffer) != 0;
}

CPoint CIni::GetPoint(LPCTSTR lpszEntry, CPoint ptDefault, LPCTSTR lpszSection)
{
	CString strPoint = GetString(lpszEntry, NULL, lpszSection);
	if(strPoint.IsEmpty())
		return ptDefault;

	CPoint ptReturn = ptDefault;
	if (_stscanf_s(strPoint,_T("(%d,%d)"), &ptReturn.x, &ptReturn.y) != 2)
		return ptDefault;

	return ptReturn;
}

CRect CIni::GetRect(LPCTSTR lpszEntry, CRect rectDefault, LPCTSTR lpszSection)
{
	CString strRect = GetString(lpszEntry, NULL, lpszSection);
	if(strRect.IsEmpty())
		return rectDefault;

	CRect rectReturn;
	//new Version found
	if (_stscanf_s(strRect, _T("%d,%d,%d,%d"), &rectReturn.left, &rectReturn.top, &rectReturn.right, &rectReturn.bottom) == 4
	//old Version found
		|| _stscanf_s(strRect, _T("(%d,%d,%d,%d)"), &rectReturn.top, &rectReturn.left, &rectReturn.bottom, &rectReturn.right) == 4)
		return rectReturn;
	return rectDefault;
}

COLORREF CIni::GetColRef(LPCTSTR lpszEntry, COLORREF crDefault, LPCTSTR lpszSection)
{
	int temp[3] = {	GetRValue(crDefault),
					GetGValue(crDefault),
					GetBValue(crDefault) };

	CString strDefault;
	strDefault.Format(_T("RGB(%hd,%hd,%hd)"), temp[0], temp[1], temp[2]);

	CString strColRef = GetString(lpszEntry, strDefault, lpszSection);
	if (_stscanf_s(strColRef, _T("RGB(%d,%d,%d)"), temp, temp+1, temp+2) != 3)
		return crDefault;

	return RGB(temp[0], temp[1], temp[2]);
}
	
void CIni::WriteString(LPCTSTR lpszEntry, LPCTSTR lpsz, LPCTSTR lpszSection)
{
	if (lpszSection != NULL) 
		SetSection(lpszSection);
	WriteValue(lpszEntry, lpsz);
}

void CIni::WriteStringUTF8(LPCTSTR lpszEntry, LPCTSTR lpsz, LPCTSTR lpszSection)
{
	WriteString(lpszEntry, lpsz, lpszSection);
}

void CIni::WriteDouble(LPCTSTR lpszEntry, double f, LPCTSTR lpszSection)
{
	TCHAR szBuffer[MAX_PATH];
	_sntprintf(szBuffer, _countof(szBuffer) - 1, _T("%g"), f);
	szBuffer[_countof(szBuffer) - 1] = _T('\0');
	WriteString(lpszEntry, szBuffer, lpszSection);
}

void CIni::WriteFloat(LPCTSTR lpszEntry, float f, LPCTSTR lpszSection)
{
	TCHAR szBuffer[MAX_PATH];
	_sntprintf(szBuffer, _countof(szBuffer) - 1, _T("%g"), f);
	szBuffer[_countof(szBuffer) - 1] = _T('\0');
	WriteString(lpszEntry, szBuffer, lpszSection);
}

void CIni::WriteInt(LPCTSTR lpszEntry, int n, LPCTSTR lpszSection)
{
	TCHAR szBuffer[MAX_PATH];
	_itot_s(n, szBuffer, 10);
	WriteString(lpszEntry, szBuffer, lpszSection);
}

void CIni::WriteUInt64(LPCTSTR lpszEntry, ULONGLONG n, LPCTSTR lpszSection)
{
	TCHAR szBuffer[MAX_PATH];
	_ui64tot_s(n, szBuffer, _countof(szBuffer), 10);
	WriteString(lpszEntry, szBuffer, lpszSection);
}

void CIni::WriteWORD(LPCTSTR lpszEntry, WORD n, LPCTSTR lpszSection)
{
	TCHAR szBuffer[MAX_PATH];
	_ultot_s(n, szBuffer, 10);
	WriteString(lpszEntry, szBuffer, lpszSection);
}

void CIni::WriteBool(LPCTSTR lpszEntry, bool b, LPCTSTR lpszSection)
{
	TCHAR szBuffer[MAX_PATH];
	_itot_s((int)b, szBuffer, 10);
	szBuffer[_countof(szBuffer) - 1] = _T('\0');
	WriteString(lpszEntry, szBuffer, lpszSection);
}

void CIni::WritePoint(LPCTSTR lpszEntry, CPoint pt, LPCTSTR lpszSection)
{
	CString strBuffer;
	strBuffer.Format(_T("(%d,%d)"), pt.x, pt.y);
	WriteString(lpszEntry, strBuffer, lpszSection);
}

void CIni::WriteRect(LPCTSTR lpszEntry, CRect rect, LPCTSTR lpszSection)
{
	CString strBuffer;
	strBuffer.Format(_T("(%d,%d,%d,%d)"), rect.top, rect.left, rect.bottom, rect.right);
	WriteString(lpszEntry, strBuffer, lpszSection);
}

void CIni::WriteColRef(LPCTSTR lpszEntry, COLORREF cr, LPCTSTR lpszSection)
{
	CString strBuffer;
	strBuffer.Format(_T("RGB(%d,%d,%d)"), GetRValue(cr), GetGValue(cr), GetBValue(cr));
	WriteString(lpszEntry, strBuffer, lpszSection);
}

void CIni::SerGetString(bool bGet, CString &rstr, LPCTSTR lpszEntry, LPCTSTR lpszSection, LPCTSTR lpszDefault)
{
	if (bGet)
		rstr = GetString(lpszEntry, lpszDefault, lpszSection);
	else
		WriteString(lpszEntry, rstr, lpszSection);
}

void CIni::SerGetDouble(bool bGet, double &f, LPCTSTR lpszEntry, LPCTSTR lpszSection, double fDefault)
{
	if (bGet)
		f = GetDouble(lpszEntry, fDefault, lpszSection);
	else
		WriteDouble(lpszEntry, f, lpszSection);
}

void CIni::SerGetFloat(bool bGet, float &f, LPCTSTR lpszEntry, LPCTSTR lpszSection, float fDefault)
{
	if (bGet)
		f = GetFloat(lpszEntry, fDefault, lpszSection);
	else
		WriteFloat(lpszEntry, f, lpszSection);
}

void CIni::SerGetInt(bool bGet, int &n, LPCTSTR lpszEntry, LPCTSTR lpszSection, int nDefault)
{
	if (bGet)
		n = GetInt(lpszEntry, nDefault, lpszSection);
	else
		WriteInt(lpszEntry, n, lpszSection);
}

void CIni::SerGetDWORD(bool bGet, DWORD &n,	LPCTSTR lpszEntry, LPCTSTR lpszSection, DWORD nDefault)
{
	if (bGet)
		n = (DWORD)GetInt(lpszEntry, nDefault, lpszSection);
	else
		WriteInt(lpszEntry, n, lpszSection);
}

void CIni::SerGetBool(bool bGet, bool &b, LPCTSTR lpszEntry, LPCTSTR lpszSection, bool bDefault)
{
	if (bGet)
		b = GetBool(lpszEntry, bDefault, lpszSection);
	else
		WriteBool(lpszEntry, b, lpszSection);
}

void CIni::SerGetPoint(bool bGet, CPoint &pt, LPCTSTR lpszEntry, LPCTSTR lpszSection, CPoint ptDefault)
{
	if (bGet)
		pt = GetPoint(lpszEntry, ptDefault, lpszSection);
	else
		WritePoint(lpszEntry, pt, lpszSection);
}

void CIni::SerGetRect(bool bGet, CRect & rect, LPCTSTR lpszEntry, LPCTSTR lpszSection, CRect rectDefault)
{
	if (bGet)
		rect = GetRect(lpszEntry, rectDefault, lpszSection);
	else
		WriteRect(lpszEntry, rect, lpszSection);
}

void CIni::SerGetColRef(bool bGet, COLORREF &cr, LPCTSTR lpszEntry, LPCTSTR lpszSection, COLORREF crDefault)
{
	if (bGet)
		cr = GetColRef(lpszEntry, crDefault, lpszSection);
	else
		WriteColRef(lpszEntry, cr, lpszSection);
}

void CIni::SerGet(bool bGet, CString &rstr, LPCTSTR lpszEntry, LPCTSTR lpszSection, LPCTSTR lpszDefault)
{
	SerGetString(bGet, rstr, lpszEntry, lpszSection, lpszDefault);
}

void CIni::SerGet(bool bGet, double &f, LPCTSTR lpszEntry, LPCTSTR lpszSection, double fDefault)
{
	SerGetDouble(bGet, f, lpszEntry, lpszSection, fDefault);
}

void CIni::SerGet(bool bGet, float &f, LPCTSTR lpszEntry, LPCTSTR lpszSection, float fDefault)
{
	SerGetFloat(bGet, f, lpszEntry, lpszSection, fDefault);
}

void CIni::SerGet(bool bGet, int &n, LPCTSTR lpszEntry, LPCTSTR lpszSection, int nDefault)
{
	SerGetInt(bGet, n, lpszEntry, lpszSection, nDefault);
}

void CIni::SerGet(bool bGet, short &n, LPCTSTR lpszEntry, LPCTSTR lpszSection, int nDefault)
{
	int nTemp = n;
	SerGetInt(bGet, nTemp, lpszEntry, lpszSection, nDefault);
	n = (short)nTemp;
}

void CIni::SerGet(bool bGet, DWORD &n, LPCTSTR lpszEntry, LPCTSTR lpszSection, DWORD nDefault)
{
	SerGetDWORD(bGet, n, lpszEntry, lpszSection, nDefault);
}

void CIni::SerGet(bool bGet, WORD &n, LPCTSTR lpszEntry, LPCTSTR lpszSection, DWORD nDefault)
{
	DWORD dwTemp = n;
	SerGetDWORD(bGet, dwTemp, lpszEntry, lpszSection, nDefault);
	n = (WORD)dwTemp;
}

void CIni::SerGet(bool bGet, CPoint &pt, LPCTSTR lpszEntry, LPCTSTR lpszSection, CPoint ptDefault)
{
	SerGetPoint(bGet, pt, lpszEntry, lpszSection, ptDefault);
}

void CIni::SerGet(bool bGet, CRect &rect, LPCTSTR lpszEntry, LPCTSTR lpszSection, CRect rectDefault)
{
	SerGetRect(bGet, rect, lpszEntry, lpszSection, rectDefault);
}

void CIni::SerGet(bool bGet, CString *ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, LPCTSTR lpszDefault)
{
	if (nCount > 0) {
		CString strBuffer;
		if (bGet) {
			strBuffer = GetString(lpszEntry, NULL, lpszSection);
			int nOffset = 0;
			for (int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, ar[i]);
				if (ar[i].GetLength() == 0)
					ar[i] = lpszDefault;
			}
		} else {
			strBuffer = ar[0];
			for (int i = 1; i < nCount; i++) {
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(ar[i]);
			}
			WriteString(lpszEntry, strBuffer, lpszSection);
		}
	}
}

void CIni::SerGet(bool bGet, double *ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, double fDefault)
{
	if (nCount > 0) {
		CString strBuffer;
		if (bGet) {
			strBuffer = GetString(lpszEntry, NULL, lpszSection);
			CString strTemp;
			int nOffset = 0;
			for (int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if (strTemp.GetLength() == 0)
					ar[i] = fDefault;
				else
					ar[i] = _tstof(strTemp);
			}
		} else {
			CString strTemp;
			strBuffer.Format(_T("%g"), ar[0]);
			for (int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%g"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(lpszEntry, strBuffer, lpszSection);
		}
	}
}

void CIni::SerGet(bool bGet, float *ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, float fDefault)
{
	if (nCount > 0) {
		CString strBuffer;
		if (bGet) {
			strBuffer = GetString(lpszEntry, NULL, lpszSection);
			CString strTemp;
			int nOffset = 0;
			for (int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if (strTemp.GetLength() == 0)
					ar[i] = fDefault;
				else
					ar[i] = (float)_tstof(strTemp);
			}
		} else {
			CString strTemp;
			strBuffer.Format(_T("%g"), ar[0]);
			for (int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%g"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(lpszEntry, strBuffer, lpszSection);
		}
	}
}

void CIni::SerGet(bool bGet, int *ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, int iDefault)
{
	if (nCount > 0) {
		CString strBuffer;
		if (bGet) {
			strBuffer = GetString(lpszEntry, NULL, lpszSection);
			CString strTemp;
			int nOffset = 0;
			for (int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if (strTemp.GetLength() == 0)
					ar[i] = iDefault;
				else
					ar[i] = _tstoi(strTemp);
			}
		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for (int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(lpszEntry, strBuffer, lpszSection);
		}
	}
}

void CIni::SerGet(bool bGet, unsigned char *ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, unsigned char ucDefault)
{
	if (nCount > 0) {
		CString strBuffer;
		if (bGet) {
			strBuffer = GetString(lpszEntry, NULL, lpszSection);
			CString strTemp;
			int nOffset = 0;
			for (int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if (strTemp.GetLength() == 0)
					ar[i] = ucDefault;
				else
					ar[i] = (unsigned char)_tstoi(strTemp);
			}
		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for (int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(lpszEntry, strBuffer, lpszSection);
		}
	}
}

void CIni::SerGet(bool bGet, short *ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, int iDefault)
{
	if (nCount > 0) {
		CString strBuffer;
		if (bGet) {
			strBuffer = GetString(lpszEntry, NULL, lpszSection);
			CString strTemp;
			int nOffset = 0;
			for (int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if (strTemp.GetLength() == 0)
					ar[i] = (short)iDefault;
				else
					ar[i] = (short)_tstoi(strTemp);
			}
		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for (int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(lpszEntry, strBuffer, lpszSection);
		}
	}
}

void CIni::SerGet(bool bGet, DWORD *ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, DWORD dwDefault)
{
	if (nCount > 0) {
		CString strBuffer;
		if (bGet) {
			strBuffer = GetString(lpszEntry, NULL, lpszSection);
			CString strTemp;
			int nOffset = 0;
			for (int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = dwDefault;
				else
					ar[i] = (DWORD)_tstoi(strTemp);
			}
		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for (int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(lpszEntry, strBuffer, lpszSection);
		}
	}
}

void CIni::SerGet(bool bGet, WORD *ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, DWORD dwDefault)
{
	if (nCount > 0) {
		CString strBuffer;
		if (bGet) {
			strBuffer = GetString(lpszEntry, NULL, lpszSection);
			CString strTemp;
			int nOffset = 0;
			for (int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if (strTemp.GetLength() == 0)
					ar[i] = (WORD)dwDefault;
				else
					ar[i] = (WORD)_tstoi(strTemp);
			}
		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for (int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(lpszEntry, strBuffer, lpszSection);
		}
	}
}

void CIni::SerGet(bool bGet, CPoint * ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, CPoint ptDefault)
{
	CString strBuffer;
	for (int i = 0; i < nCount; i++)
	{
		strBuffer.Format(_T("_%i"), i);
		strBuffer = lpszEntry + strBuffer;
		SerGet(bGet, ar[i], strBuffer, lpszSection, ptDefault);
	}
}

void CIni::SerGet(bool bGet, CRect *ar, int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection, CRect rcDefault)
{
	CString strBuffer;
	for (int i = 0; i < nCount; i++)
	{
		strBuffer.Format(_T("_%i"), i);
		strBuffer = lpszEntry + strBuffer;
		SerGet(bGet, ar[i], strBuffer, lpszSection, rcDefault);
	}
}

int CIni::Parse(const CString &strIn, int nOffset, CString &strOut)
{
	strOut.Empty();
	int nLength = strIn.GetLength();

	if (nOffset < nLength) {
		if (nOffset != 0 && strIn[nOffset] == _T(','))
			nOffset++;

		while (nOffset < nLength) {
			if (!_istspace((_TUCHAR)strIn[nOffset]))
				break;
			nOffset++;
		}

		while (nOffset < nLength) {
			strOut += strIn[nOffset];
			if (strIn[++nOffset] == _T(','))
				break;
		}
		strOut.Trim();
	}
	return nOffset;
}

bool CIni::GetBinary(LPCTSTR lpszEntry, BYTE** ppData, size_t* pBytes, LPCTSTR pszSection)
{
	*ppData = NULL;
	*pBytes = 0;

	CString str = GetString(lpszEntry, NULL, pszSection);
	if (str.IsEmpty())
		return false;
	ASSERT(str.GetLength()%2 == 0);
	size_t nLen = str.GetLength();
	*pBytes = size_t(nLen)/2;
	*ppData = new BYTE[*pBytes];
	for (size_t i=0;i<nLen;i+=2)
	{
		(*ppData)[i/2] = (BYTE)(((str[i+1] - 'A') << 4) + (str[i] - 'A'));
	}
	return true;
}

void CIni::WriteBinary(LPCTSTR lpszEntry, LPBYTE pData, size_t nBytes, LPCTSTR pszSection)
{
	// convert to string and write out
	LPTSTR lpsz = new TCHAR[nBytes*2+1];
	size_t i;
	for (i = 0; i < nBytes; i++)
	{
		lpsz[i*2] = (TCHAR)((pData[i] & 0x0F) + 'A'); //low nibble
		lpsz[i*2+1] = (TCHAR)(((pData[i] >> 4) & 0x0F) + 'A'); //high nibble
	}
	lpsz[i*2] = 0;

	WriteString(lpszEntry, lpsz, pszSection);
	delete[] lpsz;
}
// X-Ray :: eMulePlusIniClass :: End