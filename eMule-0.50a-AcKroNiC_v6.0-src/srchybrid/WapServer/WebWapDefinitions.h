//emulEspaña - Added by MoNKi [MoNKi: -Wap Server-]
#pragma once
#define SESSION_TIMEOUT_SECS	300	// 5 minutes session expiration
#define SHORT_FILENAME_LENGTH	40	// Max size of file name.

#define SESSION_TIMEOUT_SECS	300	// 5 minutes session expiration
#define SHORT_LENGTH_MAX		60	// Max size for strings maximum
#define SHORT_LENGTH			40	// Max size for strings
#define SHORT_LENGTH_MIN		30	// Max size for strings minimum
#define BAN_TIME_SECS			900	// MORPH 15 minutes ban time 
#define LOGIN_TRIES_LIMIT		4	// MORPH max number of tries to login before ban

#include <vector>

typedef struct
{
	double download;
	double upload;
	long connections;
} UpDown;

typedef struct
{
	CTime	startTime;
	uint64	lSession;
	int		lastcat;
	//>>> [ionix] - iONiX::Advanced WebInterface Account Management
	/*
	bool	admin;
	*/
	uint8 admin;
	std::vector<int>	RightsToCategories;
	bool RightsToKad;
	bool RightsToPrefs;
	bool RightsToSearch;
	bool RightsToServers;
	bool RightsToSharedList;
	bool RightsToStats;
	bool RightsToTransfered;
	bool RightsToDownloadFiles;
	CString username;
//<<< [ionix] - iONiX::Advanced WebInterface Account Management
} Session;

struct BadLogin {
	CString ip;  // MORPH badlogin fix by dreamwalker 
	uint32	tries;// MORPH badlogin fix by dreamwalker 
	uint32	datalen;
	DWORD	timestamp;
};

typedef struct
{
	CString	sFileName;
	CString sFileNameJS;
	CString	sFileType;
	CString	sFileState;
	int		iFileState;
	CString sCategory;
	uint64	m_qwFileSize;
	uint64	m_qwFileTransferred;
	long	lFileSpeed;
	long	lSourceCount;
	long	lNotCurrentSourceCount;
	long	lTransferringSourceCount;
	double	m_dblCompleted;
	int		nFileStatus;
	int		nFilePrio;
	CString	sFileHash;
	CString	sED2kLink;
	CString	sFileInfo;
	bool	bFileAutoPrio;
	bool	bIsComplete;
	bool	bIsPreview;
	bool	bIsGetFLC;
	int		iComment;
} DownloadFiles;

typedef struct
{
	bool	bIsPartFile;
	CString	sFileState;
	CString	sFileName;
	CString sFileType;
	uint64	m_qwFileSize;
	uint64	nFileTransferred;
	uint64	nFileAllTimeTransferred;
	UINT	nFileRequests;
	uint32	nFileAllTimeRequests;
	UINT	nFileAccepts;
	uint32	nFileAllTimeAccepts;
	CString sFileCompletes;
	double	dblFileCompletes;
	byte	nFilePriority;
	CString sFilePriority;
	bool	bFileAutoPriority;
	CString sFileHash;
	CString	sED2kLink;
} SharedFiles;

typedef struct
{
	CString	sClientState;
	CString	sUserHash;
	CString	sActive;
	CString sFileInfo;
	CString sClientSoft;
	CString	sClientExtra;
	CString	sUserName;
	CString	sFileName;
	uint32	nTransferredDown;
	uint32	nTransferredUp;
	int		nDataRate;
	CString	sClientNameVersion;
} UploadUsers;

typedef struct
{
	CString	sClientState;
	CString	sClientStateSpecial;
	CString	sUserHash;
	CString sClientSoft;
	CString sClientSoftSpecial;
	CString	sClientExtra;
	CString	sUserName;
	CString	sFileName;
	CString	sClientNameVersion;
	uint32	nScore;
	CString sIndex;	//SyruS CQArray-Sorting element
} QueueUsers;

