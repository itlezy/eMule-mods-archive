// TaskProcessorFiles.h: interface for the CTaskProcessor_Files class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../../Engine/TaskProcessor.h"

class CPartFile;
class CKnownFile;

struct CTask_FileHashed : public CTask
{
	CTask_FileHashed(CPartFile* pPartFile, CKnownFile* pKnownFile);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("FileHashed"); }

	CPartFile*	m_pPartFile;
	CKnownFile*	m_pKnownFile;
};

class CKnownFileList;
class CSharedFileList;
class CFileHashControl;

class CTaskProcessor_Files : public CTaskProcessor
{
	virtual bool Start();
	virtual void Stop();
	virtual void ProcessTimeout();

public:
	CTaskProcessor_Files();
	virtual ~CTaskProcessor_Files();

	__declspec(property(get=_GetKnownFiles)) CKnownFileList&	KnownFiles;
	__declspec(property(get=_GetSharedFiles)) CSharedFileList&	SharedFiles;
	__declspec(property(get=_GetFileHasher)) CFileHashControl&	FileHasher;

	CKnownFileList&		_GetKnownFiles()	const { if(!m_pKnownFiles) throw;	return *m_pKnownFiles; }
	CSharedFileList&	_GetSharedFiles()	const { if(!m_pSharedFiles)throw;	return *m_pSharedFiles; }
	CFileHashControl&	_GetFileHasher()	const { if(!m_pFileHasher) throw;	return *m_pFileHasher; }

private:
	CKnownFileList*		m_pKnownFiles;
	CSharedFileList*	m_pSharedFiles;
	CFileHashControl*	m_pFileHasher;
};

