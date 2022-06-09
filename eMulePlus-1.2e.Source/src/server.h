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
#pragma once

#pragma pack(1)
struct ServerMet_Struct
{
	uint32		m_dwIP;
	uint16		m_uPort;
	uint32		m_dwTagCount;
};

struct ServerMet_StructShort
{
	uint32		m_dwIP;
	uint16		m_uPort;
};
#pragma pack()


// Server TCP flags
#define SRV_TCPFLG_COMPRESSION		0x00000001
#define SRV_TCPFLG_NEWTAGS			0x00000008	//	Server accepts newtags (16.46+)
#define SRV_TCPFLG_UNICODE			0x00000010
#define SRV_TCPFLG_EXT_GETSOURCES	0x00000020	//	Server accepts OP_GETSOURCES containing several files, plus <HASH 16><SIZE 4> (16.44+)
#define SRV_TCPFLG_RELATEDSEARCH	0x00000040	//	17.5+
#define SRV_TCPFLG_TYPETAGINTEGER	0x00000080	//	FT_FILETYPE excepted in numeric format (17.6+)
#define SRV_TCPFLG_LARGEFILES		0x00000100	//	17.8+
#define SRV_TCPFLG_TCPOBFUSCATION	0x00000400	//	17.13+

// Server UDP flags
#define SRV_UDPFLG_EXT_GETSOURCES	0x00000001	//	Server accepts the UDP AskSource coalescing (several files in one OP_GLOBGETSOURCES)
#define SRV_UDPFLG_EXT_GETFILES		0x00000002
#define SRV_UDPFLG_NEWTAGS			0x00000008	//	Server accepts newtags (16.46+)
#define SRV_UDPFLG_UNICODE			0x00000010
#define SRV_UDPFLG_EXT_GETSOURCES2	0x00000020	//	Server accepts OP_GLOBGETSOURCES2
#define SRV_UDPFLG_RELATEDSEARCH	0x00000040	//	Related search's supported by TCP only but capability is still reported to notify clients
#define SRV_UDPFLG_TYPETAGINTEGER	0x00000080	//	FT_FILETYPE excepted in numeric format (17.6+)
#define SRV_UDPFLG_LARGEFILES		0x00000100	//	17.8+
#define SRV_UDPFLG_UDPOBFUSCATION	0x00000200	//	17.13+
#define SRV_UDPFLG_TCPOBFUSCATION	0x00000400	//	17.13+

class CServer
{
public:
	CServer(ServerMet_Struct* in_data);
	CServer(uint16 uPort, const CString &strAddr);
	CServer(const CServer *pOld);
	~CServer();
	const CString&	GetListName() const								{return m_strListName;}
	const CString&	GetFullIP() const								{return m_strFullIP;}
	const CString&	GetAddress() const;
	uint16	GetPort() const									{return m_uPort;}
	void	SetPort(uint16 uPort)							{m_uPort = uPort;}
	uint16	GetAuxPort() const								{return m_uAuxPort;}
	void	SetAuxPort(uint16 uAuxPort)							{m_uAuxPort = uAuxPort;}
	bool	HasPublicAddress() const;
	bool	HasSameAddress(const CServer& server, bool bCheckPort = true) const;
	bool	AddTagFromFile(CFile &servermet);
	void	SetListName(const CString& newname) 			{ m_strListName = newname; }
	void	SetDescription(const CString& newname) 			{ m_strDescription = newname; }
	uint32	GetIP()	const		{return m_dwIP;}
	uint32	GetFiles() const								{return m_dwFiles;}
	uint32	GetNumUsers() const								{return m_dwNumUsers;}
	CString	GetDescription() const							{return m_strDescription;}
	uint32	GetPing() const									{return m_dwPingTime;}
	byte	GetPreferences() const							{ return m_bytePreferences; }
	uint32	GetMaxUsers() const								{return m_dwMaxUsers;}
	void	SetMaxUsers(uint32 in_maxusers) 				{m_dwMaxUsers = in_maxusers;}
	void	SetUserCount(uint32 in_users)					{m_dwNumUsers = in_users;}
	void	SetFileCount(uint32 in_files)					{m_dwFiles = in_files;}
	void	ResetFailedCount()								{m_dwFailedCount = 0;}
	void	AddFailedCount()								{m_dwFailedCount++;}
	uint32	GetFailedCount() const							{return m_dwFailedCount;}
	void	SetIP(uint32 dwNewIP);
	const CString&	GetDynIP() const								{return m_strDynIP;}
	bool	HasDynIP() const								{return !m_strDynIP.IsEmpty();}
	void	SetDynIP(const CString& newdynip)				{m_strDynIP = newdynip; }
	uint32	GetLastPingedTime() const						{return lastpingedtime;}
	void	SetLastPingedTime(uint32 dwTime)				{lastpingedtime = dwTime;}
	uint32	GetRealLastPingedTime() const					{return m_dwRealLastPingedTime;}
	void	SetRealLastPingedTime(uint32 dwTime)			{m_dwRealLastPingedTime = dwTime;}
	uint32	GetLastPinged() const							{return lastpinged;}
	void	SetLastPinged(uint32 in_lastpinged)				{lastpinged = in_lastpinged;}
	byte	GetLastDescPingedCount()						{return lastdescpingedcount;}
	void	SetLastDescPingedCount(bool reset)				{if(reset){lastdescpingedcount=0;}else{lastdescpingedcount++;}}
	void	SetPing(uint32 in_ping)							{m_dwPingTime = in_ping;}
	void	SetPreference(byte byteInPreferences)			{m_bytePreferences = byteInPreferences;}
	void	SetIsStaticMember(bool bIsStatic)				{m_bIsStaticServerMember=bIsStatic;}
	bool	IsStaticMember() const							{return m_bIsStaticServerMember;}
	uint32	GetChallenge() const							{return m_dwChallenge;}
	void	SetChallenge(uint32 dwChallenge)				{m_dwChallenge = dwChallenge;}
	uint32	GetDescReqChallenge() const						{return m_dwDescReqChallenge;}
	void	SetDescReqChallenge(uint32 dwDescReqChallenge)	{m_dwDescReqChallenge = dwDescReqChallenge;}
	uint32	GetSoftMaxFiles() const							{return m_dwSoftMaxFiles;}
	void	SetSoftMaxFiles(uint32 in_dwNumSoftFiles)		{m_dwSoftMaxFiles = in_dwNumSoftFiles;}
	uint32	GetHardMaxFiles() const							{return m_dwHardMaxFiles;}
	void	SetHardMaxFiles(uint32 in_dwNumHardFiles)		{m_dwHardMaxFiles = in_dwNumHardFiles;}
	const	CString& GetVersion() const						{return m_strVersion;}
	void	SetVersion(const CString &strVersion)			{m_strVersion = strVersion;}

