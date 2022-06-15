//this file is part of eMule
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
#include "VoodooDetailDialog.h"
#include "PartFile.h"
#include "otherfunctions.h"
#include "SharedFileList.h"
#include "HighColorTab.hpp"
#include "UserMsgs.h"
#include "ListenSocket.h"
#include "preferences.h"
#include "downloadqueue.h"
#include "knownfilelist.h"
#include "Neo/VooDoo/Voodoo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->

///////////////////////////////////////////////////////////////////////////////
// CVoodooDetailPage

IMPLEMENT_DYNAMIC(CVoodooDetailPage, CResizablePage)

BEGIN_MESSAGE_MAP(CVoodooDetailPage, CResizablePage)
	ON_NOTIFY(NM_DBLCLK, IDC_VOODOOERR, OnNMDblclk)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CVoodooDetailPage::CVoodooDetailPage()
	: CResizablePage(CVoodooDetailPage::IDD, 0 )
{
	m_paSockets		= NULL;
	m_bDataChanged	= false;
	m_strCaption	= GetResString(IDS_X_VD_TITLE);
	m_psp.pszTitle	= m_strCaption;
	m_psp.dwFlags  |= PSP_USETITLE;
}

CVoodooDetailPage::~CVoodooDetailPage()
{
}

void CVoodooDetailPage::DoDataExchange(CDataExchange* pDX)
{
	CResizablePage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VOODOOERR, m_VoodooErr);
}

BOOL CVoodooDetailPage::OnInitDialog()
{
	CResizablePage::OnInitDialog();
	InitWindowStyles(this);

	m_VoodooErr.SetName(_T("VoodoErrListCtrl"));
	m_VoodooErr.ModifyStyle(LVS_SINGLESEL,0);

	m_VoodooErr.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	m_VoodooErr.InsertColumn(0, GetResString(IDS_X_VD_LSTERR_NAME), LVCFMT_LEFT, 130); 
	m_VoodooErr.InsertColumn(0, GetResString(IDS_X_VD_LSTERR_HASH), LVCFMT_LEFT, 130); 
	m_VoodooErr.InsertColumn(0, GetResString(IDS_X_VD_LSTERR_ERROR), LVCFMT_LEFT, 130); 

	m_VoodooErr.LoadSettings();

	AddAnchor(IDC_STATIC30, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC40, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC50, TOP_LEFT, BOTTOM_RIGHT);

	Localize();
	return TRUE;
}

