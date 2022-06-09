// TaskProcessorFiles.cpp: implementation of the CTaskProcessor_Files class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TaskProcessorFiles.h"
#include "../../KnownFileList.h"
#include "../../SharedFileList.h"
#include "../../FileHashControl.h"
#include "../Data/Prefs.h"

CTaskProcessor_Files::CTaskProcessor_Files()
{
	m_dwStartupTimeout = 10000;	// 10 seconds startup timeout
}

CTaskProcessor_Files::~CTaskProcessor_Files()
{
	Stop();
}

bool CTaskProcessor_Files::Start()
{
	m_dwWaitTimeout = 1000;

	EMULE_TRY

	m_pKnownFiles = new CKnownFileList(g_stEngine.Prefs.GetConfigDir());
	m_pSharedFiles = new CSharedFileList(&g_stEngine.Prefs, m_pKnownFiles);
	m_pFileHasher = new CFileHashControl;

	FileHasher.Init();
	SharedFiles.FindSharedFiles();

	return true;
	
	EMULE_CATCH

	return false;
}

void CTaskProcessor_Files::Stop()
{
	EMULE_TRY

	if(m_pSharedFiles)
	{
		delete m_pSharedFiles;
		m_pSharedFiles = NULL;
	}
	if(m_pKnownFiles)
	{
		delete m_pKnownFiles;
		m_pKnownFiles = NULL;
	}
	if(m_pFileHasher)
	{
		m_pFileHasher->Destroy();
		delete m_pFileHasher;
		m_pFileHasher = NULL;
	}

	EMULE_CATCH
}

void CTaskProcessor_Files::ProcessTimeout()
{
	EMULE_TRY

	m_dwWaitTimeout = 30000; // perform save once per 30 second

	KnownFiles.Save();

	EMULE_CATCH
}

CTask_FileHashed::CTask_FileHashed(CPartFile* pPartFile, CKnownFile* pKnownFile)
	:m_pPartFile(pPartFile)
	,m_pKnownFile(pKnownFile)
{
}

bool CTask_FileHashed::Process()
{
	EMULE_TRY

	if(m_pPartFile != 0)
		m_pPartFile->PartFileHashFinished(m_pKnownFile);
	else
	{
		g_stEngine.Files.KnownFiles.SafeAddKnownFile(m_pKnownFile);
		g_stEngine.Files.SharedFiles.SafeAddKnownFile(m_pKnownFile);
		AddLog(LOG_NOTICE, _T("File hashed: %s"), m_pKnownFile->GetFileName());
	}

	EMULE_CATCH

	return true;
}
