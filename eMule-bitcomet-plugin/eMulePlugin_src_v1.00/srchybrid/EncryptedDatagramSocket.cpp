//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

/* Basic Obfusicated Handshake Protocol UDP:
	see EncryptedStreamSocket.h

****************************** ED2K Packets

	-Keycreation Client <-> Clinet:
	 - Client A (Outgoing connection):
				Sendkey:	Md5(<UserHashClientB 16><IPClientA 4><MagicValue91 1><RandomKeyPartClientA 2>)  23
	 - Client B (Incomming connection):
				Receivekey: Md5(<UserHashClientB 16><IPClientA 4><MagicValue91 1><RandomKeyPartClientA 2>)  23
	 - Note: The first 1024 Bytes will be _NOT_ discarded for UDP keys to safe CPU time

	- Handshake
			-> The handshake is encrypted - except otherwise noted - by the Keys created above
			-> Padding is cucrently not used for UDP meaning that PaddingLen will be 0, using PaddingLens up to 16 Bytes is acceptable however
		Client A: <SemiRandomNotProtocolMarker 7 Bits[Unencrypted]><ED2K Marker 1Bit = 1><RandomKeyPart 2[Unencrypted]><MagicValue 4><PaddingLen 1><RandomBytes PaddingLen%16>	
	
	- Additional Comments:
			- For obvious reasons the UDP handshake is actually no handshake. If a different Encryption method (or better a different Key) is to be used this has to be negotiated in a TCP connection
		    - SemiRandomNotProtocolMarker is a Byte which has a value unequal any Protocol header byte. This is a compromiss, turning in complete randomness (and nice design) but gaining
			  a lower CPU usage
		    - Kad/Ed2k Marker are only indicators, which possibility could be tried first, and should not be trusted

****************************** Server Packets

	-Keycreation Client <-> Server:
	 - Client A (Outgoing connection client -> server):
				Sendkey:	Md5(<BaseKey 4><MagicValueClientServer 1><RandomKeyPartClientA 2>)  7
	 - Client B (Incomming connection):
				Receivekey: Md5(<BaseKey 4><MagicValueServerClient 1><RandomKeyPartClientA 2>)  7
	 - Note: The first 1024 Bytes will be _NOT_ discarded for UDP keys to safe CPU time

	- Handshake
			-> The handshake is encrypted - except otherwise noted - by the Keys created above
			-> Padding is cucrently not used for UDP meaning that PaddingLen will be 0, using PaddingLens up to 16 Bytes is acceptable however
		Client A: <SemiRandomNotProtocolMarker 1[Unencrypted]><RandomKeyPart 2[Unencrypted]><MagicValue 4><PaddingLen 1><RandomBytes PaddingLen%16>	

	- Overhead: 8 Bytes per UDP Packet
	
	- Security for Basic Obfuscation:
			- Random looking packets, very limited protection against passive eavesdropping single packets
	
	- Additional Comments:
			- For obvious reasons the UDP handshake is actually no handshake. If a different Encryption method (or better a different Key) is to be used this has to be negotiated in a TCP connection
		    - SemiRandomNotProtocolMarker is a Byte which has a value unequal any Protocol header byte. This is a compromiss, turning in complete randomness (and nice design) but gaining
			  a lower CPU usage

****************************** KAD Packets
			  
	-Keycreation Client <-> Client:
	 - Client A (Outgoing connection):
				Sendkey:	Md5(<KadID 16><RandomKeyPartClientA 2>)  18
	 - Client B (Incomming connection):
				Receivekey: Md5(<KadID 16><RandomKeyPartClientA 2>)  18
	 - Note: The first 1024 Bytes will be _NOT_ discarded for UDP keys to safe CPU time

	- Handshake
			-> The handshake is encrypted - except otherwise noted - by the Keys created above
			-> Padding is cucrently not used for UDP meaning that PaddingLen will be 0, using PaddingLens up to 16 Bytes is acceptable however
		Client A: <SemiRandomNotProtocolMarker 7 Bits[Unencrypted]><Kad Marker 1Bit = 0><RandomKeyPart 2[Unencrypted]><MagicValue 4><PaddingLen 1><RandomBytes PaddingLen%16><ReceiverVerifyKey 2><SenderVerifyKey 2>

	- Overhead: 12 Bytes per UDP Packet
	
	- Additional Comments:
			- For obvious reasons the UDP handshake is actually no handshake. If a different Encryption method (or better a different Key) is to be used this has to be negotiated in a TCP connection
		    - SemiRandomNotProtocolMarker is a Byte which has a value unequal any Protocol header byte. This is a compromiss, turning in complete randomness (and nice design) but gaining
			  a lower CPU usage
		    - Kad/Ed2k Marker are only indicators, which possibility could be tried first, and should not be trusted
*/