BOOL CVoodooDetailPage::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;

	if (m_bDataChanged)
	{
		CVoodooClient* client = STATIC_DOWNCAST(CVoodooClient, (*m_paSockets)[0]);

		CString strBuff;

		strBuff = client->GetName();
		GetDlgItem(IDC_DNAME)->SetWindowText( strBuff  );

		strBuff = client->GetAddress();
		GetDlgItem(IDC_DHOST)->SetWindowText( strBuff  );

		if(client->GetPort() && client->GetIP())
			strBuff = ipstr(client->GetIP(),client->GetPort());
		else if(client->GetPort())
			strBuff.AppendFormat(_T("%u"),client->GetPort());
		else
			strBuff.Empty();
		GetDlgItem(IDC_DIP)->SetWindowText( strBuff  );

		strBuff = GetResString((client->GetAction() == VA_PARTNER) ? IDS_X_VOODOO_PARTNER : (client->GetAction() == VA_MASTER) ? IDS_X_VOODOO_MASTER : (client->GetAction() == VA_SLAVE) ? IDS_X_VOODOO_SLAVE : IDS_X_VOODOO_NOTHING);
		GetDlgItem(IDC_DACTION)->SetWindowText( strBuff  );

		strBuff = GetResString((client->GetType() == CT_ED2K) ? IDS_X_VOODOO_ED2K : (client->GetType() == CT_BITTORRENT) ? IDS_X_VOODOO_BITTORRENT : IDS_X_VOODOO_OTHER);
		GetDlgItem(IDC_DTYPE)->SetWindowText( strBuff  );

		strBuff = GetResString((client->GetPerm() == VA_PARTNER) ? IDS_X_VOODOO_PARTNER : (client->GetPerm() == VA_MASTER) ? IDS_X_VOODOO_MASTER : (client->GetPerm() == VA_SLAVE) ? IDS_X_VOODOO_SLAVE : IDS_X_VOODOO_NOTHING);
		GetDlgItem(IDC_DUSABLE)->SetWindowText( strBuff  );

		if(client->socket){
			if(client->socket->IsMaster() && client->socket->IsSlave())
				strBuff = GetResString(IDS_X_VOODOO_PARTNER);
			else if(client->socket->IsMaster())
				strBuff = GetResString(IDS_X_VOODOO_MASTER);
			else if(client->socket->IsSlave())
				strBuff = GetResString(IDS_X_VOODOO_SLAVE);
			else
				strBuff = GetResString(IDS_X_VOODOO_NOTHING);
		}else
			strBuff.Empty();
		GetDlgItem(IDC_DSTATUS)->SetWindowText( strBuff  );

		if(client->socket){
			strBuff.Format(_T("%u"),client->socket->GetVoodooVersion());
			GetDlgItem(IDC_VD_VER)->SetWindowText( strBuff );
			strBuff.Format(_T("%u"),client->socket->Use64Size());
			GetDlgItem(IDC_VD_LF)->SetWindowText( strBuff  );
			strBuff.Format(_T("%u"),client->socket->GetCorruptionHandlingVersion());
			GetDlgItem(IDC_VD_CH)->SetWindowText( strBuff  );
			strBuff.Format(_T("%u"),client->socket->GetAdvDownloadSyncVersion());
			GetDlgItem(IDC_VD_TS)->SetWindowText( strBuff  );
			strBuff.Format(_T("%u"),client->socket->GetSupportsStatisticsVersion());
			GetDlgItem(IDC_VD_ST)->SetWindowText( strBuff  );
			strBuff.Format(_T("%u"),(uint8)client->socket->GetVoodooSearchVersion());
			GetDlgItem(IDC_VD_VS)->SetWindowText( strBuff  );

			strBuff.Format(_T("%u"),(uint8)client->socket->GetVoodooXSVersion());
			GetDlgItem(IDC_VD_NXS)->SetWindowText( strBuff  );
			strBuff.Format(_T("%u"),(uint8)client->socket->GetNeoFilePrefsVersion());
			GetDlgItem(IDC_VD_NFP)->SetWindowText( strBuff  );
			strBuff.Format(_T("%u"),(uint8)client->socket->GetNeoCommandVersion());
			GetDlgItem(IDC_VD_NDC)->SetWindowText( strBuff  );
		}else{
			GetDlgItem(IDC_VD_VER)->SetWindowText( _T("") );
			GetDlgItem(IDC_VD_LF)->SetWindowText( _T("")  );
			GetDlgItem(IDC_VD_CH)->SetWindowText( _T("")  );
			GetDlgItem(IDC_VD_TS)->SetWindowText( _T("")  );
			GetDlgItem(IDC_VD_ST)->SetWindowText( _T("")  );
			GetDlgItem(IDC_VD_VS)->SetWindowText( _T("")  );

			GetDlgItem(IDC_VD_NXS)->SetWindowText( _T("")  );
			GetDlgItem(IDC_VD_NFP)->SetWindowText( _T("")  );
			GetDlgItem(IDC_VD_NDC)->SetWindowText( _T("")  );
		}

		m_VoodooErr.DeleteAllItems();
		if(client->socket){

			sFileError* FileErr;
			CCKey bufKey;
			for (POSITION pos = client->socket->m_FileErrors.GetStartPosition();pos != 0;){
				client->socket->m_FileErrors.GetNextAssoc(pos,bufKey,FileErr);
				
				CKnownFile* kFile = theApp.downloadqueue->GetFileByID(FileErr->ID.Hash);
				if(!kFile)
					kFile = theApp.knownfiles->FindKnownFileByID(FileErr->ID.Hash);

				strBuff = kFile ? md4str(kFile->GetFileHash()) : GetResString(IDS_X_VD_UNKNOWN_NAME);
				
				LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)FileErr;
				int iItem = m_VoodooErr.FindItem(&find);
				if (iItem == -1)
					iItem = m_VoodooErr.InsertItem(LVIF_TEXT|LVIF_PARAM,0,strBuff,0,0,1,(LPARAM)FileErr);

				m_VoodooErr.SetItemText(iItem, 0, strBuff);

				strBuff.Format(_T("%s"), md4str(FileErr->ID.Hash));
				strBuff.AppendFormat(_T("%I64u"),FileErr->ID.Size);
				m_VoodooErr.SetItemText(iItem, 1, strBuff);

				switch(FileErr->Error){
				case RET_NONE: strBuff.Empty(); break;
				case RET_REAL: strBuff = GetResString(IDS_X_VD_RET_REAL); break;
				case RET_NEW: strBuff = GetResString(IDS_X_VD_RET_NEW); break;
				//case RET_UNAVALIBLY: strBuff = GetResString(IDS_X_VD_RET_UNAVALIBLY); break;
				case RET_UNKNOWN: strBuff = GetResString(IDS_X_VD_RET_UNKNOWN); break;
				case RET_BADSIZE: strBuff = GetResString(IDS_X_VD_RET_BADSIZE); break;
				case RET_EXIST: strBuff = GetResString(IDS_X_VD_RET_EXIST); break;
				case RET_COMPLETE: strBuff = GetResString(IDS_X_VD_RET_COMPLETE); break;
				default: strBuff = GetResString(IDS_X_VD_UNKNOWN_RET);
				}
				m_VoodooErr.SetItemText(iItem, 2, strBuff);

			}
		}
		
		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CVoodooDetailPage::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CVoodooDetailPage::Localize()
{
	GetDlgItem(IDC_STATIC30)->SetWindowText(GetResString(IDS_X_VD_GENERAL));
	
	GetDlgItem(IDC_STATIC31)->SetWindowText(GetResString(IDS_X_VD_NAME));
	GetDlgItem(IDC_STATIC32)->SetWindowText(GetResString(IDS_X_VD_ADRESS));
	GetDlgItem(IDC_STATIC33)->SetWindowText(GetResString(IDS_X_VD_ACTION));
	GetDlgItem(IDC_STATIC34)->SetWindowText(GetResString(IDS_X_VD_TYPE));
	GetDlgItem(IDC_STATIC36)->SetWindowText(GetResString(IDS_X_VD_STATUS));
	GetDlgItem(IDC_STATIC37)->SetWindowText(GetResString(IDS_X_VD_USABLE));

	GetDlgItem(IDC_STATIC40)->SetWindowText(GetResString(IDS_X_VD_SOCKET));

	GetDlgItem(IDC_STATIC48)->SetWindowText(GetResString(IDS_X_VD_VERSION));
	GetDlgItem(IDC_STATIC42)->SetWindowText(GetResString(IDS_X_VD_LARGEFILE));
	GetDlgItem(IDC_STATIC45)->SetWindowText(GetResString(IDS_X_VD_CORRUPTION_HANDLING));
	GetDlgItem(IDC_STATIC43)->SetWindowText(GetResString(IDS_X_VD_TRANSFER_SYNCHRONSATION));
	GetDlgItem(IDC_STATIC46)->SetWindowText(GetResString(IDS_X_VD_STATISTICS));
	GetDlgItem(IDC_STATIC44)->SetWindowText(GetResString(IDS_X_VD_VOODOO_SEARCH));

	GetDlgItem(IDC_STATIC53)->SetWindowText(GetResString(IDS_X_VD_SRC_EXC));
	GetDlgItem(IDC_STATIC55)->SetWindowText(GetResString(IDS_X_VD_FILE_PREFS));
	GetDlgItem(IDC_STATIC54)->SetWindowText(GetResString(IDS_X_VD_DL_CMDS));

	GetDlgItem(IDC_STATIC50)->SetWindowText(GetResString(IDS_X_VD_ERRORS));

}

