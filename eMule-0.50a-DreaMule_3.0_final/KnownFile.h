//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
class CSafeMemFile;		//Xman PowerRelease

typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;
enum EFileType;

class CKnownFile : public CAbstractFile
{
	DECLARE_DYNAMIC(CKnownFile)

public:
	CKnownFile();
	virtual ~CKnownFile();

	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	const CString& GetPath() const { return m_strDirectory; }
	void SetPath(LPCTSTR path);

	const CString& GetFilePath() const { return m_strFilePath; }
	void SetFilePath(LPCTSTR pszFilePath);

	bool	CreateFromFile(LPCTSTR directory, LPCTSTR filename, LPVOID pvProgressParam); // create date, hashset and tags from a file
	bool	LoadFromFile(CFileDataIO* file);	//load date, hashset and tags from a .met file
	bool	WriteToFile(CFileDataIO* file);
	bool	CreateAICHHashSetOnly();

	EFileType GetVerifiedFileType() { return m_verifiedFileType; }
	void	  SetVerifiedFileType(EFileType in) { m_verifiedFileType=in; }

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	CTime	GetUtcCFileDate() const { return CTime(m_tUtcLastModified); }
	uint32	GetUtcFileDate() const { return m_tUtcLastModified; }

	virtual void SetFileSize(EMFileSize nFileSize);

	// local available part hashs
	UINT	GetHashCount() const { return hashlist.GetCount(); }
	uchar*	GetPartHash(UINT part) const;
	const CArray<uchar*, uchar*>& GetHashset() const { return hashlist; }
	bool	SetHashset(const CArray<uchar*, uchar*>& aHashset);

	//Xman
	// SLUGFILLER: SafeHash remove - removed unnececery hash counter
	/*
	// nr. of part hashs according the file size wrt ED2K protocol
	uint16	GetED2KPartHashCount() const { return m_iED2KPartHashCount; }
	*/

	// nr. of 9MB parts (file data)
	__inline uint16 GetPartCount() const { return m_iPartCount; }

	// nr. of 9MB parts according the file size wrt ED2K protocol (OP_FILESTATUS)
	__inline uint16 GetED2KPartCount() const { return m_iED2KPartCount; }

	// file upload priority
	uint8	GetUpPriority(void) const { return m_iUpPriority; }
	uint8	GetUpPriorityEx(void) const {return  m_iUpPriority+1 == 5 ? 0 : m_iUpPriority+1;} //Xman Close Backdoor v2
	void	SetUpPriority(uint8 iNewUpPriority, bool bSave = true);
	bool	IsAutoUpPriority(void) const { return m_bAutoUpPriority; }
	void	SetAutoUpPriority(bool NewAutoUpPriority) { m_bAutoUpPriority = NewAutoUpPriority; }
	void	UpdateAutoUpPriority();

	// This has lost it's meaning here.. This is the total clients we know that want this file..
	// Right now this number is used for auto priorities..
	// This may be replaced with total complete source known in the network..
	//Xman see on uploadqueue don't need it:
	//uint32	GetQueuedCount() { return m_ClientUploadList.GetCount();}

	bool	LoadHashsetFromFile(CFileDataIO* file, bool checkhash);

	void	AddUploadingClient(CUpDownClient* client);
	void	RemoveUploadingClient(CUpDownClient* client);
	
	bool	HideOvershares(CSafeMemFile* file, CUpDownClient* client); //Xman PowerRelease

	//Xman advanced upload-priority
	double CalculateUploadPriorityPercent();
	void CalculateAndSetUploadPriority();
	void CalculateAndSetUploadPriority2(); //Xman the debug version
	uint64 GetWantedUpload();
	float pushfaktor;
	void UpdateVirtualUploadSources();
	UINT m_nVirtualUploadSources;
	uint32 GetVirtualSourceIndicator() const;
	void CheckAUPFilestats(bool allowUpdatePrio);
	//Xman end

	//Xman show virtual sources (morph)
	UINT m_nVirtualCompleteSourcesCount;

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

	// Tux: Improvement: Kademlia interface improvement
	// -> removed lines

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
	bool	IsMusic() const; //MORPH - Added by IceCream, added preview also for music files
	virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender);
	virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

	// aich
	CAICHHashSet*	GetAICHHashset() const							{return m_pAICHHashSet;}
	void			SetAICHHashset(CAICHHashSet* val)				{m_pAICHHashSet = val;}
	
	// Display / Info / Strings
	CString			GetInfoSummary() const;
	CString			GetUpPriorityDisplayString() const;

	// last file modification time in (DST corrected, if NTFS) real UTC format
	// NOTE: this value can *not* be compared with NT's version of the UTC time
	uint32	m_tUtcLastModified;

	CStatisticFile statistic;
	time_t m_nCompleteSourcesTime;
	uint16 m_nCompleteSourcesCount;
	uint16 m_nCompleteSourcesCountLo;
	uint16 m_nCompleteSourcesCountHi;
	CUpDownClientPtrList m_ClientUploadList;
	CArray<uint16, uint16> m_AvailPartFrequency;
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
	bool	LoadDateFromFile(CFileDataIO* file);
	void	CreateHash(CFile* pFile, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL, bool slowdown=false) const; //Xman Nice Hash
	bool	CreateHash(FILE* fp, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL, bool slowdown=false) const; //Xman Nice Hash
	bool	CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut = NULL, bool slowdown=false) const; //Xman Nice Hash
	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);

	uint32	*CalcPartSpread();	//Xman PowerRelease

	CArray<uchar*, uchar*>	hashlist;
	CString					m_strDirectory;
	CString					m_strFilePath;
	CAICHHashSet*			m_pAICHHashSet;

// Maella -One-queue-per-file- (idea bloodymad)
public:
	uint32 GetFileScore(uint32 downloadingTime);
	uint32 GetStartUploadTime() const {return m_startUploadTime;}
	void   UpdateStartUploadTime() {m_startUploadTime = GetTickCount();}

private:
	uint32 m_startUploadTime;
// Maella end

private:
	uint16 onuploadqueue;	//Xman see OnUploadqueue
	uint16 hideos;			//Xman PowerRelease
	static CBarShader s_ShareStatusBar;
	uint16	m_iPartCount;
	uint16	m_iED2KPartCount;
	uint16	m_iED2KPartHashCount;
	uint8	m_iUpPriority;
	bool	m_bAutoUpPriority;
	bool	m_PublishedED2K;
	//uint32	kadFileSearchID;	// Tux: Improvement: Kademlia interface improvement
	uint32	m_lastPublishTimeKadSrc;
	uint32	m_lastPublishTimeKadNotes;
	uint32	m_lastBuddyIP;
	Kademlia::WordList wordlist;
	UINT	m_uMetaDataVer;
	EFileType m_verifiedFileType;
};
