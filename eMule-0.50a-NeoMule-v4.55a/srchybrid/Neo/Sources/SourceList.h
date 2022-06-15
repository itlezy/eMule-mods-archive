//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.tk )
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

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->

#include "MapKey.h"

class CTag;
class CFileDataIO;
class CUpDownClient;
class CPartFile;
class CClientFileStatus; // NEO: SCFS - [SmartClientFileStatus]

#ifdef _DEBUG
enum EClientSoftware;
#define _EClientSoftware	EClientSoftware
#else
#define _EClientSoftware	uint8
#endif


#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
enum EIPType{
	IP_Unknown		= 0, // We have no alanysis done for this client
	IP_Static		= 1, // The IP is for sure static
	IP_Dynamic		= 2, // The ip is dynamic, but the client have no auto disconnect and is many days online
	IP_Temporary	= 3  // The ip is dynamic and the client have an auto disconnect in some time frames
};
#endif // NEO_SA // NEO: NSA END

#pragma pack(1)
struct IPTableStruct{
	uint32		uIP;
	uint16		uPort;

	uint32		tFirstSeen;
	uint32		tLastSeen;

	uint8		uUnreachable;

	CArray<CTag*,CTag*>* taglist;
};
#pragma pack()

// NEO: SFL - [SourceFileList]
#pragma pack(1)
struct SeenFileStruct{
	// NEO: SCFS - [SmartClientFileStatus]
	SeenFileStruct() {prtFileStatus = NULL;}
	~SeenFileStruct();
	// NEO: SCFS END
	uchar		abyFileHash[16];
	uint64		uFileSize;

	uint32		tLastSeen;

	CClientFileStatus* prtFileStatus; // NEO: SCFS - [SmartClientFileStatus]

	CArray<CTag*,CTag*>* taglist;
};
#pragma pack()
// NEO: SFL END

// NEO: SHM - [SourceHashMonitor]
#pragma pack(1)
struct PortHash_Struct{
	uint16 nPort;
	uchar  abyUserHash[16];
};
#pragma pack()

#pragma pack(1)
struct MonitorSource_Struct{
	uint32		tLastChange;
	uint16		uFastChangeCount;

	uint32		m_dwInserted;

	CArray<PortHash_Struct> m_ItemsList;
};
#pragma pack()
// NEO: SHM END

#define IP_LEVEL_ZERO 0xFFFFFFFF // Static IP
#define IP_LEVEL_1ST  0x00FFFFFF // Small network
#define IP_LEVEL_2ND  0x0000FFFF // Normal ISP, default
#define IP_LEVEL_3RD  0x000000FF // Big ISP

class CKnownSource: public CObject
{
	DECLARE_DYNAMIC(CKnownSource)

	friend class CSourceList;
	friend class CUpDownClient;
public:
	CKnownSource(CFileDataIO* file);
	CKnownSource(const uchar* key);
	~CKnownSource();

	void			Init();
	bool			IsEmpty()										{ return (m_IPTables.GetCount() == 0); }
	void			WriteToFile(CFileDataIO* file);

	void			Use()											{m_uUsed++;}
	void			UnUse()											{m_uUsed--;}
	bool			IsUsed() const									{return (m_uUsed != 0);}

	const uchar*	GetKey() const									{ return m_achUserHash; }

	void			Attach(CUpDownClient* owner);
	void			Detach(CUpDownClient* owner);
	void			SUIPassed(CUpDownClient* owner);

	void			ConnectionSuccess();
	void			ConnectionFaild();

	//void			Merge(CKnownSource* tomerge);

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	int				GetAvalibilityProbability() const				{ return m_iAvalibilityProbability; }
	void			CalculateAvalibilityProbability();
	uint32			GetRemindingIPTime();
#endif // NEO_SA // NEO: NSA END

	// NEO: SFL - [SourceFileList]
	SeenFileStruct*	AddSeenFile(const uchar* hash, uint64 size);
	void			RemoveSeenFile(const uchar* hash);
	SeenFileStruct* GetSeenFile(const uchar* hash) const;
	// NEO: SFL END

