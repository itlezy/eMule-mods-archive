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
#pragma once
#include "MapKey.h"
#include "FileIdentifier.h"

class CKnownFileList;
class CServerConnect;
class CPartFile;
class CKnownFile;
class CPublishKeywordList;
class CSafeMemFile;
class CServer;
class CCollection;

struct UnknownFile_Struct{
	CString strName;
	CString strDirectory;
	CString strSharedDirectory;
};

class CSharedFileList
{
	friend class CSharedFilesCtrl;
	friend class CClientReqSocket;

public:
	CSharedFileList(CServerConnect* in_server);
	~CSharedFileList();

	void	SendListToServer();
	void	Reload(bool bNotify);// X: [QOH] - [QueryOnHashing]
	void	Save() const;
	void	Process();
	void	Publish();
	void	DeletePartFileInstances() const;
	void	PublishNextTurn()													{ m_lastPublishED2KFlag=true;	}
	void	ClearED2KPublishInfo();
	void	ClearKadSourcePublishInfo();

	void	CreateOfferedFilePacket(CKnownFile* cur_file, CSafeMemFile* files, CServer* pServer, CUpDownClient* pClient = NULL);

	bool	SafeAddKFile(CKnownFile* toadd, bool bOnlyAdd = false);
	void	RepublishFile(CKnownFile* pFile);
	void	SetOutputCtrl(CSharedFilesCtrl* in_ctrl);
	bool	RemoveFile(CKnownFile* toremove, bool bDeleted = false);	// removes a specific shared file from the list
	void	UpdateFile(CKnownFile* toupdate);
	//void	AddFileFromNewlyCreatedCollection(const CString& rstrFilePath)		{ CheckAndAddSingleFile(rstrFilePath); }

	// GUI is not initially updated 
	bool	AddSingleSharedFile(const CString& rstrFilePath, bool bNoUpdate = false); // includes updating sharing preferences, calls CheckAndAddSingleSharedFile afterwards
	bool	AddSingleSharedDirectory(LPCTSTR rstrFilePath, bool bNoUpdate = false); 
	bool	ExcludeFile(CString strFilePath);	// excludes a specific file from being shared and removes it from the list if it exists
	
	void	AddKeywords(CKnownFile* pFile);
	void	RemoveKeywords(CKnownFile* pFile);

	CKnownFile* GetFileByID(const uchar* filehash) const;
	CKnownFile* GetFileByIdentifier(const CFileIdentifierBase& rFileIdent, bool bStrict = false) const;
#ifdef REPLACE_ATLMAP
	CKnownFile*	GetFileByIndex(size_t index);
#else
	CKnownFile*	GetFileByIndex(INT_PTR index);
#endif
	bool	IsFilePtrInList(const CKnownFile* file) const; // slow
	bool	IsUnsharedFile(const uchar* auFileHash) const;
	bool	ShouldBeShared(CString strPath, CString strFilePath, bool bMustBeShared) const;
	bool	ContainsSingleSharedFiles(CString strDirectory) const; // includes subdirs
	CString GetPseudoDirName(const CString& strDirectoryName);
	CString GetDirNameByPseudo(const CString& strPseudoName) const;

	uint64	GetDatasize(uint64 &pbytesLargest) const;
#ifdef REPLACE_ATLMAP
	size_t	GetCount()	{return m_Files_map.size(); }
#else
	INT_PTR	GetCount()	{return m_Files_map.GetCount(); }
#endif
	size_t	GetHashingCount()	{return (waitingforhash_list.GetCount()+currentlyhashing_list.GetCount()); }	// SLUGFILLER SafeHash
	bool	ProbablyHaveSingleSharedFiles() const								{ return bHaveSingleSharedFiles && !m_liSingleSharedFiles.IsEmpty(); } // might not be always up-to-date, could give false "true"s, not a problem currently
	void	HashFailed(UnknownFile_Struct* hashed);		// SLUGFILLER: SafeHash
	void	FileHashingFinished(CKnownFile* file);
	void	LoadSingleSharedFilesList();
	bool	CheckAndAddSingleFile(const CString& rstrFilePath); // add specific files without editing sharing preferences

	//bool	GetPopularityRank(const CKnownFile* pFile, uint32& rnOutSession, uint32& rnOutTotal) const;
	//Xman advanced upload-priority
	void CalculateUploadPriority(bool force=false);
	void CalculateUploadPriority_Standard();
	float m_lastavgPercent;
	uint_ptr m_avg_virtual_sources;
	uint_ptr m_avg_client_on_uploadqueue;
	//Xman end


	Poco::FastMutex	m_mutWriteList;

protected:
	bool	AddFile(CKnownFile* pFile);
	void	AddFilesFromDirectory(LPCTSTR rstrDirectory, bool bNotify);// X: [QOH] - [QueryOnHashing]
	void	FindSharedFiles(bool bNotify);// X: [QOH] - [QueryOnHashing]
	
	void	HashNextFile();
	// SLUGFILLER: SafeHash
	bool	IsHashing(const CString& rstrDirectory, const CString& rstrName);
	void	RemoveFromHashing(CKnownFile* hashed);
	// SLUGFILLER: SafeHash

	bool	CheckAndAddSingleFile(const CFileFind& ff, bool&bNotify);// X: [QOH] - [QueryOnHashing]

private:
	CKnownFilesMap m_Files_map;
	CUnsharedFilesMap m_UnsharedFiles_map;
	CStringToStringMap m_mapPseudoDirNames;
	CPublishKeywordList* m_keywords;
	CAtlList<UnknownFile_Struct*> waitingforhash_list;
	CAtlList<UnknownFile_Struct*> currentlyhashing_list;	// SLUGFILLER: SafeHash
	CServerConnect*		server;
	CSharedFilesCtrl*	output;
	CAtlList<CString>			m_liSingleSharedFiles;
	CAtlList<CString>			m_liSingleExcludedFiles;

	uint32 m_lastPublishED2K;
	bool	 m_lastPublishED2KFlag;
#ifdef REPLACE_ATLMAP
	uint_ptr m_currFileSrc;
#else
	INT_PTR m_currFileSrc;
#endif
	//int m_currFileKey;
	uint64 m_lastPublishKadSrc;// X: [64T] - [64BitTime]
	bool bHaveSingleSharedFiles;
};

class CAddFileThread : public Poco::Thread
{
public:
	CAddFileThread(CSharedFileList* pOwner, LPCTSTR directory, LPCTSTR filename, LPCTSTR strSharedDir, CPartFile* partfile = NULL);
	virtual void run();

private:
	CSharedFileList* m_pOwner;
	CString			 m_strDirectory;
	CString			 m_strFilename;
	CString			 m_strSharedDir;
	CPartFile*		 m_partfile;
};
