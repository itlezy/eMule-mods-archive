//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include <io.h>
#include "emule.h"
#include "loggable.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#ifndef _CONSOLE
#include "emuledlg.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void CLoggable::AddLogLine(bool addtostatusbar, UINT nID, ...)
{
	va_list argptr;
	va_start(argptr, nID);	
	AddLogText(false, addtostatusbar, GetResString(nID), argptr);	
	va_end(argptr);	
}

void CLoggable::AddLogLine(bool addtostatusbar, LPCTSTR line, ...)
{
	ASSERT(line != NULL);	

	va_list argptr;
	va_start(argptr, line);	
	AddLogText(false, addtostatusbar, line, argptr);	
	va_end(argptr);
}

void CLoggable::AddDebugLogLine(bool addtostatusbar, UINT nID, ...)
{
	va_list argptr;
	va_start(argptr, nID);	
	AddLogText(true, addtostatusbar, GetResString(nID), argptr);			
	va_end(argptr);	
}

void CLoggable::AddDebugLogLine(bool addtostatusbar, LPCTSTR line, ...)
{
	ASSERT(line != NULL);

	va_list argptr;
	va_start(argptr, line);
	AddLogText(true, addtostatusbar, line, argptr);	
	va_end(argptr);	
}

void CLoggable::AddLogText(bool debug, bool addtostatusbar,LPCTSTR line, va_list argptr)
{
	ASSERT(line != NULL);

	if (debug && !thePrefs.GetVerbose())
		return;	

	const size_t bufferSize = 1000;
	TCHAR bufferline[bufferSize];	
	if (_vsntprintf(bufferline, bufferSize, line, argptr) == -1)
		bufferline[bufferSize - 1] = _T('\0');
	
	if (theApp.emuledlg)
		theApp.emuledlg->AddLogText(addtostatusbar, bufferline, debug);	//Cax2 - debug log and normal log handled by the same subroutine now
#ifdef _DEBUG
	else{
		TRACE("App Log: %s\n", bufferline);
	}
#endif
}

// DbT:Logging
void CLoggable::PacketToDebugLogLine(LPCTSTR info, char * packet, uint32 size, uint8 opcode) const
{
	CString buffer; 
	buffer.Format(_T("%s: %02x, size=%u"), info, opcode, size);
	buffer += _T(", data=[");
	uint32 maxsize = 100;
	for(uint32 i = 0; i < size && i < maxsize; i++){		
		buffer.AppendFormat(_T("%02x"), (uint8)packet[i]);
		buffer += _T(" ");
	}
	buffer += ((size < maxsize) ? _T("]") : _T("..]"));
	AddDebugLogLine(false, buffer); 
}

void CLoggable::TagToDebugLogLine(LPCTSTR info, LPCTSTR tag, uint32 size, uint8 opcode) const
{
	CString buffer;
	buffer.Format(_T("%s: %02x, size=%u"), info, opcode, size);
	buffer += _T(", data=[");
	uint32 maxsize = 100;
	for(uint32 i = 0; i < size && i < maxsize; i++){		
		buffer.AppendFormat(_T("%02x"), (uint8)tag[i]);
		buffer += _T(" ");
	}
	buffer += ((size < maxsize) ? _T("]") : _T("..]"));
	AddDebugLogLine(false, buffer); 
}
// DbT:End


///////////////////////////////////////////////////////////////////////////////
// CLog

CLog::CLog()
{
	(void)m_strFilePath;
	m_uMaxFileSize = (UINT)-1;
	m_uBytesWritten = 0;
	m_tStarted = 0;
	m_fp = NULL;
}

CLog::~CLog()
{
	Close();
}

const CString& CLog::GetFilePath() const
{
	return m_strFilePath;
}

bool CLog::SetFilePath(LPCTSTR pszFilePath)
{
	if (IsOpen())
		return false;
	m_strFilePath = pszFilePath;
	return true;
}

void CLog::SetMaxFileSize(UINT uMaxFileSize)
{
	if (uMaxFileSize == 0)
		uMaxFileSize = (UINT)-1;
	else if (uMaxFileSize < 0x10000)
		uMaxFileSize = 0x10000;
	m_uMaxFileSize = uMaxFileSize;
}

bool CLog::IsOpen() const
{
	return m_fp != NULL;
}

bool CLog::Create(LPCTSTR pszFilePath, UINT uMaxFileSize)
{
	Close();
	m_strFilePath = pszFilePath;
	m_uMaxFileSize = uMaxFileSize;
	return Open();
}

bool CLog::Open()
{
	if (m_fp != NULL)
		return true;

	m_fp = fopen(m_strFilePath, "ab");
	if (m_fp != NULL)
	{
		m_tStarted = time(NULL);
		m_uBytesWritten = _filelength(fileno(m_fp));
	}
	return m_fp != NULL;
}

bool CLog::Close()
{
	if (m_fp == NULL)
		return true;
	bool bResult = (fclose(m_fp) == 0);
	m_fp = NULL;
	m_tStarted = 0;
	m_uBytesWritten = 0;
	return bResult;
}

bool CLog::Log(LPCTSTR pszMsg, int iLen)
{
	if (m_fp == NULL)
		return false;

	// don't use 'fputs' + '_filelength' -- gives poor performance
	size_t uToWrite = (iLen == -1) ? _tcslen(pszMsg) : (size_t)iLen;
	size_t uWritten = fwrite(pszMsg, 1, uToWrite, m_fp);
	bool bResult = !ferror(m_fp);
	m_uBytesWritten += uWritten;

	if (m_uBytesWritten >= m_uMaxFileSize)
	{
		time_t tStarted = m_tStarted;
		Close();

		TCHAR szDateLogStarted[40];
		_tcsftime(szDateLogStarted, ARRSIZE(szDateLogStarted), _T("%Y.%m.%d %H.%M.%S"), localtime(&tStarted));

		TCHAR szDrv[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szNam[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];
		_tsplitpath(m_strFilePath, szDrv, szDir, szNam, szExt);

		CString strLogBakNam;
		strLogBakNam = szNam;
		strLogBakNam += _T(" - ");
		strLogBakNam += szDateLogStarted;

		TCHAR szLogBakFilePath[MAX_PATH];
		_tmakepath(szLogBakFilePath, szDrv, szDir, strLogBakNam, szExt);

		if (_trename(m_strFilePath, szLogBakFilePath) != 0)
			_tremove(m_strFilePath);

		Open();
	}
	else
		fflush(m_fp);

	return bResult;
}
