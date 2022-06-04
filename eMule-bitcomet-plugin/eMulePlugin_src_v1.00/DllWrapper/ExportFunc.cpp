#include "stdafx.h"

#include "srchybrid/types.h"
#include "common.h"

#include "Core_eMuleApp.h"
#include "plugin_version.h"

int g_nErrorCode;
extern CCore_eMuleApp theDllApp;
extern host_callback_t g_host_callback_func;
plugin_interface_t g_plugin_interface;

LPCTSTR __stdcall EM_GetErrText()
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	static TCHAR* szErrText[] = 
	{
		_T("Success."),						// 0
		_T("Unknown error!"),				// 1
		_T("EmDll load init data Failed!"), // 2
		_T("EmDll hasn't init!"),			// 3
		_T("EmDll has init already!"),		// 4
		_T("Parameter pointer is NULL!"),	// 5
		_T("Task index error!"),			// 6
		_T("Invaild Task command!"),		// 7
		_T("Task Start failed!"),			// 8
		_T("Task Stop failed!"),			// 9
		_T("Set Invaild Priority!"),		// 10
		_T("ed2k link open error!"),		// 11
		_T("Peer list index error!"),		// 12
		_T("Server list index error!"),		// 13
		_T("Task not found!"),				// 14
		_T("Task can't resume!"),			// 15
		_T("Task can't stop!"),				// 16
		_T("Task can't pause!"),			// 17
		_T("Parameter error!"),				// 18
	};
	
	int nCnt = sizeof(szErrText)/sizeof(szErrText[0]);
	if(g_nErrorCode >= nCnt)
		g_nErrorCode = 1;

	return szErrText[g_nErrorCode];

}

int __stdcall EM_GetErrIndex()
{
	return g_nErrorCode;
}

LPCTSTR __stdcall EM_GetVer()
{
	//uint16 version_mjr = 1;
	//uint16 version_min = 2;
	//uint16 version_month = 3;
	//uint16 version_day = 4;

	//uint32 ver_low  = MAKELONG(version_day, version_month);
	//uint32 ver_high = MAKELONG(version_min, version_mjr);

	//uint64 ver = (((uint64)(ver_low)) & 0xffffffff) | (((uint64)(ver_high)) << 32);

	LPCTSTR szVer = chSTR(VERSION_FIRST) _T(".") chSTR(VERSION_SECOND) _T(".") chSTR(VERSION_THIRD) _T(".") chSTR(VERSION_FOURTH);

	return szVer;
}

BOOL __stdcall EM_Init(BYTE key[])
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	// 密钥验证
	BYTE DLL_KEY[] = "\xcb\xa9\xe3\xdf\x0c\x5d\x20\x2f\x0a\x57\x32\x70\x6d\x9d\xa3\x15\x19\x29\xe5\x1c\xbf\xbd\x99\x20\x78\xa4\x14\x3b\x51\x15\x73\x16\xbf\xac\xde\x0b\x04\x10\x8b\xdb\x10";
	int nKeyLen = sizeof(DLL_KEY);
	for(int i=0;i<nKeyLen;i++)
	{
		if(key[i] != DLL_KEY[i])
		{
			g_nErrorCode = 2;
			return 0;
		}
	}

	if(CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 4;
		return 0;
	}

#ifdef VERSION_DLL
	CCore_eMuleApp::NewInstance(&theDllApp);
#else
	CCore_eMuleApp::NewInstance();
#endif

	return 1;

}

BOOL __stdcall EM_Start()
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	// 初始化内核
	int result = CCore_eMuleApp::GetInstance()->InitPlugin();
	if(result == 0)
		CCore_eMuleApp::DeleteInstance();

	return result;

}

BOOL __stdcall EM_Stop()
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	int result = CCore_eMuleApp::GetInstance()->ExitPlugin();

	return result;
}

BOOL __stdcall EM_Exit()
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

#ifdef VERSION_DLL
	// 由全局对象CCore_eMuleApp theDllApp 调用析构函数
#else
	CCore_eMuleApp::DeleteInstance();
#endif

	return 1;
}

BOOL __stdcall EM_GetTaskNum(int* Num)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetTaskNum(Num);

	return result;
}

