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

#include "opcodes.h"
#include "otherstructs.h"
#include "StringConversion.h"

class Packet
{
public:
			Packet(byte byteProtocol);
			Packet(PacketHeader_Struct *pHeaderStruct); // only used for receiving packets
			Packet(CMemFile *pDataFile, byte byteProtocol = OP_EDONKEYPROT);
			Packet(char *pPacketPart, uint32 nSize, bool bLast, byte byteFilePriority, bool bFromPF);
			Packet(EnumOpcodes eOpcode, uint32 iSize, byte byteProtocol = OP_EDONKEYPROT);
			Packet(EnumOpcodes eOpcode, uint32 iSize, byte byteProtocol, byte byteFilePriority, bool bFromPF);
			~Packet();
	char	*GetHeader();
	char	*GetUDPHeader();
	char	*GetPacket();
	char	*DetachPacket();
	uint32	GetRealPacketSize()	{return m_dwSize + sizeof(PacketHeader_Struct);}
	bool	IsSplit()			{return m_bSplit;}
	bool	IsLastSplit()		{return m_bLastSplit;}
	bool	IsFromPF()			{return m_bFromPF;}
	byte	GetFilePriority()	{return m_byteFilePriority;}
	void	PackPacket();
	bool	UnpackPacket(uint32 dwMaxUnpackedSize = 0);

	char		*m_pcBuffer;
	uint32		m_dwSize;
	EnumOpcodes	m_eOpcode;
	byte		m_byteProtocol;

private:
	byte		m_byteFilePriority;
	bool		m_bSplit;
	bool		m_bLastSplit;
	bool		m_bFromPF;
	char		m_arrcHead[6];
	char		*m_pcCompleteBuffer;
	char		*m_pcTempBuffer;
};

//	Tag Types
#define TAGTYPE_NONE		0
#define TAGTYPE_HASH		0x01
#define TAGTYPE_STRING		0x02
#define TAGTYPE_UINT32		0x03
#define TAGTYPE_FLOAT32		0x04
#define TAGTYPE_BOOL		0x05
#define TAGTYPE_BOOLARRAY	0x06
#define TAGTYPE_BLOB		0x07
#define TAGTYPE_UINT16		0x08
#define TAGTYPE_UINT8		0x09
#define TAGTYPE_BSOB		0x0A
#define TAGTYPE_UINT64		0x0B
#define TAGTYPE_STR1		0x11
#define TAGTYPE_STR2		0x12
#define TAGTYPE_STR3		0x13
#define TAGTYPE_STR4		0x14
#define TAGTYPE_STR5		0x15
#define TAGTYPE_STR6		0x16
#define TAGTYPE_STR7		0x17
#define TAGTYPE_STR8		0x18
#define TAGTYPE_STR9		0x19
#define TAGTYPE_STR10		0x1A
#define TAGTYPE_STR11		0x1B
#define TAGTYPE_STR12		0x1C
#define TAGTYPE_STR13		0x1D
#define TAGTYPE_STR14		0x1E
#define TAGTYPE_STR15		0x1F
#define TAGTYPE_STR16		0x20

class CTag
{
public:
				CTag();
				CTag(byte byteTagID, uint32 dwValue);
				CTag(byte byteTagID, const TCHAR *pcValue);
				CTag(const char *pcName, uint32 dwValue);
				CTag(const char *pcName, const TCHAR *pcValue);
				CTag(byte byteTagID, const byte *pbyteHash);
				CTag(byte byteTagID, uint32 dwSize, const byte *pbyteData);
				CTag(const CTag &Tag);
				~CTag();

	void		FillFromStream(CFile &file, ECodingFormat eCF = cfLocalCodePage);
	bool		WriteToFile(CFile &file, ECodingFormat eCF = cfLocalCodePage) const;	// old eD2K tags
	bool		WriteNewEd2kTag(CFile &file, ECodingFormat eCF = cfLocalCodePage) const;	// new eD2K tags

	byte		GetTagID() const			{ return m_byteTagID; }
	const CString&	GetStringValue() const	{ return *m_pstrValue; }
	void		GetStringValue(CString *pstrOut) const	{ *pstrOut = *m_pstrValue; }
	bool		IsStringValueEmpty() const	{ return m_pstrValue->IsEmpty(); }
	bool		IsStringValueEqual(const CString &strCmp) const	{ return (*m_pstrValue == strCmp); }
	const char*	GetTagName() const			{ return m_pcName; }
	uint32		GetIntValue() const			{ return static_cast<uint32>(m_qwValue); }
	void		SetIntValue(uint32 dwValue)	{ m_qwValue = static_cast<uint64>(dwValue); }
	void		GetInt64Value(uint64 *pqwVal) const				{ *pqwVal = m_qwValue; }
	uint64		GetInt64Value() const		{ return m_qwValue; }
	float		GetFloatValue() const		{ return m_fValue; }
	const byte*	GetHashValue() const		{ return reinterpret_cast<const byte*>(m_pData); }
	uint32		GetBlobSize() const			{ return m_dwBlobSize; }
	const byte*	GetBlobPtr() const			{ return reinterpret_cast<const byte*>(m_pData); }
	byte		GetType() const				{ return m_byteTagType; }
	void		ChangeTagID(byte byteNewTagID);

	bool		IsStr() const				{ return m_byteTagType == TAGTYPE_STRING; }
	bool		IsInt() const				{ return m_byteTagType == TAGTYPE_UINT32; }
	bool		IsFloat() const				{ return m_byteTagType == TAGTYPE_FLOAT32; }
	bool		IsHash() const				{ return m_byteTagType == TAGTYPE_HASH; }
	bool		IsBlob() const				{ return m_byteTagType == TAGTYPE_BLOB; }
	bool		IsInt64() const				{ return (m_byteTagType == TAGTYPE_UINT64); }
	bool		IsAnyInt() const			{ return ((m_byteTagType == TAGTYPE_UINT32) || (m_byteTagType == TAGTYPE_UINT64)); }

protected:
	union {
		CString			*m_pstrValue;
		uint64			m_qwValue;
		float			m_fValue;
		void			*m_pData;
	};
	char			*m_pcName;
	uint32			m_dwBlobSize;
	byte			m_byteTagType;
	byte			m_byteTagID;
};

class CWrTag
{
public:
			CWrTag();
//	Old eD2K tags
	bool	WriteToFile(byte byteTagID, uint32 dwValue, CFile &file) const;
	bool	WriteToFile(byte byteTagID, uint64 qwValue, CFile &file, bool bInt64) const;
	bool	WriteToFile(byte byteTagID, const CString &strValue, CFile &file, ECodingFormat eCF = cfLocalCodePage) const;
	bool	WriteToFile(LPCSTR strName, uint64 qwValue, CFile &file, bool bInt64) const;

//	New eD2K tags
	bool	WriteNewEd2kTag(byte byteTagID, uint32 dwValue, CFile &file) const;
	bool	WriteNewEd2kTag(byte byteTagID, uint64 qwValue, CFile &file) const;
	bool	WriteNewEd2kTag(byte byteTagID, const CString &strValue, CFile &file, ECodingFormat eCF = cfLocalCodePage) const;
	bool	WriteNewEd2kTag(LPCSTR strName, uint64 qwValue, CFile &file) const;
};


//	CTag and tag string helpers

__inline int CmpED2KTagName(LPCSTR pszTagName1, LPCSTR pszTagName2)
{
//	String compare is independant from any codepage and/or LC_CTYPE setting
	return __ascii_stricmp(pszTagName1, pszTagName2);
}
