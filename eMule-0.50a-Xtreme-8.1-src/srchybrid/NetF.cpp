//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

// netfinity: Maximum Segment Size (MSS - Vista only)
// Maximum Segment Size (MSS for TCP connections) and stuff
// Need atleast to include Vista features
#include "stdafx.h"

#if (NTDDI_VERSION < NTDDI_LONGHORN)
#undef NTDDI_VERSION
#define NTDDI_VERSION   NTDDI_LONGHORN
#endif

#include <iphlpapi.h>
#include <iprtrmib.h>
#include <tcpestats.h>

#include "NetF.h"
#include "OtherFunctions.h"
#include "Log.h"

extern "C" {
        typedef ULONG (WINAPI *t_GetPerTcpConnectionEStats)(
                PMIB_TCPROW Row, TCP_ESTATS_TYPE EstatsType,
                PUCHAR Rw, ULONG RwVersion, ULONG RwSize,
                PUCHAR Ros, ULONG RosVersion, ULONG RosSize,
                PUCHAR Rod, ULONG RodVersion, ULONG RodSize);
        typedef DWORD (WINAPI *t_GetBestInterface)(
                IPAddr dwDestAddr,
                PDWORD pdwBestIfIndex);
        typedef DWORD (WINAPI *t_GetBestRoute2)(
                NET_LUID *InterfaceLuid,
                NET_IFINDEX InterfaceIndex,
                const SOCKADDR_INET *SourceAddress,
                const SOCKADDR_INET *DestinationAddress,
                ULONG AddressSortOptions,
                PMIB_IPFORWARD_ROW2 BestRoute,
                SOCKADDR_INET *BestSourceAddress);
        typedef DWORD (WINAPI *t_GetIpPathEntry)(
                PMIB_IPPATH_ROW Row);
        typedef DWORD (WINAPI *t_GetIfEntry)(
                PMIB_IFROW Row);
}

static t_GetPerTcpConnectionEStats     m_pGetPerTcpConnectionEStats;
static t_GetBestInterface              m_pGetBestInterface;
static t_GetBestRoute2                 m_pGetBestRoute2;
static t_GetIpPathEntry                m_pGetIpPathEntry;
static t_GetIfEntry                    m_pGetIfEntry;
        

CNetF	theNetF;

CNetF::CNetF(){
        // IPhelper API
        m_pGetPerTcpConnectionEStats = NULL;
        m_pGetBestInterface = NULL;
        m_pGetBestRoute2 = NULL;
        m_pGetIpPathEntry = NULL;
        m_pGetIfEntry = NULL;
        m_hIPHlp = ::LoadLibrary(_T("Iphlpapi.dll"));
        if (m_hIPHlp != NULL) {
                m_pGetPerTcpConnectionEStats = (t_GetPerTcpConnectionEStats) ::GetProcAddress(m_hIPHlp, "GetPerTcpConnectionEStats");
                m_pGetBestInterface = (t_GetBestInterface) ::GetProcAddress(m_hIPHlp, "GetBestInterface");
                m_pGetBestRoute2 = (t_GetBestRoute2) ::GetProcAddress(m_hIPHlp, "GetBestRoute2");
                m_pGetIpPathEntry = (t_GetIpPathEntry) ::GetProcAddress(m_hIPHlp, "GetIpPathEntry");
                m_pGetIfEntry = (t_GetIfEntry) ::GetProcAddress(m_hIPHlp, "GetIfEntry");
        }
}

CNetF::~CNetF(){
        // Free IPhelper API
        if (m_hIPHlp != NULL)
                ::FreeLibrary(m_hIPHlp);
}

