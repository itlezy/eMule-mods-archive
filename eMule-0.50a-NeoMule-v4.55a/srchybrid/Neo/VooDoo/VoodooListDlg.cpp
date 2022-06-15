//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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
#include "kademlia/kademlia/tag.h"
#include "VoodooListDlg.h"
#include "OtherFunctions.h"
#include "Neo/NeoPreferences.h"
#include "MenuCmds.h"
#include "Packets.h"
#include "KnownFile.h"
#include "UserMsgs.h"
#include "InputBox.h"
#include "log.h"
#include "Neo/Functions.h"
#include "Neo/VooDoo/Voodoo.h"
#include "Neo/LanCast/LanCast.h"
#include "VoodooDetailDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->

// CVoodooListDlg dialog

#define	PREF_INI_SECTION	_T("VoodooListDlg")

IMPLEMENT_DYNAMIC(CVoodooListDlg, CDialog) 

BEGIN_MESSAGE_MAP(CVoodooListDlg, CModResizableDialog) // NEO: MLD - [ModelesDialogs] 
	ON_NOTIFY(NM_DBLCLK, IDC_VOODOOLIST, OnNMDblclk)
	ON_NOTIFY(NM_CLICK, IDC_VOODOOLIST, OnClick)

	ON_BN_CLICKED(IDC_CONNECT_TO, OnBnClickedConnectTo)
	ON_BN_CLICKED(IDC_CONNECT, OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT, OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_SEARCH, OnBnClickedSearch)
	ON_BN_CLICKED(IDC_ADD, OnBnClickedAdd)
	ON_BN_CLICKED(IDC_EDIT, OnBnClickedEdit)
	ON_BN_CLICKED(IDC_REMOVE, OnBnClickedRemove)

	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

CVoodooListDlg::CVoodooListDlg()
	: CModResizableDialog(CVoodooListDlg::IDD, 0) // NEO: MLD - [ModelesDialogs] 
{
	m_VoodooDetailDialog = NULL; // NEO: MLD - [ModelesDialogs]
	m_timer = 0;
}

CVoodooListDlg::~CVoodooListDlg()
{
	ASSERT(m_VoodooDetailDialog == NULL); // NEO: MLD - [ModelesDialogs]
}

void CVoodooListDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VOODOOLIST, m_VoodooList);
}

