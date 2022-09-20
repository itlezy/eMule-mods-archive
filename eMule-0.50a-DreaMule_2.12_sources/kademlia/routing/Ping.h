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


This work is based on the java implementation of the Kademlia protocol.
Kademlia: Peer-to-peer routing based on the XOR metric
Copyright (C) 2002  Petar Maymounkov [petar@post.harvard.edu]
http://kademlia.scs.cs.nyu.edu
*/

#pragma once

#include "../../stdafx.h"

#include "Maps.h"
#include "Timer.h"

////////////////////////////////////////
namespace Kademlia {
////////////////////////////////////////

class CRoutingZone;
class CUInt128;
class CRoutingBin;

class CPing : public CCriticalSection, public CTimerEvent
{
public:
	CPing(CRoutingZone *zone, const ContactList &test, const ContactList &replacements);

	void responded(const byte *key);

	void onTimer(void);

private:

	CRoutingZone *m_zone;
	ContactList m_test;
	ContactList m_replacements;
};

} // End namespace
