//emulEspaña - Copied from WebServer.h and then modified by MoNKi [MoNKi: -Wap Server-]
#pragma once

#include "../CxImage/xImage.h"
#include <zlib/zlib.h>
#include "WapSocket.h"
#include "Partfile.h"
#include "OtherFunctions.h"
#include "WebWapDefinitions.h"

typedef struct
{
	CString			sURL;
	in_addr			inadr;
	void			*pThis;
	CWapSocket		*pSocket;
} WapThreadData;

typedef struct
{
	COLORREF cDownColor;
	COLORREF cUpColor;
	COLORREF cConnectionsColor;
	CString sScriptFile;
	CString sMain;
	CString	sServerList;
	CString	sServerMain;
	CString	sServerLine;
	CString	sTransferDownList;
	CString	sTransferDownLineLite;
	CString	sTransferDownLineLiteGood;
	CString sTransferDownFileDetails;
	CString sTransferUpList;
	CString	sTransferUpLine;
	CString sTransferUpQueueList;
	CString	sTransferUpQueueLine;
	CString	sDownloadLink;
	CString	sSharedList;
	CString	sSharedLine;
	CString	sSharedLineChanged;
	CString	sGraphs;
	CString	sLog;
	CString	sServerInfo;
	CString sDebugLog;
	CString sStats;
	CString sPreferences;
	CString	sLogin;
	CString	sConnectedServer;
	CString	sServerOptions;
	CString	sSearchMain;
	CString	sSearchList;
	CString sSearchResultLine;
	CString sClearCompleted;
	CString sBootstrapLine;
	CString sKad;
} WapTemplates;

class CWapServer
{
	friend class CWapSocket;

public:
	CWapServer(void);
	~CWapServer(void);

	int	 UpdateSessionCount();
	void StartServer(void);
	void RestartServer();
	void AddStatsLine(UpDown line);
	void ReloadTemplates();
	UINT GetSessionCount()	{ return m_Params.Sessions.GetCount();}
	bool IsRunning()	{ return m_bServerWorking;}
protected:
	static void		ProcessURL(WapThreadData);
	static void		ProcessFileReq(WapThreadData);

private:
	static CString	_GetMain(WapThreadData, long lSession);
	static CString	_GetServerList(WapThreadData);
	static CString	_GetTransferDownList(WapThreadData);
	static CString	_GetTransferUpList(WapThreadData);
	static CString	_GetTransferQueueList(WapThreadData);
	static CString	_GetDownloadLink(WapThreadData);
	static CString	_GetSharedFilesList(WapThreadData);
	static CString	_GetGraphs(WapThreadData);
	static CString	_GetLog(WapThreadData);
	static CString	_GetServerInfo(WapThreadData);
	static CString	_GetDebugLog(WapThreadData);
	static CString	_GetStats(WapThreadData);
	static CString	_GetPreferences(WapThreadData);
	static CString	_GetLoginScreen(WapThreadData);
	static CString	_GetConnectedServer(WapThreadData);
	static CString 	_GetServerOptions(WapThreadData Data);
	static CString	_GetKadPage(WapThreadData Data);

	static void		_RemoveServer(CString sIP, int nPort);
    //static CString	_GetWebSearch(WapThreadData Data);
    static CString	_GetSearch(WapThreadData Data);

	static CString	_ParseURL(CString URL, CString fieldname); 
	static CString	_ParseURLArray(CString URL, CString fieldname);
	static void		_ConnectToServer(CString sIP, int nPort);
	static bool		_IsLoggedIn(WapThreadData Data, long lSession);
	static void		_RemoveTimeOuts(WapThreadData Data);
	static bool		_RemoveSession(WapThreadData Data, long lSession);
	static bool		_GetFileHash(CString sHash, uchar *FileHash);
	static CString	_SpecialChars(CString str);
#ifdef USE_STRING_IDS
	static CString	__GetPlainResString(RESSTRIDTYPE nID, bool noquote = false);
#define _GetPlainResString(id)			__GetPlainResString(#id, false)
#define _GetPlainResStringNoQuote(id)	__GetPlainResString(#id, true)
#else
	static CString	_GetPlainResString(RESSTRIDTYPE nID, bool noquote = false);
	static CString	_GetPlainResStringNoQuote(RESSTRIDTYPE nID) { return _GetPlainResString(nID, true); }
#endif
	static void		PlainString(CString& rstr, bool noquot);
	static void		_SetSharedFilePriority(CString hash, uint8 priority);
	static CString	_GetWebCharSet();
	CString			_LoadTemplate(CString sAll, CString sTemplateName);
	static Session	GetSessionByID(WapThreadData Data,long sessionID);
	static bool		IsSessionAdmin(WapThreadData Data,CString SsessionID);
	static CString	GetPermissionDenied();
	static CString	_GetDownloadGraph(WapThreadData Data,CString filehash);
	static void		InsertCatBox(CString &Out,int preselect,CString boxlabel, bool jump=false,bool extraCats=false);
	static CString	GetSubCatLabel(int cat);
	// Common data
	GlobalParams	m_Params;
	WapTemplates	m_Templates;
	bool			m_bServerWorking;
	int				m_iSearchSortby;
	bool			m_bSearchAsc;

	// Elandal: Moved from CUpDownClient
	static CString	GetUploadFileInfo(CUpDownClient* client);

	static void DrawLineInCxImage(CxImage *image,int x1, int y1, int x2, int y2, COLORREF color);
	static void SendGraphFile(WapThreadData Data, int file_val);
	static void SendScriptFile(WapThreadData Data);
	static void SendImageFile(WapThreadData Data, CString file);
	static void SendProgressBar(WapThreadData Data, CString filehash);
	static bool BrowserAccept(WapThreadData Data, CString sAccept);
	static bool SendFile(WapThreadData Data, LPCTSTR fileName, CString contentType);
	static void SendSmallestCxImage(WapThreadData Data, CxImage *cImage);
	static CString RemoveWMLScriptInvalidChars(CString input);

	enum ENUM_WAP_IMAGE_FORMAT {
		WITHOUT_IMAGE,
		WBMP_IMAGE,
		GIF_IMAGE,
		PNG_IMAGE
	};
};