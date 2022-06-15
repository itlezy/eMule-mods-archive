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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include "BarShader.h"
#include "StatisticFile.h"
#include "AbstractFile.h"
#include <list>

class CxImage;
class CUpDownClient;
class Packet;
class CFileDataIO;
class CAICHHashTree;
class CAICHHashSet;
class CCollection;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
class CVoodooSocket;
class CMasterDatas;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
struct ExportInfo; // NEO: PIX - [PartImportExport] <-- Xanatos --
class CClientFileStatus; // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
enum EPartStatus; // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
class CKnownPreferences; // NEO: FCFG - [FileConfiguration] <-- Xanatos --
class CSafeMemFile; // NEO: IPS - [InteligentPartSharing] <-- Xanatos --

typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;
enum EFileType;

// NEO: RBT - [ReadBlockThread] -- Xanatos -->
struct ReadBlockOrder
{
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	ReadBlockOrder(uint64 startOffset, uint32 toread, CObject* client, void* request, bool voodoo = false)
#else
	ReadBlockOrder(uint64 startOffset, uint32 toread, CObject* client, void* request)
#endif // VOODOO // NEO: VOODOO END
	{
		StartOffset = startOffset;
		togo = toread;
		m_client = client;
		m_request = request;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		m_voodoo = voodoo;
#endif // VOODOO // NEO: VOODOO END
	}
	uint64			StartOffset;
	uint32			togo;
	CObject*		m_client;
	void*			m_request;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	bool			m_voodoo;
#endif // VOODOO // NEO: VOODOO END
};

#define _RBT_ACTIVE_	-2
#define RBT_ACTIVE		(byte*)_RBT_ACTIVE_
#define _RBT_ERROR_		-1
#define RBT_ERROR		(byte*)_RBT_ERROR_
// NEO: RBT END <-- Xanatos --

// NEO: IPS - [InteligentPartSharing] -- Xanatos -->
#pragma pack(1)
struct TPartOL{
	UINT	Part;
	double	OL;
};
#pragma pack()
// NEO: IPS END <-- Xanatos --

// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
#define PERM_DEFAULT	255 //-1
#define PERM_ALL		0
#define PERM_FRIENDS	1
#define PERM_NONE		2
// NEO: SSP END <-- Xanatos --

class CKnownFile : public CAbstractFile
{
	DECLARE_DYNAMIC(CKnownFile)

public:
	friend class CReadBlockFromFileThread; // NEO: RBT - [ReadBlockThread] <-- Xanatos --

	CKnownFile();
	virtual ~CKnownFile();

	virtual const CString& GetFileName(bool forceReal = false) const; // NEO: PP - [PasswordProtection] <-- Xanatos --
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bRemoveControlChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	//const CString& GetPath() const { return m_strDirectory; }
	CString GetPath(bool returnVirtual = false) const; // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
	void SetPath(LPCTSTR path);

	const CString& GetFilePath() const { return m_strFilePath; }
	void SetFilePath(LPCTSTR pszFilePath);

	bool	CreateFromFile(LPCTSTR directory, LPCTSTR filename, LPVOID pvProgressParam); // create date, hashset and tags from a file
	bool	LoadFromFile(CFileDataIO* file);	//load date, hashset and tags from a .met file
	bool	WriteToFile(CFileDataIO* file);
	// NEO: PIX - [PartImportExport] -- Xanatos -->
	bool	ExportParts(const CList<uint16>* PartList);
	BOOL	PerformExportParts(ExportInfo* Instructions); 
	// NEO: PIX END <-- Xanatos --
	bool	CreateAICHHashSetOnly();

	EFileType GetVerifiedFileType() { return m_verifiedFileType; }
	void	  SetVerifiedFileType(EFileType in) { m_verifiedFileType=in; }

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	CTime	GetUtcCFileDate() const { return CTime(m_tUtcLastModified); }
	uint32	GetUtcFileDate() const { return m_tUtcLastModified; }

	virtual void	SetFileSize(EMFileSize nFileSize);

	// local available part hashs
	UINT	GetHashCount() const { return hashlist.GetCount(); }
	uchar*	GetPartHash(UINT part) const;
	const CArray<uchar*, uchar*>& GetHashset() const { return hashlist; }
	bool	SetHashset(const CArray<uchar*, uchar*>& aHashset);

