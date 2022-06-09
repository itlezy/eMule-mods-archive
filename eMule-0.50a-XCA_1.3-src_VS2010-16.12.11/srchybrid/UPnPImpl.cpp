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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "StdAfx.h"
#include "UPnPImpl.h"
#include "OtherFunctions.h" //zz_fly :: show UPnP status
#include "Resource.h" //zz_fly :: show UPnP status

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
UPNPNAT_MAPPING CUPnPImpl::m_Mappings[PI_COUNT];

CUPnPImpl::CUPnPImpl()
:	m_bUPnPPortsForwarded( TRIS_FALSE ),
	m_hResultMessageWindow(0),
	m_nResultMessageID(0)
{
	m_bCheckAndRefresh = false;
}

CUPnPImpl::~CUPnPImpl()
{
}

void CUPnPImpl::SetMessageOnResult(HWND hWindow, UINT nMessageID){
	m_hResultMessageWindow = hWindow;
	m_nResultMessageID = nMessageID;
}

void CUPnPImpl::SendResultMessage(){
	if (m_hResultMessageWindow != 0 && m_nResultMessageID != 0){
		PostMessage(m_hResultMessageWindow, m_nResultMessageID, (WPARAM)(m_bUPnPPortsForwarded == TRIS_TRUE ? UPNP_OK : UPNP_FAILED), m_bCheckAndRefresh ? 1 : 0);
		m_nResultMessageID = 0;
		m_hResultMessageWindow = 0;
	}
}

void CUPnPImpl::ChangePort(PortIndex index, uint16 nPort)
{
	if (m_Mappings[index].internalPort != nPort)
	{
		m_Mappings[index].bChanged = true;
		m_Mappings[index].internalPort = nPort;
		if(ArePortsForwarded() == TRIS_TRUE && IsReady())
			CheckAndRefresh();
	}
}

//zz_fly :: show UPnP status :: start
CString CUPnPImpl::GetStatusString(){
	switch ( ArePortsForwarded() ){
		case TRIS_TRUE:
			return GetResString(IDS_UPNPSTATUS_OK);
		case TRIS_FALSE:
			return m_sStatusString.IsEmpty() ? GetResString(IDS_ERROR) : (GetResString(IDS_ERROR) + _T(':') + m_sStatusString);
		default:
			return GetResString(IDS_UNKNOWN);
	}
}
//zz_fly :: show UPnP status :: end

void CUPnPImpl::InitPortMapping()
{
	m_Mappings[PI_UDP].bChanged = false;
	m_Mappings[PI_UDP].bTCP = false;
	m_Mappings[PI_UDP].internalPort = 0;
	m_Mappings[PI_UDP].externalPort = 0;
	m_Mappings[PI_UDP].description = "eMule UDP";
	m_Mappings[PI_ServerUDP].bChanged = false;
	m_Mappings[PI_ServerUDP].bTCP = false;
	m_Mappings[PI_ServerUDP].internalPort = 0;
	m_Mappings[PI_ServerUDP].externalPort = 0;
	m_Mappings[PI_ServerUDP].description = "eMule Server UDP";
	m_Mappings[PI_TCP].bChanged = false;
	m_Mappings[PI_TCP].bTCP = true;
	m_Mappings[PI_TCP].internalPort = 0;
	m_Mappings[PI_TCP].externalPort = 0;
	m_Mappings[PI_TCP].description = "eMule TCP";
	m_Mappings[PI_WebTCP].bChanged = false;
	m_Mappings[PI_WebTCP].bTCP = true;
	m_Mappings[PI_WebTCP].internalPort = 0;
	m_Mappings[PI_WebTCP].externalPort = 0;
	m_Mappings[PI_WebTCP].description = "eMule Web Interface";
}