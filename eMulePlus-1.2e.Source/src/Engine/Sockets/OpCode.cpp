// OpCode.cpp: implementation of the COpCode class.
//
//////////////////////////////////////////////////////////////////////

#pragma pack(1)

#include "stdafx.h"
#include "../../packets.h"
#include "../Other/TaskProcessorLogger.h"
#include "OpCode.h"
#include "../Data/Prefs.h"

#pragma pack(1)

//////////////////////////////////////////////////////////////////////
template <> void CStream::operator << (const CString& strTxt)
{
	*this << (USHORT) strTxt.GetLength();
	DumpString(strTxt);
}

//////////////////////////////////////////////////////////////////////
void CStream::DumpString(const CString& strTxt)
{
	USHORT nLen = (USHORT) strTxt.GetLength();

#ifdef UNICODE
	if (m_bUnicode)
		Write((LPCWSTR) strTxt, nLen * sizeof(WCHAR));
	else
	{
		// convert it to ANSI string
		CQuickBuf stBuf;
		stBuf.Alloc(nLen);
		if (nLen != WideCharToMultiByte(AP_ACP, 0, strTxt, nLen, (PSTR) stBuf.m_pBufUse, nLen, NULL, NULL))
			Exception();

		Write(stBuf.m_pBufUse, nLen);
	}

#else // UNICODE

	if (m_bUnicode)
	{
		// convert it to unicode
		CQuickBuf stBuf;
		stBuf.Alloc(nLen * sizeof(WCHAR));
		if (nLen != MultiByteToWideChar(CP_ACP, 0, strTxt, nLen, (PWSTR) stBuf.m_pBufUse, nLen))
			Exception();

		Write(stBuf.m_pBufUse, nLen * sizeof(WCHAR));
	} else
		Write((LPCSTR) strTxt, nLen);

#endif // UNICODE
}

//////////////////////////////////////////////////////////////////////
template <> void CStream::operator >> (CString& strTxt)
{
	USHORT nLen;
	*this >> nLen;
	InitString(strTxt, nLen);
}

//////////////////////////////////////////////////////////////////////
//void CStream::InitTrailingString(CString& strTxt) {
//	DWORD dwSize = SizeLeft();
//	if (m_bUnicode)
//		dwSize >>= 1; // /= 2
//	if (dwSize > 0xFFFF)
//		Exception();
//	InitString(strTxt, (USHORT) SizeLeft());
//}

//////////////////////////////////////////////////////////////////////
void CStream::InitString(CString& strTxt, USHORT nLen)
{
	strTxt.Empty();
	PVOID pStr = strTxt.GetBuffer(nLen);

#ifdef UNICODE

	if (m_bUnicode)
		Read(pStr, nLen * sizeof(WCHAR));
	} else
	{
		// convert it to unicode
		CQuickBuf stBuf;
		stBuf.Alloc(nLen);
		Read(stBuf.m_pBufUse, nLen);

		if (nLen != MultiByteToWideChar(CP_ACP, 0, (PCSTR) stBuf.m_pBufUse, nLen, (PWSTR) pStr, nLen))
			Exception();
	}
#else // UNICODE
	if (m_bUnicode)
	{
		// convert it to ansi
		CQuickBuf stBuf;
		stBuf.Alloc(nLen * sizeof(WCHAR));
		Read(stBuf.m_pBufUse, nLen * sizeof(WCHAR));

		if (nLen != WideCharToMultiByte(CP_ACP, 0, (PCWSTR) stBuf.m_pBufUse, nLen, (PSTR) pStr, nLen, NULL, NULL))
			Exception();
	} else
		Read(pStr, nLen);
#endif // UNICODE

	strTxt.ReleaseBuffer(nLen);
}

