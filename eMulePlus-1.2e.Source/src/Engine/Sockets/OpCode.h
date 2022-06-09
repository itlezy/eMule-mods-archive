// OpCode.h: interface for the COpCode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPCODE_H__E3263944_435F_44B4_A12E_EAA19AAE08AD__INCLUDED_)
#define AFX_OPCODE_H__E3263944_435F_44B4_A12E_EAA19AAE08AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TaskProcessor.h"
#include "TasksSockets.h"
#include "../../OpCodes.h"

#include "../../zlib/zlib.h"

#pragma pack(1)

#define OPCODE_SKIP_DBGDUMP

struct CEmClient;

const BYTE OP_PROT_EDONKEY		= OP_EDONKEYPROT;
const BYTE OP_PROT_EMULE		= OP_EMULEPROT;
const BYTE OP_PROT_PACKED		= OP_PACKEDPROT;
const int  OP_PROT_EDONKEY_EMULE= -1;
const int  OP_PROT_UNICODE		= -2;

const BYTE OP_TRANSPORT_TCP = 0x00;
const BYTE OP_TRANSPORT_UDP = 0x01;

struct CTag_Pure { // not a valid class.
	bool m_bValid;
	CTag_Pure() : m_bValid(false) {}
	__declspec(property(get=_IsValid)) bool Valid;
	bool _IsValid() const { return m_bValid; }
};

struct CTag_DWORD : public CTag_Pure {
	DWORD m_dwValue;
	CTag_DWORD() : m_dwValue(0) {}
	CTag_DWORD& operator = (const DWORD dwValue)
	{ 
		m_dwValue = dwValue; 
		m_bValid = true; 
		return *this; 
	}
	operator DWORD() const { return m_dwValue; }
};

struct CTag_String : public CTag_Pure {
	CString m_strValue;
	CTag_String& operator = (LPCTSTR strValue)
	{ 
		m_strValue = strValue; 
		m_bValid = true; 
		return *this;
	}
	operator CString() const { return m_strValue; }
};

struct CTagEntry {
	CTag_Pure*	m_pTag;
	DWORD		m_dwName; // either string or number
	USHORT		m_nNameLen;
	BYTE		m_nType;
};

struct CStream {

	bool m_bUnicode; // reserved for use with our new protocol.
	CStream() : m_bUnicode(false) {}

	class CException {};

	virtual void Write(PCVOID, DWORD);
	virtual DWORD ReadData(PVOID, DWORD);
	virtual DWORD GetSize() = 0;

	void Read(PVOID pBuf, DWORD dwSize)
	{
		while (dwSize)
		{
			DWORD dwProgress = ReadData(pBuf, dwSize);
			if (!dwProgress)
				Exception();

			if (!(dwSize -= dwProgress))
				break;

			((PBYTE&) pBuf) += dwProgress;
		}
	}

	void Skip(DWORD);

	virtual ~CStream();

	static void Exception();

	template <class T> void operator << (const T& stObj) { Write(&stObj, sizeof(stObj)); }
	template <class T> void operator >> (T& stObj) { Read(&stObj, sizeof(stObj)); }
	
	template <> void operator << (const CString& strTxt);
	template <> void operator >> (CString& strTxt);

	void DumpString(const CString&);
	void InitString(CString&, USHORT nLen);
	//void InitTrailingString(CString& strTxt);

	template <class T> void DumpArr(const vector<T>& arrValue, int nCounterLen, bool bSupportNewTags);
	template <class T> void InitArr(vector<T>& arrValue, int nCounterLen);

	void DumpBitArray(const vector<BYTE>& arrValue);

	void DumpTags(bool bNewTagsFormat, CTagEntry* pTags, DWORD dwCount);
	void InitTags(CTagEntry* pTags, DWORD dwCount);

private:

	struct CQuickBuf {
		BYTE m_pBuf[0x1000]; // usually enough
		PVOID m_pBufUse;
		
		CQuickBuf() : m_pBufUse(m_pBuf) {}
		~CQuickBuf() { Free(); }

