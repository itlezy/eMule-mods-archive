// TasksOpcodes.cpp: implementation of client opcodes.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OpCode.h"
#include "TasksOpcodes.h"
#include "../Files/TaskProcessorFiles.h"
#include "../../SharedFileList.h"
#include "../../SafeFile.h"
#include "../Data/ClientList.h"
#include "../Data/Prefs.h"

OFFEREDFILE& OFFEREDFILE::operator = (const OFFEREDFILE& stObj)
{
	md4cpy(_Hash, stObj._Hash);
	_ClientID	= stObj._ClientID;
	_ClientPort	= stObj._ClientPort;
	_FileName	= stObj._FileName;
	_FileSize	= stObj._FileSize;
	_FileType	= stObj._FileType;

	return *this;
}


//////////////////////////////////////////////////////////////////////
// Client OpCodes
//////////////////////////////////////////////////////////////////////

void SendEmuleInfoAnswer(SOCKET hSocket, CEmClient_Peer* pClient)
{
	COpCode_EMULEINFOANSWER stMsg;
	stMsg._ClientVersion = CURRENT_VERSION_SHORT;
	stMsg._ProtocolVersion = EMULE_PROTOCOL;
	stMsg._Compression = 1;
	stMsg._UdpVersion = 4;
	stMsg._UdpPort = g_stEngine.Prefs.GetUDPPort();
	stMsg._SourceExchange = 0;//2;
	stMsg._Comments = 1;
	stMsg._ModPlus = CURRENT_PLUS_VERSION;
	stMsg._ExtendedRequest = 2;
	stMsg._ModVersion = _T(PLUS_VERSION_STR);
	stMsg._L2Hac = FILEREASKTIME;
	stMsg._Features = 0; // g_eMuleApp.m_pClientCreditList->CryptoAvailable() ? 3 : 0

	g_stEngine.SendOpCode(hSocket, stMsg, pClient, QUE_HIGH);
}

//////////////////////////////////////////////////////////////////////
// HELLO opcode
bool COpCode_HELLO::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	// Find client in the list of known clients
	if(pClient->Mule == NULL)
		 pClient->Mule = g_stEngine.ClientList.FindMuleClient(_UserHash);
	// If not found, create it
	if(pClient->Mule == NULL)
	{
		CClientMule* pMule = new CClientMule(_ClientAddr, _UserHash);
		if(pMule)
		{
			g_stEngine.ClientList.AddClient(pMule);
			pClient->Mule = pMule;
		}
		else
			AddLog(LOG_ERROR, _T("No memory"));
	}
	// Setting parent
	pClient->Mule->Parent = pClient;
	// Connected state
	pClient->Mule->OnConnected();

	// Update client information
	if(_UserName.Valid)		pClient->Mule->UserName = _UserName;
	if(_SoftVersion.Valid)	pClient->Mule->ClientVersion = _SoftVersion;
	if(_UserPort.Valid)		pClient->Mule->ClientPort = _UserPort;
	// to do ModVersion
	if(_UserUDPPort.Valid)	pClient->Mule->ClientUDPPort = _UserUDPPort;
	// to do MiscOptions
	if(_MuleVersion.Valid)	pClient->Mule->MuleVersion = _MuleVersion;

	// Send the answer
	CTask_SayHelloToPeer* pTask = new CTask_SayHelloToPeer();
	if(pTask)
	{
		pTask->SetClient(pClient);
		pTask->m_bManual = true;
		g_stEngine.Sockets.Push(pTask);
	}
	else
		AddLog(LOG_ERROR, _T("No memory"));

	return true;
}

//////////////////////////////////////////////////////////////////////
// HELLO answer
bool COpCode_HELLOANSWER::ProcessForClient(CEmClient_Peer*)
{
	return true;
}

//////////////////////////////////////////////////////////////////////
// eMule info
bool COpCode_EMULEINFO::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	SendEmuleInfoAnswer(m_hSocket, pClient);

	return true;
}

//////////////////////////////////////////////////////////////////////
// eMule info answer
bool COpCode_EMULEINFOANSWER::ProcessForClient(CEmClient_Peer* pClient)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Request filename
bool COpCode_REQUESTFILENAME::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	// Just send requested filename
	CTask_SendRequestedFileName* pTask = new CTask_SendRequestedFileName(_Hash, pClient);
	if(pTask)
		g_stEngine.Files.Push(pTask);
	else
		AddLog(LOG_ERROR, _T("No memory"));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Request filename answer
bool COpCode_REQFILENAMEANSWER::ProcessForClient(CEmClient_Peer* pClient)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start file upload
bool COpCode_STARTUPLOADREQ::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	// Check that we have that file, and add client to waiting queue
	CTask_UploadReqResult* pTask = new CTask_UploadReqResult(_Hash, pClient);
	if(pTask)
		g_stEngine.Files.Push(pTask);
	else
		AddLog(LOG_ERROR, _T("No memory"));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Accept upload request
