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
#include "StdAfx.h"
#include "deadsourcelist.h"
#include "preferences.h"
#include "opcodes.h"
#include "updownclient.h"
#include "partfile.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	CLEANUPTIME			MIN2MS(30) //Xman changed from 60

//Xman increased:
//#define BLOCKTIME		(::GetTickCount() + (m_bGlobalList ? MIN2MS(15):MIN2MS(45)))
//#define BLOCKTIMEFW		(::GetTickCount() + (m_bGlobalList ? MIN2MS(30):MIN2MS(45)))
#define BLOCKTIME		(::GetTickCount() + (m_bGlobalList ? MIN2MS(32):MIN2MS(60)))
#define BLOCKTIMEFW		(::GetTickCount() + (m_bGlobalList ? MIN2MS(40):MIN2MS(60)))

///////////////////////////////////////////////////////////////////////////////////////
//// CDeadSource

CDeadSource::CDeadSource(uint32 dwID, uint16 nPort, uint32 dwServerIP, uint16 nKadPort){
	m_dwID = dwID;
	m_dwServerIP = dwServerIP;
	m_nPort = nPort;
	m_nKadPort = nKadPort; 
	md4clr(m_aucHash);
}

CDeadSource::CDeadSource(const uchar* paucHash){
	m_dwID = 0;
	m_dwServerIP = 0;
	m_nPort = 0;
	m_nKadPort = 0;
	md4cpy(m_aucHash, paucHash);

}

bool operator==(const CDeadSource& ds1,const CDeadSource& ds2){
	//ASSERT( ((ds1.m_dwID + ds1.m_dwServerIP) ^ isnulmd4(ds1.m_aucHash)) != 0 );
	//ASSERT( ((ds2.m_dwID + ds2.m_dwServerIP) ^ isnulmd4(ds2.m_aucHash)) != 0 );
	return (
		// lowid ed2k and highid kad + ed2k check
		( (ds1.m_dwID != 0 && ds1.m_dwID == ds2.m_dwID) && ((ds1.m_nPort != 0 && ds1.m_nPort == ds2.m_nPort) || (ds1.m_nKadPort != 0 && ds1.m_nKadPort == ds2.m_nKadPort)) && (ds1.m_dwServerIP == ds2.m_dwServerIP || !IsLowID(ds1.m_dwID)) )
		// lowid kad check
		|| ( IsLowID(ds1.m_dwID) && isnulmd4(ds1.m_aucHash) == FALSE && md4cmp(ds1.m_aucHash, ds2.m_aucHash) == 0) );
}