		void Alloc(DWORD dwSize)
		{
			Free();
			if ((dwSize > sizeof(m_pBuf)) &&
				!(m_pBufUse = new BYTE[dwSize]))
				CStream::Exception();

		}
		void Free()
		{
			if (m_pBufUse && (m_pBuf != m_pBufUse))
				delete[] m_pBufUse;
			m_pBufUse = m_pBuf;
		}
	};
};

// does not really store info. Just counts
struct CStream_Measure : public CStream {
	DWORD m_dwSize;
	virtual void Write(PCVOID, DWORD);
	virtual DWORD GetSize(){ return m_dwSize; }

	inline CStream_Measure() : m_dwSize(0) {}
};

struct CStream_Mem : public CStream {
	PBYTE m_pPtr;
	DWORD m_dwSize;
	virtual void Write(PCVOID, DWORD);
	virtual DWORD ReadData(PVOID, DWORD);
	virtual DWORD GetSize(){ return m_dwSize; }

	inline CStream_Mem() : m_pPtr(NULL), m_dwSize(0) {}
};

struct CStream_ZLib : public CStream {
	virtual void Write(PCVOID, DWORD);
	virtual DWORD ReadData(PVOID, DWORD);
	virtual DWORD GetSize(){ return 0; }

	PBYTE		m_pBuf;
	CStream&	m_stSrc;
	CStream_Mem	m_stUnpacked;

	inline CStream_ZLib(CStream& stSrc) 
		:m_stSrc(stSrc)
		,m_pBuf(NULL)
	{ }
	~CStream_ZLib();
};

struct CStream_MemEx : public CStream {
	PBYTE m_pPtr;
	DWORD m_dwSize;
	DWORD m_dwSizeExtra;
	virtual void Write(PCVOID, DWORD);
	virtual DWORD GetSize(){ return m_dwSize; }

	inline CStream_MemEx() : m_pPtr(NULL), m_dwSize(0), m_dwSizeExtra(0) {}
};


class COpCode : public CTask_Tcp
{
protected:

	virtual void Z_Dump(CStream&, bool bSupportNewTags) const = 0;
	virtual void Z_Init(CStream&) = 0;

#ifndef OPCODE_SKIP_DBGDUMP
	struct CDbgDump {
		CString m_strTxt;
		void WriteParam(LPCTSTR szName, DWORD dwValue);
		void WriteParam(LPCTSTR szName, const CString&);
		void WriteParam(LPCTSTR szName, const AddrPort&);
		void WriteParam(LPCTSTR szName, const void* pBuf, DWORD dwCount, DWORD dwElementSize);
		void WriteParam(LPCTSTR szName, const CTag_DWORD&);
		void WriteParam(LPCTSTR szName, const CTag_String&);
		template <class T> void WriteParam(LPCTSTR szName, const vector<T>&);
	};
	virtual void Z_DbgDump(CDbgDump&) const = 0;
#endif // OPCODE_SKIP_DBGDUMP

	// CTask overridables
	virtual bool Process();
	virtual bool ProcessForClient(CEmClient_Peer* pClient) = 0;

public:

	bool m_bJustReceived;
	bool m_bSupportNewTags;
	COpCode() 
		:m_bJustReceived(true) 
		,m_bSupportNewTags(false)
		{}
	virtual ~COpCode();

	bool Write(CStream&, BYTE nTransport, bool bSupportNewTags) const;
	static COpCode* CreateRaw(BYTE nID, BYTE nProtocol, T_CLIENT_TYPE, LPTSTR strOpCode);
	static COpCode* Read(CStream&, BYTE nID, BYTE nProtocol, T_CLIENT_TYPE eType, LPTSTR strOpCode);

	virtual BYTE GetID() const = 0;
	virtual BYTE GetProtocol() const = 0;
	DWORD GetSize(BYTE nTransport, bool bSupportNewTags) const;

#ifndef OPCODE_SKIP_DBGDUMP

	int ShowSelf(bool bMessageBox = true, int nStyle = MB_ICONINFORMATION | MB_OK | MB_TOPMOST);

#endif // OPCODE_SKIP_DBGDUMP

};

