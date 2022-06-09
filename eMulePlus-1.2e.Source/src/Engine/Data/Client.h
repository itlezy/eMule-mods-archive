// Client.h: interface for the CClient class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../../OtherFunctions.h"

enum EnumClientType
{
	CLIENT_MULE
};

struct CEmClient_Peer;

class CClient : public CLoggable2
{
public:
	CClient();
	virtual ~CClient();

	// Basics
	__declspec(property(get=_GetClientType)) EnumClientType Type;
	__declspec(property(get=_GetParent, put=_PutParent))	CEmClient_Peer*	Parent;
	__declspec(property(get=_GetClientAddr/*,put=_PutClientAddr*/))	in_addr	ClientAddr;
	__declspec(property(get=_GetClientPort, put=_PutClientPort))	USHORT	ClientPort;

	virtual EnumClientType _GetClientType() = 0;
	CEmClient_Peer* _GetParent() const { return m_pParent; }
	in_addr	_GetClientAddr() { return *reinterpret_cast<in_addr*>(&m_Addr.Addr); }
	USHORT	_GetClientPort() const { return m_Addr.Port; }

	void _PutParent(CEmClient_Peer* pParent){ m_pParent = pParent; }

	// Comparison
	virtual bool operator==(const CClient* pClient) = 0;
	virtual bool operator==(const AddrPort Addr);

	// Connection state
	virtual void OnConnected(){ m_bConnected = true; }
	virtual void OnDisconnected(){ m_bConnected = false; m_pParent = NULL; }

	__declspec(property(get=_GetConnected)) bool Connected;

	bool _GetConnected() const { return m_bConnected; }

	// Identification
	__declspec(property(get=_GetUserName, put=_PutUserName)) CString UserName;

	CString	_GetUserName() const { return m_sUserName; }
	void _PutUserName(CString sUserName) { m_sUserName = sUserName; }
	void _PutClientPort(USHORT nPort){ m_Addr.Port = nPort; }

	USHORT	ClientCountry(){ return m_uCountry; }

	// Upload to client
	__declspec(property(get=_GetUpState, put=_PutUpState))		EnumULQState	UploadState;
	__declspec(property(get=_GetReqFile))						CKnownFile*		ReqFile;
	__declspec(property(get=_GetLastUpReq))						CPreciseTime	LastUploadRequest;
	__declspec(property(get=_GetWaitTime, put=_PutWaitTime))	CPreciseTime	WaitStartTime;

	EnumULQState _GetUpState() const { return m_eUploadState; }
	CKnownFile* _GetReqFile() const { return m_pReqFile; }
	CPreciseTime _GetLastUpReq() const { return m_tmLastUpRequest; }
	CPreciseTime _GetWaitTime() const { return m_tmWaitStartTime; }

	void _PutUpState(EnumULQState eUploadState){ m_eUploadState = eUploadState; }
	void _PutWaitTime(CPreciseTime tmWaitTime){ m_tmWaitStartTime = tmWaitTime; }

	// Download from client
	__declspec(property(get=_GetDownState, put=_PutDownState))	EnumDLQState	DownloadState;

	EnumDLQState _GetDownState() const { return m_eDownloadState; }

	void _PutDownState(EnumDLQState eDownloadState){ m_eDownloadState = eDownloadState; }

protected:
	// Basics
	CEmClient_Peer*	m_pParent;

	// Connection data
	AddrPort	m_Addr;
	CString		m_sFullIP;
	bool		m_bConnected;

	// Identification
	CString		m_sUserName;
	USHORT		m_uCountry;

	// Upload
	EnumULQState	m_eUploadState;
	CKnownFile*		m_pReqFile;
	CPreciseTime	m_tmLastUpRequest;
	CPreciseTime	m_tmWaitStartTime;

	// Download
	EnumDLQState	m_eDownloadState;
};

class CClientMule : public CClient
{
public:
	CClientMule(AddrPort Addr, HashType Hash);
	virtual ~CClientMule();

	virtual EnumClientType _GetClientType(){ return CLIENT_MULE; };

	// Comparison
	virtual bool operator==(const CClient* pClient);
	virtual bool operator==(const HashType Hash);

	// Connection state
	virtual void OnConnected();
	virtual void OnDisconnected();

	EnumClientTypes GetHashType();

	// Connection data, identification
	__declspec(property(put=_PutClientUDPPort))	USHORT	ClientUDPPort;
	__declspec(property(get=_GetClientVersion, put=_PutClientVersion))	DWORD	ClientVersion;
	__declspec(property(get=_GetMuleVersion, put=_PutMuleVersion))		BYTE	MuleVersion;
	__declspec(property(get=_GetClientSoft))	EnumClientTypes	ClientSoft;
	__declspec(property(get=_ExtProtocol))		bool	MuleProtocol;

	DWORD	_GetClientVersion() const { return m_dwClientVersion; }
	BYTE	_GetMuleVersion() const { return m_nEmuleVersion; }
	EnumClientTypes _GetClientSoft() const { return m_eClientSoft; }
	bool	_ExtProtocol() const { return m_bEmuleProtocol; }

	void _PutClientUDPPort(USHORT nUDPPort){ m_nUDPPort = nUDPPort; }
	void _PutClientVersion(DWORD dwClientVersion){ m_dwClientVersion = dwClientVersion; }
	void _PutMuleVersion(BYTE nMuleVersion);

	// Uploading
	void RequestFile(CKnownFile* pFile);
	void RequestFileBlock(CKnownFile* pFile, DWORD dwStart, DWORD dwEnd);
	void SendRankingInfo();

	bool SendNextBlockData(DWORD dwMaxSize, bool bPrioritized);
	bool IsNeedMercyPacket();

	__declspec(property(get=_GetScore))	int Score;

	int _GetScore(){ return 0; }

private:
	// Additional connection data
	USHORT	m_nUDPPort;

	// Come from server
	AddrPort	m_ServerAddr;

	// Additional identification
	DWORD		m_dwUserID;
	HashType	m_Hash;

	// Software identification
	EnumClientTypes	m_eClientSoft;
	CString	m_sClientSoft;
	DWORD	m_dwClientVersion;
	DWORD	m_dwPlusVers;
	BYTE	m_nEmuleVersion;
	BYTE	m_nCompatibleClient;
	bool	m_bIsHybrid;
	bool	m_bIsMLDonkey;

	// Supported protocols & features
	bool	m_bIsLowID;
	bool	m_bEmuleProtocol;
	BYTE	m_nAcceptCommentVer;
	BYTE	m_nDataCompVer;
	BYTE	m_nExtendedRequestsVer;
	BYTE	m_nSourceExchangeVer;
	BYTE	m_nUDPVer;
	BYTE	m_nSupportSecIdent;

	// Upload data
	struct RequestRange
	{
		DWORD dwStart;
		DWORD dwEnd;

		RequestRange(DWORD dwS, DWORD dwE)
			: dwStart(dwS), dwEnd(dwE) {}
	};
	typedef multimap<CKnownFile*, RequestRange> RequestedBlocksMap;
	typedef pair<CKnownFile*, RequestRange> BlocksMapPair;
	RequestedBlocksMap m_RequestedBlocks;
	CPreciseTime m_LastUploadTime;
};