BOOL CVoodooListDlg::OnInitDialog()
{
	CModResizableDialog::OnInitDialog(); // NEO: MLD - [ModelesDialogs] 
	InitWindowStyles(this);
	
	m_VoodooList.Init();

	AddAnchor(IDC_VOODOOLIST, TOP_LEFT, BOTTOM_RIGHT);

	AddAnchor(IDC_SSTATIC, TOP_RIGHT, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC4, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC7, TOP_RIGHT);
	AddAnchor(IDC_IPADDRESS, TOP_RIGHT);
	AddAnchor(IDC_SPORT, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC3, TOP_RIGHT);
	AddAnchor(IDC_SNAME, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC5, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC8, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC9, TOP_RIGHT);
	AddAnchor(IDC_STYPE, TOP_RIGHT);
	AddAnchor(IDC_SSOFTWARE, TOP_RIGHT);
	AddAnchor(IDC_SSPELL, TOP_RIGHT);

	AddAnchor(IDC_CONNECT_TO, BOTTOM_RIGHT);
	AddAnchor(IDC_CONNECT, BOTTOM_RIGHT);
	AddAnchor(IDC_DISCONNECT, BOTTOM_RIGHT);
	AddAnchor(IDC_SEARCH, BOTTOM_RIGHT);
	AddAnchor(IDC_ADD, BOTTOM_RIGHT);
	AddAnchor(IDC_EDIT, BOTTOM_RIGHT);
	AddAnchor(IDC_REMOVE, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);

	EnableSaveRestore(PREF_INI_SECTION);

	ReloadVoodooList();

	SetDlgItemText(IDC_SSTATIC,GetResString(IDS_X_VOODOO_EDITOR));
	SetDlgItemText(IDC_SSTATIC4,GetResString(IDS_X_VOODOO_IPADDRESS));
	SetDlgItemText(IDC_SSTATIC7,GetResString(IDS_X_VOODOO_PORT));
	SetDlgItemText(IDC_SSTATIC3,GetResString(IDS_X_VOODOO_NICK));
	SetDlgItemText(IDC_SSTATIC8,GetResString(IDS_X_VOODOO_CLIENT_ACTION));
	SetDlgItemText(IDC_SSTATIC9,GetResString(IDS_X_VOODOO_CLIENT_SOFTWARE));

	CComboBox* ClientAction = ((CComboBox*)GetDlgItem(IDC_STYPE));
	ClientAction->AddString(GetResString(IDS_X_VOODOO_NOTHING)); ASSERT(VA_NONE == 0);
	ClientAction->AddString(GetResString(IDS_X_VOODOO_SLAVE)); ASSERT(VA_SLAVE == 1);
	ClientAction->AddString(GetResString(IDS_X_VOODOO_MASTER)); ASSERT(VA_MASTER == 2);
	ClientAction->AddString(GetResString(IDS_X_VOODOO_PARTNER)); ASSERT(VA_PARTNER == 3);
	ClientAction->AddString(GetResString(IDS_X_VOODOO_BLOCK)); ASSERT(VA_BLOCK == 4);
	if(NeoPrefs.UseVoodooTransfer() && NeoPrefs.IsSlaveAllowed())
		ClientAction->SetCurSel(3);
	else if(NeoPrefs.UseVoodooTransfer())
		ClientAction->SetCurSel(1);
	else if(NeoPrefs.IsSlaveAllowed())
		ClientAction->SetCurSel(2);
	else
		ClientAction->SetCurSel(0);

	CComboBox* Software = ((CComboBox*)GetDlgItem(IDC_SSOFTWARE));
	Software->AddString(GetResString(IDS_X_VOODOO_OTHER)); ASSERT(CT_UNKNOWN == 0);
	Software->AddString(GetResString(IDS_X_VOODOO_ED2K)); ASSERT(CT_ED2K == 1);
	Software->AddString(GetResString(IDS_X_VOODOO_BITTORRENT)); ASSERT(CT_BITTORRENT == 2);
	Software->SetCurSel(0);

	SetDlgItemText(IDC_CONNECT_TO,GetResString(IDS_X_VOODOO_CONNECT_TO));
	SetDlgItemText(IDC_CONNECT,GetResString(IDS_X_VOODOO_CONNECT));
	SetDlgItemText(IDC_DISCONNECT,GetResString(IDS_X_VOODOO_DISCONNECT));
	SetDlgItemText(IDC_SEARCH,GetResString(IDS_X_VOODOO_SEARCH_ORDER));
	SetDlgItemText(IDC_ADD,GetResString(IDS_X_VOODOO_ADD));
	SetDlgItemText(IDC_EDIT,GetResString(IDS_X_VOODOO_EDIT));
	SetDlgItemText(IDC_REMOVE,GetResString(IDS_X_VOODOO_REMOVE));
	SetDlgItemText(IDOK,GetResString(IDS_FD_CLOSE));

	GetDlgItem(IDC_SEARCH)->EnableWindow(NeoPrefs.IsVoodooCastEnabled());

	// start time for calling 'RefreshData'
	VERIFY( (m_timer = SetTimer(301, 1000, 0)) != NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CVoodooListDlg::OnTimer(UINT /*nIDEvent*/)
{
	ReloadVoodooList();
}

void CVoodooListDlg::ReloadVoodooList()
{
	CWaitCursor curWait;
	m_VoodooList.SetRedraw(FALSE);

	POSITION pos = theApp.voodoo->known_list.GetHeadPosition();
	while (pos)
		m_VoodooList.LoadVoodooItem(theApp.voodoo->known_list.GetNext(pos));

	for(int i=0;i<m_VoodooList.GetItemCount();i++){
		CVoodooClient* item = (CVoodooClient*)m_VoodooList.GetItemData(i);
		if(theApp.voodoo->known_list.Find(item) == NULL)
			m_VoodooList.DeleteItem(i);
	}

	m_VoodooList.SetRedraw();
}

void CVoodooListDlg::OnClick(NMHDR* /*pNMHDR*/, LRESULT *pResult) {
	
	POSITION pos = m_VoodooList.GetFirstSelectedItemPosition();
	if(pos != NULL)
	{
		int index = m_VoodooList.GetNextSelectedItem(pos);
		if (index >= 0){
			CVoodooClient* item = (CVoodooClient*)m_VoodooList.GetItemData(index);
			if(!theApp.voodoo->IsValidKnownClinet(item)){
				ReloadVoodooList();
				return;
			}

			GetDlgItem(IDC_IPADDRESS)->SetWindowText(item->GetAddress());
			GetDlgItem(IDC_SPORT)->SetWindowText(StrLine(_T("%u"),item->GetPort()));
			GetDlgItem(IDC_SSPELL)->SetWindowText(item->GetSpell());
			GetDlgItem(IDC_SNAME)->SetWindowText(item->GetName());
			((CComboBox*)GetDlgItem(IDC_STYPE))->SetCurSel(item->GetAction());
			((CComboBox*)GetDlgItem(IDC_SSOFTWARE))->SetCurSel(item->GetType());

			GetDlgItem(IDC_CONNECT)->EnableWindow(item->GetPort() != 0);
			GetDlgItem(IDC_DISCONNECT)->EnableWindow(item->socket != NULL);
		}
	}

	*pResult = 0;
}


void CVoodooListDlg::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT *pResult) {
	
	POSITION pos = m_VoodooList.GetFirstSelectedItemPosition();
	if(pos != NULL)
	{
		int index = m_VoodooList.GetNextSelectedItem(pos);
		if (index >= 0){
			CVoodooClient* item = (CVoodooClient*)m_VoodooList.GetItemData(index);
			if(item->socket == NULL)
			{
				if(!theApp.voodoo->IsValidKnownClinet(item)){
					ReloadVoodooList();
					return;
				}
				theApp.voodoo->ConnectVoodooClient(item->GetAddress(),item->GetPort(),(item->GetAction() == VA_NONE) ? VA_QUERY : item->GetAction());
				m_VoodooList.Update(index);
			}
			else
			{
				// NEO: MLD - [ModelesDialogs]
				if(m_VoodooDetailDialog)
					m_VoodooDetailDialog->DropControl();
				m_VoodooDetailDialog = new CVoodooDetailDialog(item, &m_VoodooList);
				m_VoodooDetailDialog->OpenDialog(FALSE);
				// NEO: MLD END
				//CVoodooDetailDialog dialog(item, &m_VoodooList);
				//dialog.DoModal();
			}
		}
	}

	*pResult = 0;
}

void CVoodooListDlg::OnBnClickedConnectTo(){

	CString Address;
	GetDlgItem(IDC_IPADDRESS)->GetWindowText(Address);

	CString Port;
	GetDlgItem(IDC_SPORT)->GetWindowText(Port);

	uint16 nPort = (uint16)_tstoi(Port);
	if(nPort == 0)
		nPort = NeoPrefs.GetVoodooPort();

	uint8 uAction = (uint8)((CComboBox*)GetDlgItem(IDC_STYPE))->GetCurSel();
	if(uAction == VA_NONE)
		uAction = VA_QUERY;

	theApp.voodoo->ConnectVoodooClient(Address,nPort,uAction);
}

void CVoodooListDlg::OnBnClickedConnect(){
	POSITION pos = m_VoodooList.GetFirstSelectedItemPosition();
	while(pos != NULL)
	{
		int index = m_VoodooList.GetNextSelectedItem(pos);
		if (index >= 0){
			CVoodooClient* item = (CVoodooClient*)m_VoodooList.GetItemData(index);
			if(!theApp.voodoo->IsValidKnownClinet(item)){
				ReloadVoodooList();
				return;
			}
			theApp.voodoo->ConnectVoodooClient(item->GetAddress(),item->GetPort(),(item->GetAction() == VA_NONE) ? VA_QUERY : item->GetAction());
			m_VoodooList.Update(index);
		}
	}
}

void CVoodooListDlg::OnBnClickedDisconnect(){
	POSITION pos = m_VoodooList.GetFirstSelectedItemPosition();
	while(pos != NULL)
	{
		int index = m_VoodooList.GetNextSelectedItem(pos);
		if (index >= 0){
			CVoodooClient* item = (CVoodooClient*)m_VoodooList.GetItemData(index);
			if(!theApp.voodoo->IsValidKnownClinet(item)){
				ReloadVoodooList();
				return;
			}

			if(item->socket)
				item->socket->SendGoodBy();
			m_VoodooList.Update(index);
		}
	}
}

void CVoodooListDlg::OnBnClickedSearch(){
	if(NeoPrefs.IsVoodooCastEnabled()){
		ModLog(GetResString(IDS_X_VOODOO_SEARCH));
		theApp.lancast->SendVoodoo(VA_QUERY);
	}
}

void CVoodooListDlg::OnBnClickedAdd(){

	CString Address;
	GetDlgItem(IDC_IPADDRESS)->GetWindowText(Address);

	DWORD clientIP = inet_addr(CT2CA(Address));
	if(clientIP == INADDR_NONE)
		clientIP = 0;

	CString Port;
	GetDlgItem(IDC_SPORT)->GetWindowText(Port);

	uint16 nPort = (uint16)_tstoi(Port);
	if(nPort == 0)
		nPort = NeoPrefs.GetVoodooPort();

	if (theApp.voodoo->GetClientByAddress(Address, nPort) != NULL
	 || theApp.voodoo->GetClientByIP(clientIP, nPort) != NULL){
		MessageBox(GetResString(IDS_X_VOODOO_ADD_BOX_DENIDED),GetResString(IDS_X_VOODOO_ADD),MB_ICONEXCLAMATION);
		return;
	}

	 CString StrBuff;
	CVoodooClient* new_client = new CVoodooClient;
	new_client->SetAddress(Address);
	new_client->SetIP(clientIP);
	new_client->SetPort(nPort);
	GetDlgItem(IDC_SSPELL)->GetWindowText(StrBuff);
	new_client->SetSpell(StrBuff);
	GetDlgItem(IDC_SNAME)->GetWindowText(StrBuff);
	new_client->SetName(StrBuff);
	new_client->SetAction((uint8)((CComboBox*)GetDlgItem(IDC_STYPE))->GetCurSel());
	new_client->SetType((uint8)((CComboBox*)GetDlgItem(IDC_SSOFTWARE))->GetCurSel());

	theApp.voodoo->known_list.AddTail(new_client);
	m_VoodooList.LoadVoodooItem(new_client);
	theApp.voodoo->SaveKnownToFile();

	if(MessageBox(GetResString(IDS_X_VOODOO_ADD_BOX_SUCCES),GetResString(IDS_X_VOODOO_ADD),MB_YESNO | MB_ICONQUESTION) == IDYES)
		theApp.voodoo->ConnectVoodooClient(new_client->GetAddress(),new_client->GetPort(),VA_QUERY);
}

void CVoodooListDlg::OnBnClickedEdit(){
	POSITION pos = m_VoodooList.GetFirstSelectedItemPosition();
	if(pos != NULL)
	{
		int index = m_VoodooList.GetNextSelectedItem(pos);
		if (index >= 0){
			CVoodooClient* item = (CVoodooClient*)m_VoodooList.GetItemData(index);
			if(!theApp.voodoo->IsValidKnownClinet(item)){
				ReloadVoodooList();
				return;
			}

			//if(theApp.voodoo->GetVoodooSocket(item->GetIP(),item->GetPort(),item->socketPort) != NULL){
			//	MessageBox(GetResString(IDS_X_VOODOO_EDIT_BOX_DENIDED),GetResString(IDS_X_VOODOO_EDIT),MB_ICONEXCLAMATION);
			//	return;
			//}

			CString Address;
			GetDlgItem(IDC_IPADDRESS)->GetWindowText(Address);

			DWORD clientIP = inet_addr(CT2CA(Address));
			if(clientIP == INADDR_NONE)
				clientIP = 0;

			CString Port;
			GetDlgItem(IDC_SPORT)->GetWindowText(Port);

			uint16 nPort = (uint16)_tstoi(Port);
			if(nPort == 0)
				nPort = NeoPrefs.GetVoodooPort();

			CVoodooClient* Found;
			if (((Found = theApp.voodoo->GetClientByAddress(Address, nPort)) != NULL && Found != item)
			|| ((Found = theApp.voodoo->GetClientByIP(clientIP, nPort)) != NULL && Found != item)){
				MessageBox(GetResString(IDS_X_VOODOO_ADD_BOX_DENIDED),GetResString(IDS_X_VOODOO_ADD),MB_ICONEXCLAMATION);
				return;
			}

			CString StrBuff;
			item->SetAddress(Address);
			item->SetIP(clientIP);
			item->SetPort(nPort);
			GetDlgItem(IDC_SSPELL)->GetWindowText(StrBuff);
			item->SetSpell(StrBuff);
			GetDlgItem(IDC_SNAME)->GetWindowText(StrBuff);
			item->SetName(StrBuff);
			item->SetAction((uint8)((CComboBox*)GetDlgItem(IDC_STYPE))->GetCurSel());
			item->SetType((uint8)((CComboBox*)GetDlgItem(IDC_SSOFTWARE))->GetCurSel());

			m_VoodooList.LoadVoodooItem(item);
			theApp.voodoo->SaveKnownToFile();
		}
	}
}

void CVoodooListDlg::OnBnClickedRemove(){

	if (m_VoodooList.GetSelectedCount() == 0 
	|| IDNO == AfxMessageBox(GetResString(IDS_X_CONFIRM_VOODOODELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
		return;
	
	CTypedPtrList<CPtrList, CVoodooClient*> selectedList;
	POSITION pos = m_VoodooList.GetFirstSelectedItemPosition();
	while(pos != NULL)
	{
		int index = m_VoodooList.GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CVoodooClient*)m_VoodooList.GetItemData(index));
	}

	int iSkiped = 0;
	while (!selectedList.IsEmpty())
	{
		CVoodooClient* item = selectedList.RemoveHead();

		if(item->socket != NULL){
			iSkiped ++;
			continue;
		}

		POSITION toremove = theApp.voodoo->known_list.Find(item);
		if(toremove){
			theApp.voodoo->known_list.RemoveAt(toremove);
			theApp.voodoo->SaveKnownToFile();
			delete item;
		}

		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)item;
		sint32 result = m_VoodooList.FindItem(&find);
		if (result != (-1) )
			m_VoodooList.DeleteItem(result);
	}
	if(iSkiped)
		AfxMessageBox(StrLine(GetResString(IDS_X_VOODOODELETE_SKIPPED),iSkiped),MB_ICONWARNING | MB_OK);
}

void CVoodooListDlg::OnDestroy()
{
	// NEO: MLD - [ModelesDialogs]
	if(m_VoodooDetailDialog)
		m_VoodooDetailDialog->DropControl();
	m_VoodooDetailDialog = NULL;
	// NEO: MLD END

	if (m_timer){
		KillTimer(m_timer);
		m_timer = 0;
	}

	CResizableDialog::OnDestroy();
}

#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
