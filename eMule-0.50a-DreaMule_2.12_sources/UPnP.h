//this file is part of eMule Xtreme-Mod (http://www.xtreme-mod.net)
//Copyright (C)2002-2006 Xtreme-Mod (emulextreme@yahoo.de)

//emule Xtreme is a modification of eMule
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

//
//
//	Author: Acat-Team
//  
//  last changes:
//  12.06.2006 fixed wrong IP-LAN-Detection (Xman)
//  12.06.2006 fixed warnings due to wrong Datatypes (Xman)
//

#ifndef MYUPNP_H_

#pragma once

typedef unsigned long ulong;

class MyUPnP
{
public:
	typedef enum{
		UNAT_OK,						// Successfull
		UNAT_ERROR,						// Error, use GetLastError() to get an error description
		UNAT_NOT_OWNED_PORTMAPPING,		// Error, you are trying to remove a port mapping not owned by this class
		UNAT_EXTERNAL_PORT_IN_USE,		// Error, you are trying to add a port mapping with an external port in use
		UNAT_NOT_IN_LAN					// Error, you aren't in a LAN -> no router or firewall
	} UPNPNAT_RETURN;

	typedef enum{
		UNAT_TCP,						// TCP Protocol
		UNAT_UDP						// UDP Protocol
	} UPNPNAT_PROTOCOL;

	typedef struct{
		WORD internalPort;				// Port mapping internal port
		WORD externalPort;				// Port mapping external port
		UPNPNAT_PROTOCOL protocol;		// Protocol-> TCP (UPNPNAT_PROTOCOL:UNAT_TCP) || UDP (UPNPNAT_PROTOCOL:UNAT_UDP)
		CString description;			// Port mapping description
	} UPNPNAT_MAPPING;

	MyUPnP();
	~MyUPnP();

	UPNPNAT_RETURN AddNATPortMapping(UPNPNAT_MAPPING *mapping, bool tryRandom = false);
	UPNPNAT_RETURN RemoveNATPortMapping(UPNPNAT_MAPPING mapping, bool removeFromList = true);
	void clearNATPortMapping();
	bool RemoveSpecifiedPort(WORD port,UPNPNAT_PROTOCOL protocol);

	CString		GetLastError();
	CString		GetLocalIPStr();
	DWORD		GetLocalIP();
	bool		IsLANIP(DWORD nIP);

protected:
	void		InitLocalIP();
	void		SetLastError(CString error);

	bool addPortmap(int eport, int iport, const CString& iclient,
					   const CString& descri, const CString& type);
	bool deletePortmap(int eport, const CString& type);

	bool		isComplete() const { return !m_controlurl.IsEmpty(); }

	bool		Search(int version=1);
	bool		GetDescription();
	CString		GetProperty(const CString& name, CString& response);
	bool		InvokeCommand(const CString& name, const CString& args);

	bool		Valid()const{return (!m_name.IsEmpty()&&!m_description.IsEmpty());}
	bool		InternalSearch(int version);
	CString		m_devicename;
	CString		m_name;
	CString		m_description;
	CString		m_baseurl;
	CString		m_controlurl;
	CString		m_friendlyname;
	CString		m_modelname;
	int			m_version;

private:
	CList<UPNPNAT_MAPPING, UPNPNAT_MAPPING> m_Mappings;

	CString		m_slocalIP;
	CString		m_slastError;
	DWORD		m_uLocalIP;

	bool		isSearched;
};
#endif