CDeadSource& CDeadSource::operator=(const CDeadSource& ds){
	m_dwID = ds.m_dwID;
	m_dwServerIP = ds.m_dwServerIP;
	m_nPort = ds.m_nPort;
	m_nKadPort = ds.m_nKadPort;
	md4cpy(m_aucHash, ds.m_aucHash);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
//// CDeadSourceList

CDeadSourceList::CDeadSourceList()
{
	m_dwLastCleanUp = 0;
}

CDeadSourceList::~CDeadSourceList()
{
}

void CDeadSourceList::Init(bool bGlobalList){
	m_dwLastCleanUp = ::GetTickCount();
#ifdef REPLACE_ATLMAP
	CDeadSourceMap(bGlobalList?3001:503).swap(m_mapDeadSources);
#else
	m_mapDeadSources.InitHashTable(bGlobalList?3001:503);
#endif
	m_bGlobalList = bGlobalList;
}

bool CDeadSourceList::IsDeadSource(const CUpDownClient* pToCheck) const{
	//bool bDbgCheck = false;
	if(!pToCheck->HasLowID() || pToCheck->GetServerIP() != 0){
#ifdef REPLACE_ATLMAP
		CDeadSourceMap::const_iterator it = m_mapDeadSources.find(CDeadSource(pToCheck->GetUserIDHybrid(), pToCheck->GetUserPort(), pToCheck->GetServerIP(), pToCheck->GetKadPort()));
		if(it != m_mapDeadSources.end() && it->second > ::GetTickCount())
			return true;
#else
		uint32 dwExpTime;
		if (m_mapDeadSources.Lookup(CDeadSource(pToCheck->GetUserIDHybrid(), pToCheck->GetUserPort(), pToCheck->GetServerIP(), pToCheck->GetKadPort()), dwExpTime)){
			if (dwExpTime > ::GetTickCount())
				return true;
		}
#endif
		//bDbgCheck = true;
	}
	//Xman possible crashfix:
	//if (((pToCheck->HasValidBuddyID() || pToCheck->SupportsDirectUDPCallback()) && isnulmd4(pToCheck->GetUserHash()) == FALSE) || (pToCheck->HasLowID() && pToCheck->GetServerIP() == 0) ){
	if (isnulmd4(pToCheck->GetUserHash()) == FALSE && ((pToCheck->HasValidBuddyID() || pToCheck->SupportsDirectUDPCallback()) || (pToCheck->HasLowID() && pToCheck->GetServerIP() == 0)) ){
#ifdef REPLACE_ATLMAP
		CDeadSourceMap::const_iterator it = m_mapDeadSources.find(CDeadSource(pToCheck->GetUserHash()));
		if(it != m_mapDeadSources.end() && it->second > ::GetTickCount())
			return true;
#else
		uint32 dwExpTime;
		if (m_mapDeadSources.Lookup(CDeadSource(pToCheck->GetUserHash()), dwExpTime)){
			if (dwExpTime > ::GetTickCount())
				return true;
		}
#endif
		//bDbgCheck = true;
	}
	//ASSERT ( bDbgCheck );
	return false;
}

void CDeadSourceList::AddDeadSource(const CUpDownClient* pToAdd){
	//if (thePrefs.GetLogFilteredIPs())
	//	AddDebugLogLine(DLP_VERYLOW, false, _T("Added source to bad source list (%s) - file %s : %s")
	//	, m_bGlobalList? _T("Global"):_T("Local"), (pToAdd->GetRequestFile() != NULL)? pToAdd->GetRequestFile()->GetFileName() : _T("???"), pToAdd->DbgGetClientInfo() );

	if(!pToAdd->HasLowID())
#ifdef REPLACE_ATLMAP
		m_mapDeadSources[CDeadSource(pToAdd->GetUserIDHybrid(), pToAdd->GetUserPort(), pToAdd->GetServerIP(), pToAdd->GetKadPort())] = BLOCKTIME;
#else
		m_mapDeadSources.SetAt(CDeadSource(pToAdd->GetUserIDHybrid(), pToAdd->GetUserPort(), pToAdd->GetServerIP(), pToAdd->GetKadPort()), BLOCKTIME );
#endif
	else{
		//bool bDbgCheck = false;
		if(pToAdd->GetServerIP() != 0){
			//bDbgCheck = true;
#ifdef REPLACE_ATLMAP
			m_mapDeadSources[CDeadSource(pToAdd->GetUserIDHybrid(), pToAdd->GetUserPort(), pToAdd->GetServerIP(), 0)] = BLOCKTIMEFW;
#else
			m_mapDeadSources.SetAt(CDeadSource(pToAdd->GetUserIDHybrid(), pToAdd->GetUserPort(), pToAdd->GetServerIP(), 0), BLOCKTIMEFW);
#endif
		}
		if (pToAdd->HasValidBuddyID() || pToAdd->SupportsDirectUDPCallback()){
			//bDbgCheck = true;
#ifdef REPLACE_ATLMAP
			m_mapDeadSources[CDeadSource(pToAdd->GetUserHash())] = BLOCKTIMEFW;
#else
			m_mapDeadSources.SetAt(CDeadSource(pToAdd->GetUserHash()), BLOCKTIMEFW);
#endif
		}
		//ASSERT( bDbgCheck );
	}
	if (::GetTickCount() - m_dwLastCleanUp  > CLEANUPTIME)
		CleanUp();
}
/*
void CDeadSourceList::RemoveDeadSource(const CUpDownClient* client)
{
	if (!client->HasLowID())
#ifdef REPLACE_ATLMAP
		m_mapDeadSources.erase(CDeadSource(client->GetUserIDHybrid(), client->GetUserPort(), client->GetServerIP(), client->GetKadPort()));
#else
		m_mapDeadSources.RemoveKey(CDeadSource(client->GetUserIDHybrid(), client->GetUserPort(), client->GetServerIP(), client->GetKadPort()));
#endif
	else
	{
		if (client->GetServerIP() != 0)
#ifdef REPLACE_ATLMAP
			m_mapDeadSources.erase(CDeadSource(client->GetUserIDHybrid(), client->GetUserPort(), client->GetServerIP(), 0));
#else
			m_mapDeadSources.RemoveKey(CDeadSource(client->GetUserIDHybrid(), client->GetUserPort(), client->GetServerIP(), 0));
#endif
		if (client->HasValidBuddyID() || client->SupportsDirectUDPCallback())
#ifdef REPLACE_ATLMAP
			m_mapDeadSources.erase(CDeadSource(client->GetUserHash()));
#else
			m_mapDeadSources.RemoveKey(CDeadSource(client->GetUserHash()));
#endif
	}
}
*/
void CDeadSourceList::CleanUp(){
	m_dwLastCleanUp = ::GetTickCount();
	//if (thePrefs.GetLogFilteredIPs())
	//	AddDebugLogLine(DLP_VERYLOW, false, _T("Cleaning up DeadSourceList (%s), %i clients on List..."),  m_bGlobalList ? _T("Global") : _T("Local"), m_mapDeadSources.GetCount());
#ifdef REPLACE_ATLMAP
	for (CDeadSourceMap::const_iterator it = m_mapDeadSources.begin(); it != m_mapDeadSources.end();)
	{
		if(it->second < m_dwLastCleanUp)
			it = m_mapDeadSources.erase(it);
		else
			++it;
	}
#else
	POSITION pos = m_mapDeadSources.GetStartPosition();
	CDeadSource dsKey;
	uint32 dwExpTime;
	while (pos != NULL){
		m_mapDeadSources.GetNextAssoc( pos, dsKey, dwExpTime );
		if (dwExpTime < m_dwLastCleanUp){
			m_mapDeadSources.RemoveKey(dsKey);
		}
	}
#endif
	//if (thePrefs.GetLogFilteredIPs())
	//	AddDebugLogLine(DLP_VERYLOW, false, _T("...done, %i clients left on list"), m_mapDeadSources.GetCount());
}