	const uchar*	GetUserHash() const								{ return (uchar*)m_achUserHash; }
	bool			HasValidHash() const							{ return ((const int*)m_achUserHash[0]) != 0 || ((const int*)m_achUserHash[1]) != 0 || ((const int*)m_achUserHash[2]) != 0 || ((const int*)m_achUserHash[3]) != 0; }

	uint32			GetConnectIP() const							{ return m_dwCurrentIP; }
	uint16			GetConnectPort() const							{ return m_nCurrentPort; }

	uint32			GetIPZone() const								{ return m_dwIPZone; }

	uint32			GetIP() const									{ return m_dwUserIP; }
	uint16			GetUserPort() const								{ return m_nUserPort; }

	uint32			GetUserIDHybrid() const							{ return m_nUserIDHybrid; }
	__inline bool	HasLowID() const								{ return (m_nUserIDHybrid < 16777216); }
	uint16			GetUDPPort() const								{ return m_nUDPPort; }
	uint16			GetKadPort() const								{ return m_nKadPort; }
	
	uint32			GetLastSeen()const								{ return m_uLastSeen; }
	void			SetLastSeen()									{ m_uLastSeen = time(NULL); }

	const CString&	GetUserName() const								{ return m_strUserName; }
	const CString&	GetClientSoftVer() const						{ return m_strClientSoftware; }
	const CString&	GetClientModVer() const							{ return m_strModVersion; }
	_EClientSoftware GetClientSoft() const							{ return (_EClientSoftware)m_clientSoft; }

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	EIPType			GetIPType() const;

	uint32			GetLongestOnTime() const						{ return m_uLongestOnTime; }
	uint32			GetMidleOnTime() const							{ return m_uMidleOnTime; }
	uint32			GetShortestOnTime() const						{ return m_uShortestOnTime; }
	
	uint32			GetLongestOffTime() const						{ return m_uLongestOffTime; }
	uint32			GetMidleOffTime() const							{ return m_uMidleOffTime; }
	uint32			GetShortestOffTime() const						{ return m_uShortestOffTime; }
	
	uint32			GetLongestIPTime() const						{ return m_uLongestIPTime; }
	uint32			GetMidleIPTime() const							{ return m_uMidleIPTime; }
	uint32			GetShortestIPTime() const						{ return m_uShortestIPTime; }

	UINT			GetLargestFaildCount() const					{ return m_uLargestFaildCount; }
	UINT			GetMidleFaildCount() const						{ return m_uMidleFaildCount; }
	UINT			GetSmallestFaildCount() const					{ return m_uSmallestFaildCount; }

	uint32			GetLastSeenDuration() const						{ return m_uLastSeenDuration; }
	uint32			GetTotalSeenDuration() const					{ return m_uTotalSeenDuration; }
	uint32			GetLastLinkTime() const							{ return m_uLastLinkTime; }

	uint32			GetLastAnalisis() const							{ return m_uLastAnalisis; }
	int				GetAnalisisQuality() const						{ if (GetIPType() == IP_Unknown) return 0; return m_iAnalisisQuality; }
#endif // NEO_SA // NEO: NSA END

protected:
	friend class CSourceDetailPage;

	void	Clear();
	void	ClearTags();

	bool	LoadIPTables(CFileDataIO* file);
	void	SaveIPTables(CFileDataIO* file);

	// NEO: SFL - [SourceFileList]
	bool	LoadSeenFiles(CFileDataIO* file);
	void	SaveSeenFiles(CFileDataIO* file);
	// NEO: SFL END

	CList<IPTableStruct*,IPTableStruct*> m_IPTables;
	CMap<CCKey, const CCKey&, SeenFileStruct*, SeenFileStruct*> m_SeenFiles; // NEO: SFL - [SourceFileList]

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
 #ifdef _DEBUG_NEO	// NEO: ND - [NeoDebug]
	friend class	CSourceList;

	int 			ExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS *ep);
	CString			DumpTable();
	void			AnalyzeIPBehaviorProc();
 #endif // _DEBUG_NEO // NEO: ND END
	void			AnalyzeIPBehavior();
#endif // NEO_SA // NEO: NSA END

