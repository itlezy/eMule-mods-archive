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
#include "ShareableFile.h"
#include <list>

class CxImage;
class CUpDownClient;
class Packet;
class CFileDataIO;
class CAICHHashTree;
class CAICHRecoveryHashSet;
class CCollection;
class CAICHHashAlgo;

typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

class CKnownFile : public CShareableFile
{
	DECLARE_DYNAMIC(CKnownFile)

public:
	CKnownFile();
	virtual ~CKnownFile();

	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bRemoveControlChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!
	
	bool	CreateFromFile(LPCTSTR directory, LPCTSTR filename, LPVOID pvProgressParam); // create date, hashset and tags from a file
	bool	LoadFromFile(CFileDataIO* file);	//load date, hashset and tags from a .met file
	bool	WriteToFile(CFileDataIO* file);
	bool	CreateAICHHashSetOnly();

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	CTime	GetUtcCFileDate() const										{ return CTime(m_tUtcLastModified); }
	uint32	GetUtcFileDate() const										{ return m_tUtcLastModified; }

	// Did we not see this file for a long time so that some information should be purged?
	bool	ShouldPartiallyPurgeFile() const;
	void	SetLastSeen()												{ m_timeLastSeen = time(NULL); }

	virtual void	SetFileSize(EMFileSize nFileSize);

	// nr. of 9MB parts (file data)
	__inline uint16 GetPartCount() const								{ return m_iPartCount; }

	// nr. of 9MB parts according the file size wrt ED2K protocol (OP_FILESTATUS)
	__inline uint16 GetED2KPartCount() const { return m_iED2KPartCount; }

	// ZZUL-TRA :: PartImportExport :: Start
	// size of part, all are 9 MB but the last part may be smaller
	uint32	GetPartSize(UINT uPart) const;
	// ZZUL-TRA :: PartImportExport :: End

	// file upload priority
	uint8	GetUpPriority(void) const									{ return m_iUpPriority; }
	void	SetUpPriority(uint8 iNewUpPriority, bool bSave = true);
	bool	IsAutoUpPriority(void) const								{ return m_bAutoUpPriority; }
	void	SetAutoUpPriority(bool NewAutoUpPriority)					{ m_bAutoUpPriority = NewAutoUpPriority; }
	void	UpdateAutoUpPriority();

	// This has lost it's meaning here.. This is the total clients we know that want this file..
	// Right now this number is used for auto priorities..
	// This may be replaced with total complete source known in the network..
	uint32	GetQueuedCount() { return m_ClientUploadList.GetCount();}

	void	AddUploadingClient(CUpDownClient* client);
	void	RemoveUploadingClient(CUpDownClient* client);
	virtual void	UpdatePartsInfo();
	virtual	void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat) const;

	// comment
	void	SetFileComment(LPCTSTR pszComment);

	void	SetFileRating(UINT uRating);

	bool	GetPublishedED2K() const { return m_PublishedED2K; }
	void	SetPublishedED2K(bool val);

	uint32	GetKadFileSearchID() const { return kadFileSearchID; }
	void	SetKadFileSearchID(uint32 id) { kadFileSearchID = id; } //Don't use this unless you know what your are DOING!! (Hopefully I do.. :)

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
	void	RemoveMetaDataTags(UINT uTagType = 0);
	void	RemoveBrokenUnicodeMetaDataTags();

	// preview
	bool	IsMovie() const;
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	// Display / Info / Strings
	virtual CString	GetInfoSummary(bool bNoFormatCommands = false) const;
	CString			GetUpPriorityDisplayString() const;

	//aich
	void	SetAICHRecoverHashSetAvailable(bool bVal)			{ m_bAICHRecoverHashSetAvailable = bVal; }
	bool	IsAICHRecoverHashSetAvailable() const				{ return m_bAICHRecoverHashSetAvailable; }						

	static bool	CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL);



	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	uint32	m_tUtcLastModified;

	CStatisticFile statistic;
	time_t m_nCompleteSourcesTime;
	uint16 m_nCompleteSourcesCount;
	uint16 m_nCompleteSourcesCountLo;
	uint16 m_nCompleteSourcesCountHi;
	CUpDownClientPtrList m_ClientUploadList;
	CArray<uint16/*, uint16*/> m_AvailPartFrequency;// ZZUL-TRA :: SOTN 	
	CCollection* m_pCollection;
