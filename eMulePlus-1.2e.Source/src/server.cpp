//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "server.h"
#include "packets.h"
#include "opcodes.h"
#include "emule.h"
#include "ServerList.h"
#include "IP2Country.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServer::CServer(ServerMet_Struct *pServerMet)
{
	m_uPort = pServerMet->m_uPort;
	m_uAuxPort = 0;
	m_dwIP = pServerMet->m_dwIP;
	ipstr(&m_strFullIP, m_dwIP);
	m_dwFiles = 0;
	m_dwNumUsers = 0;
	m_bytePreferences = PR_NORMAL;
	m_dwPingTime = 0;
	m_dwFailedCount = 0;
	lastpinged = 0;
	lastpingedtime = 0;
	lastdescpingedcount = 0;
	m_bIsStaticServerMember = false;
	m_dwMaxUsers=0;
	m_dwSoftMaxFiles = 0;
	m_dwHardMaxFiles = 0;
	m_dwTCPFlags = 0;
	m_dwUDPFlags = 0;
	m_dwDescReqChallenge = 0;
	m_dwLowIDUsers = 0;
	m_dwChallenge = 0;
	m_uServerCountryIdx = g_App.m_pIP2Country->GetCountryFromIP(m_dwIP);
	m_dwServerKeyUDP = 0;
	m_bCryptPingReplyPending = false;
	m_dwIPServerKeyUDP = 0;
	m_uObfuscationPortTCP = 0;
	m_uObfuscationPortUDP = 0;
	m_dwRealLastPingedTime = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServer::CServer(uint16 uPort, const CString &strAddr)
{
	m_uPort = uPort;
	m_uAuxPort = 0;
	if ((m_dwIP = inet_addr(strAddr)) == INADDR_NONE)
	{
		m_strDynIP = strAddr;
		m_dwIP = 0;
	}
	else
		m_strDynIP = _T("");
	ipstr(&m_strFullIP, m_dwIP);
	m_dwFiles = 0;
	m_dwNumUsers = 0;
	m_bytePreferences = PR_NORMAL;
	m_dwPingTime = 0;
	m_dwFailedCount = 0;
	lastpinged = 0;
	lastpingedtime = 0;
	lastdescpingedcount = 0;
	m_bIsStaticServerMember = false;
	m_dwMaxUsers=0;
	m_dwSoftMaxFiles = 0;
	m_dwHardMaxFiles = 0;
	m_dwTCPFlags = 0;
	m_dwUDPFlags = 0;
	m_dwDescReqChallenge = 0;
	m_dwLowIDUsers = 0;
	m_dwChallenge = 0;
	m_uServerCountryIdx = g_App.m_pIP2Country->GetCountryFromIP(m_dwIP);
	m_dwServerKeyUDP = 0;
	m_bCryptPingReplyPending = false;
	m_dwIPServerKeyUDP = 0;
	m_uObfuscationPortTCP = 0;
	m_uObfuscationPortUDP = 0;
	m_dwRealLastPingedTime = 0;
}

CServer::CServer(const CServer *pOld)
{
	m_uPort = pOld->m_uPort;
	m_uAuxPort = pOld->m_uAuxPort;
	m_dwIP = pOld->m_dwIP;
	m_bIsStaticServerMember = pOld->IsStaticMember();
	m_strFullIP = pOld->m_strFullIP;
	m_dwFiles = pOld->m_dwFiles;
	m_dwNumUsers = pOld->m_dwNumUsers;
	m_bytePreferences = pOld->m_bytePreferences;
	m_dwPingTime = pOld->m_dwPingTime;
	m_dwFailedCount = pOld->m_dwFailedCount;
	lastpinged = pOld->lastpinged;
	lastpingedtime = pOld->lastpinged;
	lastdescpingedcount = pOld->lastdescpingedcount;
	m_dwMaxUsers = pOld->m_dwMaxUsers;
	m_strDescription = pOld->m_strDescription;
	m_strListName = pOld->m_strListName;
	m_strDynIP = pOld->m_strDynIP;
	m_dwSoftMaxFiles = pOld->m_dwSoftMaxFiles;
	m_dwHardMaxFiles = pOld->m_dwHardMaxFiles;
	m_strVersion = pOld->m_strVersion;
	m_dwTCPFlags = pOld->m_dwTCPFlags;
	m_dwUDPFlags = pOld->m_dwUDPFlags;
	m_dwDescReqChallenge = pOld->m_dwDescReqChallenge;
	m_dwLowIDUsers = pOld->m_dwLowIDUsers;
	m_dwChallenge = pOld->m_dwChallenge;
	m_uServerCountryIdx = g_App.m_pIP2Country->GetCountryFromIP(m_dwIP);
	m_dwServerKeyUDP = pOld->m_dwServerKeyUDP;
	m_bCryptPingReplyPending = pOld->m_bCryptPingReplyPending;
	m_dwIPServerKeyUDP = pOld->m_dwIPServerKeyUDP;
	m_uObfuscationPortTCP = pOld->m_uObfuscationPortTCP;
	m_uObfuscationPortUDP = pOld->m_uObfuscationPortUDP;
	m_dwRealLastPingedTime = pOld->m_dwRealLastPingedTime;
}

CServer::~CServer()
{
}

bool CServer::AddTagFromFile(CFile &servermet)
{
	CTag	SrvTag;

	SrvTag.FillFromStream(servermet);
	switch (SrvTag.GetTagID())
	{
		case ST_SERVERNAME:
			if (SrvTag.IsStr())
			{
#ifdef _UNICODE
				if (!m_strListName.IsEmpty())
					break;
#endif
				SrvTag.GetStringValue(&m_strListName);
				m_strListName.Remove(_T('\b'));
				m_strListName.Remove(_T('\r'));
				m_strListName.Remove(_T('\t'));
				m_strListName.Trim();
			}
			break;
		case ST_DESCRIPTION:
			if (SrvTag.IsStr())
			{
#ifdef _UNICODE
				if (!m_strDescription.IsEmpty())
					break;
#endif
				SrvTag.GetStringValue(&m_strDescription);
				m_strDescription.Remove(_T('\b'));
				m_strDescription.Remove(_T('\r'));
				m_strDescription.Remove(_T('\t'));
				m_strDescription.Trim();
			}
			break;
		case ST_PREFERENCE:
		//	Import server preferences and translate it (for faster sorting)
			if (SrvTag.IsInt())
				m_bytePreferences = static_cast<byte>(ed2k2eMule(SrvTag.GetIntValue()));
			break;
		case ST_PING:
			if (SrvTag.IsInt())
				m_dwPingTime = SrvTag.GetIntValue();
			break;
		case ST_DYNIP:
			if (SrvTag.IsStr() && !SrvTag.IsStringValueEmpty())
			{
#ifdef _UNICODE
				if (!m_strDynIP.IsEmpty())
					break;
#endif
			//	Set dynIP and reset available (out-dated) IP
				SrvTag.GetStringValue(&m_strDynIP);
				SetIP(0);
			}
			break;
		case ST_FAIL:
			if (SrvTag.IsInt())
				m_dwFailedCount = SrvTag.GetIntValue();
			break;
		case ST_LASTPING:
			if (SrvTag.IsInt())
				m_dwRealLastPingedTime = lastpingedtime = SrvTag.GetIntValue();
			break;
		case ST_MAXUSERS:
			if (SrvTag.IsInt())
				m_dwMaxUsers = SrvTag.GetIntValue();
			break;
		case ST_SOFTFILES:
			if (SrvTag.IsInt())
				m_dwSoftMaxFiles = SrvTag.GetIntValue();
			break;
		case ST_HARDFILES:
			if (SrvTag.IsInt())
				m_dwHardMaxFiles = SrvTag.GetIntValue();
			break;
		case ST_VERSION:
			if (SrvTag.IsStr())
				SrvTag.GetStringValue(&m_strVersion);
			else if (SrvTag.IsInt())
				m_strVersion.Format(_T("%u.%02u"), SrvTag.GetIntValue() >> 16, SrvTag.GetIntValue() & 0xFFFF);
			break;
		case ST_UDPFLAGS:
			if (SrvTag.IsInt())
				m_dwUDPFlags = SrvTag.GetIntValue();
			break;
		case ST_LOWIDUSERS:
			if (SrvTag.IsInt())
				m_dwLowIDUsers = SrvTag.GetIntValue();
			break;
		case ST_UDPKEY:
			if (SrvTag.IsInt())
				m_dwServerKeyUDP = SrvTag.GetIntValue();
			break;
		case ST_UDPKEYIP:
			if (SrvTag.IsInt())
				m_dwIPServerKeyUDP = SrvTag.GetIntValue();
			break;
		case ST_TCPPORTOBFUSCATION:
			if (SrvTag.IsInt() && (SrvTag.GetIntValue() <= 0xFFFF))
				m_uObfuscationPortTCP = static_cast<uint16>(SrvTag.GetIntValue());
			break;
		case ST_UDPPORTOBFUSCATION:
			if (SrvTag.IsInt() && (SrvTag.GetIntValue() <= 0xFFFF))
				m_uObfuscationPortUDP = static_cast<uint16>(SrvTag.GetIntValue());
			break;
		default:
			if (SrvTag.GetTagName() == NULL)
				break;
			if (CmpED2KTagName(SrvTag.GetTagName(), "files") == 0)
			{
				if (SrvTag.IsInt())
					m_dwFiles = SrvTag.GetIntValue();
			}
			else if (CmpED2KTagName(SrvTag.GetTagName(), "users") == 0)
			{
				if (SrvTag.IsInt())
					m_dwNumUsers = SrvTag.GetIntValue();
			}
			else if (CmpED2KTagName(SrvTag.GetTagName(), "AuxPort") == 0)
			{
				if (SrvTag.IsInt())
					m_uAuxPort = static_cast<uint16>(SrvTag.GetIntValue());
			}
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const CString& CServer::GetAddress() const
{
	if (!m_strDynIP.IsEmpty())
		return m_strDynIP;
	else
		return m_strFullIP;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SetIP() sets new IP address of the server
void CServer::SetIP(uint32 dwNewIP)
{
	ipstr(&m_strFullIP, m_dwIP = dwNewIP);
	m_uServerCountryIdx = g_App.m_pIP2Country->GetCountryFromIP(m_dwIP);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServer::HasSameAddress(const CServer& server, bool bCheckPort /*= true*/) const
{
	if ((m_dwIP == server.m_dwIP) && (!bCheckPort || m_uPort == server.m_uPort || m_uPort == 0 || server.m_uPort == 0))
	{
		if (m_dwIP != 0)
			return true;
		else if (m_strDynIP == server.m_strDynIP)
			return true;
	}
	return false;
}

/**
 * convert eDonkey pref values to eMule pref values
 * The correct server preferences (as used by eDonkey) are: Low=2, Normal=0, High=1
 * @param dwSrvPref preference as found in server.met
 * @return preference as used in eMule
 */
uint32 CServer::ed2k2eMule(uint32 dwSrvPref)
{
	return (++dwSrvPref > PR_HIGH) ? PR_LOW : dwSrvPref;
}

/**
 * convert eMule pref values to eDonkey pref values
 * The correct server preferences (as used by eDonkey) are: Low=2, Normal=0, High=1
 * @param dwSrvPref preference as found in eMule
 * @return preference as used in server.met
 */
uint32 CServer::eMule2ed2k(uint32 dwSrvPref)
{
	return (--dwSrvPref > SRV_PR_HIGH) ? SRV_PR_LOW : dwSrvPref;
}

bool CServer::HasPublicAddress() const
{
	in_addr host;

	host.S_un.S_addr = m_dwIP;

	int b1 = host.S_un.S_un_b.s_b1;
	int b2 = host.S_un.S_un_b.s_b2;

	if (b1 == 172 && (b2 >= 16 && b2 <= 31))
		return false;

#ifndef _DEBUG	// Let connect to local addresses (192.168.xx.xx) in debug
	if (b1 == 192 && b2 == 168)
		return false;
#endif //_DEBUG

	if (b1 == 0 || b1 == 10 || b1 == 127 || GetPort() == 0)
		return false;

	return true;
}

CString CServer::GetCountryName() const
{
	return g_App.m_pIP2Country->GetCountryNameByIndex(m_uServerCountryIdx);
}

void CServer::ResetIP2Country()
{
	m_uServerCountryIdx = g_App.m_pIP2Country->GetCountryFromIP(m_dwIP);
}

uint32 CServer::GetServerKeyUDP() const
{
	if ((m_dwIPServerKeyUDP != 0) && (m_dwIPServerKeyUDP == g_App.GetPublicIP()))
		return m_dwServerKeyUDP;
	else
		return 0;
}

void CServer::SetServerKeyUDP(uint32 dwServerKeyUDP)
{
	m_dwServerKeyUDP = dwServerKeyUDP;
	m_dwIPServerKeyUDP = g_App.GetPublicIP();
}

HICON CServer::GetServerInfo4Tooltips(CString &strInfo)
{
	EMULE_TRY

	if (this == NULL)
		return (HICON)NULL;

	CString		strServerCountry, strServerName = GetListName(), strServerDescription = GetDescription();
	CString		strUsers, strLastPing;
	int			iImageIndex = 0;
	CServer		*pCurServer;

	strServerName.Replace(_T("<"), _T("<<"));
	strServerName.Replace(_T("\n"), _T("<br>"));
	strServerDescription.Replace(_T("<"), _T("<<"));
	strServerDescription.Replace(_T("\n"), _T("<br>"));
	if (g_App.m_pIP2Country->ShowCountryFlag())
		strServerCountry.Format(_T(" (<b>%s</b>)"), GetCountryName());
	strInfo.Format(_T("<t=1><b>%s</b><br><t=1><b>%s</b><br><hr=100%%>"), strServerName, strServerDescription);
	if (!GetVersion().IsEmpty())
		strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s"), GetResString(IDS_SERVER_VERSION), GetVersion());
	strUsers.Format(_T("%s (%s)"), CastItoThousands(GetNumUsers()), CastItoThousands(GetMaxUsers()));
	if (m_dwLowIDUsers != 0)
		strUsers.AppendFormat(_T("; <b>%s:</b> %s"), GetResString(IDS_LOWID), CastItoThousands(m_dwLowIDUsers));
	if (m_dwRealLastPingedTime == 0)
		GetResString(&strLastPing, IDS_NEVER);
	else
	{
		SYSTEMTIME		st;

		CTime(m_dwRealLastPingedTime).GetAsSystemTime(st);
		strLastPing = COleDateTime(st).Format(_T("%c"));
	}

	strInfo.AppendFormat( _T("<br><b>%s</b><t>%s:%u%s<br><b>%s:</b><t>%s<br><b>%s:</b><t>%s - %s<br><b>%s:</b><t>%s<br><b>%s:</b><t>%u<br><b>%s:</b><t>%s<br><hr=100%%><br><b>%s:</b><t>%#x"),
		GetResString(IDS_CD_UIP), GetFullIP(), GetPort(), strServerCountry,
		GetResString(IDS_FILES), CastItoThousands(GetFiles()),
		GetResString(IDS_SHAREDFILES), CastItoThousands(GetSoftMaxFiles()), CastItoThousands(GetHardMaxFiles()),
		GetResString(IDS_UUSERS), strUsers,
		GetResString(IDS_PING), GetPing(), GetResString(IDS_LASTPING), strLastPing,
		GetResString(IDS_TT_SRV_UDP1), GetUDPFlags() );

	if ( g_App.m_pServerConnect->IsConnected()
		&& (pCurServer = g_App.m_pServerConnect->GetCurrentServer()) != NULL
		&& pCurServer->GetPort() == GetPort()
		&& _tcsicmp(pCurServer->GetAddress(), GetAddress()) == 0 )
	{
		iImageIndex = (GetFailedCount() == 0) ? 2 : 4;
		strInfo.AppendFormat( _T("<br><b>%s:</b><t>%s<br><b>%s:</b><t>%s"),
			GetResString(IDS_TT_SRV_TCP), YesNoStr(GetTCPFlags() & SRV_TCPFLG_COMPRESSION),
			GetResString(IDS_OBFUSCATION), GetResString((g_App.m_pServerConnect->IsConnectedObfuscated()) ? IDS_ST_ACTIVE : IDS_ST_INACTIVE) );
	}
	else if (GetFailedCount() > 0)
		iImageIndex = 6;

	if (IsStaticMember())
		iImageIndex++;

	return g_App.m_pMDlg->m_wndServer.m_ctlServerList.m_imageList.ExtractIcon(iImageIndex);

	EMULE_CATCH

	return (HICON)NULL;
}

CString CServer::GetUsersInfo4Tooltips()
{
	EMULE_TRY

	if (this == NULL)
		return _T("");

	CString		strInfo, strMyName = g_App.m_pPrefs->GetUserNick(), strMyID;
	uint32		iTotalUsers = 0, iTotalFiles = 0;

	strMyName.Trim();
	strMyName.Replace(_T("\n"), _T("<br>"));
	strMyName.Replace(_T("<"), _T("<<"));
	strMyID = GetResString((g_App.m_pServerConnect->IsLowID()) ? IDS_PRIOLOW : IDS_PRIOHIGH);
	g_App.m_pServerList->GetUserFileStatus(iTotalUsers, iTotalFiles);

	strInfo.Format(_T("<t=1><b>%s</b><br><t=1>%s: %u (<b>%s</b>)<br><hr=100%%><br><b>%s:<t></b>%s (%s)<br><b>%s:<t></b>%s (%s)"),
		strMyName, GetResString(IDS_USERID), g_App.m_pServerConnect->GetClientID(), strMyID,
		GetResString(IDS_UUSERS), CastItoThousands(GetNumUsers()), CastItoThousands(GetMaxUsers()),
		GetResString(IDS_FILES), CastItoThousands(GetFiles()), CastItoThousands(iTotalFiles));

	return strInfo;

	EMULE_CATCH

	return _T("");
}