void CVoodooDetailPage::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT *pResult) {
	
	POSITION pos = m_VoodooErr.GetFirstSelectedItemPosition();
	if(pos != NULL)
	{
		int index = m_VoodooErr.GetNextSelectedItem(pos);
		if (index >= 0){
			CVoodooClient* client = STATIC_DOWNCAST(CVoodooClient, (*m_paSockets)[0]);
			if(!theApp.voodoo->IsValidKnownClinet(client))
				return;

			if(client->socket)
			{
				sFileError* item = (sFileError*)m_VoodooErr.GetItemData(index);
				sFileError* FileErr;
				CCKey bufKey;
				for (POSITION pos = client->socket->m_FileErrors.GetStartPosition();pos != 0;){
					client->socket->m_FileErrors.GetNextAssoc(pos,bufKey,FileErr);
					if(FileErr == item){
						client->socket->m_FileErrors.RemoveKey(bufKey);
						CPartFile* pFile = theApp.downloadqueue->GetFileByID(FileErr->ID.Hash);
						CKnownFile* kFile = theApp.sharedfiles->GetFileByID(FileErr->ID.Hash);
						delete FileErr;

						if(pFile)
						{
							uint8 state;
							if(pFile->IsStopped())
								state = INST_STOP;
							else if(pFile->IsPaused())
								state = INST_PAUSE;
							else
								state = INST_RESUME;

							client->socket->SendDownloadInstruction(pFile,state);
						}
						else if(kFile)
						{
							client->socket->SendShareInstruction(kFile,INST_SHARE);
						}
						
						break;
					}
				}

				m_VoodooErr.DeleteItem(index);
			}
		}
	}

	*pResult = 0;
}