//////////////////////////////////////////////////////////////////////
void CStream::DumpTags(bool bNewTagsFormat, CTagEntry* pTags, DWORD dwCount)
{
	// count how many tags we have
	DWORD dwTagsCount = 0;
	for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
		if (pTags[dwIndex].m_pTag->Valid)
			dwTagsCount++;

	*this << dwTagsCount;

	// dump tags
	for (; dwCount--; pTags++)
		if (pTags->m_pTag->Valid)
		{
/*			*this << pTags->m_nType;

			// put this tag's id (or name)
			*this << pTags->m_nNameLen;
			if (1 == pTags->m_nNameLen)
				*this << (BYTE) pTags->m_dwName;
			else
				Write((PVOID) pTags->m_dwName, pTags->m_nNameLen);
*/			
			// tag's value
			switch (pTags->m_nType)
			{
			case TAGTYPE_UINT32:
				{
					DWORD dwValue = ((CTag_DWORD*)pTags->m_pTag)->m_dwValue;
					if(bNewTagsFormat)
					{
						if(dwValue <= 0xFF)
						{
							*this << (BYTE)(TAGTYPE_UINT8 | 0x80);
							*this << (BYTE)(pTags->m_dwName);
							*this << (BYTE)(dwValue);
						}
						else if(dwValue <= 0xFFFF)
						{
							*this << (BYTE)(TAGTYPE_UINT16 | 0x80);
							*this << (BYTE)(pTags->m_dwName);
							*this << (USHORT)(dwValue);
						}
						else
						{
							*this << (BYTE)(TAGTYPE_UINT32 | 0x80);
							*this << (BYTE)(pTags->m_dwName);
							*this << (DWORD)(dwValue);
						}
					}
					else
					{
						*this << (BYTE)(TAGTYPE_UINT32);
						*this << (USHORT)(pTags->m_nNameLen);
						*this << (BYTE)(pTags->m_dwName);
						*this << (DWORD)(dwValue);
					}
				}
				break;

			case TAGTYPE_STRING:
				{
					CString sValue = ((CTag_String*) pTags->m_pTag)->m_strValue;
					USHORT uLength = sValue.GetLength();
					if(bNewTagsFormat)
					{
						if(uLength >= 1 && uLength <= 16)
						{
							*this << (BYTE)((TAGTYPE_STR1 + uLength - 1) | 0x80);
							*this << (BYTE)(pTags->m_dwName);
						}
						else
						{
							*this << (BYTE)(TAGTYPE_STRING | 0x80);
							*this << (BYTE)(pTags->m_dwName);
							*this << (USHORT)(uLength);
						}
						Write((PVOID)sValue.GetBuffer(0), uLength);
					}
					else
					{
						*this << (BYTE)(TAGTYPE_STRING);
						*this << (USHORT)(pTags->m_nNameLen);
						*this << (BYTE)(pTags->m_dwName);
						*this << sValue;
					}
				}
				break;

			default:
				Exception();
			}

		}
}

template <class T> void StreamDumpElement(CStream& stStream, const T& stElement, bool bSupportNewTags)
{
	stStream << stElement;
}
template <class T> void StreamInitElement(CStream& stStream, T& stElement)
{
	stStream >> stElement;
}

//////////////////////////////////////////////////////////////////////
void CStream::DumpBitArray(const vector<BYTE>& arrValue)
{
	int nSize = arrValue.size();
	Write(&nSize, sizeof(USHORT));
	uint16 done = 0;
	while (done != nSize)
	{
		BYTE towrite = 0;
		for(int i = 0; i < 8; i++)
		{
			if(arrValue[done])
				towrite |= (1 << i);

			if (++done == nSize)
				break;
		}
		*this << towrite;
	}
}

//////////////////////////////////////////////////////////////////////
template <class T>
void CStream::DumpArr(const vector<T>& arrValue, int nCounterLen, bool bSupportNewTags)
{
	int nSize = arrValue.size();
	ASSERT(nCounterLen <= sizeof(nSize));
	if((sizeof(nCounterLen) == 1 && nSize > 255) || (sizeof(nCounterLen) == 2 && nSize > 65535))
		Exception(); // too long

	Write(&nSize, nCounterLen);
	for (int nItem = 0; nItem < nSize; nItem++)
		StreamDumpElement(*this, arrValue[nItem], bSupportNewTags);
}


//////////////////////////////////////////////////////////////////////
template <class T>
void CStream::InitArr(vector<T>& arrValue, int nCounterLen)
{
	int nSize;
	ASSERT(nCounterLen <= sizeof(nSize));
	Read(&nSize, nCounterLen);

//	arrValue.SetSize(nSize);
	arrValue.resize(nSize);
	for (int nItem = 0; nItem < nSize; nItem++)
		StreamInitElement(*this, arrValue[nItem]);
}

