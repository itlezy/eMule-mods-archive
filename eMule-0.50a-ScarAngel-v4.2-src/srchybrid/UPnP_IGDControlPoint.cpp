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

#include "StdAfx.h"
#include "emule.h"
#include "preferences.h"
#include "FirewallOpener.h"
#include "otherfunctions.h"
#include "upnp_igdcontrolpoint.h"
#include "upnplib\upnp\inc\upnptools.h"
#include "Log.h"

#include "emuleDlg.h" // for WEBGUIIA_UPDATEMYINFO
#include "UserMsgs.h" // for webguiintercation
#include "resource.h" // for myinfo text

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Static Variables
CUPnP_IGDControlPoint*					CUPnP_IGDControlPoint::m_IGDControlPoint;
UpnpClient_Handle						CUPnP_IGDControlPoint::m_ctrlPoint;
bool									CUPnP_IGDControlPoint::m_bInit;
CUPnP_IGDControlPoint::DEVICE_LIST		CUPnP_IGDControlPoint::m_devices;
CUPnP_IGDControlPoint::SERVICE_LIST		CUPnP_IGDControlPoint::m_knownServices;
CCriticalSection						CUPnP_IGDControlPoint::m_devListLock;
CUPnP_IGDControlPoint::MAPPING_LIST		CUPnP_IGDControlPoint::m_Mappings;
CCriticalSection						CUPnP_IGDControlPoint::m_MappingsLock;
CCriticalSection						CUPnP_IGDControlPoint::m_ActionThreadCS;
bool									CUPnP_IGDControlPoint::m_bStopAtFirstService;
bool									CUPnP_IGDControlPoint::m_bClearOnClose;
CEvent *								CUPnP_IGDControlPoint::InitializingEvent;
CString                                 CUPnP_IGDControlPoint::StatusString;

CUPnP_IGDControlPoint::CUPnP_IGDControlPoint(void)
{
	// Initialize variables
	m_IGDControlPoint = NULL;
	m_ctrlPoint = 0;
	m_bInit = false;
	m_bStopAtFirstService = false;
	m_bClearOnClose = false;
	UpnpAcceptsPorts = true;
	InitializingEvent = NULL;
}

CUPnP_IGDControlPoint::~CUPnP_IGDControlPoint(void)
{
	if(m_bClearOnClose)
		DeleteAllPortMappings();
    
	//Unregister control point and finish UPnP
	if(m_ctrlPoint){
		UpnpUnRegisterClient(m_ctrlPoint);
        UpnpFinish();
	}

    if (InitializingEvent)
	{  InitializingEvent->SetEvent(); 
		delete InitializingEvent; InitializingEvent=NULL;}
	//Remove devices/services
    //Lock devices and mappings before use it
	m_devListLock.Lock(); 
	m_MappingsLock.Lock();


	POSITION pos = m_devices.GetHeadPosition();
	while(pos){
		UPNP_DEVICE *item;
		item = m_devices.GetNext(pos);
		if(item)
			RemoveDevice(item);
	}
	m_devices.RemoveAll();
	m_knownServices.RemoveAll();

	//Remove mappings
	m_Mappings.RemoveAll();

	//Unlock devices and mappings
	m_MappingsLock.Unlock();
	m_devListLock.Unlock();
}

// Eanble or disbale upnp. Note that this is indepednge of accepting ports to map. 
bool CUPnP_IGDControlPoint::SetUPnPNat(bool upnpNat)
{
	if (upnpNat==true && thePrefs.IsUPnPEnabled()==false ) {
		thePrefs.m_bUPnPNat=true;
		Init(thePrefs.GetUPnPLimitToFirstConnection()); 
		UpdateAllMappings(true,false); // send any queued mappings to device. 
	    if (theApp.emuledlg->GetSafeHwnd()!= NULL) // display status window:
			PostMessage(theApp.emuledlg->GetSafeHwnd(),WEB_GUI_INTERACTION,WEBGUIIA_UPDATEMYINFO,0); // update myinfo if device detected. (from different thread!)

	}
	else if (upnpNat==false && thePrefs.IsUPnPEnabled()==true ){
   		DeleteAllPortMappingsOnClose(); // idependand of setting thePrefs.GetUPnPClearOnClose
	    // Note that devices are not removed. 
		thePrefs.m_bUPnPNat=false;
	    if (theApp.emuledlg->GetSafeHwnd()!= NULL) // display status window:
			PostMessage(theApp.emuledlg->GetSafeHwnd(),WEB_GUI_INTERACTION,WEBGUIIA_UPDATEMYINFO,0); // update myinfo if device detected. (from different thread!)
	    }
  return thePrefs.m_bUPnPNat;
}

// Initialize all UPnP thing
bool CUPnP_IGDControlPoint::Init(bool bStopAtFirstConnFound){
	if(m_bInit)
		return true;

	// Init UPnP
	int rc;
	//MORPH START leuk_he upnp bindaddr
	StatusString=L"Starting";
    LPCSTR HostIp=NULL;
	if (   (thePrefs.GetBindAddrA()!=NULL)
		&& IsLANIP((char *) thePrefs.GetBindAddrA()) )
		HostIp=thePrefs.GetBindAddrA();
	else if  ( thePrefs.GetUpnpBindAddr()!= 0 )
		HostIp=_strdup(ipstrA(htonl(thePrefs.GetUpnpBindAddr()))); //Fafner: avoid C4996 (as in 0.49b vanilla) - 080731
	if ((HostIp!= NULL) && (inet_addr(HostIp)==INADDR_NONE))
		HostIp=NULL; // prevent failing if in prev version there was no valid interface. 
	rc = UpnpInit( HostIp, thePrefs.GetUPnPPort() );
    /*
	rc = UpnpInit( NULL, thePrefs.GetUPnPPort() );
	*/
	// MORPH END leuk_he upnp bindaddr 
	if (UPNP_E_SUCCESS != rc) {
		AddLogLine(false, GetResString(IDS_UPNP_FAILEDINIT), thePrefs.GetUPnPPort(), GetErrDescription(rc) );
		StatusString.Format(GetResString(IDS_UPNP_FAILEDINIT), thePrefs.GetUPnPPort(), GetErrDescription(rc) );
		UpnpFinish();
		thePrefs.SetUpnpDetect(UPNP_NOT_DETECTED);//leuk_he autodetect upnp in wizard
		
		return false;
	}

	// Check if you are in a LAN or directly connected to Internet
	if(!IsLANIP(UpnpGetServerIpAddress())){
		AddLogLine(false, GetResString(IDS_UPNP_PUBLICIP));
		StatusString=GetResString(IDS_UPNP_PUBLICIP);
		UpnpFinish();
		thePrefs.SetUpnpDetect(UPNP_NOT_NEEDED)	;//leuk_he autodetect upnp in wizard
		UpnpAcceptsPorts=false; 
		return false;
	}

	// Register us as a Control Point
	rc = UpnpRegisterClient( (Upnp_FunPtr)IGD_Callback, &m_ctrlPoint, &m_ctrlPoint );
	if (UPNP_E_SUCCESS != rc) {
		AddLogLine(false, GetResString(IDS_UPNP_FAILEDREGISTER), GetErrDescription(rc) );
		StatusString.Format(GetResString(IDS_UPNP_FAILEDREGISTER), GetErrDescription(rc));
		UpnpFinish();
		thePrefs.SetUpnpDetect(UPNP_NOT_DETECTED);//leuk_he autodetect upnp in wizard
		return false;
	}
    InitializingEvent = new CEvent(True,True); //Wait for upnp init completion to preven false low


	//Starts timer thread
	AfxBeginThread(TimerThreadFunc, this);

	//Open UPnP Server Port on Windows Firewall
	if(thePrefs.IsOpenPortsOnStartupEnabled()){
		if (theApp.m_pFirewallOpener->OpenPort(UpnpGetServerPort(), NAT_PROTOCOL_UDP, EMULE_DEFAULTRULENAME_UPNP_UDP, true))
			if(thePrefs.GetUPnPVerboseLog())
				theApp.QueueDebugLogLine(false, GetResString(IDS_FO_TEMPUDP_S), UpnpGetServerPort());
		else
			if(thePrefs.GetUPnPVerboseLog())
				theApp.QueueDebugLogLine(false, GetResString(IDS_FO_TEMPUDP_F), UpnpGetServerPort());

		if (theApp.m_pFirewallOpener->OpenPort(UpnpGetServerPort(), NAT_PROTOCOL_TCP, EMULE_DEFAULTRULENAME_UPNP_TCP, true))
			if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, GetResString(IDS_FO_TEMPTCP_S), UpnpGetServerPort());
		else
			if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, GetResString(IDS_FO_TEMPTCP_F), UpnpGetServerPort());
	}

	m_bInit = true;
	AddLogLine(false, GetResString(IDS_UPNP_INIT), GetLocalIPStr(), UpnpGetServerPort());

	m_bStopAtFirstService = bStopAtFirstConnFound;

	// Search Devices
	// Some routers only reply to one of this SSDP searchs:
	UpnpSearchAsync(m_ctrlPoint, 5, "upnp:rootdevice", &m_ctrlPoint);
	UpnpSearchAsync(m_ctrlPoint, 5, "urn:schemas-upnp-org:device:InternetGatewayDevice:1", &m_ctrlPoint);

	return m_bInit;
}