#include "stdafx.h"
#include "EncryptedDatagramSocket.h"
#include "emule.h"
#include "md5sum.h"
#include "Log.h"
#include "preferences.h"
#include "opcodes.h"
#include "otherfunctions.h"
#include "Statistics.h"
#include "safefile.h"
#include "./kademlia/kademlia/prefs.h"
#include "./kademlia/kademlia/kademlia.h"

// random generator
#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4702) // unreachable code
#include <crypto51/osrng.h>
#pragma warning(default:4702) // unreachable code
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative

#define CRYPT_HEADER_WITHOUTPADDING		    8
#define	MAGICVALUE_UDP						91
#define MAGICVALUE_UDP_SYNC_CLIENT			0x395F2EC1
#define MAGICVALUE_UDP_SYNC_SERVER			0x13EF24D5
#define	MAGICVALUE_UDP_SERVERCLIENT			0xA5
#define	MAGICVALUE_UDP_CLIENTSERVER			0x6B

static CryptoPP::AutoSeededRandomPool cryptRandomGen;

CEncryptedDatagramSocket::CEncryptedDatagramSocket(){
}

CEncryptedDatagramSocket::~CEncryptedDatagramSocket(){

}

int CEncryptedDatagramSocket::DecryptReceivedClient(BYTE* pbyBufIn, int nBufLen, BYTE** ppbyBufOut, uint32 dwIP, uint16* nReceiverVerifyKey, uint16* nSenderVerifyKey) const{
	int nResult = nBufLen;
	*ppbyBufOut = pbyBufIn;
	
	if (nResult <= CRYPT_HEADER_WITHOUTPADDING /*|| !thePrefs.IsClientCryptLayerSupported()*/)
		return nResult;

	if (nReceiverVerifyKey == NULL || nSenderVerifyKey == NULL){
		ASSERT( false );
		return nResult;
	}
	
	switch (pbyBufIn[0]){
		case OP_EMULEPROT:
		case OP_KADEMLIAPACKEDPROT:
		case OP_KADEMLIAHEADER:
		case OP_UDPRESERVEDPROT1:
		case OP_UDPRESERVEDPROT2:
		case OP_PACKEDPROT:
			return nResult; // no encrypted packet (see description on top)
	}
	bool bKad = (pbyBufIn[0] & 0x01) == 0; // check the marker bit if this is a kad or ed2k packet, this is only an indicator since old clients have it set random
	// might be an encrypted packet, try to decrypt
	
	RC4_Key_Struct keyReceiveKey;
	uint32 dwValue = 0;
	bool bFlipTry = false;
	do{
		bKad = bFlipTry ? !bKad : bKad;
		MD5Sum md5;
		if (bKad){
			if (Kademlia::CKademlia::GetPrefs()) {
				uchar achKeyData[18];
				memcpy(achKeyData, Kademlia::CKademlia::GetPrefs()->GetKadID().GetData(), 16);
				memcpy(achKeyData + 16, pbyBufIn + 1, 2); // random key part sent from remote client
				md5.Calculate(achKeyData, sizeof(achKeyData));
			}
		}
		else{
			uchar achKeyData[23];
			md4cpy(achKeyData, thePrefs.GetUserHash());
			achKeyData[20] = MAGICVALUE_UDP;
			memcpy(achKeyData + 16, &dwIP, 4);
			memcpy(achKeyData + 21, pbyBufIn + 1, 2); // random key part sent from remote client
			md5.Calculate(achKeyData, sizeof(achKeyData));
		}
		RC4CreateKey(md5.GetRawHash(), 16, &keyReceiveKey, true);
		RC4Crypt(pbyBufIn + 3, (uchar*)&dwValue, sizeof(dwValue), &keyReceiveKey);
		bFlipTry = !bFlipTry; // next round try the other possibility
	} while (dwValue != MAGICVALUE_UDP_SYNC_CLIENT && bFlipTry); // try to decrypt as ed2k as well as kad packet if needed (max 2 rounds)
	
	if (dwValue == MAGICVALUE_UDP_SYNC_CLIENT){
		// yup this is an encrypted packet
		DEBUG_ONLY( DebugLog(_T("Received obfuscated UDP packet from clientIP: %s"), ipstr(dwIP)) );
		uint8 byPadLen;
		RC4Crypt(pbyBufIn + 7, (uchar*)&byPadLen, 1, &keyReceiveKey);
		nResult -= CRYPT_HEADER_WITHOUTPADDING;
		if (nResult <= byPadLen){
			DebugLogError(_T("Invalid obfuscated UDP packet from clientIP: %s, Paddingsize (%u) larger than received bytes"), ipstr(dwIP), byPadLen);
			return nBufLen; // pass through, let the Receivefunction do the errorhandling on this junk
		}
		if (byPadLen > 0)
			RC4Crypt(NULL, NULL, byPadLen, &keyReceiveKey);
		nResult -= byPadLen;

		if (bKad){
			if (nResult <= 4){
				DebugLogError(_T("Obfuscated Kad packet with mismatching size (verify keys missing) received from clientIP: %s"), ipstr(dwIP));
				return nBufLen; // pass through, let the Receivefunction do the errorhandling on this junk;
			}
			// read the verify keys
			*nReceiverVerifyKey = PeekUInt16(pbyBufIn + CRYPT_HEADER_WITHOUTPADDING + byPadLen);
			*nSenderVerifyKey = PeekUInt16(pbyBufIn + CRYPT_HEADER_WITHOUTPADDING + byPadLen + 2);
			nResult -= 4;
		}
		else{
			*nReceiverVerifyKey = 0;
			*nSenderVerifyKey = 0;
		}
		*ppbyBufOut = pbyBufIn + (nBufLen - nResult);
		RC4Crypt((uchar*)*ppbyBufOut, (uchar*)*ppbyBufOut, nResult, &keyReceiveKey);
		theStats.AddDownDataOverheadCrypt(nBufLen - nResult);
		return nResult; // done
	}
	else{
		DebugLogWarning(_T("Obfuscated packet expected but magicvalue mismatch on UDP packet from clientIP: %s"), ipstr(dwIP));
		return nBufLen; // pass through, let the Receivefunction do the errorhandling on this junk
	}
}

