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


/* 
 SHA haset basically exists of 1 Tree for all Parts (9.28MB) + n  Trees
 for all blocks (180KB) while n is the number of Parts.
 This means it is NOT a complete hashtree, since the 9.28MB is a given level, in order
 to be able to create a hashset format similar to the MD4 one.

 If the number of elements for the next level are odd (for example 21 blocks to spread into 2 hashs)
 the majority of elements will go into the left branch if the parent node was a left branch
 and into the right branch if the parent node was a right branch. The first node is always
 taken as a left branch.

Example tree:
	FileSize: 19506000 Bytes = 18,6 MB

								X (18,6)                                   MasterHash
							 /     \
						 X (18,55)   \
					/		\	       \
                   X(9,28)  x(9,28)   X (0,05MB)						   PartHashs
			   /      \    /       \        \
		X(4,75)   X(4,57) X(4,57)  X(4,75)   \

						[...............]
X(180KB)   X(180KB)  [...] X(140KB) | X(180KB) X(180KB [...]			   BlockHashs
									v
						 Border between first and second Part (9.28MB)

HashsIdentifier:
When sending hashs, they are send with a 16bit identifier which specifies its postion in the
tree (so StartPosition + HashDataSize would lead to the same hash)
The identifier basically describes the way from the top of the tree to the hash. a set bit (1)
means follow the left branch, a 0 means follow the right. The highest bit which is set is seen as the start-
postion (since the first node is always seend as left).

Example

								x                   0000000000000001
							 /     \		
						 x		    \				0000000000000011
					  /		\	       \
                    x       _X_          x 	        0000000000000110


Version 2 of AICH also supports 32bit identifiers to support large files, check CAICHRecoveryHashSet::CreatePartRecoveryData


*/

#pragma once
#include "map_inc.h"

#define HASHSIZE		20
#define KNOWN2_MET_FILENAME			_T("known2_64.met")
#define OLD_KNOWN2_MET_FILENAME		_T("known2.met")
#define KNOWN2_MET_VERSION			0x02

enum EAICHStatus {
	AICH_ERROR = 0,
	AICH_EMPTY,
	AICH_UNTRUSTED,
	AICH_TRUSTED,
	AICH_VERIFIED,
	AICH_HASHSETCOMPLETE
};

