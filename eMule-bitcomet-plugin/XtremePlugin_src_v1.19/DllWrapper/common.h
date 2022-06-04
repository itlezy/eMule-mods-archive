/*! \file common.h
\brief ED plugin's common header.
*/
#pragma once

#ifndef VERSION_LIB
#ifndef VERSION_DLL
//should define VERSION_LIB or VERSION_DLL in project setting
#error define VERSION_LIB or VERSION_DLL to build this project
#endif
#endif

enum task_state_enum
{
	//TASK_STOPPED, TASK_RUNNING, TASK_HASHCHECKING,
	TASK_READY			= 0,
	TASK_EMPTY			= 1,
	TASK_WAITINGFORHASH	= 2,
	TASK_HASHING		= 3,
	TASK_ERROR			= 4,
	TASK_INSUFFICIENT	= 5,
	TASK_UNKNOWN		= 6,
	TASK_PAUSED			= 7,
	TASK_COMPLETING		= 8,
	TASK_COMPLETE		= 9,
};


enum task_command_t
{
	CMD_RESUME,
	CMD_STOP,
	CMD_DELETE,
};

struct Task_info_t  // 16 items
{
	task_state_enum		Task_state;
	LPCTSTR				Task_show_name;					// 任务显示名称
	int					Task_complete_permillage;		// 0 - 1000 means 0 - 100%
	int					Task_hashcheck_permillage;
	int					Task_download_rate;				// Byte/s
	int					Task_time_left;

	int					Task_num_available_source;
	int					Task_num_total_source;
	int					Task_num_transferring_source;

	__int64				Task_size_total;
	__int64				Task_size_left;
	__int64				Task_size_finished;
	__int64				Task_size_transferred;

	int					Task_num_parts;
	//int					Task_num_pieces_left;
	LPCTSTR				Task_save_folder;
	LPCTSTR				Task_save_name;
};


struct global_statistic_t  // 7 items
{
	int g_count_task;
	int g_count_shared_file;
	int g_download_rate_kb;	
	int g_upload_rate_kb;		
	bool g_is_tasklist_loaded;
	bool g_is_server_connected;
	bool g_is_server_connecting;
	LPCTSTR g_lan_ip;
	LPCTSTR g_wan_ip;
};


struct Task_ServerInfo_t  // 17 items
{
	CString		str_name;
	CString		str_description;

	uint32		ip;
	uint16		port;

	uint32		ping;
	uint32		files;
	uint32		users;
	uint32		maxusers;
	uint32		softfiles;
	uint32		hardfiles;
	uint32		low_id_users;

	bool		staticservermember;
	uint32		failedcount; 
	CString		str_version;
};

struct Task_FileInfo_t  // 3 items
{
	LPCTSTR		Progress;
	LPCTSTR		relative_path_name;
	LPCTSTR		size;
};

struct Task_PeerInfo_t  // 10 items
{
	uint32		ip;
	uint16		port;
	uint32		progress;
	uint64		size_downloaded;
	uint32		queue_pos;
	uint32		peer_status;
	uint32		rate_download;
	uint32		rate_upload;
	uint32		peer_source;
	LPCTSTR		peer_name;
	LPCTSTR		peer_file_name;
	LPCTSTR		client_version;
};

struct ED2K_Link_Open_t  // 2 items
{
	LPCTSTR		strED2KLink;
	//uint64		offset;
	//uint64		length;
};

struct ED2K_Link_Open_Ret_t  // 2 items
{
	char		task_id_out[16];
	uint32		simple_client_id;
};

struct Connection_setting_t  // 4 items
{  // 以下各项，设置时取0则忽略
	uint32		MaxUploadSpeed;   // 单位为bytes/s
	uint32		MaxDownloadSpeed; // 单位为bytes/s
	uint16		listen_port_tcp;
	uint16		listen_port_udp;
};

struct FileSearchResultInfo_t
{
	enum search_state_enum
	{
		server_disconnected,
		server_disconnecting,
		search_stopped,
		search_in_progress,

