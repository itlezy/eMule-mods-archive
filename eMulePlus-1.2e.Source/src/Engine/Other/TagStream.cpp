#include "StdAfx.h"
#include "TagStream.h"

/////////////////////////////////////////////////////////////////////////////////////////
// create the stream (ReadPointer set to the begin, WritePointer to the begin)
CTagStream::CTagStream(UINT uiSize /*= 0*/)
{
	m_uiStreamSize = 0;

	if (uiSize != 0)
	{
		m_pStream = malloc(uiSize);
		m_pTagReadPointer = m_pStream;
		m_pTagWritePointer = m_pStream;
		m_pStreamEnd = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pStream) + uiSize);
	}
	else
	{
		m_pStream = NULL;
		m_pTagReadPointer = NULL;
		m_pTagWritePointer = NULL;
		m_pStreamEnd = NULL;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// attach memory buffer to the stream (ReadPointer set to the begin, WritePointer to the end)
CTagStream::CTagStream(LPVOID pStream, UINT uiSize)
{
	m_uiStreamSize = uiSize;
	m_pStream = pStream;
	m_pTagReadPointer = m_pStream;
	m_pStreamEnd = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pStream) + uiSize);
	m_pTagWritePointer = m_pStreamEnd;
}
/////////////////////////////////////////////////////////////////////////////////////////
CTagStream::~CTagStream()
{
	if (m_pStream != NULL)
		free(m_pStream);
}
/////////////////////////////////////////////////////////////////////////////////////////
// return tag name (tag pointer is not changed)
USHORT CTagStream::GetTagName()
{
	if (m_pTagReadPointer && m_pTagReadPointer < m_pStreamEnd)
		return (USHORT)(&m_pTagReadPointer);
	else
		return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// return tag size (tag pointer is not changed)
ULONG CTagStream::GetTagSize()
{
	LPVOID lpTagSizePosition = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pTagReadPointer)+ sizeof(USHORT));

	if (m_pTagReadPointer && m_pTagReadPointer < m_pStreamEnd)
	{
		return reinterpret_cast<ULONG>(&lpTagSizePosition);
	}
	else
		return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// return tag vlaue & move tag pointer to the next tag
void CTagStream::GetTagValue(LPVOID pVariable)
{
	const LPVOID pData = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pTagReadPointer)
											+ sizeof(USHORT)
											+ sizeof(UINT));
	const UINT uiSize = GetTagSize();
	const UINT uiTotalTagSize = sizeof(USHORT) + sizeof(ULONG) + uiSize;

	if (m_pTagReadPointer && m_pTagReadPointer < m_pStreamEnd)
	{
		//copy data
		if (pVariable != NULL)
			memcpy(pVariable, pData, uiSize);
		//move pointer
		m_pTagReadPointer = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pTagReadPointer)	+ uiTotalTagSize);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
void CTagStream::PutTag(USHORT uTag, ULONG ulSize, LPVOID pVariable)
{
	// don't add empty tags
	if (ulSize == 0)
		return;

	UINT uiTagSize = sizeof(uTag) + sizeof(ulSize) + ulSize;
	UINT uiNewStreamSize = (m_uiStreamSize + uiTagSize);
	LPVOID lpNewSteamEnd = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pTagWritePointer) + uiTagSize);
	LPVOID lpTagSizePosition = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pTagWritePointer) + sizeof(uTag));
	LPVOID lpTagDataPosition = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pTagWritePointer) + sizeof(uTag) + sizeof(ulSize));

	if (m_pStream == NULL)
	{
		m_pStream = malloc(uiTagSize);
		m_pTagReadPointer = m_pStream;
		m_pTagWritePointer = m_pStream;
		m_pStreamEnd = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pStream) + uiTagSize);
	}
	else if(lpNewSteamEnd > m_pStreamEnd)
	{
		realloc(m_pStream, uiNewStreamSize);
		m_pStreamEnd = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(m_pStream) + uiNewStreamSize);
	}

	//save tag
	memcpy(m_pTagWritePointer, &uTag, sizeof(uTag));
	memcpy(lpTagSizePosition, &ulSize, sizeof(ulSize));
	memcpy(lpTagDataPosition, pVariable, ulSize);

	//move pointer
	m_pTagWritePointer = lpNewSteamEnd;
	m_uiStreamSize = uiNewStreamSize;
}
/////////////////////////////////////////////////////////////////////////////////////////
void CTagStream::MoveReadPointer(UINT uiIndex)
{
	UINT i;

	m_pTagReadPointer = m_pStream;
	
	for (i = 0; i< uiIndex; i++)
	{
		GetTagValue(NULL);
	}
}