	// nr. of part hashs according the file size wrt ED2K protocol
	uint16	GetED2KPartHashCount() const { return m_iED2KPartHashCount; }

	// nr. of 9MB parts (file data)
	__inline uint16 GetPartCount() const { return m_iPartCount; }

	// nr. of 9MB parts according the file size wrt ED2K protocol (OP_FILESTATUS)
	__inline uint16 GetED2KPartCount() const { return m_iED2KPartCount; }

	// size of part, all are 9 MB but the last part may be smaller
	uint32	GetPartSize(UINT part) const; // NEO: MOD - [GetPartSize] <-- Xanatos --

	// file upload priority
	uint8	GetUpPriority(void) const { return m_iUpPriority; }
	uint8	GetUpPriorityEx(void) const; // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	void	SetUpPriority(uint8 iNewUpPriority, bool bSave = true);
	bool	IsAutoUpPriority(void) const { return m_bAutoUpPriority; }
	void	SetAutoUpPriority(bool NewAutoUpPriority) { m_bAutoUpPriority = NewAutoUpPriority; }
	void	UpdateAutoUpPriority();

	// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
	uint8	GetPermissions(void) const	{ return m_iPermissions; };
	void	SetPermissions(uint8 iNewPermissions) {m_iPermissions = iNewPermissions;};
	// NEO: SSP END <-- Xanatos --

	// This has lost it's meaning here.. This is the total clients we know that want this file..
	// Right now this number is used for auto priorities..
	// This may be replaced with total complete source known in the network..
	uint32	GetQueuedCount() { return m_ClientUploadList.GetCount();}

	bool	LoadHashsetFromFile(CFileDataIO* file, bool checkhash);

	void	AddUploadingClient(CUpDownClient* client);
	void	RemoveUploadingClient(CUpDownClient* client);
	virtual void	UpdatePartsInfo();
	virtual void	UpdatePartsInfoEx(EPartStatus type); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
	virtual	void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const;
	virtual void	DrawClassicShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool	 bFlat) const; // NEO: MOD - [ClassicShareStatusBar] <-- Xanatos --

	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	UINT	GetCategory() const;
	virtual void	SetCategory(UINT cat, uint8 init = 0); // NEO: MOD - [SetCategory]
	// NEO: NSC END <-- Xanatos --

	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	void	UpdateKnownPrefs(CKnownPreferences* cfgKnownPrefs);
	virtual bool HasPreferences();
	// NEO: FCFG END <-- Xanatos --

	// NEO: XCs - [SaveComments] -- Xanatos -->
	bool	LoadComments(CFileDataIO* file);
	bool	SaveComments(CFileDataIO* file);
	bool	HasComments()	{ return !m_CommentList.IsEmpty(); }
	// NEO: XCs END <-- Xanatos --

	// comment
	void	SetFileComment(LPCTSTR pszComment);

	void	SetFileRating(UINT uRating);

	bool	GetPublishedED2K() const { return m_PublishedED2K; }
	void	SetPublishedED2K(bool val);

	// NEO: KII - [KadInterfaceImprovement] -- Xanatos --
	//uint32	GetKadFileSearchID() const { return kadFileSearchID; }
	//void	SetKadFileSearchID(uint32 id) { kadFileSearchID = id; } //Don't use this unless you know what your are DOING!! (Hopefully I do.. :)

	const Kademlia::WordList& GetKadKeywords() const { return wordlist; }

	uint32	GetLastPublishTimeKadSrc() const { return m_lastPublishTimeKadSrc; }
	void	SetLastPublishTimeKadSrc(uint32 time, uint32 buddyip) { m_lastPublishTimeKadSrc = time; m_lastBuddyIP = buddyip;}
	uint32	GetLastPublishBuddy() const { return m_lastBuddyIP; }
	void	SetLastPublishTimeKadNotes(uint32 time) {m_lastPublishTimeKadNotes = time;}
	uint32	GetLastPublishTimeKadNotes() const { return m_lastPublishTimeKadNotes; }

	bool	PublishSrc();
	bool	PublishNotes();

	// file sharing
	virtual Packet* CreateSrcInfoPacket(const CUpDownClient* forClient, uint8 byRequestedVersion, uint16 nRequestedOptions) const;
	UINT	GetMetaDataVer() const { return m_uMetaDataVer; }
	void	UpdateMetaDataTags();
	void	RemoveMetaDataTags();

	// preview
	bool	IsMovie() const;
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	// aich
	CAICHHashSet*	GetAICHHashset() const							{return m_pAICHHashSet;}
	void			SetAICHHashset(CAICHHashSet* val)				{m_pAICHHashSet = val;}

	bool	CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const; // NEO: PIX - [PartImportExport] <-- Xanatos --

	// Display / Info / Strings
	CString			GetInfoSummary() const;
	CString			GetUpPriorityDisplayString() const;

	// NEO: RBT - [ReadBlockThread] -- Xanatos -->
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	bool	SetReadBlockFromFile(uint64 startOffset, uint32 toread, CObject* client, void* request, bool voodoo = false); 
#else
	bool	SetReadBlockFromFile(uint64 startOffset, uint32 toread, CObject* client, void* request);
