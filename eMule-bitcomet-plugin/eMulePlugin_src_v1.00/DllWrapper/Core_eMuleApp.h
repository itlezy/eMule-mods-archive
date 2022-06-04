// Core_BitCometCE.h : Core_BitCometCE DLL 的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "SingletonEx.h"

#include <vector>
using namespace std;

// declare for common.h 
struct global_statistic_t;
struct Task_info_t;
struct Task_ServerInfo_t;
struct Task_PeerInfo_t;
struct ED2K_Link_Open_t;
struct ED2K_Link_Open_Ret_t;
struct Connection_setting_t;
struct FileSearchResultInfo_t;
struct FileSearchResultItem_t;
struct Simple_Task_Status_t;
enum task_state_enum;

// declare for eMule
struct Requested_Block_Struct;
class CUpDownClient;
class CKnownFile;
class CPartFile;

#define	DEFAULT_NICK		thePrefs.GetHomepageBaseURL()
#define	DEFAULT_TCP_PORT_OLD	4662
#define	DEFAULT_UDP_PORT_OLD	(DEFAULT_TCP_PORT_OLD+10)

#define PORTTESTURL			_T("http://porttest.emule-project.net/connectiontest.php?tcpport=%i&udpport=%i&lang=%i")


// CCore_eMuleApp
// 有关此类实现的信息，请参阅 Core_BitCometCE.cpp
//

enum priority_enum 
{
	priority_low = 0,
	priority_normal, 
	priority_high,
	//priority_veryhigh,
	//priority_verylow,
	//priority_auto,
};

enum peer_status_enum {
	ds_downloading = 0,
	ds_onqueue,
	ds_connected,
	ds_connecting,
	ds_waitcallback,
	ds_waitcallbackkad,
	ds_reqhashset,
	ds_noneededparts,
	ds_toomanyconns,
	ds_toomanyconnskad,
	ds_lowtolowip,
	ds_banned,
	ds_error,
	ds_none,
	ds_remotequeuefull,
};
enum source_from_enum
{
	sf_server				= 0,
	sf_kademlia,
	sf_source_exchange,
	sf_passive,
	sf_link,
};

