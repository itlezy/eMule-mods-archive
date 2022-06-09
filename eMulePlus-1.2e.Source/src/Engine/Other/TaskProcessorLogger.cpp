// Logger.cpp: implementation of the CTaskProcessor_Logger class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TaskProcessorLogger.h"

bool CTaskProcessor_Logger::Start()
{
	m_dwWaitTimeout = 1000;

	m_hFile = CreateFile(_T("C:\\Io.em"), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hFile)
		return false;

	DWORD dwSize = GetFileSize(m_hFile, NULL);
	SetFilePointer(m_hFile, dwSize, NULL, FILE_BEGIN);
	return true;
}

void CTaskProcessor_Logger::Stop()
{
	if (INVALID_HANDLE_VALUE != m_hFile)
	{
		VERIFY(CloseHandle(m_hFile));
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

void CTaskProcessor_Logger::Post(const COpCode& stMsg, T_CLIENT_TYPE eType, ULONG nClientID, BOOL bOut)
{
	DWORD dwSize = stMsg.GetSize(OP_TRANSPORT_UDP, stMsg.m_bSupportNewTags);
	CTask_LogOpcode* pTask = new (dwSize) CTask_LogOpcode;
	ASSERT(pTask);
	if (pTask)
	{
		pTask->m_nIdProtoType = (eType << 16) | (stMsg.GetProtocol() << 8) | stMsg.GetID();
		pTask->m_nClientID = nClientID;
		pTask->m_bOut = bOut;
		pTask->m_dwSize = dwSize;

		CStream_Mem stStream;
		stStream.m_pPtr = pTask->m_pBuf;
		stStream.m_dwSize = dwSize;
		VERIFY(stMsg.Write(stStream, OP_TRANSPORT_UDP, stMsg.m_bSupportNewTags));
		Push(pTask);
	}
}

void CTaskProcessor_Logger::Write(PCVOID pData, DWORD dwSize)
{
	DWORD dwWritten;
	VERIFY(WriteFile(m_hFile, pData, dwSize, &dwWritten, NULL));
	ASSERT(dwWritten = dwSize);
}

bool CTask_LogOpcode::Process()
{
	CTaskProcessor_Logger* pLogger = g_stEngine.m_pLoggerProcessor;
	DWORD dwValue = 0xF00FC55C; // our so-called safe value. It is written at the beginning of each record
	pLogger->Write(&dwValue, sizeof(dwValue));

	FILETIME stTime;
	GetSystemTimeAsFileTime(&stTime);
	pLogger->Write(&stTime, sizeof(stTime));
	pLogger->Write(&m_nClientID, sizeof(m_nClientID));
	pLogger->Write(&m_nIdProtoType, sizeof(m_nIdProtoType));
	pLogger->Write(&m_bOut, 1);
	pLogger->Write(&m_dwSize, sizeof(m_dwSize));
	pLogger->Write(m_pBuf, m_dwSize);
	return true;
}

void CTaskProcessor_Logger::ProcessTimeout()
{
	m_dwWaitTimeout = 1000; // perform check once per second
	// possibly we'll later add checking for file's size overflow
}
