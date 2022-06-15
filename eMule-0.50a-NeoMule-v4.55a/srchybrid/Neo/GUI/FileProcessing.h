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

#pragma once

#include "KnownFile.h"
#include "OtherFunctions.h"

class CFileProcessingThread;

// Mighty Knife: The following class is an abstract class for a worker thread that can be used
// in CFileProcessingThread.
// For every file to process a worker thread has to be created and being added to
// CFileProcessingThread::fileWorkers. CFileProcessingThread will then call the Run()-
// methods of every object in that list.
class CFileProcessingWorker : public CObject
{
	DECLARE_DYNCREATE(CFileProcessingWorker)
protected:
	CSingleLock* m_SharedFileListLock;
	uchar		 m_fileHashToProcess [16];
	CString		 m_FilePath; 
	CFileProcessingThread* m_pOwner;

	// The following function requests the pointer of the file with the specified hash-id.
	// If it exists the SharedFileList will be locked and the pointer will be returned.
	// Otherwise the return value will be NULL.
	// The caller should call UnlockSharedFileList as soon as possible to unlock the 
	// SharedFileList !
public:
	CKnownFile*	ValidateKnownFile (const uchar* _fileHash);
	CFileProcessingWorker() { m_pOwner=NULL; 
							  m_SharedFileListLock = NULL; 
							  md4clr (m_fileHashToProcess); }
	void							UnlockSharedFilesList ();
	virtual void					SetFileHashToProcess (const uchar* _fileHash);
	virtual const uchar*			GetFileHashToProcess () { return m_fileHashToProcess; }
	virtual void					SetFilePath (CString _s) { m_FilePath = _s; }
	virtual CString					GetFilePath () { return m_FilePath; }
	void							SetOwner (CFileProcessingThread* _owner) { m_pOwner=_owner; }
	virtual CFileProcessingThread*	GetOwner () { return m_pOwner; }
	virtual ~CFileProcessingWorker () { UnlockSharedFilesList(); }
	
	// The following abstract function is called by the main implementation of the 
	// function Run() of the CFileProcessingThread-object for each file pointer 
	// in order to do the work.
	// When the function wants to access the CKnownFile object itself, it has to 
	// call ValidateKnownFile to get a valid file pointer.
	// After being completed accessing the object it must unlock the SharedFileList
	// again by calling UnlockSharedFileList !
	// The file list MUST BE UNLOCKED AS SOON AS POSSIBLE so that the user don't has
	// to wait for the thread. The thread should:
	// 1.) Get a file pointer
	// 2.) Extract all necessary data
	// 3.) Unlock the list
	// 4.) Process the file
	// 5.) Lock the file again
	// 6.) Write the results to the object
	// 7.) Unlock the list at last
	// The function should check m_pOwner->IsTerminating sometimes to
	// stop the work if the thread has to terminate.
	virtual void Run () {};
};

// Mighty Knife: Processing thread for known files, i.e. for CRC32 calculation.
// The thread must not be started upon creation. First the creator has to store
// the hash-id's of all files which the thread should process.
// After the thread has been launched the caller must not access the list anymore
// to prevent program errors until the thread has terminated.
class CFileProcessingThread : public CWinThread
{
	DECLARE_DYNCREATE(CFileProcessingThread)
protected:
	// The list of worker objects
	CTypedPtrList<CPtrList, CFileProcessingWorker*> fileWorkers;

	bool m_IsTerminating;
	bool m_IsRunning;
	CMutex m_FilelistLocked;
public:
	CFileProcessingThread()	{ m_IsTerminating = false;
							  m_IsRunning = false;	
							  // the thread object must not be destroyed because it should
							  // be able to be restarted
							  m_bAutoDelete = false; 	}

	virtual	BOOL	InitInstance() { return true; }
	virtual int		Run();

	// Shows if the thread is going to be terminated.
	// The worker thread should check this from time to time to stop processing
	// if needed.
	virtual bool	IsTerminating() { return m_IsTerminating; }

	// A way to terminate the thread. The thread is not terminated directly but a flag
	// is set which tells the worker threads to terminate as soon as possible.
	virtual void	Terminate() { m_IsTerminating = true; }

	// A way for the application to check if the thread is currently running.
	// This is not totally save, but save enough...
	virtual bool    IsRunning() const { return m_IsRunning; }

	virtual void    AddFileProcessingWorker (CFileProcessingWorker* _worker);
};
