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
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "clientlist.h" // NEO: XSF - [ExtendedSharedFiles] <-- Xanatos --
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
#include "Neo\GUI\IP2Country.h"
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

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
	AddAnchor(IDC_STATIC54, TOP_LEFT, TOP_RIGHT); // NEO: NMP - [NeoModProt] <-- Xanatos --
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

		CString buffer;
		if (client->GetUserName())
			GetDlgItem(IDC_DNAME)->SetWindowText(client->GetUserName());
		else
			GetDlgItem(IDC_DNAME)->SetWindowText(_T("?"));
		
		if (client->HasValidHash())
			GetDlgItem(IDC_DHASH)->SetWindowText(md4str(client->GetUserHash()));
		else
			GetDlgItem(IDC_DHASH)->SetWindowText(_T("?"));
		
		//GetDlgItem(IDC_DSOFT)->SetWindowText(client->GetClientSoftVer());
		GetDlgItem(IDC_DSOFT)->SetWindowText(client->DbgGetFullClientSoftVer()); // NEO: MIDI - [ModIDInfo] <-- Xanatos --

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
			buffer += _T("(In Use)");
#endif
		GetDlgItem(IDC_OBFUSCATION_STAT)->SetWindowText(buffer);

		// NEO: MOD - [ShowIP] -- Xanatos -->
		buffer.Format(_T("%s:%i"), ipstr(client->GetIP()), client->GetUserPort());
		GetDlgItem(IDC_DIP)->SetWindowText(buffer);
		// NEO: MOD END <-- Xanatos --

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
		GetDlgItem(IDC_DSCOUNTRY)->SetWindowText(client->GetCountryName(true));
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

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


		// NEO: NMP - [NeoModProt] -- Xanatos -->
		if(client->GetCommentVersion())
			buffer.Format(GetResString(IDS_X_CD_PV),client->GetCommentVersion());	
		else
			buffer.Format(GetResString(IDS_X_CD_US));
		GetDlgItem(IDC_DPCOMMENTS)->SetWindowText(buffer);

		if(client->SupportsModProt())
			buffer.Format(GetResString(IDS_X_CD_S));	
		else
			buffer.Format(GetResString(IDS_X_CD_US));
		GetDlgItem(IDC_DNMP)->SetWindowText(buffer);

		if(client->SupportsNeoXS())
			buffer.Format(GetResString(IDS_X_CD_S));
		else
			buffer.Format(GetResString(IDS_X_CD_US));
		GetDlgItem(IDC_DNMXS)->SetWindowText(buffer);

		if(client->SupportsSubChunks())
			buffer.Format(GetResString(IDS_X_CD_S));
		else
			buffer.Format(GetResString(IDS_X_CD_US));
		GetDlgItem(IDC_DSCT)->SetWindowText(buffer);

		if(client->GetIncompletePartVersion())
			buffer.Format(GetResString(IDS_X_CD_PV),(UINT)client->GetIncompletePartVersion());
		else
			buffer.Format(GetResString(IDS_X_CD_US));
		GetDlgItem(IDC_DPINCP)->SetWindowText(buffer);
		// NEO: NMP END <-- Xanatos --

