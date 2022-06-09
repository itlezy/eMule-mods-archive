//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "emule.h"
#include "Preview.h"
#include "PartFile.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CPreviewThread, CWinThread)

CPreviewThread::CPreviewThread() : m_strPlayerCmd(_T("")), m_strPlayerArgs(_T(""))
{
}

CPreviewThread::~CPreviewThread()
{
}

BOOL CPreviewThread::InitInstance()
{
	g_App.m_pPrefs->InitThreadLocale();
	return true;
}

BOOL CPreviewThread::Run()
{
	ASSERT (m_pPartFile);
	CFile* srcFile = NULL;
	CFile destFile;

	try
	{
		srcFile = m_pPartFile->GetPartFileHandle().Duplicate();
		CString strExtension = m_pPartFile->GetFileExtension();
		CString strPreviewName = m_pPartFile->GetTempDir() + _T("\\") + m_pPartFile->GetFileName().Mid(0, 10) + _T("_preview.") + strExtension;
		bool bFullSized = true;
		if (strExtension == _T("mpg") || strExtension == _T("mpeg"))
			bFullSized = false;
		destFile.Open(strPreviewName, CFile::modeWrite | CFile::shareDenyWrite | CFile::modeCreate);
		srcFile->SeekToBegin();
		if (bFullSized)
			destFile.SetLength(m_pPartFile->GetFileSize());
		destFile.SeekToBegin();
		BYTE abyBuffer[4096];

		uint32 nRead;
		while (destFile.GetPosition()+4096 < PARTSIZE*2)
		{
			nRead = srcFile->Read(abyBuffer, 4096);
			destFile.Write(abyBuffer, nRead);
		}

		srcFile->Seek(-static_cast<LONGLONG>(PARTSIZE * 2), CFile::end);
		uint32 nToGo = PARTSZ32 * 2;
		if (bFullSized)
			destFile.Seek(-static_cast<LONGLONG>(PARTSIZE * 2), CFile::end);
		do
		{
			nRead = ((nToGo - 4096) < 1) ? nToGo : 4096;
			nToGo -= nRead;
			nRead = srcFile->Read(abyBuffer, 4096);
			destFile.Write(abyBuffer, nRead);
		}
		while (nToGo);

		destFile.Close();
		srcFile->Close();
		m_pPartFile->m_bPreviewing = false;
		SHELLEXECUTEINFO SE;			// Why so complicated and not just a 'simple' ShellExecute(...)??
		memzero(&SE, sizeof(SE));
		SE.fMask = SEE_MASK_NOCLOSEPROCESS;
		if (!m_strPlayerCmd.IsEmpty())
		{
			CString	strArgs(m_strPlayerArgs);
			CString	strRunDir(m_strPlayerCmd);
			int		iSpace, i = strRunDir.ReverseFind(_T('\\'));

			strRunDir.Truncate((i >= 0) ? (i + 1) : 0);

			if (!strArgs.IsEmpty())
				strArgs += _T(' ');
			iSpace = strPreviewName.Find(_T(' '));
			if (iSpace >= 0)
				strArgs += _T('\"');
			strArgs += strPreviewName;
			if (iSpace >= 0)
				strArgs += _T('\"');

			SE.lpVerb = _T("open");
			SE.lpFile = m_strPlayerCmd.GetString();
			SE.lpParameters = strArgs.GetString();
			SE.lpDirectory = strRunDir.GetString();
		}
		else
		{
			SE.lpVerb = NULL;	// use the default verb or the open verb for the document
			SE.lpFile = strPreviewName.GetString();
		}
		SE.nShow = SW_SHOW;
		SE.cbSize = sizeof(SE);
		ShellExecuteEx(&SE);
		if (SE.hProcess)
		{
			WaitForSingleObject(SE.hProcess, INFINITE);
			DWORD dwExitCode;
			do
			{
				Sleep(300);
				GetExitCodeProcess(SE.hProcess,&dwExitCode);
			}
			while(dwExitCode == STILL_ACTIVE);
			CloseHandle(SE.hProcess);
		}
		CFile::Remove(strPreviewName);
	}
	catch(CFileException* error)
	{
		m_pPartFile->m_bPreviewing = false;
		if (srcFile->m_hFile != INVALID_HANDLE_VALUE)
			srcFile->Close();
		if (destFile.m_hFile != INVALID_HANDLE_VALUE)
			destFile.Close();
		error->Delete();
	}
	delete srcFile;
	AfxEndThread(0,true);
	return 0;
}

void CPreviewThread::SetValues(CPartFile* pPartFile, const CString &strCmd, const CString &strCmdArgs)
{
	m_pPartFile = pPartFile;
	m_strPlayerCmd = strCmd;
	m_strPlayerArgs = strCmdArgs;
}

BEGIN_MESSAGE_MAP(CPreviewThread, CWinThread)
END_MESSAGE_MAP()
