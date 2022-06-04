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
#include "stdafx.h"
#include "emule.h"
#include "ClientDetailDialog.h"
#include "UpDownClient.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "otherfunctions.h"
#include "Server.h"
#include "ServerList.h"
#include "SharedFileList.h"
#include "HighColorTab.hpp"
#include "UserMsgs.h"
#include "ListenSocket.h"
#include "preferences.h"
#include "IP2Country.h" //EastShare - added by AndCycle, IP to Country
#include "UploadQueue.h" //Xman Queuerank at clientdetail


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CClientDetailPage

IMPLEMENT_DYNAMIC(CClientDetailPage, CResizablePage)

BEGIN_MESSAGE_MAP(CClientDetailPage, CResizablePage)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CClientDetailPage::CClientDetailPage()
	: CResizablePage(CClientDetailPage::IDD, 0 )
{
	m_paClients = NULL;
	m_bDataChanged = false;
	m_strCaption	= GetResString(IDS_CD_TITLE);
	m_psp.pszTitle	= m_strCaption;
	m_psp.dwFlags  |= PSP_USETITLE;
}

CClientDetailPage::~CClientDetailPage()
{
}

BOOL CClientDetailPage::OnInitDialog()
{
	CResizablePage::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDC_STATIC30, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC40, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC50, TOP_LEFT, TOP_RIGHT);
	//zz_fly :: let it resize together with the dialog
	AddAnchor(IDC_IP, TOP_LEFT, TOP_CENTER); 
	//AddAnchor(IDC_DLOC, TOP_CENTER, TOP_RIGHT);
	//AddAnchor(IDC_DSOFT, TOP_LEFT, TOP_RIGHT);
	//zz_fly :: end
	AddAnchor(IDC_DDOWNLOADING, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_UPLOADING, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_OBFUSCATION_STAT, TOP_LEFT, TOP_RIGHT);
	
	Localize();
	return TRUE;
}

