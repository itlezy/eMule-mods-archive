// Logger.h: interface for the CTaskProcessor_Logger class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../../Engine/TaskProcessor.h"
#include "../../Engine/Sockets/OpCode.h"

struct CTask_LogOpcode : public CTask {
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("LogOpCode"); }
	ULONG m_nIdProtoType;
	ULONG m_nClientID;
	BOOL m_bOut;
	DWORD m_dwSize;
	BYTE m_pBuf[0]; // variable-sized

	inline void* operator new(size_t nSize, DWORD dwSizeExtra) { return malloc(nSize + dwSizeExtra); }
	inline void operator delete(void* pPtr) { free(pPtr); }
};

class CTaskProcessor_Logger : public CTaskProcessor
{
	virtual bool Start();
	virtual void Stop();
	virtual void ProcessTimeout();

	HANDLE m_hFile;

public:
	CTaskProcessor_Logger() :
	  m_hFile(INVALID_HANDLE_VALUE)
	  {}

	void Post(const COpCode&, T_CLIENT_TYPE eType, ULONG nClientID, BOOL bOut);
	void Write(PCVOID, DWORD); // must be callied by CTask_LogOpcode only.
};

