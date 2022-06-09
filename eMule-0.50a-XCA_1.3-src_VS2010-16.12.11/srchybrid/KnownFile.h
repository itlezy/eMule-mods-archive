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
class CSafeMemFile;		//Xman PowerRelease

typedef CAtlList<CUpDownClient*> CUpDownClientPtrList;

class CKnownFile : public CShareableFile
{
	//DECLARE_DYNAMIC(CKnownFile)

public:
	CKnownFile();
	virtual ~CKnownFile();

	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bRemoveControlChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!
	
	bool	CreateFromFile(LPCTSTR directory, LPCTSTR filename, LPVOID pvProgressParam); // create date, hashset and tags from a file
	bool	LoadFromFile(CFileDataIO* file, bool I64Time);	//load date, hashset and tags from a .met file// X: [E64T] - [Enable64BitTime]
	bool	WriteToFile(CFileDataIO* file, bool I64Time);// X: [E64T] - [Enable64BitTime]
	bool	CreateAICHHashSetOnly();

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	CTime	GetUtcCFileDate() const										{ return CTime(m_tUtcLastModified); }
	uint64	GetUtcFileDate() const { return m_tUtcLastModified; }// X: [64T] - [64BitTime]

	// Did we not see this file for a long time so that some information should be purged?
	bool	ShouldPartiallyPurgeFile() const;
	//void	SetLastSeen()												{ m_timeLastSeen = time(NULL); }

	virtual void	SetFileSize(EMFileSize nFileSize);

	// nr. of 9MB parts (file data)
	__inline uint16 GetPartCount() const								{ return m_iPartCount; }

	// nr. of 9MB parts according the file size wrt ED2K protocol (OP_FILESTATUS)
	__inline uint16 GetED2KPartCount() const { return m_iED2KPartCount; }

	// file upload priority
	uint8	GetUpPriority(void) const									{ return m_iUpPriority; }
	uint8	GetUpPriorityEx(void) const {return  m_iUpPriority+1 == 5 ? 0 : m_iUpPriority+1;} //Xman Close Backdoor v2
	void	SetUpPriority(uint8 iNewUpPriority, bool bSave = true);
	bool	IsAutoUpPriority(void) const								{ return m_bAutoUpPriority; }
	void	SetAutoUpPriority(bool NewAutoUpPriority)					{ m_bAutoUpPriority = NewAutoUpPriority; }
	void	UpdateAutoUpPriority();

	// This has lost it's meaning here.. This is the total clients we know that want this file..
	// Right now this number is used for auto priorities..
	// This may be replaced with total complete source known in the network..
	//Xman see on uploadqueue don't need it:
	//uint32	GetQueuedCount() { return m_ClientUploadList.GetCount();}

	void	AddUploadingClient(CUpDownClient* client);
	void	RemoveUploadingClient(CUpDownClient* client);
	
	bool	HideOvershares(CSafeMemFile* file, CUpDownClient* client); //Xman PowerRelease

	//Xman advanced upload-priority
	double CalculateUploadPriorityPercent();
	void CalculateAndSetUploadPriority();
#ifdef _DEBUG
	void CalculateAndSetUploadPriority2(); //Xman the debug version
#endif
	uint64 GetWantedUpload();
	float pushfaktor;
	void UpdateVirtualUploadSources();
	UINT m_nVirtualUploadSources;
	uint_ptr GetVirtualSourceIndicator() const;
	void CheckAUPFilestats(bool allowUpdatePrio);
	//Xman end

	//Xman show virtual sources (morph)
	uint_ptr m_nVirtualCompleteSourcesCount;

	//Xman see OnUploadqueue
	void AddOnUploadqueue()				{onuploadqueue++;UpdateAutoUpPriority();}
	void RemoveOnUploadqueue()			{if(onuploadqueue!=0) onuploadqueue--;UpdateAutoUpPriority();}
	uint16 GetOnUploadqueue() const		{return onuploadqueue;}
	//Xman end

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

