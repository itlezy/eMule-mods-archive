//this file is part of eMule
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
#include "emule.h"
#include "KademliaMain.h"
#include "Kademlia/Kademlia/kademlia.h"
#include "Kademlia/net/kademliaudplistener.h"
#include "Kademlia/Kademlia/prefs.h"
#include "Kademlia/routing/timer.h"
#include "OtherFunctions.h"
#ifndef _CONSOLE
#include "emuledlg.h"
#include "MuleToolbarCtrl.h"
#include "KademliaWnd.h"
#include "KadContactListCtrl.h"
#include "KadSearchListCtrl.h"
#include "UpDownClient.h"
#include "ClientList.h"

#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CKademliaMain::CKademliaMain(void)
{
	m_status = new Status();
	m_status->m_connected = false;
	m_status->m_firewalled = true;
	m_status->m_ip = 0;
	m_status->m_tcpport = 0;
	m_status->m_totalFile = 0;
	m_status->m_totalStoreSrc = 0;
	m_status->m_totalStoreKey = 0;
	m_status->m_udpport = 0;
	m_status->m_kademliaUsers = 0;
	m_status->m_totalContacts = 0;
	m_status->m_keywordPublish = false;
	m_bootstrapTimer = time(NULL);
}

CKademliaMain::~CKademliaMain(void)
{
	delete m_status;
}

void CKademliaMain::setStatus(Status* val)
{
	if (m_status->m_connected != val->m_connected)
	{
		delete m_status;
		m_status = val;
		theApp.emuledlg->ShowConnectionState();
		theApp.emuledlg->ShowUserCount();
		if(m_status->m_connected)
			AddLogLine(true, "Kademlia %s", GetResString(IDS_CONNECTED));
		else
		{
			AddLogLine(true, "Kademlia %s", GetResString(IDS_DISCONNECTED));
		}
	}
	else if( val && (m_status->m_firewalled != val->m_firewalled || m_status->m_ip != val->m_ip || m_status->m_udpport != val->m_udpport || m_status->m_tcpport != val->m_tcpport))
	{
		delete m_status;
		m_status = val;
		theApp.emuledlg->ShowConnectionState();
		theApp.emuledlg->ShowUserCount();
	}
	else
	{
		delete m_status;
		m_status = val;
		theApp.emuledlg->ShowUserCount();
	}
}

DWORD CKademliaMain::GetThreadID()
{
	return Kademlia::CTimer::getThreadID();
}

void CKademliaMain::Connect()
{
	if(!Kademlia::CTimer::getThreadID())
	{
		bool udpChange = false;
		while( theApp.glob_prefs->GetKadUDPPort() == theApp.glob_prefs->GetUDPPort() /*|| theApp.glob_prefs->GetKadUDPPort() == What is the servers listen udp port?*/)
		{
			theApp.glob_prefs->SetKadUDPPort(theApp.glob_prefs->GetKadUDPPort()+1);
			udpChange = true;
		}
		if( udpChange )
		{
			CString msg;
			msg.Format(GetResString(IDS_KADUDPPORTERR), theApp.glob_prefs->GetKadUDPPort());
			AfxMessageBox(msg);
		}
		theApp.emuledlg->kademliawnd->contactList->Hide();
		theApp.emuledlg->kademliawnd->searchList->Hide();
		Kademlia::CPrefs* startupKadPref = new Kademlia::CPrefs();
		startupKadPref->setTCPPort(theApp.glob_prefs->GetPort());
		startupKadPref->setUDPPort(theApp.glob_prefs->GetKadUDPPort());
		Kademlia::CUInt128 clientID;
		clientID.setValue((uchar*)theApp.glob_prefs->GetUserHash());
		startupKadPref->setClientHash(clientID);
		Kademlia::CKademlia::start(startupKadPref);
		Kademlia::CKademlia::setSharedFileList(theApp.sharedfiles);
		theApp.emuledlg->kademliawnd->contactList->Visable();
		theApp.emuledlg->kademliawnd->searchList->Visable();
		theApp.emuledlg->ShowConnectionState();
	}
}

struct SKadStopParams
{
	DWORD dwFlags;
	CKademliaMain* pKadMain;
};

UINT AFX_CDECL KadStopFunc(LPVOID pvParams)
{
	DWORD dwFlags = ((SKadStopParams*)pvParams)->dwFlags;
	CKademliaMain* pKadMain = ((SKadStopParams*)pvParams)->pKadMain;
	delete (SKadStopParams*)pvParams;
	pvParams = NULL;

	Kademlia::CKademlia::stop(FALSE);
	// if that thread was started during application shutdown, the 'theApp.emuledlg' may get
	// deleted while this thread is still running.
	if ((dwFlags & 1) && (theApp.emuledlg != NULL && theApp.emuledlg->IsRunning()))
	{
		theApp.emuledlg->kademliawnd->contactList->Visable();
		theApp.emuledlg->kademliawnd->searchList->Visable();
		theApp.emuledlg->kademliawnd->GetDlgItem(IDC_KADCONNECT)->EnableWindow(true);
		theApp.emuledlg->toolbar->EnableButton(IDC_TOOLBARBUTTON+0);
		//TODO: Test this alot.. The connect button may still get confused which state it should be in.
		pKadMain->m_status->m_connected = false;
		pKadMain->m_status->m_firewalled = true;
		pKadMain->m_status->m_kademliaUsers = 0;
		theApp.emuledlg->ShowConnectionState();
	}
	return 0;
}

