// Core_BitCometCE.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "Core_eMuleApp.h"

#include "SimpleClientTaskManager.h"
#include "EmuleTaskSimple.h"
#include "EmuleTaskSimpleRange.h"
#include "EmuleTaskCache.h"
#include "ExceptionDebugger/InterfaceExceptionDebugger.h"

#include <locale.h>
#include <io.h>
#include <share.h>
#include <Mmsystem.h>

#include "srchybrid/emule.h"
#include "srchybrid/opcodes.h"
#include "srchybrid/mdump.h"
#include "srchybrid/Scheduler.h"
#include "srchybrid/SearchList.h"
#include "srchybrid/kademlia/kademlia/Kademlia.h"
#include "srchybrid/kademlia/kademlia/Prefs.h"
#include "srchybrid/kademlia/kademlia/Error.h"
#include "srchybrid/kademlia/utils/UInt128.h"
#include "srchybrid/PerfLog.h"
#include <..\src\mfc\sockimpl.h>
#include <..\src\mfc\afximpl.h>
//#include "srchybrid/LastCommonRouteFinder.h"
#include "srchybrid/UploadBandwidthThrottler.h"
#include "srchybrid/ClientList.h"
#include "srchybrid/FriendList.h"
#include "srchybrid/ClientUDPSocket.h"
#include "srchybrid/DownloadQueue.h"
#include "srchybrid/IPFilter.h"
#include "srchybrid/MMServer.h"
#include "srchybrid/Statistics.h"
#include "srchybrid/OtherFunctions.h"
#include "srchybrid/WebServer.h"
#include "srchybrid/UploadQueue.h"
#include "srchybrid/SharedFileList.h"
#include "srchybrid/ServerList.h"
#include "srchybrid/Sockets.h"
#include "srchybrid/ListenSocket.h"
#include "srchybrid/ClientCredits.h"
#include "srchybrid/KnownFileList.h"
#include "srchybrid/Server.h"
#include "srchybrid/UpDownClient.h"
#include "srchybrid/ED2KLink.h"
#include "srchybrid/Preferences.h"
#include "srchybrid/secrunasuser.h"
#include "srchybrid/SafeFile.h"
#include "srchybrid/PeerCacheFinder.h"
#include "srchybrid/emuleDlg.h"
#include "srchybrid/SearchDlg.h"
#include "srchybrid/enbitmap.h"
#include "srchybrid/FirewallOpener.h"
#include "srchybrid/StringConversion.h"
#include "srchybrid/Log.h"
#include "srchybrid/Collection.h"
#include "srchybrid/LangIDs.h"
#include "srchybrid/HelpIDs.h"
#include "srchybrid/TransferWnd.h"
#include "srchybrid/SearchParams.h"
#include "srchybrid/SearchResultsWnd.h"
#include "srchybrid/SearchParamsWnd.h"
#include "srchybrid/MenuCmds.h"
#include "srchybrid/UserMsgs.h"

#include "srchybrid/types.h"
#include "common.h"

#include "BuildNumber.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BC_IMPORTED_CAT _T("_bitcomet_imported")

extern int g_nErrorCode;
static int s_last_simple_id = 0;
host_callback_t g_host_callback_func;

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//


// CCore_eMuleApp

#ifdef VERSION_DLL
BOOL CCore_eMuleApp::InitInstance()
{
	CWinApp::InitInstance();

	__if_exists(Core_Common::InterfaceExceptionDebugger)
	{
		Core_Common::InterfaceExceptionDebugger::set_program_id( _T("em") VERSIONSTRING_BUILD );
		bool is_chinese = (PRIMARYLANGID(GetSystemDefaultLangID()) == LANG_CHINESE);
		Core_Common::InterfaceExceptionDebugger::set_program_name( is_chinese ? _T("电驴插件") : _T("eMule Plugin") );
		Core_Common::InterfaceExceptionDebugger::set_unhandled_exception_filter();
	}

	return TRUE;
}

BEGIN_MESSAGE_MAP(CCore_eMuleApp, CWinApp)
END_MESSAGE_MAP()

CCore_eMuleApp theDllApp;
#endif


UINT __cdecl PluginTimer_ThreadProc( LPVOID pParam )
{
	ASSERT(pParam);
	if(pParam == NULL)
		AfxMessageBox(_T("[Fatal Error] Can't get main window handle!"));

	HWND hMainWnd = (HWND) pParam;

	HANDLE SignalsL[1] = {
		CCore_eMuleApp::GetInstance()->m_hStopThreadEvent,
	};

	int nProcessStartTick = GetTickCount();
	int nTimerInterval = 100; // ms

	//	ResetEvent(SignalsL[0]); // comment out to avoid lock up
	for(;;)
	{
		int nProcessEndTick = GetTickCount();
		int nTimeOutInterval = nTimerInterval - (nProcessEndTick-nProcessStartTick);
		nTimeOutInterval = max(nTimeOutInterval,1);
		int dwEvent = ::WaitForMultipleObjects( 1, SignalsL, FALSE, nTimeOutInterval );
		if ( dwEvent - WAIT_OBJECT_0 == 0 )
		{
			break;
		}
		else if ( dwEvent == WAIT_TIMEOUT )
		{
			nProcessStartTick = GetTickCount();
			//theApp.Timer(); // call this in the main thread
			if ( hMainWnd != NULL )
			{
				uint32 client_id = 0;
				notify_id_enum notify_id = EMULE_NOTIFY_PLUGIN_TIMER;
				BOOL ok = PostMessage(hMainWnd, WM_EMULE_PLUGIN_NOTIFY, (WPARAM)client_id, (LPARAM)notify_id);
				UNUSED_ALWAYS(ok);
			}
		}
	}

	return 0;   // thread completed successfully
};


// CCore_eMuleApp 构造
CCore_eMuleApp::CCore_eMuleApp()
	: m_hMainWnd(NULL)
	, m_dwLastSearchID(0)
{
	// 将所有重要的初始化放置在 InitInstance 中
	m_hStopThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	CSimpleClientTaskManager::NewInstance(new CSimpleClientTaskManager);
}

CCore_eMuleApp::~CCore_eMuleApp()
{
	if ( CSimpleClientTaskManager::HasInstance() )
		CSimpleClientTaskManager::DeleteInstance();
}

BOOL CCore_eMuleApp::OnBcClientNotify( WPARAM wParam, LPARAM &lParam)
{
	notify_id_enum notify_id = (notify_id_enum) wParam;
	UNUSED_ALWAYS(lParam);

	if ( notify_id == EMULE_NOTIFY_PLUGIN_TIMER )
	{
		Timer(); // call this in the main thread
	}
	else
	{
		// do nothing
	}

	return 1;
}

int CCore_eMuleApp::Timer()
{
	//HDC dc = ::GetDC(NULL);
	//CString str;
	//str.Format(_T("%d"),GetTickCount());
	//::TextOut(dc,10,10,str,str.GetLength());
	
	using namespace std;

	// for test
	int a = 0;
	if(a == 1)
	{
		if ( CSimpleClientTaskManager::HasInstance())
		{
			const list<CEmuleTaskSimple*>& task_list = CSimpleClientTaskManager::GetInstance()->get_task_list();
			for(list<CEmuleTaskSimple*>::const_iterator iter = task_list.begin(); iter != task_list.end(); iter++)
			{
				CEmuleTaskSimple* task = *iter;
				
				string uncompleted_range_dump = task->dump_uncompleted_range();
				string completed_range_dump = task->dump_completed_range();
				
				string block_list_dump = task->dump_block_list();
			}
		}
	}


	int cur_time = GetTickCount();

	// second timer
	static int timer_sec = 0;
	if(cur_time - timer_sec >= 1000)
	{
		timer_sec = cur_time;

		// process new added tasks
		for(uint32 i = 0; i < m_new_added_task.size(); i++)
		{
			uint32 task_id = m_new_added_task[i];
			CEmuleTaskSimple* new_task = CSimpleClientTaskManager::GetInstance()->get_task_by_id(task_id);
			if(new_task)
			{
				// 将创建任务前该文件已下载的数据导入 BitComet
				if(new_task->m_file_exist_already)
				{
					new_task->prepare_export_exist_file_to_host();
					host_grant_import_exist_file(task_id);
				}

				// load part status of new added tasks from bitcomet
				new_task->update_pieces_status_from_host();
			}
		}
		m_new_added_task.clear();
	}

	// 10 second timer
	static int timer_sec_10 = 0;
	if(cur_time - timer_sec_10 >= 10 * 1000)
	{
		timer_sec_10 = cur_time;

		// update piece status from host for every tasks
		if(CSimpleClientTaskManager::HasInstance())
		{
			CSimpleClientTaskManager::GetInstance()->update_pieces_status_from_host();
		}
	}

	// miniue timer
	static int timer_min = 0;
	if(cur_time - timer_min >= 60 * 1000)
	{
		timer_min = cur_time;
	}

	return 1;
}