///////////////////////////////////////////////////////////////////////////////
// CVoodooDetailDialog

IMPLEMENT_DYNAMIC(CVoodooDetailDialog, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CVoodooDetailDialog, CModListViewWalkerPropertySheet) // NEO: MLD - [ModelesDialogs]
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CVoodooDetailDialog::CVoodooDetailDialog(CVoodooClient* pSocket, CListCtrlItemWalk* pListCtrl)
	: CModListViewWalkerPropertySheet(pListCtrl) // NEO: MLD - [ModelesDialogs]
{
	m_aItems.Add(pSocket);
	Construct();
}

CVoodooDetailDialog::CVoodooDetailDialog(const CSimpleArray<CVoodooClient*>* paSockets, CListCtrlItemWalk* pListCtrl)
	: CModListViewWalkerPropertySheet(pListCtrl) // NEO: MLD - [ModelesDialogs]
{
	for (int i = 0; i < paSockets->GetSize(); i++)
		m_aItems.Add((*paSockets)[i]);
	Construct();
}

void CVoodooDetailDialog::Construct()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	m_wndVoodoo.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndVoodoo.m_psp.dwFlags |= PSP_USEICONID;
	m_wndVoodoo.m_psp.pszIcon = _T("VOODOODETAILS");
	m_wndVoodoo.SetSockets(&m_aItems);
	AddPage(&m_wndVoodoo);
}

CVoodooDetailDialog::~CVoodooDetailDialog()
{
}

void CVoodooDetailDialog::OnDestroy()
{
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CVoodooDetailDialog::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult = CModListViewWalkerPropertySheet::OnInitDialog(); // NEO: MLD - [ModelesDialogs]
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("VoodooDetailDialog")); // call this after(!) OnInitDialog
	SetWindowText(GetResString(IDS_CD_TITLE));
	return bResult;
}

#endif // VOODOO // NEO: VOODOO END <-- Xanatos --