// Give upnp a little bit of time to startup before doing outbound connections
// giving a unintend lowid:
// return nonzero if succesful. 
int CUPnP_IGDControlPoint::PauseForUpnpCompletion()
{
if(!m_bInit)
	return 0; 
if(thePrefs.GetUPnPVerboseLog())
	AddDebugLogLine(false, _T("Waiting short for upnp to complete registration.") );

if (InitializingEvent) 
       return (InitializingEvent->Lock((MINIMUM_DELAY*1000)+1)==S_OK) ; // 10 secs...  (should be UPNPTIMEOUT, btu 40 seconds is too long....)

return 0;
}



// Returns the port used by UPnP
unsigned int CUPnP_IGDControlPoint::GetPort(){
	if(!m_bInit)
		return 0;
    
	return UpnpGetServerPort(  );
}

// Handles all UPnP Events
int CUPnP_IGDControlPoint::IGD_Callback( Upnp_EventType EventType, void* Event, void* /*Cookie */){
	switch (EventType){
	//SSDP Stuff 
		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		case UPNP_DISCOVERY_SEARCH_RESULT:
		{
			//New UPnP Device found
			struct Upnp_Discovery *d_event = ( struct Upnp_Discovery * )Event;
			IXML_Document *DescDoc = NULL;
			int ret;

			if( d_event->ErrCode != UPNP_E_SUCCESS ) {
				if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, _T("UPnP: Error in Discovery Callback [%s]"), GetErrDescription(d_event->ErrCode) );
			}

			CString devType;
			CString location = CA2CT(d_event->Location);
			
			// Download Device description
			if( ( ret = UpnpDownloadXmlDoc( d_event->Location, &DescDoc ) ) != UPNP_E_SUCCESS ) {
				if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, _T("UPnP: Error obtaining device description from %s [%s]"), location, GetErrDescription(ret));
			}

			if(DescDoc){
				//Checks if is a InternetGatewayDevice and adds it to our list
				devType = GetFirstDocumentItem(DescDoc, _T("deviceType"));
				if(devType.CompareNoCase(IGD_DEVICE_TYPE) == 0){
					thePrefs.SetUpnpDetect(UPNP_DETECTED);//leuk_he autodetect upnp in wizard
					AddDevice(DescDoc, location, d_event->Expires);
				}

				ixmlDocument_free( DescDoc );
			}

			break;
		}
		case UPNP_DISCOVERY_SEARCH_TIMEOUT:
			if (thePrefs.GetUpnpDetect() != UPNP_DETECTED) {	  	 //leuk_he autodetect upnp in wizard
					StatusString=L"DeviceNotAutodetect";
				 	thePrefs.SetUpnpDetect(UPNP_NOT_DETECTED);//leuk_he autodetect upnp in wizard
			}
			InitializingEvent->SetEvent();
			break;
		case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
		{
			// UPnP Device Removed
			struct Upnp_Discovery *d_event = ( struct Upnp_Discovery * )Event;

			if( d_event->ErrCode != UPNP_E_SUCCESS ) {
				if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, _T("UPnP: Error in Discovery ByeBye Callback [%s]"), GetErrDescription(d_event->ErrCode) );
			}

			CString devType = CA2CT(d_event->DeviceType);
			devType.Trim();

			//Checks if is a InternetGatewayDevice and removes it from our list
			if(devType.CompareNoCase(IGD_DEVICE_TYPE) == 0){
				RemoveDevice( CString(CA2CT(d_event->DeviceId)));
			}

			break;
		}
		case UPNP_EVENT_RECEIVED:
		{
			// Event reveived
			struct Upnp_Event *e_event = ( struct Upnp_Event * )Event;

			// Parses the event
			OnEventReceived( e_event->Sid, e_event->EventKey, e_event->ChangedVariables );

			break;
		}
	}
	return 0;
}

// Adds a port mapping to all known/future devices
// Returns: 
//		UNAT_OK:	If the mapping has been added to our mapping list.
//					(Maybe not to the device, because it runs in a different thread).
//		UNAT_ERROR: init did fail, not UpnpAcceptsPorts allowed.
CUPnP_IGDControlPoint::UPNPNAT_RETURN CUPnP_IGDControlPoint::AddPortMapping(CUPnP_IGDControlPoint::UPNPNAT_MAPPING *mapping){
	if (UpnpAcceptsPorts==False)
		return UNAT_ERROR;

	if(mapping->externalPort == 0){
		mapping->externalPort = mapping->internalPort;
	}

	m_devListLock.Lock();
	m_MappingsLock.Lock();

	//Checks if we already have this mapping
	bool found = false;
	POSITION pos = m_Mappings.GetHeadPosition();
	while(pos){
		UPNPNAT_MAPPING item;
		item = m_Mappings.GetNext(pos);
		if(item.externalPort == mapping->externalPort &&
		   item.protocol== mapping->protocol	){	  // 9.3: fix same port tcp/udp
			found = true;
			pos = NULL;
		}
	}
	if(!m_bInit ||thePrefs.IsUPnPEnabled()==false){ // if not initialized then just note the mappings for later adding. 

	   if(!found ){
			//If we do not have this mapping, add it to our list when enabled
		   //TODO use getresstring.
		   if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, _T("Upnp:queuing port for when upnp is enabled: %s"), mapping->description);
		    m_Mappings.AddTail(*mapping);
		}
	   m_MappingsLock.Unlock();
       m_devListLock.Unlock();
	   return UNAT_OK;  
	}

	//Checks if we have devices
	if(m_devices.GetCount()==0){
		if(thePrefs.GetUPnPVerboseLog())
			theApp.QueueDebugLogLine(false, _T("UPnP: Action \"AddPortMapping\" queued until a device is found [%s]"), mapping->description);
		if(!found){
			//If we do not have this mapping, add it to our list
			m_Mappings.AddTail(*mapping);
		}
	}
	else {
		if(!found){
			//If we do not have this mapping, add it to our list
			//and try to add it to all known services.
			m_Mappings.AddTail(*mapping);
			POSITION srvpos = m_knownServices.GetHeadPosition();
			while(srvpos){
				UPNP_SERVICE *srv;
				srv = m_knownServices.GetNext(srvpos);

				if(srv && IsServiceEnabled(srv)){
					UPNPNAT_ACTIONPARAM *action = new UPNPNAT_ACTIONPARAM;
					if(action){
						action->type = UPNPNAT_ACTION_ADD;
						action->srv = *srv;
						action->mapping = *mapping;
						action->bUpdating = false;
						
						// Starts thread to add the mapping
						AfxBeginThread(ActionThreadFunc, action);
					}
				}
			}
		}
	}

	m_MappingsLock.Unlock();
	m_devListLock.Unlock();
	return UNAT_OK;
}