void CKademliaMain::DisConnect()
{
	if(Kademlia::CTimer::getThreadID())
	{
		if (theApp.emuledlg->IsRunning())
		{
			theApp.emuledlg->toolbar->EnableButton(IDC_TOOLBARBUTTON+0, false);
			theApp.emuledlg->kademliawnd->GetDlgItem(IDC_KADCONNECT)->EnableWindow(false);
			theApp.emuledlg->kademliawnd->contactList->Hide();
			theApp.emuledlg->kademliawnd->searchList->Hide();
		}

		if (theApp.emuledlg->IsRunning())
		{
			SKadStopParams* pStopParams = new SKadStopParams;
			pStopParams->dwFlags = theApp.emuledlg->IsRunning() ? 1 : 0;
			pStopParams->pKadMain = this;
			CWinThread* pKadStopThread = new CWinThread(KadStopFunc, (LPVOID)pStopParams);
			if (!pKadStopThread->CreateThread())
			{
				delete pKadStopThread;
				delete pStopParams;
				theApp.emuledlg->kademliawnd->contactList->Visable();
				theApp.emuledlg->kademliawnd->searchList->Visable();
				theApp.emuledlg->kademliawnd->GetDlgItem(IDC_KADCONNECT)->EnableWindow(true);
				theApp.emuledlg->toolbar->EnableButton(IDC_TOOLBARBUTTON+0);
				theApp.emuledlg->ShowConnectionState();
				ASSERT(0);
				return;
			}
		}
		else
		{
			// this is at least needed for a debug build. if we want to stop the kad threads and properly
			// free all memory to detect mem leaks, we have to wait until the kad threads finished and performed
			// their cleanup code. this takes a noticeable amount of time and could maybe avoided for a release build.
			// though, even for a release build there should be some special shutdown code which waits at least as long
			// as the kad threads have closed their sockets.
			//
			// a fast way (for a release build only; and only for application shutdown) could be to properly let the
			// kad threads exit the message loops, close the sockets, save the files (all this should not take too
			// much time) in a synchronized way and after that just asynchronously kill the kad threads, not caring
			// about memory (will be freed by system).
			Kademlia::CKademlia::stop(TRUE);
		}
	}
	//Please leave this in a bit for testing to see if it does lockup.. 
	if(Kademlia::CTimer::getThreadID())
	{
	}
	m_status->m_connected = false;
	m_status->m_firewalled = true;
	m_status->m_kademliaUsers = 0;
	theApp.emuledlg->ShowConnectionState();
}

void CKademliaMain::Bootstrap(CString ip,uint16 port) 
{
	if(getStatus()->m_totalContacts)
		return;
	if( Kademlia::CTimer::getThreadID() && !isConnected() && time(NULL)-m_bootstrapTimer > 10)
	{
		if( ip!="" && port )
		{
			Kademlia::CKademlia::getUDPListener()->bootstrap(ip,port);
			m_bootstrapTimer = time(NULL);
		}
		else
		{
			CUpDownClient* client = theApp.clientlist->GetRandomKadClient();
			if( client )
			{
				Kademlia::CKademlia::getUDPListener()->bootstrap(ntohl(client->GetIP()),client->GetKadPort());
				m_bootstrapTimer = time(NULL);
			}
		}
	}
}

void CKademliaMain::Bootstrap(uint32 ip,uint16 port) 
{
	if(getStatus()->m_totalContacts)
		return;
	if( Kademlia::CTimer::getThreadID() && !isConnected() && time(NULL)-m_bootstrapTimer > 10)
	{
		if( ip && port )
		{
			Kademlia::CKademlia::getUDPListener()->bootstrap(ip,port);
			m_bootstrapTimer = time(NULL);
		}
		else
		{
			CUpDownClient* client = theApp.clientlist->GetRandomKadClient();
			if( client )
			{
				Kademlia::CKademlia::getUDPListener()->bootstrap(ntohl(client->GetIP()),client->GetKadPort());
				m_bootstrapTimer = time(NULL);
			}
		}
	}
}

Status*	CKademliaMain::getStatus()
{
	return m_status;
}

bool CKademliaMain::isConnected()
{
	return m_status->m_connected;
}

bool CKademliaMain::isFirewalled()
{
	return m_status->m_firewalled;
}

uint32 CKademliaMain::getIP()
{
	return m_status->m_ip;
}

uint16 CKademliaMain::getUdpPort()
{
	return m_status->m_udpport;
}

uint16 CKademliaMain::getTcpPort()
{
	return m_status->m_tcpport;
}
