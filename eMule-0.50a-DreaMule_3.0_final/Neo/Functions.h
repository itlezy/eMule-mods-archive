//this file is part of NeoMule
//Copyright (C)2006 David Xanatos ( Xanatos@Lycos.at / http://neomule.sourceforge.net )
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


// NEO: NMP - [NeoModProt] -- Xanatos -->
enum NanoTagID
{
	// short ID's, 1-4 bytes content 
	//				= 0
	NT_NATT			= 1,
//	NT_ISPT			= 10,
	NT_OBFU			= 15,
	//				= 31

	// long ID's, max 256 bytes content
	NT_BuddyID		= 32,
	NT_BuddyIPPort	= 33,
	NT_ServerIPPort = 34,
	NT_XsBuddyIPPort= 35,
//	NT_SecPort		= 40,
	//				= 127
};


void WriteNanoTagIDLen(byte* &pBuffer, uint8 uID, uint8 uLen = 1);
uint8 GetNanoTagID(byte* pBuffer);
uint8 GetNanoTagLen(byte* &pBuffer);
// NEO: NMP END <-- Xanatos --