// Adds a port mapping to all known/future devices
// Returns: 
//		UNAT_OK:	If the mapping has been added to our mapping list.
//					(Maybe not to the device, because it runs in a different thread).
//		UNAT_ERROR: If you have not Init() the class.
CUPnP_IGDControlPoint::UPNPNAT_RETURN CUPnP_IGDControlPoint::AddPortMapping(WORD port, UPNPNAT_PROTOCOL protocol, CString description){
	UPNPNAT_MAPPING mapping;

	mapping.internalPort = mapping.externalPort = port;
	mapping.protocol = protocol;
	mapping.description = description;
				
	return AddPortMapping(&mapping);
}

// Removes a port mapping from all known devices
// Returns: 
//		UNAT_OK:	If the mapping has been removed from our mapping list.
//					(Maybe not from the device, because it runs in a different thread).
//		UNAT_ERROR: 
CUPnP_IGDControlPoint::UPNPNAT_RETURN CUPnP_IGDControlPoint::DeletePortMapping(CUPnP_IGDControlPoint::UPNPNAT_MAPPING mapping, bool removeFromList){

	m_devListLock.Lock();
	m_MappingsLock.Lock();
	if(mapping.externalPort == 0){
		mapping.externalPort = mapping.internalPort;
	}

	UPNPNAT_MAPPING item;
	if(!m_bInit || thePrefs.IsUPnPEnabled()==false) {
		// remove the queued mapping (only in queue, not in device) 
		m_MappingsLock.Lock();
        POSITION pos = m_Mappings.GetHeadPosition();
		if (pos) (item = m_Mappings.GetNext(pos));
	    while(pos && (item.externalPort != mapping.externalPort 
			          ||item.protocol != mapping.protocol)		){
  		   item = m_Mappings.GetNext(pos);
		}
	   if( pos && removeFromList)
			m_Mappings.RemoveAt(pos);
	   	m_MappingsLock.Unlock();
       m_devListLock.Unlock();
       return UNAT_OK;
	}


	POSITION old_pos, pos = m_Mappings.GetHeadPosition();
	while(pos){
		old_pos = pos;
		item = m_Mappings.GetNext(pos);
		if(item.externalPort == mapping.externalPort &&
			item.protocol== mapping.protocol ){
			POSITION srvpos = m_knownServices.GetHeadPosition();
			while(srvpos){
				UPNP_SERVICE *srv;
				srv = m_knownServices.GetNext(srvpos);
				if(srv){
					UPNPNAT_ACTIONPARAM *action = new UPNPNAT_ACTIONPARAM;
					if(action){
						action->type = UPNPNAT_ACTION_DELETE;
						action->srv = *srv;
						action->mapping = mapping;
						action->bUpdating = false;
						AfxBeginThread(ActionThreadFunc, action);
					}
				}
			}
			if(removeFromList)
				m_Mappings.RemoveAt(old_pos);

			pos = NULL;
		}
	}
	m_MappingsLock.Unlock();
	m_devListLock.Unlock();
	return UNAT_OK;
}

// Removes a port mapping from all known devices
// Returns: 
//		UNAT_OK:	If the mapping has been removed from our mapping list.
//					(Maybe not from the device, because it runs in a different thread).
//		UNAT_ERROR: If you have not Init() the class.
CUPnP_IGDControlPoint::UPNPNAT_RETURN CUPnP_IGDControlPoint::DeletePortMapping(WORD port, UPNPNAT_PROTOCOL protocol, CString description, bool removeFromList){
	UPNPNAT_MAPPING mapping;

	mapping.internalPort = mapping.externalPort = port;
	mapping.protocol = protocol;
	mapping.description = description;
				
	return DeletePortMapping(mapping, removeFromList);
}

// Removes all mapping we have in our list from all known devices
bool CUPnP_IGDControlPoint::DeleteAllPortMappings(){
	m_devListLock.Lock();
	m_MappingsLock.Lock();

	if(m_devices.GetCount()>0){
		POSITION srvpos = m_knownServices.GetHeadPosition();
		while(srvpos){
			UPNP_SERVICE *srv;
			srv = m_knownServices.GetNext(srvpos);
			if(srv){
				POSITION map_pos = m_Mappings.GetHeadPosition();
				while(map_pos){
					m_ActionThreadCS.Lock();
					UPNPNAT_MAPPING mapping = m_Mappings.GetNext(map_pos);
					DeletePortMappingFromService(srv, &mapping);
					m_ActionThreadCS.Unlock();
				}
			}
		}
	}

	m_MappingsLock.Unlock();
	m_devListLock.Unlock();

	return true;
}

bool CUPnP_IGDControlPoint::UpdateAllMappings( bool bLockDeviceList, bool bUpdating){
    if (thePrefs.IsUPnPEnabled()==false) // upnp portmapping disabled. 
		return true;

	if(bLockDeviceList)
		m_devListLock.Lock();

	if(m_devices.GetCount()>0){
		//Add mappings
		POSITION srvpos = m_knownServices.GetHeadPosition();
		while(srvpos){
			UPNP_SERVICE *srv;
			srv = m_knownServices.GetNext(srvpos);
			if(srv && IsServiceEnabled(srv)){
				m_MappingsLock.Lock();
				POSITION map_pos = m_Mappings.GetHeadPosition();
				while(map_pos){
					UPNPNAT_MAPPING mapping = m_Mappings.GetNext(map_pos);

					UPNPNAT_ACTIONPARAM *action = new UPNPNAT_ACTIONPARAM;
					if(action){
						action->type = UPNPNAT_ACTION_ADD;
						action->srv = *srv;
						action->mapping= mapping;
						action->bUpdating = bUpdating;
						AfxBeginThread(ActionThreadFunc, action);
					}
				}
				m_MappingsLock.Unlock();
			}
		}
	}

	if(bLockDeviceList)
		m_devListLock.Unlock();

	return true;
}

CString CUPnP_IGDControlPoint::GetFirstDocumentItem(IXML_Document * doc, CString item ){
	return GetFirstNodeItem( (IXML_Node *)doc, item );
}

CString CUPnP_IGDControlPoint::GetFirstElementItem( IXML_Element * element, CString item ){
	return GetFirstNodeItem( (IXML_Node *)element, item );
}

CString CUPnP_IGDControlPoint::GetFirstNodeItem( IXML_Node * root_node, CString item )
{
    IXML_Node *node;
	CString nodeVal;
	CString node_name;

	node = root_node;
    while( node != NULL ) {
		if (ixmlNode_getNodeType( node ) == eELEMENT_NODE){
			node_name = CA2CT(ixmlNode_getNodeName( node ));
			// match name
			if( node_name.CompareNoCase(item) == 0 ) {
				IXML_Node *text_node = NULL;

				text_node = ixmlNode_getFirstChild( node );
				if( text_node == NULL ) {
					return CString(_T(""));
				}

				nodeVal = CA2CT(ixmlNode_getNodeValue( text_node ));
				return nodeVal;
			}
			else{
				//Checks if we have something like "u:UPnPError" instead of "UPnPError"
				int pos = 0;
				pos = node_name.Find(_T(':'));
				if (pos != -1){
					node_name = node_name.Right(node_name.GetLength() - pos - 1);
					if( node_name.CompareNoCase(item) == 0 ) {
						IXML_Node *text_node = NULL;

						text_node = ixmlNode_getFirstChild( node );
						if( text_node == NULL ) {
							return CString(_T(""));
						}

						nodeVal = CA2CT(ixmlNode_getNodeValue( text_node ));
						return nodeVal;
					}
				}
			}
		}

		nodeVal = GetFirstNodeItem(ixmlNode_getFirstChild(node), item);
		if(!nodeVal.IsEmpty())
			return nodeVal;

        // free and next node
        node = ixmlNode_getNextSibling( node ); // next node
    }

	return CString(_T(""));
}

