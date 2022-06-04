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

// Note To Mods //
/*
Please do not change anything here and release it..
There is going to be a new forum created just for the Kademlia side of the client..
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it.. If it is a real improvement,
it will be added to the official client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/

#pragma once
#include "./Tag.h"
#include "../../shahashset.h"

struct SSearchTerm;
namespace Kademlia
{
	class CDataIO;
	class CEntry
	{
		protected:
			struct structFileNameEntry{
				CKadTagValueString	m_fileName;
				uint32				m_uPopularityIndex;
			};
		public:
						CEntry();
			virtual		~CEntry();

			virtual		CEntry* Copy();
			virtual bool IsKeyEntry()					{ return false; }

			uint64		GetIntTagValue(CKadTagNameString strTagName, bool bIncludeVirtualTags = true) const;
			bool		GetIntTagValue(CKadTagNameString strTagName, uint64& rValue, bool bIncludeVirtualTags = true) const;
			CKadTagValueString GetStrTagValue(CKadTagNameString strTagName) const;
			void		AddTag(CKadTag* pTag, uint32 uDbgSourceIP = 0);
			size_t		GetTagCount() const; // Adds filename and size to the count if not empty, even if they are not stored as tags
			void		WriteTagList(CDataIO* pData)	{ WriteTagListInc(pData, 0); }

			CKadTagValueString	GetCommonFileNameLowerCase() const;
			CKadTagValueString	GetCommonFileName() const;
			void		SetFileName(CKadTagValueString strName);

			// netfinity: Rearranged for alignment reasons
			CUInt128 m_uKeyID;
			CUInt128 m_uSourceID;
			uint64	m_uSize;
			time_t	m_tLifetime;
			uint32 m_uIP;
			uint16 m_uTCPPort;
			uint16 m_uUDPPort;
			bool	m_bSource;

		protected:
			void		WriteTagListInc(CDataIO* pData, size_t nIncreaseTagNumber = 0);
			CAtlList<structFileNameEntry> m_listFileNames;	
			TagList m_listTag;
	};

	class CKeyEntry : public CEntry
	{
		protected:
			struct structPublishingIP{ // netfinity: Rearranged for alignment reasons
				time_t				m_tLastPublish;
				uint32				m_uIP;
				uint16				m_byAICHHashIdx;
			};
		public:
			CKeyEntry();
			virtual ~CKeyEntry();

			virtual	CEntry*		Copy();
			virtual bool IsKeyEntry()					{ return true; }

			bool				StartSearchTermsMatch(const SSearchTerm* pSearchTerm);
			void				MergeIPsAndFilenames(CKeyEntry* pFromEntry);
			void				CleanUpTrackedPublishers();
			float				GetTrustValue();
			void				WritePublishTrackingDataToFile(CDataIO* pData);
			void				ReadPublishTrackingDataFromFile(CDataIO* pData, bool bIncludesAICH);
			void				DirtyDeletePublishData();
			void				WriteTagListWithPublishInfo(CDataIO* pData);
			uint16				AddRemoveAICHHash(const CAICHHash& hash, bool bAdd);
			uint16				GetAICHHashCount() const						{ return (uint16)m_aAICHHashs.GetCount(); }
#ifdef REPLACE_ATLMAP
			static void			ResetGlobalTrackingMap()						{ unordered_map<uint32, uint32>().swap(s_mapGlobalPublishIPs); }
#else
			static void			ResetGlobalTrackingMap()						{ s_mapGlobalPublishIPs.RemoveAll(); }
#endif
		protected:
			void				RecalcualteTrustValue();
			static void			AdjustGlobalPublishTracking(uint32 uIP, bool bIncrease/*, CString strDbgReason*/);
			
			// netfinity: Rearranged for alignment reasons
			CAtlList<structPublishingIP> *m_pliPublishingIPs;
#ifdef REPLACE_ATLMAP
			static unordered_map<uint32, uint32> s_mapGlobalPublishIPs; // tracks count of publishings for each 255.255.255.0/28 subnet
#else
			static CAtlMap<uint32, uint32> s_mapGlobalPublishIPs; // tracks count of publishings for each 255.255.255.0/28 subnet
#endif
			
			CAtlArray<uint8>		m_anAICHHashPopularity;
			CAtlArray<CAICHHash>	m_aAICHHashs;

			uint32	dwLastTrustValueCalc;			
			float	m_fTrustValue;
			bool SearchTermsMatch(const SSearchTerm* pSearchTerm) const;
			CKadTagValueString m_strSearchTermCacheCommonFileNameLowerCase; // contains a valid value only while 'SearchTermsMatch' is running.
	};
}