class CFileDataIO;
class CKnownFile;
class CSafeMemFile;
class CPartFile;
class CUpDownClient;
/////////////////////////////////////////////////////////////////////////////////////////
///CAICHHash
class CAICHHash 
{
public:
	~CAICHHash()									{}
	CAICHHash()										{ memset(m_abyBuffer, 0, HASHSIZE); }
	CAICHHash(CFileDataIO* file)					{ Read(file); }
	CAICHHash(uchar* data)							{ Read(data); }
	CAICHHash(const CAICHHash& k1)					{ *this = k1; }
	CAICHHash&	operator=(const CAICHHash& k1)		{ memcpy(m_abyBuffer, k1.m_abyBuffer, HASHSIZE); return *this; }
	friend bool operator==(const CAICHHash& k1,const CAICHHash& k2)	{ return memcmp(k1.m_abyBuffer, k2.m_abyBuffer, HASHSIZE) == 0;}
	friend bool operator!=(const CAICHHash& k1,const CAICHHash& k2)	{ return !(k1 == k2); }
	void		Read(CFileDataIO* file);
	void		Write(CFileDataIO* file) const;
	void		Read(uchar* data)					{ memcpy(m_abyBuffer, data, HASHSIZE); }
	CString		GetString() const;
	uchar*		GetRawHash()						{ return m_abyBuffer; }
	const uchar* GetRawHashC() const				{ return m_abyBuffer; }
	bool operator()(const CAICHHash& k1,const CAICHHash& k2) const{ return memcmp(k1.m_abyBuffer, k2.m_abyBuffer, HASHSIZE) == 0;}
#if defined(REPLACE_ATLMAP) || defined(HAVE_UNORDERED) // X: enable these code for CAICHSyncThread::run()
	size_t operator()(const CAICHHash& key) const{
		size_t hash = 1;
		for (size_t i = 0;i != HASHSIZE;i++)
			hash += (key.GetRawHashC()[i]+1)*((i*i)+1);
		return hash;
	}
#endif
	//	static int	GetHashSize()						{ return HASHSIZE;}// X: use HASHSIZE instead
private:
	uchar m_abyBuffer[HASHSIZE];
};
#ifndef REPLACE_ATLMAP
class CAICHHashTraits :
	public CElementTraitsBase<CAICHHash>,
	public CDefaultCompareTraits<CAICHHash>
{
public:
	static ULONG Hash( const CAICHHash& key ) throw()
	{
		size_t hash = 1;
		for (size_t i = 0;i != HASHSIZE;i++)
			hash += (key.GetRawHashC()[i]+1)*((i*i)+1);
		return (ULONG)hash;
	}
};
#endif
/////////////////////////////////////////////////////////////////////////////////////////
///CAICHHashAlgo
class CAICHHashAlgo 
{
public:
	virtual void	Reset() = 0;
	virtual void	Add(LPCVOID pData, DWORD nLength) = 0;
	virtual void	Finish(CAICHHash& Hash) = 0;
	virtual void	GetHash(CAICHHash& Hash) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
///CAICHHashTree
class CAICHHashTree
{
	friend class CAICHHashTree;
	friend class CAICHRecoveryHashSet;
public:
	CAICHHashTree(uint64 nDataSize, bool bLeftBranch, uint64 nBaseSize);
	~CAICHHashTree();
	//zz_fly :: known2 buffer // helper method of CList
	CAICHHashTree()									{;}
	CAICHHashTree(const CAICHHashTree& k1)					{ *this = k1; }
	CAICHHashTree&	operator=(const CAICHHashTree& k1);
	friend bool operator==(const CAICHHashTree& k1,const CAICHHashTree& k2)	{ return (k1.m_Hash == k2.m_Hash);}
	friend bool operator!=(const CAICHHashTree& k1,const CAICHHashTree& k2)	{ return !(k1 == k2); }
	//zz_fly :: end
	void			SetBlockHash(uint64 nSize, uint64 nStartPos, CAICHHashAlgo* pHashAlg);
	bool			ReCalculateHash(CAICHHashAlgo* hashalg, bool bDontReplace );
	bool			VerifyHashTree(CAICHHashAlgo* hashalg, bool bDeleteBadTrees);
	CAICHHashTree*	FindHash(uint64 nStartPos, uint64 nSize)					{uint8 buffer = 0; return FindHash(nStartPos, nSize, &buffer);}
	const CAICHHashTree* FindExistingHash(uint64 nStartPos, uint64 nSize) const		{uint8 buffer = 0; return FindExistingHash(nStartPos, nSize, &buffer);}
	uint64			GetBaseSize() const;		
	void			SetBaseSize(uint64 uValue);
	//zz_fly :: known2 buffer
	void			ClearSubTree();  //a safe way to clear subtrees
	void			MarkBuffered();  //marked the subtrees as buffered
	void			MarkUnBuffered();  //marked the subtrees as unbuffered
	//zz_fly :: end

protected:
	CAICHHashTree*	FindHash(uint64 nStartPos, uint64 nSize, uint8* nLevel);
	const CAICHHashTree* FindExistingHash(uint64 nStartPos, uint64 nSize, uint8* nLevel) const;
	bool			CreatePartRecoveryData(uint64 nStartPos, uint64 nSize, CFileDataIO* fileDataOut, uint32 wHashIdent, bool b32BitIdent);
	void			WriteHash(CFileDataIO* fileDataOut, uint32 wHashIdent, bool b32BitIdent) const;
	bool			WriteLowestLevelHashs(CFileDataIO* fileDataOut, uint32 wHashIdent, bool bNoIdent, bool b32BitIdent) const;
	bool			LoadLowestLevelHashs(CFileDataIO* fileInput);
	bool			SetHash(CFileDataIO* fileInput, uint32 wHashIdent, sint8 nLevel = (-1), bool bAllowOverwrite = true);
	bool			ReduceToBaseSize(uint64 nBaseSize);
	
	CAICHHashTree*	m_pLeftTree;
	CAICHHashTree*	m_pRightTree;

public:
	CAICHHash		m_Hash;
	uint64			m_nDataSize;		// size of data which is covered by this hash	
	bool			m_bIsLeftBranch;	// left or right branch of the tree
	bool			m_bHashValid;		// the hash is valid and not empty

private:
	// BaseSize: to save ressources we use a bool to store the basesize as currently only two values are used
	// keep the original number based calculations and checks in the code through, so it can easily be adjusted in case we want to use hashsets with different basesizes
	bool			m_bBaseSize;		// blocksize on which the lowest hash is based on
	bool			m_bIsBuffered; //zz_fly :: known2 buffer //is this tree in buffer
};

/////////////////////////////////////////////////////////////////////////////////////////
///CAICHUntrustedHashs
class CAICHUntrustedHash {
public:
	CAICHUntrustedHash(){}
	CAICHUntrustedHash(const CAICHUntrustedHash& k1)		{ m_adwIpsSigning.Copy(k1.m_adwIpsSigning); m_Hash = k1.m_Hash ; }
	bool	AddSigningIP(uint32 dwIP, bool bTestOnly);