BOOL __stdcall EM_GetTaskInfo(int nTaskIndex, Task_info_t *pData)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetTaskInfo(nTaskIndex, pData);

	return result;
}

BOOL __stdcall EM_SendTaskCmd(int nTaskIndex, task_command_t Cmd, WPARAM wParam)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	UNUSED_ALWAYS(wParam);

	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = 0;
	switch(Cmd) 
	{
	case CMD_RESUME:
		result = CCore_eMuleApp::GetInstance()->TaskResume(nTaskIndex);
		break;
	case CMD_STOP:
		result = CCore_eMuleApp::GetInstance()->TaskStop(nTaskIndex);
		break;
	case CMD_DELETE:
		result = CCore_eMuleApp::GetInstance()->TaskDelete(nTaskIndex);
		break;
	default:
		g_nErrorCode = 7;
		return 0;
	}

	return result;
}

BOOL __stdcall EM_GetGlobalStatistic(global_statistic_t *pData)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetGlobalStatistic(pData);

	return result;
}


BOOL __stdcall EM_GetServerNum(int* pCnt)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetServerNum(pCnt);

	return result;
}

BOOL __stdcall EM_GetServerInfo(int nServerIndex, Task_ServerInfo_t *pData)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetServerInfo(nServerIndex,pData);

	return result;
}

BOOL __stdcall EM_GetTaskPeerNum(int nTaskIndex, int* pCnt)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetTaskPeerNum(nTaskIndex, pCnt);

	return result;
}


BOOL __stdcall EM_GetTaskPeerInfo(int nPeerIndex, Task_PeerInfo_t *pData)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetTaskPeerInfo(nPeerIndex, pData);
	return result;
}

BOOL __stdcall EM_OpenED2KLink(ED2K_Link_Open_t &OpenInfo, ED2K_Link_Open_Ret_t &OpenRet)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	if(OpenInfo.strED2KLink == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	CString link(OpenInfo.strED2KLink);

	if(link.IsEmpty())
	{
		g_nErrorCode = 5;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->OpenED2KLink(link, OpenRet);

	return result;

}

BOOL __stdcall EM_OnPluginNotify( WPARAM wParam, LPARAM &lParam)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->OnBcClientNotify(wParam, lParam);

	return result;

}

BOOL __stdcall EM_SetDownloadPath( LPCTSTR szDownloadPath)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SetDownloadPath(szDownloadPath);

	return result;

}

BOOL __stdcall EM_GetDownloadPath( LPCTSTR *pszDownloadPath)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetDownloadPath(pszDownloadPath);

	return result;

}

BOOL __stdcall EM_SetConnectionSetting( Connection_setting_t *pInfo)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	int result = CCore_eMuleApp::GetInstance()->SetConnection(pInfo);

	return result;

}

BOOL __stdcall EM_GetConnectionSetting( Connection_setting_t *pInfo)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	int result = CCore_eMuleApp::GetInstance()->GetConnectionSetting(pInfo);

	return result;

}


BOOL __stdcall EM_SetTempPath( LPCTSTR szDownloadPath)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SetTempPath(szDownloadPath);

	return result;

}

BOOL __stdcall EM_GetTempPath( LPCTSTR *pszDownloadPath)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetDownloadPath(pszDownloadPath);
	return result;
}

int __stdcall EM_TaskHashToIndex(const uchar* filehash)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return -1;
	}

	int result = CCore_eMuleApp::GetInstance()->TaskHashToIndex(filehash);
	return result;
}


BOOL __stdcall EM_ShowEMDlg(bool bShow)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->ShowEMDlg(bShow);
	return result;
}


BOOL __stdcall EM_SetCallback(HWND hwnd, host_callback_t callback_func)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if( !CCore_eMuleApp::HasInstance() ) {
		g_nErrorCode = 3;
		return 0;
	}

	ASSERT( hwnd != NULL /*&& callback_func != NULL*/ );
	if ( hwnd == NULL /*|| callback_func == NULL*/ ) {
		g_nErrorCode = 5;
		return 0;
	}

	CCore_eMuleApp::GetInstance()->SetNotifyWindow(hwnd);
	g_host_callback_func = callback_func;

	return 1;
}

