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

#pragma once
#include "../routing/Maps.h"

class CKnownFile;
class CSafeMemFile;

namespace Kademlia
{
	void deleteTagListEntries(TagList* plistTag);
	class CByteIO;
	class CSearch
	{
			friend class CSearchManager;
		public:
			uint32 GetSearchID() const;
			uint32 GetSearchTypes() const;
			void SetSearchTypes( uint32 uVal );
			void SetTargetID( CUInt128 uVal );
			uint32 GetAnswers() const;
			uint32 GetKadPacketSent() const;
			uint32 GetRequestAnswer() const;
			void StorePacket();
			byte byPacket1[1024*50];
			byte byPacket2[1024*50];
			byte byPacket3[1024*50];
			CByteIO *pbyIO1;
			CByteIO *pbyIO2;
			CByteIO *pbyIO3;
			const CString& GetFileName() const;
			void SetFileName(const CString& sFileName);
			CUInt128 GetTarget() const;
			void AddFileID(const CUInt128& uID);
			void PreparePacket();
			void PreparePacketForTags( CByteIO* pbyPacket, CKnownFile* pFile );
			bool Stoping() const;
			uint32 GetNodeLoad() const;
			uint32 GetNodeLoadResonse() const;
			uint32 GetNodeLoadTotal() const;
			void UpdateNodeLoad( uint8 uLoad );
			enum
			{
			    NODE,
			    NODECOMPLETE,
			    FILE,
			    KEYWORD,
			    NOTES,
			    STOREFILE,
			    STOREKEYWORD,
			    STORENOTES,
			    FINDBUDDY,
			    FINDSOURCE
		};

			CSearch();
			~CSearch();

		private:
			void Go();
			void ProcessResponse(uint32 uFromIP, uint16 uFromPort, ContactList *plistResults);
			void ProcessResult(const CUInt128 &uAnswer, TagList *listInfo);
			void ProcessResultFile(const CUInt128 &uAnswer, TagList *listInfo);
			void ProcessResultKeyword(const CUInt128 &uAnswer, TagList *listInfo);
			void ProcessResultNotes(const CUInt128 &uAnswer, TagList *listInfo);
			void JumpStart();
			void SendFindValue(CContact* pContact);
			void PrepareToStop();

			bool m_bStoping;
			time_t m_tCreated;
			uint32 m_uType;
			uint32 m_uAnswers;
			uint32 m_uTotalRequestAnswers;
			uint32 m_uKadPacketSent; //Used for gui reasons.. May not be needed later..
			uint32 m_uTotalLoad;
			uint32 m_uTotalLoadResponses;
			uint32 m_uLastResponse;
			uint32 m_uSearchID;
			CUInt128 m_uTarget;
			CSafeMemFile *m_pfileSearchTerms;
			WordList m_listWords;
			CString m_sFileName;
			UIntList m_listFileIDs;
			ContactMap m_mapPossible;
			ContactMap m_mapTried;
			ContactMap m_mapResponded;
			ContactMap m_mapBest;
			ContactList m_listDelete;
			ContactMap m_mapInUse;
	};
}
void KadGetKeywordHash(const CStringW& rstrKeywordW, Kademlia::CUInt128* puKadID);
void KadGetKeywordHash(const CStringA& rstrKeywordA, Kademlia::CUInt128* puKadID);
CStringA KadGetKeywordBytes(const CStringW& rstrKeywordW);
