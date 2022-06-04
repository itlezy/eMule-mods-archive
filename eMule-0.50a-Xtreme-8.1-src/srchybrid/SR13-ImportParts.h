#pragma once
#include "PartFile.h"
//#include "SR13-ImportParts.rc.h"

#define	MP_SR13_ImportParts			13000	/* Rowaa[SR13]: Import parts to file */
#define	MP_SR13_InitiateRehash		13001	/* Rowaa[SR13]: Initiates file rehash */

struct ImportPart_Struct {
	uint64 start;
	uint64 end;
	BYTE* data;
};

bool SR13_ImportParts(CPartFile* m_partfile, CString m_strFilePath);