#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
		if(!NeoPrefs.IsNatTraversalEnabled())
			buffer.Format(GetResString(IDS_DISABLED));
		else if(client->SupportsNatTraversal())
			buffer.Format(GetResString(IDS_X_CD_PV4), (UINT)client->GetUDPPort()
			, GetResString(client->socket && client->socket->HaveNatLayer() ? IDS_YES : IDS_NO));
		else
			buffer.Format(GetResString(IDS_X_CD_US));
		GetDlgItem(IDC_DUDP)->SetWindowText(buffer);
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --

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

		buffer.Format(_T("%s"),CastItoXBytes(client->GetDatarate(), false, true));
		GetDlgItem(IDC_DAVDR)->SetWindowText(buffer);
		
		if (client->Credits()){
			GetDlgItem(IDC_DUPTOTAL)->SetWindowText(CastItoXBytes(client->Credits()->GetDownloadedTotal(), false, false));
			GetDlgItem(IDC_DDOWNTOTAL)->SetWindowText(CastItoXBytes(client->Credits()->GetUploadedTotal(), false, false));
			//buffer.Format(_T("%.1f"),(float)client->Credits()->GetScoreRatio(client->GetIP()));
			// NEO: NCS - [NeoCreditSystem] -- Xanatos -->
			float modif = 1.0F;
			if(thePrefs.UseCreditSystem())
				if (NeoPrefs.UseNeoCreditSystem())
					modif = client->Credits()->GetNeoScoreRatio(client->GetRequestFile() != NULL,client->GetIP());
				else
					modif = client->Credits()->GetScoreRatio(client->GetIP());
			buffer.Format(_T("%.1f"),modif);
			// NEO: NCS END <-- Xanatos --
			GetDlgItem(IDC_DRATIO)->SetWindowText(buffer);
			
			if (theApp.clientcredits->CryptoAvailable()){
				switch(client->Credits()->GetCurrentIdentState(client->GetIP())){
					case IS_NOTAVAILABLE:
						GetDlgItem(IDC_CDIDENT)->SetWindowText(GetResString(IDS_IDENTNOSUPPORT));
						break;
					case IS_IDFAILED:
						GetDlgItem(IDC_CDIDENT)->SetWindowText(GetResString(IDS_IDENTFAILED));
						break;
					case IS_IDNEEDED:
						GetDlgItem(IDC_CDIDENT)->SetWindowText(GetResString(IDS_X_IDENTNEED)); // NEO: MOD <-- Xanatos --
						break;
					case IS_IDBADGUY:
						GetDlgItem(IDC_CDIDENT)->SetWindowText(GetResString(IDS_X_IDBADGUY)); // NEO: MOD <-- Xanatos --
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

		if (client->GetUserName() && client->Credits()!=NULL){
			buffer.Format(_T("%.1f"),(float)client->GetScore(false,client->IsDownloading(),true));
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

	// NEO: NMP - [NeoModProt] -- Xanatos -->
	GetDlgItem(IDC_STATIC54)->SetWindowText(GetResString(IDS_X_CD_PROTOCOL));
	GetDlgItem(IDC_STATIC49)->SetWindowText(GetResString(IDS_X_CD_COMMENTS));
	GetDlgItem(IDC_STATIC56)->SetWindowText(GetResString(IDS_X_CD_INCPARTS));
	GetDlgItem(IDC_STATIC57)->SetWindowText(GetResString(IDS_X_CD_SCT));
	GetDlgItem(IDC_STATIC58)->SetWindowText(GetResString(IDS_X_CD_NEOPROT));
	GetDlgItem(IDC_STATIC59)->SetWindowText(GetResString(IDS_X_CD_NEOXS));
	// NEO: NMP END <-- Xanatos --
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
	GetDlgItem(IDC_STATIC60)->SetWindowText(GetResString(IDS_X_CD_NAT_TRAVERSAL));
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --

	GetDlgItem(IDC_STATIC50)->SetWindowText(GetResString(IDS_CD_SCORES));
	GetDlgItem(IDC_STATIC51)->SetWindowText(GetResString(IDS_CD_MOD));
	GetDlgItem(IDC_STATIC52)->SetWindowText(GetResString(IDS_CD_RATING));
	GetDlgItem(IDC_STATIC53)->SetWindowText(GetResString(IDS_CD_USCORE));
	GetDlgItem(IDC_STATIC133x)->SetWindowText(GetResString(IDS_CD_IDENT));
	GetDlgItem(IDC_CLIENTDETAIL_KAD)->SetWindowText(GetResString(IDS_KADEMLIA) + _T(":"));
}


///////////////////////////////////////////////////////////////////////////////
// CClientDetailDialog

IMPLEMENT_DYNAMIC(CClientDetailDialog, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CClientDetailDialog, CModListViewWalkerPropertySheet) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	ON_WM_DESTROY()
END_MESSAGE_MAP()

//CClientDetailDialog::CClientDetailDialog(CUpDownClient* pClient, CListCtrlItemWalk* pListCtrl)
CClientDetailDialog::CClientDetailDialog(CUpDownClient* pClient, CListCtrlItemWalk* pListCtrl, UINT uInvokePage) // NEO: XSF - [ExtendedSharedFiles] <-- Xanatos --
	: CModListViewWalkerPropertySheet(pListCtrl) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	m_aItems.Add(pClient);
	//Construct();
	Construct(uInvokePage); // NEO: XSF - [ExtendedSharedFiles] <-- Xanatos --
}

CClientDetailDialog::CClientDetailDialog(const CSimpleArray<CUpDownClient*>* paClients, CListCtrlItemWalk* pListCtrl)
	: CModListViewWalkerPropertySheet(pListCtrl) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	for (int i = 0; i < paClients->GetSize(); i++)
		m_aItems.Add((*paClients)[i]);
	Construct();
}

//void CClientDetailDialog::Construct()
void CClientDetailDialog::Construct(UINT uInvokePage) // NEO: XSF - [ExtendedSharedFiles] <-- Xanatos --
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	m_wndClient.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndClient.m_psp.dwFlags |= PSP_USEICONID;
	m_wndClient.m_psp.pszIcon = _T("CLIENTDETAILS");
	m_wndClient.SetClients(&m_aItems);
	AddPage(&m_wndClient);

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
	if(NeoPrefs.EnableSourceList()){
		m_wndSource.m_psp.dwFlags &= ~PSP_HASHELP;
		m_wndSource.m_psp.dwFlags |= PSP_USEICONID;
		m_wndSource.m_psp.pszIcon = _T("SOURCESAVER");
		m_wndSource.SetClients(&m_aItems);
		AddPage(&m_wndSource);
	}
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

	// NEO: RFL - [RequestFileList] -- Xanatos -->
	m_wndReqFiles.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndReqFiles.m_psp.dwFlags |= PSP_USEICONID;
	m_wndReqFiles.m_psp.pszIcon = _T("CLIENTREQFILES");
	m_wndReqFiles.SetClients(&m_aItems);
	AddPage(&m_wndReqFiles);
	// NEO: RFL END <-- Xanatos --

	// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
	m_wndSharedFiles.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndSharedFiles.m_psp.dwFlags |= PSP_USEICONID;
	m_wndSharedFiles.m_psp.pszIcon = _T("VIEWFILES");
	m_wndSharedFiles.SetClients(&m_aItems);
	AddPage(&m_wndSharedFiles);
	// NEO: XSF END <-- Xanatos --

	// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
	LPCTSTR pPshStartPage = MAKEINTRESOURCE(IDD_SOURCEDETAILWND);
	if (uInvokePage != 0)
		pPshStartPage = MAKEINTRESOURCE(uInvokePage);
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == pPshStartPage)
		{
			m_psh.nStartPage = i;
			break;
		}
	}
	// NEO: XSF END <-- Xanatos --
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
	BOOL bResult = CModListViewWalkerPropertySheet::OnInitDialog(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("ClientDetailDialog")); // call this after(!) OnInitDialog
	SetWindowText(GetResString(IDS_CD_TITLE));
	return bResult;
}
