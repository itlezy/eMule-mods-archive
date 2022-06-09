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
#include "FileHashControl.h"
#include "ProcessingCmdThread.h"
#ifndef NEW_SOCKETS_ENGINE
	#include "emule.h"
#else
	#include "otherfunctions.h"
#endif //NEW_SOCKETS_ENGINE

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CFileHashControl::CFileHashControl(void)
	: m_bInitialized(false)
{
	m_pProcessThread = NULL;
	m_iThreadPriority = THREAD_PRIORITY_BELOW_NORMAL;
}

CFileHashControl::~CFileHashControl(void)
{
	if (m_bInitialized)
		Destroy();
}

HRESULT CFileHashControl::Init(void)
{
	HRESULT	hr;

	if (m_bInitialized)
	//	Already initialized
		return S_FALSE;

	hr = CreateProcessingThread();
	if (FAILED(hr))
		return hr;

	m_bInitialized = true;
	return S_OK;
}

HRESULT CFileHashControl::Destroy(void)
{
	HRESULT	hr = S_OK;

	if (!m_bInitialized)
		return S_OK;

	if (m_pProcessThread == NULL)
		return S_FALSE;

//	Try to kill the thread 
	hr = KillThread();
//	Reset pointers
	m_bInitialized = false;
	m_pProcessThread = NULL;
	return hr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Adds file to the hash thread queue to be hashed
HRESULT CFileHashControl::AddToHash(const CString &strInFolder, const CString &strFileName)
{
//	Check if the current state is OK
	if (!m_bInitialized)
		return E_UNEXPECTED;

//	Try to add file to the queue
	if (!m_pProcessThread->AddFileToHash(strInFolder, strFileName))
		return E_FAIL;

	return S_OK;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CFileHashControl::CreateProcessingThread(void)
{
	if (m_pProcessThread != NULL || m_bInitialized)
		return E_UNEXPECTED;

	m_pProcessThread = reinterpret_cast<CProcessingCmdThread*>(AfxBeginThread(RUNTIME_CLASS(CProcessingCmdThread), m_iThreadPriority, 0, 0));
	if (m_pProcessThread == NULL)
		return E_FAIL;

	return S_OK;
}

HRESULT CFileHashControl::KillThread(void)
{
	if (m_pProcessThread == NULL || !m_bInitialized)
	//	If thread isn't initialized or pointer is NULL return false success
		return S_FALSE;

//	Ask the thread to stop
	m_pProcessThread->StopThread();

//	Wait until thread exits
	DWORD	dwRet = ::WaitForSingleObject(m_pProcessThread->m_hThread, 1000000);

	return (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED) ? S_OK : E_UNEXPECTED;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SetThreadPriority() sets the runtime priority of the Hashing Thread.
void CFileHashControl::SetThreadPriority(int iPriority)
{
#ifndef NEW_SOCKETS_ENGINE
	m_iThreadPriority = iPriority;

	if (m_pProcessThread != NULL && m_bInitialized)
		m_pProcessThread->SetThreadPriority(m_iThreadPriority + g_App.m_pPrefs->GetMainProcessPriority());
#endif //NEW_SOCKETS_ENGINE
}
