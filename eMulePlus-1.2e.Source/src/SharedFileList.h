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
#pragma once

#include "mapkey.h"
#ifndef NEW_SOCKETS_ENGINE
	#include "LanCast.h"
#endif //NEW_SOCKETS_ENGINE
#include "Loggable.h"

struct UnknownFile_Struct
{
	CString		m_strFileName;
	CString		m_strDirectory;
};

class CServer;
class CPartFile;
class CKnownFile;
class CPreferences;
class CServerConnect;
class CKnownFileList;
class CSharedFilesCtrl;

class CSharedFileList : public CLoggable
{
	friend class CSharedFilesCtrl;
	friend class CClientReqSocket;
	friend class CWebServer;

public:
	CMutex				m_mutexList;
	CKnownFileList	   *m_pKnownFileList;

protected:
	CMutex				m_HashMapMutex;

private:
	CPreferences	   *m_pPrefs;
#ifndef NEW_SOCKETS_ENGINE
	CServerConnect	   *m_pServerConnect;
	CSharedFilesCtrl   *m_pOutput;
	CLanCast		   m_LanCast;
#endif //NEW_SOCKETS_ENGINE
	CMapStringToString	m_SharedVDirForList;
	CTypedPtrList<CPtrList, UnknownFile_Struct*>	m_waitingForHashList;
	UnknownFile_Struct	*m_pCurrentlyHashing;
	uint32				m_dwLastPublishED2KTime;
	bool				m_bLastPublishED2KFlag;

public:
#ifndef NEW_SOCKETS_ENGINE
					CSharedFileList(CPreferences *in_prefs, CServerConnect *in_server, CKnownFileList *in_filelist);
#else
					CSharedFileList(CPreferences* in_prefs, CKnownFileList* in_filelist);
#endif //NEW_SOCKETS_ENGINE
				   ~CSharedFileList();
	void			SendListToServer();
	void			Reload();
	void			SafeAddKnownFile(CKnownFile* toadd, bool bOnlyAdd = false, bool bDelay = false);
	void			RepublishFile(CKnownFile *pFile, int iMode);
	void			SetOutputCtrl(CSharedFilesCtrl* in_ctrl);
	void			RemoveFile(CKnownFile* toremove);
	CKnownFile*		GetFileByID(const uchar* filehash);
	void			WriteToOfferedFilePacket(CKnownFile *pKFile, CMemFile &files, CServer *pServer, CUpDownClient *pClient = NULL);
	uint64			GetDatasize(uint64 *pqwLargest);
	uint16			GetCount()	{ return static_cast<uint16>(m_mapSharedFiles.GetCount()); }
	void			UpdateItem(CKnownFile *pKnownFile, bool bResort = true);
	void			NextLANBroadcast();
	void			FindSharedFiles();
	void			ClearED2KPublishInfo();
	void			Process();
	uint32			GetWaitingForHashCount()			{ return m_waitingForHashList.GetCount(); }
	void			FileHashingFailed(UnknownFile_Struct *pHashed);
	void			FileHashingFinished(CKnownFile *pKnownfile);

	CMapStringToString&		GetSharedVDirForList()		{ return m_SharedVDirForList; }

private:
	void			AddFilesFromDirectory(const CString& strDirectory);
	bool			AddFile(CKnownFile *pKnownFile, const TCHAR *pcFullName);
	void			HashNextFile();
	bool			IsHashing(const CString &strDirectory, const CString &strFileName);
	void			RemoveFromHashing(CKnownFile *pHashed);

//	virtual dir for Hybrid client to client shared files listing
	void			BuildSharedVDirForList(CStringList *sharedDirList);
	bool			AddSharedVDirForList(bool bIncomplete, const CString &strDir);

#ifdef NEW_SOCKETS
public:
#endif //NEW_SOCKETS
	typedef CMap<CCKey, const CCKey&, CKnownFile*, CKnownFile*>	CSharedFilesMap;
	CSharedFilesMap	m_mapSharedFiles;
	CCKey	m_LancastKey;
};

class CHashFileThread : public CWinThread
{
	DECLARE_DYNCREATE(CHashFileThread)

private:
	CString				m_strDirectory;
	CString				m_strFileName;
	CPartFile		   *m_pPartFile;
	bool				m_bChangeState;

public:
	virtual	BOOL		InitInstance();
	virtual int			Run();
	void				SetValues(bool bChgState, LPCTSTR strDirectory, LPCTSTR strFileName, CPartFile *pPartFile);

protected:
						CHashFileThread();
};
