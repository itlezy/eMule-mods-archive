#include "StdAfx.h"
#include "AbstractFile.h"
#include "../../opcodes.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CAbstractFile::CreateED2kLink() const
{
	CString		strLink;

	strLink.Format( _T("ed2k://|file|%s|%u|%s|/"),
					StripInvalidFilenameChars(GetFileName(), false),	// spaces to dots
					GetFileSize(),
					HashToString(16, GetFileHash()) );

	return strLink;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CAbstractFile::CreateED2kSourceLink() const
{
	uint32		dwID = 0;

#ifdef OLD_SOCKETS_ENABLED
	if (!g_eMuleApp.m_pServerConnect->IsConnected() || g_eMuleApp.m_pServerConnect->IsLowID())
	{
		return _T("");
	}
	dwID = g_eMuleApp.m_pServerConnect->GetClientID();
#endif //OLD_SOCKETS_ENABLED

	CString		strLink;

	strLink.Format( _T("ed2k://|file|%s|%u|%s|/|sources,%i.%i.%i.%i:%i|/"),
					StripInvalidFilenameChars(GetFileName(), false),	// spaces to dots
					GetFileSize(),
					HashToString(16, GetFileHash()),
					static_cast<BYTE>(dwID),
					static_cast<BYTE>(dwID>>8),
					static_cast<BYTE>(dwID>>16),
					static_cast<BYTE>(dwID>>24)
					//g_eMuleApp.m_pGlobPrefs->GetListenPort() 
					);

	return strLink;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CAbstractFile::CreateHTMLED2kLink() const
{
	return CString(_T("<a href=\"") + CreateED2kLink() + _T("\">") + StripInvalidFilenameChars(GetFileName(), true) + _T("</a>"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAbstractFile::SetFileName(const CString& NewName)
{
	m_strFileName = NewName;

	m_strFileName.Replace('/','-');
	m_strFileName.Replace('>','-');
	m_strFileName.Replace('<','-');
	m_strFileName.Replace('*','-');
	m_strFileName.Replace(':','-');
	m_strFileName.Replace('?','-');
	m_strFileName.Replace('|','-');
	m_strFileName.Replace('\"','-');
	m_strFileName.Replace('\\','-');

	if (!m_strFileName.IsEmpty())
		m_eFileType = GetED2KFileTypeID(m_strFileName);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CAbstractFile::GetFileTypeString()
{
	EMULE_TRY

	switch (m_eFileType)
	{
		case ED2KFT_AUDIO:
			return "Audio";
		case ED2KFT_VIDEO:
			return "Video";
		case ED2KFT_IMAGE:
			return "Image";
		case ED2KFT_ARCHIVE:
		case ED2KFT_CDIMAGE:
		case ED2KFT_PROGRAM:
			return "Pro";
		case ED2KFT_DOCUMENT:
			return "Doc";
		case ED2KFT_ANY:
		default:
			return "";
	}

	EMULE_CATCH

	return "";
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAbstractFile::SetFileSize(uint32 dwFileSize)
{
	m_dwFileSize = dwFileSize;

	// Examples of parthashs, hashsets and filehashs for different filesizes
	// according the ed2k protocol
	//----------------------------------------------------------------------
	//
	//File size: 3 bytes
	//File hash: 2D55E87D0E21F49B9AD25F98531F3724
	//Nr. hashs: 0
	//
	//
	//File size: 1*PARTSIZE
	//File hash: A72CA8DF7F07154E217C236C89C17619
	//Nr. hashs: 2
	//Hash[  0]: 4891ED2E5C9C49F442145A3A5F608299
	//Hash[  1]: 31D6CFE0D16AE931B73C59D7E0C089C0	*special part hash*
	//
	//
	//File size: 1*PARTSIZE + 1 byte
	//File hash: 2F620AE9D462CBB6A59FE8401D2B3D23
	//Nr. hashs: 2
	//Hash[  0]: 121795F0BEDE02DDC7C5426D0995F53F
	//Hash[  1]: C329E527945B8FE75B3C5E8826755747
	//
	//
	//File size: 2*PARTSIZE
	//File hash: A54C5E562D5E03CA7D77961EB9A745A4
	//Nr. hashs: 3
	//Hash[  0]: B3F5CE2A06BF403BFB9BFFF68BDDC4D9
	//Hash[  1]: 509AA30C9EA8FC136B1159DF2F35B8A9
	//Hash[  2]: 31D6CFE0D16AE931B73C59D7E0C089C0	*special part hash*
	//
	//
	//File size: 3*PARTSIZE
	//File hash: 5E249B96F9A46A18FC2489B005BF2667
	//Nr. hashs: 4
	//Hash[  0]: 5319896A2ECAD43BF17E2E3575278E72
	//Hash[  1]: D86EF157D5E49C5ED502EDC15BB5F82B
	//Hash[  2]: 10F2D5B1FCB95C0840519C58D708480F
	//Hash[  3]: 31D6CFE0D16AE931B73C59D7E0C089C0	*special part hash*
	//
	//
	//File size: 3*PARTSIZE + 1 byte
	//File hash: 797ED552F34380CAFF8C958207E40355
	//Nr. hashs: 4
	//Hash[  0]: FC7FD02CCD6987DCF1421F4C0AF94FB8
	//Hash[  1]: 2FE466AF8A7C06DA3365317B75A5ACFE
	//Hash[  2]: 873D3BF52629F7C1527C6E8E473C1C30
	//Hash[  3]: BCE50BEE7877BB07BB6FDA56BFE142FB
	//

	//   File size       Data parts      ED2K parts      ED2K part hashs
	// ---------------------------------------------------------------
	// 1..PARTSIZE-1     1               1                       0(!)
	// PARTSIZE            1               2(!)                   2(!)
	// PARTSIZE+1        2               2                       2
	// PARTSIZE*2        2               3(!)                   3(!)
	// PARTSIZE*2+1    3               3                       3

	if (dwFileSize == 0)
	{
		m_uPartCount = 0;
		m_dwED2KPartCount = 0;
		m_dwED2KPartHashCount = 0;
		return;
	}

	m_uPartCount = static_cast<uint16>(dwFileSize/PARTSIZE) + ((dwFileSize % PARTSIZE)? 1:0);

	m_dwED2KPartCount = dwFileSize / PARTSIZE + 1;

	m_dwED2KPartHashCount = dwFileSize / PARTSIZE;
	if (m_dwED2KPartHashCount != 0)
	{
		m_dwED2KPartHashCount += 1;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CAbstractFile::GetPartSize(uint16 uPart)
{
	// if filesize was not defined
	if (m_dwFileSize == 0)
	{
		return 0;
	}

	if ( (uPart+1) == GetPartCount() )
	{
		return (GetFileSize() % PARTSIZE);
	}

	return PARTSIZE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The  file function outside the classes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString StripInvalidFilenameChars(CString strText, bool bKeepSpaces)
{
	LPTSTR pszBuffer = strText.GetBuffer(strText.GetLength());
	LPTSTR pszSource = pszBuffer;
	LPTSTR pszDest = pszBuffer;

	while (*pszSource != '\0')
	{
		if (!((*pszSource <= 31 && *pszSource >= 0)	|| // lots of invalid chars for filenames in windows :=)
			*pszSource == '\"' || *pszSource == '*' || *pszSource == '<'  || *pszSource == '>' ||
			*pszSource == '?'  || *pszSource == '|' || *pszSource == '\\' || *pszSource == '/' ||
			*pszSource == ':') )
		{
			if (!bKeepSpaces && *pszSource == ' ')
				*pszDest = '.';
			*pszDest = *pszSource;
			pszDest++;
		}
		pszSource++;
	}
	*pszDest = '\0';
	strText.ReleaseBuffer();
	return strText;
}