//////////////////////////////////////////////////////////////////////
void CStream::InitTags(CTagEntry* pTags, DWORD dwCount)
{
	DWORD dwTagsCount;
	*this >> dwTagsCount;

	while (dwTagsCount--)
	{
		BYTE nType;
		*this >> nType;

		USHORT nLen;
		if (nType & 0x80)
		{
			nType &= 0x7F;
			nLen = 1;
		}
		else
			*this >> nLen;

		BYTE nTagID;
		CQuickBuf stTagName;

		if (1 == nLen)
			*this >> nTagID;
		else
		{
			stTagName.Alloc(nLen);
			Read(stTagName.m_pBufUse, nLen);
		}

		// check if we have such a tag id (or name)
		for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
			if ((pTags[dwIndex].m_nNameLen == nLen) &&
				((1 == nLen) ? (nTagID == (BYTE) pTags[dwIndex].m_dwName) : !memcmp(stTagName.m_pBufUse, (PVOID) pTags[dwIndex].m_dwName, nLen)))
				break; // match

		CTagEntry* pTarget = (dwIndex < dwCount) ? pTags + dwIndex : NULL;

//		if (pTarget && pTarget->m_nType != nType && pTarget->m_nType != TAGTYPE_STRING)
//			Exception(); // type mismatch

		switch (nType)
		{
		case TAGTYPE_UINT32:
		{
			DWORD dwVal;
			*this >> dwVal;
			if (pTarget && pTarget->m_nType == TAGTYPE_UINT32)
				((CTag_DWORD*) pTarget->m_pTag)->m_dwValue = dwVal;
			break;
		}
		case TAGTYPE_UINT16:
		{
			USHORT nVal;
			*this >> nVal;
			if (pTarget && pTarget->m_nType == TAGTYPE_UINT32)
				((CTag_DWORD*) pTarget->m_pTag)->m_dwValue = nVal;
			break;
		}
		case TAGTYPE_UINT8:
		{
			BYTE byteVal;
			*this >> byteVal;
			if (pTarget && pTarget->m_nType == TAGTYPE_UINT32)
				((CTag_DWORD*) pTarget->m_pTag)->m_dwValue = byteVal;
			break;
		}
		case TAGTYPE_FLOAT32:
		{
			float fVal;
			*this >> fVal;
			break;
		}
		case TAGTYPE_HASH:
			break;
		case TAGTYPE_BOOL:
		{
			bool bVal;
			*this >> bVal;
			if (pTarget && pTarget->m_nType == TAGTYPE_UINT32)
				((CTag_DWORD*) pTarget->m_pTag)->m_dwValue = bVal;
			break;
		}
		case TAGTYPE_BOOLARRAY:
		{
			USHORT nLen;
			*this >> nLen;
			CQuickBuf stBoolArray;
			stBoolArray.Alloc(nLen);
			Read(stBoolArray.m_pBufUse, nLen);
			break;
		}
		case TAGTYPE_BLOB:
		{
			DWORD dwLen;
			*this >> dwLen;
			CQuickBuf stBlob;
			stBlob.Alloc(dwLen);
			Read(stBlob.m_pBufUse, dwLen);
			break;
		}
		case TAGTYPE_STRING:
		{
			if (pTarget)
				*this >> ((CTag_String*) pTarget->m_pTag)->m_strValue;
			else
			{
				CString strVal;
				*this >> strVal; // This is a bit heave. Better just to skip this string
			}
			break;
		}
		default:
			if (nType >= TAGTYPE_STR1 && nType <= TAGTYPE_STR16)
			{
				USHORT nLen = nType - TAGTYPE_STR1 + 1;
				CQuickBuf stStr;
				stStr.Alloc(nLen);
				Read(stStr.m_pBufUse, nLen);
				BYTE* pBuf = (BYTE*)stStr.m_pBufUse;
				pBuf[nLen] = NULL;
				if (pTarget)
					((CTag_String*) pTarget->m_pTag)->m_strValue = (LPCTSTR)pBuf;
				nType = TAGTYPE_STRING;
				break;
			}
			Exception();
		}

		if (pTarget)
			pTarget->m_pTag->m_bValid = true;
	}
}

//////////////////////////////////////////////////////////////////////
void CStream::Write(PCVOID, DWORD)
{
	Exception(); // not implemented
}

//////////////////////////////////////////////////////////////////////
DWORD CStream::ReadData(PVOID, DWORD)
{
	return 0; // not implemented
}

//////////////////////////////////////////////////////////////////////
CStream::~CStream()
{
}

//////////////////////////////////////////////////////////////////////
void CStream::Skip(DWORD dwSize)
{
	BYTE pBuf[0x1000];
	while (dwSize)
	{
		DWORD dwPortion = min(dwSize, sizeof(pBuf));
		Read(pBuf, dwPortion);
		dwSize -= dwPortion;
	}
}

//////////////////////////////////////////////////////////////////////
void CStream::Exception()
{
	CException stException;
	throw stException;
}

