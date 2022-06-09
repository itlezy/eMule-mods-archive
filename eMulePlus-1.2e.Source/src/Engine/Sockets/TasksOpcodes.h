#pragma once

struct OFFEREDFILE
{
	BYTE	_Hash[16];
	ULONG	_ClientID;
	USHORT	_ClientPort;
	CString	_FileName;
	DWORD	_FileSize;
	CString	_FileType;

	OFFEREDFILE& operator = (const OFFEREDFILE& stObj);
};

//////////////////////////////////////////////////////////////////////
struct CTask_LoginToServer : public CTcpCompletionTask
{
	CTask_LoginToServer()
		: m_pServer(NULL) {}

	virtual void			SetClient(CEmClient *pClient) { m_pServer = dynamic_cast<CEmClient_Server*>(pClient); }
	virtual bool			Process();
	virtual LPCTSTR TaskName(){ return _T("LoginToServer"); }

	CEmClient_Server *m_pServer;
};
//////////////////////////////////////////////////////////////////////
struct CTask_DisconnectServer : public CTcpCompletionTask
{
	CTask_DisconnectServer()
		: m_pServer(NULL) {}

	virtual void			SetClient(CEmClient *pClient) { m_pServer = dynamic_cast<CEmClient_Server*>(pClient); }
	virtual bool			Process();
	virtual LPCTSTR TaskName(){ return _T("DisconnectServer"); }

	CEmClient_Server *m_pServer;
};
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
struct CTask_SayHelloToPeer : public CTcpCompletionTask
{
	CTask_SayHelloToPeer()
		:m_pPeer(NULL)
		,m_bManual(false)
	{ }

	virtual void	SetClient(CEmClient *pClient) { m_pPeer = dynamic_cast<CEmClient_Peer*>(pClient); }
	virtual bool	Process();
	virtual LPCTSTR TaskName(){ return _T("SayHelloToPeer"); }

	CEmClient_Peer*	m_pPeer;
	bool			m_bManual;
};
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Send list of shared files
struct CTask_SendSharedList : public CTask, CStateMachine {
	CTask_SendSharedList() { }
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("SendSharedList"); }

	CArray<OFFEREDFILE, OFFEREDFILE&> m_Files;
};

//////////////////////////////////////////////////////////////////////
// Send requested filename
struct CTask_SendRequestedFileName : public CTask, CStateMachine {
	CTask_SendRequestedFileName(HashType Hash, CEmClient* pClient);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("SendRequestedFileName"); }

	HashType	m_Hash;
	CString		m_strFileName;
	CEmClient*	m_pClient;
};

//////////////////////////////////////////////////////////////////////
// Send upload request answer
struct CTask_UploadReqResult : public CTask, CStateMachine {
	CTask_UploadReqResult(HashType Hash, CEmClient_Peer* pClient);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("UploadReqResult"); }

	HashType	m_Hash;
	CKnownFile* m_pFile;
	CEmClient_Peer*	m_pClient;
};

//////////////////////////////////////////////////////////////////////
// Request file block
struct CTask_RequestBlock : public CTask, CStateMachine {
	CTask_RequestBlock(HashType Hash, CEmClient_Peer* pClient, DWORD dwStart, DWORD dwEnd);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("RequestBlock"); }

	HashType	m_Hash;
	CKnownFile*	m_pFile;
	DWORD		m_dwStart;
	DWORD		m_dwEnd;
	CClientMule*	m_pMule;
};

//////////////////////////////////////////////////////////////////////
// Send requested block
struct CTask_SendBlock : public CTask, CStateMachine {
	CTask_SendBlock(CKnownFile* pFile, CEmClient_Peer* pClient, DWORD dwStart, DWORD dwEnd);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("SendBlock"); }

	CKnownFile*	m_pFile;
	HashType	m_Hash;
	DWORD		m_dwStart;
	DWORD		m_dwEnd;
	DWORD		m_dwToGo;
	BYTE*		m_pFileData;
	CEmClient_Peer*	m_pClient;
};

//////////////////////////////////////////////////////////////////////
// Send file status
struct CTask_SendFileStatus : public CTask, CStateMachine {
	CTask_SendFileStatus(HashType Hash, CEmClient* pClient);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("SendFileStatus"); }

	HashType	m_Hash;
	CEmClient*	m_pClient;
	BYTE*		m_pFileStatus;
	USHORT		m_nStatusSize;
};

//////////////////////////////////////////////////////////////////////
// Send hashsets
struct CTask_SendHashsets : public CTask, CStateMachine {
	CTask_SendHashsets(HashType Hash, CEmClient* pClient);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("SendHashsets"); }

	HashType	m_Hash;
	CEmClient*	m_pClient;
	HashType*	m_pFileHashes;
	USHORT		m_nHashCount;
};