IXML_NodeList *CUPnP_IGDControlPoint::GetElementsByName(IXML_Document *doc, CString name){
	return GetElementsByName((IXML_Node*)doc, name);
}

IXML_NodeList *CUPnP_IGDControlPoint::GetElementsByName(IXML_Element *element, CString name){
	return GetElementsByName((IXML_Node*)element, name);
}

IXML_NodeList *CUPnP_IGDControlPoint::GetElementsByName(IXML_Node *root_node, CString name){
	IXML_NodeList *node_list = NULL;
	return GetElementsByName(root_node, name, &node_list);
}

IXML_NodeList *CUPnP_IGDControlPoint::GetElementsByName(IXML_Node *root_node, CString name, IXML_NodeList **nodelist){
    IXML_Node *node;

	if(nodelist == NULL)
		return NULL;

	node = root_node;
    while( node != NULL ) {
		if (ixmlNode_getNodeType( node ) == eELEMENT_NODE){
			CString node_name = CA2CT(ixmlNode_getNodeName( node ));
			// match name
			if( node_name.CompareNoCase(name) == 0 ) {
				ixmlNodeList_addToNodeList(nodelist, node);
			}
			else{
				int pos = 0;
				pos = node_name.Find(_T(':'));
				if (pos != -1){
					node_name = node_name.Right(node_name.GetLength() - pos - 1);
					if( node_name.CompareNoCase(name) == 0 ) {
						ixmlNodeList_addToNodeList(nodelist, node);
					}
				}
			}
		}

		GetElementsByName(ixmlNode_getFirstChild(node), name, nodelist);

        // free and next node
        node = ixmlNode_getNextSibling( node ); // next node
    }

	return *nodelist;
}

IXML_NodeList * CUPnP_IGDControlPoint::GetDeviceList( IXML_Document * doc ){
	return GetDeviceList((IXML_Node*)doc);
}

IXML_NodeList * CUPnP_IGDControlPoint::GetDeviceList( IXML_Element * element ){
	return GetDeviceList((IXML_Node*)element);
}

IXML_NodeList * CUPnP_IGDControlPoint::GetDeviceList( IXML_Node * root_node ){
    IXML_NodeList *DeviceList = NULL;
    IXML_NodeList *devlistnodelist = NULL;
    IXML_Node *devlistnode = NULL;

    devlistnodelist = GetElementsByName( root_node, _T("deviceList") );
    if( devlistnodelist && ixmlNodeList_length( devlistnodelist ) ) {
        devlistnode = ixmlNodeList_item( devlistnodelist, 0 );
        DeviceList =  GetElementsByName( devlistnode, _T("device") );
    }

    if( devlistnodelist )
        ixmlNodeList_free( devlistnodelist );

    return DeviceList;
}

IXML_NodeList * CUPnP_IGDControlPoint::GetServiceList( IXML_Element * element ){
	return GetServiceList((IXML_Node*) element);
}

IXML_NodeList * CUPnP_IGDControlPoint::GetServiceList( IXML_Node * root_node ){
    IXML_NodeList *ServiceList = NULL;
    IXML_NodeList *servlistnodelist = NULL;
    IXML_Node *servlistnode = NULL;

    servlistnodelist = GetElementsByName( root_node, _T("serviceList") );
    if( servlistnodelist && ixmlNodeList_length( servlistnodelist ) ) {
        servlistnode = ixmlNodeList_item( servlistnodelist, 0 );
        ServiceList = GetElementsByName( servlistnode, _T("service") );
    }

    if( servlistnodelist )
        ixmlNodeList_free( servlistnodelist );

    return ServiceList;
}

CUPnP_IGDControlPoint * CUPnP_IGDControlPoint::GetInstance(){
	if (m_IGDControlPoint == NULL) {
		m_IGDControlPoint = new CUPnP_IGDControlPoint();
	}
	return m_IGDControlPoint;
}

UINT CUPnP_IGDControlPoint::RemoveInstance(LPVOID /*pParam*/ ){
	delete m_IGDControlPoint;
	m_IGDControlPoint = NULL;
	return 0;
}

void CUPnP_IGDControlPoint::AddDevice( IXML_Document * doc, CString location, int expires){
	m_devListLock.Lock();
	
	CString UDN;
	UDN = GetFirstDocumentItem(doc, _T("UDN"));
	
	POSITION pos = m_devices.GetHeadPosition();
	bool found = false;
	while(pos){
		UPNP_DEVICE *item;
		item = m_devices.GetNext(pos);
		if(item && item->UDN.CompareNoCase(UDN) == 0){
			found = true;
			//Update advertisement timeout
			item->AdvrTimeOut = expires;
			pos = NULL;
		}
	}
	
	if(!found){
		CString friendlyName = GetFirstDocumentItem(doc, _T("friendlyName"));
		theApp.QueueLogLine(false, GetResString(IDS_UPNP_NEWDEVICE), friendlyName);
		if(thePrefs.GetUPnPVerboseLog())
			theApp.QueueDebugLogLine(false, _T("UPnP: New compatible device found: %s"), friendlyName);
		UPNP_DEVICE *device = new UPNP_DEVICE;
		if(device){
			CString BaseURL;
			m_devices.AddTail(device);
			device->DevType = GetFirstDocumentItem(doc, _T("deviceType"));
			device->UDN = UDN;
			device->DescDocURL = location;
			device->FriendlyName = friendlyName;
			device->AdvrTimeOut = expires;

			BaseURL = GetFirstDocumentItem(doc, _T("URLBase"));
			if(BaseURL.IsEmpty())
				BaseURL = location;

			IXML_NodeList* devlist1 = GetDeviceList(doc);
		    int length1 = ixmlNodeList_length( devlist1 );
			for( int n1 = 0; n1 < length1; n1++ ) {
				IXML_Element *dev1;
		        dev1 = ( IXML_Element * ) ixmlNodeList_item( devlist1, n1 );
				CString devType1 = GetFirstElementItem(dev1, _T("deviceType"));
				if(devType1.CompareNoCase(WAN_DEVICE_TYPE) == 0){
					//WanDevice
					UPNP_DEVICE *wandevice = new UPNP_DEVICE;
					wandevice->DevType = devType1;
					wandevice->UDN = GetFirstElementItem(dev1, _T("UDN"));
					wandevice->FriendlyName = GetFirstElementItem(dev1, _T("friendlyName"));
					device->EmbededDevices.AddTail(wandevice);

					if(thePrefs.GetUPnPVerboseLog())
						theApp.QueueDebugLogLine(false, _T("UPnP: Added embedded device: %s [%s]"), wandevice->FriendlyName, wandevice->DevType);

					IXML_NodeList* devlist2 = GetDeviceList(dev1);
					int length2 = ixmlNodeList_length( devlist2 );
					for( int n2 = 0; n2 < length2; n2++ ) {
						IXML_Element *dev2;
						dev2 = ( IXML_Element * ) ixmlNodeList_item( devlist2, n2 );
						CString devType2 = GetFirstElementItem(dev2, _T("deviceType"));
						if(devType2.CompareNoCase(WANCON_DEVICE_TYPE) == 0){
							//WanConnectionDevice, get services
							UPNP_DEVICE *wancondevice = new UPNP_DEVICE;
							wancondevice->DevType = devType2;
							wancondevice->UDN = GetFirstElementItem(dev2, _T("UDN"));
							wancondevice->FriendlyName = GetFirstElementItem(dev2, _T("friendlyName"));
							wandevice->EmbededDevices.AddTail(wancondevice);

							if(thePrefs.GetUPnPVerboseLog())
								theApp.QueueDebugLogLine(false, _T("UPnP: Added embedded device: %s [%s]"), wancondevice->FriendlyName, wancondevice->DevType);

							IXML_NodeList* services = GetServiceList(dev2);
							int length3 = ixmlNodeList_length( services );
							int wanConnFounds = 0;
							bool enabledFound = false;
							UPNP_SERVICE *firstService = NULL;

							for( int n3 = 0; n3 < length3 && !(m_bStopAtFirstService && wanConnFounds == 1); n3++ ) {
								IXML_Element *srv;
								srv = ( IXML_Element * ) ixmlNodeList_item( services, n3 );
								CString srvType = GetFirstElementItem(srv, _T("serviceType"));
								
								if(srvType.CompareNoCase(WANIP_SERVICE_TYPE) == 0
									|| srvType.CompareNoCase(WANPPP_SERVICE_TYPE) == 0)
								{
									//Compatible Service found
									wanConnFounds++;

									CString RelURL;
									char *cAbsURL;
									UPNP_SERVICE *service = new UPNP_SERVICE;

									if(wanConnFounds == 1)
										firstService = service;

									service->ServiceType = srvType;
									service->ServiceID = GetFirstElementItem(srv, _T("serviceId"));

									RelURL = GetFirstElementItem(srv, _T("eventSubURL"));
									cAbsURL = new char[BaseURL.GetLength() + RelURL.GetLength() + 1];
									if(cAbsURL && UpnpResolveURL(CT2CA(BaseURL), CT2CA(RelURL), cAbsURL) == UPNP_E_SUCCESS)
									{
										service->EventURL = CA2CT(cAbsURL);
									}
									else{
										service->EventURL = RelURL;
									}
									delete[] cAbsURL;

									RelURL = GetFirstElementItem(srv, _T("controlURL"));
									cAbsURL = new char[BaseURL.GetLength() + RelURL.GetLength() + 1];
									if(cAbsURL && UpnpResolveURL(CT2CA(BaseURL), CT2CA(RelURL), cAbsURL) == UPNP_E_SUCCESS)
									{
										service->ControlURL = CA2CT(cAbsURL);
									}
									else{
										service->ControlURL = RelURL;
									}
									delete[] cAbsURL;

									service->Enabled = -1; //Uninitialized

									wancondevice->Services.AddTail(service);
									m_knownServices.AddTail(service);

									if(IsServiceEnabled(service)){
										enabledFound = true;
										if(thePrefs.GetUPnPVerboseLog())
											theApp.QueueDebugLogLine(false, _T("UPnP: Added service: %s (Enabled)"), service->ServiceType);
									}
									else{
										if(thePrefs.GetUPnPVerboseLog())
											theApp.QueueDebugLogLine(false, _T("UPnP: Added service: %s (Disabled)"), service->ServiceType);
									}

									//Subscribe to events
									int TimeOut = expires;
									int subsRet;
									subsRet = UpnpSubscribe(m_ctrlPoint, CT2CA(service->EventURL), &TimeOut, service->SubscriptionID);
									if(subsRet == UPNP_E_SUCCESS){
										if(thePrefs.GetUPnPVerboseLog())
											theApp.QueueDebugLogLine(false, _T("UPnP: Subscribed with service \"%s\" [SID=%s]"), service->ServiceType, CA2CT(service->SubscriptionID));
									}
									else{
										if(thePrefs.GetUPnPVerboseLog())
											theApp.QueueDebugLogLine(false, _T("UPnP: Failed to subscribe with service: %s [%s]"), service->ServiceType, GetErrDescription(subsRet));
									}
								}
							}
							ixmlNodeList_free(services);

							// If no service is enabled, force
							// try with first one.
							if(!enabledFound && firstService){
								if(thePrefs.GetUPnPVerboseLog())
									theApp.QueueDebugLogLine(false, _T("UPnP: No enabled service found. Trying with first service [%s]"), firstService->ServiceType);
								firstService->Enabled = 1;
							}
						}
					}
					ixmlNodeList_free(devlist2);
				}
			}
			ixmlNodeList_free(devlist1);
		}
		UpdateAllMappings(false, false);
	}
	m_devListLock.Unlock();
}

