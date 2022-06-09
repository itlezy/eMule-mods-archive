// TasksUpload.cpp: implementation of upload tasks.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OpCode.h"
#include "TasksOpcodes.h"
#include "../Files/TaskProcessorFiles.h"
#include "../../SharedFileList.h"
#include "../Data/Client.h"
#include "../Data/Prefs.h"


//////////////////////////////////////////////////////////////////////
// Send list of shared files
bool CTask_SendSharedList::Process()
{
	if(!g_stEngine.ServerState.IsConnected())
		return true;

	switch(State)
	{
	case TASK_START:
		State = TASK_READ;
		g_stEngine.Files.Push(this);
		return false;
	case TASK_READ:
		if(g_stEngine.Files.SharedFiles.m_mapSharedFiles.IsEmpty())
			return true;
		for(POSITION pos = g_stEngine.Files.SharedFiles.m_mapSharedFiles.GetStartPosition(); pos != NULL; )
		{
			CCKey		bufKey;
			CKnownFile*	pKnownFile = NULL;
			g_stEngine.Files.SharedFiles.m_mapSharedFiles.GetNextAssoc(pos, bufKey, pKnownFile);
			if(pKnownFile)
			{
				OFFEREDFILE stFile;
				md4cpy(stFile._Hash, pKnownFile->GetFileHash());

				uint32		dwClientID = 0;
				uint16		uClientPort = 0;

				if (g_stEngine.IsServerSupport(SRV_TCPFLG_COMPRESSION))
				{
					if (pKnownFile->IsPartFile())
					{
						stFile._ClientID	= 0xFCFCFCFC;
						stFile._ClientPort	= 0xFCFC;
					}
					else
					{
						// Publishing a complete files
						stFile._ClientID	= 0xFBFBFBFB;
						stFile._ClientPort	= 0xFBFB;
					}
				}
				else
				{
					//	If connected and highID, set client ID and port
					if(true) // m_pServerConnect->IsConnected() && !m_pServerConnect->IsLowID()
					{
						stFile._ClientID	= g_stEngine.GetClientID();
						stFile._ClientPort	= g_stEngine.Prefs.GetPort();
					}
				}
				stFile._FileName = pKnownFile->GetFileName();
				stFile._FileSize = pKnownFile->GetFileSize();
				stFile._FileType = pKnownFile->GetFileTypeString();

				//	There's no need to send FT_FILEFORMAT (file extension without ".") here as a server takes it from the file name
				/*if ( ((pServer != NULL) && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS)) ||
				( (pClient != NULL) && pClient->IsEmuleClient() &&
				( ((pClient->GetClientSoft() == SO_EMULE) && (pClient->GetVersion() >= FORM_CLIENT_VER(0, 42, 7))) ||
				((pClient->GetClientSoft() == SO_PLUS) && (pClient->GetVersion() > FORM_CLIENT_VER(1, 1, 0))) ) ) )
				*/
				m_Files.Add(stFile);
			}
		}
		State = TASK_SEND;
		g_stEngine.Sockets.Push(this);
		return false;
	case TASK_SEND:
		if(m_Files.GetCount() > 0)
		{
			COpCode_OFFERFILES stMsg;
			stMsg.m_bSupportNewTags = g_stEngine.IsServerSupport(SRV_TCPFLG_NEWTAGS);
			for(int i = 0; i < m_Files.GetCount(); i++)
			{
				COpCode_OFFERFILES::CStruct_Files stFile;
				md4cpy(&stFile._Hash, m_Files[i]._Hash);
				stFile._ClientAddr.Addr	= m_Files[i]._ClientID;
				stFile._ClientAddr.Port	= m_Files[i]._ClientPort;
				stFile._FileName		= m_Files[i]._FileName;
				stFile._FileSize		= m_Files[i]._FileSize;
				if(!m_Files[i]._FileType.IsEmpty())
					stFile._FileType	= m_Files[i]._FileType;
				stMsg._Files.push_back(stFile);
			}

			SOCKET hServerSocket = g_stEngine.ServerState.hSocket;
			CEmClient* pClient = g_stEngine.Sockets.Lookup(hServerSocket);
			if(pClient)
				g_stEngine.SendOpCode(hServerSocket, stMsg, pClient, QUE_NORMAL);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Send requested filename
CTask_SendRequestedFileName::CTask_SendRequestedFileName(HashType Hash, CEmClient* pClient)
{
	md4cpy(&m_Hash, &Hash);
	m_pClient = pClient;
}

//////////////////////////////////////////////////////////////////////
bool CTask_SendRequestedFileName::Process()
{
	switch(State)
	{
	case TASK_START:
		if(!g_stEngine.Files.SharedFiles.m_mapSharedFiles.IsEmpty())
		{
			CKnownFile* pFile = g_stEngine.Files.SharedFiles.GetFileByID(m_Hash.hash);
			if(pFile)
			{
				m_strFileName	= pFile->GetFileName();
				State = TASK_SEND;
				g_stEngine.Sockets.Push(this);
				return false;
			}
		}
		return true;
	case TASK_SEND:
		if(!m_strFileName.IsEmpty() && m_pClient)
		{
			COpCode_REQFILENAMEANSWER stMsg;
			md4cpy(&stMsg._Hash, &m_Hash);
			stMsg._FileName	= m_strFileName;
			g_stEngine.SendOpCode(m_pClient->m_hSocket, stMsg, m_pClient, QUE_HIGH);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Start file upload
CTask_UploadReqResult::CTask_UploadReqResult(HashType Hash, CEmClient_Peer* pClient)
	:m_pFile(NULL)
	,m_pClient(pClient)
{
	md4cpy(&m_Hash, &Hash);
}

//////////////////////////////////////////////////////////////////////
bool CTask_UploadReqResult::Process()
{
	switch(State)
	{
	case TASK_START:
		// If file does not exists, m_pFile becomes NULL
		if(!g_stEngine.Files.SharedFiles.m_mapSharedFiles.IsEmpty())
			m_pFile = g_stEngine.Files.SharedFiles.GetFileByID(m_Hash.hash);
		State = TASK_SEND;
		g_stEngine.Sockets.Push(this);
		return false;
	case TASK_SEND:
		if(m_pClient)
		{
			if(m_pFile != NULL)
			{
				// Request this file
				if(m_pClient->Mule)
					m_pClient->Mule->RequestFile(m_pFile);
			}
			else
			{
				// We don't have the file
				COpCode_FILEREQANSNOFIL stMsg;
				md4cpy(&stMsg._Hash, &m_Hash);
				g_stEngine.SendOpCode(m_pClient->m_hSocket, stMsg, m_pClient, QUE_HIGH);
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Request file block
CTask_RequestBlock::CTask_RequestBlock(HashType Hash, CEmClient_Peer* pClient, DWORD dwStart, DWORD dwEnd)
{
	md4cpy(&m_Hash, &Hash);
	m_pFile = NULL;
	m_pMule = pClient->Mule;
	m_dwStart = dwStart;
	m_dwEnd = dwEnd;
}

//////////////////////////////////////////////////////////////////////
bool CTask_RequestBlock::Process()
{
	switch(State)
	{
	case TASK_START:
		if(m_pMule && !g_stEngine.Files.SharedFiles.m_mapSharedFiles.IsEmpty())
		{
			m_pFile = g_stEngine.Files.SharedFiles.GetFileByID(m_Hash.hash);
			if(m_pFile)
			{
				State = TASK_SEND;
				g_stEngine.Sockets.Push(this);
				return false;
			}
		}
		return true;
	case TASK_SEND:
		if(m_pMule && m_pFile)
			m_pMule->RequestFileBlock(m_pFile, m_dwStart, m_dwEnd);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Send requested block
CTask_SendBlock::CTask_SendBlock(CKnownFile* pFile, CEmClient_Peer* pClient, DWORD dwStart, DWORD dwEnd)
{
	m_pFile = pFile;
	m_pClient = pClient;
	m_dwStart = dwStart;
	m_dwEnd = dwEnd;
	m_dwToGo = 0;
	m_pFileData = NULL;
}

//////////////////////////////////////////////////////////////////////
bool CTask_SendBlock::Process()
{
	switch(State)
	{
	case TASK_START:
		if(!g_stEngine.Files.SharedFiles.m_mapSharedFiles.IsEmpty())
		{
//			CKnownFile* pFile = g_stEngine.Files.SharedFiles.GetFileByID(m_Hash.hash);
			if(m_pFile && m_dwStart < m_pFile->GetFileSize())
			{
				// Check if this file was requested
				if(m_pClient->Mule == NULL || m_pClient->Mule->ReqFile != m_pFile)
					return true;

				md4cpy(&m_Hash, m_pFile->GetFileHash());

				DWORD dwPart = m_dwStart / PARTSIZE;
				DWORD dwStart = dwPart * PARTSIZE;
				DWORD dwEnd = ((m_pFile->GetFileSize() - dwStart) > PARTSIZE) ? (dwStart + PARTSIZE) : m_pFile->GetFileSize();
				//	Everyone is limited to a single chunk.
				//	Check that m_dwStartOffset and m_dwEndOffset are in the same chunk and
				//	make sure we don't pass the end of the file.
				if((dwPart != ((m_dwEnd - 1) / PARTSIZE)) || (m_dwEnd > dwEnd))
				{
					//	m_dwEnd goes into the next chunk or is beyond the file end.
					//	Set it to the end of the chunk that m_dwStartOffset is in.
					m_dwEnd = dwEnd;
				}
				//	This can't be a wrapped around request, since it has been limited to a single chunk.
				m_dwToGo = m_dwEnd - m_dwStart;
				//	Create a buffer for file data before we start to work with the file
				m_pFileData = new BYTE[m_dwToGo + 500];
				int	iRc = m_pFile->ReadFileForUpload(m_dwStart, m_dwToGo, m_pFileData);
				if (iRc >= 0)
				{
					State = TASK_SEND;
					g_stEngine.Sockets.Push(this);
					return false;
				}
				else
					delete[] m_pFileData;
			}
		}
		return true;
	case TASK_SEND:
		if(m_pClient && m_pFileData)
		{
			COpCode_SENDINGPART stMsg;
			md4cpy(&stMsg._Hash, &m_Hash);
			stMsg._StartData = m_dwStart;
			stMsg._EndData = m_dwEnd;
			stMsg._Data = new BYTE[m_dwToGo];
			CopyMemory(stMsg._Data, m_pFileData, m_dwToGo);
			g_stEngine.SendOpCode(m_pClient->m_hSocket, stMsg, m_pClient, QUE_LOW);
			delete[] m_pFileData;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Send file status
CTask_SendFileStatus::CTask_SendFileStatus(HashType Hash, CEmClient* pClient)
{
	md4cpy(&m_Hash, &Hash);
	m_pClient = pClient;
	m_pFileStatus = NULL;
	m_nStatusSize = 0;
}

//////////////////////////////////////////////////////////////////////
bool CTask_SendFileStatus::Process()
{
	switch(State)
	{
	case TASK_START:
		if(!g_stEngine.Files.SharedFiles.m_mapSharedFiles.IsEmpty())
		{
			CKnownFile* pFile = g_stEngine.Files.SharedFiles.GetFileByID(m_Hash.hash);
			if(pFile)
			{
				//				CSafeMemFile packetStream(16);
				//				((CPartFile*)pFile)->WritePartStatus(&packetStream);
				//				m_nStatusSize = packetStream.GetLength();
				//				m_pFileStatus = new BYTE[m_nStatusSize];
				//				packetStream.Write(m_pFileStatus, m_nStatusSize);

				m_nStatusSize = pFile->GetED2KPartCount();
				m_pFileStatus = new BYTE[m_nStatusSize];
				for(int i = 0; i < m_nStatusSize; i++)
					m_pFileStatus[i] = 1;

				State = TASK_SEND;
				g_stEngine.Sockets.Push(this);
				return false;
			}
		}
		return true;
	case TASK_SEND:
		if(m_pClient && m_pFileStatus)
		{
			COpCode_FILESTATUS stMsg;
			md4cpy(&stMsg._Hash, &m_Hash);
			for(int i = 0; i < m_nStatusSize; i++)
				stMsg._FileStatus.push_back(m_pFileStatus[i]);
			g_stEngine.SendOpCode(m_pClient->m_hSocket, stMsg, m_pClient, QUE_NORMAL);
			delete[] m_pFileStatus;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Send file hashsets
CTask_SendHashsets::CTask_SendHashsets(HashType Hash, CEmClient* pClient)
{
	md4cpy(&m_Hash, &Hash);
	m_pClient = pClient;
	m_pFileHashes = NULL;
	m_nHashCount = 0;
}

//////////////////////////////////////////////////////////////////////
bool CTask_SendHashsets::Process()
{
	switch(State)
	{
	case TASK_START:
		if(!g_stEngine.Files.SharedFiles.m_mapSharedFiles.IsEmpty())
		{
			CKnownFile* pFile = g_stEngine.Files.SharedFiles.GetFileByID(m_Hash.hash);
			if(pFile && pFile->GetHashSet() != NULL)
			{
				m_nHashCount = pFile->GetHashCount();
				m_pFileHashes = new HashType[m_nHashCount];
				CopyMemory(m_pFileHashes, pFile->GetHashSet(), sizeof(HashType)*m_nHashCount);

				State = TASK_SEND;
				g_stEngine.Sockets.Push(this);
				return false;
			}
		}
		return true;
	case TASK_SEND:
		if(m_pClient && m_pFileHashes)
		{
			COpCode_HASHSETANSWER stMsg;
			md4cpy(&stMsg._Hash, &m_Hash);
			for(int i = 0; i < m_nHashCount; i++)
				stMsg._Hashsets.push_back(m_pFileHashes[i]);
			g_stEngine.SendOpCode(m_pClient->m_hSocket, stMsg, m_pClient, QUE_NORMAL);
			delete[] m_pFileHashes;
		}
	}
	return true;
}
