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
#pragma once
#include "UPnPImpl.h"

struct UPNPUrls;
struct IGDdatas;

class CUPnPImplMiniLib: public CUPnPImpl, public Poco::Thread
{
public:
	CUPnPImplMiniLib();
	virtual ~CUPnPImplMiniLib();

	virtual void	StartDiscovery(uint16 nTCPPort, uint16 nUDPPort, uint16 nUDPServerPort);
	virtual bool	CheckAndRefresh();
	virtual void	StopAsyncFind();
	virtual void	DeletePorts();
	virtual bool	IsReady();
	virtual int		GetImplementationID()									{ return UPNP_IMPL_MINIUPNPLIB; }

protected:
	virtual void run();
	bool	OpenPort(UPNPNAT_MAPPING&mapping, bool tryRandom, bool bCheckAndRefresh, char* pachLANIP);
	void	DeletePort(UPNPNAT_MAPPING&mapping);

private:
	bool	m_bSucceededOnce;
	char	m_achLanIP[16];

	UPNPUrls*	m_pURLs;
	IGDdatas*	m_pIGDData;
	
	static Poco::FastMutex	m_mutBusy;
	volatile bool	m_bAbortDiscovery;
};