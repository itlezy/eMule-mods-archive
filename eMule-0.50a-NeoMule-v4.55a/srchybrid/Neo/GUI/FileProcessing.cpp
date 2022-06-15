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

// Original author: Mighty Knife, EMule Morph Team

#include "stdafx.h"
#include "emule.h"
#include "SharedFileList.h"
#include "FileProcessing.h"
#include "OtherFunctions.h"

IMPLEMENT_DYNCREATE(CFileProcessingWorker, CObject)

void CFileProcessingWorker::SetFileHashToProcess(const uchar* _fileHash) { 
	ASSERT (_fileHash != NULL);
	md4cpy (m_fileHashToProcess,_fileHash);
}

CKnownFile* CFileProcessingWorker::ValidateKnownFile (const uchar* _fileHash) {
	ASSERT (_fileHash != NULL);
	if (_fileHash==NULL) return NULL; // No file hash
	m_SharedFileListLock = new CSingleLock (&theApp.hashing_mut,true);
	CKnownFile* thefile = theApp.sharedfiles->GetFileByID (_fileHash);
	if (thefile==NULL) UnlockSharedFilesList (); // file not in the list
	return thefile;
}

void CFileProcessingWorker::UnlockSharedFilesList () {
	if (m_SharedFileListLock) {
		m_SharedFileListLock->Unlock();
		delete m_SharedFileListLock;
		m_SharedFileListLock = NULL;
	}
}

IMPLEMENT_DYNCREATE(CFileProcessingThread, CWinThread)

int	CFileProcessingThread::Run () {
	// NEO: SSH - [SlugFillerSafeHash]
	// BEGIN SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// END SLUGFILLER: SafeHash
	// NEO: SSH END

	m_IsRunning = true;
	// Process Run()-method of every stored worker object
	while (!fileWorkers.IsEmpty ()) {
		// Block the list so we can access it
		CSingleLock lck (&m_FilelistLocked,true);
		CFileProcessingWorker* worker = fileWorkers.RemoveHead ();
		// Release the lock
		lck.Unlock ();
		// If the thread should terminate we don't run the job
		if (!IsTerminating()) {
			// Set the owner and run the worker
			worker->SetOwner (this);
			worker->Run();
		}
		delete worker;
	}
	bool b = IsTerminating();
	m_IsTerminating = false;
	m_IsRunning = false;
	return !b;
}

void    CFileProcessingThread::AddFileProcessingWorker (CFileProcessingWorker* _worker) {
	// Block the list so we can access it
	CSingleLock lck (&m_FilelistLocked,true);
	// Add the file worker
	fileWorkers.AddTail (_worker);
	// Release the lock
	lck.Unlock ();
}
