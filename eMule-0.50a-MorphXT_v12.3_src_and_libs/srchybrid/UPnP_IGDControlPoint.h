//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
// ----------------------------------------------------------------------
//
// emulEspaña. Added by MoNKi [MoNKi: -UPnPNAT Support-]

#pragma once

#ifndef __CUPNP_IGDCONTROLPOINT_H__
#define __CUPNP_IGDCONTROLPOINT_H__

#include "upnplib\upnp\inc\upnp.h"

#define IGD_DEVICE_TYPE			_T("urn:schemas-upnp-org:device:InternetGatewayDevice:1")
#define WAN_DEVICE_TYPE			_T("urn:schemas-upnp-org:device:WANDevice:1")
#define WANCON_DEVICE_TYPE		_T("urn:schemas-upnp-org:device:WANConnectionDevice:1")
#define	WANIP_SERVICE_TYPE		_T("urn:schemas-upnp-org:service:WANIPConnection:1")
#define	WANPPP_SERVICE_TYPE		_T("urn:schemas-upnp-org:service:WANPPPConnection:1")
#define UPNP_DESCRIPTION_FORMAT	_T("eMule (%s) [%s: %u]")
#define	UPNP_ADVERTISEMENT_DECREMENT	30
#define	UPNP_PORT_LEASETIME				600
#define	UPNP_PORT_LEASETIME_STR			"600"

class CUPnP_IGDControlPoint
{
public:
	bool Init(bool bStopAtFirstConnFound = false);
	bool IsUpnpAcceptsPorts() {return UpnpAcceptsPorts;};
	int  PauseForUpnpCompletion();
    bool SetUPnPNat(bool upnpNat);
	int  GetStatusString(CString & displaystring,bool verbose=false);
	//Commented for a possible future use.
	/*
	typedef struct UPNP_INFO_VAR{
		CString Name;
		CString Value;
	};

	typedef CList<UPNP_INFO_VAR, UPNP_INFO_VAR>	UPNP_INFO_LIST;
	typedef CList<CString, CString>				STR_LIST;
	*/

	class UPNP_SERVICE
	{
	public:
		//Commented for a possible future use.
		/*
		UPNP_INFO_LIST	infoList;
		STR_LIST			Vars;
		*/
		CString				ServiceID;
		CString				ServiceType;
		CString				EventURL;
		Upnp_SID			SubscriptionID;
		CString				ControlURL;
		int					Enabled;	//-1 not initialized, 0 false, 1 true

		UPNP_SERVICE &operator= (UPNP_SERVICE &srv){
			Enabled = srv.Enabled;
			ControlURL = srv.ControlURL;
			EventURL = srv.EventURL;
			ServiceID = srv.ServiceID;
			ServiceType = srv.ServiceType;
			memcpy(SubscriptionID, srv.SubscriptionID, sizeof(Upnp_SID));
			
			//Commented for a possible future use.
			/*
			POSITION pos = srv.infoList.GetHeadPosition();
			while(pos){
				UPNP_INFO_VAR info_org, info_copy;
				info_org = srv.infoList.GetNext(pos);
				info_copy.Name = info_org.Name;
				info_copy.Value = info_org.Value;
				infoList.AddTail(info_copy);
			}

			pos = srv.Vars.GetHeadPosition();
			while(pos){
				CString var_copy;
				var_copy = srv.Vars.GetNext(pos);
				Vars.AddTail(var_copy);
			}
			*/

			return *this;
		}
	};

	typedef struct UPNP_DEVICE{
		//Commented for a possible future use.
		/*
		UPNP_INFO_LIST	infoList;
		*/
	    CString			UDN;
		CString			DescDocURL;
		CString			FriendlyName;
		CString			DevType;
		int				AdvrTimeOut;
		CList<UPNP_SERVICE*, UPNP_SERVICE*>	Services;
		CList<UPNP_DEVICE*, UPNP_DEVICE*> EmbededDevices;
	};

	typedef CList<UPNP_DEVICE*, UPNP_DEVICE*>	DEVICE_LIST;
	typedef CList<UPNP_SERVICE*, UPNP_SERVICE*>	SERVICE_LIST;

	typedef enum{
		UNAT_OK,						// Successfull
		UNAT_ERROR,						// Unknown Error
		UNAT_NOT_OWNED_PORTMAPPING,		// Error, you are trying to remove a port mapping not owned by this class
		UNAT_EXTERNAL_PORT_IN_USE,		// Error, you are trying to add a port mapping with an external port in use
		UNAT_NOT_FOUND					// Port mapping not found
	} UPNPNAT_RETURN;

	typedef enum{
		UNAT_TCP,						// TCP Protocol
		UNAT_UDP						// UDP Protocol
	} UPNPNAT_PROTOCOL;

	typedef struct UPNPNAT_MAPPING{
		WORD internalPort;				// Port mapping internal port
		WORD externalPort;				// Port mapping external port
		UPNPNAT_PROTOCOL protocol;		// Protocol-> TCP (UPNPNAT_PROTOCOL:UNAT_TCP) || UDP (UPNPNAT_PROTOCOL:UNAT_UDP)
		CString description;			// Port mapping description
		bool removeAtEnd;				// Remove port mapping on close?
	};

