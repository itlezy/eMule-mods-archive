//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->

///////////////////
// OpCode List

#define OP_VOODOOPROT				0xF7 // Protocol ID
#define VOODOOVERSION				0x03 // Version
#define VOODOOVERSION_OLD			0x02 // Old version not compatible

#define OP_VOODOO_HELLO				0x01 // Handshake
#define OP_VOODOO_GOODBY			0x02 // Disconnect, avoid reconnect try
#define OP_VOODOO_PING				0x03 // ping to keep connection alive

#define OP_SPELL					0x00 // Inicialisation
#define OP_SPELL_RESULT				0xFF

#define OP_DOWNLOAD_QUEUE			0x10 // Sends download queue to synchronize

#define OP_NEW_DOWNLOAD_ORDER		0x11 // Order new download by transfering temp files, not longer supported
#define OP_DOWNLOAD_ORDER			0x20 // Order new download
#define OP_SHARE_ORDER				0x21 // Add new shared file

#define OP_FILE_UNAVALIBLE			0x12 // this voodoo file is not avalibly on this clinet
//#define OP_DOWNLOAD_UNAVALIBLE		0x13

#define OP_GAP_LIST_REQUEST			0x14 // Request gaplist
#define OP_GAP_LIST					0x15 // send gap list

#define OP_SHARED_FILE_LIST			0x16 // Sends shared file list to synchronize

#define OP_DOWNLOAD_INSTRUCTION		0x17 // Basic Download instructions
#define OP_SHARE_INSTRUCTION		0x18 // Basic share instructions

#define	OP_HASH_SET_REQUEST			0x44 // Request hashset form slave
#define	OP_HASH_SET_RESPONSE		0x45 // send hashset to master

#define	OP_DOWNLOAD_DATA			0xA0 // Data DownLink
#define	OP_THROTTLE_BLOCK			0xA1 // trothled block

#define	OP_CORRUPT_SENDER			0xAF // Corupted source to ban

#define	OP_DATA_REQUEST				0xB0 // Data UpLink
#define	OP_UPLOAD_DATA				0xB1

// stats
#define	OP_STATISTICS				0x25 // ul/dl speed statistic
#define	OP_FILE_STATISTICS			0x26 // ul/dl speed statistic

// NEO: VOODOOs - [VoodooSearchForwarding]
// voodoo search forwarding
#define	OP_SEARCH_REQUEST			0x35 // forwarded search request
#define	OP_SEARCH_STATE				0x36 // state, succes fail, error, progress
#define	OP_SEARCH_COMMAND			0x37 // stop, cacel, more
#define	OP_SEARCH_RESULT			0x38 // forwarded search response
// NEO: VOODOOs END

/////////////////////////
// Tags

// Hello tags
#define VT_NAME						0x01
#define VT_TYPE						0x02
#define VT_PORT						0x03
#define VT_ED2K						0x04
#define VT_PERM						0x05
#define VT_NEO						0x06 // old
#define VT_FEATURES					0x08
#define VT_FEATURES_EX				0x07 // old
#define VT_FEATURES_ED2K			0x09

// Hello operators
#define VH_QUERY					0x00
#define	VH_HELLO					0x01
#define	VH_ANSWER					0x02

// client software
#define	CT_UNKNOWN					0x00
#define	CT_ED2K						0x01
#define	CT_BITTORRENT				0x02

// stats
#define	ST_DOWN_DATARATE			0x01
#define	ST_UP_DATARATE				0x02

////////////////////////
// Value Definitions

// download instructions
#define INST_RESUME					0x01 // Resume Download
#define INST_PAUSE					0x02 // Pause Download
#define INST_STOP					0x03 // Stop Download
#define INST_DELETE					0x04 // Delete Download
#define INST_COMPLETE				0x05 // Complete download

// share instructions
#define INST_UNSHARE				0x06 // Unshare this file
#define INST_SHARE					0x07 // Share this file

// extended instructions
#define	INST_SUSPEND				0x08 // set suspended flag
#define	INST_STANDBY				0x09 // set standby flag
#define	INST_FORCE					0x0A // set force flag

// file priority
#define	INST_DL_PRIO				0x0B // Set download priority Flag
#define	INST_UL_PRIO				0x0C // Set upload priority Flag

// A4AF commands
#define	INST_FORCE_ALL_A4AF			0x0D
#define	INST_GET_ALL_A4AF			0x0E
#define	INST_DROP_ALL_A4AF			0x0F

// action operator
#define	VA_NONE						0x00
#define	VA_SLAVE					0x01 // Try to slave
#define	VA_MASTER					0x02 // Ask to be master
#define	VA_PARTNER					0x03 // booth slave and master
#define	VA_BLOCK					0x04 // block this client
#define	VA_QUERY					0x10 // Request only hellp packet

// file unavalibly reasons
#define RET_NONE					0x00
#define RET_REAL					0x01 // File is already present as real
#define RET_NEW						0x02 // Faild to add new file
#define RET_UNAVALIBLY				0x03 // File is not present on master
#define RET_UNKNOWN					0x04 // slave does not know this file
#define RET_BADSIZE					0x05 // file size dies not match, id association failed
#define RET_EXIST					0x06 // File already exist
#define RET_COMPLETE				0x07 // File is complete in share or on other master

// file type operators
#define	OP_TEMP						0x01 // Voodoo temp file
#define	OP_SHARE					0x02 // Voodoo share file

// NEO: VOODOOs - [VoodooSearchForwarding]
// voodoo search forwarding
#define SS_ERROR					0x00
#define SS_OFFLINE					0x01
#define SS_FINISH					0x02

#define	SF_DONE						0x00
//#define	SF_MORE						0x01
#define	SF_HALTED					0x02
#define	SF_END						0x03
#define	SF_KAD						0x04
#define	SF_ERROR					0x05

#define SE_GENERIC					0x00
#define SE_INVALID					0x01
#define SE_KAD						0x02

#define	SC_HOLD						0x00
#define	SC_CANCEL					0x01
#define	SC_MORE						0x02
// NEO: VOODOOs END

// NEO: VOODOOx - [VoodooSourceExchange]
// Source exchange
#define	OP_SOURCE_LIST_REQUEST		0x30 // request source list
#define	OP_SOURCE_LIST				0x31 // single source or entier source list
// NEO: VOODOOx END

 // NEO: VOODOOn - [VoodooForNeo]
#define	OP_DOWNLOAD_COMMAND			0x40 // Extended commands, batch format 
#define	OP_NEO_PREFERENCES			0x41 // Neo file preferences
// NEO: VOODOOn END

#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