//////////////////////////////////////////////////////////////////////
void CStream_Measure::Write(PCVOID, DWORD dwSize)
{
	m_dwSize += dwSize;
}

//////////////////////////////////////////////////////////////////////
void CStream_Mem::Write(PCVOID pBuf, DWORD dwSize)
{
	if (dwSize > m_dwSize)
		Exception();
	CopyMemory(m_pPtr, pBuf, dwSize);
	m_pPtr += dwSize;
	m_dwSize -= dwSize;
}

//////////////////////////////////////////////////////////////////////
DWORD CStream_Mem::ReadData(PVOID pBuf, DWORD dwSize)
{
	if (dwSize > m_dwSize)
	{
		dwSize = m_dwSize;
		m_dwSize = 0;
	} else
		m_dwSize -= dwSize;

	CopyMemory(pBuf, m_pPtr, dwSize);
	m_pPtr += dwSize;
	return dwSize;
}

//////////////////////////////////////////////////////////////////////
void CStream_MemEx::Write(PCVOID pBuf, DWORD dwSize)
{
	if (dwSize > m_dwSize)
	{
		m_dwSizeExtra += dwSize;
		if (m_dwSize)
		{
			m_dwSizeExtra -= m_dwSize;
			m_dwSize = 0;
		}

	} else
	{
		CopyMemory(m_pPtr, pBuf, dwSize);
		m_pPtr += dwSize;
		m_dwSize -= dwSize;
	}
}

//////////////////////////////////////////////////////////////////////
void CStream_ZLib::Write(PCVOID pBuf, DWORD dwSize)
{
	Exception(); // not yet.
}

//////////////////////////////////////////////////////////////////////
DWORD CStream_ZLib::ReadData(PVOID pBuf, DWORD dwSize)
{
	if(m_pBuf == NULL)
	{
		DWORD dwPackedSize = m_stSrc.GetSize();
		if(dwPackedSize == 0)
			throw;	// we already tried to unpack and failed
		DWORD dwMaxUnpackedSize = dwPackedSize * 10 + 300;
		m_pBuf = new BYTE[dwMaxUnpackedSize];

		BYTE* pbytePackedBuffer = new BYTE[dwPackedSize];
		m_stSrc.ReadData(pbytePackedBuffer, dwPackedSize);

		uLongf dwUnpackedSize = dwMaxUnpackedSize;
		uint16 nResult = uncompress(m_pBuf, &dwUnpackedSize, pbytePackedBuffer, dwPackedSize);

		if (nResult == Z_OK)
		{
			m_stUnpacked.m_pPtr = (PBYTE)m_pBuf;
			m_stUnpacked.m_dwSize = dwUnpackedSize;
		}
		else
		{
			delete[] m_pBuf;
			m_pBuf = NULL;
			throw;
		}
	}
	return m_stUnpacked.ReadData(pBuf, dwSize);
}

CStream_ZLib::~CStream_ZLib()
{
	if(m_pBuf)
		delete[] m_pBuf;
}

//////////////////////////////////////////////////////////////////////
COpCode::~COpCode()
{
}

//////////////////////////////////////////////////////////////////////
bool COpCode::Write(CStream& stStream, BYTE nTransport, bool bSupportNewTags) const
{
	try
	{
		BYTE nProtocol = GetProtocol();
		stStream.m_bUnicode = (OP_PROT_UNICODE == nProtocol);
		stStream << nProtocol;

		switch (nTransport)
		{
		case OP_TRANSPORT_TCP:
			{
				// write the body size
				CStream_Measure stMeasure;
				Z_Dump(stMeasure, bSupportNewTags);

				stStream << stMeasure.m_dwSize + 1;
			}
			break;
		case OP_TRANSPORT_UDP:
			break;
		default:
			stStream.Exception(); // undefined transport format
		}

		stStream << GetID();

		// dump the rest of the opcode
		Z_Dump(stStream, bSupportNewTags);

	}
	catch (CStream::CException)
	{
		return false;
	}
	return true; // ok
}

