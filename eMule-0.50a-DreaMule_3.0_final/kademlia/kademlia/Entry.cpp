/*
Copyright (C)2003 Barry Dunne (http://www.emule-project.net)
 
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Note To Mods //
/*
Please do not change anything here and release it..
There is going to be a new forum created just for the Kademlia side of the client..
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it.. If it is a real improvement,
it will be added to the offical client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/

#include "stdafx.h"
#include "./Entry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Kademlia;

CEntry::CEntry()
{
	m_uIP = 0;
	m_uTCPPort = 0;
	m_uUDPPort = 0;
	// JOHNTODO - Maybe it's time to loose the Filename and Size member vars and just use the
	// data in the tags to save memory.. Possibly even other member vars..
	(void)m_fileName;
	m_uSize = 0;
	m_tLifetime = 0;
	m_bSource = false;
}

CEntry::~CEntry()
{
	for (TagList::const_iterator itTagList = m_listTag.begin(); itTagList != m_listTag.end(); itTagList++)
		delete *itTagList;
}

CEntry* CEntry::Copy()
{
	CEntry* pEntry = new CEntry();
	pEntry->m_fileName = m_fileName;
	pEntry->m_uIP = m_uIP;
	pEntry->m_uKeyID.SetValue(m_uKeyID);
	pEntry->m_tLifetime = m_tLifetime;
	pEntry->m_uSize = m_uSize;
	pEntry->m_bSource = m_bSource;
	pEntry->m_uSourceID.SetValue(m_uSourceID);
	pEntry->m_uTCPPort = m_uTCPPort;
	pEntry->m_uUDPPort = m_uUDPPort;
	for (TagList::const_iterator itTagList = m_listTag.begin(); itTagList != m_listTag.end(); itTagList++)
	{
		CKadTag* pTag = *itTagList;
		pEntry->m_listTag.push_back(pTag->Copy());
	}
	return pEntry;
}

uint64 CEntry::GetIntTagValue(LPCSTR szTagName) const
{
	for (TagList::const_iterator itTagList = m_listTag.begin(); itTagList != m_listTag.end(); itTagList++)
	{
		CKadTag* pTag = *itTagList;
		if (!pTag->m_name.Compare(szTagName)&& pTag->IsInt())
			return pTag->GetInt();
	}
	return 0;
}

CStringW CEntry::GetStrTagValue(LPCSTR szTagName) const
{
	for (TagList::const_iterator itTagList = m_listTag.begin(); itTagList != m_listTag.end(); itTagList++)
	{
		CKadTag* pTag = *itTagList;
		if (!pTag->m_name.Compare(szTagName)&& pTag->IsStr())
			return pTag->GetStr();
	}
	return _T("");
}