		state_end, // for boundary check
	};

	search_state_enum	state;
	uint32				item_num;
	sint32				progress_max;
	sint32				progress_current;
};

struct FileSearchResultItem_t
{
	LPCTSTR		file_name;
	uint64		file_size;
	uint32		total_source_num;
	uint32		complete_source_num;
	LPCTSTR		file_ID;
};

struct Simple_Task_Status_t
{
	uint32		download_rate;
	uint32		upload_rate;
	uint64		download_byte;
	uint64		upload_byte;
	uint32		parts_num;		 // part个数
	uint8*		parts_status;    // 每个part的下载源个数
};

#define WM_EMULE_PLUGIN_NOTIFY			(WM_USER+0x121+0)
// WPARAM - simple_client_id
// LPARAM - notify_id

// CCore_BitCometCEApp::SetWndHandle() 消息序号
enum  notify_id_enum // ！！顺序不能变！！
{
	EMULE_NOTIFY_RANGE_COMPLETED = 0,
	EMULE_NOTIFY_GET_HASHSET,
	EMULE_NOTIFY_FILE_NO_SOURCE,
	EMULE_NOTIFY_BLOCK_NO_SOURCE,
	EMULE_NOTIFY_PLUGIN_TIMER,
	EMULE_NOTIFY_DLG_CLOSE,
	EMULE_NOTIFY_FILE_SEARCH_UPDATED,
	EMULE_NOTIFY_SIMPLE_TASK_STATUS_UPDATED,
	EMULE_NOTIFY_GRANT_IMPORT_EXIST_FILE,

	EMULE_NOTIFY_END,          // defined for boundary check
};

//////////////////////////////////////////////////////////////////////////
// Host接口函数定义：
typedef BOOL (__stdcall *LPHOST_GetHostRequestBlock)(const uint32 client_id, const char* available_parts, uint64* pOffset, uint32* pLength);
typedef BOOL (__stdcall *LPHOST_GetHostPieceStatus)(const uint32 client_id,  const char* &finished_pieces, uint32& piece_num, uint32& piece_size, uint32& first_piece_size);
typedef BOOL (__stdcall *LPHOST_ReadHostBlock)(const uint32 client_id, char* pBuffer, const uint64 offset, const  uint32 length);

#define DECLEAR_PLUGIN_INTERFACE(func)	\
	LP##func func;

struct host_callback_t  // 3 items
{  // 以下各项，设置时取0则忽略
	DECLEAR_PLUGIN_INTERFACE(HOST_GetHostRequestBlock)
	DECLEAR_PLUGIN_INTERFACE(HOST_GetHostPieceStatus)
	DECLEAR_PLUGIN_INTERFACE(HOST_ReadHostBlock)

	host_callback_t() {ZeroMemory(this, sizeof(host_callback_t));}
};

//////////////////////////////////////////////////////////////////////////
// 插件接口函数定义：

