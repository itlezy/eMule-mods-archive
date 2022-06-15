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

#include "MapKey.h"
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

//////////////////////////
// Hierarchy Exception

class CHierarchyExceptio : public CException
{
	DECLARE_DYNAMIC(CHierarchyExceptio)
public:
	CHierarchyExceptio(uint8 uOpCode, uint8 uTagCode = 0)
	{
		m_uOpCode = uOpCode;
		m_uTagCode = uTagCode;
	}

	uint8 m_uOpCode;
	uint8 m_uTagCode;
};

/////////////////////////
// Structs

#pragma pack(1)
struct FileID{
	FileID() {Size = 0;}
	FileID(CSafeMemFile &packet, bool b64);
	uchar	Hash[16];
	uint64	Size;
	bool operator!=(FileID& ID)	{return Size != ID.Size || md4cmp(Hash,ID.Hash);}
};
#pragma pack()

bool WriteFileID(CSafeMemFile &packet, CKnownFile* file, bool b64);
void WriteFileID(CSafeMemFile &packet, FileID& file, bool b64);

#pragma pack(1)
struct sDataRequest{
	uint32	IdKey;
	uint64	StartOffset;
	uint64	EndOffset;
	// NEO: RBT - [ReadBlockThread]
	uint32	togo;
	FileID	ID;
	uint32	Client;
	// NEO: RBT END
};
#pragma pack()

// NO RBT BEGIN
//#pragma pack(1)
//struct sDataBuffer{
//	byte*	Data;
//	uint32	Size;
//	uint32	Time;
//};
//#pragma pack()
// NO RBT END

// remember errors
#pragma pack(1)
struct sFileError{
	sFileError(FileID id, uint8 err){
		ID = id;
		Error = err;
	}
	FileID	ID;
	uint8	Error;
};
#pragma pack()

// NEO: VOODOOs - [VoodooSearchForwarding]
class CVoodooSocket;
#pragma pack(1)
struct SSearchMaster{
	CVoodooSocket*	MasterSocket;
	DWORD			MasterID;
	DWORD			KillTimer;
};
#pragma pack()
// NEO: VOODOOs END

// stats
#pragma pack(1)
struct SVoodooStats{
	SVoodooStats(){
		uLastStatUpdate = 0;
		uUpDatarate = 0;
		uDownDatarate = 0;
	}
	uint32 uLastStatUpdate;
	uint32 uUpDatarate;
	uint32 uDownDatarate;
};
typedef SVoodooStats SFileVoodooStats;
#pragma pack()


//////////////////////
// CMasterDatas

class CMasterDatas
{
public:
	CMasterDatas();
	~CMasterDatas();

	bool	HaveGapList(){ return gaplist != NULL; }
	bool	IsComplete(uint64 start, uint64 end) const;

	CTypedPtrList<CPtrList, Gap_Struct*>* gaplist;
};


//////////////////////
// CVoodooSocket

class CVoodooSocket : public CEMSocket
{
	friend class CVoodoo;
	friend class CVoodooClient;
	friend class CVoodooDetailPage;
	DECLARE_DYNCREATE(CVoodooSocket)

public:
	CVoodooSocket();
	virtual ~CVoodooSocket();

	void	Disconnect(LPCTSTR pszReason);

	void	SendGoodBy(bool bFhash = false);
	void	SendPing(uint8 uPong = FALSE);
	virtual void SendPacket(CSafeMemFile &data, uint8 opcode, bool bControlPacket = true);

	void	SetAddress(LPCTSTR Address) { if(Address) clientAddress = Address;}
	CString& GetAddress()	{return clientAddress;}
	void	SetIP(DWORD dwIP) {clientIP = dwIP;}
	DWORD	GetIP()			{return clientIP;}
	void	SetPort(uint16 uPort) {clientPort = uPort;}
	uint16	GetPort()		{return clientPort;}

	CString& GetName() {return m_sName;}