typedef enum
{
	DOWN_SORT_STATE,
	DOWN_SORT_TYPE,
	DOWN_SORT_NAME,
	DOWN_SORT_SIZE,
	DOWN_SORT_TRANSFERRED,
	DOWN_SORT_SPEED,
	DOWN_SORT_PROGRESS,
	DOWN_SORT_SOURCES,
	DOWN_SORT_PRIORITY,
	DOWN_SORT_CATEGORY,
	DOWN_SORT_FAKECHECK
} DownloadSort;

typedef enum
{
	UP_SORT_CLIENT,
	UP_SORT_USER,
	UP_SORT_VERSION,
	UP_SORT_FILENAME,
	UP_SORT_TRANSFERRED,
	UP_SORT_SPEED
} UploadSort;

typedef enum
{
	QU_SORT_CLIENT,
	QU_SORT_USER,
	QU_SORT_VERSION,
	QU_SORT_FILENAME,
	QU_SORT_SCORE
} QueueSort;

typedef enum
{
	SHARED_SORT_STATE,
	SHARED_SORT_TYPE,
	SHARED_SORT_NAME,
	SHARED_SORT_SIZE,
	SHARED_SORT_TRANSFERRED,
	SHARED_SORT_ALL_TIME_TRANSFERRED,
	SHARED_SORT_REQUESTS,
	SHARED_SORT_ALL_TIME_REQUESTS,
	SHARED_SORT_ACCEPTS,
	SHARED_SORT_ALL_TIME_ACCEPTS,
	SHARED_SORT_COMPLETES,
	SHARED_SORT_PRIORITY
} SharedSort;

typedef struct
{
	CString	sServerName;
	CString	sServerDescription;
	int		nServerPort;
	CString	sServerIP;
	CString sServerFullIP;
	CString sServerState;
	bool    bServerStatic;
	uint32		nServerUsers;
	uint32		nServerMaxUsers;
	uint32		nServerFiles;
	CString sServerPriority;
	byte   nServerPriority;
	int		nServerPing;
	int		nServerFailed;
	uint32		nServerSoftLimit;
	uint32		nServerHardLimit;
	CString	sServerVersion;
} ServerEntry;

typedef enum
{
	SERVER_SORT_STATE,
	SERVER_SORT_NAME,
	SERVER_SORT_IP,
	SERVER_SORT_DESCRIPTION,
	SERVER_SORT_PING,
	SERVER_SORT_USERS,
	SERVER_SORT_FILES,
	SERVER_SORT_PRIORITY,
	SERVER_SORT_FAILED,
	SERVER_SORT_LIMIT,
	SERVER_SORT_VERSION
} ServerSort;

typedef struct
{
	uint32			nUsers;
	DownloadSort	DownloadSort;
	bool			bDownloadSortReverse;
	UploadSort		UploadSort;
	bool			bUploadSortReverse;
	QueueSort		QueueSort;
	bool			bQueueSortReverse;
	ServerSort		ServerSort;
	bool			bServerSortReverse;
	SharedSort		SharedSort;
	bool			bSharedSortReverse;	
	bool			bShowUploadQueue;
	bool			bShowUploadQueueBanned;
	bool			bShowUploadQueueFriend;
	bool			bShowTransferLine;//Purity: Action Buttons
	CString			sShowTransferFile;//Purity: Action Buttons
	bool			bShowServerLine;//Purity: Action Buttons
	CString			sShowServerIP;//Purity: Action Buttons
	bool			bShowSharedLine;//Purity: Action Buttons
	CString			sShowSharedFile;//Purity: Action Buttons

	CArray<UpDown>		PointsForWeb;
	CArray<Session, Session>	Sessions;

	CString			sLastModified;
	CString			sETag;

	CArray<BadLogin>	badlogins;	//TransferredData= IP : time

} GlobalParams;
