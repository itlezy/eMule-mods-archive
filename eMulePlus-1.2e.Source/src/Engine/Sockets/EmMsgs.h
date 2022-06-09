/////////////////////////////////////////////////////////
// EmMsgs.h - definition of all eMule messages (opcodes)
//
// NOTE Don't put <#pragma once> directive here. 
// This file should be included multiple times.



/////////////////////////////////////////////////////////
// Client OpCodes 
/////////////////////////////////////////////////////////

// Both HELLO and HELLOANSWER have almost the same parameters.

#ifndef OPCODE_HELLO_HELLOANSWER_PARAMS
	#define OPCODE_HELLO_HELLOANSWER_PARAMS \
		PARAM_HASH(UserHash) \
		PARAM_ADDRPORT(ClientAddr) \
		PARAM_TAGS_BEGIN \
			PARAM_TAG_STR(UserName,			CT_NAME) \
			PARAM_TAG_DWORD(SoftVersion,	CT_VERSION) \
			PARAM_TAG_DWORD(UserPort,		CT_PORT) \
			PARAM_TAG_STR(ModVersion,		CT_MOD_VERSION) \
			PARAM_TAG_DWORD(UserUDPPort,	CT_EMULE_UDPPORTS) \
			PARAM_TAG_DWORD(MiscOptions,	CT_EMULE_MISCOPTIONS1) \
			PARAM_TAG_DWORD(MuleVersion,	CT_EMULE_VERSION) \
		PARAM_TAGS_END \
		PARAM_ADDRPORT(ServerAddr)
#endif //OPCODE_HELLO_HELLOANSWER_PARAMS

/////////////////////////////////////////////////////////
// Hello OpCode, sent right after connecting to client
BEGIN_OPCODE(0x01, HELLO, EDONKEY, PEER)
	PARAM_BYTE_JUNK(16)
	OPCODE_HELLO_HELLOANSWER_PARAMS
END_OPCODE

/////////////////////////////////////////////////////////
// Answer to Hello OpCode
BEGIN_OPCODE(0x4C, HELLOANSWER, EDONKEY, PEER)
	OPCODE_HELLO_HELLOANSWER_PARAMS
END_OPCODE


// Both EMULEINFO and EMULEINFOANSWER have exactly the same parameters.
#ifndef OPCODE_EMULE_EMULEINFO_PARAMS
	#define OPCODE_EMULE_EMULEINFO_PARAMS \
		PARAM_BYTE(ClientVersion) \
		PARAM_BYTE(ProtocolVersion) \
		PARAM_TAGS_BEGIN \
			PARAM_TAG_DWORD(Compression,	ET_COMPRESSION) \
			PARAM_TAG_DWORD(UdpVersion,		ET_UDPVER) \
			PARAM_TAG_DWORD(UdpPort,		ET_UDPPORT) \
			PARAM_TAG_DWORD(SourceExchange,	ET_SOURCEEXCHANGE) \
			PARAM_TAG_DWORD(Comments,		ET_COMMENTS) \
			PARAM_TAG_DWORD(ModPlus,		ET_MOD_PLUS) \
			PARAM_TAG_DWORD(ExtendedRequest,ET_EXTENDEDREQUEST) \
			PARAM_TAG_STR(ModVersion,		ET_MOD_VERSION) \
			PARAM_TAG_DWORD(L2Hac,			ET_L2HAC) \
			PARAM_TAG_DWORD(Features,		ET_FEATURES) \
 		PARAM_TAGS_END \
//		PARAM_ADDRPORT(ServerAddr) \

#endif // OPCODE_EMULE_EMULEINFO_PARAMS


/////////////////////////////////////////////////////////
// EmuleInfo OpCode
BEGIN_OPCODE(0x01, EMULEINFO, EMULE, PEER)
	OPCODE_EMULE_EMULEINFO_PARAMS
END_OPCODE

