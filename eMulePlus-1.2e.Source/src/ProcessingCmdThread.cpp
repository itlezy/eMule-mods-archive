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

#include "stdafx.h"
#ifndef NEW_SOCKETS_ENGINE
	#include "emule.h"
#else
	#include "otherfunctions.h"
	#include "Engine/Files/TaskProcessorFiles.h"
#endif //NEW_SOCKETS_ENGINE
#include "ProcessingCmdThread.h"
#include "SharedFileList.h"
#include "KnownFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CProcessingCmdThread, CWinThread)

BEGIN_MESSAGE_MAP(CProcessingCmdThread, CWinThread)
	ON_THREAD_MESSAGE(CMD_HASH, OnHash)
	ON_THREAD_MESSAGE(CMD_STOP, OnStop)
END_MESSAGE_MAP()

CProcessingCmdThread::CProcessingCmdThread()
{
}

BOOL CProcessingCmdThread::InitInstance()
{
#ifdef EP_SPIDERWEB
//	Setup Structured Exception handler for the this thread
	_set_se_translator(StructuredExceptionHandler);
#endif
	return TRUE;
}

//	Stop the thread
bool CProcessingCmdThread::StopThread()
{
	bool	bPosted = true;

	if (m_hThread != NULL && !IsThreadAboutToStop())
		bPosted = B2b(PostThreadMessage(CMD_STOP, 0, 0));

	return bPosted;
}

bool CProcessingCmdThread::IsThreadAboutToStop()
{
	MSG	msg;

	return B2b(::PeekMessage(&msg, reinterpret_cast<HWND>(-1), CMD_STOP, CMD_STOP, PM_NOREMOVE));
}

void CProcessingCmdThread::OnHash(WPARAM wParam, LPARAM lParam)
{
	NOPRM(wParam);
	UnknownFile_Struct	*pStruct = reinterpret_cast<UnknownFile_Struct*>(lParam);
	bool	bDelete = true;

	if (!IsThreadAboutToStop())
		bDelete = HashFile(pStruct);
	else if (GetThreadPriority() < g_App.m_pPrefs->GetMainProcessPriority())
	//	We need to boost priority to exit faster
		SetThreadPriority(g_App.m_pPrefs->GetMainProcessPriority() + THREAD_PRIORITY_NORMAL);

	if (bDelete)
		delete pStruct;
}

void CProcessingCmdThread::OnStop(WPARAM wParam, LPARAM lParam)
{
	NOPRM(wParam); NOPRM(lParam);
	MSG	msg;

//	Purge all messages
	while (::PeekMessage(&msg, reinterpret_cast<HWND>(-1), CMD_HASH, CMD_STOP, PM_REMOVE))
	{
	//	Free hashing informations
		if (msg.message == CMD_HASH)
			delete reinterpret_cast<UnknownFile_Struct*>(msg.lParam);
	}

	::PostQuitMessage(0);
}

// Performs actual hashing of the file
bool CProcessingCmdThread::HashFile(UnknownFile_Struct *pStruct)
{
	bool	bDelete = true;
	CKnownFile	*pNewFile = new CKnownFile();

	if (pNewFile != NULL)
	{
	//	Run creation of the hash data from file
		if (pNewFile->CreateFromFile(pStruct->m_strDirectory, pStruct->m_strFileName, false))
		{
#ifndef NEW_SOCKETS_ENGINE
		//	Notify main program about finished hashing
		//	It assumes that window getting this message will at some point release the buffer
		//	theoretically, in some border cases, it can cause small memory leak, but it's unlikely
			if ( IsThreadAboutToStop() || g_App.m_pMDlg == NULL || !g_App.m_pMDlg->IsRunning() || !::IsWindow(g_App.m_pMDlg->m_hWnd) ||
				!::PostMessage(g_App.m_pMDlg->m_hWnd, TM_FINISHEDHASHING, 0, reinterpret_cast<LPARAM>(pNewFile)) )
			{
				delete pNewFile;
			}
#else
			CTask_FileHashed	*pTask = new CTask_FileHashed(NULL, pNewFile);

			g_stEngine.Files.Push(pTask);
#endif //NEW_SOCKETS_ENGINE
		}
		else
		{
#ifndef NEW_SOCKETS_ENGINE
		//	Notify main program of hash failure
			if (g_App.m_pMDlg != NULL && g_App.m_pMDlg->IsRunning() && ::IsWindow(g_App.m_pMDlg->m_hWnd))
				bDelete = !::PostMessage(g_App.m_pMDlg->m_hWnd, TM_HASHFAILED, 0, reinterpret_cast<LPARAM>(pStruct));
#endif //NEW_SOCKETS_ENGINE
			delete pNewFile;
		}
	}

	return bDelete;
}

// Adds file to hashing queue
bool CProcessingCmdThread::AddFileToHash(const CString &strFolder, const CString &strFileName)
{
//	Check if we can add something
	if (m_hThread == NULL || IsThreadAboutToStop())
		return false;

	UnknownFile_Struct	*pStruct = new UnknownFile_Struct;

//	Fill transfer structure with processing data
	pStruct->m_strDirectory = strFolder;
	pStruct->m_strFileName = strFileName;

	bool	bPosted = B2b(PostThreadMessage(CMD_HASH, 0, reinterpret_cast<LPARAM>(pStruct)));

	if (!bPosted)
		delete pStruct;

	return bPosted;
}