/*template<class T>
class CField
{
public:
	T		m_data;

			CField<T>() {}
			CField<T>(T &data)
				: m_data(data) {}
	operator T()<T> { return(m_data); }
	operator const T()<T> const { return(m_data); }

};*/

// declare opcodes
#define PARAM_DWORD(name)	PARAM_SIMPLE_ORD(name, DWORD)
#define PARAM_USHORT(name)	PARAM_SIMPLE_ORD(name, USHORT)
#define PARAM_BYTE(name)	PARAM_SIMPLE_ORD(name, BYTE)

#define PARAM_ADDRPORT(name)	PARAM_SIMPLE(name, AddrPort)
#define PARAM_STRING(name)		PARAM_SIMPLE(name, CString)
#define PARAM_HASH(name)		PARAM_SIMPLE(name, HashType)	//PARAM_BUF(name, BYTE, 16)

#define PARAM_INTERNAL_DEFINITION
#define PARAM_SIMPLE_ORD PARAM_SIMPLE

#ifndef OPCODE_SKIP_DBGDUMP

#define BEGIN_OPCODE(id, name, prot, source) \
	const BYTE OP_CODE_##name = id; \
	class COpCode_##name : public COpCode { \
		virtual void Z_Dump(CStream&, bool bSupportNewTags) const;\
		virtual void Z_Init(CStream&);\
		virtual void Z_DbgDump(CDbgDump&) const;\
		virtual bool ProcessForClient(CEmClient_Peer* pClient);\
	public:\
		virtual BYTE GetID() const;\
		virtual BYTE GetProtocol() const;\
		COpCode_##name();\
		virtual ~COpCode_##name();\
		virtual LPCTSTR TaskName(){ return _T(#name); }\

#else // OPCODE_SKIP_DBGDUMP

#define BEGIN_OPCODE(id, name, prot, source) \
	const BYTE OP_CODE_##name = id; \
	class COpCode_##name : public COpCode { \
		virtual void Z_Dump(CStream&, bool bSupportNewTags) const;\
		virtual void Z_Init(CStream&);\
		virtual bool ProcessForClient(CEmClient_Peer* pClient);\
	public:\
		virtual BYTE GetID() const;\
		virtual BYTE GetProtocol() const;\
		COpCode_##name();\
		virtual ~COpCode_##name();\
		virtual LPCTSTR TaskName(){ return _T(#name); }\

#endif // OPCODE_SKIP_DBGDUMP

#define END_OPCODE };

#define PARAM_BYTE_JUNK(value)
#define PARAM_BUF(name, type, count) type _##name[count];
#define PARAM_DATABLOCK(name) BYTE* _##name; DWORD _Start##name; DWORD _End##name;
#define PARAM_SIMPLE(name, type) type _##name;
#define PARAM_TAGS_BEGIN
#define PARAM_TAGS_END
#define PARAM_TAG_STR(name, id) CTag_String _##name;
#define PARAM_TAG_DWORD(name, id) CTag_DWORD _##name;
//#define PARAM_TEXT_TRAILING(name) CString _##name;
#define PARAM_ARRAY(name, type, counter) vector<type> _##name;
#define PARAM_BITARRAY(name) vector<BYTE> _##name;

#ifndef OPCODE_SKIP_DBGDUMP
#define PARAM_COMPLEXARRAY_BEGIN(msg, name) struct CStruct_##name { CStruct_##name(); void Z_Dump(CStream&, bool) const; void Z_Init(CStream&); void Z_DbgDump(CDbgDump&) const;
#else
#define PARAM_COMPLEXARRAY_BEGIN(msg, name) struct CStruct_##name { CStruct_##name(); void Z_Dump(CStream&, bool) const; void Z_Init(CStream&);
#endif // OPCODE_SKIP_DBGDUMP

#define PARAM_COMPLEXARRAY_END(name) }; vector<CStruct_##name> _##name;

#include "EmMsgs.h"
#include "EmUndef.h"

#pragma pack()

#endif // !defined(AFX_OPCODE_H__E3263944_435F_44B4_A12E_EAA19AAE08AD__INCLUDED_)