private:
	uchar	m_achUserHash[16];

	uint32	m_dwIPZone;
	uint32	m_dwIPLevel;
	bool	m_bIPZoneInvalid;

	uint32	m_dwCurrentIP;
	uint16	m_nCurrentPort;

	IPTableStruct* m_TemporaryIPTable; // store tepoorary an invalid IP table
	IPTableStruct* m_CurrentIPTable; // store the current valid IP table

	uint32	m_dwUserIP;
	uint16	m_nUserPort;

	uint32	m_nUserIDHybrid;
	uint16	m_nUDPPort;
	uint16	m_nKadPort;

	uint32	m_uLastSeen; 

	CString	m_strUserName;
	CString m_strClientSoftware;
	CString m_strModVersion;
	uint8	m_clientSoft;

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	int		m_iAvalibilityProbability;

	uint8	m_uStaticIP;

	uint32	m_uLongestOnTime;
	uint32	m_uMidleOnTime;
	uint32	m_uShortestOnTime;
	
	uint32	m_uLongestOffTime;
	uint32	m_uMidleOffTime;
	uint32	m_uShortestOffTime;
	
	uint32	m_uLongestIPTime;
	uint32	m_uMidleIPTime;
	uint32	m_uShortestIPTime;

	UINT	m_uLargestFaildCount;
	UINT	m_uMidleFaildCount;
	UINT	m_uSmallestFaildCount;

	uint32	m_uLastSeenDuration;
	uint32	m_uTotalSeenDuration;
	uint32	m_uLastLinkTime;

	uint32	m_uLastAnalisis;
	int		m_iAnalisisQuality;
	bool	m_bAnalisisNeeded;
#endif // NEO_SA // NEO: NSA END

	uint16	m_uUsed;

	CArray<CTag*,CTag*> taglist;

	void CreateIPZone(uint32 dwIP, uint32 dwLevel)	{ m_dwIPLevel = dwLevel; m_dwIPZone = (dwIP & dwLevel); }
	bool CheckIPZone(uint32 dwIP, uint32 dwLevel)	{ return ((m_dwIPZone & dwLevel) == (dwIP & dwLevel)); }
	bool CheckIPZone(uint32 dwIP)					{ return (m_dwIPZone == (dwIP & m_dwIPLevel)); }
	void ReConsiderIPZone(uint32 dwIP, bool bAllowReset = true);
	void ReConsiderIPZone();
	//void CleanUpIPTables();

	IPTableStruct* CreateNewIPTable();
	bool HandleIPTable(IPTableStruct* IPTable);
};


class CSourceList
{
public:
	CSourceList();
	~CSourceList();
	
	CKnownSource* GetSource(const uchar* key);
	//CKnownSource* MergeSource(CKnownSource* source);
	void	Process();

	bool	IsSourcePtrInList(const CKnownSource* source) const;
	bool	RemoveSourceFromPtrList(CKnownSource* source);

	void	FindSources(CPartFile* pFile); // NEO: SFL - [SourceFileList]

	// NEO: SHM - [SourceHashMonitor]
	bool	MonitorSource(CUpDownClient* toadd);
	UINT	GetMonitorCount() const		{ return m_MonitoredSourceList.GetCount(); }
	// NEO: SHM END

protected:
	friend class CSourceListDlg;

	bool	LoadList();
	void	ClearList();
	void	SaveList();

	bool	SourceHaveFiles(CKnownSource* source);

	void	RemoveAllMonitoredSource(); // NEO: SHM - [SourceHashMonitor]

	CMap<CCKey, const CCKey&, CKnownSource*, CKnownSource*> m_mapSources;

	// Monirot hash changes
	CMap<uint32, uint32, MonitorSource_Struct*, MonitorSource_Struct*> m_MonitoredSourceList; // NEO: SHM - [SourceHashMonitor]

private:
	uint32			m_nLastSaved;
	uint32			m_nLastCleanUp;
	uint32			m_dwLastTrackedCleanUp; // NEO: SHM - [SourceHashMonitor]
};

#endif // NEO_CD // NEO: NCD END <-- Xanatos --