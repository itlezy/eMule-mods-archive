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

#pragma pack(1)
//			SERVER TO CLIENT
struct PacketHeader_Struct
{
	byte		byteEDonkeyProtocol;
	uint32		dwPacketLength;
	EnumOpcodes	eCommand;
};

struct UDP_Header_Struct
{
	byte		byteEDonkeyProtocol;
	EnumOpcodes	eCommand;
};
#pragma pack()

struct Requested_Block_Struct
{
	uint64		qwStartOffset;
	uint64		qwEndOffset;
	uchar		m_fileHash[16];
};

struct Requested_File_Struct
{
	uchar		m_fileHash[16];
	uint32		m_dwLastAskedTime;
	byte		m_byteNumBadRequests;
};

struct Gap_Struct
{
	uint64		qwStartOffset;
	uint64		qwEndOffset;
};
