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
#include "map_inc.h"
#include "../utils/UInt128.h"

namespace Kademlia
{
	struct TrackPackets_Struct{
		uint32 dwIP;
		uint32 dwInserted;
		uint8  byOpcode;
	};

	struct TrackChallenge_Struct{
		uint32 uIP;
		uint32 dwInserted;
		uint8  byOpcode;
		CUInt128 uContactID;
		CUInt128 uChallenge;
	};

	struct TrackPacketsIn_Struct{
		struct TrackedRequestIn_Struct{ // netfinity: Rearranged for alignment reasons
			uint32	m_nCount;
			uint32	m_dwFirstAdded;
			bool	m_bDbgLogged;
			uint8	m_byOpcode;
		};

		TrackPacketsIn_Struct() { m_dwLastExpire = 0; m_uIP = 0; }

		// netfinity: Rearranged for alignment reasons
		CAtlArray<TrackedRequestIn_Struct> m_aTrackedRequests;
		uint32	m_uIP;
		uint32	m_dwLastExpire;	
	};

	class CPacketTracking
	{
		public:
			CPacketTracking();
			virtual ~CPacketTracking();

		protected:
			void AddTrackedOutPacket(uint32 dwIP, uint8 byOpcode);
			bool IsOnOutTrackList(uint32 dwIP, uint8 byOpcode, bool bDontRemove = false);
			bool InTrackListIsAllowedPacket(uint32 uIP, uint8 byOpcode, bool bValidReceiverkey);
			void InTrackListCleanup();
			void AddLegacyChallenge(CUInt128 uContactID, CUInt128 uChallengeID, uint32 uIP, uint8 byOpcode);
			bool IsLegacyChallenge(CUInt128 uChallengeID, uint32 uIP, uint8 byOpcode, CUInt128& ruContactID);
			bool HasActiveLegacyChallenge(uint32 uIP) const;

		private:
			bool IsTrackedOutListRequestPacket(uint8 byOpcode) const;
			CAtlList<TrackPackets_Struct> listTrackedRequests;
			CAtlList<TrackChallenge_Struct> listChallengeRequests;
			CAtlList<TrackPacketsIn_Struct*>					m_liTrackPacketsIn;
#ifdef REPLACE_ATLMAP
			unordered_map<int, TrackPacketsIn_Struct*>	m_mapTrackPacketsIn;
#else
			CAtlMap<int, TrackPacketsIn_Struct*>	m_mapTrackPacketsIn;
#endif
			uint32 dwLastTrackInCleanup;
	};
}