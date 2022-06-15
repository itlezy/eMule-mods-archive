//this file is part of NeoMule
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

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->

#pragma once

class CSafeMemFile;
class CKnownFile;

struct SSearchTree
{
	SSearchTree(){
		type = AND;
		left = NULL;
		right = NULL;
	}

	~SSearchTree(){
		if (type == String)
			delete astr;
	}
	
	enum ESearchTermType {
		AND,
		OR,
		NAND,
		String,
	} type;
	
	CStringWArray* astr;

	SSearchTree* left;
	SSearchTree* right;
};

struct SSearchRoot
{
	SSearchRoot()
	{
		pSearchTerms = NULL;
		ullMinSize = 0;
		ullMaxSize = 0;
		uAvailability = 0;
		uComplete = 0;
		//ulMinBitrate = 0;
		//ulMinLength = 0;
		bUnicode = true;
	}

	SSearchTree* pSearchTerms;

	CString strFileType;
	uint64 ullMinSize;
	uint64 ullMaxSize;
	UINT uAvailability;
	CString strExtension;
	UINT uComplete;
	//CString strCodec;
	//ULONG ulMinBitrate;
	//ULONG ulMinLength;
	//CString strTitle;
	//CString strAlbum;
	//CString strArtist;
	//CString strSpecialTitle;
	bool bUnicode;
};

static CString* _pstrDbgSearchExpr_;

SSearchTree* CreateSearchTree(CSafeMemFile& bio, SSearchRoot* pSearchRoot, int iLevel = 0);
bool SearchRootMatch(const SSearchRoot* pSearch, const CKnownFile* kFile);
bool SearchTreeMatch(const SSearchTree* pSearchTerm, const CStringW str);
void FreeTree(SSearchTree* pSearchTerms);

#endif //LANCAST // NEO: NLC END <-- Xanatos --