int CEncryptedDatagramSocket::EncryptSendClient(uchar** ppbyBuf, int nBufLen, const uchar* pachClientHashOrKadID, bool bKad, uint16 nReceiverVerifyKey, uint16 nSenderVerifyKey) const{
	ASSERT( theAppPtr->GetPublicIP() != 0 || bKad );
	ASSERT( thePrefs.IsClientCryptLayerSupported() );

	uint8 byPadLen = 0;			// padding disabled for UDP currently
	const uint32 nCryptHeaderLen = byPadLen + CRYPT_HEADER_WITHOUTPADDING + (bKad ? 4 : 0);
	uint32 nCryptedLen = nBufLen + nCryptHeaderLen;
	uchar* pachCryptedBuffer = new uchar[nCryptedLen];
	
	uint16 nRandomKeyPart = (uint16)cryptRandomGen.GenerateWord32(0x0000, 0xFFFF);
	MD5Sum md5;
	if (bKad){
		uchar achKeyData[18];
		md4cpy(achKeyData, pachClientHashOrKadID);
		memcpy(achKeyData+16, &nRandomKeyPart, 2);
		md5.Calculate(achKeyData, sizeof(achKeyData));
	}
	else{
		uchar achKeyData[23];
		md4cpy(achKeyData, pachClientHashOrKadID);
		uint32 dwIP = theAppPtr->GetPublicIP();
		memcpy(achKeyData+16, &dwIP, 4);
		memcpy(achKeyData+21, &nRandomKeyPart, 2);
		achKeyData[20] = MAGICVALUE_UDP;
		md5.Calculate(achKeyData, sizeof(achKeyData));
	}
	RC4_Key_Struct keySendKey;
	RC4CreateKey(md5.GetRawHash(), 16, &keySendKey, true);

	// create the semi random byte encryption header
	uint8 bySemiRandomNotProtocolMarker = 0;
	int i;
	for (i = 0; i < 128; i++){
		bySemiRandomNotProtocolMarker = cryptRandomGen.GenerateByte();
		bySemiRandomNotProtocolMarker = bKad ? (bySemiRandomNotProtocolMarker & 0xFE) : (bySemiRandomNotProtocolMarker | 0x01); // set the ed2k/kad marker bit

		bool bOk = false;
		switch (bySemiRandomNotProtocolMarker){ // not allowed values
			case OP_EMULEPROT:
			case OP_KADEMLIAPACKEDPROT:
			case OP_KADEMLIAHEADER:
			case OP_UDPRESERVEDPROT1:
			case OP_UDPRESERVEDPROT2:
			case OP_PACKEDPROT:
				break;
			default:
				bOk = true;
		}
		if (bOk)
			break;
	}
	if (i >= 128){
		// either we have _real_ bad luck or the randomgenerator is a bit messed up
		ASSERT( false );
		bySemiRandomNotProtocolMarker = 0x01;
	}

	uint32 dwMagicValue = MAGICVALUE_UDP_SYNC_CLIENT;
	pachCryptedBuffer[0] = bySemiRandomNotProtocolMarker;
	memcpy(pachCryptedBuffer + 1, &nRandomKeyPart, 2);
	RC4Crypt((uchar*)&dwMagicValue, pachCryptedBuffer + 3, 4, &keySendKey);
	RC4Crypt((uchar*)&byPadLen, pachCryptedBuffer + 7, 1, &keySendKey);

	for (int j = 0; j < byPadLen; j++){
		uint8 byRand = (uint8)rand();	// they actually dont really need to be random, but it doesn't hurts either
		RC4Crypt((uchar*)&byRand, pachCryptedBuffer + CRYPT_HEADER_WITHOUTPADDING + j, 1, &keySendKey);
	}

	if (bKad){
		RC4Crypt((uchar*)&nReceiverVerifyKey, pachCryptedBuffer + CRYPT_HEADER_WITHOUTPADDING + byPadLen, 2, &keySendKey);
		RC4Crypt((uchar*)&nSenderVerifyKey, pachCryptedBuffer + CRYPT_HEADER_WITHOUTPADDING + byPadLen + 2, 2, &keySendKey);
	}

	RC4Crypt(*ppbyBuf, pachCryptedBuffer + nCryptHeaderLen, nBufLen, &keySendKey);
	delete[] *ppbyBuf;
	*ppbyBuf = pachCryptedBuffer;

	theStats.AddUpDataOverheadCrypt(nCryptedLen - nBufLen);
	return nCryptedLen;
}

