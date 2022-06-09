//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include <locale.h>
#include <share.h>
#include <io.h>
#include "Preferences.h"
#include "opcodes.h"
#include "otherfunctions.h"
#include "ini2.h"
#include "resource.h"
#ifndef NEW_SOCKETS_ENGINE
	#include "emule.h"
#else
	#include "MuleListCtrl.h"
#endif //NEW_SOCKETS_ENGINE
#include "SharedFileList.h"
#include "ServerList.h"
#include "MD5Sum.h"
#include "Category.h"

// For SecuredVars
const DWORD CCriticalSection_INL2::s_dwSafeValue = 0x159D37BF;


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

bool CPreferences::bWinVerAlreadyDetected = false;

static const TCHAR *s_apcShortcutNames[] =
{
	_T("GeneralMinimizeWindowToTrayShortcut"),		//SCUT_WIN_MINIMIZE
	_T("GeneralSwitchWindowShortcut"),				//SCUT_WIN_SWITCH
	_T("GeneralShowServerWndShortcut"),				//SCUT_WIN_SRV
	_T("GeneralShowTransferWndShortcut"),			//SCUT_WIN_TRANSFER
	_T("GeneralShowSearchWndShortcut"),				//SCUT_WIN_SEARCH
	_T("GeneralShowSharedFilesWndShortcut"),		//SCUT_WIN_SHAREDFILES
	_T("GeneralShowChatWndShortcut"),				//SCUT_WIN_CHAT
	_T("GeneralShowIRCWndShortcut"),				//SCUT_WIN_IRC
	_T("GeneralShowStatisticsWndShortcut"),			//SCUT_WIN_STATS
	_T("GeneralShowPreferencesWndShortcut"),		//SCUT_WIN_PREFS
	_T("DownloadCancelShortcut"),					//SCUT_DL_CANCEL
	_T("DownloadStopShortcut"),						//SCUT_DL_STOP
	_T("DownloadPauseShortcut"),					//SCUT_DL_PAUSE
	_T("DownloadResumeShortcut"),					//SCUT_DL_RESUME
	_T("DownloadOpenShortcut"),						//SCUT_FILE_OPEN
	_T("DownloadOpenFolderShortcut"),				//SCUT_FILE_OPENDIR
	_T("DownloadPreviewShortcut"),					//SCUT_DL_PREVIEW
	_T("DownloadRenameShortcut"),					//SCUT_FILE_RENAME
	_T("DownloadViewFileCommentsShortcut"),			//SCUT_FILE_COMMENTS
	_T("DownloadViewFileDetailsShortcut"),			//SCUT_FILE_DETAILS
	_T("DownloadViewFileDetailsSourcesShortcut"),	//SCUT_DL_FD_SOURCES
	_T("DownloadClearCompletedFilesShortcut"),		//SCUT_DL_CLEAR
	_T("DownloadClearAllCompletedFilesShortcut"),	//SCUT_DL_CLEARALL
	_T("DownloadShowAllUploadsShortcut"),			//SCUT_DL_SHOWALL
	_T("DownloadUseDefaultSortShortcut"),			//SCUT_DL_DEFSORT
	_T("DownloadPreallocateShortcut"),				//SCUT_DL_PREALLOC
	_T("DownloadA4AFShortcut"),						//SCUT_DL_A4AF
	_T("DownloadA4AFAutoShortcut"),					//SCUT_DL_A4AFAUTO
	_T("DownloadA4AFOtherShortcut"),				//SCUT_DL_A4AFOTHER
	_T("DownloadA4AFSameCatShortcut"),				//SCUT_DL_A4AFSAMECAT
	_T("DownloadED2KLinkShortcut"),					//SCUT_LINK
	_T("DownloadED2kLinkHtmlShortcut"),				//SCUT_LINK_HTML
	_T("DownloadED2kLinkSourceShortcut"),			//SCUT_LINK_SOURCE
	_T("UploadViewClientDetailsShortcut"),			//SCUT_SRC_DETAILS
	_T("UploadAddClientToFriendsShortcut"),			//SCUT_SRC_FRIEND
	_T("UploadSendMessageClientShortcut"),			//SCUT_SRC_MSG
	_T("UploadViewClientFilesShortcut"),			//SCUT_SRC_SHAREDFILES
	_T("ShareEditFileCommentsShortcut"),			//SCUT_FILE_EDITCOMMENTS
	_T("CopyHashShortcut"),							//SCUT_LINK_HASH
	_T("FilenameCleanupShortcut")					//SCUT_FILE_NAMECLEANUP
};

const StructLanguage g_aLangs[] =
{
	{MAKELANGID(LANG_BASQUE, SUBLANG_DEFAULT), 1252},					// Basque
	{MAKELANGID(LANG_BELARUSIAN, SUBLANG_DEFAULT), 1251},				// Belarusian
	{MAKELANGID(LANG_CATALAN, SUBLANG_DEFAULT), 1252},					// Catalan
	{MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), 936},		// Chinese (simplified)
	{MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL), 950},		// Chinese (traditional)
	{MAKELANGID(LANG_CROATIAN, SUBLANG_DEFAULT), 1250},					// Croatian
	{MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT), 1250},					// Czech
	{MAKELANGID(LANG_DANISH, SUBLANG_DEFAULT), 1252},					// Danish
	{MAKELANGID(LANG_DUTCH, SUBLANG_DEFAULT), 1252},					// Dutch
	{MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), 1252},					// English
	{MAKELANGID(LANG_SPECIAL, SUBLNG_EXTREMADURAN), 1252},				// Extremaduran
	{MAKELANGID(LANG_FINNISH, SUBLANG_DEFAULT), 1252},					// Finnish
	{MAKELANGID(LANG_FRENCH, SUBLANG_DEFAULT), 1252},					// French
	{MAKELANGID(LANG_GERMAN, SUBLANG_DEFAULT), 1252},					// German
	{MAKELANGID(LANG_GREEK, SUBLANG_DEFAULT), 1253},					// Greek
	{MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT), 1255},					// Hebrew
//	{MAKELANGID(LANG_HUNGARIAN, SUBLANG_DEFAULT), 1250},				// Hungarian
	{MAKELANGID(LANG_ITALIAN, SUBLANG_DEFAULT), 1252},					// Italian
	{MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), 949},					// Korean
	{MAKELANGID(LANG_LITHUANIAN, SUBLANG_DEFAULT), 1257},				// Lithuanian
	{MAKELANGID(LANG_MALAY, SUBLANG_MALAY_MALAYSIA), 1252},				// Malay
	{MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL), 1252},		// Norwegian (Bokmal)
	{MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT), 1250},					// Polish
	{MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE), 1252},			// Portuguese (Portugal)
	{MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN), 1252},	// Portuguese (Brazilian)
	{MAKELANGID(LANG_ROMANIAN, SUBLANG_DEFAULT), 1250},					// Romanian
	{MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT), 1251},					// Russian
	{MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN), 1250},			// Serbian
	{MAKELANGID(LANG_SLOVENIAN, SUBLANG_DEFAULT), 1250},				// Slovenian
	{MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH), 1252},					// Spanish
	{MAKELANGID(LANG_TURKISH, SUBLANG_DEFAULT), 1254},					// Turkish
	{MAKELANGID(LANG_UKRAINIAN, SUBLANG_DEFAULT), 1251},				// Ukrainian
	{0, 0}
};

// Current extras for user lists
#define EP_PREFUPD_COMMENT_FILTER		_T("fastest eMule ever")
#define EP_PREFUPD_FILENAME_CLEANUP		_T("sharelita|emule-island.com")