	typedef struct UPNPNAT_FULLMAPPING{
		WORD internalPort;				// Port mapping internal port
		WORD externalPort;				// Port mapping external port
		UPNPNAT_PROTOCOL protocol;		// Protocol-> TCP (UPNPNAT_PROTOCOL:UNAT_TCP) || UDP (UPNPNAT_PROTOCOL:UNAT_UDP)
		CString internalClient;			// Internal client IP (xxx.xxx.xxx.xxx)
		BOOL enabled;					// Is this mapping enabled?
		DWORD leaseDuration;			// Port mapping lease duration
		CString description;			// Port mapping description
	};

	typedef CList<UPNPNAT_MAPPING, UPNPNAT_MAPPING> MAPPING_LIST;

	UPNPNAT_RETURN	AddPortMapping(UPNPNAT_MAPPING *mapping);
	UPNPNAT_RETURN	AddPortMapping(WORD port, UPNPNAT_PROTOCOL protocol, CString description);
	UPNPNAT_RETURN	DeletePortMapping(UPNPNAT_MAPPING mapping, bool removeFromList = true);
	UPNPNAT_RETURN	DeletePortMapping(WORD port, UPNPNAT_PROTOCOL protocol, CString description, bool removeFromList = true);
	bool			DeleteAllPortMappings();
	void			DeleteAllPortMappingsOnClose();
	
	unsigned int	GetPort();

	// Singleton
	static CUPnP_IGDControlPoint *	GetInstance();
	static UINT  RemoveInstance(LPVOID /*pParam*/ );
private:
	bool UpnpAcceptsPorts; // Accepts ports, even when upnp is not enabled.  So not restart is required on enableing [leuk_he]
	static CString StatusString; // string to show in display (for errors) 
	typedef enum{
		UPNPNAT_ACTION_ADD,
		UPNPNAT_ACTION_DELETE
	} UPNPNAT_ACTIONTYPE;

	typedef struct UPNPNAT_ACTIONPARAM {
		UPNPNAT_ACTIONTYPE type;
		UPNP_SERVICE srv;
		UPNPNAT_MAPPING mapping;
		bool bUpdating;
	};

	// Singleton
	CUPnP_IGDControlPoint(void);
	~CUPnP_IGDControlPoint(void);
	static CUPnP_IGDControlPoint * m_IGDControlPoint;

	//ControlPoint
	static UpnpClient_Handle m_ctrlPoint;

	//Initialized?
	static bool m_bInit;

	//Stop at first WanXXConnection found?
	static bool m_bStopAtFirstService;

	//Indicate if all the mappings has to be removed on close
	static bool m_bClearOnClose;

	//Lists
	static MAPPING_LIST m_Mappings;
	static DEVICE_LIST m_devices;
	static SERVICE_LIST m_knownServices;

	//Locks
	static CCriticalSection m_MappingsLock;
	static CCriticalSection m_devListLock;
	static CCriticalSection m_ActionThreadCS;

	//Threads
	static UINT TimerThreadFunc( LPVOID pParam );
	static UINT ActionThreadFunc( LPVOID pParam );

	//Main Callback Function
	static int				IGD_Callback( Upnp_EventType EventType, void* Event, void* Cookie );

	//IXML Helpers
	static CString			GetFirstDocumentItem(IXML_Document * doc, CString item );
	static CString			GetFirstElementItem( IXML_Element * element, CString item );
	static CString			GetFirstNodeItem( IXML_Node * root_node, CString item );
	static IXML_NodeList*	GetElementsByName(IXML_Document *doc, CString name);
	static IXML_NodeList*	GetElementsByName(IXML_Element *element, CString name);
	static IXML_NodeList*	GetElementsByName(IXML_Node *root_node, CString name);
	static IXML_NodeList*	GetElementsByName(IXML_Node *root_node, CString name, IXML_NodeList **nodelist);
	static IXML_NodeList*	GetDeviceList( IXML_Document * doc );
	static IXML_NodeList*	GetDeviceList( IXML_Element * doc );
	static IXML_NodeList*	GetDeviceList( IXML_Node * doc );
	static IXML_NodeList*	GetServiceList( IXML_Element * element );
	static IXML_NodeList*	GetServiceList( IXML_Node * root_node );

	//Device related functions
	static void				AddDevice( IXML_Document * doc, CString location, int expires);
	static void				RemoveDevice( CString devID );
	static void				RemoveDevice( UPNP_DEVICE *dev );
	static void				CheckTimeouts();

	//Service related functions
	static UPNPNAT_RETURN	AddPortMappingToService(UPNP_SERVICE *srv, UPNPNAT_MAPPING *mapping, bool bIsUpdating = false);
	static UPNPNAT_RETURN	DeletePortMappingFromService(UPNP_SERVICE *srv, UPNPNAT_MAPPING *mapping);
	static UPNPNAT_RETURN	GetSpecificPortMappingEntryFromService(UPNP_SERVICE *srv, UPNPNAT_MAPPING *mapping, UPNPNAT_FULLMAPPING *fullMapping, bool bLog = true);
	static bool				IsServiceEnabled(UPNP_SERVICE *srv);
	static void				OnEventReceived(Upnp_SID sid, int evntkey, IXML_Document * changes );
	static bool				UpdateAllMappings( bool bLockDeviceList = true, bool bUpdating = true );

	//Error functions
	static CString			GetErrDescription(int err);
	static CString			GetErrDescription(IXML_Document* errDoc, int err);

	//IP
	static CString			GetLocalIPStr();
	static bool				IsLANIP(unsigned long nIP);
	static bool				IsLANIP(char *cIP);
   	static CEvent*          InitializingEvent;

};

#endif //__CUPNP_IGDCONTROLPOINT_H__