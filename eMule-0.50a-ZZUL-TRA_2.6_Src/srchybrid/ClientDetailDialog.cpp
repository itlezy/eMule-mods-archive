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
#include "Addons/IP2Country/IP2Country.h" //EastShare - added by AndCycle, IP to Country
#ifdef CLIENTANALYZER
#include "Addons/AntiLeech/ClientAnalyzer.h"
#endif

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
	m_paClients		= NULL;
	m_bDataChanged	= false;
	m_strCaption	= GetResString(IDS_CD_TITLE);
	m_psp.pszTitle	= m_strCaption;
	m_psp.dwFlags  |= PSP_USETITLE;
}

CClientDetailPage::~CClientDetailPage()
{
}

void CClientDetailPage::DoDataExchange(CDataExchange* pDX)
{
	CResizablePage::DoDataExchange(pDX);
}

BOOL CClientDetailPage::OnInitDialog()
{
	CResizablePage::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDC_STATIC30, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC40, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC50, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_DNAME, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_DSNAME, TOP_LEFT, TOP_RIGHT);
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
		CUpDownClient* client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);

		//>> pP: FunnyNick
		CString buffer;
		if (client->GetUserName() && client->GetUserName() != client->GetUserName(false))
		{
			CString temp;
			temp.Format(L"%s [%s]", client->GetUserName(), client->GetUserName(false));
			GetDlgItem(IDC_DNAME)->SetWindowText(temp);
		}
		else
		//>> pP: FunnyNick
		if (client->GetUserName())
			GetDlgItem(IDC_DNAME)->SetWindowText(client->GetUserName());
		else
			GetDlgItem(IDC_DNAME)->SetWindowText(_T("?"));

		//EastShare Start - added by AndCycle, IP to Country
		// Superlexx
		bool longCountryName = true;
		GetDlgItem(IDC_DLOC)->SetWindowText(client->GetCountryName(longCountryName));
		//MORPH START - Added by Commander, CountryFlag
		if (theApp.ip2country->ShowCountryFlag()){
			countryflag = theApp.ip2country->GetFlagImageList()->ExtractIcon(client->GetCountryFlagIndex());
			((CStatic*)GetDlgItem(IDC_COUNTRYFLAG))->SetIcon(countryflag);
			((CStatic*)GetDlgItem(IDC_COUNTRYFLAG))->ShowWindow(SW_SHOW);
			RECT rect1;
			RECT rect2;
			((CStatic*)GetDlgItem(IDC_COUNTRYFLAG))->GetWindowRect(&rect1);
			GetDlgItem(IDC_DLOC)->GetWindowRect(&rect2);
			ScreenToClient(&rect1);
			ScreenToClient(&rect2);
			GetDlgItem(IDC_DLOC)->MoveWindow(CRect(rect1.right+2, rect2.top, rect2.right, rect2.bottom),TRUE);
		}
		//MORPH END - Added by Commander, CountryFlag
		//EastShare End - added by AndCycle, IP to Country

		if (client->HasValidHash())
			GetDlgItem(IDC_DHASH)->SetWindowText(md4str(client->GetUserHash()));
		else
			GetDlgItem(IDC_DHASH)->SetWindowText(_T("?"));
		
		GetDlgItem(IDC_DSOFT)->SetWindowText(client->DbgGetFullClientSoftVer());

		//Show IP (kts)
		if(client->GetIP() && client->GetUserPort()){
			CString strUserIp;
			strUserIp.Format(_T("%s : %u"),ipstr(client->GetIP()), client->GetUserPort());
			GetDlgItem(IDC_NEIP)->SetWindowText(strUserIp);
		}
		else
			GetDlgItem(IDC_NEIP)->SetWindowText(_T("-"));
		//Show IP (kts)
		if (client->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (client->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()) 
			&& (client->IsObfuscatedConnectionEstablished() || !(client->socket != NULL && client->socket->IsConnected())))
		{
			buffer = GetResString(IDS_ENABLED);
		}
		else if (client->SupportsCryptLayer())
			buffer = GetResString(IDS_SUPPORTED);
		else
			buffer = GetResString(IDS_IDENTNOSUPPORT);
//#if defined(_DEBUG) // Spike2 - show it always
		if (client->IsObfuscatedConnectionEstablished())
			buffer += _T(" (In Use)"); //Spike2 added one Space to make it look better
