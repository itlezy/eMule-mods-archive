
#include "stdafx.h"
#include "functions.h"

CString DeCodeString(CString String)
{
	CString buffer = String;
	CString value = _T("");
	TCHAR nvalue[2] = {20,0};
	int num1 = 0;
	int num2 = 0;
	int findPos = 0;

	while ((findPos = buffer.Find(_T("%"))) > -1)
	{
		value = buffer.Mid(findPos, 3);
		swscanf(value,_T("%%%01X%01X"),&num1,&num2);
		nvalue[0]  = (TCHAR)num1 << 4;
		nvalue[0] |= (TCHAR)num2;
		buffer.Replace(value,nvalue);
	}

	return buffer;
}

CString ParseString(CString &strIn, int &nOffset, TCHAR what)
{
	CString strOut;
	int nLength = strIn.GetLength();

	if(nOffset < nLength)
	{
		if(strIn[nOffset] == what)
		{
			nOffset++;
			return strOut;
		}

		while(nOffset < nLength)
		{
			strOut += strIn[nOffset];
			if(strIn[++nOffset] == what)
				break;
		}

		strOut.Trim();
		nOffset++;
	}

	return strOut;
}

CString StrLine(LPCTSTR line, ...)
{
	ASSERT(line != NULL);

	va_list argptr;
	va_start(argptr, line);
	const size_t bufferSize = 1000;
	TCHAR bufferline[bufferSize];	
	if (_vsnwprintf(bufferline, bufferSize, line, argptr) == -1)
		bufferline[bufferSize - 1] = _T('\0');
	va_end(argptr);	

	return bufferline;
}