typedef BOOL	(__stdcall *LPEM_Init)(BYTE key[]);
typedef BOOL 	(__stdcall *LPEM_Exit)();
typedef BOOL	(__stdcall *LPEM_Start)();
typedef BOOL 	(__stdcall *LPEM_Stop)();
typedef LPCTSTR (__stdcall *LPEM_GetVer)();
typedef LPCTSTR (__stdcall *LPEM_GetErrText)();
typedef int 	(__stdcall *LPEM_GetErrIndex)();
typedef BOOL 	(__stdcall *LPEM_GetTaskNum)(int* Num);
typedef BOOL 	(__stdcall *LPEM_GetTaskInfo)(int nTaskIndex, Task_info_t *pData);
typedef BOOL	(__stdcall *LPEM_SendTaskCmd)(int nTaskIndex, task_command_t Cmd, WPARAM wParam);
typedef BOOL	(__stdcall *LPEM_GetGlobalStatistic)(global_statistic_t *pData);
typedef BOOL	(__stdcall *LPEM_GetServerNum)(int *pCnt);
typedef BOOL	(__stdcall *LPEM_GetServerInfo)(int nServerIndex, Task_ServerInfo_t *pData);
typedef BOOL	(__stdcall *LPEM_GetTaskPeerNum)(int nTaskIndex, int *pCnt);
typedef BOOL	(__stdcall *LPEM_GetTaskPeerInfo)(int nPeerIndex, Task_PeerInfo_t *pData);
typedef BOOL	(__stdcall *LPEM_OpenED2KLink)(ED2K_Link_Open_t &OpenInfo, ED2K_Link_Open_Ret_t& OpenRet);
typedef BOOL	(__stdcall *LPEM_OnPluginNotify)(WPARAM wParam, LPARAM &lParam);
typedef BOOL	(__stdcall *LPEM_SetConnectionSetting)( Connection_setting_t *pInfo);
typedef BOOL	(__stdcall *LPEM_GetConnectionSetting)( Connection_setting_t *pInfo);
typedef BOOL	(__stdcall *LPEM_SetDownloadPath)( LPCTSTR szDownloadPath);
typedef BOOL	(__stdcall *LPEM_GetDownloadPath)( LPCTSTR *pszDownloadPath);
typedef BOOL	(__stdcall *LPEM_SetTempPath)( LPCTSTR szTempPath);
typedef BOOL	(__stdcall *LPEM_GetTempPath)( LPCTSTR *pszTempPath);
typedef int     (__stdcall *LPEM_TaskHashToIndex)( const unsigned char* filehash);
typedef BOOL	(__stdcall *LPEM_ShowEMDlg)( bool bShow);
typedef BOOL	(__stdcall *LPEM_SetCallback)(HWND hwnd, host_callback_t callback_func);
typedef BOOL 	(__stdcall *LPEM_GetSharedFileInfo)(int nIndex, Task_info_t *pData);
typedef BOOL 	(__stdcall *LPEM_GetSharedFileNum)(int* Num);
typedef int		(__stdcall *LPEM_SharedFileHashToIndex)( const unsigned char* filehash);
typedef BOOL	(__stdcall *LPEM_SimpleClientOpen)(ED2K_Link_Open_t &OpenInfo, ED2K_Link_Open_Ret_t &OpenRet);
typedef BOOL	(__stdcall *LPEM_SimpleClientGetFinishBlock)(const uint32 client_id, uint64* pOffset, uint32* pLength);
typedef BOOL	(__stdcall *LPEM_SimpleClientReadData)(const uint32 client_id, char* pBuffer, const uint64 offset, const  uint32 length);
typedef BOOL	(__stdcall *LPEM_SimpleClientClose)(const uint32 client_id);
typedef BOOL	(__stdcall *LPEM_SimpleClientGetLastError)(const uint32 client_id, uint64* pOffset, uint32* pLength);
typedef BOOL	(__stdcall *LPEM_SimpleClientReadHashset)(const uint32 client_id, char* pBuffer, uint32 buf_size, uint32& read_cnt);
typedef BOOL	(__stdcall *LPEM_SimpleClientGetStatus)(const uint32 client_id, Simple_Task_Status_t& status);
typedef BOOL	(__stdcall *LPEM_SimpleClientGetImportBlock)(const uint32 client_id, uint64* pOffset, uint32* pLength);
typedef BOOL	(__stdcall *LPEM_SimpleClientImportExistFile)(const uint32 client_id, char* pBuffer, const uint64 offset, const  uint32 length);
typedef BOOL	(__stdcall *LPEM_SearchFileStart)(LPCTSTR filename, uint64 file_size);
typedef BOOL	(__stdcall *LPEM_SearchFileStop)();
typedef BOOL	(__stdcall *LPEM_ConnectServer)();
typedef BOOL	(__stdcall *LPEM_GetFileSearchResultInfo)(FileSearchResultInfo_t& info);
typedef BOOL	(__stdcall *LPEM_GetFileSearchResultItem)(int nItemIndex, FileSearchResultItem_t& item);

