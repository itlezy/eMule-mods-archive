/*
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

struct SSearchTerm
{
	SSearchTerm();
	~SSearchTerm();

	enum ESearchTermType
	{
	    AND,
	    OR,
	    NOT,
	    String,
	    MetaTag,
	    OpGreaterEqual,
	    OpLessEqual,
	    OpGreater,
	    OpLess,
	    OpEqual,
	    OpNotEqual
	} m_type;
	Kademlia::CKadTag* m_pTag;
	CStringWArray* m_pastr;
	SSearchTerm* m_pLeft;
	SSearchTerm* m_pRight;
};

namespace Kademlia
{

	class CIndexed
	{
		public:
			CIndexed();
			~CIndexed();

			bool AddKeyword(const CUInt128& uKeyWordID, const CUInt128& uSourceID, Kademlia::CEntry* pEntry, uint8& uLoad);
			bool AddSources(const CUInt128& uKeyWordID, const CUInt128& uSourceID, Kademlia::CEntry* pEntry, uint8& uLoad);
			bool AddNotes(const CUInt128& uKeyID, const CUInt128& uSourceID, Kademlia::CEntry* pEntry, uint8& uLoad);
			bool AddLoad(const CUInt128& uKeyID, uint32 uTime);
			uint32 GetFileKeyCount();
			void SendValidKeywordResult(const CUInt128& uKeyID, const SSearchTerm* pSearchTerms, uint32 uIP, uint16 uPort, bool bOldClient, bool bKad2, uint16 uStartPosition);
			void SendValidSourceResult(const CUInt128& uKeyID, uint32 uIP, uint16 uPort, bool bKad2, uint16 uStartPosition, uint64 uFileSize);
			void SendValidNoteResult(const CUInt128& uKeyID, uint32 uIP, uint16 uPort, bool bKad2, uint64 uFileSize);
			bool SendStoreRequest(const CUInt128& uKeyID);
			uint32 m_uTotalIndexSource;
			uint32 m_uTotalIndexKeyword;
			uint32 m_uTotalIndexNotes;
			uint32 m_uTotalIndexLoad;
		private:
			void ReadFile(void);
			void Clean(void);
			time_t m_tLastClean;
			CKeyHashMap m_mapKeyword;
			CSrcHashMap m_mapSources;
			CSrcHashMap m_mapNotes;
			CLoadMap m_mapLoad;
			static CString	m_sSourceFileName;
			static CString	m_sKeyFileName;
			static CString	m_sLoadFileName;
	};
}
