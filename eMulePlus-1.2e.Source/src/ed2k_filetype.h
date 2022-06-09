#pragma once
#include "types.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////
//	File Types (NB: used as index for tables)
enum EED2KFileType
{
	ED2KFT_ANY = 0,
	ED2KFT_AUDIO,
	ED2KFT_VIDEO,
	ED2KFT_IMAGE,
	ED2KFT_PROGRAM,
	ED2KFT_DOCUMENT,
	ED2KFT_ARCHIVE,
	ED2KFT_CDIMAGE,

	ED2KFT_COUNT
};
//typedef EnumDomain<_EED2KFileType> EED2KFileType;
///////////////////////////////////////////////////////////////////////////////////////////////////////
struct SED2KFileType
{
	LPCTSTR pszExt;
	EED2KFileType iFileType;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////
bool InitFileTypeArray(void);
int __cdecl CompareE2DKFileType(const void* p1, const void* p2);
EED2KFileType GetED2KFileTypeID(LPCTSTR pszFileName);
