#pragma once

#include <deque>

using namespace std;

#ifdef byte
#undef byte
#endif
typedef unsigned char		byte;
typedef  signed  char		sint8;
typedef unsigned short		uint16;
typedef  signed  short		sint16;
typedef unsigned long		uint32;
typedef  signed  long		sint32;
typedef unsigned __int64	uint64;
typedef signed __int64		sint64;
typedef unsigned char		uchar;
template <typename type> void safe_delete(type &del) { delete del; del = 0; }

extern "C" int __cdecl __ascii_stricmp(const char *dst, const char *src);

// clientsoft
enum EnumClientTypes
{
	SO_PLUS			=	0,
	SO_EMULE			,
	SO_AMULE			,
	SO_EDONKEYHYBRID	,
	SO_EDONKEY			,
	SO_MLDONKEY			,
	SO_OLDEMULE			,
	SO_SHAREAZA			,
	SO_XMULE			,
	SO_LPHANT			,
	SO_UNKNOWN			, // should be always before the last
	SO_LAST
};

enum EnumEndTransferSession 
{
	ETS_TIMEOUT			= 0,
	ETS_DISCONNECT,		
	ETS_BAN,
	ETS_CANCELED,
	ETS_END_OF_DOWNLOAD,
	ETS_FILE_ERROR,
	ETS_BLOCKED_CHUNK,
	ETS_TERMINATOR
};


#ifdef NEW_SOCKETS

#pragma pack(1)

struct AddrPort
{
	ULONG	Addr;
	USHORT	Port;
};

#pragma pack()

#endif //NEW_SOCKETS