//#endif
		GetDlgItem(IDC_OBFUSCATION_STAT)->SetWindowText(buffer);

		buffer.Format(_T("%s"),(client->HasLowID() ? GetResString(IDS_IDLOW):GetResString(IDS_IDHIGH)));
		GetDlgItem(IDC_DID)->SetWindowText(buffer);
		
		if (client->GetServerIP()){
			GetDlgItem(IDC_DSIP)->SetWindowText(ipstr(client->GetServerIP()));
			CServer* cserver = theApp.serverlist->GetServerByIPTCP(client->GetServerIP(), client->GetServerPort());
			if (cserver)
				GetDlgItem(IDC_DSNAME)->SetWindowText(cserver->GetListName());
			else
				GetDlgItem(IDC_DSNAME)->SetWindowText(_T("?"));
		}
		else{
			GetDlgItem(IDC_DSIP)->SetWindowText(_T("?"));
			GetDlgItem(IDC_DSNAME)->SetWindowText(_T("?"));
		}

		CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
		if (file)
			GetDlgItem(IDC_DDOWNLOADING)->SetWindowText(file->GetFileName());
		else
			GetDlgItem(IDC_DDOWNLOADING)->SetWindowText(_T("-"));

		if (client->GetRequestFile())
			GetDlgItem(IDC_UPLOADING)->SetWindowText( client->GetRequestFile()->GetFileName()  );
		else 
			GetDlgItem(IDC_UPLOADING)->SetWindowText(_T("-"));

		GetDlgItem(IDC_DDUP)->SetWindowText(CastItoXBytes(client->GetTransferredDown(), false, false));

		GetDlgItem(IDC_DDOWN)->SetWindowText(CastItoXBytes(client->GetTransferredUp(), false, false));

		buffer.Format(_T("%s"), CastItoXBytes(client->GetDownloadDatarate(), false, true));
		GetDlgItem(IDC_DAVUR)->SetWindowText(buffer);
//ZZUL +
        if(client->GetDatarate() >= 0) {
		    buffer.Format(_T("%s"),CastItoXBytes((uint32)client->GetDatarate(), false, true));
        } else {
            buffer.Format(_T("?? %s"),GetResString(IDS_KBYTESPERSEC));
        }
//ZZUL -
		GetDlgItem(IDC_DAVDR)->SetWindowText(buffer);
		
#ifdef CLIENTANALYZER
		buffer = L"-";
#endif				
		if (client->Credits()){
			GetDlgItem(IDC_DUPTOTAL)->SetWindowText(CastItoXBytes(client->Credits()->GetDownloadedTotal(), false, false));
			GetDlgItem(IDC_DDOWNTOTAL)->SetWindowText(CastItoXBytes(client->Credits()->GetUploadedTotal(), false, false));
			switch (thePrefs.UseCreditSystem())
				{              		  
					case 1:
			            buffer.Format(_T("%.1f [%.1f]"),(float)client->Credits()->GetScoreRatio(client->GetIP()), (float)client->Credits()->GetMyScoreRatio(client->GetIP()));  // ZZUL-TRA :: OwnCredits
						break;
#ifdef CLIENTANALYZER
					case 2: 
						if(client->GetAntiLeechData())
			            buffer.Format(_T("%.1f [%.1f]"),(float)client->GetAntiLeechData()->GetScore(),(float)client->Credits()->GetMyScoreRatio(client->GetIP()));  // ZZUL-TRA :: OwnCredits
#endif						break;
			   }

#ifndef CLIENTANALYZER
			GetDlgItem(IDC_DRATIO)->SetWindowText(buffer);
#endif
			
			if (theApp.clientcredits->CryptoAvailable()){
				switch(client->Credits()->GetCurrentIdentState(client->GetIP())){
					case IS_NOTAVAILABLE:
						GetDlgItem(IDC_CDIDENT)->SetWindowText(GetResString(IDS_IDENTNOSUPPORT));
						break;
					case IS_IDFAILED:
					case IS_IDNEEDED:
					case IS_IDBADGUY:
						GetDlgItem(IDC_CDIDENT)->SetWindowText(GetResString(IDS_IDENTFAILED));
						break;
					case IS_IDENTIFIED:
						GetDlgItem(IDC_CDIDENT)->SetWindowText(GetResString(IDS_IDENTOK));
						break;
				}
			}
			else
				GetDlgItem(IDC_CDIDENT)->SetWindowText(GetResString(IDS_IDENTNOSUPPORT));
		}	
		else{
			GetDlgItem(IDC_DDOWNTOTAL)->SetWindowText(_T("?"));
			GetDlgItem(IDC_DUPTOTAL)->SetWindowText(_T("?"));
			GetDlgItem(IDC_DRATIO)->SetWindowText(_T("?"));
			GetDlgItem(IDC_CDIDENT)->SetWindowText(_T("?"));
		}

#ifdef CLIENTANALYZER
		GetDlgItem(IDC_DRATIO)->SetWindowText(buffer);
		if (client->IsBadGuy()) 
		{
			GetDlgItem(IDC_ANTILEECH_INFO)->SetWindowText(client->GetAntiLeechData()->GetAntiLeechDataString());
			GetDlgItem(IDC_ANTILEECH_LABEL)->SetWindowText(GetResString(IDS_ANTILEECH_LABEL));
		}
		else 
		{
			GetDlgItem(IDC_ANTILEECH_INFO)->SetWindowText(L"");
			GetDlgItem(IDC_ANTILEECH_LABEL)->SetWindowText(L"");
		}
