//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->

#include "EMSocket.h"
#include "VoodooOpcodes.h"

class CSafeMemFile;
class CUpDownClient;
enum EDebugLogPriority;
// NEO: VOODOOs - [VoodooSearchForwarding]
struct SSearchParams;
enum ESearchType;
// NEO: VOODOOs END
enum EFilePrefsLevel; // NEO: VOODOOn - [VoodooForNeo]
struct PartFileBufferedData;
struct Gap_Struct;

#include "VoodooSocket.h"

class CVoodooClient : public CObject
{
	DECLARE_DYNAMIC(CVoodooClient)

	CVoodooClient(){
		socket = NULL;
		m_uFails = 0;
		m_uLost = 0;
	}
	~CVoodooClient(){
		ASSERT(socket == NULL);
	}

public:
	void	SetName(LPCTSTR sName) {m_sName = sName;}
	const CString& GetName() const {return m_sName;}

	void	SetAddress(LPCTSTR Address) { if(Address) clientAddress = Address;}
	const CString& GetAddress() const {return clientAddress;}
	void	SetIP(DWORD dwIP) {clientIP = dwIP;}
	DWORD	GetIP()	const {return clientIP;}
	void	SetPort(uint16 uPort) {clientPort = uPort;}
	uint16	GetPort() const {return clientPort;}
	void	SetAction(uint8 uAction) {m_uAction = uAction;}
	uint8	GetAction() const {return m_uAction;}
	void	SetType(uint8 uType) {m_uType = uType;}
	uint8	GetType() const {return m_uType;}
	void	SetPerm(uint8 uPerm) {m_uPerm = uPerm;}
	uint8	GetPerm() const {return m_uPerm;}
	void	SetSpell(LPCTSTR sSpell) {m_sSpell = sSpell;}
	const CString& GetSpell() const {return m_sSpell;}

	CVoodooSocket* socket;

protected:
	friend class CVoodoo;

	void	Attach(CVoodooSocket* Socket){
		clientIP = Socket->clientIP;
		clientPort = Socket->clientPort;
		socket = Socket;
		m_uPerm = Socket->m_uPerm;
		m_sSpell = Socket->m_sSpell;
		m_sName = Socket->m_sName;
		m_uType = Socket->m_uType;
		m_uFails = 0;
		m_uLost = 0;
	}

	CString m_sName;

	uint8	m_uAction;

	CString	clientAddress;
	DWORD	clientIP;
	uint16	clientPort;

	CString m_sSpell;

	uint8	m_uType;
	uint8	m_uPerm;

	uint16	m_uFails;
	uint32	m_uLost;
};

#define STAT_REFRESH SEC2MS(1)

//////////////////////
// CVoodoo

class CVoodoo : public CAsyncSocketEx
{
	friend class CVoodooSocket;
	friend class CVoodooListDlg;

public:
	CVoodoo();
	virtual ~CVoodoo();

	void	ConnectVoodooClient(LPCTSTR lpszHostAddress, uint16 nHostPort, uint8 uAction = VA_SLAVE);
	void	ConnectVoodooClient(DWORD nHostAddress, uint16 nHostPort, uint8 uAction = VA_SLAVE, LPCTSTR lpszHostAddress = NULL);

	void	ManifestGapList(CPartFile* pFile);
	void	ManifestDownloadOrder(CPartFile* pFile);
	void	ManifestDownloadInstruction(CPartFile* pFile, uint8 uInstruction, uint32 Flag1 = NULL, uint32 Flag2 = NULL);
	void	ManifestShareInstruction(CKnownFile* kFile, uint8 uInstruction, uint32 Flag1 = NULL, uint32 Flag2 = NULL); 
	// NEO: VOODOOs - [VoodooSearchForwarding]
	// Search forwarding
	void	ManifestNewSearch(SSearchParams* pParams);
	void	ManifestSearchCommand(DWORD dwSearchID, uint8 uCommand);
	// NEO: VOODOOs END
	void	ManifestThrottleBlock(CPartFile* pFile, uint64 start, uint64 end, bool bRelease = false, CVoodooSocket* source = NULL);
	void	ManifestCorruptedSenderWarning(DWORD dwIP);
	// NEO: VOODOOx - [VoodooSourceExchange]
	void	ManifestSingleSource(CPartFile* pFile, CUpDownClient* sClient);
	void	ManifestSourceListRequest(CPartFile* pFile); 
	// NEO: VOODOOx END
	// NEO: VOODOOn - [VoodooForNeo]
	void	ManifestNeoPreferences(EFilePrefsLevel Kind, CKnownFile* kFile = NULL, int Cat = -1);
	void	ManifestDownloadCommand(CTypedPtrList<CPtrList, CPartFile*>& FileQueue, uint8 uCommand, uint32 Flag1 = NULL, uint32 Flag2 = NULL);
	// NEO: VOODOOn END

	// NEO: NLC - [NeoLanCast]
	bool	IsVoodooClientEd2k(DWORD dwIP, uint16 uPort);
	bool	IsVoodooClient(DWORD dwIP, uint16 uPort);
	void	AddVoodooClient(DWORD dwIP, uint16 uPort, uint8 uAction = VA_QUERY);
	// NEO: NLC - [NeoLanCast]
	bool	IsValidKnownClinet(CVoodooClient* VoodooClient);

	bool	Start();
	void	Stop();
	bool	StartListening();
	bool	IsListening() {return bListening;}
	bool	IsStarted() {return bStarted;}

	virtual void OnAccept(int nErrorCode);
	bool    IsValidSocket(CVoodooSocket* totest);
	void	Process();
	bool	Rebind();

	void	TryToConnect();

	// voodoo search forwarding
	CMap<DWORD,DWORD,SSearchMaster,SSearchMaster> m_SearchMasterMap; // NEO: VOODOOs - [VoodooSearchForwarding]

	// stats
	uint32	GetUpDatarate();
	uint32	GetDownDatarate();
	//uint32	GetUpDatarate(CKnownFile* File);
	uint32	GetDownDatarate(CKnownFile* File);

protected:
	void	RemoveSocket(CVoodooSocket* todel);
	void	AddSocket(CVoodooSocket* toadd);
	void	KillAllSockets();

	void	ClearKnown();
	CVoodooClient* GetClientByAddress(LPCTSTR address, uint16 port);
	CVoodooClient* GetClientByIP(uint32 nIP, uint16 nPort, bool bResolve = false, CVoodooSocket* Socket = NULL);
	bool	AddKnown(CVoodooSocket* VoodooClient);
	void	CleanUpKnown(CVoodooSocket* VoodooClient);

	void	LoadKnownFile();
	void	SaveKnownToFile();

	CTypedPtrList<CPtrList, CVoodooSocket*> socket_list;
	CTypedPtrList<CPtrList, CVoodooClient*> known_list;

private:
	bool	bListening;
	bool	bStarted;
	uint32	uLastSearch;

	uint16  m_port;
};

#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