DWORD CNetF::GetMSSFromSocket(SOCKET socket){
        DWORD   dwError;
        if (m_pGetPerTcpConnectionEStats != NULL){
                sockaddr_in     sa_remote = {0};
                int sa_remote_len = sizeof(sa_remote);
                getpeername(socket, reinterpret_cast<sockaddr*>(&sa_remote), &sa_remote_len);
                sockaddr_in     sa_local = {0};
                int sa_local_len = sizeof(sa_local);
                getsockname(socket, reinterpret_cast<sockaddr*>(&sa_local), &sa_local_len);
                if (sa_local.sin_addr.S_un.S_addr == INADDR_ANY || sa_local.sin_addr.S_un.S_addr == INADDR_NONE){
                        const ULONG     bestIfAddr = GetBestInterfaceIP(sa_remote.sin_addr.S_un.S_addr);
                        if (bestIfAddr != INADDR_NONE)
                                sa_local.sin_addr.S_un.S_addr = bestIfAddr;
                }
                MIB_TCPROW tcprow;
                tcprow.dwState = MIB_TCP_STATE_ESTAB;
                tcprow.dwLocalAddr = sa_local.sin_addr.S_un.S_addr;
                tcprow.dwLocalPort = sa_local.sin_port;
                tcprow.dwRemoteAddr = sa_remote.sin_addr.S_un.S_addr;
                tcprow.dwRemotePort = sa_remote.sin_port;
                TCP_ESTATS_SYN_OPTS_ROS_v0 synopts = {0};
                if ((dwError = m_pGetPerTcpConnectionEStats(&tcprow, static_cast<TCP_ESTATS_TYPE>(TcpConnectionEstatsSynOpts), NULL, 0, 0 , reinterpret_cast<PUCHAR>(&synopts), 0, sizeof(synopts), NULL, 0, 0)) == NO_ERROR){
                        const DWORD dwIfMtu = GetBestInterfaceMTU(sa_remote.sin_addr.S_un.S_addr);
                        if (dwIfMtu == 0 || (dwIfMtu - 40) > synopts.MssRcvd)
                                return synopts.MssRcvd;
                        else
                                return dwIfMtu - 40;
                }
                else
                        DebugLogError(_T(__FUNCTION__) _T(": Failed to retrieve MSS option for destination %s: %s"), ipstr(sa_remote.sin_addr), GetErrorMessage(dwError, 1));
        }
        return 0;
}

ULONG CNetF::GetBestInterfaceIP(const ULONG dest_addr){
        SOCKADDR_INET           saBestInterfaceAddress = {0};
        SOCKADDR_INET           saDestinationAddress = {0};
        MIB_IPFORWARD_ROW2      rowBestRoute = {0};
        DWORD   dwBestIfIndex;
        DWORD   dwError;
        saDestinationAddress.Ipv4.sin_family = AF_INET;
        saDestinationAddress.Ipv4.sin_addr.S_un.S_addr = dest_addr;
        if (m_pGetBestInterface != NULL && m_pGetBestRoute2 != NULL){
                if ((dwError = m_pGetBestInterface(dest_addr, &dwBestIfIndex)) == NO_ERROR){
                        if ((dwError = m_pGetBestRoute2(NULL, dwBestIfIndex, NULL, &saDestinationAddress, 0, &rowBestRoute, &saBestInterfaceAddress)) == NO_ERROR){
                                return saBestInterfaceAddress.Ipv4.sin_addr.S_un.S_addr;
                        }
                        else{
                                DebugLogError(_T(__FUNCTION__) _T(": Failed to retrieve best route source ip address to dest %s: %s"), ipstr(dest_addr), GetErrorMessage(dwError, 1));
                        }
                }
                else{
                        DebugLogError(_T(__FUNCTION__) _T(": Failed to retrieve best interface index for destination %s: %s"), ipstr(dest_addr), GetErrorMessage(dwError, 1));
                }
        }
        return INADDR_NONE;
}

DWORD CNetF::GetBestInterfaceMTU(const ULONG dest_addr){
        SOCKADDR_INET           saDestinationAddress = {0};
        MIB_IFROW                       rowIf = {0};
        DWORD   dwBestIfIndex;
        DWORD   dwError;
        saDestinationAddress.Ipv4.sin_family = AF_INET;
        saDestinationAddress.Ipv4.sin_addr.S_un.S_addr = dest_addr;
        if (m_pGetBestInterface != NULL && m_pGetIfEntry != NULL){
                if ((dwError = m_pGetBestInterface(dest_addr, &dwBestIfIndex)) == NO_ERROR){
                        rowIf.dwIndex = dwBestIfIndex;
                        if ((dwError = m_pGetIfEntry(&rowIf)) == NO_ERROR){
                                return rowIf.dwMtu;
                        }
                        else{
                                DebugLogError(_T(__FUNCTION__) _T(": Failed to retrieve best interface mtu to dest %s: %s"), ipstr(dest_addr), GetErrorMessage(dwError, 1));
                        }
                }
                else{
                        DebugLogError(_T(__FUNCTION__) _T(": Failed to retrieve best interface index for destination %s: %s"), ipstr(dest_addr), GetErrorMessage(dwError, 1));
                }
        }
        return 0;
}