	uint32	ed2k2eMule(uint32 dwSrvPref);
	uint32	eMule2ed2k(uint32 dwSrvPref);

	void	SetTCPFlags(uint32 dwFlags)		{m_dwTCPFlags = dwFlags;}
	uint32	GetTCPFlags() const				{return m_dwTCPFlags;}
	void	SetUDPFlags(uint32 dwFlags)		{m_dwUDPFlags = dwFlags;}
	uint32	GetUDPFlags() const				{return m_dwUDPFlags;}

	HICON	GetServerInfo4Tooltips(CString &strInfo);
	CString	GetUsersInfo4Tooltips();

	uint32	GetLowIDUsers() const							{ return m_dwLowIDUsers; }
	void	SetLowIDUsers(uint32 dwLowIDUsers)				{ m_dwLowIDUsers = dwLowIDUsers; }

	uint16	GetObfuscationPortTCP() const					{ return m_uObfuscationPortTCP; }
	void	SetObfuscationPortTCP(uint16 uPort)				{ m_uObfuscationPortTCP = uPort; }
	uint16	GetObfuscationPortUDP() const					{ return m_uObfuscationPortUDP; }
	void	SetObfuscationPortUDP(uint16 uPort)				{ m_uObfuscationPortUDP = uPort; }
	uint32	GetServerKeyUDP() const;
	uint32	GetServerKeyUDPForce() const					{ return m_dwServerKeyUDP; }
	void	SetServerKeyUDP(uint32 dwServerKeyUDP);
	bool	GetCryptPingReplyPending() const				{ return m_bCryptPingReplyPending; }
	void	SetCryptPingReplyPending(bool bVal)				{ m_bCryptPingReplyPending = bVal; }
	uint32	GetServerKeyUDPIP() const						{ return m_dwIPServerKeyUDP; }

	bool	SupportsLargeFilesTCP() const					{ return (m_dwTCPFlags & SRV_TCPFLG_LARGEFILES) != 0; }
	bool	SupportsObfuscationUDP() const					{ return (GetUDPFlags() & SRV_UDPFLG_UDPOBFUSCATION) != 0; }
	bool	SupportsObfuscationTCP() const					{ return (GetObfuscationPortTCP() != 0) && (((GetUDPFlags() & SRV_UDPFLG_TCPOBFUSCATION) != 0) || ((GetTCPFlags() & SRV_TCPFLG_TCPOBFUSCATION) != 0)); }

	CString	GetCountryName() const;
	void	ResetIP2Country();
	bool	HasServerName() const							{ return !_tcsstr(m_strListName, m_strFullIP); }

private:
	uint32		m_dwChallenge;
	uint32		m_dwDescReqChallenge;
	uint32		lastpinged;		//This is to get the ping delay. relative time (ms)
	uint32		lastpingedtime; //This is to decided when we retry the ping. absolute time (seconds) also saved in server.met
	byte		lastdescpingedcount;
	byte		m_bytePreferences;
	uint16		m_uServerCountryIdx;
	uint32		m_dwFiles;
	uint32		m_dwNumUsers;
	uint32		m_dwMaxUsers;
	uint32		m_dwSoftMaxFiles;
	uint32		m_dwHardMaxFiles;
	uint32		m_dwPingTime;
	CString		m_strDescription;
	CString		m_strListName;
	CString		m_strDynIP;
	CString		m_strFullIP;
	CString		m_strVersion;
	uint32		m_dwIP;
	uint16		m_uPort;
	uint16		m_uAuxPort;
	uint32		m_dwFailedCount;
	uint32		m_dwTCPFlags;
	uint32		m_dwUDPFlags;
	uint32		m_dwLowIDUsers;
	uint32		m_dwServerKeyUDP;
	uint32		m_dwIPServerKeyUDP;
	uint16		m_uObfuscationPortTCP;
	uint16		m_uObfuscationPortUDP;
	uint32		m_dwRealLastPingedTime;	// last pinged time without any random modifier
	bool		m_bIsStaticServerMember;
	bool		m_bCryptPingReplyPending;
};
