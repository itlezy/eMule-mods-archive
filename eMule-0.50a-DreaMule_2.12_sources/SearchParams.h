#pragma once

class CSafeMemFile;


///////////////////////////////////////////////////////////////////////////////
// ESearchType

enum ESearchType
{
	//NOTE: The numbers are *equal* to the entries in the comboxbox -> TODO: use item data
	SearchTypeEd2kServer = 0,
	SearchTypeEd2kGlobal,
	SearchTypeKademlia,
	SearchTypeFileDonkey
};


#define	MAX_SEARCH_EXPRESSION_LEN	512

///////////////////////////////////////////////////////////////////////////////
// SSearchParams

struct SSearchParams
{
	SSearchParams()
	{
		dwSearchID = (DWORD)-1;
		eType = SearchTypeEd2kServer;
		bClientSharedFiles = false;
		ullMinSize = 0;
		ullMaxSize = 0;
		uAvailability = 0;
		uComplete = 0;
		ulMinBitrate = 0;
		ulMinLength = 0;
		bMatchKeywords = false;
		bUnicode = true;
	}
	DWORD dwSearchID;
	bool bClientSharedFiles;
	CString strSearchTitle;
	CString strExpression;
	CString strKeyword;
	CString strBooleanExpr;
	ESearchType eType;
	CStringA strFileType;
	CString strMinSize;
	uint64 ullMinSize;
	CString strMaxSize;
	uint64 ullMaxSize;
	UINT uAvailability;
	CString strExtension;
	UINT uComplete;
	CString strCodec;
	ULONG ulMinBitrate;
	ULONG ulMinLength;
	CString strTitle;
	CString strAlbum;
	CString strArtist;
	CString strSpecialTitle;
	bool bMatchKeywords;
	bool bUnicode;
};

bool GetSearchPacket(CSafeMemFile* data, SSearchParams* pParams, bool bTargetSupports64Bit, bool* pbPacketUsing64Bit);
