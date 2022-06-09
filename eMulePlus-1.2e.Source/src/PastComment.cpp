//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#ifndef NEW_SOCKETS_ENGINE
#include "emule.h"
#else
#include "opcodes.h"
#endif //NEW_SOCKETS_ENGINE
#include "PastComment.h"
#include "otherfunctions.h"

static const CString strPastCommentNul(_T(""));

CPastComment::CPastComment() : m_eRating(PF_RATING_NONE)
{
	md4clr(m_ClientHash);
}

CPastComment::CPastComment(const uchar client[16]) : m_eRating(PF_RATING_NONE)
{
	md4cpy(m_ClientHash, client);
}

CPastComment::CPastComment(const uchar client[16], const CString &strClientName, const CString &strFileName, const CString &strComment, EnumPartFileRating eRating)
	:  m_strClientName(strClientName), m_strFileName(strFileName), m_strComment(strComment), m_eRating(eRating)
{ 
	md4cpy(m_ClientHash, client);
}

bool CPastComment::operator==(const CPastComment &pc) const
{
	return (md4cmp(m_ClientHash, pc.m_ClientHash) == 0);
}

bool CPastCommentList::GetCommentRating(const uchar client[16], CString *pstrComment, EnumPartFileRating *peRate) const
{
	POSITION		pos;

	if ((pos = FindComment(client)) == NULL)
	{
		*pstrComment = _T("");
		*peRate = PF_RATING_NONE;
		return false;
	}
	CPastComment	pc = GetAt(pos);

	*pstrComment = pc.GetComment();
	*peRate = pc.GetRating();
	return true;
}

const CString& CPastCommentList::GetFileName(const uchar client[16])
{
	POSITION pos;
	if ((pos = FindComment(client)) == NULL)
		return strPastCommentNul;
	return GetAt(pos).GetFileName();
}

const CString& CPastCommentList::GetComment(const uchar client[16])
{
	POSITION pos;
	if ((pos = FindComment(client)) == NULL)
		return strPastCommentNul;
	return GetAt(pos).GetComment();
}

EnumPartFileRating CPastCommentList::GetRate(const uchar client[16])
{
	POSITION pos;
	if ((pos = FindComment(client)) == NULL)
		return PF_RATING_NONE;
	return GetAt(pos).GetRating();
}

void CPastCommentList::Add(const uchar client[16], const CString &strClientName, const CString &strFileName, const CString &strComment, EnumPartFileRating eRating)
{
	POSITION pos;
	CPastComment pc(client, strClientName, strFileName, strComment, eRating);
	if ((pos = Find(pc)) == NULL)
		AddTail(pc);
	else
		SetAt(pos, pc);
}

void CPastCommentList::Remove(const uchar client[16])
{
	POSITION pos;
	if ((pos = FindComment(client)) == NULL)
		return;
	RemoveAt(pos);
}

POSITION CPastCommentList::FindComment(const uchar client[16]) const
{
	CNode	*pNode = m_pNodeHead;

	for (; pNode != NULL; pNode = pNode->pNext)
		if (md4cmp(pNode->data.GetClientHash(), client) == 0)
			return (POSITION)pNode;
	return NULL;
}
