//this file is part of eMule Xtreme-Mod (http://www.xtreme-mod.net)
//Copyright (C)2002-2007 Xtreme-Mod (emulextreme@yahoo.de)

//emule Xtreme is a modification of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

//
//
//	Author: Acat-Team
//  
//  last changes:
//  12.06.2006 fixed wrong IP-LAN-Detection (Xman)
//  12.06.2006 fixed warnings due to wrong Datatypes (Xman)
//

#pragma once
#include "UPnPImpl.h"

class CUPnPImplACAT: public CUPnPImpl, public Poco::Thread
{
public:
	CUPnPImplACAT();

	virtual void	StartDiscovery(uint16 nTCPPort, uint16 nUDPPort, uint16 nUDPServerPort, uint16 nTCPWebPort);
	virtual bool	CheckAndRefresh();
	virtual void	StopAsyncFind();
	virtual void	DeletePorts();
	virtual bool	IsReady();
	virtual int		GetImplementationID()									{ return UPNP_IMPL_ACAT; }

protected:
	virtual void run();

	bool		isComplete() const { return !m_controlurl.IsEmpty(); }
	CStringA	GetLocalIPStr();
	DWORD		GetLocalIP();
	bool		Search();
	bool		OpenPort(UPNPNAT_MAPPING&mapping, bool tryRandom, bool bCheckAndRefresh);
	void		DeletePort(UPNPNAT_MAPPING&mapping);
	bool		GetDescription();
	bool		InvokeCommand(const char* name, const char* args);

private:
	bool	m_bSucceededOnce;
	CStringA	m_slocalIP;
	DWORD		m_uLocalIP;

	CStringA		m_name;
	CStringA		m_description;
	CStringA		m_controlurl;
	//CStringA		m_friendlyname;
	//CStringA		m_modelname;
	
	static Poco::FastMutex	m_mutBusy;
	volatile bool	m_bAbortDiscovery;
};