BOOL __stdcall EM_GetSharedFileInfo(int nIndex, Task_info_t *pData)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	BOOL result = CCore_eMuleApp::GetInstance()->GetSharedFileInfo(nIndex,pData);
	return result;
}

BOOL __stdcall EM_GetSharedFileNum(int* pNum)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	ASSERT(pNum != NULL);
	if (pNum == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetSharedFileNum(pNum);
	return result;
}

int __stdcall EM_SharedFileHashToIndex(const uchar* filehash)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return -1;
	}

	int result = CCore_eMuleApp::GetInstance()->SharedFileHashToIndex(filehash);
	return result;
}

BOOL __stdcall EM_SimpleClientOpen(ED2K_Link_Open_t &OpenInfo, ED2K_Link_Open_Ret_t &OpenRet)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	if(OpenInfo.strED2KLink == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	//CString link(OpenInfo.strED2KLink);

	/*
	if(link.IsEmpty())
	{
	g_nErrorCode = 5;
	return 0;
	}
	*/

	//MessageBox(NULL, _T("123"), NULL, MB_OK);
	BOOL result = CCore_eMuleApp::GetInstance()->SimpleClientOpen(OpenInfo, OpenRet);
	return result;

}

BOOL __stdcall EM_SimpleClientGetFinishBlock(const uint32 client_id, uint64* pOffset, uint32* pLength)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SimpleClientGetFinishBlock(client_id, pOffset, pLength);
	return result;
}

BOOL __stdcall EM_SimpleClientReadData(const uint32 client_id, char* pBuffer, 	const uint64 offset, const uint32 length)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SimpleClientReadData(client_id, pBuffer, offset, length);
	return result;
}

BOOL __stdcall EM_SimpleClientClose(const uint32 client_id)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SimpleClientClose(client_id);
	return result;
}

BOOL __stdcall EM_SimpleClientGetLastError(const uint32 client_id, uint64* pOffset, uint32* pLength)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );
#endif

	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = 0; //CCore_eMuleApp::GetInstance()->SimpleClientRemove((const uchar*)hash);
	return result;
}

BOOL __stdcall EM_SimpleClientReadHashset(uint32 client_id, char* pBuffer, uint32 buf_size, uint32& read_cnt)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SimpleClientReadHashset(client_id, pBuffer, buf_size, read_cnt);
	return result;
}

BOOL __stdcall EM_SimpleClientGetStatus(uint32 client_id, Simple_Task_Status_t& status)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SimpleClientGetStatus(client_id, status);
	return result;
}

BOOL __stdcall EM_SimpleClientGetImportBlock(const uint32 client_id, uint64* pOffset, uint32* pLength)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SimpleClientGetImportBlock(client_id, pOffset, pLength);
	return result;
}

BOOL __stdcall EM_SimpleClientImportExistFile(const uint32 client_id, char* pBuffer, const uint64 offset, const  uint32 length)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SimpleClientImportExistFile(client_id, pBuffer, offset, length);
	return result;
}

BOOL __stdcall EM_SearchFileStart(LPCTSTR filename, uint64 file_size)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SearchFileStart(filename, file_size);
	return result;
}

BOOL __stdcall EM_SearchFileStop()
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->SearchFileStop();
	return result;
}

BOOL __stdcall EM_ConnectServer()
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->ConnectServer();
	return result;
}

BOOL __stdcall EM_GetFileSearchResultInfo(FileSearchResultInfo_t& info)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetFileSearchResultInfo(info);
	return result;
}

BOOL __stdcall EM_GetFileSearchResultItem(int nItemIndex, FileSearchResultItem_t& item)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif
	// 此处为普通函数体
	if(!CCore_eMuleApp::HasInstance())
	{
		g_nErrorCode = 3;
		return 0;
	}

	BOOL result = CCore_eMuleApp::GetInstance()->GetFileSearchResultItem(nItemIndex, item);
	return result;
}