struct plugin_interface_t
{
	DECLEAR_PLUGIN_INTERFACE(EM_Init)
	DECLEAR_PLUGIN_INTERFACE(EM_Exit)
	DECLEAR_PLUGIN_INTERFACE(EM_Start)
	DECLEAR_PLUGIN_INTERFACE(EM_Stop)
	DECLEAR_PLUGIN_INTERFACE(EM_GetVer)
	DECLEAR_PLUGIN_INTERFACE(EM_GetErrText)
	DECLEAR_PLUGIN_INTERFACE(EM_GetErrIndex)
	DECLEAR_PLUGIN_INTERFACE(EM_GetTaskNum)
	DECLEAR_PLUGIN_INTERFACE(EM_GetTaskInfo)
	DECLEAR_PLUGIN_INTERFACE(EM_SendTaskCmd)
	DECLEAR_PLUGIN_INTERFACE(EM_GetGlobalStatistic)
	DECLEAR_PLUGIN_INTERFACE(EM_GetServerNum)
	DECLEAR_PLUGIN_INTERFACE(EM_GetServerInfo)
	DECLEAR_PLUGIN_INTERFACE(EM_GetTaskPeerNum)
	DECLEAR_PLUGIN_INTERFACE(EM_GetTaskPeerInfo)
	DECLEAR_PLUGIN_INTERFACE(EM_OpenED2KLink)
	DECLEAR_PLUGIN_INTERFACE(EM_OnPluginNotify)
	DECLEAR_PLUGIN_INTERFACE(EM_SetConnectionSetting)
	DECLEAR_PLUGIN_INTERFACE(EM_GetConnectionSetting)
	DECLEAR_PLUGIN_INTERFACE(EM_SetDownloadPath)
	DECLEAR_PLUGIN_INTERFACE(EM_GetDownloadPath)
	DECLEAR_PLUGIN_INTERFACE(EM_SetTempPath)
	DECLEAR_PLUGIN_INTERFACE(EM_GetTempPath)
	DECLEAR_PLUGIN_INTERFACE(EM_TaskHashToIndex)
	DECLEAR_PLUGIN_INTERFACE(EM_ShowEMDlg)
	DECLEAR_PLUGIN_INTERFACE(EM_SetCallback)
	DECLEAR_PLUGIN_INTERFACE(EM_GetSharedFileInfo)
	DECLEAR_PLUGIN_INTERFACE(EM_GetSharedFileNum)
	DECLEAR_PLUGIN_INTERFACE(EM_SharedFileHashToIndex)
	DECLEAR_PLUGIN_INTERFACE(EM_SimpleClientOpen)
	DECLEAR_PLUGIN_INTERFACE(EM_SimpleClientGetFinishBlock)
	DECLEAR_PLUGIN_INTERFACE(EM_SimpleClientReadData)
	DECLEAR_PLUGIN_INTERFACE(EM_SimpleClientClose)
	DECLEAR_PLUGIN_INTERFACE(EM_SimpleClientGetLastError)
	DECLEAR_PLUGIN_INTERFACE(EM_SimpleClientReadHashset)
	DECLEAR_PLUGIN_INTERFACE(EM_SimpleClientGetStatus)
	DECLEAR_PLUGIN_INTERFACE(EM_SimpleClientGetImportBlock)
	DECLEAR_PLUGIN_INTERFACE(EM_SimpleClientImportExistFile)
	DECLEAR_PLUGIN_INTERFACE(EM_SearchFileStart)
	DECLEAR_PLUGIN_INTERFACE(EM_SearchFileStop)
	DECLEAR_PLUGIN_INTERFACE(EM_ConnectServer)
	DECLEAR_PLUGIN_INTERFACE(EM_GetFileSearchResultInfo)
	DECLEAR_PLUGIN_INTERFACE(EM_GetFileSearchResultItem)

	plugin_interface_t() {ZeroMemory(this, sizeof(plugin_interface_t));}
};

#undef DECLEAR_PLUGIN_INTERFACE