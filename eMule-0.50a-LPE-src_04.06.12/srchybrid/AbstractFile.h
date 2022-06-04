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
#include <list>
#include "opcodes.h"
#include "FileIdentifier.h"

/*
										CPartFile
										/
							CKnownFile
							/
			CShareableFile
			/
CAbstractFile - CCollectionFile
			\
			CSearchFile
*/
#define IsCShareableFile(pfile) (typeid(*(pfile)) == typeid(CShareableFile))
#define IsCPartFile(pfile) (typeid(*(pfile)) == typeid(CPartFile))
#define IsCKnownFile(pfile) (typeid(*(pfile)) == typeid(CKnownFile))
//#define IsKindOfCKnownFile(pfile) (IsCKnownFile(pfile) || IsCPartFile(pfile))
#define IsKindOfCKnownFile(pfile) (!IsCShareableFile(pfile))
#define IsCCollectionFile(pfile) (typeid(*(pfile)) == typeid(CCollectionFile))
#define IsCSearchFile(pfile) (typeid(*(pfile)) == typeid(CSearchFile))
namespace Kademlia
{
	class CUInt128;
	class CEntry;
	class CKadTagValueString;
	typedef std::list<CKadTagValueString> WordList;
};

class CTag;

typedef CAtlList<Kademlia::CEntry*> CKadEntryPtrList;

class CAbstractFile
#ifdef _DEBUG
	: public CObject
#endif
{
	//DECLARE_DYNAMIC(CAbstractFile)

public:
	CAbstractFile();
	CAbstractFile(const CAbstractFile* pAbstractFile);
	virtual ~CAbstractFile();

	const CString& GetFileName() const { return m_strFileName; }
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bAutoSetFileType = true, bool bRemoveControlChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!

	// returns the ED2K file type (an ASCII string)
	const CString& GetFileType() const { return m_strFileType; }
	virtual void SetFileType(LPCTSTR pszFileType);

	// returns the file type which is used to be shown in the GUI
	CString GetFileTypeDisplayStr() const;

	CFileIdentifier& GetFileIdentifier()				{ return m_FileIdentifier; }
	const CFileIdentifier& GetFileIdentifierC() const	{ return m_FileIdentifier; }
	const uchar* GetFileHash() const					{ return m_FileIdentifier.GetMD4Hash(); }
	void SetFileHash(const uchar* pucFileHash)			{ m_FileIdentifier.SetMD4Hash(pucFileHash); }
	bool HasNullHash() const;
	CString GetED2kLink(bool bHashset = false, /*bool bHTML = false, */bool bHostname = false, bool bSource = false, uint32 dwSourceIP = 0) const;


	EMFileSize		GetFileSize() const					{ return m_nFileSize; }
	virtual void	SetFileSize(EMFileSize nFileSize)	{ m_nFileSize = nFileSize; }
	bool			IsLargeFile() const					{ return m_nFileSize > (uint64)OLD_MAX_EMULE_FILE_SIZE; }

	uint32 GetIntTagValue(uint8 tagname) const;
	uint32 GetIntTagValue(LPCSTR tagname) const;
	bool GetIntTagValue(uint8 tagname, uint32& ruValue) const;
	uint64 GetInt64TagValue(uint8 tagname) const;
	uint64 GetInt64TagValue(LPCSTR tagname) const;
	bool GetInt64TagValue(uint8 tagname, uint64& ruValue) const;
	void SetIntTagValue(uint8 tagname, uint32 uValue);
	void SetInt64TagValue(uint8 tagname, uint64 uValue);
	LPCTSTR GetStrTagValue(uint8 tagname) const;
	LPCTSTR GetStrTagValue(LPCSTR tagname) const;
	void SetStrTagValue(uint8 tagname, LPCTSTR);
	CTag* GetTag(uint8 tagname, uint8 tagtype) const;
	CTag* GetTag(LPCSTR tagname, uint8 tagtype) const;
	CTag* GetTag(uint8 tagname) const;
	CTag* GetTag(LPCSTR tagname) const;
	const CAtlArray<CTag*>& GetTags() const { return taglist; }
	void AddTagUnique(CTag* pTag);
	void DeleteTag(uint8 tagname);
	void DeleteTag(CTag* pTag);
	void ClearTags();
	void CopyTags(const CAtlArray<CTag*>& tags);
	virtual bool IsPartFile() const { return false; }

	//Xman Code Improvement for choosing to use compression
	bool IsCompressible() const {return compressible;}
	void UpdateCompressible();
	//Xman end
	//Xman Code Improvement for HasCollectionExtention
	bool HasCollectionExtenesion_Xtreme() const {return m_bhasCollectionExtention;}
	//Xman end

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CFileIdentifier m_FileIdentifier;
	CString m_strFileName;
	EMFileSize	m_nFileSize;
	CString m_strFileType;
	CAtlArray<CTag*> taglist;
	//Xman Code Improvement for choosing to use compression
	bool compressible;
	//Xman Code Improvement for HasCollectionExtention
	bool m_bhasCollectionExtention;

};