	// NEO: NLC - [NeoLanCast]
	bool	IsLanSocket() { return true; }
	void	SetOnLan();
	// NEO: NLC END
	void	ConfigSocket();

	bool	IsVoodooSocket() { return true; }
	
	// voodoo members
	bool	IsMaster() {return m_bIsMaster;}
	bool	IsSlave() {return m_bIsSlave;}

	// DownLink
	void	SplitPart(CPartFile* pFile, PartFileBufferedData *item);
	void	TransferFileData(CPartFile* pFile, PartFileBufferedData *item, DWORD dwIP);
	void	SendThrottleBlock(CPartFile* pFile, uint64 start, uint64 end, bool bRelease);

	void	SendCorruptedSenderWarning(DWORD dwIP);

	// UpLink
	void	RequestFileData(CKnownFile* kFile, sDataRequest &DataRequest);
	void	SendFileData(sDataRequest* DataRequest, byte* Data); // NEO: RBT - [ReadBlockThread]
	//sDataBuffer* GetBufferedUpload(CKnownFile* kFile, sDataRequest &DataRequest); // NO RBT

	// Voodoo Protocol details
	uint8	GetVoodooVersion()				{return m_uVoodooVersion;}

	// general cleint abilitier
	bool	Use64Size()						{return m_u64FileSizeSupport;}
	uint8	GetCorruptionHandlingVersion()	{return m_uCorruptionHandling;}
	uint8	GetAdvDownloadSyncVersion()		{return m_uAdvDownloadSync;}
	uint8	GetSupportsStatisticsVersion()	{return m_uSupportsStatistics;}

	// ed2k specific cleint abilities
	uint8	GetVoodooSearchVersion()		{if(m_uType != CT_ED2K) return 0; return m_uVoodooSearch;} // NEO: VOODOOs - [VoodooSearchForwarding]
	uint8	GetVoodooXSVersion()			{if(m_uType != CT_ED2K) return 0; return m_uVoodooXS ;} // NEO: VOODOOx - [VoodooSourceExchange]
	uint8	GetNeoFilePrefsVersion()		{if(m_uType != CT_ED2K) return 0; return m_uNeoFilePrefs;} // NEO: VOODOOn - [VoodooForNeo]
	uint8	GetNeoCommandVersion()			{if(m_uType != CT_ED2K) return 0; return m_uNeoCommandVersion;}  // NEO: VOODOOn - [VoodooForNeo]

	// stats
	uint32	GetUpDatarate()		{return m_Stats.uUpDatarate;}
	uint32	GetDownDatarate()	{return m_Stats.uDownDatarate;}

	// NEO: VOODOOs - [VoodooSearchForwarding]
	// voodoo search forwarding
	void	SendSearchState(DWORD dwSearchID, uint8 uState, uint32 uValue = 0);
	void	SendSearchCommand(DWORD dwSearchID, uint8 uCommand);
	void	SendSearchResult(DWORD dwSearchID,ESearchType eType,CSafeMemFile &packet, uint32 nServerIP = 0, uint16 nServerPort = 0);

	bool	m_bHaveMoreResults;
	// NEO: VOODOOs END

protected:
	// Socket members
	bool	CheckTimeOut();
	void	ResetTimeOutTimer();
	void	Delete_Timed();
	virtual void Safe_Delete();
	void	Process();

	virtual void OnConnect(int nErrorCode);
	void		 OnClose(int nErrorCode);
	void		 OnSend(int nErrorCode);
	void		 OnReceive(int nErrorCode);
	void		 OnError(int nErrorCode);

	virtual bool PacketReceived(Packet* packet);

	// Voodoo members
	bool	IsUsable() {return (m_uPerm & VA_SLAVE) != 0;}
	CString& GetSpell() {return m_sSpell;}
	bool	IsED2K() {return m_uType == CT_ED2K;}

