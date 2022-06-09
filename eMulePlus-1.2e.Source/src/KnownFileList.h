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

#include "KnownFile.h"
#ifndef NEW_SOCKETS_ENGINE
#include "emule.h"
#endif //NEW_SOCKETS_ENGINE
#include "Loggable.h"

class CKnownFileList : public CArray<CKnownFile*, CKnownFile*>, public CLoggable
{
	friend class CSharedFilesWnd;
	friend class CFileStatistic;
public:
	CMutex		m_mutexList;

	CKnownFileList(const CString& in_appdir);
	~CKnownFileList();
	void	SafeAddKnownFile(CKnownFile* toadd);
	bool	Init();
	bool Load();
	void	Save();
	void	Clear();
	CKnownFile*	FindKnownFile(const CString &strFileName, uint32 dwDate, uint64 qwSize);
	CKnownFile* FindKnownFileByID(const uchar* hash);	//SyruS show completed files (0.28b)
	uint32	GetTotalRequested() {return m_iNumRequested;}
	bool merge();
	bool RemoveFile(CKnownFile* toRemove);

private:	
	bool SavePartTraffic();
	bool LoadPartTraffic();
	bool SavePartPrio();
	bool LoadPartPrio();
	CString m_strFilesPath;	
	CString m_strTrafficPath;	
	CString m_strPartPermissionsPath;

	uint64	m_qwNumTransferred;
	uint16	m_iNumRequested;
	uint16	m_iNumAccepted;
	bool	m_bInSave;
};
