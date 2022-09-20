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
#define OP_MODPROT				0x4D // 'M' 
#define	OP_MODPACKEDPROT 		0x6D // 'm'

#define OP_MODINFOPACKET		0x01

#define MT_MOD_PROTOCOL_EXTENSIONS  0x4D //'M'
#define MT_MOD_PROTOCOL_EXTENSIONS2 0x6D //'m'

#define MT_NEO_PROTOCOL_EXTENSIONS  0x4E //'N'
#define MT_NEO_PROTOCOL_EXTENSIONS2 0x6E //'n'

#define OP_MODMULTIPACKET		0x11	// NEO: NMPm - [NeoModProtMultiPacket]
#define OP_MODMULTIPACKETANSWER	0x12	// NEO: NMPm - [NeoModProtMultiPacket]
// NEO: NMP END <-- Xanatos --

#define ET_INCOMPLETEPARTS		0x3D	// NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
#define OP_FILEINCSTATUS		0x8E	// NEO: ICS - [InteligentChunkSelection] <-- Xanatos -- 

#define OP_SUBCHUNKMAPS			0x53	// NEO: SCT - [SubChunkTransfer] <-- Xanatos --


#define OP_NEO_ANSWERSOURCES	0x82	// NEO: NXS - [NeoXS] <-- Xanatos --

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
#define	OP_NAT_CARIER			OP_MODPROT

#define OP_NAT_PING				0xD7

#define OP_NAT_SYN				0xD0	// <Ver 1>(<MSS 4>)
#define OP_NAT_SYN_ACK			0xD1	// <Ver 1>(<MSS 4>)

#define OP_NAT_DATA				0xD2	// <SeqNr 4>{data}
#define OP_NAT_DATA_ACK			0xD3	// <SeqNr 4><RecvWndSize 4>

#define OP_NAT_FIN				0xD4	// (empty)
#define OP_NAT_FIN_ACK			0xD5	// (empty)

#define OP_NAT_RST				0xD6	// (<Err 4>)

//
#define MT_EMULE_BUDDYID		0x40 //'@'	// <hash 16>
#define MT_XS_EMULE_BUDDYIP		0x42 //'B'	// <ip 4>
#define MT_XS_EMULE_BUDDYUDP	0x62 //'b'	// <port 2>

#define OP_NAT_CALLBACKREQUEST_KAD	0xB0

//
#define OP_XS_BUDDY_REQ			0xB1	// empty
#define OP_XS_BUDDY_ANSWER		0xB2	// <result 1>
#define OP_XS_BUDDYPING			0xB3	// empty
#define OP_XS_MULTICALLBACKUDP	0xB4	//
#define OP_XS_MULTICALLBACKTCP	0xB5	//

#define OP_CALLBACKREQUEST_XS		0xBA //
#define OP_NAT_CALLBACKREQUEST_XS	0xBB //

//
#define OP_PUBLICPORT_REQ		0xA7	// empty
#define	OP_PUBLICPORT_ANSWER	0xA8	// <port 2>

#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