//ZZUL-TRA :: PowerShare :: Start
	UINT m_nVirtualCompleteSourcesCount;
//ZZUL-TRA :: PowerShare :: End

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

//ZZUL-TRA :: PowerShare :: Start
	void    SetPowerShared(int newValue);
	bool    GetPowerShared() const;
	int		GetPowerSharedMode() const {return m_powershared;}
	bool	GetPowerShareAuto() const {return m_bPowerShareAuto;}
	void	SetPowerShareLimit(int newValue) {m_iPowerShareLimit = newValue;}
	int		GetPowerShareLimit() const {return m_iPowerShareLimit;}
	bool	GetPowerShareLimited() const {return m_bPowerShareLimited;}
        void	UpdatePowerShareLimit(bool autopowershare, bool limitedpowershare);

private:
	int		m_powershared;
	bool	m_bPowerShareAuto;
	bool	m_bpowershared;
	int		m_iPowerShareLimit;
	bool	m_bPowerShareLimited;
//ZZUL-TRA :: PowerShare :: End

protected:
	//preview
	bool	GrabImage(CString strFileName, uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	bool	LoadTagsFromFile(CFileDataIO* file);
	bool	LoadDateFromFile(CFileDataIO* file);
	static void	CreateHash(CFile* pFile, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL);
	static bool	CreateHash(FILE* fp, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL);
	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);

private:
// ZZUL-TRA :: Spreadbars :: Start
	int		m_iSpreadbarSetStatus;
	// ZZUL-TRA :: Spreadbars :: End 
	static CBarShader s_ShareStatusBar;
	uint16	m_iPartCount;
	uint16	m_iED2KPartCount;
	uint8	m_iUpPriority;
	bool	m_bAutoUpPriority;
	bool	m_PublishedED2K;
	uint32	kadFileSearchID;
	uint32	m_lastPublishTimeKadSrc;
	uint32	m_lastPublishTimeKadNotes;
	uint32	m_lastBuddyIP;
	Kademlia::WordList wordlist;
	UINT	m_uMetaDataVer;
	time_t	m_timeLastSeen; // we only "see" files when they are in a shared directory
	bool	m_bAICHRecoverHashSetAvailable;

// ZZUL-TRA :: Feedback :: Start
public:
	CString GetFeedBackString(const bool& bBBCodeStyle = 0) const;
// ZZUL-TRA :: Feedback :: End
// ZZUL-TRA :: SeeOnQueue :: Start
private:
	uint16	m_uOnUploadQueue;
public:
	void	AddOnUploadqueue()				{m_uOnUploadQueue++;}
	void	RemoveOnUploadqueue()			{if(m_uOnUploadQueue) m_uOnUploadQueue--;}
	uint16	GetOnUploadqueue() const		{return m_uOnUploadQueue;}
// ZZUL-TRA :: SeeOnQueue :: End

//ZZUL-TRA :: PowerShare :: Start
public:
	void	SetPsAmountLimit(int newValue) {m_iPsAmountLimit = newValue;}
	int		GetPsAmountLimit() const {return m_iPsAmountLimit;}
protected:
	int		m_iPsAmountLimit;
//ZZUL-TRA :: PowerShare :: End

// ZZUL-TRA :: SOTN :: Start
private:
	CArray<uint16/*, uint16*/> m_SOTNAvailPartFrequency;
public:	
	bool	WriteSafePartStatus(CSafeMemFile* file, CUpDownClient* client, const bool bUDP = false);
	bool	WritePartStatus(CSafeMemFile* file, CUpDownClient* client, const bool bUDP);
	bool	GetShareOnlyTheNeed() const;
	uint8*	GetPartStatus(CUpDownClient* client = NULL) const;
// ZZUL-TRA :: SOTN :: End
};
