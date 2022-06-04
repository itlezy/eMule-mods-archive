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
it will be added to the official client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/

#pragma once

namespace Kademlia
{
	class CUInt128
	{
		public:
			// netfinity: Inlined default constructor
			CUInt128() : m_uData0(0), m_uData1(0), m_uData2(0), m_uData3(0) {} // netfinity: Shouldn't really initialize here but kept temporarily for backwards compability
			// netfinity: Special constructor, can be used to avoid copying of return values
			CUInt128(uint32 hhword, uint32 hlword, uint32 lhword, uint32 llword) : m_uData0(hhword), m_uData1(hlword), m_uData2(lhword), m_uData3(llword) {}
			CUInt128(bool bFill);
			CUInt128(ULONG uValue);
			CUInt128(const byte *pbyValueBE);
			//Generates a new number, copying the most significant 'numBits' bits from 'value'.
			//The remaining bits are randomly generated.
			CUInt128(const CUInt128 &uValue, uint_ptr uNumBits/* = 128*/);
			// netfinity: Copy constructor - This one is extra fast as it is completly inline!!!
			CUInt128(const CUInt128 &uValue) : m_uData0(uValue.m_uData0), m_uData1(uValue.m_uData1), m_uData2(uValue.m_uData2), m_uData3(uValue.m_uData3) {}

			const byte* GetData() const;
			byte* GetDataPtr() const;
			/** Bit at level 0 being most significant. */
			uint_ptr GetBitNumber(uint_ptr uBit) const;
			int CompareTo(const CUInt128 &uOther) const;
			int CompareTo(ULONG uValue) const;
			void ToHexString(CString *pstr) const;
			CString ToHexString() const;
			void ToBinaryString(CString *pstr, bool bTrim = false) const;
			void ToByteArray(byte *pby) const;
			ULONG Get32BitChunk(sint_ptr iVal) const;
			void SetValue(const CUInt128 &uValue);
			void SetValue(ULONG uValue);
			void SetValueBE(const byte *pbyValueBE);
			void SetValueRandom();
			//void SetValueGUID();
			//void SetBitNumber(sint_ptr uBit, sint_ptr uValue);
			void ShiftLeft(sint_ptr uBits);
			void Add(const CUInt128 &uValue);
			//void Add(ULONG uValue);
			//void Div(ULONG uValue);
			//void Subtract(const CUInt128 &uValue);
			//void Subtract(ULONG uValue);
			void Inc();
			CUInt128& Xor(const CUInt128 &uValue);
			//CUInt128& XorBE(const byte *pbyValueBE);
			//void operator+  (const CUInt128 &uValue);
			//void operator-  (const CUInt128 &uValue);
			void operator=  (const CUInt128 &uValue);
			bool operator<  (const CUInt128 &uValue) const;
			bool operator>  (const CUInt128 &uValue) const;
			bool operator<= (const CUInt128 &uValue) const;
			bool operator>= (const CUInt128 &uValue) const;
			bool operator== (const CUInt128 &uValue) const;
			bool operator!= (const CUInt128 &uValue) const;
			//void operator+  (ULONG uValue);
			//void operator-  (ULONG uValue);
			void operator=  (ULONG uValue);
			bool operator<  (ULONG uValue) const;
			bool operator>  (ULONG uValue) const;
			bool operator<= (ULONG uValue) const;
			bool operator>= (ULONG uValue) const;
			bool operator== (ULONG uValue) const;
			bool operator!= (ULONG uValue) const;
		private:
			// netfinity: Constructors can't initialize arrays inline
			union
			{
				ULONG		m_uData[4];
				struct
				{
					ULONG	m_uData0;
					ULONG	m_uData1;
					ULONG	m_uData2;
					ULONG	m_uData3;
				};
			};
	};

	struct uint128_unordered{
		size_t operator()(const CUInt128&u) const{
			size_t* pval = (size_t*)u.GetData();
#ifdef _WIN64
			return _rotl_ptr(*pval, 11)+*(pval+1);
#else
			size_t hash = 0;
			for(uint_ptr i = 0; i < 4; ++i)
				hash = _rotl_ptr(hash, 11) + *pval++;
			return hash;
#endif
		}
		bool operator()(const CUInt128&u1, const CUInt128&u2) const{
			return u1 == u2;
		}
	};


	// netfinity: This code is better used inline, for speed