int CEncryptedDatagramSocket::DecryptReceivedServer(BYTE* pbyBufIn, int nBufLen, BYTE** ppbyBufOut, uint32 dwBaseKey, uint32 dbgIP) const{
	int nResult = nBufLen;
	*ppbyBufOut = pbyBufIn;
	
	if (nResult <= CRYPT_HEADER_WITHOUTPADDING || !thePrefs.IsServerCryptLayerUDPEnabled() || dwBaseKey == 0)
		return nResult;
	
	if(pbyBufIn[0] == OP_EDONKEYPROT)
			return nResult; // no encrypted packet (see description on top)

	// might be an encrypted packet, try to decrypt
	uchar achKeyData[7];
	memcpy(achKeyData, &dwBaseKey, 4);
	achKeyData[4] = MAGICVALUE_UDP_SERVERCLIENT;
	memcpy(achKeyData + 5, pbyBufIn + 1, 2); // random key part sent from remote server
	MD5Sum md5(achKeyData, sizeof(achKeyData));
	RC4_Key_Struct keyReceiveKey;
	RC4CreateKey(md5.GetRawHash(), 16, &keyReceiveKey, true);
	
	uint32 dwValue;
	RC4Crypt(pbyBufIn + 3, (uchar*)&dwValue, sizeof(dwValue), &keyReceiveKey);
	if (dwValue == MAGICVALUE_UDP_SYNC_SERVER){
		// yup this is an encrypted packet
		if (thePrefs.GetDebugServerUDPLevel() > 0)
			DEBUG_ONLY( DebugLog(_T("Received obfuscated UDP packet from ServerIP: %s"), ipstr(dbgIP)) );
		uint8 byPadLen;
		RC4Crypt(pbyBufIn + 7, (uchar*)&byPadLen, 1, &keyReceiveKey);
		byPadLen &= 15;
		nResult -= CRYPT_HEADER_WITHOUTPADDING;
		if (nResult <= byPadLen){
			DebugLogError(_T("Invalid obfuscated UDP packet from ServerIP: %s, Paddingsize (%u) larger than received bytes"), ipstr(dbgIP), byPadLen);
			return nBufLen; // pass through, let the Receivefunction do the errorhandling on this junk
		}
		if (byPadLen > 0)
			RC4Crypt(NULL, NULL, byPadLen, &keyReceiveKey);
		nResult -= byPadLen;
		*ppbyBufOut = pbyBufIn + (nBufLen - nResult);
		RC4Crypt((uchar*)*ppbyBufOut, (uchar*)*ppbyBufOut, nResult, &keyReceiveKey);
		
		theStats.AddDownDataOverheadCrypt(nBufLen - nResult);
		return nResult; // done
	}
	else{
		DebugLogWarning(_T("Obfuscated packet expected but magicvalue mismatch on UDP packet from ServerIP: %s"), ipstr(dbgIP));
		return nBufLen; // pass through, let the Receivefunction do the errorhandling on this junk
	}
}