#endif // VOODOO // NEO: VOODOO END
	// NEO: RBT END <-- Xanatos --

	virtual bool Publishable()				{ return true; } // NEO: SAFS - [ShowAllFilesInShare] <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	virtual bool IsVoodooFile() const {return HaveMasters();}
	bool HaveMasters() const {return (!m_MasterMap.IsEmpty());}
	virtual void AddMaster(CVoodooSocket* Master);
	virtual void RemoveMaster(CVoodooSocket* Master);
	CMasterDatas* GetMasterDatas(CVoodooSocket* Master);
	CVoodooSocket* GetMaster(uint64 start, uint64 end);
	bool	IsRealFile() const;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	virtual uint16	GetPartAvailibility(uint16 i) const		{return (m_AvailPartFrequency.IsEmpty()			? 0 : m_AvailPartFrequency[i]);			} // NEO: NPT - [NeoPartTraffic]  <-- Xanatos --

	// NEO: IPS - [InteligentPartSharing] -- Xanatos -->
	uint8	GetPartState(UINT part) const;
	bool	WritePartSelection(CSafeMemFile* file, CUpDownClient* client);

	uint8	GetIPSPartStatus(UINT part) const;
	void	SetIPSPartStatus(UINT part, uint8 state) { m_IPSPartStatus[part] = state; } 

	void	ResetIPSList();
	void	GetHideMap(CClientFileStatus* status, CMap<UINT, UINT, BOOL, BOOL> &GetHideMap) const; // NEO: SCFS - [SmartClientFileStatus]
	void	ReCalculateIPS() const;
	void	CalculateIPS();

	double	GetPartShared(uint16 part);
	// NEO: IPS END <-- Xanatos --

	// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
	void	SetReleasePriority(uint8 uReleasePriority, bool bSave = true);
	uint8	IsReleasePriority() const {return m_uReleasePriority;}
	uint8	IsReleasePriorityEx() const;
	bool	GetReleasePriority() const {return IsReleasePriorityEx() && (m_fReleaseModifyer > 1);} // NEO: NXC - [NewExtendedCategories]
	float	GetReleaseModifyer() const {return m_fReleaseModifyer;}
	bool	GetPowerShared() const {return m_bPowerShared;}
	void	CalcRelease(bool bFlag = false);
	// NEO: SRS END <-- Xanatos --

	// NEO: MQ - [MultiQueue] -- Xanatos -->
	uint32	GetStartUploadTime() const {return m_startUploadTime;}
	void	UpdateStartUploadTime() {m_startUploadTime = time(NULL);}
	// NEO: MQ END <-- Xanatos --

	// NEO: PP - [PasswordProtection] -- Xanatos -->
	void	LoadProtection();
	CString	GetPWProt()						{ if (!m_bProtectionLoaded) LoadProtection(); return m_pwProt; }
	void	SetPWProt( CString iNewPW );
	bool	IsPWProt() const;
	void	SetPWProtShow(bool in)			{ m_isPWProtShow=in; }
	bool	IsPWProtHidden() const			{ return IsPWProt() && !m_isPWProtShow; }
	// NEO: PP END <-- Xanatos --

	// NEO: CRC - [MorphCRCTag] -- Xanatos -->
	bool	IsCRCOk() const;
	bool    IsCRC32Calculated() const	{return m_CRC32[0] || m_CRC32[1] || m_CRC32[2] || m_CRC32[3];}
	const BYTE*	GetCalculatedCRC32() const		{return m_CRC32;}
	BYTE*	GetCalculatedCRC32rw() 		{return m_CRC32;}
	// NEO: CRC END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	virtual void GetTooltipFileInfo(CString &info);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	uint32	m_tUtcLastModified;

	CStatisticFile statistic;
	time_t m_nCompleteSourcesTime;
	uint32 m_nCompleteSourcesSize; // NEO: MOD - [RelativeChunkDisplay] <-- Xanatos --
	uint16 m_nCompleteSourcesCount;
	uint16 m_nCompleteSourcesCountLo;
	uint16 m_nCompleteSourcesCountHi;
	CUpDownClientPtrList m_ClientUploadList;
	CArray<uint16, uint16> m_AvailPartFrequency;
	CArray<uint16, uint16> m_AvailIncPartFrequency; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	CCollection* m_pCollection;

	UINT		m_category; // NEO: NSC - [NeoSharedCategories] <-- Xanatos --

	CKnownPreferences* KnownPrefs; // NEO: FCFG - [FileConfiguration] <-- Xanatos --

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//preview
	bool	GrabImage(CString strFileName, uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	bool	LoadTagsFromFile(CFileDataIO* file);
	bool	LoadDateFromFile(CFileDataIO* file);
	void	CreateHash(CFile* pFile, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	bool	CreateHash(FILE* fp, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const;
	//bool	CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL) const; // moved to public // NEO: PIX - [PartImportExport] <-- Xanatos --
	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);

	CArray<uchar*, uchar*>	hashlist;
	CString					m_strDirectory;
	CString					m_strFilePath;
	CAICHHashSet*			m_pAICHHashSet;

	// NEO: RBT - [ReadBlockThread] -- Xanatos -->
	CList<ReadBlockOrder*,ReadBlockOrder*> m_BlocksToRead;
	CCriticalSection m_BlocksToReadLocker;
	CWinThread* m_ReadThread;
	// NEO: RBT END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	CMap<CVoodooSocket*,CVoodooSocket*,CMasterDatas*,CMasterDatas*> m_MasterMap;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

private:
	static CBarShader s_ShareStatusBar;
	uint16	m_iPartCount;
	uint16	m_iED2KPartCount;
	uint16	m_iED2KPartHashCount;
	uint8	m_iUpPriority;
	uint8	m_iPermissions; // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	bool	m_bAutoUpPriority;
	bool	m_PublishedED2K;
	//uint32	kadFileSearchID; // NEO: KII - [KadInterfaceImprovement] <-- Xanatos --
	uint32	m_lastPublishTimeKadSrc;
	uint32	m_lastPublishTimeKadNotes;
	uint32	m_lastBuddyIP;
	Kademlia::WordList wordlist;
	UINT	m_uMetaDataVer;
	EFileType m_verifiedFileType;

	// NEO: IPS - [InteligentPartSharing] -- Xanatos -->
	DWORD	m_uLastIPSCalcTime;
	CMap<UINT, UINT, uint8, uint8> m_IPSPartStatus;
	CArray<TPartOL,TPartOL> *m_IPSPartsInfo;
	// NEO: IPS END <-- Xanatos --

	// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
	uint32	m_uLastReleaseModifyer;
	uint8	m_uReleasePriority;
	float	m_fReleaseModifyer;
	bool	m_bPowerShared;
	// NEO: SRS END <-- Xanatos --

	uint32  m_startUploadTime; // NEO: MQ - [MultiQueue] <-- Xanatos --

	// NEO: PP - [PasswordProtection] -- Xanatos -->
	bool	m_bProtectionLoaded;
	CString	m_pwProt;
	bool	m_isPWProtShow;
	// NEO: PP END <-- Xanatos --

	BYTE    m_CRC32[4]; // NEO: CRC - [MorphCRCTag] <-- Xanatos --
};

// NEO: RBT - [ReadBlockThread] -- Xanatos -->
class CReadBlockFromFileThread : public CWinThread
{
	DECLARE_DYNCREATE(CReadBlockFromFileThread)
protected:
	CReadBlockFromFileThread()	{}

	friend class CKnownFile;
	CFile file;
public:
	virtual	BOOL	InitInstance() {return true;}
	virtual int		Run();
	void			SetKnownFile(CKnownFile* pOwner);
private:
	CKnownFile*		m_pOwner;
	CString			fullname;
};
// NEO: RBT END <-- Xanatos --