//////////////////////////////////////////////////////////////////////
COpCode *COpCode::CreateRaw(BYTE nID, BYTE nProtocol, T_CLIENT_TYPE eType, LPTSTR strOpCode)
{
	// Some messages with OP_PROT_EMULE protocol are received in a compressed way, and their
	// protocol ID is OP_PROT_PACKED. But this doesn't matter in this function.
	if (OP_PROT_PACKED == nProtocol)
		nProtocol = OP_PROT_EMULE;

//	switch ((eType << 16) | (nProtocol << 8) | nID)
	{
//		#define BEGIN_OPCODE(id, name, prot, source)	case (T_CLIENT_##source << 16) | (OP_PROT_##prot << 8) | id: _tcscpy(strOpCode, _T(#name)); return new COpCode_##name;
		#define BEGIN_OPCODE(id, name, prot, source) \
			if(nID == id && \
			(nProtocol == OP_PROT_##prot || (OP_PROT_##prot == OP_PROT_EDONKEY_EMULE && (nProtocol == OP_PROT_EMULE || nProtocol == OP_PROT_EDONKEY))) && \
			(eType == T_CLIENT_##source || (T_CLIENT_##source == T_CLIENT_PEER_SERVER && (eType == T_CLIENT_PEER || eType == T_CLIENT_SERVER)))) \
			{ \
				_tcscpy(strOpCode, _T(#name)); \
				return new COpCode_##name; \
			} \

		#define END_OPCODE
		#define PARAM_BYTE_JUNK(value)
		#define PARAM_BUF(name, type, count)
		#define PARAM_DATABLOCK(name)
		#define PARAM_SIMPLE(name, type)
		#define PARAM_SIMPLE_ORD PARAM_SIMPLE
		#define PARAM_TAGS_BEGIN
		#define PARAM_TAGS_END
		#define PARAM_TAG_STR(name, id)
		#define PARAM_TAG_DWORD(name, id)
		//#define PARAM_TEXT_TRAILING(name)
		#define PARAM_ARRAY(name, type, counter)
		#define PARAM_BITARRAY(name)
		#define PARAM_COMPLEXARRAY_BEGIN(msg, name)
		#define PARAM_COMPLEXARRAY_END(name)

		#include "EmMsgs.h"
		#include "EmUndef.h"
	}
	_tcscpy(strOpCode, _T("Unknown"));
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
COpCode *COpCode::Read(CStream &stStream, BYTE nID, BYTE nProtocol, T_CLIENT_TYPE eType, LPTSTR strOpCode)
{
	COpCode* pNewOpCode = CreateRaw(nID, nProtocol, eType, strOpCode);

	if (pNewOpCode != NULL)
	{
		try
		{
			switch (nProtocol)
			{
			case OP_PROT_PACKED:
				{
					CStream_ZLib stZStream(stStream);
					pNewOpCode->Z_Init(stZStream);
				}
				break;
			case OP_PROT_UNICODE:
				stStream.m_bUnicode = true;
				// no break;
			default:
				pNewOpCode->Z_Init(stStream);
			}
		}
		catch (CStream::CException)
		{
			delete pNewOpCode;
			pNewOpCode = NULL;
		}
	}

	return pNewOpCode;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD COpCode::GetSize(BYTE nTransport, bool bSupportNewTags) const
{
	CStream_Measure stStream;
	return Write(stStream, nTransport, bSupportNewTags) ? stStream.m_dwSize : 0;
}

//////////////////////////////////////////////////////////////////////
bool COpCode::Process()
{
	CEmClient* pClient = NULL;
	if (g_stEngine.m_pSocketsProcessor->IsInContext(*this))
	{
		pClient = g_stEngine.Sockets.Lookup(m_hSocket);
		if (!pClient)
			return true; // no more relevant!

		if (m_bJustReceived)
		{
			m_bJustReceived = false;
			// write it to log
			if(g_stEngine.Prefs.SaveLogsIO)
				g_stEngine.m_pLoggerProcessor->Post(*this, pClient->GetType(), pClient->m_nClientID, FALSE);
		}
	}

	AddLog(LOG_DEBUG, _T("Processing opcode %s (%x), (socket=%u)"), TaskName(), GetID(), pClient ? pClient->m_hSocket : 0);
#if defined(_DEBUG) && !defined(OPCODE_SKIP_DBGDUMP)
	ShowSelf(false);
#endif //_DEBUG && OPCODE_SKIP_DBGDUMP

	if(pClient->GetType() != T_CLIENT_PEER && pClient->GetType() != T_CLIENT_SERVER)
		return false;

	return ProcessForClient(reinterpret_cast<CEmClient_Peer*>(pClient));
}

//////////////////////////////////////////////////////////////////////
// Debug stuff
#ifndef OPCODE_SKIP_DBGDUMP

void COpCode::CDbgDump::WriteParam(LPCTSTR szName, DWORD dwValue)
{
	CString strDelta;
	strDelta.Format(_T("\r\n%s = %u"), szName, dwValue);
	m_strTxt += strDelta;
}
void COpCode::CDbgDump::WriteParam(LPCTSTR szName, const CString& strValue)
{
	CString strDelta;
	strDelta.Format(_T("\r\n%s = %s"), szName, strValue);
	m_strTxt += strDelta;
}
void COpCode::CDbgDump::WriteParam(LPCTSTR szName, const AddrPort& stValue)
{
	CString strDelta;
	strDelta.Format(_T("\r\n%s = %u.%u.%u.%u:%u"), szName,
		((in_addr&)stValue._IPAddr).S_un.S_un_b.s_b1,
		((in_addr&)stValue._IPAddr).S_un.S_un_b.s_b2,
		((in_addr&)stValue._IPAddr).S_un.S_un_b.s_b3,
		((in_addr&)stValue._IPAddr).S_un.S_un_b.s_b4,
		stValue._Port);

	m_strTxt += strDelta;
}
void COpCode::CDbgDump::WriteParam(LPCTSTR szName, const void*, DWORD dwCount, DWORD)
{
	CString strDelta;
	strDelta.Format(_T("\r\n%s = [Count=%u]"), szName, dwCount);
	m_strTxt += strDelta;
}
void COpCode::CDbgDump::WriteParam(LPCTSTR szName, const CTag_DWORD& stValue)
{
	CString strDelta;
	strDelta.Format(_T("\r\n%s = [%c] %u"), szName, stValue.m_bValid ? _T('+') : _T('-'), stValue.m_dwValue);
	m_strTxt += strDelta;
}
void COpCode::CDbgDump::WriteParam(LPCTSTR szName, const CTag_String& stValue)
{
	CString strDelta;
	strDelta.Format(_T("\r\n%s = [%c] %s"), szName, stValue.m_bValid ? _T('+') : _T('-'), stValue.m_strValue);
	m_strTxt += strDelta;
}
template <class T>
void COpCode::CDbgDump::WriteParam(LPCTSTR szName, const vector<T>& arrValue)
{
	CString strDelta;
	strDelta.Format(_T("\r\n%s = [Count=%u]"), szName, arrValue.size());
	m_strTxt += strDelta;
}

int COpCode::ShowSelf(bool bMessageBox /* = true */, int nStyle /* = MB_ICONINFORMATION | MB_OK | MB_TOPMOST */)
{
	CDbgDump stDump;
	stDump.m_strTxt.Format(_T("Opcode ID=%u, Ptr=%08X"), GetID(), this);
	Z_DbgDump(stDump);

	if(bMessageBox)
		return ::MessageBox(NULL, stDump.m_strTxt, _T("Opcode parameters"), nStyle);

	AddLog(LOG_DEBUG, "%s", stDump.m_strTxt);

	return 0;
}
#endif // OPCODE_SKIP_DBGDUMP


//////////////////////////////////////////////////////////////////////
// All OpCodes
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructors

#define BEGIN_OPCODE(id, name, prot, source)	COpCode_##name::COpCode_##name() {
#define END_OPCODE						}
#define PARAM_BYTE_JUNK(value)
#define PARAM_BUF(name, type, count)	ZeroMemory(_##name, count * sizeof(type));
#define PARAM_DATABLOCK(name)			_##name = NULL; _Start##name = 0; _End##name = 0;
#define PARAM_SIMPLE(name, type)
#define PARAM_SIMPLE_ORD(name, type)	_##name = 0;
#define PARAM_TAGS_BEGIN
#define PARAM_TAGS_END
#define PARAM_TAG_STR(name, id)
#define PARAM_TAG_DWORD(name, id)
//#define PARAM_TEXT_TRAILING(name)
#define PARAM_ARRAY(name, type, counter)
#define PARAM_BITARRAY(name)
#define PARAM_COMPLEXARRAY_BEGIN(msg, name) } COpCode_##msg::CStruct_##name::CStruct_##name() {
#define PARAM_COMPLEXARRAY_END(name)

#include "EmMsgs.h"
#include "EmUndef.h"

//////////////////////////////////////////////////////////////////////
// Destructors


#define BEGIN_OPCODE(id, name, prot, source)	COpCode_##name::~COpCode_##name() {
#define END_OPCODE						}
#define PARAM_BYTE_JUNK(value)
#define PARAM_BUF(name, type, count)
#define PARAM_DATABLOCK(name)		if(_##name){ delete[] _##name; _##name = NULL; }
#define PARAM_SIMPLE(name, type)
#define PARAM_SIMPLE_ORD PARAM_SIMPLE
#define PARAM_TAGS_BEGIN
#define PARAM_TAGS_END
#define PARAM_TAG_STR(name, id)
#define PARAM_TAG_DWORD(name, id)
//#define PARAM_TEXT_TRAILING(name)
#define PARAM_ARRAY(name, type, counter)
#define PARAM_BITARRAY(name)
#define PARAM_COMPLEXARRAY_BEGIN(msg, name)
#define PARAM_COMPLEXARRAY_END(name)

#include "EmMsgs.h"
#include "EmUndef.h"

//////////////////////////////////////////////////////////////////////
// GetID()

#define BEGIN_OPCODE(id, name, prot, source)	BYTE COpCode_##name::GetID() const { return id;
#define END_OPCODE						}
#define PARAM_BYTE_JUNK(value)
#define PARAM_BUF(name, type, count)
#define PARAM_DATABLOCK(name)
#define PARAM_SIMPLE(name, type)
#define PARAM_SIMPLE_ORD PARAM_SIMPLE
#define PARAM_TAGS_BEGIN
#define PARAM_TAGS_END
#define PARAM_TAG_STR(name, id)
#define PARAM_TAG_DWORD(name, id)
//#define PARAM_TEXT_TRAILING(name)
#define PARAM_ARRAY(name, type, counter)
#define PARAM_BITARRAY(name)
#define PARAM_COMPLEXARRAY_BEGIN(msg, name)
#define PARAM_COMPLEXARRAY_END(name)

#include "EmMsgs.h"
#include "EmUndef.h"

//////////////////////////////////////////////////////////////////////
// GetProtocol()

#define BEGIN_OPCODE(id, name, prot, source)	BYTE COpCode_##name::GetProtocol() const { return OP_PROT_##prot;
#define END_OPCODE						}
#define PARAM_BYTE_JUNK(value)
#define PARAM_BUF(name, type, count)
#define PARAM_DATABLOCK(name)
#define PARAM_SIMPLE(name, type)
#define PARAM_SIMPLE_ORD PARAM_SIMPLE
#define PARAM_TAGS_BEGIN
#define PARAM_TAGS_END
#define PARAM_TAG_STR(name, id)
#define PARAM_TAG_DWORD(name, id)
//#define PARAM_TEXT_TRAILING(name)
#define PARAM_ARRAY(name, type, counter)
#define PARAM_BITARRAY(name)
#define PARAM_COMPLEXARRAY_BEGIN(msg, name)
#define PARAM_COMPLEXARRAY_END(name)

#include "EmMsgs.h"
#include "EmUndef.h"

//////////////////////////////////////////////////////////////////////
// Z_Dump

#define BEGIN_OPCODE(id, name, prot, source)	void COpCode_##name::Z_Dump(CStream& stStream, bool bSupportNewTags) const {
#define END_OPCODE						}
#define PARAM_BYTE_JUNK(value)			stStream << (BYTE) value;
#define PARAM_BUF(name, type, count)	stStream.Write(_##name, count * sizeof(type));
#define PARAM_DATABLOCK(name)			stStream << _Start##name; stStream << _End##name; stStream.Write(_##name, (_End##name - _Start##name));
#define PARAM_SIMPLE(name, type)		stStream << _##name;
#define PARAM_SIMPLE_ORD PARAM_SIMPLE
#define PARAM_TAGS_BEGIN				CTagEntry pTagsInfo[] = { 
#define PARAM_TAGS_END					}; stStream.DumpTags(bSupportNewTags, pTagsInfo, _countof(pTagsInfo));
#define PARAM_TAG_STR(name, id)			(CTag_Pure*) &_##name, id, 1, TAGTYPE_STRING,
#define PARAM_TAG_DWORD(name, id)		(CTag_Pure*) &_##name, id, 1, TAGTYPE_UINT32,
//#define PARAM_TEXT_TRAILING(name)		stStream.DumpString(_##name);
#define PARAM_ARRAY(name, type, counter) stStream.DumpArr(_##name, sizeof(counter), bSupportNewTags);
#define PARAM_BITARRAY(name)			stStream.DumpBitArray(_##name);
#define PARAM_COMPLEXARRAY_BEGIN(msg, name) stStream.DumpArr(_##name, sizeof(DWORD), bSupportNewTags); } template <> void StreamDumpElement(CStream& stStream, const COpCode_##msg::CStruct_##name& stElement, bool bSupportNewTags) { stElement.Z_Dump(stStream, bSupportNewTags); } void COpCode_##msg::CStruct_##name::Z_Dump(CStream& stStream, bool bSupportNewTags) const {
#define PARAM_COMPLEXARRAY_END(name)

#include "EmMsgs.h"
#include "EmUndef.h"

//////////////////////////////////////////////////////////////////////
// Z_Init

#define BEGIN_OPCODE(id, name, prot, source)	void COpCode_##name::Z_Init(CStream& stStream) {
#define END_OPCODE						}
#define PARAM_BYTE_JUNK(value)			BYTE nJunk; stStream >> nJunk; if (value != nJunk) stStream.Exception();
#define PARAM_BUF(name, type, count)	stStream.Read(_##name, count * sizeof(type));
#define PARAM_DATABLOCK(name)			stStream >> _Start##name; stStream >> _End##name; stStream.Read(_##name, (_End##name - _Start##name));
#define PARAM_SIMPLE(name, type)		stStream >> _##name;
#define PARAM_SIMPLE_ORD PARAM_SIMPLE
#define PARAM_TAGS_BEGIN				CTagEntry pTagsInfo[] = { 
#define PARAM_TAGS_END					}; stStream.InitTags(pTagsInfo, _countof(pTagsInfo));
#define PARAM_TAG_STR(name, id)			(CTag_Pure*) &_##name, id, 1, TAGTYPE_STRING,
#define PARAM_TAG_DWORD(name, id)		(CTag_Pure*) &_##name, id, 1, TAGTYPE_UINT32,
//#define PARAM_TEXT_TRAILING(name)		stStream.InitTrailingString(_##name);
#define PARAM_ARRAY(name, type, counter) stStream.InitArr(_##name, sizeof(counter));
#define PARAM_BITARRAY(name)			stStream.InitArr(_##name, sizeof(USHORT));
#define PARAM_COMPLEXARRAY_BEGIN(msg, name) stStream.InitArr(_##name, sizeof(DWORD)); } template <> void StreamInitElement(CStream& stStream, COpCode_##msg::CStruct_##name& stElement) { stElement.Z_Init(stStream); } void COpCode_##msg::CStruct_##name::Z_Init(CStream& stStream) {
#define PARAM_COMPLEXARRAY_END(name)

#include "EmMsgs.h"
#include "EmUndef.h"

//////////////////////////////////////////////////////////////////////
// Z_DbgDump

#ifndef OPCODE_SKIP_DBGDUMP

#define BEGIN_OPCODE(id, name, prot, source)	void COpCode_##name::Z_DbgDump(CDbgDump& stDump) const { stDump.m_strTxt.Format("%s (%u)  Ptr=%08X  Protocol=%u  Size=%u\n", #name, id, this, OP_PROT_##prot, GetSize(OP_TRANSPORT_UDP));
#define END_OPCODE						}
#define PARAM_BYTE_JUNK(value)
#define PARAM_BUF(name, type, count)	stDump.WriteParam(#name, _##name, count, sizeof(type));
#define PARAM_DATABLOCK(name)
#define PARAM_SIMPLE(name, type)		stDump.WriteParam(#name, _##name);
#define PARAM_SIMPLE_ORD PARAM_SIMPLE
#define PARAM_TAGS_BEGIN
#define PARAM_TAGS_END
#define PARAM_TAG_STR(name, id)			stDump.WriteParam(#name, _##name);
#define PARAM_TAG_DWORD(name, id)		stDump.WriteParam(#name, _##name);
//#define PARAM_TEXT_TRAILING(name)		stDump.WriteParam(#name, _##name);
#define PARAM_ARRAY(name, type, counter) stDump.WriteParam(#name, _##name);
#define PARAM_BITARRAY(name)			stDump.WriteParam(#name, _##name);
#define PARAM_COMPLEXARRAY_BEGIN(msg, name) stDump.WriteParam(#name, _##name); } void COpCode_##msg::CStruct_##name::Z_DbgDump(CDbgDump& stDump) const {
#define PARAM_COMPLEXARRAY_END(name)
//#define PARAM_COMPLEXARRAY_BEGIN(msg, name) stStream.InitArr(_##name, sizeof(DWORD)); } template <> void StreamInitElement(CStream& stStream, COpCode_##msg::CStruct_##name& stElement) { stElement.Z_Init(stStream); } void COpCode_##msg::CStruct_##name::Z_Init(CStream& stStream) {
//#define PARAM_COMPLEXARRAY_END(name)

#include "EmMsgs.h"
#include "EmUndef.h"

#endif // OPCODE_SKIP_DBGDUMP

#pragma pack()