	CAICHHash				m_Hash;
	CAtlArray<uint32>	m_adwIpsSigning;
};

/////////////////////////////////////////////////////////////////////////////////////////
///CAICHUntrustedHashs
class CAICHRequestedData {
public:
	CAICHRequestedData()	{m_nPart = 0; m_pPartFile = NULL; m_pClient= NULL;}
	CAICHRequestedData&	operator=(const CAICHRequestedData& k1)		{ m_nPart = k1.m_nPart; m_pPartFile = k1.m_pPartFile; m_pClient = k1.m_pClient; return *this; }
	uint16			m_nPart;
	CPartFile*		m_pPartFile;
	CUpDownClient*	m_pClient;
};

/////////////////////////////////////////////////////////////////////////////////////////
///CAICHRecoveryHashSet
class CAICHRecoveryHashSet
{
public:
	CAICHRecoveryHashSet(CKnownFile* pOwner, EMFileSize nSize = (uint64)0);
	~CAICHRecoveryHashSet(void);
	bool			CreatePartRecoveryData(uint64 nPartStartPos, CFileDataIO* fileDataOut/*, bool bDbgDontLoad = false*/);
	bool			ReadRecoveryData(uint64 nPartStartPos, CSafeMemFile* fileDataIn);
	bool			ReCalculateHash(bool bDontReplace = false);
	bool			VerifyHashTree(bool bDeleteBadTrees);
	void			UntrustedHashReceived(const CAICHHash& Hash, uint32 dwFromIP);
	bool			IsPartDataAvailable(uint64 nPartStartPos);
	void			SetStatus(EAICHStatus bNewValue)			{m_eStatus = bNewValue;}
	EAICHStatus		GetStatus()	const							{return m_eStatus;}
	void			SetOwner(CKnownFile* val)					{m_pOwner = val;}
	
	void			FreeHashSet();
	void			SetFileSize(EMFileSize nSize);
	
	const CAICHHash& GetMasterHash() const						{return m_pHashTree.m_Hash;} 
	void			SetMasterHash(const CAICHHash& Hash, EAICHStatus eNewStatus);
	bool			HasValidMasterHash() const					{return m_pHashTree.m_bHashValid;}
	bool			GetPartHashs(CAtlArray<CAICHHash>& rResult) const;
	const CAICHHashTree*	FindPartHash(uint16 nPart);

	bool			SaveHashSet();
	bool			LoadHashSet(); // Loading from known2.met

	static CAICHHashAlgo*	GetNewHashAlgo();
	static void		ClientAICHRequestFailed(CUpDownClient* pClient);
	static void		RemoveClientAICHRequest(const CUpDownClient* pClient);
	static bool		IsClientRequestPending(const CPartFile* pForFile, uint16 nPart);
	static CAICHRequestedData GetAICHReqDetails(const  CUpDownClient* pClient);
	static void		AddStoredAICHHash(CAICHHash Hash);
	bool			CanTrustMajority() const{return HasValidMasterHash() && GetStatus() != AICH_TRUSTED && GetStatus() != AICH_VERIFIED;}// X: [IP] - [Import Parts]
	bool			TrustMajority();
#ifdef _DEBUG
	void			DbgTest();
#endif
	CAICHHashTree	m_pHashTree;
	static CAtlList<CAICHRequestedData>	m_liRequestedData;
	static Poco::FastMutex				m_mutKnown2File;
	//zz_fly :: known2 buffer
	//do not update known2.met until we have buffered enough hashsets. reduce the diskio during hashing.
	static bool		SaveHashSetToFile(bool forced); //this method also called in uploadtimer.
	static Poco::FastMutex				m_mutSaveHashSet; //make sure there is only one saving process in progress.
	//zz_fly :: end
private:
	static CAtlList<CAICHHash>			m_liAICHHashsStored; // contains all AICH hashes stored in known2*.met
	CKnownFile*		m_pOwner;
	EAICHStatus		m_eStatus;
	CAtlArray<CAICHUntrustedHash> m_aUntrustedHashs;
	//zz_fly :: known2 buffer
	static CAtlList<CAICHHashTree>	m_liBufferedHashTree; //the hashsets we have buffered
	static uint32	m_nLastSaved; //last time we saved the hashsets
	static uint64	m_uBufferedSize; //buffered hashsets size
	//zz_fly :: end
};