BEGIN_OPCODE(0x02, EMULEINFOANSWER, EMULE, PEER)
	OPCODE_EMULE_EMULEINFO_PARAMS
END_OPCODE


/////////////////////////////////////////////////////////
// Offer files
BEGIN_OPCODE(0x15, OFFERFILES, EDONKEY, PEER)
	PARAM_COMPLEXARRAY_BEGIN(OFFERFILES, Files)
		PARAM_HASH(Hash)
		PARAM_ADDRPORT(ClientAddr)
		PARAM_TAGS_BEGIN
			PARAM_TAG_STR(FileName,		FT_FILENAME)
			PARAM_TAG_DWORD(FileSize,	FT_FILESIZE)
			PARAM_TAG_STR(FileType,		FT_FILETYPE)
		PARAM_TAGS_END
	PARAM_COMPLEXARRAY_END(Files)
END_OPCODE

/////////////////////////////////////////////////////////
// Request filename
BEGIN_OPCODE(0x58, REQUESTFILENAME, EDONKEY, PEER)
	PARAM_HASH(Hash)
END_OPCODE

/////////////////////////////////////////////////////////
// Request filename answer
BEGIN_OPCODE(0x59, REQFILENAMEANSWER, EDONKEY, PEER)
	PARAM_HASH(Hash)
	PARAM_STRING(FileName)
END_OPCODE

/////////////////////////////////////////////////////////
// Request file upload
BEGIN_OPCODE(0x54, STARTUPLOADREQ, EDONKEY, PEER)
	PARAM_HASH(Hash)
END_OPCODE

/////////////////////////////////////////////////////////
// Accept file upload
BEGIN_OPCODE(0x55, ACCEPTUPLOADREQ, EDONKEY, PEER)
	// Empty
END_OPCODE

/////////////////////////////////////////////////////////
// No such file
BEGIN_OPCODE(0x48, FILEREQANSNOFIL, EDONKEY, PEER)
	PARAM_HASH(Hash)
END_OPCODE

/////////////////////////////////////////////////////////
// No such file
BEGIN_OPCODE(0x47, REQUESTPARTS, EDONKEY, PEER)
	PARAM_HASH(Hash)
	PARAM_DWORD(Start1)
	PARAM_DWORD(Start2)
	PARAM_DWORD(Start3)
	PARAM_DWORD(End1)
	PARAM_DWORD(End2)
	PARAM_DWORD(End3)
END_OPCODE

/////////////////////////////////////////////////////////
// Sending file part
BEGIN_OPCODE(0x46, SENDINGPART, EDONKEY, PEER)
	PARAM_HASH(Hash)
	PARAM_DATABLOCK(Data)
END_OPCODE

/////////////////////////////////////////////////////////
// Set requested file
BEGIN_OPCODE(0x4F, SETREQFILEID, EDONKEY, PEER)
	PARAM_HASH(Hash)
END_OPCODE

/////////////////////////////////////////////////////////
// File status
BEGIN_OPCODE(0x50, FILESTATUS, EDONKEY, PEER)
	PARAM_HASH(Hash)
	PARAM_BITARRAY(FileStatus)
END_OPCODE

/////////////////////////////////////////////////////////
// Hash request
BEGIN_OPCODE(0x51, HASHSETREQUEST, EDONKEY, PEER)
	PARAM_HASH(Hash)
END_OPCODE


/////////////////////////////////////////////////////////
// Hash request answer
BEGIN_OPCODE(0x52, HASHSETANSWER, EDONKEY, PEER)
	PARAM_HASH(Hash)
	PARAM_ARRAY(Hashsets, HashType, USHORT)
END_OPCODE


/////////////////////////////////////////////////////////
// Upload ranking info
BEGIN_OPCODE(0x60, QUEUERANKING, EMULE, PEER)
	PARAM_USHORT(Rank)
	// Filling junk
	PARAM_DWORD(Junk1)
	PARAM_DWORD(Junk2)
	PARAM_USHORT(Junk3)
