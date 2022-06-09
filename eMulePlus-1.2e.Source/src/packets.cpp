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

#include "stdafx.h"
#include "packets.h"
#include "otherfunctions.h"
#include "zlib/zlib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::Packet(byte byteProtocol)
{
	m_bSplit = false;
	m_bLastSplit = false;
	m_dwSize = 0;
	m_pcBuffer = NULL;
	m_pcCompleteBuffer = NULL;
	m_pcTempBuffer = NULL;
	m_byteProtocol = byteProtocol;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::Packet(PacketHeader_Struct *pHeaderStruct)
{
	m_bSplit = false;
	m_bLastSplit = false;
	m_pcTempBuffer = NULL;
	m_pcBuffer = NULL;
	m_pcCompleteBuffer = NULL;
	m_dwSize = pHeaderStruct->dwPacketLength - 1;
	m_eOpcode = pHeaderStruct->eCommand;
	m_byteProtocol = pHeaderStruct->byteEDonkeyProtocol;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::Packet(char *pcPacketPart, uint32 dwSize, bool bLast, byte byteFilePriority, bool bFromPF)
{
	m_byteFilePriority = byteFilePriority;
	m_bFromPF = bFromPF;
	m_bSplit = true;
	m_bLastSplit = bLast;
	m_pcTempBuffer = NULL;
	m_pcBuffer = NULL;
	m_pcCompleteBuffer = pcPacketPart;
	m_dwSize = dwSize - sizeof(PacketHeader_Struct);
	m_byteProtocol = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Packet(EnumOpCodes,uint32,eProtocol) constructs a Packet with the spec'd opcode, data size, and protocol.
Packet::Packet(EnumOpcodes eOpcode, uint32 dwSize/*=0*/, byte byteProtocol/*=OP_EDONKEYPROT*/)
{
	m_bSplit = false;
	m_bLastSplit = false;
	m_pcTempBuffer = NULL;
	if (dwSize)
	{
		m_pcCompleteBuffer = new char[dwSize + sizeof(PacketHeader_Struct)];
		m_pcBuffer = m_pcCompleteBuffer + sizeof(PacketHeader_Struct);
		memzero(m_pcCompleteBuffer, dwSize + sizeof(PacketHeader_Struct));
	}
	else
	{
		m_pcBuffer = NULL;
		m_pcCompleteBuffer = NULL;
	}
	m_eOpcode = eOpcode;
	m_dwSize = dwSize;
	m_byteProtocol = byteProtocol;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::Packet(EnumOpcodes eOpcode, uint32 dwSize, byte byteProtocol, byte byteFilePriority, bool bFromPF)
{
	m_byteFilePriority = byteFilePriority;
//	'bFromPF' is only used to set the "from PartFile" state of the Packet. If 'm_bFromPF' = false then this
//	packet was formed from a complete shared file rather than a part file.
//	It is used only in the UploadClient to track transferred data stats.
	m_bFromPF = bFromPF;
	m_bSplit = false;
	m_bLastSplit = false;
	m_pcTempBuffer = NULL;
	if (dwSize)
	{
		m_pcCompleteBuffer = new char[dwSize + sizeof(PacketHeader_Struct)];
		m_pcBuffer = m_pcCompleteBuffer + sizeof(PacketHeader_Struct);
		memzero(m_pcCompleteBuffer, dwSize + sizeof(PacketHeader_Struct));
	}
	else
	{
		m_pcBuffer = NULL;
		m_pcCompleteBuffer = NULL;
	}
	m_eOpcode = eOpcode;
	m_dwSize = dwSize;
	m_byteProtocol = byteProtocol;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::Packet(CMemFile *pDataFile, byte byteProtocol)
{
	m_bSplit = false;
	m_bLastSplit = false;
	m_dwSize = static_cast<uint32>(pDataFile->GetLength());
	m_pcCompleteBuffer = new char[m_dwSize + sizeof(PacketHeader_Struct)];
	m_pcBuffer = m_pcCompleteBuffer + sizeof(PacketHeader_Struct);

	BYTE	*pbyteBuffer = pDataFile->Detach();

	memcpy2(m_pcBuffer, pbyteBuffer, m_dwSize);
	free(pbyteBuffer);
	m_pcTempBuffer = NULL;
	m_byteProtocol = byteProtocol;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet::~Packet()
{
	if (m_pcCompleteBuffer != NULL)
		delete[] m_pcCompleteBuffer;
	else if (m_pcBuffer != NULL)
		delete[] m_pcBuffer;
	delete[] m_pcTempBuffer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *Packet::GetPacket()
{
	if (m_pcCompleteBuffer != NULL)
	{
		if (!m_bSplit)
			memcpy(m_pcCompleteBuffer, GetHeader(), sizeof(PacketHeader_Struct));

		return m_pcCompleteBuffer;
	}
	else
	{
		delete[] m_pcTempBuffer;
		m_pcTempBuffer = NULL;

		m_pcTempBuffer = new char[m_dwSize + sizeof(PacketHeader_Struct)];
		memcpy(m_pcTempBuffer, GetHeader(), sizeof(PacketHeader_Struct));
		memcpy2(m_pcTempBuffer + sizeof(PacketHeader_Struct), m_pcBuffer, m_dwSize);

		return m_pcTempBuffer;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *Packet::DetachPacket()
{
	if (m_pcCompleteBuffer != NULL)
	{
		if (!m_bSplit)
			memcpy(m_pcCompleteBuffer, GetHeader(), sizeof(PacketHeader_Struct));

		char	*pcResult = m_pcCompleteBuffer;

		m_pcCompleteBuffer = NULL;
		m_pcBuffer = NULL;

		return pcResult;
	}
	else
	{
		delete[] m_pcTempBuffer;
		m_pcTempBuffer = NULL;

		m_pcTempBuffer = new char[m_dwSize + sizeof(PacketHeader_Struct)];
		memcpy(m_pcTempBuffer, GetHeader(), sizeof(PacketHeader_Struct));
		memcpy2(m_pcTempBuffer + sizeof(PacketHeader_Struct), m_pcBuffer, m_dwSize);

		char	*pcResult = m_pcTempBuffer;

		m_pcTempBuffer = NULL;
		return pcResult;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *Packet::GetHeader()
{
	ASSERT(!m_bSplit);

	PacketHeader_Struct		*pHeaderStruct = reinterpret_cast<PacketHeader_Struct*>(m_arrcHead);

	pHeaderStruct->eCommand = m_eOpcode;
	pHeaderStruct->byteEDonkeyProtocol = m_byteProtocol;
	pHeaderStruct->dwPacketLength = m_dwSize + 1;
	return m_arrcHead;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char *Packet::GetUDPHeader()
{
	ASSERT(!m_bSplit);

	UDP_Header_Struct	*pUDPHeader = reinterpret_cast<UDP_Header_Struct*>(m_arrcHead);

	pUDPHeader->eCommand = m_eOpcode;
	pUDPHeader->byteEDonkeyProtocol = m_byteProtocol;

	return m_arrcHead;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Packet::PackPacket()
{
	ASSERT(!m_bSplit);

	uLongf	dwNewSize = m_dwSize + 300;
	BYTE	*pbyteOutput = new BYTE[dwNewSize];
//	Zip 'm_pcBuffer' to 'pbyteOutput'.
	int		iResult = compress2(pbyteOutput,&dwNewSize,reinterpret_cast<BYTE*>(m_pcBuffer),m_dwSize,9);

//	If the compressed buffer is actually larger than the original...
	if ((iResult != Z_OK) || (m_dwSize <= dwNewSize))
	{
		delete[] pbyteOutput;
		return;
	}
	m_byteProtocol = OP_PACKEDPROT;
	memcpy2(m_pcBuffer, pbyteOutput, m_dwSize = dwNewSize);
	delete[] pbyteOutput;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Packet::UnpackPacket(uint32 dwMaxUnpackedSize /*= 0*/)
{
	ASSERT(m_byteProtocol == OP_PACKEDPROT);

	if (dwMaxUnpackedSize == 0)
		dwMaxUnpackedSize = m_dwSize * 10 + 300;
	
	uLongf	dwUnpackedSize = dwMaxUnpackedSize;
	BYTE	*pbyteUnpackBuffer = new BYTE[dwUnpackedSize];
	int		iResult = uncompress(pbyteUnpackBuffer, &dwUnpackedSize, reinterpret_cast<BYTE*>(m_pcBuffer), m_dwSize);

	if (iResult == Z_OK)
	{
		ASSERT(m_pcCompleteBuffer == NULL);
		ASSERT(m_pcBuffer != NULL);
		m_dwSize = dwUnpackedSize;
		delete[] m_pcBuffer;
		m_pcBuffer = reinterpret_cast<char*>(pbyteUnpackBuffer);
		m_byteProtocol = OP_EMULEPROT;
		return true;
	}
	delete[] pbyteUnpackBuffer;

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CTag
CTag::CTag(const char *pcName, uint32 dwValue)
{
	m_qwValue = static_cast<uint64>(dwValue);
	m_pcName = _strdup(pcName);
	m_byteTagType = TAGTYPE_UINT32;
	m_byteTagID = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTag::CTag(byte byteTagID, uint32 dwValue)
{
	m_qwValue = static_cast<uint64>(dwValue);
	m_pcName = NULL;
	m_byteTagType = TAGTYPE_UINT32;
	m_byteTagID = byteTagID;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTag::CTag(const char *pcName, const TCHAR *pcValue)
{
	m_pstrValue = new CString(pcValue);
	m_pcName = _strdup(pcName);
	m_byteTagType = TAGTYPE_STRING;
	m_byteTagID = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTag::CTag(byte byteTagID, const TCHAR *pcValue)
{
	m_pstrValue = new CString(pcValue);
	m_pcName = NULL;
	m_byteTagType = TAGTYPE_STRING;
	m_byteTagID = byteTagID;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTag::CTag(byte byteTagID, const byte *pbyteHash)
{
	m_pData = malloc(16);
	md4cpy(m_pData, pbyteHash);
	m_pcName = NULL;
	m_byteTagType = TAGTYPE_HASH;
	m_byteTagID = byteTagID;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTag::CTag(byte byteTagID, uint32 dwSize, const byte *pbyteData)
{
	m_pData = malloc(dwSize);
	memcpy2(m_pData, pbyteData, m_dwBlobSize = dwSize);
	m_pcName = NULL;
	m_byteTagType = TAGTYPE_BLOB;
	m_byteTagID = byteTagID;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTag::CTag()
{
	m_qwValue = 0;
	m_pcName = NULL;
	m_byteTagType = TAGTYPE_NONE;
	m_byteTagID = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTag::CTag(const CTag &Tag)
{
	m_byteTagType = Tag.m_byteTagType;
	m_byteTagID = Tag.m_byteTagID;
	m_pcName = (Tag.m_pcName != NULL) ? _strdup(Tag.m_pcName) : NULL;
	m_dwBlobSize = 0;
	if (Tag.IsStr())
		m_pstrValue = new CString(Tag.GetStringValue());
	else if (Tag.IsAnyInt())
		Tag.GetInt64Value(&m_qwValue);
	else if (Tag.IsFloat())
		m_fValue = Tag.GetFloatValue();
	else if (Tag.IsHash())
	{
		m_pData = malloc(16);
		md4cpy(m_pData, Tag.GetHashValue());
	}
	else if (Tag.IsBlob())
	{
		m_pData = malloc(m_dwBlobSize = Tag.GetBlobSize());
		memcpy(m_pData, Tag.GetBlobPtr(), m_dwBlobSize);
	}
	else
	{
		ASSERT(0);
		m_qwValue = 0;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Setup a tag from a CFile stream
//	(it shouldn't be a constructor to avoid memory leaks of dynamic members)
void CTag::FillFromStream(CFile &file, ECodingFormat eCF/*=cfLocalCodePage*/)
{
	uint32	dwLength;
	uint16	uTmp;
	byte	byteType, byteOctet;

	file.Read(&byteType, 1);
	if (byteType & 0x80)
	{
		byteType &= 0x7F;
		file.Read(&m_byteTagID, 1);
	}
	else
	{
		file.Read(&uTmp, 2);
	//	Read the tag identifier; Either a special tag code or a tag name
		if ((dwLength = uTmp) == 1)
			file.Read(&m_byteTagID, 1);
		else
		{
			m_pcName = reinterpret_cast<char*>(malloc(dwLength + 1));
			file.Read(m_pcName, dwLength);
			m_pcName[dwLength] = '\0';
		}
	}

	m_dwBlobSize = 0;

//	NOTE: It's very important that we read the *entire* packet data, even if we do not use each tag
	if (byteType == TAGTYPE_STRING)
	{
		file.Read(&uTmp, 2);
		dwLength = uTmp;
		m_pstrValue = new CString();
		m_byteTagType = TAGTYPE_STRING;	//	set type after allocation, but before reading to free memory on exception
		ReadMB2Str(eCF, m_pstrValue, file, dwLength);
	}
	else if (byteType == TAGTYPE_UINT32)
	{
		m_byteTagType = TAGTYPE_UINT32;
		file.Read(&dwLength, 4);
		m_qwValue = static_cast<uint64>(dwLength);
	}
	else if (byteType == TAGTYPE_UINT16)
	{
		m_byteTagType = TAGTYPE_UINT32;
		file.Read(&uTmp, 2);
		m_qwValue = static_cast<uint64>(uTmp);
	}
	else if (byteType == TAGTYPE_UINT8)
	{
		m_byteTagType = TAGTYPE_UINT32;
		file.Read(&byteOctet, 1);
		m_qwValue = static_cast<uint64>(byteOctet);
	}
	else if (byteType == TAGTYPE_UINT64)
	{
		m_byteTagType = TAGTYPE_UINT64;
		file.Read(&m_qwValue, 8);
	}
	else if ((byteType >= TAGTYPE_STR1) && (byteType <= TAGTYPE_STR16))
	{
		dwLength = byteType - TAGTYPE_STR1 + 1;
		m_pstrValue = new CString();
		m_byteTagType = TAGTYPE_STRING;	//	set type after allocation, but before reading to free memory on exception
		ReadMB2Str(eCF, m_pstrValue, file, dwLength);
	}
	else if (byteType == TAGTYPE_FLOAT32) // (used by Hybrid 0.48)
	{
		m_byteTagType = TAGTYPE_FLOAT32;
		file.Read(&m_fValue, 4);
	}
	else if (byteType == TAGTYPE_HASH)
	{
		m_pData = malloc(16);
		m_byteTagType = TAGTYPE_HASH;	//	set type after allocation, but before reading to free memory on exception
		file.Read(m_pData, 16);
	}
	else if (byteType == TAGTYPE_BOOL)
	{
		m_byteTagType = TAGTYPE_BOOL;
		file.Seek(1, CFile::current);
	}
	else if (byteType == TAGTYPE_BOOLARRAY)
	{
		m_byteTagType = TAGTYPE_BOOLARRAY;
		file.Read(&uTmp, 2);
		file.Seek((uTmp / 8) + 1, CFile::current);
	}
	else if (byteType == TAGTYPE_BLOB)
	{
		file.Read(&dwLength, 4);
		if (dwLength <= (file.GetLength() - file.GetPosition()))
		{
			m_pData = malloc(dwLength);
			m_byteTagType = TAGTYPE_BLOB;	//	set type after allocation, but before reading to free memory on exception
			file.Read(m_pData, m_dwBlobSize = dwLength);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTag::~CTag()
{
	if (m_pcName != NULL)
		free(m_pcName);
	if (IsStr())
		delete m_pstrValue;
	else if ((IsHash() || IsBlob()) && (m_pData != NULL))
		free(m_pData);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTag::WriteNewEd2kTag(CFile &file, ECodingFormat eCF/*=cfLocalCodePage*/) const
{
	byte		byteType = m_byteTagType;
	uint32		dwVal, dwStrValLen = 0;
	CStringA	strValA;

	if (byteType == TAGTYPE_UINT64)
	{
		if (m_qwValue <= 0xFFFFFFFFui64)
			byteType = TAGTYPE_UINT32;
	}
	if (byteType == TAGTYPE_UINT32)
	{
		dwVal = static_cast<uint32>(m_qwValue);
		if (dwVal <= 0xFF)
			byteType = TAGTYPE_UINT8;
		else if (dwVal <= 0xFFFF)
			byteType = TAGTYPE_UINT16;
	}
	else if (byteType == TAGTYPE_STRING)
	{
		dwStrValLen = Str2MB(eCF, &strValA, *m_pstrValue);
		if ((dwStrValLen >= 1) && (dwStrValLen <= 16))
			byteType = static_cast<byte>(TAGTYPE_STR1 + dwStrValLen - 1);
	}

//	Write tag type/name
	if (m_pcName != NULL)
	{
		uint32	dwTagNameLen = strlen(m_pcName);

		file.Write(&byteType, 1);
		file.Write(&dwTagNameLen, 2);
		file.Write(m_pcName, dwTagNameLen);
	}
	else
	{
		byte	byteTmp = byteType | 0x80;

		file.Write(&byteTmp, 1);
		file.Write(&m_byteTagID, 1);
	}

//	Write tag data
	if (byteType == TAGTYPE_STRING)
	{
		file.Write(&dwStrValLen, 2);
		file.Write(strValA.GetString(), dwStrValLen);
	}
	else if ((byteType >= TAGTYPE_STR1) && (byteType <= TAGTYPE_STR16))
		file.Write(strValA.GetString(), dwStrValLen);
	else if (byteType == TAGTYPE_UINT32)
		file.Write(&m_qwValue, 4);
	else if (byteType == TAGTYPE_UINT16)
		file.Write(&m_qwValue, 2);
	else if (byteType == TAGTYPE_UINT8)
		file.Write(&m_qwValue, 1);
	else if (byteType == TAGTYPE_UINT64)
		file.Write(&m_qwValue, 8);
	else if (byteType == TAGTYPE_FLOAT32)
		file.Write(&m_fValue, 4);
	else if (byteType == TAGTYPE_HASH)
		file.Write(m_pData, 16);
	else if (byteType == TAGTYPE_BLOB)
	{
		file.Write(&m_dwBlobSize, 4);
		file.Write(m_pData, m_dwBlobSize);
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTag::WriteToFile(CFile &file, ECodingFormat eCF/*=cfLocalCodePage*/) const
{
	uint16	uLength;

	file.Write(&m_byteTagType, 1);

//	Write the tag identifier; Either a special tag code or the tag name
	if (m_pcName != NULL)
	{
		uLength = static_cast<uint16>(strlen(m_pcName));
		file.Write(&uLength, 2);
		file.Write(m_pcName, uLength);
	}
	else
	{
		uLength = 1;
		file.Write(&uLength, 2);
		file.Write(&m_byteTagID, uLength);
	}

//	Write the tag data value
	if (IsStr())
	{
		CStringA	strValA;

		uLength = static_cast<uint16>(Str2MB(eCF, &strValA, *m_pstrValue));
		file.Write(&uLength, 2);
		file.Write(strValA.GetString(), strValA.GetLength());
	}
	else if (IsInt())
		file.Write(&m_qwValue, 4);
	else if (IsInt64())
		file.Write(&m_qwValue, 8);
	else if (IsFloat())
		file.Write(&m_fValue, 4);
	else if (IsBlob())
	{
		file.Write(&m_dwBlobSize, 4);
		file.Write(m_pData, m_dwBlobSize);
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTag::ChangeTagID(byte byteNewTagID)
{
	if (m_pcName != NULL)
	{
		free(m_pcName);
		m_pcName = NULL;
	}
	m_byteTagID = byteNewTagID;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CWrNumTag
CWrTag::CWrTag()
{}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWrTag::WriteNewEd2kTag(byte byteTagID, uint32 dwValue, CFile &file) const
{
	uint32	dwSz;
	byte	abyteBuf[6];

	if (dwValue <= 0xFF)
	{
		abyteBuf[0] = TAGTYPE_UINT8 | 0x80;
		dwSz = 3;
	}
	else if (dwValue <= 0xFFFF)
	{
		abyteBuf[0] = TAGTYPE_UINT16 | 0x80;
		dwSz = 4;
	}
	else
	{
		abyteBuf[0] = TAGTYPE_UINT32 | 0x80;
		dwSz = 6;
	}
	abyteBuf[1] = byteTagID;
	POKE_DWORD(&abyteBuf[2], dwValue);

	file.Write(abyteBuf, dwSz);
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWrTag::WriteNewEd2kTag(byte byteTagID, uint64 qwValue, CFile &file) const
{
	if (qwValue <= 0xFFFFFFFFui64)
		return WriteNewEd2kTag(byteTagID, static_cast<uint32>(qwValue), file);

	byte	abyteBuf[10];

	abyteBuf[0] = TAGTYPE_UINT64 | 0x80;
	abyteBuf[1] = byteTagID;
	POKE_QWORD(&abyteBuf[2], qwValue);

	file.Write(abyteBuf, sizeof(abyteBuf));
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWrTag::WriteToFile(byte byteTagID, uint32 dwValue, CFile &file) const
{
	byte	abyteBuf[8];

	abyteBuf[0] = TAGTYPE_UINT32;
	POKE_WORD(&abyteBuf[1], 1);
	abyteBuf[3] = byteTagID;
	POKE_DWORD(&abyteBuf[4], dwValue);

	file.Write(abyteBuf, sizeof(abyteBuf));
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWrTag::WriteToFile(byte byteTagID, uint64 qwValue, CFile &file, bool bInt64) const
{
	if (!bInt64)
		return WriteToFile(byteTagID, static_cast<uint32>(qwValue), file);

	byte	abyteBuf[12];

	abyteBuf[0] = TAGTYPE_UINT64;
	POKE_WORD(&abyteBuf[1], 1);
	abyteBuf[3] = byteTagID;
	POKE_QWORD(&abyteBuf[4], qwValue);

	file.Write(abyteBuf, sizeof(abyteBuf));
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWrTag::WriteNewEd2kTag(LPCSTR strName, uint64 qwValue, CFile &file) const
{
	uint32	dwVal, dwSz, dwLen = strlen(strName);
	byte	abyteBuf[3];

	if (qwValue <= 0xFFFFFFFFui64)
	{
		dwVal = static_cast<uint32>(qwValue);
		if (dwVal <= 0xFF)
		{
			abyteBuf[0] = TAGTYPE_UINT8;
			dwSz = 1;
		}
		else if (dwVal <= 0xFFFF)
		{
			abyteBuf[0] = TAGTYPE_UINT16;
			dwSz = 2;
		}
		else
		{
			abyteBuf[0] = TAGTYPE_UINT32;
			dwSz = 4;
		}
	}
	else
	{
		abyteBuf[0] = TAGTYPE_UINT64;
		dwSz = 8;
	}
	POKE_WORD(&abyteBuf[1], static_cast<uint16>(dwLen));

	file.Write(abyteBuf, sizeof(abyteBuf));
	file.Write(strName, dwLen);
	file.Write(&qwValue, dwSz);
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWrTag::WriteToFile(LPCSTR strName, uint64 qwValue, CFile &file, bool bInt64) const
{
	uint32	dwSz, dwLen = strlen(strName);
	byte	abyteBuf[3];

	if (bInt64)
	{
		abyteBuf[0] = TAGTYPE_UINT64;
		dwSz = 8;
	}
	else
	{
		abyteBuf[0] = TAGTYPE_UINT32;
		dwSz = 4;
	}
	POKE_WORD(&abyteBuf[1], static_cast<uint16>(dwLen));

	file.Write(abyteBuf, sizeof(abyteBuf));
	file.Write(strName, dwLen);
	file.Write(&qwValue, dwSz);
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWrTag::WriteNewEd2kTag(byte byteTagID, const CString &strValue, CFile &file, ECodingFormat eCF/*=cfLocalCodePage*/) const
{
	byte	byteType, abyteBuf[4];
	uint32	dwSz, dwStrLen;
	CStringA	strMultiByteBuf;

	Str2MB(eCF, &strMultiByteBuf, strValue);
	dwStrLen = strMultiByteBuf.GetLength();

	byteType = static_cast<byte>(((dwStrLen >= 1) && (dwStrLen <= 16)) ? (TAGTYPE_STR1 + dwStrLen - 1) : TAGTYPE_STRING);

	abyteBuf[0] = byteType | 0x80;
	abyteBuf[1] = byteTagID;

	dwSz = 2;
	if (byteType == TAGTYPE_STRING)
	{
		dwSz = 4;
		POKE_WORD(&abyteBuf[2], static_cast<uint16>(dwStrLen));
	}
	file.Write(abyteBuf, dwSz);
	file.Write(strMultiByteBuf.GetString(), dwStrLen);

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CWrTag::WriteToFile(byte byteTagID, const CString &strValue, CFile &file, ECodingFormat eCF/*=cfLocalCodePage*/) const
{
	byte		abyteBuf[6];
	CStringA	strMultiByteBuf;

	Str2MB(eCF, &strMultiByteBuf, strValue);

	abyteBuf[0] = TAGTYPE_STRING;
	POKE_WORD(&abyteBuf[1], 1);
	abyteBuf[3] = byteTagID;
	POKE_WORD(&abyteBuf[4], static_cast<uint16>(strMultiByteBuf.GetLength()));

	file.Write(abyteBuf, sizeof(abyteBuf));
	file.Write(strMultiByteBuf.GetString(), strMultiByteBuf.GetLength());

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