BOOL CCore_eMuleApp::InitPlugin(/*HWND main_wnd*/)
{
	// 初始化 OLE 库
	static BOOL bOleInitOK = 0;

	free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("preferences.ini"));

	// 初始化EmuleApp对象
	theAppPtr = new CemuleApp(this);
	theAppPtr->InitEmuleLib();

	return CWinApp::InitInstance();
}

void CCore_eMuleApp::wrapper_on_tasklist_loaded(void)
{
	// 检查并创建 _bitcomet_imported 分类
	get_bc_imported_category();

	// 清除上次运行创建的 _bitcomet_imported 分类的文件
	for(int i = 0; i < theAppPtr->downloadqueue->GetFileCount(); )
	{
		bool find = false;
		CPartFile* file = theAppPtr->downloadqueue->GetFileByIndex(i);
		if(file)
		{
			UINT cat_id = file->GetCategory();
			if(thePrefs.GetCategory(cat_id))
			{
				CString cat_title = thePrefs.GetCategory(cat_id)->strTitle;
				if( cat_title == BC_IMPORTED_CAT)
				{
					if( !CSimpleClientTaskManager::GetInstance()->is_in_task_list(file->GetFileHash()) )
					{
						find = true;
					}
				}
			}
		}
		if(find)
		{
			file->DeleteFile();
		}
		else
		{
			i++;
		}
	}
}

BOOL CCore_eMuleApp::ExitPlugin(void)
{
	// kill timer thread
	SetEvent(m_hStopThreadEvent);

	if (theAppPtr != NULL)
	{
		CemuleDlg* pdlg = (CemuleDlg*)theAppPtr->m_pMainWnd;
		if(!pdlg->m_closed)  // 若对话框已经关闭了，则 m_closed 应已经被设置为true
		{
			pdlg->CloseWnd();
		}

		// 销毁对象
		theAppPtr->ExitEmulePlugin();
		delete theAppPtr;
		theAppPtr = NULL;
	}

	return 1;
}

