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

#include "StdAfx.h"
#include "emule.h"
#include "LanSearch.h"
#include "packets.h"
#include "otherfunctions.h"
#include "log.h"
#include "KnownFile.h"
#include "PartFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->

LPCWSTR _awszInvLanKeywordChars_ = L"\\/:*?\"<>|";

#define EMPTY_SEARCHTREE (SSearchTree*)0xFFFFFFFF

void KadTagStrMakeLower(CStringW& rwstr);


SSearchTree* CreateSearchTree(CSafeMemFile& bio, SSearchRoot* pSearchRoot, int iLevel)
{
	// the max. depth has to match our own limit for creating the search expression 
	// (see also 'ParsedSearchExpression' and 'GetSearchPacket')
	if (iLevel >= 24){
		AddDebugLogLine(false, _T("***NOTE: Search expression tree exceeds depth limit!"));
		return NULL;
	}
	iLevel++;

	uint8 op = bio.ReadUInt8();
	if (op == 0x00)
	{
		SSearchTree* pSearchTerm = new SSearchTree;
		uint8 boolop = bio.ReadUInt8();
		switch(boolop){
			case 0x00:  // AND
				pSearchTerm->type = SSearchTree::AND;
				if (_pstrDbgSearchExpr_)
					_pstrDbgSearchExpr_->Append(_T(" AND"));
				break;
			case 0x01: // OR
				pSearchTerm->type = SSearchTree::OR;
				if (_pstrDbgSearchExpr_)
					_pstrDbgSearchExpr_->Append(_T(" OR"));
				break;
			case 0x02: // NAND
				pSearchTerm->type = SSearchTree::NAND;
				if (_pstrDbgSearchExpr_)
					_pstrDbgSearchExpr_->Append(_T(" NOT"));
				break;
			default:
				AddDebugLogLine(false, _T("*** Unknown boolean search operator 0x%02x (CreateSearchTree)"), boolop);
				delete pSearchTerm;
				return NULL;
		}

		if ((pSearchTerm->left = CreateSearchTree(bio, pSearchRoot, iLevel)) == NULL){
			ASSERT(0);
			delete pSearchTerm;
			return NULL;
		}

		if ((pSearchTerm->right = CreateSearchTree(bio, pSearchRoot, iLevel)) == NULL){
			ASSERT(0);
			FreeTree(pSearchTerm->left);
			delete pSearchTerm;
			return NULL;
		}

		if(pSearchTerm->left == EMPTY_SEARCHTREE && pSearchTerm->right != EMPTY_SEARCHTREE)
		{
			SSearchTree* todel = pSearchTerm;
			pSearchTerm = pSearchTerm->right;
			delete todel;
		}
		else if(pSearchTerm->left != EMPTY_SEARCHTREE && pSearchTerm->right == EMPTY_SEARCHTREE)
		{
			SSearchTree* todel = pSearchTerm;
			pSearchTerm = pSearchTerm->left;
			delete todel;
		}
		else if(pSearchTerm->left == EMPTY_SEARCHTREE && pSearchTerm->right == EMPTY_SEARCHTREE)
		{
			delete pSearchTerm;
			pSearchTerm = EMPTY_SEARCHTREE;
		}

		return pSearchTerm;
	}
	else if (op == 0x01) // String
	{
		CStringW str(bio.ReadStringUTF8());

		KadTagStrMakeLower(str); // make lowercase, the search code expects lower case strings!
		if (_pstrDbgSearchExpr_)
			_pstrDbgSearchExpr_->AppendFormat(_T(" \"%ls\""), str);

		SSearchTree* pSearchTerm = new SSearchTree;
		pSearchTerm->type = SSearchTree::String;
		pSearchTerm->astr = new CStringWArray;

		// pre-tokenize the string term
		int iPosTok = 0;
		CStringW strTok(str.Tokenize(_awszInvLanKeywordChars_, iPosTok));
		while (!strTok.IsEmpty())
		{
			pSearchTerm->astr->Add(strTok);
			strTok = str.Tokenize(_awszInvLanKeywordChars_, iPosTok);
		}

		return pSearchTerm;
	}
	else if (op == 0x02) // Meta tag
	{
		// read tag value
		CStringW strValue(bio.ReadStringUTF8());

		KadTagStrMakeLower(strValue); // make lowercase, the search code expects lower case strings!

		// read tag name
		CStringA strTagName;
		uint16 lenTagName = bio.ReadUInt16();
		bio.Read(strTagName.GetBuffer(lenTagName), lenTagName);
		strTagName.ReleaseBuffer(lenTagName);

		switch((BYTE)strTagName[0]){
			case FT_FILEFORMAT:
				pSearchRoot->strExtension = strValue;
				break;
			case FT_FILETYPE:
				pSearchRoot->strFileType = strValue;
				break;
			default:
				AddDebugLogLine(false, _T("*** Unknown search Meta Tag  0x%02x (CreateSearchTree)"), (BYTE)strTagName[0]);
		}

		if (_pstrDbgSearchExpr_) {
			if (lenTagName == 1)
				_pstrDbgSearchExpr_->AppendFormat(_T(" Tag%02X=\"%ls\""), (BYTE)strTagName[0], strValue);
			else
				_pstrDbgSearchExpr_->AppendFormat(_T(" \"%s\"=\"%ls\""), strTagName, strValue);
		}
		return EMPTY_SEARCHTREE;
	}
	else if (op == 0x03 || op == 0x08) // Numeric Relation (0x03=32-bit or 0x08=64-bit)
	{
		// read tag value
		uint64 ullValue = (op == 0x03) ? bio.ReadUInt32() : bio.ReadUInt64();

		// read integer operator
		uint8 mmop = bio.ReadUInt8();
		if (mmop >= 0x06){
			AddDebugLogLine(false, _T("*** Unknown integer search op=0x%02x (CreateSearchTree)"), mmop);
			return NULL;
		}

		// read tag name
		CStringA strTagName;
		uint16 lenTagName = bio.ReadUInt16();
		bio.Read(strTagName.GetBuffer(lenTagName), lenTagName);
		strTagName.ReleaseBuffer(lenTagName);

		switch((BYTE)strTagName[0]){
			case FT_SOURCES:
				pSearchRoot->uAvailability = (UINT)ullValue;
				break;
			case FT_COMPLETE_SOURCES:
				pSearchRoot->uComplete = (UINT)ullValue;
				break;
			case FT_FILESIZE:
				switch(mmop){
					case 0x00: //SSearchTree::OpEqual:
						pSearchRoot->ullMinSize = ullValue;
						pSearchRoot->ullMaxSize = ullValue;
						break;
					case 0x01: //SSearchTree::OpGreaterEqual:
						pSearchRoot->ullMinSize = ullValue;
						break;
					case 0x02: //SSearchTree::OpLessEqual:
						pSearchRoot->ullMaxSize = ullValue;
						break;
					case 0x03: //SSearchTree::OpGreater:
						pSearchRoot->ullMinSize = ullValue+1;
						break;
					case 0x04: //SSearchTree::OpLess:
						pSearchRoot->ullMaxSize = ullValue-1;
						break;
					case 0x05: //SSearchTree::OpNotEqual:
						break;
				}
				break;
			default:
				AddDebugLogLine(false, _T("*** Unknown search Min Max Tag  0x%02x (CreateSearchTree)"), (BYTE)strTagName[0]);
		}

		if (_pstrDbgSearchExpr_) {
			if (lenTagName == 1)
				_pstrDbgSearchExpr_->AppendFormat(_T(" Tag%02X%I64u"), (BYTE)strTagName[0], ullValue);
			else
				_pstrDbgSearchExpr_->AppendFormat(_T(" \"%s\"%I64u"), strTagName, ullValue);
		}
		return EMPTY_SEARCHTREE;
	}
	else{
		AddDebugLogLine(false, _T("*** Unknown search op=0x%02x (CreateSearchTree)"), op);
		return NULL;
	}
}