BOOL CClientDetailPage::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;

	if (m_bDataChanged)
	{

		//Xman Code Fix
		//don't know how this happend, but happend with a friend:
		if(m_paClients==NULL)
			return FALSE;
		//Xman end

		//CUpDownClient* client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
		CUpDownClient* client = (CUpDownClient*)(*m_paClients)[0];

		CString buffer;
		SetDlgItemText(IDC_IP,client->GetUserIPString()?client->GetUserIPString():_T("?"));
		
		//EastShare Start - added by AndCycle, IP to Country
		SetDlgItemText(IDC_DLOC,client->GetCountryName(/*true*/));
		//EastShare End - added by AndCycle, IP to Country

		SetDlgItemText(IDC_DHASH,client->HasValidHash()?md4str(client->GetUserHash()):_T("?"));
		
		SetDlgItemText(IDC_DSOFT,client->DbgGetFullClientSoftVer()); //Xman ModId

		if (client->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (client->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()) 
			&& (client->IsObfuscatedConnectionEstablished() || !(client->socket != NULL && client->socket->IsConnected())))
		{
			buffer = GetResString(IDS_ENABLED);
		}
		else if (client->SupportsCryptLayer())
			buffer = GetResString(IDS_SUPPORTED);
		else
			buffer = GetResString(IDS_IDENTNOSUPPORT);
#if defined(_DEBUG) 
		if (client->IsObfuscatedConnectionEstablished())
			buffer += _T(" (In Use)");
#endif
		SetDlgItemText(IDC_OBFUSCATION_STAT,buffer);

		buffer = (client->HasLowID() ? GetResString(IDS_IDLOW):GetResString(IDS_IDHIGH));
		SetDlgItemText(IDC_DID,buffer);
		
		if (client->GetServerIP()){
			SetDlgItemText(IDC_DSIP,ipstr(client->GetServerIP()));
			
			CServer* cserver = theApp.serverlist->GetServerByIPTCP(client->GetServerIP(), client->GetServerPort());
			if (cserver)
				SetDlgItemText(IDC_DSNAME,cserver->GetListName());
			else
				SetDlgItemText(IDC_DSNAME,_T("?"));
		}
		else{
			SetDlgItemText(IDC_DSIP,_T("?"));
			SetDlgItemText(IDC_DSNAME,_T("?"));
		}

		//Xman Queuerank at clientdetail
		if(client->GetUploadState()==US_ONUPLOADQUEUE)
			buffer.Format(_T("%u"),theApp.uploadqueue->GetWaitingPosition(client));
		else
			buffer = _T('-');
		SetDlgItemText(IDC_DOWNQUEUERANK,buffer);
		if(client->GetDownloadState()==DS_ONQUEUE)
		{
			if(client->IsRemoteQueueFull())
				buffer = GetResString(IDS_QUEUEFULL);
			else
				buffer.Format(_T("%u"), client->GetRemoteQueueRank());
		}
		else
			buffer = _T('-');
		SetDlgItemText(IDC_UPLOADQUEURANK,buffer);
		//Xman end


		CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
		SetDlgItemText(IDC_DDOWNLOADING,file?file->GetFileName():_T("-"));

		SetDlgItemText(IDC_UPLOADING, client->GetRequestFile()?client->GetRequestFile()->GetFileName():_T("-"));

		SetDlgItemText(IDC_DDUP,CastItoXBytes(client->GetTransferredDown(), false, false));

		SetDlgItemText(IDC_DDOWN,CastItoXBytes(client->GetTransferredUp(), false, false));

		buffer = CastItoXBytes(client->GetDownloadDatarate(), false, true);
		SetDlgItemText(IDC_DAVUR,buffer);

		buffer = CastItoXBytes(client->GetUploadDatarate(), false, true); //Xman // Maella -Accurate measure of bandwidth
		SetDlgItemText(IDC_DAVDR,buffer);
		
		if (client->Credits()){
			SetDlgItemText(IDC_DUPTOTAL,CastItoXBytes(client->Credits()->GetDownloadedTotal(), false, false));
			SetDlgItemText(IDC_DDOWNTOTAL,CastItoXBytes(client->Credits()->GetUploadedTotal(), false, false));
			buffer.Format(_T("%.1f [%.1f]"),(float)client->Credits()->GetScoreRatio(client->GetIP()), (float)client->Credits()->GetMyScoreRatio(client->GetIP())); //  See own credits VQB
			SetDlgItemText(IDC_DRATIO,buffer);
			
			if (theApp.clientcredits->CryptoAvailable()){
				switch(client->Credits()->GetCurrentIdentState(client->GetIP())){
					case IS_NOTAVAILABLE:
						SetDlgItemText(IDC_CDIDENT,GetResString(IDS_IDENTNOSUPPORT));
						break;
					case IS_IDFAILED:
					case IS_IDNEEDED:
					case IS_IDBADGUY:
						SetDlgItemText(IDC_CDIDENT,GetResString(IDS_IDENTFAILED));
						break;
					case IS_IDENTIFIED:
						SetDlgItemText(IDC_CDIDENT,GetResString(IDS_IDENTOK));
						break;
				}
			}
			else
				SetDlgItemText(IDC_CDIDENT,GetResString(IDS_IDENTNOSUPPORT));
		}	
		else{
			SetDlgItemText(IDC_DDOWNTOTAL,_T("?"));
			SetDlgItemText(IDC_DUPTOTAL,_T("?"));
			SetDlgItemText(IDC_DRATIO,_T("?"));
			SetDlgItemText(IDC_CDIDENT,_T("?"));
		}

		if (client->GetUserName() && client->Credits()!=NULL){
			buffer.Format(_T("%.1f"),(float)client->GetScore(false,client->IsDownloading(),true));
			SetDlgItemText(IDC_DRATING,buffer);
		}
		else
			SetDlgItemText(IDC_DRATING,_T("?"));

		if (client->GetUploadState() != US_NONE && client->Credits()!=NULL){	
			if (!client->IsPBFClient()){ // ==> Pay Back First
				buffer.Format(_T("%u"),client->GetScore(false,client->IsDownloading(),false));
				SetDlgItemText(IDC_DSCORE,buffer);
		}
		else
// ==> Pay Back First
				SetDlgItemText(IDC_DSCORE,GetResString(IDS_PBFDETAIL));
		}
		else
// <== Pay Back First
			SetDlgItemText(IDC_DSCORE,_T("-"));

		buffer = GetResString(client->GetKadPort()?IDS_CONNECTED:IDS_DISCONNECTED);
		SetDlgItemText(IDC_CLIENTDETAIL_KADCON,buffer);

		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CClientDetailPage::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CClientDetailPage::Localize()
{
	SetDlgItemText(IDC_STATIC30,GetResString(IDS_CD_GENERAL));
	SetDlgItemText(IDC_STATIC31,GetResString(IDS_IP) + _T(':'));
	SetDlgItemText(IDC_STATIC32,GetResString(IDS_CD_UHASH));
	SetDlgItemText(IDC_STATIC33,GetResString(IDS_CD_CSOFT) + _T(':'));
	SetDlgItemText(IDC_STATIC35,GetResString(IDS_CD_SIP));
	SetDlgItemText(IDC_STATIC38,GetResString(IDS_CD_SNAME));
	SetDlgItemText(IDC_STATIC_OBF_LABEL,GetResString(IDS_OBFUSCATION) + _T(':'));

	SetDlgItemText(IDC_STATIC40,GetResString(IDS_CD_TRANS));
	SetDlgItemText(IDC_STATIC41,GetResString(IDS_CD_CDOWN));
	SetDlgItemText(IDC_STATIC42,GetResString(IDS_CD_DOWN));
	SetDlgItemText(IDC_STATIC43,GetResString(IDS_CD_ADOWN));
	SetDlgItemText(IDC_STATIC44,GetResString(IDS_CD_TDOWN));
	SetDlgItemText(IDC_STATIC45,GetResString(IDS_CD_UP));
	SetDlgItemText(IDC_STATIC46,GetResString(IDS_CD_AUP));
	SetDlgItemText(IDC_STATIC47,GetResString(IDS_CD_TUP));
	SetDlgItemText(IDC_STATIC48,GetResString(IDS_CD_UPLOADREQ));

	SetDlgItemText(IDC_STATIC_UPQUEUERANK,_T("Upload QueueRank:"));
	SetDlgItemText(IDC_STATIC_DOWNQUEUERANK,_T("Download QueueRank:"));

	SetDlgItemText(IDC_STATIC50,GetResString(IDS_CD_SCORES));
	SetDlgItemText(IDC_STATIC51,GetResString(IDS_CD_MOD));
	SetDlgItemText(IDC_STATIC52,GetResString(IDS_CD_RATING));
	SetDlgItemText(IDC_STATIC53,GetResString(IDS_CD_USCORE));
	SetDlgItemText(IDC_STATIC133x,GetResString(IDS_CD_IDENT));
	SetDlgItemText(IDC_CLIENTDETAIL_KAD,GetResString(IDS_KADEMLIA) + _T(':'));
}


///////////////////////////////////////////////////////////////////////////////
// CClientDetailDialog

IMPLEMENT_DYNAMIC(CClientDetailDialog, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CClientDetailDialog, CListViewWalkerPropertySheet)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CClientDetailDialog::CClientDetailDialog(CUpDownClient* pClient, CListCtrlItemWalk* pListCtrl)
	: CListViewWalkerPropertySheet(pListCtrl)
{
	m_aItems.Add(pClient);
	Construct();
}

CClientDetailDialog::CClientDetailDialog(const CSimpleArray<CUpDownClient*>* paClients, CListCtrlItemWalk* pListCtrl)
	: CListViewWalkerPropertySheet(pListCtrl)
{
	for (int i = 0; i < paClients->GetSize(); i++)
		m_aItems.Add((*paClients)[i]);
	Construct();
}

void CClientDetailDialog::Construct()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	m_wndClient.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndClient.m_psp.dwFlags |= PSP_USEICONID;
	m_wndClient.m_psp.pszIcon = _T("CLIENTSKNOWN");
	m_wndClient.SetClients(&m_aItems);
	AddPage(&m_wndClient);
}

CClientDetailDialog::~CClientDetailDialog()
{
}

void CClientDetailDialog::OnDestroy()
{
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CClientDetailDialog::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult = CListViewWalkerPropertySheet::OnInitDialog();
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("ClientDetailDialog"), !thePrefs.prefReadonly); // call this after(!) OnInitDialog // X: [ROP] - [ReadOnlyPreference]
	SetWindowText(GetResString(IDS_CD_TITLE));
	return bResult;
}