	void	SetAction(uint8 uAction) {m_uAction = uAction;}
	uint8	GetAction() {if(m_uAction == VA_QUERY) return VA_NONE; return m_uAction;}

	uint8	GetType() {return m_uType;}

	uint16	GetED2KPort()	{return m_nPortED2K;}

	void	SendVoodooHello(uint8 uHello = VH_HELLO);
	void	RecivedVoodooHello(CSafeMemFile &packet);
	void	ConnectionEstablished();

	void	RecivedPing(CSafeMemFile &packet);

	void	PrepDisconnect();
	void	DetachFiles();

	void	SendSpell();
	void	SpellRecived(CSafeMemFile &packet);
	void	SendSpellResult(uint8 uResult);
	void	SpellResultRecived(CSafeMemFile &packet);

	void	SendDownloadQueue();
	void	SynchronizeDownloadQueue(CSafeMemFile &packet);
	void	SendDownloadQueueV2();

	void	SendDownloadOrder(CPartFile* pFile);
	void	DownloadOrderRecived(CSafeMemFile &packet);

	void	SendShareOrder(CKnownFile* kFile);
	void	ShareOrderRecived(CSafeMemFile &packet);

	void	SendFileUnavalible(FileID &newID, uint8 uErr);
	void	FileUnavalibleRecived(CSafeMemFile &packet);
	bool	IsFileErr(CKnownFile* kFile);

	void	RequestGapList(CPartFile* pFile);
	void	GapListRequestRecived(CSafeMemFile &packet);
	void	SendGapList(CPartFile* pFile);
	void	GapListRecived(CSafeMemFile &packet);

	void	SendSharedFilesList();
	void	SynchronizeSharedFileList(CSafeMemFile &packet);
	void	SendSharedFilesListV2();

	void	SendDownloadInstruction(CPartFile* pFile, uint8 uInstruction, uint32 Flag1 = NULL, uint32 Flag2 = NULL);
	void	DownloadInstructionRecived(CSafeMemFile &packet);

	void	SendShareInstruction(CKnownFile* kFile, uint8 uInstruction, uint32 Flag1 = NULL, uint32 Flag2 = NULL);
	void	ShareInstructionRecived(CSafeMemFile &packet);

	void	SendHashSetRequest(CPartFile* pFile);
	void	HashSetRequestRecived(CSafeMemFile &packet);
	void	SendHashSetResponde(CKnownFile* kFile);
	void	HashSetRespondeRecived(CSafeMemFile &packet);

	// stats
	void	SendStatistics();
	void	SendFileStatistics(CMap<CKnownFile*,CKnownFile*,SFileVoodooStats,SFileVoodooStats> &FileStats);
	void	StatisticsRecived(CSafeMemFile &packet);
	void	FileStatisticsRecived(CSafeMemFile &packet);

	// NEO: VOODOOs - [VoodooSearchForwarding]
	// voodoo search forwarding
	void	SendNewSearch(SSearchParams* pParams);
	void	NewSearchRecived(CSafeMemFile &packet);
	void	SearchStateRecived(CSafeMemFile &packet);
	void	SearchCommandRecived(CSafeMemFile &packet);
	void	SearchResultRecived(CSafeMemFile &packet);
	// NEO: VOODOOs END

	// NEO: VOODOOx - [VoodooSourceExchange]
	// Source exchange
	void	RequestSourceList(CPartFile* pFile);
	void	SourceListRequestRecived(CSafeMemFile &packet);

	void	SendSingleSource(CPartFile* pFile, CUpDownClient* sClient);
	void	SendSourceList(CPartFile* pFile);
	void	SourceListRecived(CSafeMemFile &packet);
	// NEO: VOODOOx END

	// NEO: VOODOOn - [VoodooForNeo]
	void	SendNeoPreferences(EFilePrefsLevel Kind, CKnownFile* kFile = NULL, int Cat = -1);
	void	NeoPreferencesRecived(CSafeMemFile &packet);