bool SearchTreeMatch(const SSearchTree* pSearchTerm, const CStringW str)
{
	// boolean operators
	if (pSearchTerm->type == SSearchTree::AND)
		return SearchTreeMatch(pSearchTerm->left, str) && SearchTreeMatch(pSearchTerm->right, str);
	
	if (pSearchTerm->type == SSearchTree::OR)
		return SearchTreeMatch(pSearchTerm->left, str) || SearchTreeMatch(pSearchTerm->right, str);
	
	if (pSearchTerm->type == SSearchTree::NAND)
		return SearchTreeMatch(pSearchTerm->left, str) && !SearchTreeMatch(pSearchTerm->right, str);

	// word which is to be searched in the file name (and in additional meta data as done by some ed2k servers???)
	if (pSearchTerm->type == SSearchTree::String)
	{
		int iStrSearchTerms = pSearchTerm->astr->GetCount();
		if (iStrSearchTerms == 0)
			return false;

		// if there are more than one search strings specified (e.g. "aaa bbb ccc") the entire string is handled
		// like "aaa AND bbb AND ccc". search all strings from the string search term in the tokenized list of
		// the file name. all strings of string search term have to be found (AND)
		for (int iSearchTerm = 0; iSearchTerm < iStrSearchTerms; iSearchTerm++)
		{
			// this will not give the same results as when tokenizing the filename string, but it is 20 times faster.
			if (wcsstr(str, pSearchTerm->astr->GetAt(iSearchTerm)) == NULL)
				return false;
		}

		return true;
	}

	return false;
}

bool SearchRootMatch(const SSearchRoot* pSearch, const CKnownFile* kFile)
{
	CStringW str(kFile->GetFileName());

	KadTagStrMakeLower(str); // make lowercase, the search code expects lower case strings!

	if(SearchTreeMatch(pSearch->pSearchTerms,str) == false)
		return false;

	if(pSearch->ullMaxSize){
		if(kFile->GetFileSize() > pSearch->ullMaxSize)
			return false;
	}
	
	if(pSearch->ullMinSize){
		if(kFile->GetFileSize() < pSearch->ullMinSize)
			return false;
	}

	if(pSearch->uAvailability){
		UINT uSourceCount = 1 + kFile->IsPartFile() ? ((const CPartFile*)kFile)->GetSourceCount() : const_cast<CKnownFile*>(kFile)->GetQueuedCount();
		if(uSourceCount < pSearch->uAvailability)
			return false;
	}

	if(pSearch->uComplete){
		if(kFile->m_nCompleteSourcesCount < pSearch->uComplete)
			return false;
	}

	if(pSearch->strFileType){
		if (wcsstr(kFile->GetFileType(), pSearch->strFileType) == NULL)
			return false;
	}

	if(!pSearch->strExtension.IsEmpty()){
		CStringW ext = str.Right(str.ReverseFind('.'));

		if (wcsstr(ext, pSearch->strExtension) == NULL)
			return false;
	}

	return true;
}

void FreeTree(SSearchTree* pSearchTerms)
{
	if(pSearchTerms)
	{
		if (pSearchTerms->left)
			FreeTree(pSearchTerms->left);
		if (pSearchTerms->right)
			FreeTree(pSearchTerms->right);
		delete pSearchTerms;
	}
}

#endif //LANCAST // NEO: NLC END <-- Xanatos --
