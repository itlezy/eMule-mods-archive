//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "types.h"

class CUpDownClient;

class CFriend
{
public:
						CFriend(void);
						CFriend(CUpDownClient *client);
						CFriend( uchar t_m_abyUserhash[16], uint32 tm_dwLastSeen,
								 uint32 tm_dwLastUsedIP, uint32 tm_nLastUsedPort,
								 uint32 tm_dwLastChatted, CString tm_strName,
								 uint32 tm_dwHasHash );
					   ~CFriend(void);
	uint32				m_dwLastUsedIP;
	uint32				m_nLastUsedPort;
	uint32				m_dwLastChatted;
	uint32				m_dwHasHash;
	CString				m_strName;


	CUpDownClient*		GetLinkedClient() const		{ return m_pLinkedClient; }
	uint32				GetLastSeen() const			{ return m_dwLastSeen; }
	const uchar*		GetUserHash() const			{ return m_abyUserhash; }
	void				SetLinkedClient(CUpDownClient *pLinkedClient);

	void				LoadFromFile(CFile &file);
	void				WriteToFile(CFile &file);

private:
	uchar				m_abyUserhash[16];
	uint32				m_dwLastSeen;
	CUpDownClient		*m_pLinkedClient;
};
