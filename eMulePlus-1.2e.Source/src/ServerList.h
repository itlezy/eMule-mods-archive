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

#include "ServerListCtrl.h"
#include "WebServer.h"
#include "Loggable.h"

#define CFGFILE_STATICSERVERS		_T("staticservers.dat")

class CServer;

class CServerList : public CLoggable
{
	friend class	CServerListCtrl;
	friend class	CWebServer;

public:
	CServerList();
	~CServerList(void);
	bool	Init();
	bool	AddServer(CServer *pServer, bool bChangeServerInfo = false);
	void	RemoveServer(CServer *pServer);
	bool	AddServerMetToList(const CString &strFile, bool bMergeWithPrevList = true);
	void	LoadServersFromTextFile(const CString &strFilename);
	bool	SaveServersToTextFile();
	bool	SaveServerMetToFile();
	void	ServerStats();
	void	ResetServerPos()			{ m_dwServerPos = 0; }
	void	ResetSearchServerPos()		{ searchserverpos = 0; }
	CServer* GetNextServer(bool bOnlyObfuscated);
	CServer* GetSuccServer(const CServer *pLastSrv) const;
	CServer *GetNextSearchServer();
	CServer *GetNextStatServer();
	CServer *GetServerAt(uint32 pos)	{ return(m_serverList.GetAt(m_serverList.FindIndex(pos))); }
	uint32	GetServerCount()			{ return(m_serverList.GetCount()); }
	CServer *FindServerByAddress(const CServer& in_server, bool bCheckPort = true);
	CServer *GetServerByAddress(const CString &address, uint16 port);
	CServer *GetServerByIP(uint32 nIP, uint16 port = 0);
	bool	IsGoodServerIP(CServer *in_server);
	void	GetServersStatus(uint32 &total, uint32 &failed, uint32 &user, uint32 &file,
				uint32 &dwLowIdUsers, uint32 &tuser, uint32 &tfile, double &occ);
	void	GetUserFileStatus(uint32 &user, uint32 &file);
	void	Sort();
	void	MoveServerDown(CServer *aServer);
	uint32	GetServerPosition() const	{ return m_dwServerPos; }
	void	SetServerPosition(uint32 dwNewPosition)
	{
		m_dwServerPos = (dwNewPosition < (uint32)m_serverList.GetCount()) ? dwNewPosition : 0;
	}

	uint32	GetDeletedServerCount() const	{ return m_dwDelSrvCnt; }
	void	Process();
	void	AutoUpdate();
	void	ResetIP2Country();
	void	CheckForExpiredUDPKeys();

private:
	uint32								m_dwServerPos;
	uint32								searchserverpos;
	uint32								statserverpos;
	CTypedPtrList<CPtrList, CServer *>	m_serverList;
	uint32								m_dwDelSrvCnt;
	uint32								m_nLastSaved;
	CCriticalSection					m_csServerMetFile;
	bool								m_bListLoaded;
};