BOOL CCore_eMuleApp::GetTaskNum(int *nNum)
{
	if(nNum == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	if (theAppPtr->downloadqueue == NULL)
		return 0;

	*nNum = theAppPtr->downloadqueue->GetFileCount();
	return 1;
}

BOOL CCore_eMuleApp::GetTaskInfo( int nIndex, Task_info_t *pDataOut )
{
	if( pDataOut == NULL )
	{
		g_nErrorCode = 5;
		return 0;
	}

	if (theAppPtr->downloadqueue == NULL)
		return 0;

	CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByIndex(nIndex);
	if ( NULL == part_file_ptr )
	{
		g_nErrorCode = 6;
		return 0;
	}

	m_task_info.Task_state							= (task_state_enum)part_file_ptr->GetStatus();

	if ( m_task_info.Task_show_name != NULL )
	{
		delete m_task_info.Task_show_name;
		m_task_info.Task_show_name = NULL;
	}
	m_task_info.Task_show_name						= _tcsdup( part_file_ptr->GetFileName() );
	ASSERT( m_task_info.Task_show_name != NULL );

	m_task_info.Task_complete_permillage			= (int)(part_file_ptr->GetPercentCompleted() * 10);
	m_task_info.Task_hashcheck_permillage			= 0;

	m_task_info.Task_download_rate					= part_file_ptr->GetDownloadDatarate10();
	m_task_info.Task_time_left						= (int)part_file_ptr->getTimeRemaining();
	m_task_info.Task_num_available_source			= (int)part_file_ptr->GetSourceCount();
	m_task_info.Task_num_total_source				= (int)part_file_ptr->GetNotCurrentSourcesCount();
	m_task_info.Task_num_transferring_source		= (int)part_file_ptr->GetTransferringSrcCount();

	m_task_info.Task_size_total						= (__int64)part_file_ptr->GetFileSize();
	m_task_info.Task_size_left						= (__int64)(part_file_ptr->GetFileSize() - part_file_ptr->GetCompletedSize());
	m_task_info.Task_size_finished					= (__int64)part_file_ptr->GetCompletedSize();
	m_task_info.Task_size_transferred				= (__int64)part_file_ptr->GetTransferred();
	m_task_info.Task_num_parts						= part_file_ptr->GetPartCount();

	if ( m_task_info.Task_save_folder != NULL )
	{
		delete m_task_info.Task_save_folder;
		m_task_info.Task_save_folder = NULL;
	}
	m_task_info.Task_save_folder 					= _tcsdup( part_file_ptr->GetPath() );
	ASSERT( m_task_info.Task_save_folder != NULL );

	if ( m_task_info.Task_save_name != NULL )
	{
		delete m_task_info.Task_save_name;
		m_task_info.Task_save_name	= NULL;
	}
	m_task_info.Task_save_name						= _tcsdup( part_file_ptr->GetFileName());	
	ASSERT( m_task_info.Task_save_name );

	// export data
	pDataOut->Task_state							= m_task_info.Task_state;
	pDataOut->Task_show_name						= m_task_info.Task_show_name;
	pDataOut->Task_complete_permillage				= m_task_info.Task_complete_permillage;
	pDataOut->Task_hashcheck_permillage				= m_task_info.Task_hashcheck_permillage;

	pDataOut->Task_download_rate					= m_task_info.Task_download_rate;
	pDataOut->Task_time_left						= m_task_info.Task_time_left;
	pDataOut->Task_num_available_source				= m_task_info.Task_num_available_source;
	pDataOut->Task_num_total_source					= m_task_info.Task_num_total_source;
	pDataOut->Task_num_transferring_source			= m_task_info.Task_num_transferring_source;

	pDataOut->Task_size_total						= m_task_info.Task_size_total;
	pDataOut->Task_size_left						= m_task_info.Task_size_left;
	pDataOut->Task_size_finished					= m_task_info.Task_size_finished;
	pDataOut->Task_size_transferred					= m_task_info.Task_size_transferred;
	pDataOut->Task_num_parts						= m_task_info.Task_num_parts;

	pDataOut->Task_save_folder						= m_task_info.Task_save_folder;
	pDataOut->Task_save_name						= m_task_info.Task_save_name;

	return 1;
}

BOOL CCore_eMuleApp::GetGlobalStatistic(global_statistic_t *pDataOut)
{
	if(pDataOut == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	ZeroMemory(pDataOut, sizeof(global_statistic_t));

	// export data
	pDataOut->g_count_task				= 0;
	if(theAppPtr != NULL && theAppPtr->downloadqueue != NULL )
	{
		pDataOut->g_count_task = theAppPtr->downloadqueue->GetFileCount();
	}
	pDataOut->g_count_shared_file       = 0;
	if(theAppPtr != NULL && theAppPtr->sharedfiles != NULL )
	{
		pDataOut->g_count_shared_file = theAppPtr->sharedfiles->GetCount();
	}
	pDataOut->g_download_rate_kb	    = 0;
	pDataOut->g_upload_rate_kb		    = 0;

	pDataOut->g_is_tasklist_loaded = false;
	if(theAppPtr != NULL && theAppPtr->emuledlg != NULL )
	{
		pDataOut->g_is_tasklist_loaded = (theAppPtr->emuledlg->status > 4);
	}

	pDataOut->g_is_server_connected = false;
	pDataOut->g_is_server_connecting = false;
	if(theAppPtr != NULL && theAppPtr->serverconnect != NULL )
	{
		pDataOut->g_is_server_connected = theAppPtr->serverconnect->IsConnected();
		pDataOut->g_is_server_connecting = theAppPtr->serverconnect->IsConnecting();
	}

	m_global_statistic.g_lan_ip.Empty();
	pDataOut->g_lan_ip                  = m_global_statistic.g_lan_ip;

	m_global_statistic.g_wan_ip.Empty();
	pDataOut->g_wan_ip                  = m_global_statistic.g_wan_ip;

	return 1;
}


BOOL CCore_eMuleApp::GetServerNum(int *pCnt)
{
	if(pCnt == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	if (theAppPtr->serverlist == NULL)
		return 0;

	*pCnt = (int)theAppPtr->serverlist->GetServerCount();
	// 本函数预先取出所有server信息，存于m_listTracker，供GetServerInfo()调用
	m_listServer.clear();
	for (int i = 0; i < *pCnt; i++)
	{
		CServer* server_ptr = theAppPtr->serverlist->GetServerAt(i);
		if (server_ptr == NULL)
			break;

		CCore_eMuleApp::Task_ServerInfo_t_internal server_info;
		server_info.str_name			= server_ptr->GetListName();
		server_info.str_description		= server_ptr->GetDescription();
		server_info.str_version			= server_ptr->GetVersion();
		server_info.ip					= server_ptr->GetIP();
		server_info.port				= server_ptr->GetPort();
		server_info.failedcount			= server_ptr->GetFailedCount();
		server_info.files				= server_ptr->GetFiles();
		server_info.hardfiles			= server_ptr->GetHardFiles();
		server_info.softfiles			= server_ptr->GetSoftFiles();
		server_info.maxusers			= server_ptr->GetMaxUsers();
		server_info.ping				= server_ptr->GetPing();
		server_info.staticservermember	= server_ptr->IsStaticMember();
		server_info.low_id_users		= server_ptr->GetLowIDUsers();

		m_listServer.push_back(server_info);
	}
	return 1;
}

BOOL CCore_eMuleApp::GetServerInfo(int nIndex,  Task_ServerInfo_t *pDataOut)
// 本函数在 GetServerCnt() 之后调用，由 GetServerCnt() 函数实际读取数据
{
	if(pDataOut == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	//int nCnt = theAppPtr->m_listServer.size();
	int nCnt = m_listServer.size();

	if(nIndex >= nCnt || nIndex < 0 )
	{
		g_nErrorCode = 13;
		return 0;
	}

	Task_ServerInfo_t *pData = reinterpret_cast<Task_ServerInfo_t*>(&(m_listServer[nIndex]));

	// export data
	/*pDataOut->tracker_url           = pData->tracker_url;
	pDataOut->num_connect_retries   = pData->num_connect_retries;   
	pDataOut->next_req_time_left    = pData->next_req_time_left;    
	pDataOut->tracker_log           = pData->tracker_log;*/

	pDataOut->failedcount			= pData->failedcount;
	pDataOut->files					= pData->files;
	pDataOut->hardfiles				= pData->hardfiles;
	pDataOut->ip					= pData->ip;
	pDataOut->low_id_users			= pData->low_id_users;
	pDataOut->maxusers				= pData->maxusers;
	pDataOut->ping					= pData->ping;
	pDataOut->port					= pData->port;
	pDataOut->softfiles				= pData->softfiles;
	pDataOut->staticservermember	= pData->staticservermember;
	pDataOut->str_description		= pData->str_description;
	pDataOut->str_name				= pData->str_name;
	pDataOut->str_version			= pData->str_version;
	pDataOut->users					= pData->users;

	return 1;
}

BOOL CCore_eMuleApp::GetTaskPeerNum(int nIndex,  int *pCnt)
{
	if(pCnt == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	if (theAppPtr->serverlist == NULL)
		return 0;

	CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByIndex(nIndex);
	if (part_file_ptr == NULL)
	{
		g_nErrorCode = 6;
		return 0;
	}

	*pCnt = part_file_ptr->GetSourceCount();

	// 本函数预先取出所有server信息，存于m_listPeer，供GetTaskPeerInfo()调用
	m_listPeer.clear();
	Task_PeerInfo_t_internal task_peerinfo_internal;
	for (POSITION pos = part_file_ptr->srclist.GetHeadPosition(); pos != NULL; )
	{ 
		CUpDownClient* cur_src						= part_file_ptr->srclist.GetNext(pos);
		task_peerinfo_internal.ip					= cur_src->GetIP();
		task_peerinfo_internal.port					= cur_src->GetUserPort();

		task_peerinfo_internal.peer_source			= (source_from_enum)cur_src->GetSourceFrom();
		task_peerinfo_internal.peer_name			= cur_src->GetUserName();
		task_peerinfo_internal.queue_pos			= (uint32)cur_src->GetRemoteQueueRank();
		task_peerinfo_internal.size_downloaded		= (uint32)cur_src->GetTransferredDown();
		task_peerinfo_internal.client_version		= cur_src->GetClientSoftVer();
		task_peerinfo_internal.peer_status			= (peer_status_enum)cur_src->GetDownloadState();
		task_peerinfo_internal.peer_file_name		= cur_src->GetClientFilename();
		task_peerinfo_internal.rate_upload			= (uint32)cur_src->GetDownloadDatarate10();
		task_peerinfo_internal.rate_download		= (uint32)cur_src->GetDownloadDatarate();
	}

	//part_file_ptr->srclist.

	return 1;
}

BOOL CCore_eMuleApp::GetTaskPeerInfo(int nPeerIndex,  Task_PeerInfo_t *pDataOut)
// 本函数在 GetTaskPeerCnt() 之后调用，由 GetTaskPeerCnt() 函数实际读取数据
{
	if(pDataOut == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	int nCnt = m_listPeer.size();
	if(nPeerIndex >= nCnt || nPeerIndex < 0 )
	{
		g_nErrorCode = 13;
		return 0;
	}

	Task_PeerInfo_t_internal *pData = &(m_listPeer[nPeerIndex]);

	// export data
	pDataOut->ip                  	= pData->ip;      
	pDataOut->port					= pData->port;
	pDataOut->progress				= pData->progress_total_permillage;       
	pDataOut->client_version		= pData->client_version.GetBuffer();
	pDataOut->peer_file_name		= pData->peer_file_name.GetBuffer();	     
	pDataOut->peer_name				= pData->peer_name.GetBuffer();		
	pDataOut->peer_source			= (uint32)pData->peer_source;  
	pDataOut->peer_status			= (uint32)pData ->peer_status;    
	pDataOut->queue_pos				= pData->queue_pos;
	pDataOut->rate_download			= pData->rate_download;
	pDataOut->rate_upload			= pData->rate_upload;
	pDataOut->size_downloaded		= pData->size_downloaded;   

	return 1;
}

BOOL CCore_eMuleApp::TaskStart(int nIndex)
{

	int task_num = 0;
	if( !GetTaskNum(&task_num) || task_num == 0 /*task not exsit*/)
	{
		g_nErrorCode = 6;
		return 0;
	}

	CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByIndex(nIndex);

	if( part_file_ptr != NULL && part_file_ptr->CanResumeFile() )
//	if ( BitCometCoreInterface::GetInstance()->task_start( pTask, false, true ) )
	{
		part_file_ptr->ResumeFile();
	}
	else
	{
		g_nErrorCode = 8;
		return 0;
	}

	return 1;
}

BOOL CCore_eMuleApp::TaskResume(int nIndex)
{
	int task_num = 0;
	if( !GetTaskNum(&task_num) || task_num == 0 /*task not exsit*/)
	{
		g_nErrorCode = 6;
		return 0;
	}
	
	if (theAppPtr->downloadqueue == NULL)
		return 0;

	if( 0 <= nIndex && nIndex < task_num )
	{
		CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByIndex(nIndex);
		if (part_file_ptr == NULL)
		{
			g_nErrorCode = 6;
			return 0;
		}

		if ( part_file_ptr->CanResumeFile() )
		{
			part_file_ptr->ResumeFile();
			
		}
		return 1;
	}
	g_nErrorCode = 9;
	return 0;	
}

BOOL CCore_eMuleApp::TaskStop(int nIndex)
{
	int task_num = 0;
	if( !GetTaskNum(&task_num) || task_num == 0 /*task not exsit*/)
	{
		g_nErrorCode = 6;
		return 0;
	}

	if (theAppPtr->downloadqueue == NULL)
		return 0;

	if( 0 <= nIndex && nIndex < task_num )
	{
		CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByIndex(nIndex);
		if (part_file_ptr == NULL)
		{
			g_nErrorCode = 6;
			return 0;
		}

		if ( part_file_ptr->CanStopFile() )
		{
			part_file_ptr->StopFile();
		}
		return 1;
	}

	g_nErrorCode = 9;
	return 0;	
}

BOOL CCore_eMuleApp::TaskDelete(int nIndex)
{
	int task_num = 0;
	if( !GetTaskNum(&task_num) || task_num == 0 /*task not exsit*/)
	{
		g_nErrorCode = 6;
		return 0;
	}

	if (theAppPtr->downloadqueue == NULL)
		return 0;

	if( 0 <= nIndex && nIndex < task_num )
	{
		CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByIndex(nIndex);
		if (part_file_ptr == NULL)
		{
			g_nErrorCode = 6;
			return 0;
		}

		part_file_ptr->DeleteFile();
		return 1;
	}

	g_nErrorCode = 9;
	return 0;	
}

BOOL CCore_eMuleApp::TaskPause(int nIndex)
{
	int task_num = 0;
	if( !GetTaskNum(&task_num) || task_num == 0 /*task not exsit*/)
	{
		g_nErrorCode = 6;
		return 0;
	}

	if (theAppPtr->downloadqueue == NULL)
		return 0;

	if( 0 <= nIndex && nIndex < task_num )
	{
		CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByIndex(nIndex);
		if (part_file_ptr == NULL)
		{
			g_nErrorCode = 6;
			return 0;
		}

		if ( part_file_ptr->CanPauseFile() )
		{
			part_file_ptr->PauseFile();
			
		}
		return 1; 
	}
	g_nErrorCode = 9;
	return 0;	
}

BOOL CCore_eMuleApp::TaskPriority(int nIndex, int nNewPriority)
{
	int task_num = 0;
	if( !GetTaskNum(&task_num) || task_num == 0 /*task not exsit*/)
	{
		g_nErrorCode = 6;
		return 0;
	}

	if (theAppPtr->downloadqueue == NULL)
		return 0;

	if( 0 <= nIndex && nIndex < task_num )
	{
		CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByIndex(nIndex);
		if (part_file_ptr == NULL)
		{
			g_nErrorCode = 6;
			return 0;
		}
		part_file_ptr->SetDownPriority(static_cast<uint8>(nNewPriority));
		return 1;
	}
	g_nErrorCode = 9;
	return 0;
}

BOOL CCore_eMuleApp::GetDownloadPath( LPCTSTR *pszDownloadPath)
{
	if(pszDownloadPath == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	m_download_path = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	*pszDownloadPath = m_download_path.GetBuffer();

	return 1;
}

BOOL CCore_eMuleApp::SetDownloadPath( LPCTSTR szDownloadPath)
{
	if(szDownloadPath == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	m_plugin_settings.download_path	= szDownloadPath;

	thePrefs.m_strIncomingDir = szDownloadPath;

	// check whether the directory thePrefs.incomingdir is existed
	// if not, create this directory
	if (!PathFileExists(thePrefs.m_strIncomingDir))
	{
		::CreateDirectory(thePrefs.m_strIncomingDir, 0);
	}
	return 1;
}

BOOL CCore_eMuleApp::SetConnection( Connection_setting_t *pInfo)
{
	if(pInfo == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	m_plugin_settings.MaxUploadSpeed	= pInfo->MaxUploadSpeed;
	m_plugin_settings.MaxDownloadSpeed	= pInfo->MaxDownloadSpeed;
	m_plugin_settings.listen_port_tcp	= pInfo->listen_port_tcp;
	m_plugin_settings.listen_port_udp	= pInfo->listen_port_udp;

	// do not set rate limit since v 1.03
	//thePrefs.SetMaxDownload((UINT)(pInfo->MaxDownloadSpeed / 1024));	// 0 means unlimited
	//thePrefs.SetMaxUpload((UINT)(pInfo->MaxUploadSpeed / 1024));		// 0 means unlimited

	uint16 nNewPort = 0;

	nNewPort = static_cast<uint16>(pInfo->listen_port_tcp);
	if ( nNewPort != 0 && nNewPort != (int)thePrefs.port )
	{
		thePrefs.port = nNewPort;
		if(theAppPtr)
		{
			if (theAppPtr->IsPortchangeAllowed() && theAppPtr->listensocket != NULL)
				theAppPtr->listensocket->Rebind();
		}
	}

	nNewPort = static_cast<uint16>(pInfo->listen_port_udp);
	if ( nNewPort != 0 && nNewPort != thePrefs.udpport )
	{
		thePrefs.udpport = nNewPort;
		if(theAppPtr)
		{
			if (theAppPtr->IsPortchangeAllowed() && theAppPtr->clientudp != NULL)
				theAppPtr->clientudp->Rebind();
		}
	}

	if(theAppPtr)
	{
		if (theAppPtr->scheduler != NULL)
			theAppPtr->scheduler->SaveOriginals();
	}

	return 1;
}

BOOL CCore_eMuleApp::GetConnectionSetting( Connection_setting_t *pInfo)
{
	if(pInfo == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	pInfo->MaxUploadSpeed	= static_cast<int>(thePrefs.GetMaxDownloadInBytesPerSec());
	pInfo->MaxDownloadSpeed = static_cast<int>(thePrefs.GetMaxUpload()) * 1024;
	pInfo->listen_port_tcp	= thePrefs.GetPort();
	pInfo->listen_port_udp	= thePrefs.GetUDPPort();

	//pInfo->MaxUploadSpeed = SETTING(MAX_UPLOAD_SPEED);
	//pInfo->MaxDownloadSpeed = SETTING(MAX_DOWNLOAD_SPEED);
	//pInfo->listen_port_tcp = SETTING(LISTEN_PORT_TCP);
	//pInfo->listen_port_udp = SETTING(LISTEN_PORT_UDP);

	return 1;
}

BOOL CCore_eMuleApp::SearchFileStart(LPCTSTR filename, uint64 file_size)
{
	if ( theAppPtr == NULL || theAppPtr->emuledlg == NULL || theAppPtr->emuledlg->searchwnd == NULL 
		|| theAppPtr->emuledlg->searchwnd->m_pwndResults == NULL 
		|| theAppPtr->emuledlg->searchwnd->m_pwndResults->m_pwndParams == NULL)
		return FALSE;

	// 清除已打开的搜索结果
	SendMessage(theAppPtr->emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_DELETEALLSEARCHES, 0);

	// 转义括号，以防当作布尔表达式运算符
	CString strExpression = filename;
	strExpression.Replace(_T("("), _T(" "));
	strExpression.Replace(_T(")"), _T(" "));

	SSearchParams* pParams = new SSearchParams;
	pParams->strExpression = strExpression;
	pParams->eType = SearchTypeEd2kGlobal;
	pParams->strSpecialTitle = filename;
	if (pParams->strSpecialTitle.GetLength() > 50){
		pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
	}
	uint64 file_size_min = file_size;
	if(file_size_min > 10)
		file_size_min -= 10;
	uint64 file_size_max = file_size + 10;

	file_size_max = max(file_size_max, file_size_min);

	TCHAR buffer[65];
	_ui64tot(file_size_min, buffer, 10);
	pParams->strMinSize = CString(buffer) + _T("b");
	pParams->ullMinSize = file_size_min;

	_ui64tot(file_size_max, buffer, 10);
	pParams->strMaxSize = CString(buffer) + _T("b");
	pParams->ullMaxSize = file_size_max;

	theAppPtr->emuledlg->searchwnd->m_pwndResults->StartSearch(pParams);
	m_dwLastSearchID = pParams->dwSearchID;
	theAppPtr->emuledlg->SetActiveDialog(theAppPtr->emuledlg->searchwnd);

	int iCurSel = theAppPtr->emuledlg->searchwnd->m_pwndResults->searchselect.GetCurSel();
	if(iCurSel >= 0)
	{
		TCITEM item;
		item.mask = TCIF_PARAM;
		if (theAppPtr->emuledlg->searchwnd->m_pwndResults->searchselect.GetItem(iCurSel, &item) && item.lParam != NULL)
		{
			theAppPtr->emuledlg->searchwnd->m_pwndResults->m_pwndParams->SetParameters((const SSearchParams*)item.lParam);
		}
	}
	

	return TRUE;
}

BOOL CCore_eMuleApp::SearchFileStop()
{
	if ( theAppPtr == NULL || theAppPtr->emuledlg == NULL )
		return FALSE;

	// 清除已打开的搜索结果
	SendMessage(theAppPtr->emuledlg->m_hWnd,WEB_GUI_INTERACTION, WEBGUIIA_DELETEALLSEARCHES, 0);

	m_dwLastSearchID = 0;

	return TRUE;
}

BOOL CCore_eMuleApp::GetFileSearchResultInfo(FileSearchResultInfo_t& info)
{
	info.state = FileSearchResultInfo_t::search_stopped;
	info.item_num = 0;
	info.progress_max = 0;
	info.progress_current = 0;

	if ( theAppPtr == NULL || theAppPtr->emuledlg == NULL || theAppPtr->emuledlg->searchwnd == NULL 
		|| theAppPtr->emuledlg->searchwnd->m_pwndResults == NULL 
		|| theAppPtr->searchlist == NULL)
		return FALSE;

	info.state = FileSearchResultInfo_t::server_disconnected;
	if(theAppPtr->serverconnect->IsConnecting())
	{
		info.state = FileSearchResultInfo_t::server_disconnecting;
	}
	else if(theAppPtr->serverconnect->IsConnected())
	{
		if(m_dwLastSearchID == 0)
			info.state = FileSearchResultInfo_t::search_stopped;
		else
			info.state = FileSearchResultInfo_t::search_in_progress;
	}

	if(info.state != FileSearchResultInfo_t::search_in_progress)
	{
		info.item_num = 0;
		return TRUE;
	}

	CQArray<SearchFileStruct, SearchFileStruct> SearchFileArray;
	theAppPtr->searchlist->GetWebList(&SearchFileArray, 0);

	m_lastSearchFileResult.clear();
	for (uint16 i = 0; i < SearchFileArray.GetCount(); ++i)
	{
		FileSearchResultItem_t_internal item;
		SearchFileStruct& structFile = SearchFileArray.GetAt(i);

		item.file_name = structFile.m_strFileName;
		item.file_size = structFile.m_uFileSize;
		item.total_source_num = structFile.m_uSourceCount;
		item.complete_source_num = structFile.m_dwCompleteSourceCount;
		item.file_ID = structFile.m_strFileHash;

		m_lastSearchFileResult.push_back(item);
	}

	info.item_num = m_lastSearchFileResult.size();

	theAppPtr->emuledlg->searchwnd->m_pwndResults->get_search_progress(info.progress_max, info.progress_current);

	return TRUE;
}

BOOL CCore_eMuleApp::GetFileSearchResultItem(uint32 nItemIndex, FileSearchResultItem_t& item)
{
	if ( nItemIndex >= m_lastSearchFileResult.size())
		return FALSE;

	item.file_name = m_lastSearchFileResult[nItemIndex].file_name.GetBuffer();
	item.file_size = m_lastSearchFileResult[nItemIndex].file_size;
	item.total_source_num = m_lastSearchFileResult[nItemIndex].total_source_num;
	item.complete_source_num = m_lastSearchFileResult[nItemIndex].complete_source_num;
	item.file_ID = m_lastSearchFileResult[nItemIndex].file_ID.GetBuffer();

	return TRUE;
}

BOOL CCore_eMuleApp::ConnectServer()
{
	if ( theAppPtr == NULL || theAppPtr->emuledlg == NULL )
		return FALSE;

	theAppPtr->emuledlg->PostMessage(WM_COMMAND, MP_CONNECT, 0);

	return TRUE;
}

BOOL CCore_eMuleApp::OpenED2KLink(CString link, ED2K_Link_Open_Ret_t& open_ret)
{
	memset(open_ret.task_id_out, 0, 16);

	if (link.IsEmpty())
		return FALSE;

	bool is_tasklist_loaded = false;
	if(theAppPtr && theAppPtr->emuledlg)
		is_tasklist_loaded = (theAppPtr->emuledlg->status > 4);
	if(!is_tasklist_loaded)
	{
		theAppPtr->pstrPendingLink = new CString(link);
		theAppPtr->sendstruct.cbData = (theAppPtr->pstrPendingLink->GetLength() + 1)*sizeof(TCHAR);
		theAppPtr->sendstruct.dwData = OP_ED2KLINK;
		theAppPtr->sendstruct.lpData = const_cast<LPTSTR>((LPCTSTR)(*(theAppPtr->pstrPendingLink)));
		return TRUE;
	}

	theAppPtr->AddEd2kLinksToDownload(link, 0);
	CED2KLink* pLink = NULL;
	try
	{
		pLink = CED2KLink::CreateLinkFromUrl(link.Trim());
	}
	catch(CString error)
	{
		return FALSE;
	}
	if (pLink)
	{
		if (pLink->GetKind() == CED2KLink::kFile)
		{
			memcpy(open_ret.task_id_out, pLink->GetFileLink()->GetHashKey(), 16);
		}
		else
		{
			return FALSE;
		}
		delete pLink;
	}

	return TRUE;
}

BOOL CCore_eMuleApp::GetTempPath( LPCTSTR *pszTempPath)
{
	if(pszTempPath == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	ASSERT(thePrefs.GetTempDirCount() == 1);

	m_temp_path = thePrefs.GetTempDir();
	*pszTempPath = m_temp_path.GetBuffer();

	return 1;

}

BOOL CCore_eMuleApp::SetTempPath( LPCTSTR szTempPath)
{
	if(szTempPath == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	m_plugin_settings.temp_path	= szTempPath;

	if(thePrefs.GetTempDirCount() == 0)
	{
		thePrefs.tempdir.Add(CString(szTempPath));
	}
	else
	{
		thePrefs.tempdir.SetAt(0, CString(szTempPath));
	}

	// check whether the directory thePrefs.incomingdir is existed
	// if not, create this directory
	if (!PathFileExists(thePrefs.GetTempDir()))
	{
		::CreateDirectory(thePrefs.GetTempDir(), 0);
	}
	return 1;
}

int CCore_eMuleApp::TaskHashToIndex(const uchar filehash[16])
{
	ASSERT(filehash != NULL);
	if (filehash == NULL)
	{
		return -1;
	}

	int task_num = 0;

	if (!GetTaskNum(&task_num) || task_num == 0)
	{
		return -1;
	}

	if (theAppPtr->downloadqueue == NULL)
		return -1;

	int file_num = theAppPtr->downloadqueue->GetFileCount();
	for ( int i=0; i < file_num; i++)
	{
		CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByIndex(i);
		if ( part_file_ptr == NULL )
			continue;

		if ( memcmp(part_file_ptr->GetFileHash(), filehash, 16) == 0)
		{
			return i;
		}
	}
	return -1;
}


BOOL CCore_eMuleApp::ShowEMDlg(bool bShow)
{
	if (theAppPtr == NULL || theAppPtr->m_pMainWnd == NULL)
		return FALSE;

	if(bShow)
	{
		theAppPtr->m_pMainWnd->ShowWindow(SW_SHOW);
		HWND hwnd = theAppPtr->m_pMainWnd->GetSafeHwnd();
		if(hwnd)
		{
			SetForegroundWindow(hwnd);
			if( IsIconic(hwnd))
			{
				// restore
				::ShowWindow(hwnd, SW_RESTORE);
			}
		}
	}
	else
	{
		theAppPtr->m_pMainWnd->ShowWindow(SW_HIDE);
	}
	return TRUE;
}

void CCore_eMuleApp::SetNotifyWindow(HWND hwnd)
{
	m_hMainWnd = hwnd;

	// start timer thread
	AfxBeginThread(PluginTimer_ThreadProc, (LPVOID)m_hMainWnd, THREAD_PRIORITY_NORMAL);
	TRACE(_T("[AfxBeginThread] - PluginTimer_ThreadProc\n"));

}

int CCore_eMuleApp::SharedFileHashToIndex(const uchar filehash[16])
{
	ASSERT(filehash != NULL);
	if (filehash == NULL)
	{
		return -1;
	}

	int shared_file_num = 0;

	if (!GetSharedFileNum(&shared_file_num) || shared_file_num == 0)
	{
		return -1;
	}

	if (theAppPtr->sharedfiles == NULL)
		return -1;

	int file_num = theAppPtr->sharedfiles->GetCount();
	for ( int i=0; i < file_num; i++)
	{
		CKnownFile* known_file_ptr = theAppPtr->sharedfiles->GetFileByIndex(i);
		if ( known_file_ptr == NULL )
			continue;

		if ( memcmp(known_file_ptr->GetFileHash(), filehash, 16) == 0)
		{
			return i;
		}
	}
	return -1;
}

BOOL CCore_eMuleApp::GetSharedFileInfo(int nIndex, Task_info_t *pDataOut)
{
	if( pDataOut == NULL )
	{
		g_nErrorCode = 5;
		return 0;
	}

	if (theAppPtr->sharedfiles == NULL)
		return 0;

	CKnownFile* known_file_ptr = theAppPtr->sharedfiles->GetFileByIndex(nIndex);
	if (NULL == known_file_ptr)
	{
		g_nErrorCode = 6;
		return 0;
	}

	m_task_info.Task_state							= TASK_COMPLETE;

	if ( m_task_info.Task_show_name != NULL )
	{
		delete m_task_info.Task_show_name;
		m_task_info.Task_show_name = NULL;
	}
	m_task_info.Task_show_name						= _tcsdup(known_file_ptr->GetFileName());
	ASSERT( m_task_info.Task_show_name != NULL );

	m_task_info.Task_complete_permillage			= 1000;
	m_task_info.Task_hashcheck_permillage			= 0;

	m_task_info.Task_download_rate					= 0;
	m_task_info.Task_time_left						= 0;
	m_task_info.Task_num_available_source			= 0;
	m_task_info.Task_num_total_source				= 0;
	m_task_info.Task_num_transferring_source		= 0;

	m_task_info.Task_size_total						= (__int64)known_file_ptr->GetFileSize();
	m_task_info.Task_size_left						= 0;
	m_task_info.Task_size_finished					= (__int64)known_file_ptr->GetFileSize();
	m_task_info.Task_size_transferred				= 0;
	m_task_info.Task_num_parts						= known_file_ptr->GetPartCount();

	if ( m_task_info.Task_save_folder != NULL )
	{
		delete m_task_info.Task_save_folder;
		m_task_info.Task_save_folder = NULL;
	}
	m_task_info.Task_save_folder 					= _tcsdup(known_file_ptr->GetPath());
	ASSERT( m_task_info.Task_save_folder != NULL );

	if ( m_task_info.Task_save_name != NULL )
	{
		delete m_task_info.Task_save_name;
		m_task_info.Task_save_name	= NULL;
	}
	m_task_info.Task_save_name						= _tcsdup(known_file_ptr->GetFileName());	
	ASSERT( m_task_info.Task_save_name );

	// export data
	pDataOut->Task_state							= m_task_info.Task_state;
	pDataOut->Task_show_name						= m_task_info.Task_show_name;
	pDataOut->Task_complete_permillage				= m_task_info.Task_complete_permillage;
	pDataOut->Task_hashcheck_permillage				= m_task_info.Task_hashcheck_permillage;

	pDataOut->Task_download_rate					= m_task_info.Task_download_rate;
	pDataOut->Task_time_left						= m_task_info.Task_time_left;
	pDataOut->Task_num_available_source				= m_task_info.Task_num_available_source;
	pDataOut->Task_num_total_source					= m_task_info.Task_num_total_source;
	pDataOut->Task_num_transferring_source			= m_task_info.Task_num_transferring_source;

	pDataOut->Task_size_total						= m_task_info.Task_size_total;
	pDataOut->Task_size_left						= m_task_info.Task_size_left;
	pDataOut->Task_size_finished					= m_task_info.Task_size_finished;
	pDataOut->Task_size_transferred					= m_task_info.Task_size_transferred;
	pDataOut->Task_num_parts						= m_task_info.Task_num_parts;

	pDataOut->Task_save_folder						= m_task_info.Task_save_folder;
	pDataOut->Task_save_name						= m_task_info.Task_save_name;

	return 1;
}

BOOL CCore_eMuleApp::GetSharedFileNum(int* pNum)
{
	if(pNum == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	if (theAppPtr->sharedfiles == NULL)
		return 0;

	*pNum = theAppPtr->sharedfiles->GetCount();
	return 1;
}

bool CCore_eMuleApp::wrapper_is_simple_client(const uchar FileID[16])
{
	if (!CSimpleClientTaskManager::HasInstance())
		return FALSE;

	ASSERT(FileID != NULL);

	if (CSimpleClientTaskManager::GetInstance()->is_in_task_list(FileID))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CCore_eMuleApp::SimpleClientOpen(ED2K_Link_Open_t &OpenInfo, ED2K_Link_Open_Ret_t &OpenRet)
{	
	bool	ret = false;

	ASSERT( CSimpleClientTaskManager::HasInstance() );
	if ( !CSimpleClientTaskManager::HasInstance() )
	{
		g_nErrorCode = 1;	
		return 0;
	}

	ASSERT(OpenInfo.strED2KLink != NULL);
	if (OpenInfo.strED2KLink == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	CString link = OpenInfo.strED2KLink;
	CED2KLink* pLink = NULL;
	try
	{
		pLink = CED2KLink::CreateLinkFromUrl(link.Trim());
	}
	catch(CString error)
	{
		return 0;
	}
	ASSERT(pLink != NULL && pLink->GetKind() == CED2KLink::kFile);
	if (pLink == NULL || pLink->GetKind() != CED2KLink::kFile)
	{
		g_nErrorCode = 11;
		return 0;
	}

	CED2KFileLink* pFileLink = NULL;
	pFileLink = pLink->GetFileLink();
	ASSERT(pFileLink != NULL);
	if (pFileLink == NULL)
	{
		delete	pLink;
		pLink	= NULL;

		g_nErrorCode = 1;
		return 0;
	}

	if(theAppPtr == NULL || theAppPtr->downloadqueue == NULL)
	{
		g_nErrorCode = 1;	
		return 0;
	}

	// 判断文件是否已存在于下载队列。若已存在，尽快把已下载数据传给BC
	bool find_exist_part_file = false;
	CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByID(pFileLink->GetHashKey());
	if ( part_file_ptr == NULL )  
	{
		// 任务不存在, 新建任务
		theAppPtr->AddEd2kLinksToDownload(link, 0);

		part_file_ptr = theAppPtr->downloadqueue->GetFileByID( pFileLink->GetHashKey() );
		if(part_file_ptr == NULL)
		{
			AddDebugLogLine(DLP_VERYLOW, _T("%hs: add ed2k link failed: %s"), __FUNCTION__, link.GetBuffer());
			return FALSE;
		}
		
		AddDebugLogLine(DLP_VERYLOW, _T("%hs: add ed2k link ok: %s"), __FUNCTION__, link.GetBuffer());
	}
	else
	{
		// 任务已经存在,需要切换到Simple模式
		find_exist_part_file = true;

		AddDebugLogLine(DLP_VERYLOW, _T("%hs: find exist ed2k link: %s"), __FUNCTION__, link.GetBuffer());
	}

	// 设置为Simple模式
	uint32 bc_cat = get_bc_imported_category();
	part_file_ptr->SetCategory(bc_cat);

	// 启动下载
	if ( part_file_ptr->CanResumeFile() )
		part_file_ptr->ResumeFile();
	
	// 设置任务传回的参数
	memcpy(OpenRet.task_id_out, pFileLink->GetHashKey(), 16);
	OpenRet.simple_client_id = s_last_simple_id++;

	// 在SimpleClientTaskManager中注册该任务
	ret = CSimpleClientTaskManager::GetInstance()->add_task(pFileLink->GetHashKey(), OpenRet.simple_client_id);
	if(ret)
	{
		m_new_added_task.push_back(OpenRet.simple_client_id);
		CEmuleTaskSimple* task = CSimpleClientTaskManager::GetInstance()->get_task_by_id(OpenRet.simple_client_id);
		if(task)
			task->m_file_exist_already = find_exist_part_file;
	}

	delete	pLink;
	pLink	= NULL;

	return ret ? 1 : 0;
}

BOOL CCore_eMuleApp::SimpleClientClose(const int client_id)
{
	BOOL ret = 0;

	ASSERT(CSimpleClientTaskManager::HasInstance());
	if (!CSimpleClientTaskManager::HasInstance())
		return 0;

	uchar hash[16];
	const uchar* p = CSimpleClientTaskManager::GetInstance()->clientid_to_hash(client_id);
	if ( p == NULL )
		return 0;

	memcpy(hash, p, sizeof(hash));

	if ( theAppPtr == NULL || theAppPtr->downloadqueue == NULL )
		return 0;

	bool need_delete = true;

	CEmuleTaskSimple* ptask = NULL;
	ptask = CSimpleClientTaskManager::GetInstance()->get_task_by_id(client_id);
	if(ptask && ptask->m_file_exist_already)
		need_delete = false;

	UINT bc_cat = get_bc_imported_category();
	CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByID(hash);
	if ( part_file_ptr != NULL && part_file_ptr->GetCategory() != bc_cat)
		need_delete = false;

	if(need_delete)
	{
		if ( part_file_ptr != NULL )
			part_file_ptr->DeleteFile();
	}

	if ( CSimpleClientTaskManager::GetInstance()->delete_task(hash) )
		ret = 1;

	return ret;
}

BOOL CCore_eMuleApp::SimpleClientReadData(const int simple_client_id, 
		char* pBuffer, const uint64 offset, const uint32 length)
{
	ASSERT( pBuffer != NULL );
	if (pBuffer == NULL) {
		g_nErrorCode = 5;	
		return 0;
	}

	ASSERT( CSimpleClientTaskManager::HasInstance() );
	if ( !CSimpleClientTaskManager::HasInstance() ) {
		g_nErrorCode = 3;
		return 0;
	}

	CEmuleTaskSimple* ptask = NULL;
	ptask = CSimpleClientTaskManager::GetInstance()->get_task_by_id(simple_client_id);

	ASSERT ( ptask != NULL );
	if ( ptask == NULL ) 
	{
		g_nErrorCode = 14;
		return 0;
	}

	uint64 end = (offset + length - 1);
	CEmuleTaskSimpleRange* range = ptask->find_finished_range(offset, end);
	if ( range == NULL ) {
		g_nErrorCode = 18;
		return 0;
	}

	bool ok = range->read_data(pBuffer);
	if(ok)
	{
		range->release_cache_auto();
		ptask->delete_completed_range(range);
		return 1;
	}

	return 0;
}

//BOOL CCore_eMuleApp::SimpleClientRemove(const uchar hash[16])
//{
//	ASSERT( hash != NULL );
//	if ( hash == NULL )
//	{
//		g_nErrorCode = 5;
//		return 0;
//	}
//
//	if ( CSimpleClientTaskManager::GetInstance()->is_in_task_list(hash) )
//	{
//		g_nErrorCode = 2;
//		return 0;
//	}
//	
//	CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByID(hash);
//	if ( part_file_ptr == NULL )
//	{
//		g_nErrorCode = 14;
//		return 0;
//	}
//
//	part_file_ptr->DeleteFile();
//
//	return 1;
//}

 
BOOL CCore_eMuleApp::SimpleClientGetFinishBlock(int simple_client_id, uint64* pOffset, uint32* pLength)
{
	ASSERT(pOffset != NULL && pLength != NULL);
	if(pOffset == NULL || pLength == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	*pOffset = 0;
	*pLength = 0;

	ASSERT ( CSimpleClientTaskManager::HasInstance() );
	if ( !CSimpleClientTaskManager::HasInstance() ) 
	{
		g_nErrorCode = 1;
		return 0;
	}

	CEmuleTaskSimple* ptask = NULL;
	ptask = CSimpleClientTaskManager::GetInstance()->get_task_by_id(simple_client_id);
	if ( ptask == NULL ) 
	{
		// 若任务刚删除，此处应该已经找不到任务对象
		g_nErrorCode = 14;
		return 0;
	}

	CEmuleTaskSimpleRange* range = ptask->get_first_completed_range();
	if(range == NULL)
	{
		g_nErrorCode = 18;
		return 0;
	}

	uint64 start, end;
	range->get_range(start, end);

	*pOffset 	= start;
	*pLength	= static_cast<uint32>(end - start + 1);

	return 1;
}

BOOL CCore_eMuleApp::SimpleClientReadHashset(int client_id, char* pBuffer, uint32 buf_size, uint32& read_cnt)
// 若 pBuffer == NULL 或 buf_size == 0，则通过 read_cnt 返回需要读取的长度
{
	read_cnt = 0;

	ASSERT ( CSimpleClientTaskManager::HasInstance() );
	if ( !CSimpleClientTaskManager::HasInstance() ) 
	{
		g_nErrorCode = 1;
		return 0;
	}

	uchar hash[16];
	const uchar* p = CSimpleClientTaskManager::GetInstance()->clientid_to_hash(client_id);
	if ( p == NULL )
		return 0;

	memcpy(hash, p, sizeof(hash));

	if ( theAppPtr == NULL || theAppPtr->downloadqueue == NULL )
		return 0;

	CPartFile* file = theAppPtr->downloadqueue->GetFileByID(hash);

	if ( file == NULL )
		return 0;

	CString hashset;
	hashset = _T("p=");
	for (UINT j = 0; j < file->GetHashCount(); j++)
	{
		if (j > 0)
			hashset += _T(':');
		hashset += EncodeBase16(file->GetPartHash(j), 16);
	}

	USES_CONVERSION;
	CStringA hashset_str = T2A(hashset);

	if(pBuffer == NULL || buf_size == 0)
	{
		read_cnt = hashset_str.GetLength();
		return 1;
	}

	if(buf_size >= (uint32)hashset_str.GetLength())
	{
		read_cnt = hashset_str.GetLength();
		memcpy(pBuffer, hashset_str.GetBuffer(), hashset_str.GetLength());
		return 1;
	}

	return 0;
}

BOOL CCore_eMuleApp::SimpleClientGetStatus(int client_id, Simple_Task_Status_t& status)
{
	status.download_rate = 0;
	status.upload_rate = 0;
	status.download_byte = 0;
	status.upload_byte = 0;

	ASSERT ( CSimpleClientTaskManager::HasInstance() );
	if ( !CSimpleClientTaskManager::HasInstance() ) 
	{
		g_nErrorCode = 1;
		return 0;
	}

	uchar hash[16];
	const uchar* p = CSimpleClientTaskManager::GetInstance()->clientid_to_hash(client_id);
	if ( p == NULL )
		return 0;

	memcpy(hash, p, sizeof(hash));

	if ( theAppPtr == NULL || theAppPtr->downloadqueue == NULL )
		return 0;

	CPartFile* file = theAppPtr->downloadqueue->GetFileByID(hash);

	if ( file == NULL )
		return 0;
	
	// 统计下载信息
	status.download_byte = file->GetTransferred();
	if (file->GetTransferringSrcCount())
		status.download_rate = file->GetDownloadDatarate10();

	// 计算part下载源个数
	m_task_status.parts_status.clear();
	m_task_status.parts_status.resize(file->GetPartCount(), 0);
	for (POSITION pos = file->srclist.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = file->srclist.GetNext(pos);
		if( cur_src->GetPartStatus() )
		{		
			for (UINT i = 0; i < file->GetPartCount(); i++)
			{
				if (cur_src->IsPartAvailable(i) && m_task_status.parts_status[i] <= 255)
					m_task_status.parts_status[i] ++;
			}
		}
	}
	if( !m_task_status.parts_status.empty() )
	{
		status.parts_status = &m_task_status.parts_status[0];
		status.parts_num = m_task_status.parts_status.size();
	}
	else
	{
		status.parts_status = NULL;
		status.parts_num = 0;
	}

	// 统计上传信息
	for (POSITION pos = theAppPtr->uploadqueue->GetFirstFromUploadList(); pos != 0; theAppPtr->uploadqueue->GetNextFromUploadList(pos))
	{
		CUpDownClient* cur_client = theAppPtr->uploadqueue->GetQueueClientAt(pos);

		if( memcmp(cur_client->GetUploadFileID(), hash, 16) != 0 )
			continue;
		
		status.upload_rate += cur_client->GetDownloadDatarate10();
		status.upload_byte += cur_client->GetSessionUp();
	}

	return 1;
}

BOOL CCore_eMuleApp::SimpleClientGetImportBlock(int simple_client_id, uint64* pOffset, uint32* pLength)
{
	ASSERT(pOffset != NULL && pLength != NULL);
	if(pOffset == NULL || pLength == NULL)
	{
		g_nErrorCode = 5;
		return 0;
	}

	*pOffset = 0;
	*pLength = 0;

	ASSERT ( CSimpleClientTaskManager::HasInstance() );
	if ( !CSimpleClientTaskManager::HasInstance() ) 
	{
		g_nErrorCode = 1;
		return 0;
	}

	CEmuleTaskSimple* ptask = NULL;
	ptask = CSimpleClientTaskManager::GetInstance()->get_task_by_id(simple_client_id);
	if ( ptask == NULL ) 
	{
		// 若任务刚删除，此处应该已经找不到任务对象
		g_nErrorCode = 14;
		return 0;
	}
	
	uint64 offset = 0;
	uint32 len = 0;
	if(!ptask->get_export_block_range(offset, len))
	{
		g_nErrorCode = 18;
		return 0;
	}

	*pOffset 	= offset;
	*pLength	= len;

	return 1;
}

BOOL CCore_eMuleApp::SimpleClientImportExistFile(const int simple_client_id, char* pBuffer, const uint64 offset, const uint32 length)
{
	ASSERT( pBuffer != NULL );
	if (pBuffer == NULL) {
		g_nErrorCode = 5;	
		return 0;
	}

	ASSERT( CSimpleClientTaskManager::HasInstance() );
	if ( !CSimpleClientTaskManager::HasInstance() ) {
		g_nErrorCode = 3;
		return 0;
	}

	uchar hash[16];
	const uchar* p = CSimpleClientTaskManager::GetInstance()->clientid_to_hash(simple_client_id);
	if ( p == NULL )
		return 0;

	memcpy(hash, p, sizeof(hash));

	if ( theAppPtr == NULL || theAppPtr->downloadqueue == NULL )
		return 0;

	CPartFile* file = theAppPtr->downloadqueue->GetFileByID(hash);

	if ( file == NULL )
		return 0;

	uint64 file_len = file->GetFileSize();
	if ( offset + length > file_len ) 
	{
		g_nErrorCode = 18;
		return 0;
	}

	if(length == 0)
		return 1;

	uint64 start = offset;
	uint64 end = offset + length - 1;
	if(!file->IsComplete(start, end, true))
	{
		return 0;
	}

	try
	{
		file->m_hpartfile.Seek(offset, 0);
		if(file->m_hpartfile.Read(pBuffer, length) != length)
			return 0;
	}
	catch(CString error)
	{
		return 0;
	}

	return 1;
}

void CCore_eMuleApp::host_on_range_complete(uint32 simple_client_id)
{
	// 区块完成处理
	::PostMessage(m_hMainWnd, WM_EMULE_PLUGIN_NOTIFY, simple_client_id, EMULE_NOTIFY_RANGE_COMPLETED);
}

void CCore_eMuleApp::host_get_request_block(const uint32 client_id, const char* available_parts, uint64* pOffset, uint32* pLength)
{
	if ( g_host_callback_func.HOST_GetHostRequestBlock == NULL ) {
		*pOffset = 0;
		*pLength = 0;
		return;
	}

	g_host_callback_func.HOST_GetHostRequestBlock(client_id, available_parts, pOffset, pLength);
}

void CCore_eMuleApp::host_on_get_hashset(uint32 simple_client_id)
{
	if(simple_client_id == -1)
		return;

	::PostMessage(m_hMainWnd, WM_EMULE_PLUGIN_NOTIFY, simple_client_id, EMULE_NOTIFY_GET_HASHSET);
}

void CCore_eMuleApp::host_grant_import_exist_file(uint32 simple_client_id)
{
	if(simple_client_id == -1)
		return;

	::PostMessage(m_hMainWnd, WM_EMULE_PLUGIN_NOTIFY, simple_client_id, EMULE_NOTIFY_GRANT_IMPORT_EXIST_FILE);
}

uint32 CCore_eMuleApp::wrapper_simple_client_write_buffer(const uchar FileID[16], uint64 transize, const BYTE *data, uint64 start, uint64 end, 
		Requested_Block_Struct *block, const CUpDownClient* client, CPartFile& part_file, CTypedPtrList<CPtrList, Requested_Block_Struct*>& requestedblocks_list)
{
	UNUSED_ALWAYS(transize);
	UNUSED_ALWAYS(client);

	CEmuleTaskSimple* ptask = NULL;
	ptask = CSimpleClientTaskManager::GetInstance()->get_task_by_hash(FileID);

	ASSERT ( ptask != NULL );
	if ( ptask == NULL ) {
		return 0;
	}

	ptask->on_block_request_received(data, start, end, block->StartOffset, block->EndOffset, client);

	if(ptask->has_finished_range())
	{
		theDllApp.host_on_range_complete(ptask->get_client_id());
	}

	int len_data = static_cast<int>(end - start + 1); 

	// Mark this small section of the file as filled
	part_file.FillGap(start, end);

	// Update the flushed mark on the requested block 
	// The loop here is unfortunate but necessary to detect deleted blocks.
	POSITION pos = requestedblocks_list.GetHeadPosition();
	while (pos != NULL)
	{	
		if (requestedblocks_list.GetNext(pos) == block)
			block->transferred += len_data;
	}

	return len_data;
}

bool CCore_eMuleApp::wrapper_simple_client_get_next_requested_block(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count,
									const uchar FileID[16], uint16 nPartCount, CTypedPtrList<CPtrList, Requested_Block_Struct*>& requestedblocks_list)
{
	// 本函数被调用时机: [1] 连上peer开始下载. [2] 下载完一个block
	// 返回请求块原则: [1]需要是同一个 part 的. [2] 尽量满足max_block，以防断流. [3] 不应返回该 CUpDownClient 已经发出请求的block

	uint16 max_block = *count;
	max_block = min(max_block, 10);
	*count = 0;

	//simple interface
	if (!CSimpleClientTaskManager::HasInstance())
	{
		return false;
	}

	CEmuleTaskSimple* ptask = NULL;
	ptask 	= CSimpleClientTaskManager::GetInstance()->get_task_by_hash(FileID);

	ASSERT( ptask != NULL );
	if(ptask == NULL)
		return false;

	// [1] 查找插件上次向主程序请求的，但现在还未分配给 CUpDownClient 的下载块
	list<request_block_t*> block_list;
	ptask->get_unfinished_block(block_list, true);

	// [2] 确定该 CUpDownClient 的可用part列表
	string available_parts(nPartCount, '0');
	for ( int i=0; i < nPartCount; i++ ) 
	{
		available_parts[i] = sender->IsPartAvailable(i) ? '1' : '0';
	}

	// [3] 确定该 CUpDownClient 已请求的下载块
	vector<request_block_t*> requested_blocks;
	// requestedblocks_list 是该任务向所有 CUpDownClient 发出的请求, 此处不适用
	//for(POSITION pos = requestedblocks_list.GetHeadPosition(); pos != NULL;)
	//{
	//	Requested_Block_Struct* pRequested_Block = requestedblocks_list.GetNext( pos );
	//	if(pRequested_Block != NULL)
	//	{
	//		request_block_t block(pRequested_Block->StartOffset, pRequested_Block->EndOffset);
	//		requested_blocks.push_back(block);
	//	}
	//}
	ptask->get_requested_block(requested_blocks, sender);

	// [3] 找出合适的请求block
	int suitable_part = -1;
	vector<request_block_t*> suitable_blocks;

	request_block_t::get_suitable_parts(block_list, available_parts, requested_blocks, suitable_part, suitable_blocks);

	// [4] 若请求块不足，则向主程序请求更多的下载区块
	while( suitable_blocks.size() < max_block) 
	{
		uint32 	client_id = ptask->get_client_id();

		uint64 	offsets = 0;
		uint32	length	= 0;

		// 向主程序请求更多的下载块。可能无需要部分，不返回任何请求区块
		theDllApp.host_get_request_block(client_id, available_parts.data(), &offsets, &length);

		if ( length == 0 )
		{
			break;
		}

		// 向 CEmuleTaskSimple 对象添加新的下载区域。允许重复的请求区块
		list<request_block_t*> requested_block_list;
		ptask->add_range(offsets, static_cast<uint64>(offsets+length-1), requested_block_list);
		// 主程序返回的 requested_block_list 可能已向其它 CUpDownClient 请求过了

		// 合并新请求列表
		block_list.sort();
		block_list.unique();
		size_t old_size = block_list.size();

		requested_block_list.sort();
		block_list.merge(requested_block_list);
		block_list.unique();
		size_t new_size = block_list.size();

		if(new_size == old_size)
		{
			break;
		}
		
		// 找出合适的请求block
		request_block_t::get_suitable_parts(block_list, available_parts, requested_blocks, suitable_part, suitable_blocks);
	} 

	// [5] 若主程序没有新请求，以前请求的分块也都分配给其它CUpDownClient下载，则重复请求已分配给其它CUpDownClient的分块
	if(suitable_blocks.empty())
	{
		ptask->get_unfinished_block(block_list, false);
		request_block_t::get_suitable_parts(block_list, available_parts, requested_blocks, suitable_part, suitable_blocks);
	}


	// [6] 向 CUpDownClient 发出下载请求
	uint16 newBlockCount = 0;
	if(!suitable_blocks.empty())
	{
		for(vector<request_block_t*>::const_iterator iter = suitable_blocks.begin(); iter != suitable_blocks.end(); iter++)
		{
			request_block_t* block = *iter;
			ASSERT(block);
			if(block)
			{
				ASSERT(sender->IsPartAvailable(block->PartNum));
				if ( sender->IsPartAvailable(block->PartNum) )
				{
					block->add_download_client(sender);

					Requested_Block_Struct* pRequested_Block = new Requested_Block_Struct;
					pRequested_Block->StartOffset	= block->StartOffset;
					pRequested_Block->EndOffset		= block->EndOffset;
					pRequested_Block->transferred	= 0;
					memcpy(pRequested_Block->FileID, FileID, 16);
					newblocks[newBlockCount++] = pRequested_Block;
					requestedblocks_list.AddTail(pRequested_Block);
					*count = newBlockCount;

					if( newBlockCount >= max_block ) 
					{
						break;
					}
				}
			}
		}
	}

	return (newBlockCount > 0);
}

void CCore_eMuleApp::wrapper_on_emule_dlg_close(void)
{
	// 调用SimpleClientTaskManager的清理函数，清除正在下载的任务
	if (CSimpleClientTaskManager::HasInstance())
		CSimpleClientTaskManager::GetInstance()->clear();

	// 通知 PluginHost 退出
	uint32 client_id = 0;
	notify_id_enum notify_id = EMULE_NOTIFY_DLG_CLOSE;
	::PostMessage(m_hMainWnd, WM_EMULE_PLUGIN_NOTIFY, (WPARAM)client_id, (LPARAM)notify_id);
}

void CCore_eMuleApp::wrapper_on_cancel_block_request(const uchar FileID[16], uint64 start, uint64 end, const CUpDownClient* client)
{
	if ( !wrapper_is_simple_client(FileID) )
		return;

	if (!CSimpleClientTaskManager::HasInstance())
		return;

	CEmuleTaskSimple* ptask = NULL;
	ptask 	= CSimpleClientTaskManager::GetInstance()->get_task_by_hash(FileID);

	ASSERT( ptask != NULL );
	if(ptask == NULL)
		return;

	ptask->on_block_request_cancel(start, end, client);

}

void CCore_eMuleApp::wrapper_on_get_hashset(const uchar FileID[16])
{
	if ( !wrapper_is_simple_client(FileID) )
		return;

	if (!CSimpleClientTaskManager::HasInstance())
		return;

	CEmuleTaskSimple* ptask = NULL;
	ptask 	= CSimpleClientTaskManager::GetInstance()->get_task_by_hash(FileID);

	ASSERT( ptask != NULL );
	if(ptask == NULL)
		return;

	host_on_get_hashset(ptask->get_client_id());
}

uint32 CCore_eMuleApp::get_bc_imported_category()
{
	// 检查并创建 _bitcomet_imported 分类
	uint32 cat_id = 0;
	bool bc_cat_existed = false;
	for (uint32 i = 0; i < (uint32)thePrefs.GetCatCount(); i++)
	{
		CString cat_title;
		if(thePrefs.GetCategory(i))
			cat_title = thePrefs.GetCategory(i)->strTitle;
		if(cat_title == BC_IMPORTED_CAT)
		{
			bc_cat_existed = true;
			cat_id = i;
			break;
		}
	}
	if(!bc_cat_existed)
	{
		cat_id = theAppPtr->emuledlg->transferwnd->AddCategory(BC_IMPORTED_CAT, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),_T(""),_T(""));
		theAppPtr->emuledlg->searchwnd->UpdateCatTabs();
		thePrefs.SaveCats();
		theAppPtr->emuledlg->transferwnd->VerifyCatTabSize();
	}

	return cat_id;
}