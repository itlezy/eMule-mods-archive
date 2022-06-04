#pragma once

// netfinity: Changed to types for better portability between different CPU targets
typedef unsigned char		uchar;
/*typedef unsigned char		uint8;
typedef	  signed char		sint8;

typedef unsigned short		uint16;
typedef	  signed short		sint16;

typedef unsigned int		uint32;
typedef	  signed int		sint32;*/

typedef unsigned __int8		uint8;
typedef	  signed __int8		sint8;

typedef unsigned __int16	uint16;
typedef	  signed __int16	sint16;

typedef unsigned __int32	uint32;
typedef	  signed __int32	sint32;

typedef unsigned __int64	uint64;
typedef   signed __int64	sint64;

#ifdef _WIN64
typedef uint64	uint_ptr;
typedef sint64	sint_ptr;
#define INT_PTR_MIN LLONG_MIN
#define _ultot_s_ptr _ui64tot_s
#define _ultoa_s_ptr _ui64toa_s
#define _rotl_ptr _rotl64
#else
typedef uint32	uint_ptr;
typedef	sint32	sint_ptr;
#define INT_PTR_MIN INT_MIN
#define _ultot_s_ptr _ultot_s
#define _ultoa_s_ptr _ultoa_s
#define _rotl_ptr _rotl
#endif

#ifdef _DEBUG
#include "Debug_FileSize.h"
#define USE_DEBUG_EMFILESIZE
typedef CEMFileSize			EMFileSize;
#else
typedef unsigned __int64	EMFileSize;
#endif