	uint64	GetLastPublishTimeKadSrc() const { return m_lastPublishTimeKadSrc; }// X: [64T] - [64BitTime]
	void	SetLastPublishTimeKadSrc(uint64 time, uint32 buddyip) { m_lastPublishTimeKadSrc = time; m_lastBuddyIP = buddyip;}// X: [64T] - [64BitTime]
	uint32	GetLastPublishBuddy() const { return m_lastBuddyIP; }
	void	SetLastPublishTimeKadNotes(uint64 time) {m_lastPublishTimeKadNotes = time;}// X: [64T] - [64BitTime]
	uint64	GetLastPublishTimeKadNotes() const { return m_lastPublishTimeKadNotes; }// X: [64T] - [64BitTime]

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

	static bool	CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL, bool slowdown=false); //Xman Nice Hash



	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	uint64	m_tUtcLastModified;// X: [64T] - [64BitTime]

	CStatisticFile statistic;
	time_t m_nCompleteSourcesTime;
	uint16 m_nCompleteSourcesCount;
	uint16 m_nCompleteSourcesCountLo;
	uint16 m_nCompleteSourcesCountHi;
	CUpDownClientPtrList m_ClientUploadList;
	CAtlArray<uint16> m_AvailPartFrequency;
	CCollection* m_pCollection;

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//preview
	bool	GrabImage(CString strFileName, uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	bool	LoadTagsFromFile(CFileDataIO* file);
	bool	LoadDateFromFile(CFileDataIO* file, bool I64Time);// X: [E64T] - [Enable64BitTime]
	static void	CreateHash(CFile* pFile, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL, bool slowdown=false); //Xman Nice Hash
	static bool	CreateHash(FILE* fp, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL, bool slowdown=false); //Xman Nice Hash
	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);

	// ==> Removed Dynamic Hide OS [SlugFiller/Xman] - Stulle
	//uint32	*CalcPartSpread();	//Xman PowerRelease
	// <== Removed Dynamic Hide OS [SlugFiller/Xman] - Stulle

// Maella -One-queue-per-file- (idea bloodymad)
public:
	uint_ptr GetFileScore(uint32 downloadingTime);
	uint32 GetStartUploadTime() const {return m_startUploadTime;}
	void   UpdateStartUploadTime() {m_startUploadTime = GetTickCount();}

private:
	uint32 m_startUploadTime;
// Maella end

	uint16 onuploadqueue;	//Xman see OnUploadqueue
	// ==> Removed Dynamic Hide OS [SlugFiller/Xman] - Stulle
	//uint16 hideos;			//Xman PowerRelease
	// <== Removed Dynamic Hide OS [SlugFiller/Xman] - Stulle
	//static CBarShader s_ShareStatusBar;
	uint16	m_iPartCount;
	uint16	m_iED2KPartCount;
	uint8	m_iUpPriority;
	bool	m_bAutoUpPriority;
	bool	m_PublishedED2K;
	uint32	kadFileSearchID;
	uint64	m_lastPublishTimeKadSrc;// X: [64T] - [64BitTime]
	uint64	m_lastPublishTimeKadNotes;// X: [64T] - [64BitTime]
	uint32	m_lastBuddyIP;
	Kademlia::WordList wordlist;
	UINT	m_uMetaDataVer;
	//time_t	m_timeLastSeen; // we only "see" files when they are in a shared directory
	bool	m_bAICHRecoverHashSetAvailable;

// ZZUL-TRA :: SOTN :: Start
private:
        CAtlArray<uint16> m_SOTNAvailPartFrequency; 
public:
	bool	WriteSafePartStatus(CSafeMemFile* file, CUpDownClient* client, const bool bUDP = false);
	bool	WritePartStatus(CSafeMemFile* file, CUpDownClient* client, const bool bUDP);
	bool	GetShareOnlyTheNeed() const;
	uint8*	GetPartStatus(CUpDownClient* client = NULL) const;
// ZZUL-TRA :: SOTN :: End
};