	void	SendDownloadCommand(CTypedPtrList<CPtrList, CPartFile*>& ServerReqQueue, uint8 uCommand, uint32 Flag1 = NULL, uint32 Flag2 = NULL);
	void	DownloadCommandRecived(CSafeMemFile &packet);
	// NEO: VOODOOn END

	// packet processor
	bool	ProcessVoodooPacket(const BYTE* packet, uint32 size, UINT opcode);

	// Debug Members
	void	PacketToDebugLogLine(LPCTSTR protocol, const uchar* packet, uint32 size, UINT opcode);
	CString DbgGetClientInfo();
	static CString GetClientDesc(uint8 uAction);
	CString GetClientDesc();

	// stats
	SVoodooStats m_Stats;
	CMap<CKnownFile*,CKnownFile*,SFileVoodooStats,SFileVoodooStats> m_FileStats;
	CMap<CCKey,const CCKey&,sFileError*,sFileError*> m_FileErrors;

private:
	CPartFile*	GetDownloadingFile(CSafeMemFile &packet, bool bRequest = true, bool bLogUnknown = true);
	CKnownFile*	GetSharedFile(CSafeMemFile &packet);

	void	ExecuteDownloadInstruction(CPartFile* pFile, uint8 uInstruction, uint32 Flag1 = NULL, uint32 Flag2 = NULL);

	// DownLink
	void	RecivedFileData(CSafeMemFile &packet);
	void	RecivedThrottleBlock(CSafeMemFile &packet);

	void	RecivedCorruptedSenderWarning(CSafeMemFile &packet);

	// UpLink
	void	FileDataRequestRecived(CSafeMemFile &packet);
	void	GetFileData(CKnownFile* kFile, sDataRequest* DataRequest); // NEO: RBT - [ReadBlockThread]
	//sDataBuffer* GetFileData(CKnownFile* kFile, sDataRequest &DataRequest); // NO RBT
	void	FileDataAnswerRecived(CSafeMemFile &packet);

	// socket properitys
	uint32	timeout_timer;
	bool	deletethis;
	uint32	deltimer;
	CString	clientAddress;
	DWORD	clientIP;
	uint16	clientPort;

	// voodoo status
	uint8	m_uAction;
	bool	m_bIsMaster;
	bool	m_bIsSlave;

	// voodoo version and supported features
	uint8	m_uVoodooVersion;
	UINT	m_u64FileSizeSupport	: 1, // Large File support
			m_uCorruptionHandling	: 4, // corruption handling
			m_uAdvDownloadSync		: 4, // prevent 2 nodes from downloading the same part at one time
			m_uSupportsStatistics	: 3; // transfer stats version 1 global, version 2 also per file stats
	UINT	m_uNeoFilePrefs			: 3, // neo file preferences // NEO: VOODOOn - [VoodooForNeo]
			m_uVoodooXS				: 3, // voodoo source exchange // NEO: VOODOOx - [VoodooSourceExchange]
			m_uVoodooSearch			: 3, // voodoo search forwarding 	// NEO: VOODOOs - [VoodooSearchForwarding]
			m_uNeoCommandVersion	: 3; // voodo for neo commands  // NEO: VOODOOn - [VoodooForNeo]


	// voodoo properties
	CString m_sName;
	uint16	m_nVoodoPort;
	uint8	m_uType;
	uint8	m_uPerm;
	CString m_sSpell;

	uint16	m_nPortED2K;

	// stats
	uint32	m_uLastStatUpdate;

	// NO RBT BEGIN
	//CMap<uint32,uint32,sDataBuffer*,sDataBuffer*> m_UploadBuffer;
	//CMap<uint32,uint32,uint32,uint32> m_DataRequested;
	//void	CleanUpUploadBuffer(bool bClear = false);
	// NO RBT END

	FileID	m_LastErrFile; // we may send a operation request directly it must be ignorred or we will request the file again, and the whole storry repeads iself endles for ever
};

#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