void CUPnP_IGDControlPoint::RemoveDevice( UPNP_DEVICE *dev ){
	//Do not use the mutex here, is a recursive function
	CString fName = dev->FriendlyName;

	POSITION pos = dev->EmbededDevices.GetHeadPosition();
	while(pos){
		UPNP_DEVICE *item;
		item = dev->EmbededDevices.GetNext(pos);
		if(item){
			RemoveDevice(item);
		}
	}

	pos = dev->Services.GetHeadPosition();
	while(pos){
		UPNP_SERVICE *item;
		item = dev->Services.GetNext(pos);

		POSITION srvpos = m_knownServices.GetHeadPosition();
		while(srvpos){
			UPNP_SERVICE *item2;
			POSITION curpos = srvpos;
			item2 = m_knownServices.GetNext(srvpos);
			if(item == item2){
				m_knownServices.RemoveAt(curpos);
				srvpos = NULL;
			}
		}

		if(item)
			delete item;
	}

	delete dev;

	if(thePrefs.GetUPnPVerboseLog())
		theApp.QueueDebugLogLine(false, _T("UPnP: Device removed: %s"), fName);
}

void CUPnP_IGDControlPoint::RemoveDevice( CString devID ){
	m_devListLock.Lock();
	
	POSITION old_pos, pos = m_devices.GetHeadPosition();
	while(pos){
		UPNP_DEVICE *item;
		old_pos = pos;
		item = m_devices.GetNext(pos);
		if(item && item->UDN.CompareNoCase(devID) == 0){
			if(thePrefs.GetUPnPVerboseLog())
				theApp.QueueDebugLogLine(false, _T("UPnP: Device removed: %s"), item->FriendlyName);
			RemoveDevice(item);
			m_devices.RemoveAt(old_pos);
			pos = NULL;
		}
	}
	m_devListLock.Unlock();
}

void CUPnP_IGDControlPoint::CheckTimeouts(){
	if(!m_bInit)
		return;

	m_devListLock.Lock();
	
	POSITION old_pos, pos = m_devices.GetHeadPosition();
	while(pos){
		UPNP_DEVICE *item;
		old_pos = pos;
		item = m_devices.GetNext(pos);
		item->AdvrTimeOut -= UPNP_ADVERTISEMENT_DECREMENT;
		if(item->AdvrTimeOut <= 0){
			RemoveDevice(item);
			m_devices.RemoveAt(old_pos);
		}
		else if(item->AdvrTimeOut < UPNP_ADVERTISEMENT_DECREMENT * 2){
			//About to expire, send a search to try to renew
            UpnpSearchAsync( m_ctrlPoint, UPNP_ADVERTISEMENT_DECREMENT,
                                       CT2CA(item->UDN), &m_ctrlPoint);
		}
	}
	m_devListLock.Unlock();
}

UINT CUPnP_IGDControlPoint::TimerThreadFunc( LPVOID /*pParam*/ ){
	int sleepTime = UPNP_ADVERTISEMENT_DECREMENT * 1000;
	int updateTimeF = UPNP_PORT_LEASETIME * 800;
	static long int updateTime = updateTimeF;

	static long int testTime = sleepTime; //SiRoB

	while(m_IGDControlPoint){
		// SiRoB >>
		/*
		Sleep(sleepTime);
		*/
		Sleep(1000);
		testTime-=1000;
		if (testTime <= 0) {
			testTime = sleepTime;
		// << SiRoB
			CheckTimeouts();

			updateTime -= sleepTime;
			if(updateTime <= 0) {
				UpdateAllMappings();
				updateTime = updateTimeF;
			}
		} // SiRoB
	};

	return 1;
}