#ifdef VERSION_DLL
class CCore_eMuleApp : public CWinApp, public SingletonEx<CCore_eMuleApp>
#else
class CCore_eMuleApp  : public BitCometClientLib::Singleton<CCore_eMuleApp>
#endif
{
public:
	CCore_eMuleApp();
	~CCore_eMuleApp();

// 重写
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()

public:
	HANDLE m_hStopThreadEvent;
	BOOL OnBcClientNotify( WPARAM wParam, LPARAM &lParam);
	int Timer();

public:
	BOOL InitPlugin(/*HWND main_wnd*/);
	BOOL ExitPlugin(void);

	BOOL GetGlobalStatistic(global_statistic_t *pDataOut);

	BOOL GetTaskNum(int* pNum);
	BOOL GetTaskInfo(int nIndex, Task_info_t *pDataOut);
	int TaskHashToIndex(const uchar filehash[16]);

	BOOL GetServerNum(int *pCnt);
	BOOL GetServerInfo(int nIndex,  Task_ServerInfo_t *pDataOut);
	BOOL GetTaskPeerNum(int nIndex,  int *pCnt);   // 取出所有peer信息放到vector里面，等待GetTaskPeerInfo一一读取
	BOOL GetTaskPeerInfo(int nPeerIndex,  Task_PeerInfo_t *pDataOut);	// 取GetTaskPeerNum函数调用后缓冲的任务信息

	BOOL GetSharedFileInfo(int nIndex, Task_info_t *pDataOut);
	BOOL GetSharedFileNum(int* pNum);
	int SharedFileHashToIndex(const uchar filehash[16]);

	BOOL TaskStart(int nIndex);
	BOOL TaskStop(int nIndex);
	BOOL TaskPause(int nIndex);
	BOOL TaskResume(int nIndex);
	BOOL TaskDelete(int nIndex);
	BOOL TaskPriority(int nIndex, int nNewPriority);

	BOOL OpenED2KLink(CString link, ED2K_Link_Open_Ret_t& open_ret);

	BOOL SetDownloadPath( LPCTSTR szDownloadPath);
	BOOL GetDownloadPath( LPCTSTR *pszDownloadPath);

	BOOL SetTempPath(LPCTSTR szTempPath);
	BOOL GetTempPath(LPCTSTR *pszTempPath);

	BOOL SetConnection( Connection_setting_t *pInfo);
	BOOL GetConnectionSetting( Connection_setting_t *pInfo);
	BOOL SearchFileStart(LPCTSTR filename, uint64 file_size);
	BOOL SearchFileStop();
	BOOL GetFileSearchResultInfo(FileSearchResultInfo_t& info);
	BOOL GetFileSearchResultItem(uint32 nItemIndex, FileSearchResultItem_t& item);
	BOOL ConnectServer();

	// [1] 建立一个Simple任务, 如果emule任务列表里原来已存在该任务, 则直接使用该任务,
	//     如果emule任务列表里不存在该任务, 则新建任务。
	BOOL SimpleClientOpen(ED2K_Link_Open_t &OpenInfo, ED2K_Link_Open_Ret_t &OpenRet);

	// [2] 本函数在接受到EMULE_NOTIFY_RANGE_COMPLETED通知后调用，来读取完成下载的区段数据
	// 注: client_id要是确实存在的任务id, pBuffer要指向能存下足够容纳length长度的数据的一段缓冲区
	BOOL SimpleClientReadData(const int simple_client_id, char* pBuffer, const uint64 offset, const uint32 length);

	// [3] 关闭一个Simple任务, 删除该emule任务和下载文件
	BOOL SimpleClientClose(const int simple_client_id);
	
	//BOOL SimpleClientRemove(const uchar hash[16]);

	// [4] 获取第一个完成的区块位置
	BOOL SimpleClientGetFinishBlock(int simple_client_id, uint64* pOffset, uint32* pLength);

	// [5] 读取hashset
	BOOL SimpleClientReadHashset(int client_id, char* pBuffer, uint32 buf_size, uint32& read_cnt);

	// [6] 读取状态
	BOOL SimpleClientGetStatus(int client_id, Simple_Task_Status_t& status);

	// [7] 获取一个已存在任务已下载数据的区块位置
	BOOL SimpleClientGetImportBlock(int simple_client_id, uint64* pOffset, uint32* pLength);

	// [8] 读取已存在任务已下载的数据
	BOOL SimpleClientImportExistFile(const int simple_client_id, char* pBuffer, const uint64 offset, const uint32 length);

	BOOL ShowEMDlg(bool bShow);
	void SetNotifyWindow(HWND hwnd);

protected:
	void host_get_request_block(
			const uint32 client_id, 
			const char* available_parts,
		   	uint64* pOffset,
		   	uint32* pLength
			);

	void host_on_range_complete(uint32 simple_client_id);
	void host_on_get_hashset(uint32 simple_client_id);
	void host_grant_import_exist_file(uint32 simple_client_id);

protected:
	struct task_info_t_internal	// 29 items
	{
		task_state_enum		Task_state;
		LPCTSTR				Task_show_name;                // 任务显示名称
		int					Task_complete_permillage;		// 0 - 1000 means 0 - 100%
		int					Task_hashcheck_permillage;

		int					Task_download_rate;	// Byte/s
		int					Task_time_left;
		int					Task_num_available_source;
		int					Task_num_total_source;
		int					Task_num_transferring_source;

		__int64				Task_size_total;
		__int64				Task_size_left;
		__int64				Task_size_finished;
		__int64				Task_size_transferred;

		int					Task_num_parts;
		LPCTSTR				Task_save_folder;
		LPCTSTR				Task_save_name;

		task_info_t_internal()
		{
			Task_show_name			= NULL;
			Task_save_folder		= NULL;
			Task_save_name			= NULL;
		}

		~task_info_t_internal()
		{
			if ( Task_show_name != NULL )
			{
				delete Task_show_name;
				Task_show_name		= NULL;
			}

			if ( Task_save_folder != NULL )
			{
				delete Task_save_folder;
				Task_save_folder	= NULL;
			}

			if ( Task_show_name != NULL )
			{
				delete Task_save_name;
				Task_save_name		= NULL;
			}
		}
	};

	struct Task_ServerInfo_t_internal  // 4 items
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

	struct Task_PeerInfo_t_internal  // 11 items
	{
		uint32						ip;
		uint16						port;
		CString						peer_name;
		CString						peer_file_name;
		source_from_enum			peer_source;
		uint64						size_downloaded;
		uint32						progress_total_permillage;		// 0 - 1000 means 0 - 100%
		CString						client_version;
		uint32						queue_pos;						// -1 means queue is full
		peer_status_enum			peer_status;
		uint32						rate_download;
		uint32						rate_upload;
	};

	struct global_statistic_t_internal
	{
		CString g_lan_ip;
		CString g_wan_ip;
	};

	struct FileSearchResultItem_t_internal
	{
		CString		file_name;
		uint64		file_size;
		uint32		total_source_num;
		uint32		complete_source_num;
		CString		file_ID;
	};

	struct Simple_Task_Status_t_internal
	{
		vector<uint8>	parts_status; // 每个part的下载源个数
	};
	
	task_info_t_internal							m_task_info;
	vector<Task_ServerInfo_t_internal>				m_listServer;
	//vector<Torrent_FileInfo_t_internal>				m_listFile;
	vector<Task_PeerInfo_t_internal>				m_listPeer;
	global_statistic_t_internal						m_global_statistic;
	vector<int>										m_task_percent;
	CString											m_download_path;
	CString											m_temp_path;
	CString											m_completed_task_name;
	CString											m_completed_file_name;
	vector<uint32>									m_new_added_task;  // new task needs to load part status after added to CSimpleClientTaskManager
	HWND											m_hMainWnd;
	DWORD											m_dwLastSearchID;
	vector<FileSearchResultItem_t_internal>			m_lastSearchFileResult;
	Simple_Task_Status_t_internal					m_task_status;

public:
	
	struct plugin_settings_t
	{
		CString temp_path;
		CString download_path;

		uint32	MaxUploadSpeed;   // 单位为bytes/s
		uint32	MaxDownloadSpeed; // 单位为bytes/s
		uint16	listen_port_tcp;
		uint16	listen_port_udp;

		plugin_settings_t() : MaxUploadSpeed(0), MaxDownloadSpeed(0), listen_port_tcp(0), listen_port_udp(0) {}
	};
	plugin_settings_t		m_plugin_settings;

public:
	// called by partfile
	bool wrapper_is_simple_client(const uchar FileID[16]);
	uint32 wrapper_simple_client_write_buffer(const uchar FileID[16], uint64 transize, const BYTE *data, uint64 start, uint64 end, 
		Requested_Block_Struct *block, const CUpDownClient* client, CPartFile& part_file, CTypedPtrList<CPtrList, Requested_Block_Struct*>& requestedblocks_list);
	bool wrapper_simple_client_get_next_requested_block(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count,
		const uchar FileID[16], uint16 nPartCount, CTypedPtrList<CPtrList, Requested_Block_Struct*>& requestedblocks_list);

	// called by CemuleDlg
	void wrapper_on_emule_dlg_close(void);
	void wrapper_on_tasklist_loaded(void);

	// called by CUpDownClient
	void wrapper_on_cancel_block_request(const uchar FileID[16], uint64 start, uint64 end, const CUpDownClient* client);

	// called by ...
	void wrapper_on_get_hashset(const uchar FileID[16]);

protected:
	uint32 get_bc_imported_category();
};

#ifdef VERSION_DLL
	extern CCore_eMuleApp theDllApp;
#endif