END_OPCODE


/////////////////////////////////////////////////////////
// Cancel transfer
BEGIN_OPCODE(0x56, CANCELTRANSFER, EDONKEY, PEER)
	// Empty
END_OPCODE


/////////////////////////////////////////////////////////
// Server OpCodes
/////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
// Login request
BEGIN_OPCODE(0x01, LOGINREQUEST, EDONKEY, SERVER)
	PARAM_HASH(UserHash)
	PARAM_ADDRPORT(ClientAddr)
	PARAM_TAGS_BEGIN
		PARAM_TAG_STR(UserName,				CT_NAME)
		PARAM_TAG_DWORD(ClientSoftVersion,	CT_VERSION)
		PARAM_TAG_DWORD(UserPort,			CT_PORT)
		PARAM_TAG_DWORD(Flags,				CT_SERVER_FLAGS)
		PARAM_TAG_DWORD(eMuleVersion,		CT_EMULE_VERSION)
	PARAM_TAGS_END
END_OPCODE

/////////////////////////////////////////////////////////
// Server message
BEGIN_OPCODE(0x38, SERVERMESSAGE, EDONKEY_EMULE, PEER_SERVER)
	PARAM_STRING(Msg)
	#ifdef PARAM_INTERNAL_DEFINITION // Internal variables
	int			m_nMsgCount;
	#endif // PARAM_INTERNAL_DEFINITION
END_OPCODE


/////////////////////////////////////////////////////////
// ID change
BEGIN_OPCODE(0x40, IDCHANGE, EDONKEY, SERVER)
	PARAM_DWORD(NewClientID)
	PARAM_DWORD(SupportedFeatures)
END_OPCODE

/////////////////////////////////////////////////////////
// Search results
BEGIN_OPCODE(0x33, SEARCHRESULT, EDONKEY, SERVER)
	// To do
END_OPCODE

/////////////////////////////////////////////////////////
// Found sources
BEGIN_OPCODE(0x42, FOUNDSOURCES, EDONKEY, SERVER)
	PARAM_HASH(Hash)
	PARAM_ARRAY(Sources, AddrPort, BYTE)
END_OPCODE

/////////////////////////////////////////////////////////
// Server status update
BEGIN_OPCODE(0x34, SERVERSTATUS, EDONKEY, SERVER)
	PARAM_DWORD(NumberOfUsers)
	PARAM_DWORD(NumberOfFiles)
END_OPCODE

/////////////////////////////////////////////////////////
// Server info
BEGIN_OPCODE(0x41, SERVERIDENT, EDONKEY, SERVER)
	PARAM_HASH(ServerHash)
	PARAM_ADDRPORT(ServerAddr)

	PARAM_TAGS_BEGIN
		PARAM_TAG_STR(ServerName,		ST_SERVERNAME)
		PARAM_TAG_STR(ServerDescription,ST_DESCRIPTION)
	PARAM_TAGS_END
END_OPCODE

/////////////////////////////////////////////////////////
// Server list
BEGIN_OPCODE(0x32, SERVERLIST, EDONKEY, SERVER)
	PARAM_ARRAY(Servers, AddrPort, BYTE)
END_OPCODE

/////////////////////////////////////////////////////////
BEGIN_OPCODE(0x1C, CALLBACKREQUEST, EDONKEY, SERVER)
	PARAM_DWORD(UserID)
END_OPCODE

/////////////////////////////////////////////////////////
//	Potential response to CALLBACKREQUEST. Unobserved in
//		the wild ;)
BEGIN_OPCODE(0x36, CALLBACKFAIL, EDONKEY, SERVER)
	// Todo
END_OPCODE

/////////////////////////////////////////////////////////
// Some client request callback through server
BEGIN_OPCODE(0x35, CALLBACKREQUESTED, EDONKEY, SERVER)
	PARAM_ADDRPORT(ClientAddr)
END_OPCODE