CUPnP_IGDControlPoint::UPNPNAT_RETURN CUPnP_IGDControlPoint::AddPortMappingToService(CUPnP_IGDControlPoint::UPNP_SERVICE *srv, CUPnP_IGDControlPoint::UPNPNAT_MAPPING *mapping, bool bIsUpdating){
	if(!m_bInit)
		return UNAT_ERROR;

	UPNPNAT_RETURN Status = UNAT_ERROR;

	CString protocol;
	CString desc;
	CString intPort, extPort;
	bool bUpdate = false;

	if (mapping->protocol == UNAT_TCP){
		protocol = _T("TCP");
	}
	else {
		protocol = _T("UDP");
	}
		
	desc.Format(UPNP_DESCRIPTION_FORMAT, mapping->description, protocol, mapping->externalPort);
	_itow(mapping->internalPort, intPort.GetBufferSetLength(10), 10);
	intPort.ReleaseBuffer();
	_itow(mapping->externalPort, extPort.GetBufferSetLength(10), 10);
	extPort.ReleaseBuffer();

	//Check if the portmaping already exists
	UPNPNAT_FULLMAPPING fullMapping;
	if((thePrefs.m_bUPnPForceUpdate==0) && GetSpecificPortMappingEntryFromService(srv, mapping, &fullMapping, false) == UNAT_OK){
		if(fullMapping.internalClient == GetLocalIPStr()){
			if(fullMapping.description.Left(7).MakeLower() != _T("emule (")){
				if(thePrefs.GetUPnPVerboseLog())  {
					theApp.QueueDebugLogLine(false,_T("UPnP: Couldn't add mapping: \"%s\". The port %d is already mapped to other application (\"%s\" on %s:%d). [%s]"), desc, mapping->externalPort, fullMapping.description, fullMapping.internalClient, fullMapping.internalPort, srv->ServiceType);
				};
				return UNAT_NOT_OWNED_PORTMAPPING;
			}
			else{
				if(fullMapping.enabled == TRUE && fullMapping.leaseDuration == 0){
					if(bIsUpdating){
						if(thePrefs.GetUPnPVerboseLog()) {
							theApp.QueueDebugLogLine(false, _T("UPnP: The port mapping \"%s\" doesn't need an update. [%s]"), desc, srv->ServiceType);
						}
					}
					else 
						if(thePrefs.GetUPnPVerboseLog()) {
							theApp.QueueDebugLogLine(false,_T("UPnP: The port mapping \"%s\" doesn't need to be recreated. [%s]"), desc, srv->ServiceType);
						};
					//Mapping is already OK
					return UNAT_OK;
				}
				else{
					bUpdate = true;
				}
			}
		}
		else{
			if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, _T("UPnP: Couldn't add mapping: \"%s\". The port %d is already mapped to other pc/application (\"%s\" on %s:%d). [%s]"), desc, mapping->externalPort, fullMapping.description, fullMapping.internalClient, fullMapping.internalPort, srv->ServiceType);
			return UNAT_NOT_OWNED_PORTMAPPING;
		}
	}

	IXML_Document *actionNode = NULL;
	char actionName[] = "AddPortMapping";
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewRemoteHost", "");
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewExternalPort", CT2CA(extPort));
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewProtocol", CT2CA(protocol.GetBuffer()));
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewInternalPort", CT2CA(intPort));
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewInternalClient", CT2CA(GetLocalIPStr().GetBuffer()));
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewEnabled", "1");
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewPortMappingDescription", CT2CA(desc));

	//Only set a lease time if we want to remove it on close
	if(m_bClearOnClose){
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewLeaseDuration", UPNP_PORT_LEASETIME_STR);
	}
	else{
		UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
			"NewLeaseDuration", "0");
	}

	IXML_Document* RespNode = NULL;
	int rc = UpnpSendAction( m_ctrlPoint,CT2CA(srv->ControlURL),
		CT2CA(srv->ServiceType), NULL, actionNode, &RespNode);
	if( rc != UPNP_E_SUCCESS)
	{
		// if m_bClearOnClose==TRUE we have already tried with an static port mapping
		if(m_bClearOnClose){
			//Maybe the IGD do not support dynamic port mappings,
			//try with an static one (NewLeaseDuration = 0).

			if( RespNode )
				ixmlDocument_free( RespNode );
			
			IXML_NodeList *nodeList = NULL;
			IXML_Node *textNode = NULL;
			IXML_Node *tmpNode = NULL;
			nodeList = GetElementsByName( actionNode, _T("NewLeaseDuration"));

			if(nodeList) {
				tmpNode = ixmlNodeList_item( nodeList, 0 );
				if(tmpNode) {
					textNode = ixmlNode_getFirstChild( tmpNode );
					ixmlNode_setNodeValue( textNode , "0");
				}
				ixmlNodeList_free(nodeList);
			}

			rc = UpnpSendAction( m_ctrlPoint,CT2CA(srv->ControlURL),
				CT2CA(srv->ServiceType), NULL, actionNode, &RespNode);
			}

		//This can be changed if we tried with an static port mapping
		if(rc == UPNP_E_SUCCESS){
			Status = UNAT_OK;
			if(bUpdate)	{
				if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, _T("UPnP: Updated port mapping \"%s\" (%s). [%s]"), desc, _T("Static"), srv->ServiceType);
			}
			else{
				if(thePrefs.GetUPnPVerboseLog()) {
					theApp.QueueDebugLogLine(false, _T( "UPnP: Added port mapping \"%s\" (%s). [%s]"), desc, _T("Static"), srv->ServiceType);
				}
			}
		}
		else{
			if(bIsUpdating){
				if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, _T("UPnP: Failed to update port mapping \"%s\" [%s] [%s]"), desc, srv->ServiceType, GetErrDescription(RespNode, rc));
			}
			else {
				if(thePrefs.GetUPnPVerboseLog()) 
					theApp.QueueDebugLogLine(false, _T("UPnP: Failed to add port mapping \"%s\" [%s] [%s]"), desc, srv->ServiceType, GetErrDescription(RespNode, rc));		
			}
		}
	}
	else{
		Status = UNAT_OK;
		if(bUpdate)	{
			if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false,_T("UPnP: Updated port mapping \"%s\" (%s). [%s]"), desc, _T("Dynamic"), srv->ServiceType);
		}
		else {
			if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false,_T( "UPnP: Added port mapping \"%s\" (%s). [%s]"), desc, _T("Dynamic"), srv->ServiceType);
		}
	}

    if( RespNode )
        ixmlDocument_free( RespNode );

    if( actionNode )
        ixmlDocument_free( actionNode );

	return Status;
}

CUPnP_IGDControlPoint::UPNPNAT_RETURN CUPnP_IGDControlPoint::DeletePortMappingFromService(CUPnP_IGDControlPoint::UPNP_SERVICE *srv, CUPnP_IGDControlPoint::UPNPNAT_MAPPING *mapping){
	if(!m_bInit)
		return UNAT_ERROR;

	UPNPNAT_RETURN Status = UNAT_ERROR;

	CString protocol;
	CString desc;
	CString extPort;

	if (mapping->protocol == UNAT_TCP){
		protocol = _T("TCP");
	}
	else {
		protocol = _T("UDP");
	}
		
	desc.Format(UPNP_DESCRIPTION_FORMAT, mapping->description, protocol, mapping->externalPort);
	_itow(mapping->externalPort, extPort.GetBufferSetLength(10), 10);
	extPort.ReleaseBuffer();

	//Check if the portmapping belong to us
	UPNPNAT_FULLMAPPING fullMapping;
	UPNPNAT_RETURN ret = GetSpecificPortMappingEntryFromService(srv, mapping, &fullMapping, false);
	if( ret == UNAT_OK ){
		if(fullMapping.internalClient == GetLocalIPStr()){
			if(fullMapping.description.Left(7).MakeLower() != _T("emule (")){
				return UNAT_NOT_OWNED_PORTMAPPING;
			}
		}
		else{
			return UNAT_NOT_OWNED_PORTMAPPING;
		}
	}
	else if(ret == UNAT_NOT_FOUND){
		if(thePrefs.GetUPnPVerboseLog())
			theApp.QueueDebugLogLine(false, _T("UPnP: Deleting port mapping \"%s\" aborted. [%s] [NoSuchEntryInArray (714)]"), desc, srv->ServiceType);
		return UNAT_NOT_FOUND;
	}

	IXML_Document *actionNode = NULL;
	char actionName[] = "DeletePortMapping";
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewRemoteHost", "");
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewExternalPort", CT2CA(extPort));
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewProtocol", CT2CA(protocol.GetBuffer()));

	IXML_Document* RespNode = NULL;
	int rc = UpnpSendAction( m_ctrlPoint,CT2CA(srv->ControlURL),
		CT2CA(srv->ServiceType), NULL, actionNode, &RespNode);
	if( rc != UPNP_E_SUCCESS)
	{
		if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false,_T("UPnP: Failed to delete port mapping \"%s\". [%s] [%s]"), desc, srv->ServiceType, GetErrDescription(RespNode, rc));
	}
	else{
		Status = UNAT_OK;
		if(thePrefs.GetUPnPVerboseLog())
					theApp.QueueDebugLogLine(false, _T("UPnP: Deleted port mapping \"%s\". [%s]"), desc, srv->ServiceType);
	}

    if( RespNode )
        ixmlDocument_free( RespNode );
    if( actionNode )
        ixmlDocument_free( actionNode );

	return Status;
}