	inline
	void CUInt128::SetValue(const CUInt128 &uValue)
	{
#ifdef _WIN64
		((uint64*)m_uData)[0] = ((uint64*)uValue.m_uData)[0];
		((uint64*)m_uData)[1] = ((uint64*)uValue.m_uData)[1];
#else
		m_uData[0] = uValue.m_uData[0];
		m_uData[1] = uValue.m_uData[1];
		m_uData[2] = uValue.m_uData[2];
		m_uData[3] = uValue.m_uData[3];
#endif
	}

	inline
	void CUInt128::SetValue(ULONG uValue)
	{
		m_uData[0] = 0;
		m_uData[1] = 0;
		m_uData[2] = 0;
		m_uData[3] = uValue;
	}

	inline
	CUInt128& CUInt128::Xor(const CUInt128 &uValue)
	{
#ifdef _WIN64
		((uint64*)m_uData)[0] ^= ((uint64*)uValue.m_uData)[0];
		((uint64*)m_uData)[1] ^= ((uint64*)uValue.m_uData)[1];
#else
		m_uData[0] ^= uValue.m_uData[0];
		m_uData[1] ^= uValue.m_uData[1];
		m_uData[2] ^= uValue.m_uData[2];
		m_uData[3] ^= uValue.m_uData[3];
#endif
		return *this;
	}

	inline
	void CUInt128::operator=  (const CUInt128 &uValue)
	{
		SetValue(uValue);
	}

	inline
	bool CUInt128::operator<  (const CUInt128 &uValue) const
	{
		if (m_uData[0] == uValue.m_uData[0])
			if (m_uData[1] == uValue.m_uData[1])
				if (m_uData[2] == uValue.m_uData[2])
					return (m_uData[3] < uValue.m_uData[3]);
				else
					return (m_uData[2] < uValue.m_uData[2]);
			else
				return (m_uData[1] < uValue.m_uData[1]);
		else
			return (m_uData[0] < uValue.m_uData[0]);
	}

	inline
	bool CUInt128::operator>  (const CUInt128 &uValue) const
	{
		if (m_uData[0] == uValue.m_uData[0])
			if (m_uData[1] == uValue.m_uData[1])
				if (m_uData[2] == uValue.m_uData[2])
					return (m_uData[3] > uValue.m_uData[3]);
				else
					return (m_uData[2] > uValue.m_uData[2]);
			else
				return (m_uData[1] > uValue.m_uData[1]);
		else
			return (m_uData[0] > uValue.m_uData[0]);
	}

	inline
	bool CUInt128::operator<= (const CUInt128 &uValue) const
	{
		if (m_uData[0] == uValue.m_uData[0])
			if (m_uData[1] == uValue.m_uData[1])
				if (m_uData[2] == uValue.m_uData[2])
					return (m_uData[3] <= uValue.m_uData[3]);
				else
					return (m_uData[2] < uValue.m_uData[2]);
			else
				return (m_uData[1] < uValue.m_uData[1]);
		else
			return (m_uData[0] < uValue.m_uData[0]);
	}

	inline
	bool CUInt128::operator>= (const CUInt128 &uValue) const
	{
		if (m_uData[0] == uValue.m_uData[0])
			if (m_uData[1] == uValue.m_uData[1])
				if (m_uData[2] == uValue.m_uData[2])
					return (m_uData[3] >= uValue.m_uData[3]);
				else
					return (m_uData[2] > uValue.m_uData[2]);
			else
				return (m_uData[1] > uValue.m_uData[1]);
		else
			return (m_uData[0] > uValue.m_uData[0]);
	}

	inline
	bool CUInt128::operator== (const CUInt128 &uValue) const
	{
#ifdef _WIN64
		return (((uint64*)m_uData)[0] == ((uint64*)uValue.m_uData)[0] && ((uint64*)m_uData)[1] == ((uint64*)uValue.m_uData)[1]);
#else
		return (m_uData[0] == uValue.m_uData[0] && m_uData[1] == uValue.m_uData[1] && m_uData[2] == uValue.m_uData[2] && m_uData[3] == uValue.m_uData[3]);
#endif
	}

	inline
	bool CUInt128::operator!= (const CUInt128 &uValue) const
	{
#ifdef _WIN64
		return (((uint64*)m_uData)[0] != ((uint64*)uValue.m_uData)[0] || ((uint64*)m_uData)[1] != ((uint64*)uValue.m_uData)[1]);
#else
		return (m_uData[0] != uValue.m_uData[0] || m_uData[1] != uValue.m_uData[1] || m_uData[2] != uValue.m_uData[2] || m_uData[3] != uValue.m_uData[3]);
#endif
	}
}