#ifdef VERSION_DLL
extern "C" __declspec(dllexport) 
#endif
BOOL __stdcall EM_GetPluginInterface(plugin_interface_t& plugin_interface)
{
#ifdef VERSION_DLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

#define ASSIGN_PLUGIN_INTERFACE(func)	\
	g_plugin_interface.##func = func;

	ASSIGN_PLUGIN_INTERFACE(EM_Init);
	ASSIGN_PLUGIN_INTERFACE(EM_Exit)
	ASSIGN_PLUGIN_INTERFACE(EM_Start)
	ASSIGN_PLUGIN_INTERFACE(EM_Stop)
	ASSIGN_PLUGIN_INTERFACE(EM_GetVer)
	ASSIGN_PLUGIN_INTERFACE(EM_GetErrText)
	ASSIGN_PLUGIN_INTERFACE(EM_GetErrIndex)
	ASSIGN_PLUGIN_INTERFACE(EM_GetTaskNum)
	ASSIGN_PLUGIN_INTERFACE(EM_GetTaskInfo)
	ASSIGN_PLUGIN_INTERFACE(EM_SendTaskCmd)
	ASSIGN_PLUGIN_INTERFACE(EM_GetGlobalStatistic)
	ASSIGN_PLUGIN_INTERFACE(EM_GetServerNum)
	ASSIGN_PLUGIN_INTERFACE(EM_GetServerInfo)
	ASSIGN_PLUGIN_INTERFACE(EM_GetTaskPeerNum)
	ASSIGN_PLUGIN_INTERFACE(EM_GetTaskPeerInfo)
	ASSIGN_PLUGIN_INTERFACE(EM_OpenED2KLink)
	ASSIGN_PLUGIN_INTERFACE(EM_OnPluginNotify)
	ASSIGN_PLUGIN_INTERFACE(EM_SetConnectionSetting)
	ASSIGN_PLUGIN_INTERFACE(EM_GetConnectionSetting)
	ASSIGN_PLUGIN_INTERFACE(EM_SetDownloadPath)
	ASSIGN_PLUGIN_INTERFACE(EM_GetDownloadPath)
	ASSIGN_PLUGIN_INTERFACE(EM_SetTempPath)
	ASSIGN_PLUGIN_INTERFACE(EM_GetTempPath)
	ASSIGN_PLUGIN_INTERFACE(EM_TaskHashToIndex)
	ASSIGN_PLUGIN_INTERFACE(EM_ShowEMDlg)
	ASSIGN_PLUGIN_INTERFACE(EM_SetCallback)
	ASSIGN_PLUGIN_INTERFACE(EM_GetSharedFileInfo)
	ASSIGN_PLUGIN_INTERFACE(EM_GetSharedFileNum)
	ASSIGN_PLUGIN_INTERFACE(EM_SharedFileHashToIndex)
	ASSIGN_PLUGIN_INTERFACE(EM_SimpleClientOpen)
	ASSIGN_PLUGIN_INTERFACE(EM_SimpleClientGetFinishBlock)
	ASSIGN_PLUGIN_INTERFACE(EM_SimpleClientReadData)
	ASSIGN_PLUGIN_INTERFACE(EM_SimpleClientClose)
	ASSIGN_PLUGIN_INTERFACE(EM_SimpleClientGetLastError)
	ASSIGN_PLUGIN_INTERFACE(EM_SimpleClientReadHashset)
	ASSIGN_PLUGIN_INTERFACE(EM_SimpleClientGetStatus)
	ASSIGN_PLUGIN_INTERFACE(EM_SimpleClientGetImportBlock)
	ASSIGN_PLUGIN_INTERFACE(EM_SimpleClientImportExistFile)
	ASSIGN_PLUGIN_INTERFACE(EM_SearchFileStart)
	ASSIGN_PLUGIN_INTERFACE(EM_SearchFileStop)
	ASSIGN_PLUGIN_INTERFACE(EM_ConnectServer)
	ASSIGN_PLUGIN_INTERFACE(EM_GetFileSearchResultInfo)
	ASSIGN_PLUGIN_INTERFACE(EM_GetFileSearchResultItem)

	plugin_interface = g_plugin_interface;
	return 1;

#undef ASSIGN_PLUGIN_INTERFACE
}