CUPnP_IGDControlPoint::UPNPNAT_RETURN CUPnP_IGDControlPoint::GetSpecificPortMappingEntryFromService(CUPnP_IGDControlPoint::UPNP_SERVICE *srv, UPNPNAT_MAPPING *mapping, UPNPNAT_FULLMAPPING *fullMapping, bool bLog){
	if(!m_bInit)
		return UNAT_ERROR;

	UPNPNAT_RETURN status = UNAT_ERROR;

	CString protocol;
	CString desc;
	CString extPort;

	if (mapping->protocol == UNAT_TCP){
		protocol = _T("TCP");
	}
	else {
		protocol = _T("UDP");
	}
		
	desc.Format(UPNP_DESCRIPTION_FORMAT, mapping->description, protocol, mapping->externalPort);
	_itow(mapping->externalPort, extPort.GetBufferSetLength(10), 10);
	extPort.ReleaseBuffer();

	IXML_Document *actionNode = NULL;
	char actionName[] = "GetSpecificPortMappingEntry";
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewRemoteHost", "");
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewExternalPort", CT2CA(extPort));
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),
		"NewProtocol", CT2CA(protocol.GetBuffer()));

	IXML_Document* RespNode = NULL;
	int rc = UpnpSendAction( m_ctrlPoint,CT2CA(srv->ControlURL),
		CT2CA(srv->ServiceType), NULL, actionNode, &RespNode);
	if( rc != UPNP_E_SUCCESS)
	{
		if(rc == 714){
			//NoSuchEntryInArray
			status = UNAT_NOT_FOUND;
		}
		else{
			//Other error
			status = UNAT_ERROR;
		}

		if(bLog && thePrefs.GetUPnPVerboseLog())
			theApp.QueueDebugLogLine(false, _T("UPnP: Failed to get specific port mapping entry \"%s\". [%s]"), desc, GetErrDescription(RespNode, rc));
	}
	else{
		fullMapping->externalPort = mapping->externalPort;
		fullMapping->protocol = mapping->protocol;
		fullMapping->internalPort =(WORD)  _ttoi(GetFirstDocumentItem(RespNode, _T("NewInternalPort")));
		fullMapping->description = GetFirstDocumentItem(RespNode, _T("NewPortMappingDescription"));
		fullMapping->enabled = _ttoi(GetFirstDocumentItem(RespNode, _T("NewEnabled"))) == 0 ? false : true;
		fullMapping->leaseDuration = _ttoi(GetFirstDocumentItem(RespNode, _T("NewLeaseDuration")));

		// WinXP returns the host name instead of the ip for the "NewInternalClient" var.
		// Try to get the ip using the host name.
		CString internalClient = GetFirstDocumentItem(RespNode, _T("NewInternalClient"));

		LPHOSTENT lphost;
		lphost = gethostbyname(CT2CA(internalClient));
		if (lphost != NULL){
			internalClient = CA2CT(inet_ntoa(*(in_addr*)lphost->h_addr_list[0]));
		}

		fullMapping->internalClient = internalClient;
		status = UNAT_OK;
	}

    if( RespNode )
        ixmlDocument_free( RespNode );
    if( actionNode )
        ixmlDocument_free( actionNode );

	return status;
}



/* Experimental
CUPnP_IGDControlPoint::UPNPNAT_RETURN CUPnP_IGDControlPoint::GetExternalIPAddress(CUPnP_IGDControlPoint::UPNP_SERVICE *srv, bool bLog){
	if(!m_bInit)
		return UNAT_ERROR;

	UPNPNAT_RETURN status = UNAT_ERROR;

	IXML_Document *actionNode = NULL;
	char actionName[] = "GetExternalIPAddress";
	UpnpAddToAction( &actionNode, actionName , CT2CA(srv->ServiceType),		"", "");

	IXML_Document* RespNode = NULL;
	int rc = UpnpSendAction( m_ctrlPoint,CT2CA(srv->ControlURL),
		CT2CA(srv->ServiceType), NULL, actionNode, &RespNode);
	if( rc != UPNP_E_SUCCESS)
	{
		if(rc == 714){
			//NoSuchEntryInArray
			status = UNAT_NOT_FOUND;
		}
		else{
			//Other error
			status = UNAT_ERROR;
		}

		if(bLog && thePrefs.GetUPnPVerboseLog())
			theApp.QueueDebugLogLine(false, _T("UPnP: Failed to get external ip\"%s\". [%s]"), desc, GetErrDescription(RespNode, rc));
	}
	else{
		Cstring ExternalIp=  GetFirstDocumentItem(RespNode, _T("ExternalIp")));
		status = UNAT_OK;
	}

    if( RespNode )
        ixmlDocument_free( RespNode );
    if( actionNode )
        ixmlDocument_free( actionNode );

	return status;
}
 Experimental */ 







/////////////////////////////////////////////////////////////////////////////////
// Returns a CString with the local IP in format xxx.xxx.xxx.xxx
/////////////////////////////////////////////////////////////////////////////////
CString CUPnP_IGDControlPoint::GetLocalIPStr()
{
	if(!m_bInit)
		return CString(_T(""));
	else
		return CString(CA2CT(UpnpGetServerIpAddress()));
}

/////////////////////////////////////////////////////////////////////////////////
// Returns true if nIP is a LAN ip, false otherwise
/////////////////////////////////////////////////////////////////////////////////
bool CUPnP_IGDControlPoint::IsLANIP(unsigned long nIP){
	// filter LAN IP's
	// -------------------------------------------
	// 0.*
	// 10.0.0.0 - 10.255.255.255  class A
	// 172.16.0.0 - 172.31.255.255  class B
	// 192.168.0.0 - 192.168.255.255 class C

	unsigned char nFirst = (unsigned char)nIP;
	unsigned char nSecond = (unsigned char)(nIP >> 8);

	if (nFirst==192 && nSecond==168) // check this 1st, because those LANs IPs are mostly spreaded
		return true;

	if (nFirst==172 && nSecond>=16 && nSecond<=31)
		return true;

	if (nFirst==0 || nFirst==10)
		return true;

	return false; 
}

bool CUPnP_IGDControlPoint::IsLANIP(char *cIP){
	if(cIP == NULL)
		return false;

	return IsLANIP(inet_addr(cIP));
}

UINT CUPnP_IGDControlPoint::ActionThreadFunc( LPVOID pParam ){
	m_ActionThreadCS.Lock();
	UPNPNAT_ACTIONPARAM *action	= reinterpret_cast<UPNPNAT_ACTIONPARAM *>(pParam);

	if(action){
		if(IsServiceEnabled(&(action->srv))){
			switch(action->type){
				case UPNPNAT_ACTION_ADD:
					AddPortMappingToService(&(action->srv), &(action->mapping), action->bUpdating);
					break;
				case UPNPNAT_ACTION_DELETE:
					DeletePortMappingFromService(&(action->srv), &(action->mapping));
					break;
			}
		}
		delete action;
	}
	if (InitializingEvent) InitializingEvent->SetEvent();// ports added, tell main thead to continue. 
	m_ActionThreadCS.Unlock();
	return 1;
}