//	UpdateUserStrList updates current user string lists with new value
//	This method is used to propagate new values to current installations
//	Update value should be kept for 1-2 releases (better one), than replaced with
//	empty string or new update
//	Update value is preserved in the configuration file to identify
//	that update took place, as we allow users to modify their settings
//	if they want to change a list in different way
static void UpdateUserStrList(CString *pstrVal, CString *pstrCurUpdVal, const TCHAR *pcUpdVal)
{
//	Check that update value from ini is different with current update value
	if (pstrCurUpdVal->Compare(pcUpdVal) != 0)
	{
		*pstrCurUpdVal = pcUpdVal;
		if (pstrCurUpdVal->IsEmpty())
			return;

		CString	strToken, strTokUpd;
		int		iPos, iPosUpd;

		for (iPosUpd = 0;;)
		{
			strTokUpd = pstrCurUpdVal->Tokenize(_T("|"), iPosUpd);
			if (strTokUpd.IsEmpty())
				break;
			for (iPos = 0;;)
			{
				strToken = pstrVal->Tokenize(_T("|"), iPos);
				if (strToken.IsEmpty())
				{
				//	Not present, extend the current list with new one
					if (!pstrVal->IsEmpty())
						*pstrVal += _T('|');
					*pstrVal += strTokUpd;
					break;
				}
				if (strToken.CompareNoCase(strTokUpd) == 0)
					break;	// string is present, no need to update
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPreferences::CPreferences()
{
	srand((unsigned)time(NULL));	//	Seed the random number generator
	m_bIsNTBased = false;

//	Get application start directory
	TCHAR path[MAX_PATH];

	::GetModuleFileName(NULL, path, MAX_PATH);

	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];

	_tsplitpath(path, drive, dir, NULL, NULL);

	m_strAppDir = CString(drive) + CString(dir);
	m_strConfigDir = m_strAppDir + _T("Config\\");

	::CreateDirectory(GetConfigDir(), 0);

#if 1	//code left for smooth migration, delete in v1.2f
	// This file was moved (in 1.2c) to the AppDir for compatibility reasons
	// Delete old for a while as some signature application searches first in config dir
	::DeleteFile(m_strConfigDir + _T("onlinesig.dat"));
#endif
	m_strDownloadLogFilePath.Format(_T("%sdownload.log"), m_strAppDir);
#if 1 //code left for smooth migration, delete in v1.3
	if (GetTextFileFormat(m_strDownloadLogFilePath) == tffANSI)
	{
		CString strBackupFile(m_strDownloadLogFilePath);

		strBackupFile += _T(".ansi");

		_tremove(strBackupFile);
		_trename(m_strDownloadLogFilePath, strBackupFile);
	}
#endif

//	Load preferences from "preferences.ini" file
	LoadIniPreferences();

	m_nSmartIdState = 0;
	LoadDatPreferences();

//	Load filter words from the file
	LoadFilterFile();

//	Mark as emule client
	m_userHash[5] = 14;
	m_userHash[14] = 111;

	LoadSharedDirs();
	LoadTempDirs();

	LoadServerlistAddresses();

	m_bUseProxyListenPort = false;
	m_nListenPort = 0;

//	Make sure the Incoming and primary Temp dirs exist
	CString	strTemp;

	CreateAllDirectories(m_sIncomingDir.Get(&strTemp));
	CreateAllDirectories(m_sTempDir.Get(&strTemp));

	m_pNameTag = NULL;
	SetUserNickTag();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPreferences::~CPreferences()
{
#ifndef NEW_SOCKETS_ENGINE
	CCat::Finalize();

#endif //NEW_SOCKETS_ENGINE
	free(m_pNameTag);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadServerlistAddresses() loads the server addresses from "config/addresses.dat" into 'm_addressesList'.
void CPreferences::LoadServerlistAddresses()
{
//	First we must free list
	m_addressesList.RemoveAll();

	CString		strTmp = m_strConfigDir;
	CStdioFile	addressesFile;

	strTmp += _T("addresses.dat");
	if (addressesFile.Open(strTmp, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
	{
		while (addressesFile.ReadString(strTmp))
		{
		//	Allow commented-out entries in the addresses.dat file.
			strTmp.TrimLeft();
			if (!strTmp.IsEmpty() && strTmp[0] != '#')
				m_addressesList.AddHead(strTmp);
		}
		addressesFile.Close();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SetStandardValues() sets default values for the legacy preference info.
void CPreferences::SetStandardValues()
{
//	Read the user hash from "config/userhash.dat".
	CString		strFullPath = m_strConfigDir;

	strFullPath += _T("userhash.dat");

	FILE	*pHashFile = _tfsopen(strFullPath, _T("rb"), _SH_DENYWR);

	if (pHashFile != NULL)
	{
		if (!fread(m_userHash, sizeof(m_userHash), 1, pHashFile))
			CreateUserHash();
		fclose(pHashFile);
	}
	else
		CreateUserHash();

	WINDOWPLACEMENT		defaultWPM;

	defaultWPM.length = sizeof(WINDOWPLACEMENT);
	defaultWPM.rcNormalPosition.left = 10;
	defaultWPM.rcNormalPosition.top = 10;
	defaultWPM.rcNormalPosition.right = 700;
	defaultWPM.rcNormalPosition.bottom = 500;
	defaultWPM.showCmd = 0;
	m_WindowPlacement = defaultWPM;

	Save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SetMaxDownloadWithCheck() sets max download limit checking the possible limits.
uint32 CPreferences::SetMaxDownloadWithCheck(uint32 dwMaxDownload)
{
	if (dwMaxDownload < 10)
		dwMaxDownload = 10;
	if(dwMaxDownload > m_dwMaxGraphDownloadRate)
		dwMaxDownload = m_dwMaxGraphDownloadRate;

	m_dwMaxDownload = dwMaxDownload;
	return dwMaxDownload;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SetMaxUploadWithCheck() sets max upload limit checking the possible limits.
uint32 CPreferences::SetMaxUploadWithCheck(uint32 dwMaxUpload)
{
	if (dwMaxUpload < 10)
		dwMaxUpload = 10;
	if (dwMaxUpload > m_dwMaxGraphUploadRate)
		dwMaxUpload = m_dwMaxGraphUploadRate;

	m_dwMaxUpload = dwMaxUpload;
	return dwMaxUpload;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetMaxDownload() handles download rate limiting based on the max upload rate setting.
uint16 CPreferences::GetMaxDownload()
{
//	Don't be a Lam3r :)
	uint32		dwMaxUp = GetMaxUpload();
	uint32		dwMaxDown;

//	max-up...
//	0..4:	if max-up < 1/3 max-down, max-down = 3 * max-up. Otherwise unchanged
//	4..9:	if max-up < 1/4 max-down, max-down = 4 * max-up. Otherwise unchanged
//	10..:	Unchanged (or unlimited if the "limitless download" preference is set)
	if (LimitlessDownload())
	{
		if (dwMaxUp >= 100)
			return UNLIMITED;
		else
			dwMaxDown = m_dwMaxGraphDownloadRate;
	}
	else
		dwMaxDown = m_dwMaxDownload;
	return static_cast<uint16>(TieUploadDownload(dwMaxUp, dwMaxDown));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Save() writes all the various preferences out to their appropriate files
bool CPreferences::Save()
{
	Preferences_Ext_Struct	prefsExt;
	bool	bError = false;
	CString	strFullPath = m_strConfigDir;

	strFullPath += _T("preferences.dat");

	FILE	*pPrefFile = _tfsopen(strFullPath, _T("wb"), _SH_DENYWR);

	if (pPrefFile != NULL)
	{
		prefsExt.version = PREFFILE_VERSION;
		prefsExt.EmuleWindowPlacement = m_WindowPlacement;
		md4cpy(prefsExt.abyteUserHash, m_userHash);

		bError = (fwrite(&prefsExt, sizeof(Preferences_Ext_Struct), 1, pPrefFile) == 0);
#ifndef NEW_SOCKETS_ENGINE
	//	Force OS to flush data direct to disk on shutdown
		if (!g_App.m_pMDlg->IsRunning())
			_commit(_fileno(pPrefFile));
#endif //NEW_SOCKETS_ENGINE
		fclose(pPrefFile);
	}
	else
		bError = true;

//	Saving HashID to "config/userhash.dat"
	strFullPath.Format(_T("%suserhash.dat"), m_strConfigDir);

	FILE	*pHashFile = _tfsopen(strFullPath, _T("wb"), _SH_DENYWR);

	if (pHashFile != NULL)
	{
		bError = (fwrite(m_userHash, sizeof(m_userHash), 1, pHashFile) == 0);
		fclose(pHashFile);
	}
	else
		bError = true;

	SaveIniPreferences();

	strFullPath.Format(_T("%sshareddir.dat"), m_strConfigDir);

	CStdioFile	sharedDirsFile;

	if (sharedDirsFile.Open(strFullPath, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite|CFile::typeBinary))
	{
#ifdef _UNICODE
		uint16	uBOM = 0xFEFF;

		sharedDirsFile.Write(&uBOM, sizeof(uBOM));
#endif
		CSingleLock	sLock(&m_csDirList, true);

		for (POSITION pos = m_sharedDirList.GetHeadPosition(); pos != NULL;)
		{
			sharedDirsFile.WriteString(m_sharedDirList.GetNext(pos));
			sharedDirsFile.Write(_T("\r\n"), 2 * sizeof(TCHAR));
		}
		sLock.Unlock();
		sharedDirsFile.Close();
	}
	else
		bError = true;

	strFullPath.Format(_T("%stempdir.dat"),m_strConfigDir);

	CStdioFile	tempDirsFile;

	if (tempDirsFile.Open(strFullPath, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite|CFile::typeBinary))
	{
#ifdef _UNICODE
		uint16	uBOM = 0xFEFF;

		tempDirsFile.Write(&uBOM, sizeof(uBOM));
#endif
		CSingleLock	sLock(&m_csDirList, true);

		for (POSITION pos = m_tempDirList.GetHeadPosition(); pos != NULL;)
		{
			tempDirsFile.WriteString(m_tempDirList.GetNext(pos).GetBuffer());
			tempDirsFile.Write(_T("\r\n"), 2 * sizeof(TCHAR));
		}
		sLock.Unlock();
		tempDirsFile.Close();
	}
	else
		bError = true;

	CreateAllDirectories(m_sIncomingDir.Get(&strFullPath));
	CreateAllDirectories(m_sTempDir.Get(&strFullPath));

	return bError;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CreateUserHash() generates a random 16 byte ID in 'm_userHash'.
void CPreferences::CreateUserHash()
{
	uint16 *pnHash = reinterpret_cast<uint16*>(m_userHash);

	for (int i = 0; i < 8; i++)
		pnHash[i] = static_cast<uint16>(rand() | ((rand() > (RAND_MAX / 2)) ? 0x8000 : 0));

//	Mark as emule client
	m_userHash[5] = 14;
	m_userHash[14] = 111;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPreferences::GetColumnWidth(EnumTable t, int index) const
{
	switch (t)
	{
		case TABLE_DOWNLOAD:
			return m_auDownloadColumnWidths[index];
		case TABLE_UPLOAD:
			return m_auUploadColumnWidths[index];
		case TABLE_QUEUE:
			return m_auQueueColumnWidths[index];
		case TABLE_SEARCH:
			return m_auSearchColumnWidths[index];
		case TABLE_SHARED:
			return m_auSharedColumnWidths[index];
		case TABLE_SERVER:
			return m_auServerColumnWidths[index];
		case TABLE_IRCNICK:
			return m_auIrcNickColumnWidths[index];
		case TABLE_IRC:
			return m_auIrcColumnWidths[index];
		case TABLE_CLIENTLIST:
			return m_auClientListColumnWidths[index];
		case TABLE_PARTSTATUS:
			return m_auPartStatusColumnWidths[index];
		case TABLE_FRIENDLIST:
			return m_auFriendColumnWidths[index];
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetColumnWidth(EnumTable t, int index, int width)
{
	switch (t)
	{
		case TABLE_DOWNLOAD:
			m_auDownloadColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_UPLOAD:
			m_auUploadColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_QUEUE:
			m_auQueueColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_SEARCH:
			m_auSearchColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_SHARED:
			m_auSharedColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_SERVER:
			m_auServerColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_IRCNICK:
			m_auIrcNickColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_IRC:
			m_auIrcColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_CLIENTLIST:
			m_auClientListColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_PARTSTATUS:
			m_auPartStatusColumnWidths[index] = static_cast<uint16>(width);
			break;
		case TABLE_FRIENDLIST:
			m_auFriendColumnWidths[index] = static_cast<uint16>(width);
			break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPreferences::GetColumnHidden(EnumTable t, int index) const
{
	switch (t)
	{
		case TABLE_DOWNLOAD:
			return m_abDownloadColumnHidden[index];
		case TABLE_UPLOAD:
			return m_abUploadColumnHidden[index];
		case TABLE_QUEUE:
			return m_abQueueColumnHidden[index];
		case TABLE_SEARCH:
			return m_abSearchColumnHidden[index];
		case TABLE_SHARED:
			return m_abSharedColumnHidden[index];
		case TABLE_SERVER:
			return m_abServerColumnHidden[index];
		case TABLE_IRCNICK:
			return m_abIrcNickColumnHidden[index];
		case TABLE_IRC:
			return m_abIrcColumnHidden[index];
		case TABLE_CLIENTLIST:
			return m_abClientListColumnHidden[index];
		case TABLE_PARTSTATUS:
			return m_abPartStatusColumnHidden[index];
		case TABLE_FRIENDLIST:
			return m_abFriendColumnHidden[index];
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetColumnHidden(EnumTable t, int index, bool bHidden)
{
	switch (t)
	{
		case TABLE_DOWNLOAD:
			m_abDownloadColumnHidden[index] = bHidden;
			break;
		case TABLE_UPLOAD:
			m_abUploadColumnHidden[index] = bHidden;
			break;
		case TABLE_QUEUE:
			m_abQueueColumnHidden[index] = bHidden;
			break;
		case TABLE_SEARCH:
			m_abSearchColumnHidden[index] = bHidden;
			break;
		case TABLE_SHARED:
			m_abSharedColumnHidden[index] = bHidden;
			break;
		case TABLE_SERVER:
			m_abServerColumnHidden[index] = bHidden;
			break;
		case TABLE_IRCNICK:
			m_abIrcNickColumnHidden[index] = bHidden;
			break;
		case TABLE_IRC:
			m_abIrcColumnHidden[index] = bHidden;
			break;
		case TABLE_CLIENTLIST:
			m_abClientListColumnHidden[index] = bHidden;
			break;
		case TABLE_PARTSTATUS:
			m_abPartStatusColumnHidden[index] = bHidden;
			break;
		case TABLE_FRIENDLIST:
			m_abFriendColumnHidden[index] = bHidden;
			break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPreferences::GetColumnOrder(EnumTable t, int index) const
{
	switch (t)
	{
		case TABLE_DOWNLOAD:
			return m_aiDownloadColumnOrder[index];
		case TABLE_UPLOAD:
			return m_aiUploadColumnOrder[index];
		case TABLE_QUEUE:
			return m_aiQueueColumnOrder[index];
		case TABLE_SEARCH:
			return m_aiSearchColumnOrder[index];
		case TABLE_SHARED:
			return m_aiSharedColumnOrder[index];
		case TABLE_SERVER:
			return m_aiServerColumnOrder[index];
		case TABLE_IRCNICK:
			return m_aiIrcNickColumnOrder[index];
		case TABLE_IRC:
			return m_aiIrcColumnOrder[index];
		case TABLE_CLIENTLIST:
			return m_aiClientListColumnOrder[index];
		case TABLE_PARTSTATUS:
			return m_aiPartStatusColumnOrder[index];
		case TABLE_FRIENDLIST:
			return m_aiFriendColumnOrder[index];
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetColumnOrder(EnumTable t, int *piOrder)
{
	switch (t)
	{
		case TABLE_DOWNLOAD:
			memcpy2(m_aiDownloadColumnOrder, piOrder, sizeof(m_aiDownloadColumnOrder));
			break;
		case TABLE_UPLOAD:
			memcpy2(m_aiUploadColumnOrder, piOrder, sizeof(m_aiUploadColumnOrder));
			break;
		case TABLE_QUEUE:
			memcpy2(m_aiQueueColumnOrder, piOrder, sizeof(m_aiQueueColumnOrder));
			break;
		case TABLE_SEARCH:
			memcpy2(m_aiSearchColumnOrder, piOrder, sizeof(m_aiSearchColumnOrder));
			break;
		case TABLE_SHARED:
			memcpy2(m_aiSharedColumnOrder, piOrder, sizeof(m_aiSharedColumnOrder));
			break;
		case TABLE_SERVER:
			memcpy2(m_aiServerColumnOrder, piOrder, sizeof(m_aiServerColumnOrder));
			break;
		case TABLE_IRCNICK:
			memcpy2(m_aiIrcNickColumnOrder, piOrder, sizeof(m_aiIrcNickColumnOrder));
			break;
		case TABLE_IRC:
			memcpy2(m_aiIrcColumnOrder, piOrder, sizeof(m_aiIrcColumnOrder));
			break;
		case TABLE_CLIENTLIST:
			memcpy2(m_aiClientListColumnOrder, piOrder, sizeof(m_aiClientListColumnOrder));
			break;
		case TABLE_PARTSTATUS:
			memcpy2(m_aiPartStatusColumnOrder, piOrder, sizeof(m_aiPartStatusColumnOrder));
			break;
		case TABLE_FRIENDLIST:
			memcpy2(m_aiFriendColumnOrder, piOrder, sizeof(m_aiFriendColumnOrder));
			break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetRecommendedMaxConnections() calculates a recommended connection limit based on the maximum
//		supported by the OS.
int CPreferences::GetRecommendedMaxConnections()
{
//	Get an estimate of the maximum number of connections the OS will support
	int iRealMax = ::GetMaxConnections();

//	If limit is out of range, recommend 500
	if(iRealMax == -1 || iRealMax > 520)
		return 500;

//	If limit is between 0 and 19, recommend it
	if(iRealMax < 20)
		return iRealMax;

//	If limit is between 20 and 256, recommend it be reduced by 10
	if(iRealMax <= 256)
		return iRealMax - 10;

//	If limit is between 257 and 520, recommend it be reduced by 20
	return iRealMax - 20;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SaveIniPreferences()
{
	EMULE_TRY

	CString		strBuf;

	strBuf.Format(_T("%spreferences.ini"), m_strConfigDir);

	CIni		ini(strBuf);

	ini.SetDefaultCategory(_T("eMule"));

	ini.SetString(_T("AppVersion"), CURRENT_VERSION_LONG);
	ini.SetString(_T("Nick"), *m_sNick.Get(&strBuf));
	ini.SetString(_T("IncomingDir"), *m_sIncomingDir.Get(&strBuf));
	ini.SetString(_T("TempDir"), *m_sTempDir.Get(&strBuf));
	ini.SetString(_T("VideoPlayer"), m_strVideoPlayer);
	ini.SetString(_T("VideoPlayerArgs"), m_strVideoPlayerArgs);

	FractionalRate2String(&strBuf, m_dwMaxUpload);
	ini.SetString(_T("MaxUpload"), strBuf);
	FractionalRate2String(&strBuf, m_dwMaxDownload);
	ini.SetString(_T("MaxDownload"), strBuf);
	ini.SetInt(_T("MaxConnections"), m_nMaxConnections);
	ini.SetBool(_T("DeadServerAction"), m_bRemoveDeadServers);
	ini.SetInt(_T("Port"), m_uPort);
	ini.SetInt(_T("UDPPort"), m_uUDPPort);
	ini.SetBool(_T("OpenXPFirewallPorts"),m_bOpenPorts);
	ini.SetBool(_T("UseServerAuxPort"), m_bUseServerAuxPort);
	ini.SetInt(_T("MaxSourcesPerFile"), m_dwMaxSourcePerFile);
	ini.SetWORD(_T("Language"), m_nLanguageID);
	ini.SetInt(_T("SeeShare"), m_nSeeShares);
	ini.SetInt(_T("FilePermission"), m_byteFilePermission);

	ini.SetBool(_T("PartTrafficEnabled"), m_bUsePartTraffic);
	ini.SetInt(_T("UploadbarStyle"), m_byteUpbarPartTraffic);
	ini.SetInt(_T("UploadbarColor"), m_byteUpbarColorPartTraffic);

	ini.SetInt(_T("ToolTipDelay"), m_dwTooltipDelay);
	ini.SetInt(_T("QueueSize"), m_dwQueueSize);
	ini.SetInt(_T("StatGraphsInterval"), m_nTrafficOMeterInterval);
	ini.SetInt(_T("StatsInterval"), m_nStatsInterval);
	ini.SetInt(_T("DetailColumnWidth"), m_iDetailColumnWidth);
	ini.SetInt(_T("DownloadCapacity"), m_dwMaxGraphDownloadRate / 10);
	ini.SetInt(_T("UploadCapacity"), m_dwMaxGraphUploadRate / 10);
#if 1	//code left for smooth migration, delete in v1.2f
	ini.SetInt(_T("DeadServerRetries"), m_dwDeadServerRetries);
#endif
	ini.SetInt(_T("DeadServerRetry"), m_dwDeadServerRetries);
	ini.SetInt(_T("VariousStatisticsMaxValue"), m_nStatsMax);
	ini.SetInt(_T("StatsAverageMinutes"), m_nStatsAverageMinutes);
	ini.SetInt(_T("MaxConnectionsPerFiveSeconds"), m_nMaxConPerFive);

	ini.SetInt(_T("BadClientBanTime"), m_dwBadClientBanTime);
	ini.SetInt(_T("BadClientMinRequestTime"), m_dwMinRequestTime);
	ini.SetInt(_T("BadClientMinRequestNum"), m_byteBadClientBan);
	ini.SetBool(_T("BanMessageEnabled"), m_bBanMessageEnabled);
	ini.SetBool(_T("BanEnabled"), m_bBanEnabled);

	ini.SetInt(_T("SLS_WhenSourcesOutdated"), m_dwSLSOutdated);
	ini.SetInt(_T("SLS_MaxSourcesPerFile"), m_dwSLSMaxSrcPerFile);
	ini.SetBool(_T("SLS_Enable"), m_bSLSEnable);

	ini.SetBool(_T("Reconnect"), m_bReconnect);
	ini.SetBool(_T("Scoresystem"), m_bUseServerPriorities);

	ini.SetBool(_T("UAP"), m_bUAP);
	ini.SetBool(_T("DAP"), m_bDAP);
	ini.SetBool(_T("DisableXS"), m_bDisableXS);
	ini.SetBool(_T("StartDownloadPaused"), m_bStartDownloadPaused);
	ini.SetBool(_T("DownloadPausedOnComplete"), m_bDownloadPausedOnComplete);
	ini.SetBool(_T("ResumeOtherCat"), m_bResumeOtherCat);
	ini.SetBool(_T("AutoClearCompleted"), m_bAutoClearCompleted);
	ini.SetBool(_T("AutoSourcesEnabled"), m_bAutoSourcesEnabled);
	ini.SetInt(_T("MinAutoSourcesPerFile"), m_nMinAutoSourcesPerFile);
	ini.SetInt(_T("MaxAutoSourcesPerFile"), m_nMaxAutoSourcesPerFile);
	ini.SetInt(_T("MaxAutoSourcesTotal"), m_nMaxAutoSourcesTotal);
	ini.SetInt(_T("MaxAutoExchangeSources"), m_nMaxAutoExchangeSources);
	ini.SetBool(_T("AutoSourcesLogEnabled"), m_bAutoSourcesLogEnabled);
	ini.SetBool(_T("ClientTransferLogEnabled"), m_bClientTransferLogEnabled);

	ini.SetString(_T("ToolbarSettings"), *m_sToolbarSettings.Get(&strBuf));
	ini.SetString(_T("ToolbarBitmap"), *m_sToolbarBitmap.Get(&strBuf));
	ini.SetString(_T("ToolbarBitmapFolder"), *m_sToolbarBitmapFolder.Get(&strBuf));
	ini.SetInt(_T("ToolbarLabels"), m_iToolbarLabels);
	ini.SetBool(_T("Serverlist"), m_bAutoServerList);
	ini.SetBool(_T("UpdateNotify"), m_bUpdateNotify);
	ini.SetBool(_T("MinimizeToTray"), m_bMinToTray);
	ini.SetBool(_T("AddServersFromServer"), m_bAddServersFromServer);
	ini.SetBool(_T("AddServersFromClient"), m_bAddServersFromClients);
	ini.SetBool(_T("Splashscreen"), m_bSplashScreen);
	ini.SetBool(_T("BringToFront"), m_bBringToForeground);
	ini.SetBool(_T("BeepOnError"), m_bBeepOnError);
	ini.SetBool(_T("ConfirmExit"), m_bConfirmExit);
	ini.SetBool(_T("ConfirmDisconnect"), m_bConfirmDisconnect);
	ini.SetBool(_T("ConfirmFriendDel"), m_bConfirmFriendDel);
	ini.SetBool(_T("FilterBadIPs"), m_bFilterBadIP);
	ini.SetBool(_T("FilterServersByIP"), m_bFilterServersByIP);
	ini.SetBool(_T("Autoconnect"), m_bAutoConnect);
	ini.SetBool(_T("OnlineSignature"), m_bOnlineSig);
	ini.SetBool(_T("StartupMinimized"), m_bStartMinimized);
	ini.SetBool(_T("ToolbarSpeedMeter"), m_bShowToolbarSpeedMeter);
	ini.SetBool(_T("CountryFlag"), m_bShowCountryFlag);
	ini.SetBool(_T("CloseToTray"), m_bCloseToTray);
	ini.SetBool(_T("DetailsOnClick"), m_bDetailsOnClick);
	ini.SetInt(_T("SrvConTimeout"), m_nSrvConTimeout);
	ini.SetInt(_T("SlowCompleteBlockSize"), m_dwSlowCompleteBlockSize);
	ini.SetBool(_T("ShowRatesOnTitle"), m_bShowRatesInTitle);

	ini.SetBool(_T("AutoConnectStaticOnly"), m_bAutoConnectStaticOnly);
	ini.SetInt(_T("3DDepth"), m_byteDepth3D);

	ini.SetBool(_T("NotifyOnDownload"), m_bUseDownloadNotifier);
	ini.SetBool(_T("NotifyOnDownloadAdd"), m_bUseDownloadAddNotifier);
	ini.SetBool(_T("NotifyOnChat"), m_bUseChatNotifier);
	ini.SetBool(_T("NotifyOnLog"), m_bUseLogNotifier);
	ini.SetBool(_T("NotifierUseSound"), m_bUseSoundInNotifier);
	ini.SetBool(_T("NotifierUseScheduler"), m_bUseSchedulerNotifier);
	ini.SetBool(_T("NotifierPopEveryChatMessage"), m_bNotifierPopsEveryChatMsg);
	ini.SetBool(_T("NotifyOnImportantError"), m_bNotifierImportantError);
	ini.SetBool(_T("NotifyOnWebServerError"), m_bNotifierWebServerError);
	ini.SetBool(_T("NotifyOnServerError"), m_bNotifierServerError);
	ini.SetString(_T("NotifierSoundPath"), *m_sNotifierSoundFilePath.Get(&strBuf));

	ini.SetString(_T("MessageFilter"), m_strMessageFilter);
	ini.SetString(_T("CommentFilter"), *m_sCommentFilter.Get(&strBuf));
	ini.SetString(_T("CommentFilterUpd"), m_strCommentFilterUpd);
	ini.SetString(_T("TxtEditor"), m_strTxtEditor);
	ini.SetBool(_T("ShowRatingIcons"), m_bShowRatingIcons);
	ini.SetBool(_T("SmartIdCheck"), m_bSmartIdCheck);

	ini.SetString(_T("IRCServer"), *m_sIRCServer.Get(&strBuf));
	ini.SetString(_T("IRCNick"), *m_sIRCNick.Get(&strBuf));
	ini.SetBool(_T("IRCAddTimestamp"), m_bIRCAddTimestamp);
	ini.SetString(_T("IRCFilterName"), *m_sIRCChanNameFilter.Get(&strBuf));
	ini.SetInt(_T("IRCFilterUser"), m_nIRCChannelUserFilter);
	ini.SetBool(_T("IRCUseFilter"), m_bIRCUseChanFilter);
	ini.SetString(_T("IRCPerformString"), *m_sIRCPerformString.Get(&strBuf));
	ini.SetBool(_T("IRCUsePerform"), m_bIRCUsePerform);
	ini.SetBool(_T("IRCListOnConnect"), m_bIRCListOnConnect);
	ini.SetBool(_T("IRCIgnoreInfoMessage"), m_bIRCIgnoreInfoMessage);
	ini.SetBool(_T("IRCMessageEncodingUTF8"), m_bIRCMessageEncodingUTF8);
	ini.SetBool(_T("IRCStripColor"), m_bIRCStripColor);

	ini.SetBool(_T("UseSortingDefaults"), m_bUseSort);
	ini.SetBool(_T("Use2ndSortSources"), m_bUseSrcSortCol2);
	ini.SetBool(_T("PausedStoppedLast"), m_bPausedStoppedLast);
	ini.SetUInt32(_T("SortServersCode"), m_uiServerSortCol);
	ini.SetUInt32(_T("SortUploadsCode"), m_uiUploadSortCol);
	ini.SetUInt32(_T("SortQueueCode"), m_uiQueueSortCol);
	ini.SetUInt32(_T("SortSearchCode"), m_uiSearchSortCol);
	ini.SetUInt32(_T("SortIrcCode"), m_uiIRCSortCol);
	ini.SetUInt32(_T("SortFilesCode"), m_uiFileSortCol);
	ini.SetUInt32(_T("SortDownloadsCode"), m_uiDownloadSortCol);
	ini.SetUInt32(_T("SortClientListCode"), m_uiClientListSortCol);
	ini.SetUInt32(_T("1stSortSourcesCode"), m_uiSrcSortCol1);
	ini.SetUInt32(_T("2ndSortSourcesCode"), m_uiSrcSortCol2);

	ini.SetBool(_T("CommunityEnabled"), m_bCommunityEnabled);
	ini.SetString(_T("CommunityString"), *m_sCommunityString.Get(&strBuf));
	ini.SetBool(_T("CommunityNoBan"), m_bCommunityNoBan);
	ini.SetInt(_T("NotificationDisplayTime"), m_nNotificationDisplayTime);
	ini.SetInt(_T("NotificationFontSize"), m_nNotificationFontSize);
	ini.SetInt(_T("Profile"), m_uiProfile);
	ini.SetInt(_T("PriorityHigh"), m_nPriorityHigh);
	ini.SetInt(_T("PriorityLow"), m_nPriorityLow);
	ini.SetBool(_T("RestartWaiting"), m_bRestartWaiting);
	ini.SetBool(_T("LogToFile"), m_bLogToFile);
	ini.SetBool(_T("LogUploadToFile"), m_bLogUploadToFile);
	ini.SetBool(_T("LogDownloadToFile"), m_bLogDownloadToFile);
	ini.SetBool(_T("BugReport"), m_bBugReport);
	ini.SetBool(_T("AllowLocalHostIP"), m_bAllowLocalHostIP);
	ini.SetBool(_T("DisableXSUpTo"), m_bDisableXSUpTo);
	ini.SetInt(_T("XSUpTo"), m_nXSUpTo);
	ini.SetString(_T("UsedFont"), *m_sUsedFont.Get(&strBuf));
	ini.SetInt(_T("FontSize"), m_nFontSize);
	ini.SetInt(_T("HashingPriority"), m_HashingPriority);
	ini.SetBool(_T("UploadParts"), m_bUploadParts);
	ini.SetBool(_T("A4AFStringEnabled"), m_bA4AFStringEnabled);
	ini.SetBool(_T("A4AFCountEnabled"), m_bA4AFCountEnabled);
	ini.SetInt(_T("SearchMethod"), m_dwSearchMethod);
	ini.SetBool(_T("KeepSearchHistory"), m_bKeepSearchHistory);
	ini.SetBool(_T("AutoTakeED2KLinks"), m_bAutoTakeLinks);
	ini.SetInt(_T("FileBufferSize"), m_dwFileBufferSize);
	ini.SetInt(_T("MainProcessPriority"), m_nMainProcessPriority);
	ini.SetBool(_T("ShowAverageDataRate"),m_bShowAverageDataRate);
	ini.SetBool(_T("LimitlessDownload"),m_bLimitlessDownload);
	ini.SetBool(_T("SCHEnabled"), m_bSCHEnabled);
	ini.SetBool(_T("SCHExceptMon"), m_bSCHExceptMon);
	ini.SetBool(_T("SCHExceptTue"), m_bSCHExceptTue);
	ini.SetBool(_T("SCHExceptWed"), m_bSCHExceptWed);
	ini.SetBool(_T("SCHExceptThu"), m_bSCHExceptThu);
	ini.SetBool(_T("SCHExceptFri"), m_bSCHExceptFri);
	ini.SetBool(_T("SCHExceptSat"), m_bSCHExceptSat);
	ini.SetBool(_T("SCHExceptSun"), m_bSCHExceptSun);
	ini.SetInt(_T("SCHShift1"),m_nSCHShift1);
	ini.SetInt(_T("SCHShift2"),m_nSCHShift2);
	FractionalRate2String(&strBuf, m_uSCHShift1Upload);
	ini.SetString(_T("SCHShift1Up"), strBuf);
	FractionalRate2String(&strBuf, m_uSCHShift1Download);
	ini.SetString(_T("SCHShift1Down"), strBuf);
	ini.SetInt(_T("SCHShift1Conn"),m_nSCHShift1conn);
	ini.SetInt(_T("SCHShift15sec"),m_nSCHShift15sec);
	FractionalRate2String(&strBuf, m_uSCHShift2Upload);
	ini.SetString(_T("SCHShift2Up"), strBuf);
	FractionalRate2String(&strBuf, m_uSCHShift2Download);
	ini.SetString(_T("SCHShift2Down"), strBuf);
	ini.SetInt(_T("SCHShift2Conn"),m_nSCHShift2conn);
	ini.SetInt(_T("SCHShift25sec"),m_nSCHShift25sec);
	ini.SetBool(_T("ShowFileTypeIcon"), m_bShowFileTypeIcon);

	ini.SetBool(_T("MultipleInstances"), m_bMultiple);

	ini.SetBool(_T("Verbose"), m_bVerbose);
	ini.SetBool(_T("ManualHighPrio"), m_bManuallyAddedServerHighPrio);
	ini.SetBool(_T("ShowOverhead"), m_bShowOverhead);
	ini.SetBool(_T("VideoPreviewBackupped"), m_bBackupPreview);	//Cax2 string kept for compatibility with off client

	ini.SetInt(_T("AllCatType"), CCat::GetAllCatType());

	ini.SetArray(m_auDownloadColumnWidths, ARRSIZE(m_auDownloadColumnWidths), _T("DownloadColumnWidths"));
	ini.SetArray(m_abDownloadColumnHidden, ARRSIZE(m_abDownloadColumnHidden), _T("DownloadColumnHidden"));
	ini.SetArray(m_aiDownloadColumnOrder, ARRSIZE(m_aiDownloadColumnOrder), _T("DownloadColumnOrder"));
	ini.SetArray(m_auUploadColumnWidths, ARRSIZE(m_auUploadColumnWidths), _T("UploadColumnWidths"));
	ini.SetArray(m_abUploadColumnHidden, ARRSIZE(m_abUploadColumnHidden), _T("UploadColumnHidden"));
	ini.SetArray(m_aiUploadColumnOrder, ARRSIZE(m_aiUploadColumnOrder), _T("UploadColumnOrder"));
	ini.SetArray(m_auQueueColumnWidths, ARRSIZE(m_auQueueColumnWidths), _T("QueueColumnWidths"));
	ini.SetArray(m_abQueueColumnHidden, ARRSIZE(m_abQueueColumnHidden), _T("QueueColumnHidden"));
	ini.SetArray(m_aiQueueColumnOrder, ARRSIZE(m_aiQueueColumnOrder), _T("QueueColumnOrder"));
	ini.SetArray(m_auSearchColumnWidths, ARRSIZE(m_auSearchColumnWidths), _T("SearchColumnWidths"));
	ini.SetArray(m_abSearchColumnHidden, ARRSIZE(m_abSearchColumnHidden), _T("SearchColumnHidden"));
	ini.SetArray(m_aiSearchColumnOrder, ARRSIZE(m_aiSearchColumnOrder), _T("SearchColumnOrder"));
	ini.SetArray(m_auSharedColumnWidths, ARRSIZE(m_auSharedColumnWidths), _T("SharedColumnWidths"));
	ini.SetArray(m_abSharedColumnHidden, ARRSIZE(m_abSharedColumnHidden), _T("SharedColumnHidden"));
	ini.SetArray(m_aiSharedColumnOrder, ARRSIZE(m_aiSharedColumnOrder), _T("SharedColumnOrder"));
	ini.SetArray(m_auServerColumnWidths, ARRSIZE(m_auServerColumnWidths), _T("ServerColumnWidths"));
	ini.SetArray(m_abServerColumnHidden, ARRSIZE(m_abServerColumnHidden), _T("ServerColumnHidden"));
	ini.SetArray(m_aiServerColumnOrder, ARRSIZE(m_aiServerColumnOrder), _T("ServerColumnOrder"));
	ini.SetArray(m_auIrcNickColumnWidths, ARRSIZE(m_auIrcNickColumnWidths), _T("IrcNickColumnWidths"));
	ini.SetArray(m_abIrcNickColumnHidden, ARRSIZE(m_abIrcNickColumnHidden), _T("IrcNickColumnHidden"));
	ini.SetArray(m_aiIrcNickColumnOrder, ARRSIZE(m_aiIrcNickColumnOrder), _T("IrcNickColumnOrder"));
	ini.SetArray(m_auIrcColumnWidths, ARRSIZE(m_auIrcColumnWidths), _T("IrcColumnWidths"));
	ini.SetArray(m_abIrcColumnHidden, ARRSIZE(m_abIrcColumnHidden), _T("IrcColumnHidden"));
	ini.SetArray(m_aiIrcColumnOrder, ARRSIZE(m_aiIrcColumnOrder), _T("IrcColumnOrder"));
	ini.SetArray(m_auClientListColumnWidths, ARRSIZE(m_auClientListColumnWidths), _T("ClientListColumnWidths"));
	ini.SetArray(m_abClientListColumnHidden, ARRSIZE(m_abClientListColumnHidden), _T("ClientListColumnHidden"));
	ini.SetArray(m_aiClientListColumnOrder, ARRSIZE(m_aiClientListColumnOrder), _T("ClientListColumnOrder"));
	ini.SetArray(m_auPartStatusColumnWidths, ARRSIZE(m_auPartStatusColumnWidths), _T("PartStatusColumnWidths"));
	ini.SetArray(m_abPartStatusColumnHidden, ARRSIZE(m_abPartStatusColumnHidden), _T("PartStatusColumnHidden"));
	ini.SetArray(m_aiPartStatusColumnOrder, ARRSIZE(m_aiPartStatusColumnOrder), _T("PartStatusColumnOrder"));
	ini.SetArray(m_auFriendColumnWidths, ARRSIZE(m_auFriendColumnWidths), _T("FriendColumnWidths"));
	ini.SetArray(m_abFriendColumnHidden, ARRSIZE(m_abFriendColumnHidden), _T("FriendColumnHidden"));
	ini.SetArray(m_aiFriendColumnOrder, ARRSIZE(m_aiFriendColumnOrder), _T("FriendColumnOrder"));

//	Provide a mechanism for all tables to store/retrieve sort order
	ini.SetInt(_T("TableSortItemDownload"), m_iTableSortItemDownload);
	ini.SetInt(_T("TableSortItemDownload2"), m_iTableSortItemDownload2);
	ini.SetInt(_T("TableSortItemUpload"), m_iTableSortItemUpload);
	ini.SetInt(_T("TableSortItemQueue"), m_iTableSortItemQueue);
	ini.SetInt(_T("TableSortItemSearch"), m_iTableSortItemSearch);
	ini.SetInt(_T("TableSortItemShared"), m_iTableSortItemShared);
	ini.SetInt(_T("TableSortItemServer"), m_iTableSortItemServer);
	ini.SetInt(_T("TableSortItemIRCNick"), m_iTableSortItemIRCNick);
	ini.SetInt(_T("TableSortItemIRCChannel"), m_iTableSortItemIRCChannel);
	ini.SetInt(_T("TableSortItemClientList"), m_iTableSortItemClientList);
	ini.SetInt(_T("TableSortItemPartStatus"), m_iTableSortItemPartStatus);
	ini.SetInt(_T("TableSortItemFriend"), m_iTableSortItemFriend);
	ini.SetBool(_T("TableSortAscendingDownload"), m_bTableSortAscendingDownload);
	ini.SetBool(_T("TableSortAscendingDownload2"), m_bTableSortAscendingDownload2);
	ini.SetBool(_T("TableSortAscendingUpload"), m_bTableSortAscendingUpload);
	ini.SetBool(_T("TableSortAscendingQueue"), m_bTableSortAscendingQueue);
	ini.SetBool(_T("TableSortAscendingSearch"), m_bTableSortAscendingSearch);
	ini.SetBool(_T("TableSortAscendingShared"), m_bTableSortAscendingShared);
	ini.SetBool(_T("TableSortAscendingServer"), m_bTableSortAscendingServer);
	ini.SetBool(_T("TableSortAscendingIRCNick"), m_bTableSortAscendingIRCNick);
	ini.SetBool(_T("TableSortAscendingIRCChannel"), m_bTableSortAscendingIRCChannel);
	ini.SetBool(_T("TableSortAscendingClientList"), m_bTableSortAscendingClientList);
	ini.SetBool(_T("TableSortAscendingPartStatus"), m_bTableSortAscendingPartStatus);
	ini.SetBool(_T("TableSortAscendingFriend"), m_bTableSortAscendingFriend);

	for (unsigned i = 0; i < ARRSIZE(m_dwStatColors); i++)
	{
		strBuf.Format(_T("StatColor%u"), i);
		ini.SetUInt32(strBuf, GetStatsColor(i));
	}
	ini.SetInt(_T("ConnectionsGraphRatio"), m_byteGraphRatio);

	ini.SetBool(_T("MlDonkeyCM"), m_bCounterMeasures);
	ini.SetBool(_T("CMNotLog"), m_bCMNotLog);

	ini.SetInt(_T("ServerKeepAliveTimeout"), m_dwServerKeepAliveTimeout);
	ini.SetBool(_T("WatchClipboard4ED2kFilelinks"), m_bWatchClipboard);
	ini.SetBool(_T("ExportLocalizedLinks"), m_bExportLocalizedLinks);

//	AutoCheck for new version
	ini.SetBool(_T("AutoCheckForNewVersion"), m_bAutoCheckForNewVersion);
	ini.SetInt(_T("AutoCheckLastTime"), m_iAutoCheckLastTime);

	ini.SetString(_T("A4AFAutoFileHash"), HashToString(m_A4AF_FileHash));

//	Messaging
	ini.SetInt(_T("AcceptMessageFrom"), m_uiAcceptMessagesFrom);
	ini.SetString(_T("AwayStateMessage"), *m_sAwayStateMessage.Get(&strBuf));

	ini.SetBool(_T("ScanFilter"), m_bScanFilter);
	ini.SetBool(_T("FakeRxDataFilter"), m_bFakeRxDataFilter);
	ini.SetBool(_T("PreviewSmallBlocks"), m_bPreviewSmallBlocks);
	ini.SetBool(_T("ShowFullFileStatusIcons"), m_bShowFullFileStatusIcons);
	ini.SetBool(_T("ShowPausedGray"), m_bShowPausedGray);
	ini.SetBool(_T("RoundSizes"), m_bRoundSizes);
	ini.SetBool(_T("TransferredOnCompleted"), m_bTransferredOnCompleted);
	ini.SetBool(_T("UseDwlPercentage"), m_bUseDwlPercentage);
	ini.SetString(_T("URLsForICC"), *m_sURLsForICC.Get(&strBuf));
	ini.SetInt(_T("DetailedPartsFilter"), m_byteDetailedPartsFilter);

	ini.SetInt(_T("SmartFilterMaxQueueRank"), m_nSmartFilterMaxQR);
	ini.SetBool(_T("SmartFilterShowOnQueue"), m_bSmartFilterShowOQ);

	ini.SetInt(_T("FilterLevel"), static_cast<int>(m_byteIPFilterLevel));
	ini.SetString(_T("UpdateURLIPFilter"), m_strIPFilterURL);	// Value name same as eMule Morph for maximum compatibility
	ini.SetBool(_T("AutoUPdateIPFilter"), m_bIPFilterUpdateOnStart);	// Value name same as eMule Morph for maximum compatibility

	ini.SetInt(_T("LastIPFilterUpdate"), m_dwLastIPFilterUpdate);
	ini.SetInt(_T("IPFilterUpdateFrequency"), m_dwIPFilterUpdateFrequency);

//	Antivirus
	ini.SetBool(_T("AVEnabled"), m_bAVEnabled);
	ini.SetString(_T("AVPath"), m_strAVPath);
	ini.SetString(_T("AVParams"), m_strAVParams);
	ini.SetBool(_T("AVScanCompleted"), m_bAVScanCompleted);

//	Splitter settings
	ini.SetInt(_T("SplitterbarPositionFriend"), m_uSplitterPosFriend);

	ini.SetArray(&m_adRollupPos[0][0], sizeof(m_adRollupPos) / sizeof(m_adRollupPos[0][0]), _T("RollupPos"));
	ini.SetArray(m_abRollupState, ARRSIZE(m_abRollupState), _T("RollupState"));

#if 0//Enable to proceed work on obfuscation
	ini.SetBool(_T("CryptLayerRequested"), m_bCryptLayerRequested);
	ini.SetBool(_T("CryptLayerRequired"), m_bCryptLayerRequired);
	ini.SetBool(_T("CryptLayerSupported"), m_bCryptLayerSupported);
#endif

//	Proxy
	ini.SetDefaultCategory(_T("Proxy"));
	ini.SetBool(_T("EnablePassword"), m_proxy.m_bEnablePassword);
	ini.SetBool(_T("EnableProxy"), m_proxy.m_bUseProxy);
	ini.SetString(_T("Name"), CString(m_proxy.m_strName));
	ini.SetString(_T("Password"), Crypt(CString(m_proxy.m_strPassword)));
	ini.SetString(_T("User"), CString(m_proxy.m_strUser));
	ini.SetInt(_T("Port"), m_proxy.m_uPort);
	ini.SetInt(_T("Type"), m_proxy.m_nType);

//	WebServer
	ini.SetDefaultCategory(_T("WebServer"));

	ini.SetString(_T("Password"), HashToString(m_abyteWebPassword));
	ini.SetString(_T("PasswordLow"), HashToString(m_abyteLowWebPassword));
	ini.SetInt(_T("Port"), m_uWSPort);
	ini.SetBool(_T("Enabled"), m_bWebEnabled);
	ini.SetBool(_T("EnabledLow"), m_bWebLowEnabled);
	ini.SetInt(_T("PageRefreshTime"), m_nWebPageRefresh);
	ini.SetBool(_T("IntruderDetection"), m_bWebIntruderDetection);
	ini.SetInt(_T("TempDisableLogin"), m_dwWebTempDisableLogin);
	ini.SetInt(_T("LoginAttemptsAllowed"), m_dwWSLoginAttemptsAllowed);
	ini.SetString(_T("WebTemplateFile"), m_strTemplateFile);
	ini.SetBool(_T("MenuLocked"), m_WSPrefs.bMenuLocked);
	ini.SetBool(_T("ShowUploadQueue"), m_WSPrefs.bShowUploadQueue);
	ini.SetBool(_T("ShowUploadQueueBanned"), m_WSPrefs.bShowUploadQueueBanned);
	ini.SetBool(_T("ShowUploadQueueFriend"), m_WSPrefs.bShowUploadQueueFriend);
	ini.SetBool(_T("ShowUploadQueueCredit"), m_WSPrefs.bShowUploadQueueCredit);
	ini.SetArray(m_WSPrefs.abDownloadColHidden, ARRSIZE(m_WSPrefs.abDownloadColHidden), _T("downloadColumnHidden"));
	ini.SetArray(m_WSPrefs.abUploadColHidden, ARRSIZE(m_WSPrefs.abUploadColHidden), _T("uploadColumnHidden"));
	ini.SetArray(m_WSPrefs.abQueueColHidden, ARRSIZE(m_WSPrefs.abQueueColHidden), _T("queueColumnHidden"));
	ini.SetArray(m_WSPrefs.abSharedColHidden, ARRSIZE(m_WSPrefs.abSharedColHidden), _T("sharedColumnHidden"));
	ini.SetArray(m_WSPrefs.abServerColHidden, ARRSIZE(m_WSPrefs.abServerColHidden), _T("serverColumnHidden"));
	ini.SetArray(m_WSPrefs.abSearchColHidden, ARRSIZE(m_WSPrefs.abSearchColHidden), _T("searchColumnHidden"));
	ini.SetUInt32(_T("DownloadSort"), m_WSPrefs.dwDownloadSort);
	ini.SetUInt32(_T("UploadSort"), m_WSPrefs.dwUploadSort);
	ini.SetUInt32(_T("QueueSort"), m_WSPrefs.dwQueueSort);
	ini.SetUInt32(_T("SharedSort"), m_WSPrefs.dwSharedSort);
	ini.SetUInt32(_T("ServerSort"), m_WSPrefs.dwServerSort);
	ini.SetUInt32(_T("SearchSort"), m_WSPrefs.dwSearchSort);
	ini.SetArray(m_WSPrefs.abDownloadSortAsc, ARRSIZE(m_WSPrefs.abDownloadSortAsc), _T("DownloadSortDir"));
	ini.SetArray(m_WSPrefs.abUploadSortAsc, ARRSIZE(m_WSPrefs.abUploadSortAsc), _T("UploadSortDir"));
	ini.SetArray(m_WSPrefs.abQueueSortAsc, ARRSIZE(m_WSPrefs.abQueueSortAsc), _T("QueueSortDir"));
	ini.SetArray(m_WSPrefs.aiSharedSortAsc, ARRSIZE(m_WSPrefs.aiSharedSortAsc), _T("SharedSortDir"));
	ini.SetArray(m_WSPrefs.abServerSortAsc, ARRSIZE(m_WSPrefs.abServerSortAsc), _T("ServerSortDir"));
	ini.SetArray(m_WSPrefs.abSearchSortAsc, ARRSIZE(m_WSPrefs.abSearchSortAsc), _T("SearchSortDir"));

//	MobileMule
	ini.SetDefaultCategory(_T("MobileMule"));
	ini.SetString(_T("Password"), HashToString(m_abyteMMPassword));
	ini.SetBool(_T("Enabled"), m_bMMEnabled);
	ini.SetInt(_T("Port"), m_uMMPort);

//	LANCAST
	ini.SetDefaultCategory(_T("Lancast"));
	ini.SetBool(_T("Enabled"), m_bLancastEnabled);
	ini.SetBool(_T("UseDefaultIP"), m_bUseDefaultIP);
	strBuf.Format(_T("%i.%i.%i.%i"),(byte)m_dwLancastIP,(byte)(m_dwLancastIP>>8),(byte)(m_dwLancastIP>>16),(byte)(m_dwLancastIP>>24));
	ini.SetString(_T("IPAddress"), strBuf);
	strBuf.Format(_T("%i.%i.%i.%i"),(byte)m_dwLancastSubnet,(byte)(m_dwLancastSubnet>>8),(byte)(m_dwLancastSubnet>>16),(byte)(m_dwLancastSubnet>>24));
	ini.SetString(_T("Subnet"), strBuf);
	ini.SetInt(_T("Port"), m_uLancastPort);

	ini.SetDefaultCategory(_T("Cleanup"));
	ini.SetBool(_T("AutoFilenameCleanup"), m_bAutoFilenameCleanup);
	ini.SetBool(_T("CleanupTags"), m_bFilenameCleanupTags);
	ini.SetString(_T("FilenameCleanups"), *m_sFilenameCleanups.Get(&strBuf));
	ini.SetString(_T("FilenameCleanupsUpd"), m_strFilenameCleanupsUpd);

//	SMTP Notifier
	ini.SetDefaultCategory(_T("SMTP"));
	ini.SetString(_T("Server"), *m_sSMTPServer.Get(&strBuf));
	ini.SetString(_T("Name"), *m_sSMTPName.Get(&strBuf));
	ini.SetString(_T("From"), *m_sSMTPFrom.Get(&strBuf));
	ini.SetString(_T("To"), *m_sSMTPTo.Get(&strBuf));
	ini.SetString(_T("UserName"), *m_sSMTPUserName.Get(&strBuf));
	ini.SetString(_T("Password"), Crypt(*m_sSMTPPassword.Get(&strBuf)));
	ini.SetBool(_T("Authenticated"), m_bSMTPAuthenticated);
	ini.SetBool(_T("Info"), m_bSMTPInfo);
	ini.SetBool(_T("Warning"), m_bSMTPWarning);
	ini.SetBool(_T("MsgInSubject"), m_bSMTPMsgInSubj);

//	Backup feature
	ini.SetDefaultCategory(_T("Backup"));
	ini.SetBool(_T("DatFiles"), m_bDatFiles);
	ini.SetBool(_T("MetFiles"), m_bMetFiles);
	ini.SetBool(_T("IniFiles"), m_bIniFiles);
	ini.SetBool(_T("PartFiles"), m_bPartFiles);
	ini.SetBool(_T("PartMetFiles"), m_bPartMetFiles);
	ini.SetBool(_T("PartTxtsrcFiles"), m_bPartTxtsrcFiles);
	ini.SetBool(_T("AutoBackup"), m_bAutoBackup);
	ini.SetBool(_T("BackupOverwrite"), m_bBackupOverwrite);
	ini.SetString(_T("BackupDir"), *m_sBackupDir.Get(&strBuf));
	ini.SetBool(_T("ScheduledBackup"), m_bScheduledBackup);
	ini.SetInt(_T("ScheduledBackupInterval"), m_nScheduledBackupInterval);

//	FakeCheck
	ini.SetDefaultCategory(_T("FakeCheck"));
	ini.SetInt(_T("FakesDatVersion"),m_nFakesDatVersion);
	ini.SetBool(_T("UpdateFakeStartup"),m_bUpdateFakeStartup);
	ini.SetString(_T("FakeListURL"), m_sFakeListURL);
	ini.SetInt(_T("DownloadingFakeListVersion"), m_dwDLingFakeListVersion);
	ini.SetString(_T("DownloadingFakeListLink"), m_strDLingFakeListLink);
	ini.SetUInt32(_T("FakeListDownloadColor"), m_clrFakeListDownloadColor);

//	Shortcut management
	ini.SetDefaultCategory(_T("Shortcuts"));
	for (int i = 0; i < SCUT_COUNT; i++)
		ini.SetInt(s_apcShortcutNames[i], m_anShortcutCode[i]);

	ini.SaveAndClose();

//	Now file is closed, so we can update stats over SaveStats()
	SaveStats();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SaveCats()
{
	CString		strTemp;

	strTemp.Format(_T("%scategory.ini"), m_strConfigDir);
	_tremove(strTemp);

	if (CCat::GetNumCats() > 1)
	{
		CIni		catini(strTemp);

		catini.SetDefaultCategory(_T("General"));
		catini.SetInt(_T("Count"), CCat::GetNumCats() - CCat::GetNumPredefinedCats());

		for (int i = 1; i < CCat::GetNumPredefinedCats(); i++)
		{
			CCat	*pCat = CCat::GetCatByIndex(i);

			strTemp.Format(_T("PredefinedCat%#i"), i);
			catini.SetInt(strTemp, pCat->m_eCatID);
		}

	//	If there are user-defined categories...
		if (CCat::GetNumCats() > CCat::GetNumPredefinedCats())
		{
			for (int ix = CCat::GetNumPredefinedCats(); ix < CCat::GetNumCats(); ix++)
			{
				CCat	*pCat = CCat::GetCatByIndex(ix);

				strTemp.Format(_T("Cat#%u"), ix - CCat::GetNumPredefinedCats() + 1);
				catini.SetDefaultCategory(strTemp);
				catini.SetString(_T("Title"), pCat->m_strTitle);
				catini.SetString(_T("Incoming"), pCat->m_strSavePath);

				if (TempDirListCheckAndAdd(pCat->m_strTempPath, true) < 0)
				{
				//	There's no such directory in the list and its creation failed
					pCat->m_strTempPath = GetTempDir();
				}
				catini.SetString(_T("Temp"), pCat->m_strTempPath);
				catini.SetString(_T("Comment"), pCat->m_strComment);
				catini.SetUInt32(_T("Color"), pCat->m_crColor);
				catini.SetInt(_T("Priority"), pCat->m_iPriority);
				catini.SetString(_T("AutoCat"), pCat->m_strAutoCatExt);
				catini.SetInt(_T("CategoryID"), pCat->m_eCatID);
			}
		}
		catini.SaveAndClose();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetDefaultStatsColor()
COLORREF CPreferences::GetDefaultStatsColor(unsigned uiIdx)
{
	static const COLORREF acrDefColors[12] =
	{
		RGB(  0,   0,  64), RGB(192, 192, 255), RGB(128, 255, 128), RGB(  0, 210,   0),
		RGB(  0, 128,   0), RGB(255, 128, 128), RGB(200,   0,   0), RGB(140,   0,   0),
		RGB(150, 150, 255), RGB(192,   0, 192), RGB(255, 192, 255), RGB(255, 255, 128)
	};
	if (uiIdx >= ARRSIZE(acrDefColors))
		uiIdx = 0;
	return acrDefColors[uiIdx];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadIniPreferences() loads preferences stored in the "preferences.ini" file.
void CPreferences::LoadIniPreferences()
{
	CString strFileName;

	strFileName.Format(_T("%spreferences.ini"), m_strConfigDir);

//	Load using read-only to get AppVersion to detect if version was changed
	CIni* pIni = new CIni(strFileName, INI_MODE_READONLY);

	pIni->SetDefaultCategory(_T("eMule"));

	CString strPrefsVersion(pIni->GetString(_T("AppVersion"), _T("")));

	delete pIni;

	CFileFind findFileName;

	bool bChangedVersion = false;
	int loadstatsFromOld = 0;	// -khaos--+++> Fix to stats being lost when version changes!

	if ((strPrefsVersion.Compare(CURRENT_VERSION_LONG) != 0) && findFileName.FindFile(strFileName) && !strPrefsVersion.IsEmpty())
	{
		CString strNewName;

	//	Cax2 we can add the code to find our version's setting, if the ini file is from a different version...
	//	Rename the old .ini file
		strNewName.Format(_T("%spreferences[%s].ini"), m_strConfigDir, strPrefsVersion);
		::DeleteFile(strNewName);
	//	Process new file only after successful rename, otherwise process original file
	//	NB! rename will fail if AppVersion contained restricted characters
		if (::MoveFile(strFileName, strNewName))
		{
			strFileName = strNewName;

			bChangedVersion = true;
			loadstatsFromOld = 2; // -khaos--+++> Set this to 2 so that LoadStats will load 'em from ini.old
		}
	}

	CIni ini(strFileName, INI_MODE_READONLY);
	ini.SetDefaultCategory(_T("eMule"));

//	Set language as early as possible to specify correct locale
	GetWindowsVersion();	//required to set m_bIsNTBased

	m_nLanguageID = ini.GetWORD(_T("Language"), 0);
	SetLanguage();

	uint32	dwVal;
	uint16	uVal;
	static TCHAR	acNick[] = _T("       [ePlus]");

//	Generate a random six character default nickname
	for (unsigned ui = 0; ui < 6; ui++)
		acNick[ui] = static_cast<TCHAR>(rand() % (_T('z') - _T('a')) + _T('a'));

	ini.GetString(m_sNick.GetRaw(), _T("Nick"), acNick);

	CString	strBuf;

//	Get the incoming directory (default appdir\Incoming)
	strBuf.Format(_T("%sIncoming"), m_strAppDir);
	ini.GetString(m_sIncomingDir.GetRaw(), _T("IncomingDir"), strBuf);

//	Get the temp directory (default appdir\Temp)
	strBuf.Format(_T("%sTemp"), m_strAppDir);
	ini.GetString(m_sTempDir.GetRaw(), _T("TempDir"), strBuf);

//	Get the Video Player path and settings
	ini.GetString(&m_strVideoPlayer, _T("VideoPlayer"), _T(""));
	ini.GetString(&m_strVideoPlayerArgs, _T("VideoPlayerArgs"), _T(""));
	m_bBackupPreview = ini.GetBool(_T("VideoPreviewBackupped"), false);	//Cax2 string kept for compatibility with off client
	m_bPreviewSmallBlocks = ini.GetBool(_T("PreviewSmallBlocks"), false);

	// If Video Player path is invalid, try to find Media Player Classic or VLC path in the registry
	if (_taccess(m_strVideoPlayer.Trim(), 0) != 0)
	{
		HKEY	hKey;
		TCHAR	szBuf[255];
		DWORD	dwBufLen = sizeof(szBuf);
		LONG	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Gabest\\Media Player Classic"), 0, KEY_QUERY_VALUE, &hKey);

		if (lResult == ERROR_SUCCESS)
		{
			lResult = RegQueryValueEx(hKey, _T("ExePath"), NULL, NULL, (LPBYTE) szBuf, &dwBufLen);
			RegCloseKey(hKey);
			if ((lResult == ERROR_SUCCESS) && (dwBufLen <= sizeof(szBuf)))
			{
				m_strVideoPlayer = szBuf;
				m_bPreviewSmallBlocks = true;
			}
		}

		if (lResult != ERROR_SUCCESS)
		{
			lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\VideoLAN\\VLC"), 0, KEY_QUERY_VALUE, &hKey);
			if (lResult == ERROR_SUCCESS)
			{
				lResult = RegQueryValueEx(hKey, _T("InstallDir"), NULL, NULL, (LPBYTE) szBuf, &dwBufLen);
				RegCloseKey(hKey);
				if ((lResult == ERROR_SUCCESS) && (dwBufLen <= sizeof(szBuf)))
				{
					strBuf = szBuf;
					strBuf += _T("\\vlc.exe");
					m_strVideoPlayer = strBuf;
				}
			}
		}
	}

//	Get the Upload Capacity
	m_dwMaxGraphUploadRate = ValidateUpCapability(10 * ini.GetInt(_T("UploadCapacity"), PREF_DEF_UPCAP / 10));
//	Get the Download Capacity
	m_dwMaxGraphDownloadRate = ValidateDownCapability(10 * ini.GetInt(_T("DownloadCapacity"), PREF_DEF_DOWNCAP / 10));
//	Get the Upload Limit (default - pinned to Upload Capacity)
	m_dwMaxUpload = static_cast<uint32>((ini.GetDouble(_T("MaxUpload"), m_dwMaxGraphUploadRate) * 10) + 0.5);
	SetMaxUploadWithCheck(m_dwMaxUpload);
//	Get the Download limit (default - pinned to Download Capacity)
	m_dwMaxDownload = static_cast<uint32>((ini.GetDouble(_T("MaxDownload"), m_dwMaxGraphDownloadRate) * 10) + 0.5);
	SetMaxDownloadWithCheck(m_dwMaxDownload);
//	Get the Connection Limit (default - a bit less than the max the OS will support)
	m_nMaxConnections = ini.GetUInt16(_T("MaxConnections"), GetRecommendedMaxConnections());
//	Get the Dead Server Action (default 2? - non-zero indicates "remove")
	m_bRemoveDeadServers = ini.GetBool(_T("DeadServerAction"), true);

//	Get the TCP port
	dwVal = ini.GetInt(_T("Port"), 0);
	if (dwVal == 0)
	{
	//	Value is incorrect or doesn't exist -> select random port
		do
		{
		//	Range: PREF_MIN_NEWUSER_RNDPORT + (0 .. 32767)
			dwVal = rand() + PREF_MIN_NEWUSER_RNDPORT;
		//	Exclude other default ports to avoid conflicts
		} while ((dwVal == PREF_DEF_WS_PORT) || (dwVal == PREF_DEF_MM_PORT));
	}
	m_uPort = static_cast<uint16>(dwVal);

//	Get the UDP port
	dwVal = ini.GetInt(_T("UDPPort"), ~0);
	if (dwVal == ~0)
		dwVal = m_uPort + (m_uPort & 0x1E) + 1;	//value doesn't exist -> select based on TCP port
	m_uUDPPort = static_cast<uint16>(dwVal);

//	Get Windows XP firewall ports opener
	m_bOpenPorts = ini.GetBool(_T("OpenXPFirewallPorts"), true);

	m_bUseServerAuxPort = ini.GetBool(_T("UseServerAuxPort"), false);
//	Get the Sources Per File Limit (default 400)
	m_dwMaxSourcePerFile = ini.GetInt(_T("MaxSourcesPerFile"), 400);
	if (m_dwMaxSourcePerFile < PREF_MIN_MAXFILESRC)
		m_dwMaxSourcePerFile = PREF_MIN_MAXFILESRC;
	if (m_dwMaxSourcePerFile > PREF_MAX_MAXFILESRC)
		m_dwMaxSourcePerFile = PREF_MAX_MAXFILESRC;
//	Get the Queue Size Limit (default 3 x Min Queue Size - pinned between Min Queue Size and Max Queue Size)
	m_dwQueueSize = ini.GetInt(_T("QueueSize"), 3 * MIN_QUEUE);
	if (m_dwQueueSize > MAX_QUEUE)
		m_dwQueueSize = MAX_QUEUE;
	if (m_dwQueueSize < MIN_QUEUE)
		m_dwQueueSize = MIN_QUEUE;
//	Get the See Shares setting (default Everybody)
	m_nSeeShares = (enSeeShare)ini.GetInt(_T("SeeShare"), SEE_SHARE_NOONE);
//	Default shared file permission
	dwVal = ini.GetInt(_T("FilePermission"), PERM_ALL);
	if (dwVal > PERM_NOONE)
		dwVal = PERM_ALL;
	m_byteFilePermission = static_cast<byte>(dwVal);
	m_bUsePartTraffic = ini.GetBool(_T("PartTrafficEnabled"), false);
	m_byteUpbarPartTraffic = static_cast<byte>(ini.GetInt(_T("UploadbarStyle"), 2));
	if (m_byteUpbarPartTraffic > 3)
		m_byteUpbarPartTraffic = 2;
	dwVal = ini.GetInt(_T("UploadbarColor"), 0);
	if (dwVal > 3)
		dwVal = 0;
	m_byteUpbarColorPartTraffic = static_cast<byte>(dwVal);
	m_dwTooltipDelay = ini.GetInt(_T("ToolTipDelay"), 2);
	if (m_dwTooltipDelay > 60)
		m_dwTooltipDelay = 60;
	m_nTrafficOMeterInterval = ini.GetUInt16(_T("StatGraphsInterval"), 3);
	m_nStatsInterval = ini.GetUInt16(_T("StatsInterval"), 5);
	m_iDetailColumnWidth = ini.GetInt(_T("DetailColumnWidth"), 150);
#if 1	//code left for smooth migration, delete in v1.3
	m_dwDeadServerRetries = ini.GetInt(_T("DeadServerRetries"), 1);
	m_dwDeadServerRetries = ini.GetInt(_T("DeadServerRetry"), m_dwDeadServerRetries);
#else
	m_dwDeadServerRetries = ini.GetInt(_T("DeadServerRetry"), 1);
#endif
	if (m_dwDeadServerRetries == 0)
		m_dwDeadServerRetries = 1;
	if (m_dwDeadServerRetries > PREF_MAX_DEADSRVRETRY)
		m_dwDeadServerRetries = PREF_MAX_DEADSRVRETRY;
	m_nStatsMax = ini.GetUInt16(_T("VariousStatisticsMaxValue"), 100);
	m_nStatsAverageMinutes = static_cast<byte>(ini.GetInt(_T("StatsAverageMinutes"), 5));
	m_nMaxConPerFive = ini.GetUInt16(_T("MaxConnectionsPerFiveSeconds"), GetDefaultMaxConPerFiveSecs());
	m_bShowRatesInTitle = ini.GetBool(_T("ShowRatesOnTitle"), false);

	dwVal = (IsNTBased()) ? 200 : 100;
	if (m_nMaxConPerFive > dwVal)
		m_nMaxConPerFive = static_cast<uint16>(dwVal);
	if (m_nMaxConPerFive < 5)
		m_nMaxConPerFive = 5;

	m_dwBadClientBanTime = ini.GetInt(_T("BadClientBanTime"), MIN2MS(60));
	if (m_dwBadClientBanTime > MIN2MS(PREF_MAX_MINS2BAN))
		m_dwBadClientBanTime = MIN2MS(PREF_MAX_MINS2BAN);
	if (m_dwBadClientBanTime < MIN2MS(PREF_MIN_MINS2BAN))
		m_dwBadClientBanTime = MIN2MS(PREF_MIN_MINS2BAN);

	m_dwMinRequestTime = ini.GetInt(_T("BadClientMinRequestTime"), 590000);
	if (m_dwMinRequestTime > 590000)
		m_dwMinRequestTime = 590000;
	if (m_dwMinRequestTime < 120000)
		m_dwMinRequestTime = 120000;

	dwVal = ini.GetInt(_T("BadClientMinRequestNum"), 3);
	if (dwVal < PREF_MIN_BANTIMES)
		dwVal = PREF_MIN_BANTIMES;
	if (dwVal > PREF_MAX_BANTIMES)
		dwVal = PREF_MAX_BANTIMES;
	m_byteBadClientBan = static_cast<byte>(dwVal);
	m_bBanMessageEnabled = ini.GetBool(_T("BanMessageEnabled"), false);
	m_bBanEnabled = ini.GetBool(_T("BanEnabled"), true);

	m_dwSLSOutdated = ini.GetInt(_T("SLS_WhenSourcesOutdated"), 1);
	if (m_dwSLSOutdated > PREF_MAX_FILESLSDAYS)
		m_dwSLSOutdated = PREF_MAX_FILESLSDAYS;
	if (m_dwSLSOutdated < PREF_MIN_FILESLSDAYS)
		m_dwSLSOutdated = PREF_MIN_FILESLSDAYS;

	m_dwSLSMaxSrcPerFile = ini.GetInt(_T("SLS_MaxSourcesPerFile"), 10);
	if (m_dwSLSMaxSrcPerFile > PREF_MAX_MAXFILESLS)
		m_dwSLSMaxSrcPerFile = PREF_MAX_MAXFILESLS;
	if (m_dwSLSMaxSrcPerFile < PREF_MIN_MAXFILESLS)
		m_dwSLSMaxSrcPerFile = PREF_MIN_MAXFILESLS;

	m_bSLSEnable = ini.GetBool(_T("SLS_Enable"), true);

	m_bReconnect = ini.GetBool(_T("Reconnect"), true);
	m_bUseServerPriorities = ini.GetBool(_T("Scoresystem"), true);

	m_bUAP = ini.GetBool(_T("UAP"), true);
	m_bDAP = ini.GetBool(_T("DAP"), true);
	m_bDisableXS = ini.GetBool(_T("DisableXS"), false);
	m_bStartDownloadPaused = ini.GetBool(_T("StartDownloadPaused"), false);
	m_bDownloadPausedOnComplete = ini.GetBool(_T("DownloadPausedOnComplete"), false);
	m_bResumeOtherCat = ini.GetBool(_T("ResumeOtherCat"), true);
	m_bAutoClearCompleted = ini.GetBool(_T("AutoClearCompleted"), false);
	m_nMinAutoSourcesPerFile = ini.GetInt(_T("MinAutoSourcesPerFile"), 150);
	m_nMaxAutoSourcesPerFile = ini.GetInt(_T("MaxAutoSourcesPerFile"), 800);
	m_bAutoSourcesEnabled = ini.GetBool(_T("AutoSourcesEnabled"), false);
	if (m_bAutoSourcesEnabled)
	{
		m_dwMaxSourcePerFile = m_nMinAutoSourcesPerFile;
		m_nXSUpTo = 100;
	}
	m_nMaxAutoSourcesTotal = ini.GetInt(_T("MaxAutoSourcesTotal"), 2500);
	m_nMaxAutoExchangeSources = ini.GetInt(_T("MaxAutoExchangeSources"), 400);
	m_bAutoSourcesLogEnabled = ini.GetBool(_T("AutoSourcesLogEnabled"), false);
	m_bClientTransferLogEnabled = ini.GetBool(_T("ClientTransferLogEnabled"), true);

	ini.GetString(m_sToolbarSettings.GetRaw(), _T("ToolbarSettings"), GetDefaultToolbarSettings());
	ini.GetString(m_sToolbarBitmap.GetRaw(), _T("ToolbarBitmap"), _T(""));
	ini.GetString(m_sToolbarBitmapFolder.GetRaw(), _T("ToolbarBitmapFolder"), _T(""));
	m_iToolbarLabels = ini.GetInt(_T("ToolbarLabels"), 1);
	if (static_cast<unsigned>(m_iToolbarLabels) > 2u)
		m_iToolbarLabels = 1;
	m_bAutoServerList = ini.GetBool(_T("Serverlist"), false);
	m_bUpdateNotify = ini.GetBool(_T("UpdateNotify"), true);
	m_bMinToTray = ini.GetBool(_T("MinimizeToTray"), true);
	m_bAddServersFromServer = ini.GetBool(_T("AddServersFromServer"), true);
	m_bAddServersFromClients = ini.GetBool(_T("AddServersFromClient"), false);
	m_bSplashScreen = ini.GetBool(_T("Splashscreen"), true);
	m_bBringToForeground = ini.GetBool(_T("BringToFront"), true);
	m_bBeepOnError = ini.GetBool(_T("BeepOnError"), true);
	m_bConfirmExit = ini.GetBool(_T("ConfirmExit"), true);
	m_bConfirmDisconnect = ini.GetBool(_T("ConfirmDisconnect"), false);
	m_bConfirmFriendDel = ini.GetBool(_T("ConfirmFriendDel"), true);
	m_bFilterBadIP = ini.GetBool(_T("FilterBadIPs"), true);
	m_bFilterServersByIP = ini.GetBool(_T("FilterServersByIP"), true);
	m_bAutoConnect = ini.GetBool(_T("Autoconnect"), false);
	m_bOnlineSig = ini.GetBool(_T("OnlineSignature"), false);
	m_bStartMinimized = ini.GetBool(_T("StartupMinimized"), false);
	m_bShowToolbarSpeedMeter = ini.GetBool(_T("ToolbarSpeedMeter"), false);
	m_bShowCountryFlag = ini.GetBool(_T("CountryFlag"), true);
	m_bCloseToTray = ini.GetBool(_T("CloseToTray"), false);
	m_bDetailsOnClick = ini.GetBool(_T("DetailsOnClick"), false);

	CCat::SetAllCatType(static_cast<_EnumCategories>(ini.GetInt(_T("AllCatType"), CAT_ALL)));

	m_nSrvConTimeout = ini.GetInt(_T("SrvConTimeout"), SEC2MS(20));
	if (m_nSrvConTimeout < SEC2MS(PREF_MIN_SRVCONNTIMEOUT))
		m_nSrvConTimeout = SEC2MS(PREF_MIN_SRVCONNTIMEOUT);
	if (m_nSrvConTimeout > SEC2MS(PREF_MAX_SRVCONNTIMEOUT))
		m_nSrvConTimeout = SEC2MS(PREF_MAX_SRVCONNTIMEOUT);

	m_dwSlowCompleteBlockSize = ini.GetInt(_T("SlowCompleteBlockSize"), 32);
	if (m_dwSlowCompleteBlockSize < PREF_MIN_COMPLETEBLOCKSZ)
		m_dwSlowCompleteBlockSize = PREF_MIN_COMPLETEBLOCKSZ;
	if (m_dwSlowCompleteBlockSize > PREF_MAX_COMPLETEBLOCKSZ)
		m_dwSlowCompleteBlockSize = PREF_MAX_COMPLETEBLOCKSZ;

	m_bAutoConnectStaticOnly = ini.GetBool(_T("AutoConnectStaticOnly"), false);
	m_byteDepth3D = static_cast<byte>(ini.GetInt(_T("3DDepth"), 0));
	if ((m_byteDepth3D > 5) && (m_byteDepth3D < 251))	//check for possible values
		m_byteDepth3D = 0;

	m_bUseDownloadAddNotifier = ini.GetBool(_T("NotifyOnDownloadAdd"), false);
	m_bUseDownloadNotifier = ini.GetBool(_T("NotifyOnDownload"), false);
	m_bUseChatNotifier = ini.GetBool(_T("NotifyOnChat"), false);
	m_bUseLogNotifier = ini.GetBool(_T("NotifyOnLog"), false);
	m_bUseSoundInNotifier = ini.GetBool(_T("NotifierUseSound"), false);
	m_bUseSchedulerNotifier = ini.GetBool(_T("NotifierUseScheduler"), false);
	m_bNotifierPopsEveryChatMsg = ini.GetBool(_T("NotifierPopEveryChatMessage"), false);
	m_bNotifierImportantError = ini.GetBool(_T("NotifyOnImportantError"), false);
	m_bNotifierWebServerError = ini.GetBool(_T("NotifyOnWebServerError"), false);
	m_bNotifierServerError = ini.GetBool(_T("NotifyOnServerError"), false);
	ini.GetString(m_sNotifierSoundFilePath.GetRaw(), _T("NotifierSoundPath"), _T(""));

#if 1	//code left for smooth migration, delete in v1.2f
	ini.GetString(&strBuf, _T("IRCServer"), _T("irc.emuleplus.info"));
// Change old invalid values to current default
	strBuf.Trim().MakeLower();
	if (strBuf.Left(13) == _T("irc.freshirc."))
		strBuf = _T("irc.emuleplus.info");
	*m_sIRCServer.GetRaw() = strBuf;
#else
	ini.GetString(m_sIRCServer.GetRaw(), _T("IRCServer"), _T("irc.emuleplus.info"));
#endif

	ini.GetString(m_sIRCNick.GetRaw(), _T("IRCNick"), m_sNick.Get());
	m_bIRCAddTimestamp = ini.GetBool(_T("IRCAddTimestamp"), true);
	ini.GetString(m_sIRCChanNameFilter.GetRaw(), _T("IRCFilterName"), _T(""));
	m_bIRCUseChanFilter = ini.GetBool(_T("IRCUseFilter"), false);
	m_nIRCChannelUserFilter = ini.GetUInt16(_T("IRCFilterUser"), 0);
	ini.GetString(m_sIRCPerformString.GetRaw(), _T("IRCPerformString"), _T("/join #emule+"));
	m_bIRCUsePerform = ini.GetBool(_T("IRCUsePerform"), true);
	m_bIRCListOnConnect = ini.GetBool(_T("IRCListOnConnect"), true);
	m_bIRCIgnoreInfoMessage = ini.GetBool(_T("IRCIgnoreInfoMessage"), false);
	m_bIRCMessageEncodingUTF8 = ini.GetBool(_T("IRCMessageEncodingUTF8"), false);
	m_bIRCStripColor = ini.GetBool(_T("IRCStripColor"), false);

	m_bSmartIdCheck = ini.GetBool(_T("SmartIdCheck"), true);
	m_bVerbose = ini.GetBool(_T("Verbose"), false);
	m_bManuallyAddedServerHighPrio = ini.GetBool(_T("ManualHighPrio"), false);
	m_bShowOverhead = ini.GetBool(_T("ShowOverhead"), true);

	ini.GetString(&m_strTxtEditor, _T("TxtEditor"), _T("notepad.exe"));
	ini.GetString(&m_strMessageFilter, _T("MessageFilter"), _T("Defcon|Your client has an infinite queue"));
	ini.GetString(m_sCommentFilter.GetRaw(), _T("CommentFilter"), _T("http://www.|emulespeedup|fastest eMule ever"));
//	Update current user value with new tokens
//	NB! update value shouldn't be used for more than 2 releases (one is the best)
	ini.GetString(&m_strCommentFilterUpd, _T("CommentFilterUpd"), _T(""));
	UpdateUserStrList(m_sCommentFilter.GetRaw(), &m_strCommentFilterUpd, EP_PREFUPD_COMMENT_FILTER);
	m_bShowRatingIcons = ini.GetBool(_T("ShowRatingIcons"), true);

	m_bUseSort = ini.GetBool(_T("UseSortingDefaults"), true);
	m_bUseSrcSortCol2 = ini.GetBool(_T("Use2ndSortSources"), false);
	m_bPausedStoppedLast = ini.GetBool(_T("PausedStoppedLast"), false);
	m_uiServerSortCol = ini.GetInt(_T("SortServersCode"), SL_COLUMN_SERVERNAME);
	if ((m_uiServerSortCol & MLC_COLUMNMASK) >= SL_COLUMN_NUMCOLUMNS)
		m_uiServerSortCol = SL_COLUMN_SERVERNAME;
	m_uiUploadSortCol = ini.GetInt(_T("SortUploadsCode"), MLC_SORTDESC | ULCOL_UPLOADTIME);
	if ((m_uiUploadSortCol & MLC_COLUMNMASK) >= ULCOL_NUMCOLUMNS)
		m_uiUploadSortCol = MLC_SORTDESC | ULCOL_UPLOADTIME;
	m_uiQueueSortCol = ini.GetInt(_T("SortQueueCode"), MLC_SORTDESC | QLCOL_QLRATING);
	if ((m_uiQueueSortCol & MLC_COLUMNMASK) >= QLCOL_NUMCOLUMNS)
		m_uiQueueSortCol = MLC_SORTDESC | QLCOL_QLRATING;
	m_uiSearchSortCol = ini.GetInt(_T("SortSearchCode"), MLC_SORTDESC | SL_COLUMN_SOURCES);
	if ((m_uiSearchSortCol & MLC_COLUMNMASK) >= SL_NUMCOLUMNS)
		m_uiSearchSortCol = MLC_SORTDESC | SL_COLUMN_SOURCES;
	m_uiIRCSortCol = ini.GetInt(_T("SortIrcCode"), IRC2COL_NAME);
	if ((m_uiIRCSortCol & MLC_COLUMNMASK) >= IRC2COL_NUMCOLUMNS)
		m_uiIRCSortCol = IRC2COL_NAME;
	m_uiFileSortCol = ini.GetInt(_T("SortFilesCode"), SFL_COLUMN_FILENAME);
	if ((m_uiFileSortCol & MLC_COLUMNMASK) >= SFL_NUMCOLUMNS)
		m_uiFileSortCol = SFL_COLUMN_FILENAME;
	m_uiClientListSortCol = ini.GetInt(_T("SortClientListCode"), CLCOL_USERNAME);
	if ((m_uiClientListSortCol & MLC_COLUMNMASK) >= CLCOL_NUMCOLUMNS)
		m_uiClientListSortCol = CLCOL_USERNAME;
	m_uiDownloadSortCol = ini.GetInt(_T("SortDownloadsCode"), DLCOL_STATUS);
	if ((m_uiDownloadSortCol & MLC_COLUMNMASK) >= DLCOL_NUMCOLUMNS)
		m_uiDownloadSortCol = DLCOL_STATUS;
	m_uiSrcSortCol1 = ini.GetInt(_T("1stSortSourcesCode"), DLCOL_PRIORITY);
	if ((m_uiSrcSortCol1 & MLC_COLUMNMASK) >= DLCOL_NUMCOLUMNS)
		m_uiSrcSortCol1 = DLCOL_PRIORITY;
	m_uiSrcSortCol2 = ini.GetInt(_T("2ndSortSourcesCode"), DLCOL_FILENAME);
	if ((m_uiSrcSortCol2 & MLC_COLUMNMASK) >= DLCOL_NUMCOLUMNS)
		m_uiSrcSortCol2 = DLCOL_FILENAME;

	m_bCommunityEnabled = ini.GetBool(_T("CommunityEnabled"), false);
	ini.GetString(m_sCommunityString.GetRaw(), _T("CommunityString"), _T(""));
	m_bCommunityNoBan = ini.GetBool(_T("CommunityNoBan"), false);
	m_nNotificationDisplayTime = ini.GetInt(_T("NotificationDisplayTime"), 4000);
	m_nNotificationFontSize = ini.GetInt(_T("NotificationFontSize"), 70);
	m_uiProfile = ini.GetInt(_T("Profile"), 0);
	if (m_uiProfile > 11)
		m_uiProfile = 0;
	m_nPriorityHigh = static_cast<uint16>(ini.GetInt(_T("PriorityHigh"), PREF_DEF_DLSRC4HIGHPRIO));
	m_nPriorityLow = static_cast<uint16>(ini.GetInt(_T("PriorityLow"), PREF_DEF_DLSRC4LOWPRIO));
	if (m_nPriorityLow < m_nPriorityHigh)
	{
		m_nPriorityLow = m_nPriorityHigh * 2;
		if (m_nPriorityLow <= m_nPriorityHigh)	// high value is too big
			m_nPriorityHigh = 0;
	}
	if (m_nPriorityHigh == 0)
	{
		m_nPriorityLow = PREF_DEF_DLSRC4LOWPRIO;
		m_nPriorityHigh = PREF_DEF_DLSRC4HIGHPRIO;
	}
	m_bRestartWaiting = ini.GetBool(_T("RestartWaiting"), true);
	m_bLogToFile = ini.GetBool(_T("LogToFile"), false);
	m_bLogUploadToFile = ini.GetBool(_T("LogUploadToFile"), false);
	m_bLogDownloadToFile = ini.GetBool(_T("LogDownloadToFile"), false);
	m_bBugReport = ini.GetBool(_T("BugReport"), false);
	m_bAllowLocalHostIP = ini.GetBool(_T("AllowLocalHostIP"), false);
	m_bDisableXSUpTo = ini.GetBool(_T("DisableXSUpTo"), false);
	m_nXSUpTo = ini.GetUInt16(_T("XSUpTo"), uVal = GetMaxSourcePerFileSoft());
	if(m_nXSUpTo > uVal)
		m_nXSUpTo = uVal;
	ini.GetString(m_sUsedFont.GetRaw(), _T("UsedFont"), _T("MS Sans Serif"));
	m_nFontSize = static_cast<byte>(ini.GetInt(_T("FontSize"), 80));
	m_HashingPriority = (enHashPriority)ini.GetInt(_T("HashingPriority"), 2);
	m_bUploadParts = ini.GetBool(_T("UploadParts"), false);
	m_bA4AFStringEnabled = ini.GetBool(_T("A4AFStringEnabled"), false);
	m_bA4AFCountEnabled = ini.GetBool(_T("A4AFCountEnabled"), false);
	m_bBanMessageEnabled = ini.GetBool(_T("BanMessageEnabled"), false);
	m_dwSearchMethod = ini.GetInt(_T("SearchMethod"), EP_SEARCHMETHOD_SERV);
	if (m_dwSearchMethod >= EP_SEARCHMETHOD_COUNT)
		m_dwSearchMethod = EP_SEARCHMETHOD_SERV;
	m_bKeepSearchHistory = ini.GetBool(_T("KeepSearchHistory"), true);
	m_bAutoTakeLinks = ini.GetBool(_T("AutoTakeED2KLinks"), false);
#if 1	//code left for smooth migration, delete in v1.3
	m_dwFileBufferSize = ini.GetInt(_T("FileBufferSizePref"), 35);
	m_dwFileBufferSize *= 30000;	// old eMule Plus setting was in 30000 byte blocks
	m_dwFileBufferSize = ini.GetInt(_T("FileBufferSize"), m_dwFileBufferSize);
#else
	m_dwFileBufferSize = ini.GetInt(_T("FileBufferSize"), 1024 * 1024);	// 1MB
#endif
	m_dwFileBufferSize &= ~(32 * 1024 - 1);	// internally size is kept with 32KB granularity
//	Max size is a little bit bigger than chunk size to allow flushing of the whole chunk
	if ((m_dwFileBufferSize == 0) || (m_dwFileBufferSize > ((PARTSZ32 / (32u * 1024u) + 1u) * 32u * 1024u)))
		m_dwFileBufferSize = 1024 * 1024;
	m_nMainProcessPriority = ini.GetInt(_T("MainProcessPriority"), 0);
	m_bShowAverageDataRate = ini.GetBool(_T("ShowAverageDataRate"), false);
	m_bLimitlessDownload = ini.GetBool(_T("LimitlessDownload"), false);

	m_bSCHEnabled = ini.GetBool(_T("SCHEnabled"), false);
	m_bSCHExceptMon = ini.GetBool(_T("SCHExceptMon"), false);
	m_bSCHExceptTue = ini.GetBool(_T("SCHExceptTue"), false);
	m_bSCHExceptWed = ini.GetBool(_T("SCHExceptWed"), false);
	m_bSCHExceptThu = ini.GetBool(_T("SCHExceptThu"), false);
	m_bSCHExceptFri = ini.GetBool(_T("SCHExceptFri"), false);
	m_bSCHExceptSat = ini.GetBool(_T("SCHExceptSat"), true);
	m_bSCHExceptSun = ini.GetBool(_T("SCHExceptSun"), true);
	m_nSCHShift1 = ini.GetInt(_T("SCHShift1"),64800);
	m_nSCHShift2 = ini.GetInt(_T("SCHShift2"),21600);
	m_uSCHShift1Upload = static_cast<uint16>((ini.GetDouble(_T("SCHShift1Up"), m_dwMaxGraphUploadRate) * 10) + 0.5);
	if (m_uSCHShift1Upload < 10)
		m_uSCHShift1Upload = 10;
	if(m_uSCHShift1Upload > m_dwMaxGraphUploadRate)
		m_uSCHShift1Upload = static_cast<uint16>(m_dwMaxGraphUploadRate);
	m_uSCHShift1Download = static_cast<uint16>((ini.GetDouble(_T("SCHShift1Down"), m_dwMaxGraphDownloadRate) * 10) + 0.5);
	if (m_uSCHShift1Download < 10)
		m_uSCHShift1Download = 10;
	if(m_uSCHShift1Download > m_dwMaxGraphDownloadRate)
		m_uSCHShift1Download = static_cast<uint16>(m_dwMaxGraphDownloadRate);
	m_nSCHShift1conn = ini.GetUInt16(_T("SCHShift1Conn"), 512);
	m_nSCHShift15sec = ini.GetUInt16(_T("SCHShift15sec"), 40);
	m_uSCHShift2Upload = static_cast<uint16>((ini.GetDouble(_T("SCHShift2Up"), m_dwMaxGraphUploadRate) * 10) + 0.5);
	if (m_uSCHShift2Upload < 10)
		m_uSCHShift2Upload = 10;
	if(m_uSCHShift2Upload > m_dwMaxGraphUploadRate)
		m_uSCHShift2Upload = (uint16)m_dwMaxGraphUploadRate;
	m_uSCHShift2Download = static_cast<uint16>((ini.GetDouble(_T("SCHShift2Down"), m_dwMaxGraphDownloadRate) * 10) + 0.5);
	if (m_uSCHShift2Download < 10)
		m_uSCHShift2Download = 10;
	if(m_uSCHShift2Download > m_dwMaxGraphDownloadRate)
		m_uSCHShift2Download = (uint16)m_dwMaxGraphDownloadRate;

	m_nSCHShift2conn = ini.GetUInt16(_T("SCHShift2Conn"), 256);
	m_nSCHShift25sec = ini.GetUInt16(_T("SCHShift25sec"), 20);
	m_bShowFileTypeIcon = ini.GetBool(_T("ShowFileTypeIcon"), true);

	m_bMultiple = ini.GetBool(_T("MultipleInstances"), false);

	ini.GetArray(m_auDownloadColumnWidths, ARRSIZE(m_auDownloadColumnWidths), _T("DownloadColumnWidths"));
	ini.GetArray(m_abDownloadColumnHidden, ARRSIZE(m_abDownloadColumnHidden), _T("DownloadColumnHidden"));
	ini.GetArray(m_aiDownloadColumnOrder, ARRSIZE(m_aiDownloadColumnOrder), _T("DownloadColumnOrder"));
	ini.GetArray(m_auUploadColumnWidths, ARRSIZE(m_auUploadColumnWidths), _T("UploadColumnWidths"));
	ini.GetArray(m_abUploadColumnHidden, ARRSIZE(m_abUploadColumnHidden), _T("UploadColumnHidden"));
	ini.GetArray(m_aiUploadColumnOrder, ARRSIZE(m_aiUploadColumnOrder), _T("UploadColumnOrder"));
	ini.GetArray(m_auQueueColumnWidths, ARRSIZE(m_auQueueColumnWidths), _T("QueueColumnWidths"));
	ini.GetArray(m_abQueueColumnHidden, ARRSIZE(m_abQueueColumnHidden), _T("QueueColumnHidden"));
	ini.GetArray(m_aiQueueColumnOrder, ARRSIZE(m_aiQueueColumnOrder), _T("QueueColumnOrder"));
	ini.GetArray(m_auSearchColumnWidths, ARRSIZE(m_auSearchColumnWidths), _T("SearchColumnWidths"));
	ini.GetArray(m_abSearchColumnHidden, ARRSIZE(m_abSearchColumnHidden), _T("SearchColumnHidden"));
	ini.GetArray(m_aiSearchColumnOrder, ARRSIZE(m_aiSearchColumnOrder), _T("SearchColumnOrder"));
	ini.GetArray(m_auSharedColumnWidths, ARRSIZE(m_auSharedColumnWidths), _T("SharedColumnWidths"));
	ini.GetArray(m_abSharedColumnHidden, ARRSIZE(m_abSharedColumnHidden), _T("SharedColumnHidden"));
	ini.GetArray(m_aiSharedColumnOrder, ARRSIZE(m_aiSharedColumnOrder), _T("SharedColumnOrder"));
	ini.GetArray(m_auServerColumnWidths, ARRSIZE(m_auServerColumnWidths), _T("ServerColumnWidths"));
	ini.GetArray(m_abServerColumnHidden, ARRSIZE(m_abServerColumnHidden), _T("ServerColumnHidden"));
	ini.GetArray(m_aiServerColumnOrder, ARRSIZE(m_aiServerColumnOrder), _T("ServerColumnOrder"));
	ini.GetArray(m_auIrcNickColumnWidths, ARRSIZE(m_auIrcNickColumnWidths), _T("IrcNickColumnWidths"));
	ini.GetArray(m_abIrcNickColumnHidden, ARRSIZE(m_abIrcNickColumnHidden), _T("IrcNickColumnHidden"));
	ini.GetArray(m_aiIrcNickColumnOrder, ARRSIZE(m_aiIrcNickColumnOrder), _T("IrcNickColumnOrder"));
	ini.GetArray(m_auIrcColumnWidths, ARRSIZE(m_auIrcColumnWidths), _T("IrcColumnWidths"));
	ini.GetArray(m_abIrcColumnHidden, ARRSIZE(m_abIrcColumnHidden), _T("IrcColumnHidden"));
	ini.GetArray(m_aiIrcColumnOrder, ARRSIZE(m_aiIrcColumnOrder), _T("IrcColumnOrder"));
	ini.GetArray(m_auClientListColumnWidths, ARRSIZE(m_auClientListColumnWidths), _T("ClientListColumnWidths"));
	ini.GetArray(m_abClientListColumnHidden, ARRSIZE(m_abClientListColumnHidden), _T("ClientListColumnHidden"));
	ini.GetArray(m_aiClientListColumnOrder, ARRSIZE(m_aiClientListColumnOrder), _T("ClientListColumnOrder"));
	ini.GetArray(m_auPartStatusColumnWidths, ARRSIZE(m_auPartStatusColumnWidths), _T("PartStatusColumnWidths"));
	ini.GetArray(m_abPartStatusColumnHidden, ARRSIZE(m_abPartStatusColumnHidden), _T("PartStatusColumnHidden"));
	ini.GetArray(m_aiPartStatusColumnOrder, ARRSIZE(m_aiPartStatusColumnOrder), _T("PartStatusColumnOrder"));
	ini.GetArray(m_auFriendColumnWidths, ARRSIZE(m_auFriendColumnWidths), _T("FriendColumnWidths"));
	ini.GetArray(m_abFriendColumnHidden, ARRSIZE(m_abFriendColumnHidden), _T("FriendColumnHidden"));
	ini.GetArray(m_aiFriendColumnOrder, ARRSIZE(m_aiFriendColumnOrder), _T("FriendColumnOrder"));

//	Provide a mechanism for all tables to store/retrieve sort order
	m_iTableSortItemDownload = ini.GetInt(_T("TableSortItemDownload"), DLCOL_STATUS);
	if (m_iTableSortItemDownload >= DLCOL_NUMCOLUMNS)
		m_iTableSortItemDownload = DLCOL_STATUS;
	m_iTableSortItemDownload2 = ini.GetInt(_T("TableSortItemDownload2"), DLCOL_PRIORITY);
	if (m_iTableSortItemDownload2 >= DLCOL_NUMCOLUMNS)
		m_iTableSortItemDownload2 = DLCOL_PRIORITY;
	m_iTableSortItemUpload = ini.GetInt(_T("TableSortItemUpload"), ULCOL_UPLOADTIME);
	if (m_iTableSortItemUpload >= ULCOL_NUMCOLUMNS)
		m_iTableSortItemUpload = ULCOL_UPLOADTIME;
	m_iTableSortItemQueue = ini.GetInt(_T("TableSortItemQueue"), QLCOL_QLRATING);
	if (m_iTableSortItemQueue >= QLCOL_NUMCOLUMNS)
		m_iTableSortItemQueue = QLCOL_QLRATING;
	m_iTableSortItemSearch = ini.GetInt(_T("TableSortItemSearch"), SL_COLUMN_SOURCES);
	m_iTableSortItemSearch &= MLC_COLUMNMASK | MLC_SORTALT;
	if ((m_iTableSortItemSearch & MLC_COLUMNMASK) >= SL_NUMCOLUMNS)	//allow to save alternate criterion
		m_iTableSortItemSearch = SL_COLUMN_SOURCES;
	m_iTableSortItemShared = ini.GetInt(_T("TableSortItemShared"), SFL_COLUMN_FILENAME);
	m_iTableSortItemShared &= MLC_COLUMNMASK | MLC_SORTALT;
	if ((m_iTableSortItemShared & MLC_COLUMNMASK) >= SFL_NUMCOLUMNS)	//allow to save alternate criterion
		m_iTableSortItemShared = SFL_COLUMN_FILENAME;
	m_iTableSortItemServer = ini.GetInt(_T("TableSortItemServer"), SL_COLUMN_SERVERNAME);
	m_iTableSortItemServer &= MLC_COLUMNMASK | MLC_SORTALT;
	if ((m_iTableSortItemServer & MLC_COLUMNMASK) >= SL_COLUMN_NUMCOLUMNS)	//allow to save alternate criterion
		m_iTableSortItemServer = SL_COLUMN_SERVERNAME;
	m_iTableSortItemIRCNick = ini.GetInt(_T("TableSortItemIRCNicks"), IRC1COL_STATUS);
	if (m_iTableSortItemIRCNick >= IRC1COL_NUMCOLUMNS)
		m_iTableSortItemIRCNick = IRC1COL_STATUS;
	m_iTableSortItemIRCChannel = ini.GetInt(_T("TableSortItemIRCChannel"), IRC2COL_NAME);
	if (m_iTableSortItemIRCChannel >= IRC2COL_NUMCOLUMNS)
		m_iTableSortItemIRCChannel = IRC2COL_NAME;
	m_iTableSortItemClientList = ini.GetInt(_T("TableSortItemClientList"), CLCOL_USERNAME);
	if (m_iTableSortItemClientList >= CLCOL_NUMCOLUMNS)
		m_iTableSortItemClientList = CLCOL_USERNAME;
	m_iTableSortItemPartStatus = ini.GetInt(_T("TableSortItemPartStatus"), FDPARTSCOL_NUMBER);
	m_iTableSortItemPartStatus &= MLC_COLUMNMASK | MLC_SORTALT;
	if ((m_iTableSortItemPartStatus & MLC_COLUMNMASK) >= FDPARTSCOL_NUMCOLUMNS)	//allow to save alternate criterion
		m_iTableSortItemPartStatus = FDPARTSCOL_NUMBER;
	m_iTableSortItemFriend = ini.GetInt(_T("TableSortItemFriend"), FRIENDCOL_USERNAME);
	if (m_iTableSortItemFriend >= FRIENDCOL_NUMCOLUMNS)
		m_iTableSortItemFriend = FRIENDCOL_USERNAME;
	m_bTableSortAscendingDownload = ini.GetBool(_T("TableSortAscendingDownload"), true);
	m_bTableSortAscendingDownload2 = ini.GetBool(_T("TableSortAscendingDownload2"), true);
	m_bTableSortAscendingUpload = ini.GetBool(_T("TableSortAscendingUpload"), true);
	m_bTableSortAscendingQueue = ini.GetBool(_T("TableSortAscendingQueue"), true);
	m_bTableSortAscendingSearch = ini.GetBool(_T("TableSortAscendingSearch"), true);
	m_bTableSortAscendingShared = ini.GetBool(_T("TableSortAscendingShared"), true);
	m_bTableSortAscendingServer = ini.GetBool(_T("TableSortAscendingServer"), true);
	m_bTableSortAscendingIRCNick = ini.GetBool(_T("TableSortAscendingIRCNick"), true);
	m_bTableSortAscendingIRCChannel = ini.GetBool(_T("TableSortAscendingIRCChannel"), true);
	m_bTableSortAscendingClientList = ini.GetBool(_T("TableSortAscendingClientList"), true);
	m_bTableSortAscendingPartStatus = ini.GetBool(_T("TableSortAscendingPartStatus"), true);
	m_bTableSortAscendingFriend = ini.GetBool(_T("TableSortAscendingFriend"), true);

	if (m_nStatsAverageMinutes < 1)
		m_nStatsAverageMinutes = 5;

	for (unsigned ui = 0; ui < ARRSIZE(m_dwStatColors); ui++)
	{
		strBuf.Format(_T("StatColor%u"), ui);
		m_dwStatColors[ui] = ini.GetUInt32(strBuf, ~0ul);
		if (m_dwStatColors[ui] == ~0)
			m_dwStatColors[ui] = GetDefaultStatsColor(ui);
	}
	dwVal = ini.GetInt(_T("ConnectionsGraphRatio"), 1);
	if (((dwVal - 1u) >= 5) && (dwVal != 10) && (dwVal != 255))
		dwVal = 1;
	m_byteGraphRatio = static_cast<byte>(dwVal);
	m_bCounterMeasures = ini.GetBool(_T("MlDonkeyCM"), true);
	m_bCMNotLog = ini.GetBool(_T("CMNotLog"), false);

	m_dwServerKeepAliveTimeout = ini.GetInt(_T("ServerKeepAliveTimeout"),0);
	m_bWatchClipboard = ini.GetBool(_T("WatchClipboard4ED2kFilelinks"), false);
	m_bExportLocalizedLinks = ini.GetBool(_T("ExportLocalizedLinks"), false);

//	AutoCheck for new version
	m_bAutoCheckForNewVersion = ini.GetBool(_T("AutoCheckForNewVersion"), true);
	m_iAutoCheckLastTime = ini.GetInt(_T("AutoCheckLastTime"), 0);

//	Load hash of A4AF auto file
	ini.GetString(&strBuf, _T("A4AFAutoFileHash"), _T(""));
	StringToHash(strBuf, m_A4AF_FileHash);

//	Messaging
	m_uiAcceptMessagesFrom = ini.GetInt(_T("AcceptMessageFrom"), 1);
	if ((m_uiAcceptMessagesFrom - 1u) > 3u)
		m_uiAcceptMessagesFrom = 1;
	m_bAwayState = false;
	ini.GetString(m_sAwayStateMessage.GetRaw(), _T("AwayStateMessage"),_T(""));

	m_bScanFilter = ini.GetBool(_T("ScanFilter"), true);
	m_bFakeRxDataFilter = ini.GetBool(_T("FakeRxDataFilter"), false);
	m_bShowFullFileStatusIcons = ini.GetBool(_T("ShowFullFileStatusIcons"), true);
	m_bShowPausedGray = ini.GetBool(_T("ShowPausedGray"), false);
	m_bRoundSizes = ini.GetBool(_T("RoundSizes"), true);
	m_bTransferredOnCompleted = ini.GetBool(_T("TransferredOnCompleted"), false);
	m_bUseDwlPercentage = ini.GetBool(_T("UseDwlPercentage"), true);
	ini.GetString(m_sURLsForICC.GetRaw(), _T("URLsForICC"), _T("www.microsoft.com|www.google.com|www.yahoo.com|www.ibm.com|www.army.mil|www.networksolutions.com|www.ripe.net|www.w3.org"));
	m_byteDetailedPartsFilter = static_cast<byte>(ini.GetInt(_T("DetailedPartsFilter"), 0));

	m_nSmartFilterMaxQR = ini.GetUInt16(_T("SmartFilterMaxQueueRank"), 25);
	m_bSmartFilterShowOQ = ini.GetBool(_T("SmartFilterShowOnQueue"), false);

	m_byteIPFilterLevel = static_cast<byte>(ini.GetInt(_T("FilterLevel"), 127));
	ini.GetString(&m_strIPFilterURL, _T("UpdateURLIPFilter"), _T("http://emulepawcio.sourceforge.net/nieuwe_site/eplus_fakes/ipfilter.dat"));
	m_bIPFilterUpdateOnStart	= ini.GetBool(_T("AutoUPdateIPFilter"), false);

	m_dwLastIPFilterUpdate		= ini.GetInt(_T("LastIPFilterUpdate"), 0);
	m_dwIPFilterUpdateFrequency = ini.GetInt(_T("IPFilterUpdateFrequency"), 6);
	if (m_dwIPFilterUpdateFrequency > 6)
		m_dwIPFilterUpdateFrequency = 6;

//	Antivirus
	m_bAVEnabled				= ini.GetBool(_T("AVEnabled"), false);
	ini.GetString(&m_strAVPath, _T("AVPath"), _T(""));
	ini.GetString(&m_strAVParams, _T("AVParams"), _T(""));
	m_bAVScanCompleted			= ini.GetBool(_T("AVScanCompleted"), false);

//	Splitter settings
	m_uSplitterPosFriend = ini.GetUInt16(_T("SplitterbarPositionFriend"), 240);

	ini.GetArray(&m_adRollupPos[0][0], sizeof(m_adRollupPos) / sizeof(m_adRollupPos[0][0]), _T("RollupPos"));
	ini.GetArray(m_abRollupState, ARRSIZE(m_abRollupState), _T("RollupState"));

#if 0//Enable to proceed work on obfuscation
	m_bCryptLayerRequested = ini.GetBool(_T("CryptLayerRequested"), false);
	m_bCryptLayerRequired = ini.GetBool(_T("CryptLayerRequired"), false);
#ifdef _CRYPT_READY
	m_bCryptLayerSupported = ini.GetBool(_T("CryptLayerSupported"), true);
#else
	m_bCryptLayerSupported = ini.GetBool(_T("CryptLayerSupported"), false);
#endif
#else
	m_bCryptLayerRequested = false;
	m_bCryptLayerRequired = false;
	m_bCryptLayerSupported = false;
#endif

//	Proxy
	ini.SetDefaultCategory(_T("Proxy"));
	m_proxy.m_bEnablePassword = ini.GetBool(_T("EnablePassword"), false);
	m_proxy.m_bUseProxy = ini.GetBool(_T("EnableProxy"), false);
	m_proxy.m_strName = ini.GetString(_T("Name"), _T(""));
	m_proxy.m_strPassword = Decrypt(ini.GetString(_T("Password"), _T("")));
	m_proxy.m_strUser = ini.GetString(_T("User"), _T(""));
	dwVal = ini.GetInt(_T("Port"), PREF_DEF_PROXY_PORT);
	if ((dwVal - 1u) > 0xFFFEu)
		dwVal = PREF_DEF_PROXY_PORT;
	m_proxy.m_uPort = static_cast<uint16>(dwVal);

	dwVal = ini.GetInt(_T("Type"), PROXYTYPE_SOCKS4);
	if ((dwVal < PROXYTYPE_SOCKS4) || (dwVal > PROXYTYPE_HTTP11))
		dwVal = PROXYTYPE_SOCKS4;
	m_proxy.m_nType = static_cast<uint16>(dwVal);

//	WebServer
	ini.SetDefaultCategory(_T("WebServer"));
	ini.GetString(&strBuf, _T("Password"), _T(""));
	StringToHash(strBuf, m_abyteWebPassword);
	ini.GetString(&strBuf, _T("PasswordLow"), _T(""));
	StringToHash(strBuf, m_abyteLowWebPassword);
	dwVal = ini.GetInt(_T("Port"), PREF_DEF_WS_PORT);
	if ((dwVal - 1u) > 0xFFFEu)
		dwVal = PREF_DEF_WS_PORT;
	m_uWSPort = static_cast<uint16>(dwVal);
	m_bWebEnabled = ini.GetBool(_T("Enabled"), false);
	m_bWebLowEnabled = ini.GetBool(_T("EnabledLow"), false);
	m_nWebPageRefresh = ini.GetInt(_T("PageRefreshTime"), 120);
	m_bWebIntruderDetection = ini.GetBool(_T("IntruderDetection"), true);
	m_dwWebTempDisableLogin = ini.GetInt(_T("TempDisableLogin"), 30);
	if ((m_dwWebTempDisableLogin - 1u) <= (PREF_MIN_WSDISABLELOGIN - 1))	// < min & != 0
		m_dwWebTempDisableLogin = PREF_MIN_WSDISABLELOGIN;
	if (m_dwWebTempDisableLogin > PREF_MAX_WSDISABLELOGIN)
		m_dwWebTempDisableLogin = PREF_MAX_WSDISABLELOGIN;
	m_dwWSLoginAttemptsAllowed = ini.GetInt(_T("LoginAttemptsAllowed"), 3);
	if ((m_dwWSLoginAttemptsAllowed < PREF_MIN_WSBADLOGINTRIES) || (m_dwWSLoginAttemptsAllowed > PREF_MAX_WSBADLOGINTRIES))
		m_dwWSLoginAttemptsAllowed = 3;
	ini.GetString(&m_strTemplateFile, _T("WebTemplateFile"), _T("eMuleXP.tmpl"));
	m_WSPrefs.bMenuLocked = ini.GetBool(_T("MenuLocked"), false);
	m_WSPrefs.bShowUploadQueue = ini.GetBool(_T("ShowUploadQueue"), false);
	m_WSPrefs.bShowUploadQueueBanned = ini.GetBool(_T("ShowUploadQueueBanned"), false);
	m_WSPrefs.bShowUploadQueueFriend = ini.GetBool(_T("ShowUploadQueueFriend"), false);
	m_WSPrefs.bShowUploadQueueCredit = ini.GetBool(_T("ShowUploadQueueCredit"), false);
	ini.GetArray(m_WSPrefs.abDownloadColHidden, ARRSIZE(m_WSPrefs.abDownloadColHidden), _T("downloadColumnHidden"));
	ini.GetArray(m_WSPrefs.abUploadColHidden, ARRSIZE(m_WSPrefs.abUploadColHidden), _T("uploadColumnHidden"));
	ini.GetArray(m_WSPrefs.abQueueColHidden, ARRSIZE(m_WSPrefs.abQueueColHidden), _T("queueColumnHidden"));
	ini.GetArray(m_WSPrefs.abSharedColHidden, ARRSIZE(m_WSPrefs.abSharedColHidden), _T("sharedColumnHidden"));
	ini.GetArray(m_WSPrefs.abServerColHidden, ARRSIZE(m_WSPrefs.abServerColHidden), _T("serverColumnHidden"));
	ini.GetArray(m_WSPrefs.abSearchColHidden, ARRSIZE(m_WSPrefs.abSearchColHidden), _T("searchColumnHidden"));
	m_WSPrefs.dwDownloadSort = ini.GetUInt32(_T("DownloadSort"), WS_DLCOL_NAME);
	if (m_WSPrefs.dwDownloadSort >= WS_DLCOL_NUMCOLUMNS)
		m_WSPrefs.dwDownloadSort = WS_DLCOL_NAME;
	m_WSPrefs.dwUploadSort = ini.GetUInt32(_T("UploadSort"), WS_ULCOL_USER);
	if (m_WSPrefs.dwUploadSort >= WS_ULCOL_NUMCOLUMNS)
		m_WSPrefs.dwUploadSort = WS_ULCOL_USER;
	m_WSPrefs.dwQueueSort = ini.GetUInt32(_T("QueueSort"), WS_QUCOL_SCORE);
	if (m_WSPrefs.dwQueueSort >= WS_QUCOL_NUMCOLUMNS)
		m_WSPrefs.dwQueueSort = WS_QUCOL_SCORE;
	m_WSPrefs.dwSharedSort = ini.GetUInt32(_T("SharedSort"), WS_SFLCOL_NAME);
	if (m_WSPrefs.dwSharedSort >= WS_SFLCOL_NUMCOLUMNS)
		m_WSPrefs.dwSharedSort = WS_SFLCOL_NAME;
	m_WSPrefs.dwServerSort = ini.GetUInt32(_T("ServerSort"), WS_SRVCOL_NAME);
	if (m_WSPrefs.dwServerSort >= WS_SRVCOL_NUMCOLUMNS)
		m_WSPrefs.dwServerSort = WS_SRVCOL_NAME;
	m_WSPrefs.dwSearchSort = ini.GetUInt32(_T("SearchSort"), WS_SLCOL_SOURCES);
	if (m_WSPrefs.dwSearchSort >= WS_SLCOL_NUMCOLUMNS)
		m_WSPrefs.dwSearchSort = WS_SLCOL_SOURCES;
	ini.GetArray(m_WSPrefs.abDownloadSortAsc, ARRSIZE(m_WSPrefs.abDownloadSortAsc), _T("DownloadSortDir"));
	ini.GetArray(m_WSPrefs.abUploadSortAsc, ARRSIZE(m_WSPrefs.abUploadSortAsc), _T("UploadSortDir"));
	ini.GetArray(m_WSPrefs.abQueueSortAsc, ARRSIZE(m_WSPrefs.abQueueSortAsc), _T("QueueSortDir"));
	ini.GetArray(m_WSPrefs.aiSharedSortAsc, ARRSIZE(m_WSPrefs.aiSharedSortAsc), _T("SharedSortDir"));
	ini.GetArray(m_WSPrefs.abServerSortAsc, ARRSIZE(m_WSPrefs.abServerSortAsc), _T("ServerSortDir"));
	ini.GetArray(m_WSPrefs.abSearchSortAsc, ARRSIZE(m_WSPrefs.abSearchSortAsc), _T("SearchSortDir"));

//	MobileMule
	ini.SetDefaultCategory(_T("MobileMule"));
	ini.GetString(&strBuf, _T("Password"), _T(""));
	StringToHash(strBuf, m_abyteMMPassword);
	m_bMMEnabled = ini.GetBool(_T("Enabled"), false);
	dwVal = ini.GetInt(_T("Port"), PREF_DEF_MM_PORT);
	if ((dwVal - 1u) > 0xFFFEu)
		dwVal = PREF_DEF_MM_PORT;
	m_uMMPort = static_cast<uint16>(dwVal);

//	LANCAST
//	Get the Default Local IP address, using this clients hostname,
//	we will use this ip as the "default" Lancast ip, if no ip is found in the ini file.
	char	acLocalHost[255];

	gethostname(acLocalHost, 255);	// Get this clients hostname

	hostent	*pHost = gethostbyname(acLocalHost);	// Get the host info

	if (pHost != NULL)
		ipstr(&strBuf, *reinterpret_cast<in_addr*>(*pHost->h_addr_list));	// Get this clients default ip as a string
	else
		strBuf = _T("");
	ini.SetDefaultCategory(_T("Lancast"));
	m_bLancastEnabled = ini.GetBool(_T("Enabled"), false);
	m_bUseDefaultIP = ini.GetBool(_T("UseDefaultIP"), true);
	m_dwDefaultLancastIP = inet_addr(strBuf);
	m_dwLancastIP = inet_addr(ini.GetString(_T("IPAddress"), strBuf));
	ini.GetString(&strBuf, _T("Subnet"), _T("255.255.0.0"));
	m_dwLancastSubnet = inet_addr(strBuf);
	dwVal = ini.GetInt(_T("Port"), PREF_DEF_LANCAST_PORT);
	if ((dwVal - 1u) > 0xFFFEu)
		dwVal = PREF_DEF_LANCAST_PORT;
	m_uLancastPort = static_cast<uint16>(dwVal);

//	Cleanup
	ini.SetDefaultCategory(_T("Cleanup"));
	m_bAutoFilenameCleanup = ini.GetBool(_T("AutoFilenameCleanup"), false);
	m_bFilenameCleanupTags = ini.GetBool(_T("CleanupTags"), false);
	ini.GetString(m_sFilenameCleanups.GetRaw(), _T("FilenameCleanups"), _T("http|www.|sharereactor|shareconnector|sharelita|emule-island.com|sharelive|filedonkey|saugstube|eselfilme|eseldownloads|donkey-mania|emulemovies|spanishare|goldesel.6x.to|elitedivx|deviance|filenexus|ac3-guru"));
//	Update current user value with new tokens
//	NB! update value shouldn't be used for more than 2 releases (one is the best)
	ini.GetString(&m_strFilenameCleanupsUpd, _T("FilenameCleanupsUpd"), _T(""));
	UpdateUserStrList(m_sFilenameCleanups.GetRaw(), &m_strFilenameCleanupsUpd, EP_PREFUPD_FILENAME_CLEANUP);

//	SMTP
	ini.SetDefaultCategory(_T("SMTP"));
	ini.GetString(m_sSMTPServer.GetRaw(), _T("Server"), _T("smtp.server.com"));
	ini.GetString(m_sSMTPName.GetRaw(), _T("Name"), CLIENT_NAME _T(" Messenger"));
	ini.GetString(m_sSMTPFrom.GetRaw(), _T("From"), _T("messenger@emuleplus.info"));
	ini.GetString(m_sSMTPTo.GetRaw(), _T("To"), _T("user@domain.com"));
	ini.GetString(m_sSMTPUserName.GetRaw(), _T("UserName"), _T("username"));
	*m_sSMTPPassword.GetRaw() = Decrypt(ini.GetString(_T("Password"), Crypt(_T("password"))));
	m_bSMTPAuthenticated = ini.GetBool(_T("Authenticated"), false);
	m_bSMTPInfo = ini.GetBool(_T("Info"), false);
	m_bSMTPWarning = ini.GetBool(_T("Warning"), false);
	m_bSMTPMsgInSubj = ini.GetBool(_T("MsgInSubject"), false);

//	Backup feature
	ini.SetDefaultCategory(_T("Backup"));
	m_bDatFiles = ini.GetBool(_T("DatFiles"), false);
	m_bMetFiles = ini.GetBool(_T("MetFiles"), false);
	m_bIniFiles = ini.GetBool(_T("IniFiles"), false);
	m_bPartFiles = ini.GetBool(_T("PartFiles"), false);
	m_bPartMetFiles = ini.GetBool(_T("PartMetFiles"), false);
	m_bPartTxtsrcFiles = ini.GetBool(_T("PartTxtsrcFiles"), false);
	m_bAutoBackup = ini.GetBool(_T("AutoBackup"), true);
	m_bBackupOverwrite = ini.GetBool(_T("BackupOverwrite"), true);
	strBuf.Format(_T("%sBackup"), m_strAppDir);
	ini.GetString(m_sBackupDir.GetRaw(), _T("BackupDir"), strBuf);
	m_bScheduledBackup = ini.GetBool(_T("ScheduledBackup"),false);
	m_nScheduledBackupInterval = ini.GetUInt16(_T("ScheduledBackupInterval"),5);

//	FakeCheck
	ini.SetDefaultCategory(_T("FakeCheck"));
	m_nFakesDatVersion		= ini.GetInt(_T("FakesDatVersion"), 0);
	m_bUpdateFakeStartup	= ini.GetBool(_T("UpdateFakeStartup"), false);
	ini.GetString(&m_sFakeListURL, _T("FakeListURL"), _T("http://www.donkeyfakes.net/download/ed2kfakes.txt"));
	m_sFakeListURL.Trim();
	m_dwDLingFakeListVersion = ini.GetInt(_T("DownloadingFakeListVersion"), 0);
	ini.GetString(&m_strDLingFakeListLink, _T("DownloadingFakeListLink"), _T(""));
	m_clrFakeListDownloadColor = ini.GetUInt32(_T("FakeListDownloadColor"), 0);

//	Change old invalid values to current default
	CString strFakeFileName = ::PathFindFileName(m_sFakeListURL);

	if (strFakeFileName.CompareNoCase(_T("ed2kfakes.txt")) != 0)
	{
		m_sFakeListURL = _T("http://www.donkeyfakes.net/download/ed2kfakes.txt");
	}

//	Shortcut management
	ini.SetDefaultCategory(_T("Shortcuts"));
//	VK_RETURN is used as the default value because it is the first element
//	in the dropdown list of the shortcut dialog (in the settings)
	for (int i = 0; i < SCUT_COUNT; i++)
		m_anShortcutCode[i] = static_cast<short>(ini.GetInt(s_apcShortcutNames[i], VK_RETURN));
//	Import old shortcut parameters
	m_anShortcutCode[SCUT_FILE_OPENDIR] = static_cast<short>(ini.GetInt(_T("ShareOpenShortcut"), m_anShortcutCode[SCUT_FILE_OPENDIR]));
	m_anShortcutCode[SCUT_LINK] = static_cast<short>(ini.GetInt(_T("ShareED2KLinkShortcut"), m_anShortcutCode[SCUT_LINK]));
	m_anShortcutCode[SCUT_LINK_HTML] = static_cast<short>(ini.GetInt(_T("ShareED2kLinkHtmlShortcut"), m_anShortcutCode[SCUT_LINK_HTML]));
	m_anShortcutCode[SCUT_LINK_SOURCE] = static_cast<short>(ini.GetInt(_T("ShareED2kLinkSourceShortcut"), m_anShortcutCode[SCUT_LINK_SOURCE]));

	ini.CloseWithoutSave();

	if (strPrefsVersion.IsEmpty())
	{
	//	Application version is empty -> assume that it's the first start
		strBuf = GetResString(IDS_MULTIPLE, m_nLanguageID);

#ifndef NEW_SOCKETS_ENGINE
		m_bMultiple = (IDYES == ::MessageBox(0, strBuf, CLIENT_NAME, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2));	// Make "no" the default
#else
		m_bMultiple = FALSE;
#endif //NEW_SOCKETS_ENGINE
	//	Force saving user selection as we can't start if another instance is running
		if (!m_bMultiple)
			bChangedVersion = true;
	}

//	Now when file is closed, let's call LoadStats()
	// statsitic
	LoadStats(loadstatsFromOld, strFileName);

//	Categories
	LoadCats();
	if (CCat::GetNumCats() == 1)
		CCat::SetAllCatType(CCat::GetCatByIndex(0)->m_eCatID);

	if(bChangedVersion)
		SaveIniPreferences();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadDatPreferences() loads the legacy preference info (version, user hash, window position, etc) from
//		"config/preferences.dat"
void CPreferences::LoadDatPreferences()
{
	Preferences_Ext_Struct	prefsExt;
	CString		strFullPath(m_strConfigDir);

	strFullPath += _T("preferences.dat");

	FILE	*pPrefsFile = _tfsopen(strFullPath, _T("rb"), _SH_DENYWR);

	if (pPrefsFile == NULL)
	{
		SetStandardValues();
		return;
	}

	for (;;)
	{
		if (fread(&prefsExt, sizeof(Preferences_Ext_Struct), 1, pPrefsFile) != 0)
		{
			md4cpy(m_userHash, prefsExt.abyteUserHash);
			if (md4cmp0(m_userHash) != 0)	//regenerate user hash if file content is bogus
			{
				m_WindowPlacement = prefsExt.EmuleWindowPlacement;
				break;
			}
		}
		SetStandardValues();
		break;
	}
	fclose(pPrefsFile);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Load filter words from the file
// Returns true for success, false for failure
bool CPreferences::LoadFilterFile()
{
	CStdioFile	FilterFile;
	CString	strFullPath = m_strConfigDir;

	strFullPath += _T("filter.dat");

//	If filters.txt doesn't exist make a new one and write directions
	if (!PathFileExists(strFullPath))
	{
		static const TCHAR	acHdr[] = _T("# filter search result using listed words\r\n")
			_T("# min 2 characters\r\n")
			_T("# empty lines are ok, '#' for comment\r\n");

		if (!FilterFile.Open(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
			return false;

#ifdef _UNICODE
		const uint16	uBOM = 0xFEFFu;

		FilterFile.Write(&uBOM, sizeof(uBOM));
#endif
		FilterFile.WriteString(acHdr);
	//	File is closed automatically in destructor
		return true;
	}

//	Open filter file
	if (!FilterFile.Open(strFullPath, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
		return false;

#ifdef _UNICODE
	uint16	uBOM;

	FilterFile.Read(&uBOM, sizeof(uBOM));
	if (uBOM != 0xFEFFu)
	{
	//	No Unicode header found, reopen in text mode to import plain text format
		FilterFile.Close();
		if (!FilterFile.Open(strFullPath, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
			return false;
	}
#endif

	CString	strLine;

//	Parse each line
	while (FilterFile.ReadString(strLine))
	{
		strLine.Trim(_T(" \r\n"));

	//	Ignore '#' comment lines and single letters
		if ((strLine.GetLength() > 1) && (_T('#') != strLine[0]))
		{
			m_strFilterWords += _T(' ');
			m_strFilterWords += strLine;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadSharedDirs() loads the shared file paths from "config/shareddir.dat".
void CPreferences::LoadSharedDirs()
{
	CString		strTmp;
	CStdioFile	sDirFile;

	strTmp.Format(_T("%sshareddir.dat"), m_strConfigDir);

	if (sDirFile.Open(strTmp, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
	{
#ifdef _UNICODE
		uint16	uBOM;

		sDirFile.Read(&uBOM, sizeof(uBOM));
		if (uBOM != 0xFEFFu)
		{
		//	No Unicode header found, reopen in text mode to import plain text format
			sDirFile.Close();
			if (!sDirFile.Open(strTmp, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
				return;
		}
#endif
		try
		{
			while (sDirFile.ReadString(strTmp))
				SharedDirListCheckAndAdd(strTmp, true, true);
		}
		catch (...)
		{}	// file system corruption can cause reading error, don't crash here just because of that

		sDirFile.Close();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadTempDirs() loads the supplemental temp directory paths from "config/tempdir.dat".
void CPreferences::LoadTempDirs()
{
	CString			strTmp;
	CStdioFile		tempDirFile;

	strTmp.Format(_T("%stempdir.dat"), m_strConfigDir);

	if (tempDirFile.Open(strTmp, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
	{
#ifdef _UNICODE
		uint16	uBOM;

		tempDirFile.Read(&uBOM, sizeof(uBOM));
		if (uBOM != 0xFEFFu)
		{
		//	No Unicode header found, reopen in text mode to import plain text format
			tempDirFile.Close();
			if (!tempDirFile.Open(strTmp, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
				return;
		}
#endif
		try
		{
			while (tempDirFile.ReadString(strTmp))
				TempDirListCheckAndAdd(strTmp, true);
		}
		catch (...)
		{}	// file system corruption can cause reading error, don't crash here just because of that

		tempDirFile.Close();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::LoadCats()
{
#ifndef NEW_SOCKETS_ENGINE
	CCat::g_iNumPredefinedCats = 0;

	CString	strTmpBuf = m_strConfigDir;

	strTmpBuf += _T("Category.ini");

//	Default cat
	CCat	*pNewCat = new CCat();

	pNewCat->m_strTitle = CCat::GetPredefinedCatTitle(CAT_ALL, GetLanguageID());
	pNewCat->m_strSavePath = GetIncomingDir();
	pNewCat->m_strTempPath = GetTempDir();
	pNewCat->m_strComment = _T("");
	pNewCat->m_strAutoCatExt = _T("");
	pNewCat->m_eCatID = CAT_ALL;
	pNewCat->m_bIsPredefined = true;
	CCat::AddCat(pNewCat, false);
	CCat::g_iNumPredefinedCats++;

	if (!PathFileExists(strTmpBuf))
		return;

	CIni	iniCat(strTmpBuf);

	iniCat.SetDefaultCategory(_T("General"));

	EnumCategories	eCatID;

	for (unsigned ui = 1; ui <= CAT_MAXPREDEFINEDCATS; ui++)
	{
		strTmpBuf.Format(_T("PredefinedCat%u"), ui);
		eCatID = static_cast<_EnumCategories>(iniCat.GetInt(strTmpBuf, CAT_NONE));
		if (eCatID == CAT_NONE)
			break;
		else
		{
			pNewCat = new CCat();

			pNewCat->m_strTitle = CCat::GetPredefinedCatTitle(eCatID,GetLanguageID());
			pNewCat->m_strSavePath = GetIncomingDir();
			pNewCat->m_strTempPath = GetTempDir();
			pNewCat->m_strComment = _T("");
			pNewCat->m_strAutoCatExt = _T("");
			pNewCat->m_eCatID = eCatID;
			pNewCat->m_bIsPredefined = true;
			CCat::AddCat(pNewCat, false);
			CCat::g_iNumPredefinedCats++;
		}
	}

	unsigned	uiVal, uiMax = static_cast<unsigned>(iniCat.GetInt(_T("Count"), 0));

	for (unsigned ui = 1; ui <= uiMax; ui++)
	{
		strTmpBuf.Format(_T("Cat#%u"), ui);
		iniCat.SetDefaultCategory(strTmpBuf);
		pNewCat = new CCat();

		TCHAR	strSavePath[MAX_PATH], strTempPath[MAX_PATH];

		iniCat.GetString(&pNewCat->m_strTitle, _T("Title"), _T(""));
	//	Process Incoming directory, if not set get default
		_tcsncpy(strSavePath, iniCat.GetString(_T("Incoming"), _T("")), ARRSIZE(strSavePath));
		strSavePath[ARRSIZE(strSavePath) - 1] = _T('\0');
		MakeFolderName(strSavePath);
		if (!IsShareableDirectory(strSavePath))
		{
			_tcsncpy(strSavePath, GetIncomingDir(), ARRSIZE(strSavePath));
			strSavePath[ARRSIZE(strSavePath) - 1] = _T('\0');
			MakeFolderName(strSavePath);
		}
		pNewCat->m_strSavePath = strSavePath;
	//	Process Temporary directory, if not set get default
		_tcsncpy(strTempPath, iniCat.GetString(_T("Temp"), GetTempDir()), ARRSIZE(strTempPath));
		strTempPath[ARRSIZE(strTempPath) - 1] = _T('\0');
		MakeFolderName(strTempPath);
		pNewCat->m_strTempPath = strTempPath;

		if (TempDirListCheckAndAdd(pNewCat->m_strTempPath, true) < 0)
		//	There's no such directory in the list and its creation failed
			pNewCat->m_strTempPath = GetTempDir();
		iniCat.GetString(&pNewCat->m_strComment, _T("Comment"), _T(""));
	//	Try to import original eMule setting first
		uiVal = iniCat.GetInt(_T("a4afPriority"), -1) + 1;
		uiVal = iniCat.GetInt(_T("Priority"), uiVal);
		if (uiVal > 4u)
			uiVal = 0;
		pNewCat->m_iPriority = static_cast<byte>(uiVal);
		pNewCat->m_crColor = static_cast<COLORREF>(iniCat.GetUInt32(_T("Color"), RGB(0, 0, 0)));
		iniCat.GetString(&pNewCat->m_strAutoCatExt, _T("AutoCat"), _T(""));
		pNewCat->m_eCatID = static_cast<_EnumCategories>(iniCat.GetInt(_T("CategoryID"), ui));
		pNewCat->m_bIsPredefined = false;

		CCat::AddCat(pNewCat, false);	//	Add category
	}
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LANCAST (moosetea) - get the lancast ip address
uint32 CPreferences::GetLancastIP()
{
	return (GetLancastUseDefaultIP()) ? m_dwDefaultLancastIP : m_dwLancastIP;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CPreferences::GetWindowsVersion()
{
	if (!bWinVerAlreadyDetected)
	{
		bWinVerAlreadyDetected = true;

		bool	bIsNTNBased;

		m_wWinVer = DetectWinVersion(&bIsNTNBased);
		m_bIsNTBased = bIsNTNBased; // <== Hrm. May have misread the member name when I renamed it. MOREVIT
	}
	return m_wWinVer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetDefaultMaxConPerFiveSecs() picks a safe value for the maximum number of connections in a 5 second
//		period based on the OS version.
uint16 CPreferences::GetDefaultMaxConPerFiveSecs()
{
	switch (GetWindowsVersion())
	{
		case _WINVER_98_: /*= _WINVER_NT4_*/
			return (m_bIsNTBased) ? 10 : 5;
		case _WINVER_95_:
			return 5;
		case _WINVER_ME_:
			return 10;
		case _WINVER_2K_:
		case _WINVER_XP_:
		case _WINVER_SE_:	//Win Server 2003
		case _WINVER_VISTA_:
			return 20;
		default:
			return 10;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Provide a mechanism for all tables to store/retrieve sort order
int CPreferences::GetColumnSortItem(EnumTable t) const
{
	switch (t)
	{
		case TABLE_DOWNLOAD:
			return m_iTableSortItemDownload;
		case TABLE_DOWNLOAD2:
			return m_iTableSortItemDownload2;
		case TABLE_UPLOAD:
			return m_iTableSortItemUpload;
		case TABLE_QUEUE:
			return m_iTableSortItemQueue;
		case TABLE_SEARCH:
			return m_iTableSortItemSearch;
		case TABLE_SHARED:
			return m_iTableSortItemShared;
		case TABLE_SERVER:
			return m_iTableSortItemServer;
		case TABLE_IRCNICK:
			return m_iTableSortItemIRCNick;
		case TABLE_IRC:
			return m_iTableSortItemIRCChannel;
		case TABLE_CLIENTLIST:
			return m_iTableSortItemClientList;
		case TABLE_PARTSTATUS:
			return m_iTableSortItemPartStatus;
		case TABLE_FRIENDLIST:
			return m_iTableSortItemFriend;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Provide a mechanism for all tables to store/retrieve sort order
bool CPreferences::GetColumnSortAscending(EnumTable t) const
{
	switch (t)
	{
		case TABLE_DOWNLOAD:
			return m_bTableSortAscendingDownload;
		case TABLE_DOWNLOAD2:
			return m_bTableSortAscendingDownload2;
		case TABLE_UPLOAD:
			return m_bTableSortAscendingUpload;
		case TABLE_QUEUE:
			return m_bTableSortAscendingQueue;
		case TABLE_SEARCH:
			return m_bTableSortAscendingSearch;
		case TABLE_SHARED:
			return m_bTableSortAscendingShared;
		case TABLE_SERVER:
			return m_bTableSortAscendingServer;
		case TABLE_IRCNICK:
			return m_bTableSortAscendingIRCNick;
		case TABLE_IRC:
			return m_bTableSortAscendingIRCChannel;
		case TABLE_CLIENTLIST:
			return m_bTableSortAscendingClientList;
		case TABLE_PARTSTATUS:
			return m_bTableSortAscendingPartStatus;
		case TABLE_FRIENDLIST:
			return m_bTableSortAscendingFriend;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Provide a mechanism for all tables to store/retrieve sort order
void CPreferences::SetColumnSortItem(EnumTable t, int sortItem)
{
	switch (t)
	{
		case TABLE_DOWNLOAD:
			m_iTableSortItemDownload = sortItem;
			break;
		case TABLE_DOWNLOAD2:
			m_iTableSortItemDownload2 = sortItem;
			break;
		case TABLE_UPLOAD:
			m_iTableSortItemUpload = sortItem;
			break;
		case TABLE_QUEUE:
			m_iTableSortItemQueue = sortItem;
			break;
		case TABLE_SEARCH:
			m_iTableSortItemSearch = sortItem;
			break;
		case TABLE_SHARED:
			m_iTableSortItemShared = sortItem;
			break;
		case TABLE_SERVER:
			m_iTableSortItemServer = sortItem;
			break;
		case TABLE_IRCNICK:
			m_iTableSortItemIRCNick = sortItem;
			break;
		case TABLE_IRC:
			m_iTableSortItemIRCChannel = sortItem;
			break;
		case TABLE_CLIENTLIST:
			m_iTableSortItemClientList = sortItem;
			break;
		case TABLE_PARTSTATUS:
			m_iTableSortItemPartStatus = sortItem;
			break;
		case TABLE_FRIENDLIST:
			m_iTableSortItemFriend = sortItem;
			break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Provide a mechanism for all tables to store/retrieve sort order
void CPreferences::SetColumnSortAscending(EnumTable t, bool sortAscending)
{
	switch(t)
	{
		case TABLE_DOWNLOAD:
			m_bTableSortAscendingDownload = sortAscending;
			break;
		case TABLE_DOWNLOAD2:
			m_bTableSortAscendingDownload2 = sortAscending;
			break;
		case TABLE_UPLOAD:
			m_bTableSortAscendingUpload = sortAscending;
			break;
		case TABLE_QUEUE:
			m_bTableSortAscendingQueue = sortAscending;
			break;
		case TABLE_SEARCH:
			m_bTableSortAscendingSearch = sortAscending;
			break;
		case TABLE_SHARED:
			m_bTableSortAscendingShared = sortAscending;
			break;
		case TABLE_SERVER:
			m_bTableSortAscendingServer = sortAscending;
			break;
		case TABLE_IRCNICK:
			m_bTableSortAscendingIRCNick = sortAscending;
			break;
		case TABLE_IRC:
			m_bTableSortAscendingIRCChannel = sortAscending;
			break;
		case TABLE_CLIENTLIST:
			m_bTableSortAscendingClientList = sortAscending;
			break;
		case TABLE_PARTSTATUS:
			m_bTableSortAscendingPartStatus = sortAscending;
			break;
		case TABLE_FRIENDLIST:
			m_bTableSortAscendingFriend = sortAscending;
			break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetLanguage()
{
	switch (m_nLanguageID)
	{
		// read from ini and supported by the program?
		case MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_BASQUE, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_BELARUSIAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_CATALAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED):
		case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL):
		case MAKELANGID(LANG_CROATIAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_DANISH, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_DUTCH, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_SPECIAL, SUBLNG_EXTREMADURAN):
		case MAKELANGID(LANG_FINNISH, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_FRENCH, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_GERMAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_GREEK, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_HUNGARIAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_ITALIAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_LITHUANIAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_MALAY, SUBLANG_MALAY_MALAYSIA):
		case MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL):
		case MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE):
		case MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN):
		case MAKELANGID(LANG_ROMANIAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN):
		case MAKELANGID(LANG_SLOVENIAN, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH):
		case MAKELANGID(LANG_TURKISH,SUBLANG_DEFAULT):
		case MAKELANGID(LANG_UKRAINIAN, SUBLANG_DEFAULT):
			InitThreadLocale();
			break;
		default:
		{
			DWORD	dwPrimLang, dwLocalLang = ::GetThreadLocale();

			dwPrimLang = PRIMARYLANGID(dwLocalLang);
			switch (dwPrimLang)
			{
				case LANG_ENGLISH:
				case LANG_BASQUE:
				case LANG_BELARUSIAN:
				case LANG_CATALAN:
				case LANG_CHINESE:
				case LANG_CZECH:
				case LANG_DANISH:
				case LANG_DUTCH:
				case LANG_FINNISH:
				case LANG_FRENCH:
				case LANG_GERMAN:
				case LANG_GREEK:
				case LANG_HEBREW:
//				case LANG_HUNGARIAN:
				case LANG_ITALIAN:
				case LANG_KOREAN:
				case LANG_LITHUANIAN:
				case LANG_MALAY:
				case LANG_NORWEGIAN:
				case LANG_POLISH:
				case LANG_PORTUGUESE:
				case LANG_ROMANIAN:
				case LANG_RUSSIAN:
				case LANG_SLOVENIAN:
				case LANG_SPANISH:
				case LANG_TURKISH:
				case LANG_UKRAINIAN:
					m_nLanguageID = MAKELANGID(dwPrimLang, SUBLANG_DEFAULT);
					break;

#ifndef SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_CYRILLIC
#define SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_CYRILLIC	0x07
#endif
#ifndef SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_LATIN
#define SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_LATIN	0x06 
#endif
				default:
				//	Damn, M$ for this mess...
					if (dwPrimLang == LANG_SERBIAN)	//LANG_CROATIAN == LANG_SERBIAN
					{
						if ( (dwLocalLang == MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN)) ||
							(dwLocalLang == MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_CYRILLIC)) ||
							(dwLocalLang == MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_LATIN)) ||
							(dwLocalLang == MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_CYRILLIC)) )
						{
							m_nLanguageID = MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN);
						}
						else
							m_nLanguageID = MAKELANGID(LANG_CROATIAN, SUBLANG_DEFAULT);
					}
					else
						m_nLanguageID = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);
			}

			InitThreadLocale();

#ifndef NEW_SOCKETS_ENGINE
			::MessageBox( 0, GetResString(IDS_MB_LANGUAGEINFO, m_nLanguageID),
				GetResString(IDS_PW_LANG, m_nLanguageID), MB_ICONASTERISK );
#endif //NEW_SOCKETS_ENGINE
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetMainProcessPriority(int Value)
{
	m_nMainProcessPriority = Value;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPreferences::IsNTBased(void)
{
	GetWindowsVersion();
	return m_bIsNTBased;
}

void CPreferences::ResetStatistics()
{
	// Save a backup so that we can undo this action
	SaveStats(1);

	// SET ALL CUMULATIVE STAT VALUES TO 0  :'-(

	m_nTotalDownloadedBytes = 0;
	m_nTotalUploadedBytes = 0;
	cumDownOverheadTotal = 0;
	cumDownOverheadFileReq = 0;
	cumDownOverheadSrcEx = 0;
	cumDownOverheadServer = 0;
	cumDownOverheadOther = 0;
	cumDownOverheadTotalPackets = 0;
	cumDownOverheadFileReqPackets = 0;
	cumDownOverheadSrcExPackets = 0;
	cumDownOverheadServerPackets = 0;
	cumDownOverheadOtherPackets = 0;
	cumUpOverheadTotal = 0;
	cumUpOverheadFileReq = 0;
	cumUpOverheadSrcEx = 0;
	cumUpOverheadServer = 0;
	cumUpOverheadOther = 0;
	cumUpOverheadTotalPackets = 0;
	cumUpOverheadFileReqPackets = 0;
	cumUpOverheadSrcExPackets = 0;
	cumUpOverheadServerPackets = 0;
	cumUpOverheadOtherPackets = 0;
	cumUpSuccessfulSessions = 0;
	cumUpFailedSessions = 0;
	cumUpAvgTime = 0;
	cumDownCompletedFiles = 0;
	dwCumDownSuccessfulSessions = 0;
	dwCumDownFailedSessions = 0;
	cumDownAvgTime = 0;
	m_qwCumLostFromCorruption = 0;
	m_qwSesLostFromCorruption = 0;
	m_qwCumSavedFromCompression = 0;
	m_qwSesSavedFromCompression = 0;
	m_dwCumPartsSavedByICH = 0;
	memzero(cumUpDataClients, sizeof(cumUpDataClients));
	memzero(cumDownDataClients, sizeof(cumDownDataClients));
	cumConnAvgDownRate = 0;
	cumConnMaxAvgDownRate = 0;
	cumConnMaxDownRate = 0;
	cumConnAvgUpRate = 0;
	dwCumConnRunTime = 0;
	cumConnNumReconnects = 0;
	cumConnAvgConnections = 0;
	cumConnMaxConnLimitReached = 0;
	cumConnPeakConnections = 0;
	cumConnDownloadTime = 0;
	cumConnUploadTime = 0;
	cumConnTransferTime = 0;
	cumConnServerDuration = 0;
	cumConnMaxAvgUpRate = 0;
	cumConnMaxUpRate = 0;
	cumSrvrsMostWorkingServers = 0;
	cumSrvrsMostUsersOnline = 0;
	cumSrvrsMostFilesAvail = 0;
	cumSharedMostFilesShared = 0;
	m_qwCumSharedLargestShareSize = 0;
	m_qwCumSharedLargestAvgFileSize = 0;
	m_qwCumSharedLargestFileSize = 0;
	cumUpData_File = 0;
	cumUpData_PartFile = 0;
	cumUpData_Community = 0;
	cumUpData_NoCommunity = 0;

	// Set the time of last reset...
	time_t	timeNow;
	
	time(&timeNow);
	stat_datetimeLastReset = timeNow;

	// Save the reset stats
	SaveStats();

	// End Reset Statistics
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Load Statistics
// This used to be integrated in LoadIniPreferences, but it has been altered
// so that it can be used to load the backup created when the stats are reset.
bool CPreferences::LoadStats(int loadBackUp, CString sFileName)
{
	EMULE_TRY

	// loadBackUp is 0 by default
	// loadBackUp = 0: Load the stats normally like we used to do in LoadIniPreferences
	// loadBackUp = 1: Load the stats from statbkup.ini and create a backup of the current stats.  Also, do not initialize session variables.
	// loadBackUp = 2: Load the stats from preferences.ini.old because the version has changed.
	CString		strFullPath;
	CFileFind	findBackUp;
	bool		bAlreadySaved = false;

	switch (loadBackUp)
	{
		case 0:
			strFullPath.Format(_T("%spreferences.ini"), m_strConfigDir);
			break;
		case 1:
			strFullPath.Format(_T("%sstatbkup.ini"), m_strConfigDir);
			if (!findBackUp.FindFile(strFullPath))
				return false;
			SaveStats(2);	// Save our temp backup of current values to statbkuptmp.ini, we will be renaming it at the end of this function.
			break;
		case 2:
			if (sFileName.IsEmpty())
				return false;
			strFullPath = sFileName;
			if (!findBackUp.FindFile(strFullPath))
				return false;
			break;
	}

	CIni	ini(strFullPath);

	ini.SetDefaultCategory(_T("Statistics"));

	m_nTotalDownloadedBytes.Put(ini.GetUInt64(_T("TotalDownloadedBytes"), 0));
	m_nTotalUploadedBytes.Put(ini.GetUInt64(_T("TotalUploadedBytes"), 0));

	// Load stats for cumulative downline overhead
	cumDownOverheadTotal = ini.GetUInt64(_T("DownOverheadTotal"), 0);
	cumDownOverheadFileReq = ini.GetUInt64(_T("DownOverheadFileReq"), 0);
	cumDownOverheadSrcEx = ini.GetUInt64(_T("DownOverheadSrcEx"), 0);
	cumDownOverheadServer = ini.GetUInt64(_T("DownOverheadServer"), 0);
	cumDownOverheadOther = ini.GetUInt64(_T("DownOverheadOther"), 0);
	cumDownOverheadTotalPackets = ini.GetUInt64(_T("DownOverheadTotalPackets"), 0);
	cumDownOverheadFileReqPackets = ini.GetUInt64(_T("DownOverheadFileReqPackets"), 0);
	cumDownOverheadSrcExPackets = ini.GetUInt64(_T("DownOverheadSrcExPackets"), 0);
	cumDownOverheadServerPackets = ini.GetUInt64(_T("DownOverheadServerPackets"), 0);
	cumDownOverheadOtherPackets = ini.GetUInt64(_T("DownOverheadOtherPackets"), 0);

	// Load stats for cumulative upline overhead
	cumUpOverheadTotal = ini.GetUInt64(_T("UpOverheadTotal"), 0);
	cumUpOverheadFileReq = ini.GetUInt64(_T("UpOverheadFileReq"), 0);
	cumUpOverheadSrcEx = ini.GetUInt64(_T("UpOverheadSrcEx"), 0);
	cumUpOverheadServer = ini.GetUInt64(_T("UpOverheadServer"), 0);
	cumUpOverheadOther = ini.GetUInt64(_T("UpOverheadOther"), 0);
	cumUpOverheadTotalPackets = ini.GetUInt64(_T("UpOverheadTotalPackets"), 0);
	cumUpOverheadFileReqPackets = ini.GetUInt64(_T("UpOverheadFileReqPackets"), 0);
	cumUpOverheadSrcExPackets = ini.GetUInt64(_T("UpOverheadSrcExPackets"), 0);
	cumUpOverheadServerPackets = ini.GetUInt64(_T("UpOverheadServerPackets"), 0);
	cumUpOverheadOtherPackets = ini.GetUInt64(_T("UpOverheadOtherPackets"), 0);

	// Load stats for cumulative upline data
	cumUpSuccessfulSessions =	ini.GetUInt32(_T("UpSuccessfulSessions"), 0);
	cumUpFailedSessions =		ini.GetUInt32(_T("UpFailedSessions"), 0);
	cumUpAvgTime =			ini.GetUInt32(_T("UpAvgTime"), 0);

	// Load cumulative client breakdown stats for sent bytes
	cumUpDataClients[SO_EDONKEY] = ini.GetUInt64(_T("UpData_EDONKEY"), 0);
	cumUpDataClients[SO_EDONKEYHYBRID] = ini.GetUInt64(_T("UpData_EDONKEYHYBRID"), 0);
	cumUpDataClients[SO_EMULE] = ini.GetUInt64(_T("UpData_EMULE"), 0);
	cumUpDataClients[SO_PLUS] = ini.GetUInt64(_T("UpData_EMULEPLUS"), 0);
	cumUpDataClients[SO_MLDONKEY] = ini.GetUInt64(_T("UpData_MLDONKEY"), 0);
	cumUpDataClients[SO_XMULE] = ini.GetUInt64(_T("UpData_LMULE"), 0);
	cumUpDataClients[SO_AMULE] = ini.GetUInt64(_T("UpData_AMULE"), 0);
	cumUpDataClients[SO_LPHANT] = ini.GetUInt64(_T("UpData_LPHANT"), 0);
	cumUpDataClients[SO_OLDEMULE] = 0;	// initialize unused
	cumUpDataClients[SO_SHAREAZA] = ini.GetUInt64(_T("UpData_SHAREAZA"),0);
	cumUpDataClients[SO_UNKNOWN] = 0;	// initialize unused

	// Load cumulative source breakdown stats for sent bytes
	cumUpData_File = ini.GetUInt64(_T("UpData_File"), 0);
	cumUpData_PartFile = ini.GetUInt64(_T("UpData_Partfile"), 0);

	// Load cumulative community breakdown stats for sent bytes
	cumUpData_Community = ini.GetUInt64(_T("UpData_Community"), 0);
	cumUpData_NoCommunity = ini.GetUInt64(_T("UpData_NoCommunity"), 0);

	// Load stats for cumulative downline data
	cumDownCompletedFiles = ini.GetUInt32(_T("DownCompletedFiles"), 0);
	dwCumDownSuccessfulSessions = ini.GetInt(_T("DownSuccessfulSessions"), 0);
	dwCumDownFailedSessions = ini.GetInt(_T("DownFailedSessions"), 0);
	cumDownAvgTime = ini.GetUInt32(_T("DownAvgTime"), 0);

	// Cumulative statistics for saved due to compression/lost due to corruption
	m_qwCumLostFromCorruption = ini.GetUInt64(_T("LostFromCorruption"), 0);
	m_qwCumSavedFromCompression = ini.GetUInt64(_T("SavedFromCompression"), 0);
	m_dwCumPartsSavedByICH = ini.GetUInt32(_T("PartsSavedByICH"), 0);

	// Load cumulative client breakdown stats for received bytes
	cumDownDataClients[SO_EDONKEY] = ini.GetUInt64(_T("DownData_EDONKEY"), 0);
	cumDownDataClients[SO_EDONKEYHYBRID] = ini.GetUInt64(_T("DownData_EDONKEYHYBRID"), 0);
	cumDownDataClients[SO_EMULE] = ini.GetUInt64(_T("DownData_EMULE"), 0);
	cumDownDataClients[SO_PLUS] = ini.GetUInt64(_T("DownData_EMULEPLUS"), 0);
	cumDownDataClients[SO_MLDONKEY] = ini.GetUInt64(_T("DownData_MLDONKEY"), 0);
	cumDownDataClients[SO_XMULE] = ini.GetUInt64(_T("DownData_LMULE"), 0);
	cumDownDataClients[SO_AMULE] = ini.GetUInt64(_T("DownData_AMULE"), 0);
	cumDownDataClients[SO_LPHANT] = ini.GetUInt64(_T("DownData_LPHANT"), 0);
	cumDownDataClients[SO_OLDEMULE] = 0;	// eklmn: initialize unused
	cumDownDataClients[SO_SHAREAZA] = ini.GetUInt64(_T("DownData_SHAREAZA"), 0);
	cumDownDataClients[SO_UNKNOWN] = 0;

	// Load stats for cumulative connection data
	cumConnAvgDownRate = ini.GetDouble(_T("ConnAvgDownRate"), 0);
	cumConnMaxAvgDownRate = ini.GetDouble(_T("ConnMaxAvgDownRate"), 0);
	cumConnMaxDownRate = ini.GetDouble(_T("ConnMaxDownRate"), 0);
	cumConnAvgUpRate = ini.GetDouble(_T("ConnAvgUpRate"), 0);
	cumConnMaxAvgUpRate = ini.GetDouble(_T("ConnMaxAvgUpRate"), 0);
	cumConnMaxUpRate = ini.GetDouble(_T("ConnMaxUpRate"), 0);

	dwCumConnRunTime = ini.GetUInt32(_T("ConnRunTime"), 0);
	cumConnTransferTime = ini.GetUInt32(_T("ConnTransferTime"), 0);
	cumConnDownloadTime = ini.GetUInt32(_T("ConnDownloadTime"), 0);
	cumConnUploadTime = ini.GetUInt32(_T("ConnUploadTime"), 0);
	cumConnServerDuration = ini.GetUInt32(_T("ConnServerDuration"), 0);
	cumConnNumReconnects = ini.GetUInt16(_T("ConnNumReconnects"), 0);
	cumConnAvgConnections = ini.GetUInt16(_T("ConnAvgConnections"), 0);
	cumConnMaxConnLimitReached = ini.GetUInt16(_T("ConnMaxConnLimitReached"), 0);
	cumConnPeakConnections = ini.GetUInt16(_T("ConnPeakConnections"), 0);

	// Load date/time of last reset
	stat_datetimeLastReset = ini.GetUInt32(_T("statsDateTimeLastReset"), 0);

	// Smart Load For Restores - Don't overwrite records that are greater than the backed up ones
	if (loadBackUp == 1)
	{
		uint64	qwVal;
		uint32	dwVal;

	//	Load records for servers / network
		if ((dwVal = ini.GetUInt32(_T("SrvrsMostWorkingServers"), 0)) > cumSrvrsMostWorkingServers)
			cumSrvrsMostWorkingServers = static_cast<uint16>(dwVal);
		if ((dwVal = ini.GetUInt32(_T("SrvrsMostUsersOnline"), 0)) > cumSrvrsMostUsersOnline)
			cumSrvrsMostUsersOnline =	dwVal;
		if ((dwVal = ini.GetUInt32(_T("SrvrsMostFilesAvail"), 0)) > cumSrvrsMostFilesAvail)
			cumSrvrsMostFilesAvail = dwVal;

	//	Load records for shared files
		if ((dwVal = ini.GetUInt32(_T("SharedMostFilesShared"), 0)) > cumSharedMostFilesShared)
			cumSharedMostFilesShared = static_cast<uint16>(dwVal);

		if ((qwVal = ini.GetUInt64(_T("SharedLargestShareSize"), 0)) > m_qwCumSharedLargestShareSize)
			m_qwCumSharedLargestShareSize = qwVal;
		if ((qwVal = ini.GetUInt64(_T("SharedLargestAvgFileSize"), 0)) > m_qwCumSharedLargestAvgFileSize)
			m_qwCumSharedLargestAvgFileSize = qwVal;
		if ((qwVal = ini.GetUInt64(_T("SharedLargestFileSize"), 0)) > m_qwCumSharedLargestFileSize)
			m_qwCumSharedLargestFileSize = qwVal;

	//	Check to make sure the backup of the values we just overwrote exists. If so, rename it to the backup file.
	//	This allows us to undo a restore, so to speak, just in case we don't like the restored values...
		CString	sINIBackUp;

		sINIBackUp.Format(_T("%sstatbkuptmp.ini"), m_strConfigDir);
		if (findBackUp.FindFile(sINIBackUp))
		{
			ini.SaveAndClose();
			CFile::Remove(strFullPath); // Remove the backup that we just restored from
			CFile::Rename(sINIBackUp, strFullPath); // Rename our temporary backup to the normal statbkup.ini filename.
			bAlreadySaved = true;
		}

		// Since we know this is a restore, now we should call ShowStatistics to update the data items to the new ones we just loaded.
		// Otherwise user is left waiting around for the tick counter to reach the next automatic update (Depending on setting in prefs)
#ifndef NEW_SOCKETS_ENGINE
		g_App.m_pMDlg->m_dlgStatistics.ShowStatistics();
#endif //NEW_SOCKETS_ENGINE
	}
	// Stupid Load -> Just load the values.
	else
	{
		// Load records for servers / network
		cumSrvrsMostWorkingServers = ini.GetUInt16(_T("SrvrsMostWorkingServers"), 0);
		cumSrvrsMostUsersOnline =	ini.GetUInt32(_T("SrvrsMostUsersOnline"), 0);
		cumSrvrsMostFilesAvail = ini.GetUInt32(_T("SrvrsMostFilesAvail"), 0);

		// Load records for shared files
		cumSharedMostFilesShared = ini.GetUInt16(_T("SharedMostFilesShared"), 0);

		m_qwCumSharedLargestShareSize = ini.GetUInt64(_T("SharedLargestShareSize"), 0);
		m_qwCumSharedLargestAvgFileSize = ini.GetUInt64(_T("SharedLargestAvgFileSize"), 0);
		m_qwCumSharedLargestFileSize = ini.GetUInt64(_T("SharedLargestFileSize"), 0);

		ini.GetString(&m_strStatsExpandedTreeItems, _T("statsExpandedTreeItems"), PREF_DEF_STATREE_MASK);

	//	Initialize new session statistic variables...
		sesDownAvgTime = 0;
		sesDownCompletedFiles = 0;
		memzero(sesDownDataClients, sizeof(sesDownDataClients));
		memzero(sesUpDataClients, sizeof(sesUpDataClients));
		memset(sesUpDataPriority, 0, sizeof(sesUpDataPriority));
		sesUpData_File = 0;
		sesUpData_PartFile = 0;
		sesUpData_Community = 0;
		sesUpData_NoCommunity = 0;
		sesDownSuccessfulSessions = 0;
		sesDownFailedSessions = 0;
		sesDownFailedSessionsNoRequiredData = 0;
		m_qwSesLostFromCorruption = 0;
		m_qwSesSavedFromCompression = 0;
		m_dwSesPartsSavedByICH = 0;
	}

	if (stat_datetimeLastReset == 0)
	{
		time_t timeNow;
		time(&timeNow);
		stat_datetimeLastReset = timeNow;
	}

	// End Load Stats
	if (!bAlreadySaved)
		ini.CloseWithoutSave();

	return true;

	EMULE_CATCH
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This formats the UCT long value that is saved for stat_datetimeLastReset
// If this value is 0 (Never reset), then it returns Unknown.
CString CPreferences::GetStatsLastResetStr()
{
	CString	returnStr;

	if (GetStatsLastResetLng())
	{
		// DonGato: fixed localization of Last reset
		time_t	lastResetDateTime = (time_t) GetStatsLastResetLng();
		COleDateTime statsResetLocalized(lastResetDateTime);
		returnStr = statsResetLocalized.Format(_T("%c"));
	}
	else
		returnStr = GetResString(IDS_UNKNOWN);

	return returnStr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SaveStats(int bBackUp)
{
	// This function saves all of the new statistics in my addon.  It is also used to
	// save backups for the Reset Stats function, and the Restore Stats function (Which is actually LoadStats)
	// bBackUp = 0: DEFAULT; save to preferences.ini
	// bBackUp = 1: Save to statbkup.ini, which is used to restore after a reset
	// bBackUp = 2: Save to statbkuptmp.ini, which is temporarily created during a restore and then renamed to statbkup.ini

	CString	strFullPath = m_strConfigDir;
	double	dTmp;

	switch (bBackUp)
	{
		case 0:
			strFullPath += _T("preferences.ini");
			break;
		case 1:
			strFullPath += _T("statbkup.ini");
			break;
		case 2:
			strFullPath += _T("statbkuptmp.ini");
			break;
	}

	CIni ini(strFullPath);
	ini.SetDefaultCategory(_T("Statistics"));

	// Save cumulative statistics to preferences.ini, going in order as they appear in CStatisticsDlg::ShowStatistics.
	// We do NOT SET the values in prefs struct here.

	// Save Cum Down Data
	ini.SetUInt64(_T("TotalDownloadedBytes"), /* g_App.stat_sessionReceivedBytes + */GetTotalDownloaded());

	// Save Complete Downloads - This is saved and incremented in partfile.cpp.
	ini.SetInt(_T("DownCompletedFiles"), cumDownCompletedFiles);

	// Save Successful Download Sessions
	ini.SetUInt32(_T("DownSuccessfulSessions"), dwCumDownSuccessfulSessions);

	// Save Failed Download Sessions
	ini.SetUInt32(_T("DownFailedSessions"), dwCumDownFailedSessions);
	ini.SetInt(_T("DownAvgTime"), (GetDownC_AvgTime() + GetDownS_AvgTime()) / 2);

	// Cumulative statistics for saved due to compression/lost due to corruption
	ini.SetUInt64(_T("LostFromCorruption"), m_qwCumLostFromCorruption + m_qwSesLostFromCorruption);
	ini.SetUInt64(_T("SavedFromCompression"), m_qwSesSavedFromCompression + m_qwCumSavedFromCompression);
	ini.SetInt(_T("PartsSavedByICH"), m_dwCumPartsSavedByICH + m_dwSesPartsSavedByICH);

	// Save cumulative client breakdown stats for received bytes...
	ini.SetUInt64(_T("DownData_EDONKEY"), GetCumDownData(SO_EDONKEY));
	ini.SetUInt64(_T("DownData_EDONKEYHYBRID"), GetCumDownData(SO_EDONKEYHYBRID));
	ini.SetUInt64(_T("DownData_EMULE"), GetCumDownData(SO_EMULE));
	ini.SetUInt64(_T("DownData_EMULEPLUS"), GetCumDownData(SO_PLUS));
	ini.SetUInt64(_T("DownData_MLDONKEY"), GetCumDownData(SO_MLDONKEY));
	ini.SetUInt64(_T("DownData_LMULE"), GetCumDownData(SO_XMULE));
	ini.SetUInt64(_T("DownData_AMULE"), GetCumDownData(SO_AMULE));
	ini.SetUInt64(_T("DownData_LPHANT"), GetCumDownData(SO_LPHANT));
	ini.SetUInt64(_T("DownData_SHAREAZA"), GetCumDownData(SO_SHAREAZA));

	// Save Cumulative Downline Overhead Statistics
#ifndef NEW_SOCKETS_ENGINE
	if (g_App.m_pDownloadQueue)
	{
		ini.SetUInt64(_T("DownOverheadTotal"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadFileRequest() + g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchange() + g_App.m_pDownloadQueue->GetDownDataOverheadServer() + g_App.m_pDownloadQueue->GetDownDataOverheadOther() + GetDownOverheadTotal() );
		ini.SetUInt64(_T("DownOverheadFileReq"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadFileRequest() + GetDownOverheadFileReq() );
		ini.SetUInt64(_T("DownOverheadSrcEx"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchange() + GetDownOverheadSrcEx() );
		ini.SetUInt64(_T("DownOverheadServer"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadServer() + GetDownOverheadServer() );
		ini.SetUInt64(_T("DownOverheadOther"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadOther() + GetDownOverheadOther() );
		// BavarianSnail - corrected saving of overhead values
		ini.SetUInt64(_T("DownOverheadTotalPackets"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadFileRequestPackets() + g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchangePackets() + g_App.m_pDownloadQueue->GetDownDataOverheadServerPackets() + g_App.m_pDownloadQueue->GetDownDataOverheadOtherPackets() + GetDownOverheadTotalPackets() );
		ini.SetUInt64(_T("DownOverheadFileReqPackets"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadFileRequestPackets() + GetDownOverheadFileReqPackets() );
		ini.SetUInt64(_T("DownOverheadSrcExPackets"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadSourceExchangePackets() + GetDownOverheadSrcExPackets() );
		ini.SetUInt64(_T("DownOverheadServerPackets"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadServerPackets() + GetDownOverheadServerPackets() );
		ini.SetUInt64(_T("DownOverheadOtherPackets"), (uint64)g_App.m_pDownloadQueue->GetDownDataOverheadOtherPackets() + GetDownOverheadOtherPackets() );
	}
#endif //NEW_SOCKETS_ENGINE

	// Save Cumulative Upline Statistics
	ini.SetUInt64(_T("TotalUploadedBytes"), /* g_App.stat_sessionSentBytes + */GetTotalUploaded() );
#ifndef NEW_SOCKETS_ENGINE
	if (g_App.m_pUploadQueue)
	{
		ini.SetInt(_T("UpSuccessfulSessions"), g_App.m_pUploadQueue->GetSuccessfulUpCount()+GetUpSuccessfulSessions());
		ini.SetInt(_T("UpFailedSessions"), g_App.m_pUploadQueue->GetFailedUpCount()+GetUpFailedSessions());
		ini.SetInt(_T("UpAvgTime"), (g_App.m_pUploadQueue->GetAverageUpTime()+GetUpAvgTime())/2);

		// Save Cumulative Upline Overhead Statistics
		ini.SetUInt64(_T("UpOverheadTotal"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadFileRequest() + g_App.m_pUploadQueue->GetUpDataOverheadSourceExchange() + g_App.m_pUploadQueue->GetUpDataOverheadServer() + g_App.m_pUploadQueue->GetUpDataOverheadOther() + GetUpOverheadTotal() );
		ini.SetUInt64(_T("UpOverheadFileReq"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadFileRequest() + GetUpOverheadFileReq() );
		ini.SetUInt64(_T("UpOverheadSrcEx"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadSourceExchange() + GetUpOverheadSrcEx() );
		ini.SetUInt64(_T("UpOverheadServer"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadServer() + GetUpOverheadServer() );
		ini.SetUInt64(_T("UpOverheadOther"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadOther() + GetUpOverheadOther() );
		ini.SetUInt64(_T("UpOverheadTotalPackets"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadFileRequestPackets() + g_App.m_pUploadQueue->GetUpDataOverheadSourceExchangePackets() + g_App.m_pUploadQueue->GetUpDataOverheadServerPackets() + g_App.m_pUploadQueue->GetUpDataOverheadOtherPackets() + GetUpOverheadTotalPackets() );
		ini.SetUInt64(_T("UpOverheadFileReqPackets"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadFileRequestPackets() + GetUpOverheadFileReqPackets() );
		ini.SetUInt64(_T("UpOverheadSrcExPackets"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadSourceExchangePackets() + GetUpOverheadSrcExPackets() );
		ini.SetUInt64(_T("UpOverheadServerPackets"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadServerPackets() + GetUpOverheadServerPackets() );
		ini.SetUInt64(_T("UpOverheadOtherPackets"), (uint64)g_App.m_pUploadQueue->GetUpDataOverheadOtherPackets() + GetUpOverheadOtherPackets() );
	}
#endif //NEW_SOCKETS_ENGINE

	// Save Cumulative Client Breakdown Stats For Sent Bytes
	ini.SetUInt64(_T("UpData_EDONKEY"), GetCumUpData(SO_EDONKEY));
	ini.SetUInt64(_T("UpData_EDONKEYHYBRID"), GetCumUpData(SO_EDONKEYHYBRID));
	ini.SetUInt64(_T("UpData_EMULE"), GetCumUpData(SO_EMULE));
	ini.SetUInt64(_T("UpData_EMULEPLUS"), GetCumUpData(SO_PLUS));
	ini.SetUInt64(_T("UpData_MLDONKEY"), GetCumUpData(SO_MLDONKEY));
	ini.SetUInt64(_T("UpData_LMULE"), GetCumUpData(SO_XMULE));
	ini.SetUInt64(_T("UpData_AMULE"), GetCumUpData(SO_AMULE));
	ini.SetUInt64(_T("UpData_LPHANT"), GetCumUpData(SO_LPHANT));
	ini.SetUInt64(_T("UpData_SHAREAZA"), GetCumUpData(SO_SHAREAZA));

	// Save cumulative source breakdown stats for sent bytes
	ini.SetUInt64(_T("UpData_File"), GetCumUpData_File());
	ini.SetUInt64(_T("UpData_Partfile"), GetCumUpData_PartFile());

	// Save cumulative community breakdown stats for sent bytes
	ini.SetUInt64(_T("UpData_Community"), GetCumUpData_Community() );
	ini.SetUInt64(_T("UpData_NoCommunity"), GetCumUpData_NoCommunity() );

	// Save Cumulative Connection Statistics
#ifndef NEW_SOCKETS_ENGINE
	if(g_App.m_pMDlg != NULL)
	{
		// Download Rate Average
		dTmp = g_App.m_pMDlg->m_dlgStatistics.GetAvgDownloadRate(AVG_TOTAL);
		ini.SetDouble(_T("ConnAvgDownRate"), dTmp);
		// Max Download Rate Average
		if (dTmp > GetConnMaxAvgDownRate())
			Add2ConnMaxAvgDownRate(dTmp);
		ini.SetDouble(_T("ConnMaxAvgDownRate"), GetConnMaxAvgDownRate());
		// Upload Rate Average
		dTmp = g_App.m_pMDlg->m_dlgStatistics.GetAvgUploadRate(AVG_TOTAL);
		ini.SetDouble(_T("ConnAvgUpRate"), dTmp);
		// Max Upload Rate Average
		if (dTmp > GetConnMaxAvgUpRate())
			Add2ConnMaxAvgUpRate(dTmp);
		ini.SetDouble(_T("ConnMaxAvgUpRate"), GetConnMaxAvgUpRate());
	}
	if (g_App.m_pDownloadQueue)
	{
		// Max Download Rate
		dTmp = static_cast<double>(g_App.m_pDownloadQueue->GetDataRate()) / 1024.0;
		if (dTmp > GetConnMaxDownRate())
			Add2ConnMaxDownRate(dTmp);
		ini.SetDouble(_T("ConnMaxDownRate"), GetConnMaxDownRate());
	}
	// Max Upload Rate
	if (g_App.m_pUploadQueue)
	{
		dTmp = static_cast<double>(g_App.m_pUploadQueue->GetDataRate()) / 1024.0;
		if (dTmp > GetConnMaxUpRate())
			Add2ConnMaxUpRate(dTmp);
		ini.SetDouble(_T("ConnMaxUpRate"), GetConnMaxUpRate());
	}
#endif //NEW_SOCKETS_ENGINE

	// Overall Run Time
#ifndef NEW_SOCKETS_ENGINE
	ini.SetUInt32(_T("ConnRunTime"), dwCumConnRunTime + (::GetTickCount() - g_App.stat_starttime) / 1000u);
	// Number of Reconnects
	if (g_App.stat_reconnects>0)
		ini.SetWORD(_T("ConnNumReconnects"), g_App.stat_reconnects - 1 + GetConnNumReconnects());
	else
		ini.SetWORD(_T("ConnNumReconnects"), GetConnNumReconnects());
#endif //NEW_SOCKETS_ENGINE

#ifdef OLD_SOCKETS_ENABLED
	if (g_App.m_pServerConnect)
	{
		// Average Connections
		if (g_App.m_pServerConnect->IsConnected())
			ini.SetUInt32(_T("ConnAvgConnections"), (uint32)(g_App.m_pMDlg->m_dlgStatistics.GetAverageConnections()+cumConnAvgConnections)/2);
	}
	if (g_App.m_pListenSocket)
	{
		// Max Connection Limit Reached
		if (g_App.m_pListenSocket->GetMaxConnectionsReachedCount() > 0)
			ini.SetUInt32(_T("ConnMaxConnLimitReached"), g_App.m_pListenSocket->GetMaxConnectionsReachedCount() + cumConnMaxConnLimitReached);
	}
#endif //OLD_SOCKETS_ENABLED

#ifndef NEW_SOCKETS_ENGINE
	if(g_App.m_pMDlg != NULL)
	{
		// Peak Connections
		if (g_App.m_pMDlg->m_dlgStatistics.GetPeakConnections() > cumConnPeakConnections)
			cumConnPeakConnections = static_cast<uint16>(g_App.m_pMDlg->m_dlgStatistics.GetPeakConnections());
		ini.SetInt(_T("ConnPeakConnections"), cumConnPeakConnections);

		// Time Stuff...
		ini.SetUInt32(_T("ConnTransferTime"), GetConnTransferTime() + g_App.m_pMDlg->m_dlgStatistics.GetTransferTime());
		ini.SetUInt32(_T("ConnUploadTime"), GetConnUploadTime() + g_App.m_pMDlg->m_dlgStatistics.GetUploadTime());
		ini.SetUInt32(_T("ConnDownloadTime"), GetConnDownloadTime() + g_App.m_pMDlg->m_dlgStatistics.GetDownloadTime());
		ini.SetUInt32(_T("ConnServerDuration"), GetConnServerDuration() + g_App.m_pMDlg->m_dlgStatistics.GetServerDuration());
	}
	// Compare and Save Server Records
	uint32 servtotal, servfail, servuser, servfile, servtuser, servtfile, dwSrvLowIdUsers;
	double servocc;
	if (g_App.m_pServerList)
	{
		g_App.m_pServerList->GetServersStatus( servtotal, servfail, servuser, servfile,
			dwSrvLowIdUsers, servtuser, servtfile, servocc );

		if ((servtotal - servfail) > cumSrvrsMostWorkingServers)
			cumSrvrsMostWorkingServers = static_cast<uint16>(servtotal - servfail);
		ini.SetInt(_T("SrvrsMostWorkingServers"), cumSrvrsMostWorkingServers);
		if (servtuser > cumSrvrsMostUsersOnline)
			cumSrvrsMostUsersOnline = servtuser;
		ini.SetInt(_T("SrvrsMostUsersOnline"), cumSrvrsMostUsersOnline);
		if (servtfile > cumSrvrsMostFilesAvail)
			cumSrvrsMostFilesAvail = servtfile;
		ini.SetInt(_T("SrvrsMostFilesAvail"), cumSrvrsMostFilesAvail);
	}

	if (g_App.m_pSharedFilesList)
	{
		// Compare and Save Shared File Records
		if (g_App.m_pSharedFilesList->GetCount()>cumSharedMostFilesShared)
			cumSharedMostFilesShared = g_App.m_pSharedFilesList->GetCount();
		ini.SetInt(_T("SharedMostFilesShared"), cumSharedMostFilesShared);

		uint64 qwLargestFile;
		uint64 qwTotalSz = g_App.m_pSharedFilesList->GetDatasize(&qwLargestFile);

		if (qwTotalSz > m_qwCumSharedLargestShareSize)
			m_qwCumSharedLargestShareSize = qwTotalSz;
		ini.SetUInt64(_T("SharedLargestShareSize"), m_qwCumSharedLargestShareSize);
		if (qwLargestFile > m_qwCumSharedLargestFileSize)
			m_qwCumSharedLargestFileSize = qwLargestFile;
		ini.SetUInt64(_T("SharedLargestFileSize"), m_qwCumSharedLargestFileSize);
		if (g_App.m_pSharedFilesList->GetCount() != 0)
		{
			uint64 qwTmp = qwTotalSz / static_cast<uint64>(g_App.m_pSharedFilesList->GetCount());

			if (qwTmp > m_qwCumSharedLargestAvgFileSize)
				m_qwCumSharedLargestAvgFileSize = qwTmp;
		}
		ini.SetUInt64(_T("SharedLargestAvgFileSize"), m_qwCumSharedLargestAvgFileSize);
		ini.SetUInt32(_T("statsDateTimeLastReset"), stat_datetimeLastReset);
	}
#endif //NEW_SOCKETS_ENGINE

	// These aren't really statistics, but they're a part of my add-on, so we'll save them here and load them in LoadStats
	ini.SetString(_T("statsExpandedTreeItems"), m_strStatsExpandedTreeItems);

	// If we are saving a back-up or a temporary back-up, return now.
	// if (bBackUp != 0) return;

	// by default save chages & close the file
	// if (bBackUp == 0)
		ini.SaveAndClose();

	// End SaveStats
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetRecordStructMembers()
{
	// The purpose of this function is to be called from CStatisticsDlg::ShowStatistics()
	// This was easier than making a bunch of functions to interface with the record
	// members of the prefs struct from ShowStatistics.

	// This function is going to compare current values with previously saved records, and if
	// the current values are greater, the corresponding member of m_prefs will be updated.
	// We will not write to INI here, because this code is going to be called a lot more often
	// than SaveStats()  - Khaos

#ifndef NEW_SOCKETS_ENGINE
	CString buffer;

	// Servers
	uint32 servtotal, servfail, servuser, servfile, servtuser, servtfile, dwSrvLowIdUsers;
	double servocc;
	g_App.m_pServerList->GetServersStatus( servtotal, servfail, servuser, servfile,
		dwSrvLowIdUsers, servtuser, servtfile, servocc );
	if ((servtotal - servfail) > cumSrvrsMostWorkingServers)
		cumSrvrsMostWorkingServers = static_cast<uint16>(servtotal - servfail);
	if (servtuser > cumSrvrsMostUsersOnline)
		cumSrvrsMostUsersOnline = servtuser;
	if (servtfile > cumSrvrsMostFilesAvail)
		cumSrvrsMostFilesAvail = servtfile;

	// Shared Files
	if (g_App.m_pSharedFilesList->GetCount() > cumSharedMostFilesShared)
		cumSharedMostFilesShared = g_App.m_pSharedFilesList->GetCount();

	uint64 qwLargestFile;
	uint64 qwTotalSz = g_App.m_pSharedFilesList->GetDatasize(&qwLargestFile);

	if (qwTotalSz > m_qwCumSharedLargestShareSize)
		m_qwCumSharedLargestShareSize = qwTotalSz;
	if (qwLargestFile > m_qwCumSharedLargestFileSize)
		m_qwCumSharedLargestFileSize = qwLargestFile;
	if (g_App.m_pSharedFilesList->GetCount() != 0)
	{
		uint64 qwTmp = qwTotalSz / static_cast<uint64>(g_App.m_pSharedFilesList->GetCount());

		if (qwTmp > m_qwCumSharedLargestAvgFileSize)
			m_qwCumSharedLargestAvgFileSize = qwTmp;
	}
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::Add2SessionUpTransferData(unsigned uiClientID, bool bFromPF, uint32 dwBytes, bool bCommunity, byte byteFilePrio)
{
	//	This function adds the transferred bytes to the appropriate variables,
	//	as well as to the totals for all clients.
	//	PARAMETERS:
	//	uiClientID - The identifier for which client software sent or received this data, eg SO_EMULE
	//	bFromPF - Applies only to uploads.  True is from partfile, False is from non-partfile.
	//	bytes - Number of bytes sent by the client.  Subtract header before calling.
	//	bCommunity - True is from Community remote user, False is from non-Community.

	sesUpDataClients[(uiClientID < SO_LAST) ? uiClientID : SO_UNKNOWN] += dwBytes;

	//	Add to our total for sent bytes...
#ifndef NEW_SOCKETS_ENGINE
	g_App.UpdateSentBytes(dwBytes);
#endif

	if (bFromPF)
		sesUpData_PartFile += dwBytes;
	else
		sesUpData_File += dwBytes;

	if (bCommunity)
		sesUpData_Community += dwBytes;
	else
		sesUpData_NoCommunity += dwBytes;

	if (byteFilePrio < ARRSIZE(sesUpDataPriority))
		sesUpDataPriority[byteFilePrio] += dwBytes;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::Add2SessionDownTransferData(unsigned uiClientID, uint32 dwBytes)
{
	sesDownDataClients[(uiClientID < SO_LAST) ? uiClientID : SO_UNKNOWN] += dwBytes;

	//	Add to our total for received bytes...
#ifndef NEW_SOCKETS_ENGINE
	g_App.UpdateReceivedBytes(dwBytes);
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SaveCompletedDownloadsStat()
{
	EMULE_TRY

	// This function saves the values for the completed
	// download members to INI.  It is called from
	// CPartfile::PerformFileComplete ...   - Khaos

	CString	strFullPath = m_strConfigDir;

	strFullPath += _T("preferences.ini");

	CIni ini(strFullPath);

	ini.SetDefaultCategory(_T("Statistics"));

	ini.SetUInt32(_T("DownCompletedFiles"), GetDownCompletedFiles());
	ini.SetUInt32(_T("DownSessionCompletedFiles"), GetDownSessionCompletedFiles());

	ini.SaveAndClose();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPreferences::IsShareableDirectory(const CString& rstrDir) const
{
	if(IsInstallationDirectory(rstrDir))
		return false;

	CString strFullPath;

	if(PathCanonicalize(strFullPath.GetBuffer(MAX_PATH), rstrDir))
		strFullPath.ReleaseBuffer();
	else
		strFullPath = rstrDir;

	//	Skip sharing of several special eMule folders
	if(!CompareDirectories(strFullPath, GetTempDir()))			// ".\eMule\Temp"
		return false;

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPreferences::IsInstallationDirectory(const CString& rstrDir) const
{
	CString strFullPath;
	if(PathCanonicalize(strFullPath.GetBuffer(MAX_PATH), rstrDir))
		strFullPath.ReleaseBuffer();
	else
		strFullPath = rstrDir;

	// skip sharing of several special eMule folders
	if (!CompareDirectories(strFullPath, GetAppDir()))					// ".\eMule"
		return true;
	if (!CompareDirectories(strFullPath, GetConfigDir()))				// ".\eMule\Config"
		return true;
	if (!CompareDirectories(strFullPath, GetBackupDir()))				// ".\eMule\Backup"
		return true;
	if (!CompareDirectories(strFullPath, (GetAppDir() + _T("WebServer"))))// ".\eMule\Webserver"
		return true;
	if (!CompareDirectories(strFullPath, (GetAppDir() + _T("Db"))))		// ".\eMule\Db"
		return true;

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SharedDirListCheckAndAdd() checks existence of a directory in the list.
//		Params:
//			strNewDir - new directory;
//			bAdd      - add or not a directory to the list if it wasn't found in it.
//			bVerifyExistence - verify the existence of the directory before addition to the list.
//		Return:
//			TRUE if a directory wasn't found in the list, else FALSE.
bool CPreferences::SharedDirListCheckAndAdd(const CString &strNewDir, bool bAdd, bool bVerifyExistence/*false*/)
{
	CString	strDir = strNewDir;

	strDir.TrimRight(_T(" \r\n"));
//	All list entries has trailing '\'
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');

	m_csDirList.Lock();

	for (POSITION pos = m_sharedDirList.GetHeadPosition(); pos != NULL; )
	{
		if (strDir.CompareNoCase(m_sharedDirList.GetNext(pos)) == 0)
		{
		//	Such directory already exists
			m_csDirList.Unlock();
			return false;
		}
	}
	if (bAdd)
	{
		if (bVerifyExistence)
		{
			DWORD dwFileAttributes = GetFileAttributes(strDir);

		//	If directory exists, add it to the list, otherwise report error
			if ((dwFileAttributes == INVALID_FILE_ATTRIBUTES) || !(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				m_csDirList.Unlock();
				return false;
			}
		}
		m_sharedDirList.AddTail(strDir);
	}

	m_csDirList.Unlock();
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SharedDirListRefill() fills the list with new values.
//		Params:
//			pNewList - a list with new values.
void CPreferences::SharedDirListRefill(CStringList *pNewList)
{
	m_csDirList.Lock();

	m_sharedDirList.RemoveAll();
	m_sharedDirList.AddTail(pNewList);

	m_csDirList.Unlock();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SharedDirListCopy() makes a copy of the current list.
//		Params:
//			pOutputList - output copy.
void CPreferences::SharedDirListCopy(CStringList *pOutputList)
{
	m_csDirList.Lock();

	pOutputList->AddTail(&m_sharedDirList);

	m_csDirList.Unlock();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SharedDirListCmp() compare two lists.
//		Params:
//			pNewList - a list to compare with the current.
//		Return:
//			TRUE if lists are different, else FALSE.
bool CPreferences::SharedDirListCmp(CStringList *pNewList)
{
	bool	bRc = true;

	m_csDirList.Lock();

	if (pNewList->GetCount() == m_sharedDirList.GetCount())
	{
		bRc = false;

		POSITION pos1 = pNewList->GetHeadPosition();

		for (POSITION pos = m_sharedDirList.GetHeadPosition(); pos != NULL; )
		{
			if (m_sharedDirList.GetNext(pos).CompareNoCase(pNewList->GetNext(pos1)) != 0)
			{
				bRc = true;
				break;
			}
		}
	}

	m_csDirList.Unlock();
	return bRc;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	TempDirListCheckAndAdd() checks existence of a directory in the list and create it if required.
//		Params:
//			strNewDir - new directory;
//			bAdd      - create and add a directory to the list if it wasn't found in it.
//		Return:
//			0 - a directory already exists or impossible to add;
//			< 0 - a directory wasn't found in the list (impossible to create if bAdd was true);
//			> 0 - a directory was successfully created and added to the list.
int CPreferences::TempDirListCheckAndAdd(const CString &strNewDir, bool bAdd)
{
	CString	strDir = strNewDir;

	strDir.TrimRight(_T(" \r\n"));
//	All list entries has trailing '\'
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');

//	Don't add the main temporary directory to the list
	if (strDir.CompareNoCase(GetTempDir() + _T('\\')) == 0)
		return 0;

	m_csDirList.Lock();

	for (POSITION pos = m_tempDirList.GetHeadPosition(); pos != NULL; )
	{
		if (strDir.CompareNoCase(m_tempDirList.GetNext(pos)) == 0)
		{
		//	Such directory already exists
			m_csDirList.Unlock();
			return 0;
		}
	}

	int	iRc = -1;

	if (bAdd)
	{
		CreateAllDirectories(&strDir);	//	Create all required directories

	//	If directory was successfully created, add it to the list, otherwise report error
		DWORD dwFileAttributes = GetFileAttributes(strDir);

		if ((dwFileAttributes != INVALID_FILE_ATTRIBUTES) && (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			m_tempDirList.AddTail(strDir);
			iRc = 1;
		}
	}

	m_csDirList.Unlock();
	return iRc;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	TempDirListRefill() fills the list with new values.
//		Params:
//			pNewList - a list with new values.
void CPreferences::TempDirListRefill(CStringList *pNewList)
{
	m_csDirList.Lock();

	m_tempDirList.RemoveAll();
	m_tempDirList.AddTail(pNewList);

	m_csDirList.Unlock();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	TempDirListCopy() makes a copy of the current list.
//		Params:
//			pOutputList - output copy.
void CPreferences::TempDirListCopy(CStringList *pOutputList)
{
	m_csDirList.Lock();

	pOutputList->AddTail(&m_tempDirList);

	m_csDirList.Unlock();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	TempDirListCmp() compare two lists.
//		Params:
//			pNewList - a list to compare with the current.
//		Return:
//			TRUE if lists are different, else FALSE.
bool CPreferences::TempDirListCmp(CStringList *pNewList)
{
	bool	bRc = true;

	m_csDirList.Lock();

	if (pNewList->GetCount() == m_tempDirList.GetCount())
	{
		bRc = false;

		POSITION pos1 = pNewList->GetHeadPosition();

		for (POSITION pos = m_tempDirList.GetHeadPosition(); pos != NULL; )
		{
			if (m_tempDirList.GetNext(pos).CompareNoCase(pNewList->GetNext(pos1)) != 0)
			{
				bRc = true;
				break;
			}
		}
	}

	m_csDirList.Unlock();
	return bRc;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SetUserNickTag() prepares User Nickname data to be used for packets.
void CPreferences::SetUserNickTag()
{
	CMemFile	NameBuff(16);
	CWrTag		tagWr;
	void		*pTmp;

	tagWr.WriteToFile(CT_NAME, m_sNick.Get(), NameBuff, cfUTF8);
	pTmp = m_pNameTag;
	m_dwNameTagSz = static_cast<uint32>(NameBuff.GetLength());
	m_pNameTag = NameBuff.Detach();
	free(pTmp);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetA4AFHash(const byte* pbyteFileHash)
{
	md4cpy(m_A4AF_FileHash, pbyteFileHash);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16 CPreferences::GetMaxSourcePerFileSoft() const
{
	unsigned ui = static_cast<unsigned>(m_dwMaxSourcePerFile) * 9u / 10u;

	return static_cast<uint16>((ui > MAX_SOURCES_FILE_SOFT) ? MAX_SOURCES_FILE_SOFT : ui);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16 CPreferences::GetMaxSourcePerFileUDP() const
{
	unsigned ui = (static_cast<unsigned>(m_dwMaxSourcePerFile) * 3u) >> 2u;

	return static_cast<uint16>((ui > MAX_SOURCES_FILE_UDP)? MAX_SOURCES_FILE_UDP : ui);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetWSPass(const CString &strNewPass)
{
	MD5Sum(strNewPass, m_abyteWebPassword);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetWSLowPass(const CString &strNewPass)
{
	MD5Sum(strNewPass, m_abyteLowWebPassword);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::SetMMPass(const CString &strNewPass)
{
	MD5Sum(strNewPass, m_abyteMMPassword);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferences::InitThreadLocale()
{
//	Set thread locale, this is used for:
//	- MBCS->Unicode conversions (e.g. search results).
//	- Unicode->MBCS conversions (e.g. publishing local files (names) in network, or saving text files on local disk)...
//	Note: this function is not supported on Windows 95/98/Me
	if (m_bIsNTBased)
	{
		LCID lcidSystem = GetSystemDefaultLCID();
		LANGID lidSystem = LANGIDFROMLCID(lcidSystem);
		UINT uSortIdUser = SORT_DEFAULT;
		UINT uSortVerUser = 0;
		LCID lcid = MAKESORTLCID(lidSystem, uSortIdUser, uSortVerUser);

		SetThreadLocale(lcid);
	}

//	If we set the thread locale (see comments above) we also have to specify the proper
//	code page for the C-RTL, otherwise we may not be able to store some strings as MBCS
//	(Unicode->MBCS conversion may fail)
	_tsetlocale(LC_CTYPE, _T(".ACP"));
}