int CEncryptedDatagramSocket::EncryptSendServer(uchar** ppbyBuf, int nBufLen, uint32 dwBaseKey) const{
	ASSERT( thePrefs.IsServerCryptLayerUDPEnabled() );
	ASSERT( dwBaseKey != 0 );
	
	uint8 byPadLen = 0;			// padding disabled for UDP currently
	uint32 nCryptedLen = nBufLen + byPadLen + CRYPT_HEADER_WITHOUTPADDING;
	uchar* pachCryptedBuffer = new uchar[nCryptedLen];
	
	uint16 nRandomKeyPart = (uint16)cryptRandomGen.GenerateWord32(0x0000, 0xFFFF);

	uchar achKeyData[7];
	memcpy(achKeyData, &dwBaseKey, 4);
	achKeyData[4] = MAGICVALUE_UDP_CLIENTSERVER;
	memcpy(achKeyData + 5, &nRandomKeyPart, 2);
	MD5Sum md5(achKeyData, sizeof(achKeyData));
	RC4_Key_Struct keySendKey;
	RC4CreateKey(md5.GetRawHash(), 16, &keySendKey, true);

	// create the semi random byte encryption header
	uint8 bySemiRandomNotProtocolMarker = 0;
	int i;
	for (i = 0; i < 128; i++){
		bySemiRandomNotProtocolMarker = cryptRandomGen.GenerateByte();
		if (bySemiRandomNotProtocolMarker != OP_EDONKEYPROT) // not allowed values
			break;
	}
	if (i >= 128){
		// either we have _real_ bad luck or the randomgenerator is a bit messed up
		ASSERT( false );
		bySemiRandomNotProtocolMarker = 0x01;
	}

	uint32 dwMagicValue = MAGICVALUE_UDP_SYNC_SERVER;
	pachCryptedBuffer[0] = bySemiRandomNotProtocolMarker;
	memcpy(pachCryptedBuffer + 1, &nRandomKeyPart, 2);
	RC4Crypt((uchar*)&dwMagicValue, pachCryptedBuffer + 3, 4, &keySendKey);
	RC4Crypt((uchar*)&byPadLen, pachCryptedBuffer + 7, 1, &keySendKey);

	for (int j = 0; j < byPadLen; j++){
		uint8 byRand = (uint8)rand();	// they actually dont really need to be random, but it doesn't hurts either
		RC4Crypt((uchar*)&byRand, pachCryptedBuffer + CRYPT_HEADER_WITHOUTPADDING + j, 1, &keySendKey);
	}
	RC4Crypt(*ppbyBuf, pachCryptedBuffer + CRYPT_HEADER_WITHOUTPADDING + byPadLen, nBufLen, &keySendKey);
	delete[] *ppbyBuf;
	*ppbyBuf = pachCryptedBuffer;

	theStats.AddUpDataOverheadCrypt(nCryptedLen - nBufLen);
	return nCryptedLen;
}