#endif

		if (client->GetUserName() && client->Credits()!=NULL){
			buffer.Format(_T("%.0f"),(float)client->GetScore(false,client->IsDownloading(),true)); //morph4u %.1f -> %.0f, keine komma stellen
			GetDlgItem(IDC_DRATING)->SetWindowText(buffer);
		}
		else
			GetDlgItem(IDC_DRATING)->SetWindowText(_T("?"));

		if (client->GetUploadState() != US_NONE && client->Credits()!=NULL){
			if (!client->GetFriendSlot()){
				buffer.Format(_T("%u"),client->GetScore(false,client->IsDownloading(),false));
				GetDlgItem(IDC_DSCORE)->SetWindowText(buffer);
			}
			else
				GetDlgItem(IDC_DSCORE)->SetWindowText(GetResString(IDS_FRIENDDETAIL));
		}
		else
			GetDlgItem(IDC_DSCORE)->SetWindowText(_T("-"));

		if (client->GetKadPort() )
			buffer.Format( _T("%s"), GetResString(IDS_CONNECTED));
		else
			buffer.Format( _T("%s"), GetResString(IDS_DISCONNECTED));
		GetDlgItem(IDC_CLIENTDETAIL_KADCON)->SetWindowText(buffer);

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
	GetDlgItem(IDC_STATIC30)->SetWindowText(GetResString(IDS_CD_GENERAL));
	GetDlgItem(IDC_STATIC31)->SetWindowText(GetResString(IDS_CD_UNAME));
	GetDlgItem(IDC_STATIC32)->SetWindowText(GetResString(IDS_CD_UHASH));
	GetDlgItem(IDC_STATIC33)->SetWindowText(GetResString(IDS_CD_CSOFT) + _T(':'));
	GetDlgItem(IDC_STATIC35)->SetWindowText(GetResString(IDS_CD_SIP));
	GetDlgItem(IDC_STATIC38)->SetWindowText(GetResString(IDS_CD_SNAME));
	GetDlgItem(IDC_STATIC_OBF_LABEL)->SetWindowText(GetResString(IDS_OBFUSCATION) + _T(':'));

	GetDlgItem(IDC_STATIC40)->SetWindowText(GetResString(IDS_CD_TRANS));
	GetDlgItem(IDC_STATIC41)->SetWindowText(GetResString(IDS_CD_CDOWN));
	GetDlgItem(IDC_STATIC42)->SetWindowText(GetResString(IDS_CD_DOWN));
	GetDlgItem(IDC_STATIC43)->SetWindowText(GetResString(IDS_CD_ADOWN));
	GetDlgItem(IDC_STATIC44)->SetWindowText(GetResString(IDS_CD_TDOWN));
	GetDlgItem(IDC_STATIC45)->SetWindowText(GetResString(IDS_CD_UP));
	GetDlgItem(IDC_STATIC46)->SetWindowText(GetResString(IDS_CD_AUP));
	GetDlgItem(IDC_STATIC47)->SetWindowText(GetResString(IDS_CD_TUP));
	GetDlgItem(IDC_STATIC48)->SetWindowText(GetResString(IDS_CD_UPLOADREQ));

	GetDlgItem(IDC_STATIC50)->SetWindowText(GetResString(IDS_CD_SCORES));
	GetDlgItem(IDC_STATIC51)->SetWindowText(GetResString(IDS_CD_MOD));
	GetDlgItem(IDC_STATIC52)->SetWindowText(GetResString(IDS_CD_RATING));
	GetDlgItem(IDC_STATIC53)->SetWindowText(GetResString(IDS_CD_USCORE));
	GetDlgItem(IDC_STATIC133x)->SetWindowText(GetResString(IDS_CD_IDENT));
	GetDlgItem(IDC_DLOC2)->SetWindowText(GetResString(IDS_COUNTRY) + _T(":"));
	GetDlgItem(IDC_CLIENTDETAIL_KAD)->SetWindowText(GetResString(IDS_KADEMLIA) + _T(":"));
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
	m_wndClient.m_psp.pszIcon = _T("CLIENTDETAILS");
	m_wndClient.SetClients(&m_aItems);
	AddPage(&m_wndClient);

	// ZZUL-TRA :: ReqFiles :: Start
	m_wndReqFiles.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndReqFiles.m_psp.dwFlags |= PSP_USEICONID;
	m_wndReqFiles.m_psp.pszIcon = _T("CLIENTREQFILES");
	m_wndReqFiles.SetClients(&m_aItems);
	AddPage(&m_wndReqFiles);
	// ZZUL-TRA :: ReqFiles :: End
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
	EnableSaveRestore(_T("ClientDetailDialog")); // call this after(!) OnInitDialog
	SetWindowText(GetResString(IDS_CD_TITLE));
	return bResult;
}