bool CUPnP_IGDControlPoint::IsServiceEnabled(CUPnP_IGDControlPoint::UPNP_SERVICE *srv){
	if(!m_bInit)
		return false;

	if(srv->Enabled != -1)
		return (srv->Enabled == 1 ? true : false);

	bool status = false;

	IXML_Document *actionNode = NULL;
	char actionName[] = "GetStatusInfo";
	actionNode = UpnpMakeAction(actionName, CT2CA(srv->ServiceType), 0, NULL);

	IXML_Document* RespNode = NULL;
	int rc = UpnpSendAction( m_ctrlPoint,CT2CA(srv->ControlURL),
		CT2CA(srv->ServiceType), NULL, actionNode, &RespNode);
	if( rc != UPNP_E_SUCCESS)
	{
		if(thePrefs.GetUPnPVerboseLog())
			theApp.QueueDebugLogLine(false, _T("UPnP: Failed to get GetStatusInfo from [%s] [%s])"), srv->ServiceType, GetErrDescription(RespNode, rc));
	}
	else{
		CString strStatus = GetFirstDocumentItem(RespNode, _T("NewConnectionStatus"));
		if(strStatus.CompareNoCase(_T("Connected")) == 0)
			status = true;
	}

    if( RespNode )
        ixmlDocument_free( RespNode );
    if( actionNode )
        ixmlDocument_free( actionNode );

	srv->Enabled = (status == true ? 1 : 0);
	return status;
}

void CUPnP_IGDControlPoint::OnEventReceived(Upnp_SID sid, int /* evntkey */, IXML_Document * changes ){
	bool update = false;
	
	m_devListLock.Lock();

	POSITION srvpos = m_knownServices.GetHeadPosition();
	while(srvpos){
		UPNP_SERVICE *srv;
		srv = m_knownServices.GetNext(srvpos);

		if(srv){
			if(strcmp(srv->SubscriptionID, sid) == 0){
				//if(thePrefs.GetUPnPVerboseLog())
				//	theApp.QueueDebugLogLine(false, _T("UPnP: Event received from service \"%s\" [SID=%s]"), srv->ServiceType, CA2CT(srv->SubscriptionID));
				
				//Parse Event
				IXML_NodeList *properties = NULL;

				properties = GetElementsByName( changes, _T("property") );
				if(properties != NULL){
					int length = ixmlNodeList_length( properties );
					for( int i = 0; i < length; i++ ) {
						IXML_Element *property = NULL;

						property = ( IXML_Element * ) ixmlNodeList_item( properties, i );
						if(property){
							IXML_NodeList *variables = NULL;
							variables = GetElementsByName( property, _T("ConnectionStatus") );
							if(variables){
								int length2 = ixmlNodeList_length( variables );
								if( length2 > 0 ) {
									IXML_Element *variable = NULL;
									CString value;

									variable = ( IXML_Element * ) ixmlNodeList_item( variables, 0 );
										
									IXML_Node *child = ixmlNode_getFirstChild( ( IXML_Node * ) variable );

									if( ( child != 0 ) && ( ixmlNode_getNodeType( child ) == eTEXT_NODE ) ) {
										value = CA2CT(ixmlNode_getNodeValue( child ));
									}

									if(value.CompareNoCase(_T("Connected")) == 0){
										if(thePrefs.GetUPnPVerboseLog())
											theApp.QueueDebugLogLine(false, _T("UPnP: New ConnectionStatus for \"%s\" (Connected) [SID=%s] "), srv->ServiceType, CA2CT(srv->SubscriptionID));
										if(srv->Enabled != 1)
											update = true;
										srv->Enabled = 1;
									}
									else{
										if(thePrefs.GetUPnPVerboseLog())
											theApp.QueueDebugLogLine(false, _T("UPnP: New ConnectionStatus for \"%s\" (Disconnected) [SID=%s]"), srv->ServiceType, CA2CT(srv->SubscriptionID));
										if(srv->Enabled != 0)
											update = true;
										srv->Enabled = 0;
									}
								}
								ixmlNodeList_free( variables );
							}
						}
					}
					ixmlNodeList_free( properties );
				}
			}
		}
	}

	m_devListLock.Unlock();

	if(update)
		UpdateAllMappings();
}

CString CUPnP_IGDControlPoint::GetErrDescription(int err){
	CString errDesc;

	if(err < 0){
		errDesc = CA2CT(UpnpGetErrorMessage(err));
	}
	else{
		errDesc = _T("HTTP_ERROR");
	}

	errDesc.AppendFormat(_T(" (%d)"), err);
	return errDesc;
}

CString CUPnP_IGDControlPoint::GetErrDescription(IXML_Document* errDoc, int err){
	CString errDesc;
	int err_n = 0;

	err_n = _ttoi(GetFirstDocumentItem(errDoc, _T("errorCode")));
	if(err_n == 0){
		errDesc = GetErrDescription(err);
	}
	else{
		errDesc = GetFirstDocumentItem(errDoc, _T("errorDescription"));
		if(errDesc.IsEmpty())
			errDesc = _T("Unknown Error");
		errDesc.AppendFormat(_T(" (%d)"), err_n);
	}
	return errDesc;
}

void CUPnP_IGDControlPoint::DeleteAllPortMappingsOnClose(){
	m_bClearOnClose = true;
}

int  CUPnP_IGDControlPoint::GetStatusString(CString & displaystring,bool verbose)
{
	if(!m_bInit){
		if	(StatusString.IsEmpty()	)
    		displaystring=GetResString(IDS_UPNP_INFO_NONEED);
		else 
			displaystring=StatusString;
		return (1);
	}

	CString upnpIpPort ;
	upnpIpPort .Format(_T("\t%s:%u\r\n"),CString(CA2CT(UpnpGetServerIpAddress())),(int)UpnpGetServerPort());

	m_devListLock.Lock();
	m_MappingsLock.Lock();

	if(m_devices.GetCount()>0){
		displaystring += GetResString(IDS_ENABLED) + _T("\r\n");
		if (verbose)
			displaystring += GetResString(IDS_IP)+_T(":")+ GetResString(IDS_PORT) +upnpIpPort ;
        POSITION devpos;
		devpos= m_devices.GetHeadPosition();
		while (devpos) {
			UPNP_DEVICE *item;
			item = m_devices.GetNext(devpos);
			displaystring += item->FriendlyName +  _T("\r\n");
		}
		if (verbose) {
			POSITION srvpos = m_knownServices.GetHeadPosition();
			while(srvpos){
				UPNP_SERVICE *srv;
				srv = m_knownServices.GetNext(srvpos);
				displaystring += _T("srv:") +  srv->ServiceID + _T(":") + srv->ServiceType ;
				switch (srv->Enabled){
					case -1:
						displaystring += GetResString(IDS_UPNP_INFOUNINIT);
						break;
					case 1:
						displaystring += GetResString(IDS_UPNP_INFOENABLED);
						break;
					case 0:
					default:
						displaystring += GetResString(IDS_UPNP_INFODISABLED);
				}
			}
		}
		POSITION map_pos = m_Mappings.GetHeadPosition();
		while(map_pos){
				UPNPNAT_MAPPING mapping = m_Mappings.GetNext(map_pos);
				CString port;
				port.Format(_T("%d"), mapping.externalPort );
				if (verbose)
					displaystring +=  port + ((mapping.protocol == UNAT_UDP)?_T(":UDP\t"): _T(":TCP\t")) +  mapping.description +_T("\r\n");
				else
				{
					displaystring +=  port + ((mapping.protocol == UNAT_UDP)?_T(":UDP"): _T(":TCP"));
					if(map_pos)
						displaystring += _T(", ");
				}
		}
	}
	else 
	{	displaystring +=  GetResString(IDS_UPNP_INFOSTANDBY ); //No Device detected\r\n
		if (verbose)
			displaystring += GetResString(IDS_IP)+_T(":")+ GetResString(IDS_PORT) + upnpIpPort ;
	}
	m_MappingsLock.Unlock();
	m_devListLock.Unlock();
	return 0;
}



