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

#include "stdafx.h"
#include "Emule.h"
#include "Opcodes.h"
#include "Functions.h"
#include "resource.h"
#include "Preferences.h"
#include "MenuCmds.h"
#include "OtherFunctions.h"
#include "TitleMenu.h"

// NEO: NMP - [NeoModProt] -- Xanatos -->
void WriteNanoTagIDLen(byte* &pBuffer, uint8 uID, uint8 uLen)
{
	ASSERT((uLen <= 4 ) || (uID >= 32)); // be carefull with the id's
	// Note: we allow Long ID's with less than 5 chars content

	bool bShort = (uID < 32);

	(byte&)*pBuffer  = (bShort ? 0x00 : 0x01				<< 0); 
	(byte&)*pBuffer |= ((uID	 & (bShort ? 0x1f : 0x7f))	<< 1);

	if(bShort){
		(byte&)*pBuffer |= (((uLen-1) & 0x03)				<< 6);
	}else{
		pBuffer++;
		(byte&)*pBuffer = uLen;
	}
	pBuffer++;
}

uint8 GetNanoTagID(byte* pBuffer)
{
	return ((byte)*pBuffer >> 1) & (((byte)*pBuffer & 0x01) ? 0x7f : 0x1f);
}

uint8 GetNanoTagLen(byte* &pBuffer)
{
	uint8 uLen;
	if((byte)*pBuffer & 0x01){
		pBuffer++;
		uLen = (byte)*pBuffer;
	}else{
		uLen = (((byte)*pBuffer >> 6) & 0x03)+1;
	}
	pBuffer++;
	return uLen;
}
// NEO: NMP END <-- Xanatos --