bool COpCode_ACCEPTUPLOADREQ::ProcessForClient(CEmClient_Peer*)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// No such file
bool COpCode_FILEREQANSNOFIL::ProcessForClient(CEmClient_Peer*)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Request parts
bool COpCode_REQUESTPARTS::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	// Request file blocks
	// To optimize (maybe) - pass all 3 ranges in one task instead of 3 tasks
	CTask_RequestBlock* 
	pTask = new CTask_RequestBlock(_Hash, pClient, _Start1, _End1);
	if(pTask)
		g_stEngine.Files.Push(pTask);
	else
		AddLog(LOG_ERROR, _T("No memory"));
	pTask = new CTask_RequestBlock(_Hash, pClient, _Start2, _End2);
	if(pTask)
		g_stEngine.Files.Push(pTask);
	else
		AddLog(LOG_ERROR, _T("No memory"));
	pTask = new CTask_RequestBlock(_Hash, pClient, _Start3, _End3);
	if(pTask)
		g_stEngine.Files.Push(pTask);
	else
		AddLog(LOG_ERROR, _T("No memory"));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sending file part
bool COpCode_SENDINGPART::ProcessForClient(CEmClient_Peer*)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get file status
bool COpCode_SETREQFILEID::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	// Send file status, if we have it
	CTask_SendFileStatus* pTask = new CTask_SendFileStatus(_Hash, pClient);
	if(pTask)
		g_stEngine.Files.Push(pTask);
	else
		AddLog(LOG_ERROR, _T("No memory"));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File status
bool COpCode_FILESTATUS::ProcessForClient(CEmClient_Peer*)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hash request
bool COpCode_HASHSETREQUEST::ProcessForClient(CEmClient_Peer* pClient)
{
	ASSERT(pClient); // this opcode is processed only in the main context

	// Send file hashsets, if we have the file
	CTask_SendHashsets* pTask = new CTask_SendHashsets(_Hash, pClient);
	if(pTask)
		g_stEngine.Files.Push(pTask);
	else
		AddLog(LOG_ERROR, _T("No memory"));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hash request answer
bool COpCode_HASHSETANSWER::ProcessForClient(CEmClient_Peer*)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Offer files
bool COpCode_OFFERFILES::ProcessForClient(CEmClient_Peer*)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send ranking info
bool COpCode_QUEUERANKING::ProcessForClient(CEmClient_Peer*)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Cancel transfer
bool COpCode_CANCELTRANSFER::ProcessForClient(CEmClient_Peer* pClient)
{
	g_stEngine.ClientList.RemoveFromUploadQueue(pClient->Mule, ETS_CANCELED);
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// (!) Can be both we connected to him or he connected to us
// Sending HELLO/HELLOANSWER opcode.

#define FillHelloMessage() \
	md4cpy(&stMsg._UserHash, g_stEngine.Prefs.GetUserHash()); \
	stMsg._ClientAddr.Addr = g_stEngine.GetClientID(); \
	stMsg._ClientAddr.Port = g_stEngine.Prefs.GetPort(); \
	stMsg._UserName = g_stEngine.Prefs.GetUserNick(); \
	stMsg._SoftVersion = EDONKEYVERSION; \
	stMsg._UserUDPPort = g_stEngine.Prefs.GetUDPPort(); \
	stMsg._MiscOptions = \
		(4						<< 4*6) |	/* UDP version */ \
		(1						<< 4*5) |	/* Data compression version */ \
		(0/*dwSupportSecIdent*/	<< 4*4) |	/* Secure Ident */ \
		(0/*2*/					<< 4*3) |	/* Source Exchange */ \
		(2						<< 4*2) |	/* Ext. Requests */ \
		(1						<< 4*1) |	/* Comments */ \
		(1/*dwNoViewSharedFiles*/<< 1*2);	/* No View Shared files */ \
	stMsg._MuleVersion = \
		(PLUS_COMPATIBLECLIENTID << 24) | \
		((CURRENT_PLUS_VERSION & 0x7F00) << 9) | \
		((CURRENT_PLUS_VERSION & 0xF0) << 6) | \
		((CURRENT_PLUS_VERSION & 0x7) << 7);

bool CTask_SayHelloToPeer::Process()
{
	if (m_pPeer == NULL)
		return true;

	if((m_iMessage == FD_CONNECT && !m_pPeer->m_bFromOutside) || m_bManual)
	{
		if(m_bManual)
		{
			COpCode_HELLOANSWER stMsg;
			FillHelloMessage();
			g_stEngine.SendOpCode(m_pPeer->m_hSocket, stMsg, m_pPeer, QUE_HIGH);
		}
		else
		{
			COpCode_HELLO stMsg;
			FillHelloMessage();
			g_stEngine.SendOpCode(m_pPeer->m_hSocket, stMsg, m_pPeer, QUE_HIGH);
		}
		/*	if(g_stEngine.ServerState.IsConnecting() || g_stEngine.ServerState.IsConnected())
		{
		stMsg._ServerAddr._IPAddr = g_stEngine.ServerState.nAddr;
		stMsg._ServerAddr._Port = g_stEngine.ServerState.nPort;
		}
		else
		{
		stMsg._ServerAddr._IPAddr = 0;
		stMsg._ServerAddr._Port = 0;
		}*/

		if(m_pPeer->Mule && m_pPeer->Mule->MuleProtocol)
			SendEmuleInfoAnswer(m_pPeer->m_hSocket, m_pPeer);
	}
	else
	{
		ASSERT(FALSE);
	}

	return true;
}
