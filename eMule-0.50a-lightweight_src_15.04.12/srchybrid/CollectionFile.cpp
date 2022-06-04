//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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

#include "StdAfx.h"
#include "collectionfile.h"
#include "OtherFunctions.h"
#include "Packets.h"
#include "Ed2kLink.h"
#include "resource.h"
#include "Log.h"
#include "Kademlia/Kademlia/Entry.h"
#include "Kademlia/Kademlia/Tag.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//IMPLEMENT_DYNAMIC(CCollectionFile, CAbstractFile)

CCollectionFile::CCollectionFile(void)
{
}

CCollectionFile::CCollectionFile(CFileDataIO* in_data)
{
	UINT tagcount = in_data->ReadUInt32();

	for (UINT i = 0; i < tagcount; i++)
	{
		CTag* toadd = new CTag(in_data, true);
		if (toadd)
			taglist.Add(toadd);
	}
	
	CTag* pTagHash = GetTag(FT_FILEHASH);
	if(pTagHash)
		SetFileHash(pTagHash->GetHash());
	else
		ASSERT(0);
	
	pTagHash = GetTag(FT_AICH_HASH);
	if (pTagHash != NULL && pTagHash->IsStr())
	{
		CAICHHash hash;
		if (DecodeBase32(pTagHash->GetStr(), hash) == HASHSIZE)
			m_FileIdentifier.SetAICHHash(hash);
		else
			ASSERT( false );
	}
	
	// here we have two choices
	//	- if the server/client sent us a filetype, we could use it (though it could be wrong)
	//	- we always trust our filetype list and determine the filetype by the extension of the file
	//
	// if we received a filetype from server, we use it.
	// if we did not receive a filetype, we determine it by examining the file's extension.
	//
	// but, in no case, we will use the receive file type when adding this search result to the download queue, to avoid
	// that we are using 'wrong' file types in part files. (this has to be handled when creating the part files)
	LPCTSTR rstrFileType = GetStrTagValue(FT_FILETYPE);
	SetFileName(GetStrTagValue(FT_FILENAME), false, rstrFileType[0] == 0);
	SetFileSize(GetInt64TagValue(FT_FILESIZE));
	if (rstrFileType[0] != 0)
	{
		if (_tcscmp(rstrFileType, _T(ED2KFTSTR_PROGRAM))==0)
		{
			CString strDetailFileType = GetFileTypeByName(GetFileName());
			if (!strDetailFileType.IsEmpty())
				SetFileType(strDetailFileType);
			else
				SetFileType(rstrFileType);
		}
		else
			SetFileType(rstrFileType);
	}

	if(GetFileSize() == (uint64)0 || !GetFileName().Compare(_T("")))
		ASSERT(0);
}

CCollectionFile::CCollectionFile(CAbstractFile* pAbstractFile) : CAbstractFile(pAbstractFile)
{
	ClearTags();

	taglist.Add(new CTag(FT_FILEHASH, pAbstractFile->GetFileHash()));
	taglist.Add(new CTag(FT_FILESIZE, pAbstractFile->GetFileSize(), true));
	taglist.Add(new CTag(FT_FILENAME, pAbstractFile->GetFileName()));

	if (m_FileIdentifier.HasAICHHash())
		taglist.Add(new CTag(FT_AICH_HASH, m_FileIdentifier.GetAICHHash().GetString()));
}

bool CCollectionFile::InitFromLink(CString sLink)
{
	CED2KLink* pLink = NULL;
	CED2KFileLink* pFileLink = NULL;
	try 
	{
		pLink = CED2KLink::CreateLinkFromUrl(sLink);
		/*if(!pLink)
			throw GetResString(IDS_ERR_NOTAFILELINK);
		pFileLink = pLink->GetFileLink();*/
		if(!pLink || pLink->GetKind() != CED2KLink::kFile)
			throw GetResString(IDS_ERR_NOTAFILELINK);
		pFileLink = (CED2KFileLink*)pLink;
	} 
	catch (CString error) 
	{
			delete pLink;
		CString strBuffer;
		strBuffer.Format(GetResString(IDS_ERR_INVALIDLINK),error);
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), strBuffer);
		return false;
	}

	taglist.Add(new CTag(FT_FILEHASH, pFileLink->GetHashKey()));
	m_FileIdentifier.SetMD4Hash(pFileLink->GetHashKey());

	taglist.Add(new CTag(FT_FILESIZE, pFileLink->GetSize(), true));
	SetFileSize(pFileLink->GetSize());

	taglist.Add(new CTag(FT_FILENAME, pFileLink->GetName()));
	SetFileName(pFileLink->GetName(), false, false);

	if (pFileLink->HasValidAICHHash())
	{
		taglist.Add(new CTag(FT_AICH_HASH, pFileLink->GetAICHHash().GetString()));
		m_FileIdentifier.SetAICHHash(pFileLink->GetAICHHash());
	}

	delete pLink;
	return true;
}

CCollectionFile::~CCollectionFile(void)
{
}

void CCollectionFile::WriteCollectionInfo(CFileDataIO *out_data)
{
	out_data->WriteUInt32((uint32)taglist.GetCount());

	for (size_t i = 0; i < taglist.GetCount(); i++)
	{
		CTag tempTag(*taglist.GetAt(i));
		tempTag.WriteNewEd2kTag(out_data, utf8strRaw);
	}
}