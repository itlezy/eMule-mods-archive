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
#include "PPgInterface.h"
#include "Neo/NeoPreferences.h"
#include "OtherFunctions.h"
#include "Neo/GUI/CP/TreeFunctions.h"
#include "emuledlg.h"
#include "MuleToolbarCtrl.h"
#include "Neo/Functions.h"
#include "downloadqueue.h" // NEO: NTB - [NeoToolbarButtons]
#include "Neo/Defaults.h"
#include "TransferWnd.h"
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] 
#include "Neo/BC/BandwidthControl.h"
#endif // NEO_BC // NEO: NBC END
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
#include "Neo\GUI\IP2Country.h"
#include "transferwnd.h"
#include "serverwnd.h"
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
#include "TransferWnd.h"
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "searchdlg.h"
#include "searchresultswnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// NEO: NTB - [NeoToolbarButtons]

class CNeoToolbarButton : public CTreeOptionsCombo
{
public:
	CNeoToolbarButton();
	virtual ~CNeoToolbarButton();

protected:
	//{{AFX_VIRTUAL(CNeoToolbarButton)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CNeoToolbarButton)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

  DECLARE_DYNCREATE(CNeoToolbarButton)
};


IMPLEMENT_DYNCREATE(CNeoToolbarButton, CTreeOptionsCombo)

CNeoToolbarButton::CNeoToolbarButton()
{
}

CNeoToolbarButton::~CNeoToolbarButton()
{
}

BEGIN_MESSAGE_MAP(CNeoToolbarButton, CTreeOptionsCombo)
	//{{AFX_MSG_MAP(CNeoToolbarButton)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CString GetNeoCmdWStr(uint8 uNeoCmdL, uint8 uNeoCmdW);
int CNeoToolbarButton::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeOptionsCombo::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	switch((uint8)(m_pTreeCtrl->GetOpaque(m_hTreeCtrlItem,TRUE)))
	{
		case 0:
			AddString(GetResString(IDS_X_COLLECT_ALL_SOURCES));
			AddString(GetResString(IDS_X_COLLECT_XS_SOURCES));
			AddString(GetResString(IDS_X_COLLECT_SVR_SOURCES));
			AddString(GetResString(IDS_X_COLLECT_KAD_SOURCES));
			AddString(GetResString(IDS_X_COLLECT_UDP_SOURCES));
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange]
			AddString(GetResString(IDS_X_COLLECT_VOODOO_SOURCES));
#endif // VOODOO // NEO: VOODOOx END
			AddString(GetResString(IDS_X_AHL_INCREASE));
			AddString(GetResString(IDS_X_AHL_DECREASE));
			break;
		case 1:
			AddString(GetResString(IDS_X_DROP_NNP));
			AddString(GetResString(IDS_X_DROP_FULLQ));
			AddString(GetResString(IDS_X_DROP_HIGHQ));
				// NEO: TCR - [TCPConnectionRetry]
			AddString(GetResString(IDS_X_DROP_WAITINGRETRY));
				// NEO: TCR END
				// NEO: XSC - [ExtremeSourceCache]
			AddString(GetResString(IDS_X_DROP_CACHED));
				// NEO: XSC END
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
			AddString(GetResString(IDS_X_DROP_LOADED));
#endif // NEO_SS // NEO: NSS END
			AddString(GetResString(IDS_X_DROP_TOMANY));
			AddString(GetResString(IDS_X_DROP_LOW2LOW));
			break;
		case 2:
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
			AddString(GetResString(IDS_X_LOADSRCMANUALLY));
			AddString(GetResString(IDS_X_SAVESRCMANUALLY));
#endif // NEO_SS // NEO: NSS END
#ifdef NEO_CD // NEO: SFL - [SourceFileList]
			AddString(GetResString(IDS_X_FINDSOURCES));
#endif // NEO_CD // NEO: SFL END
			break;
		case 3:
			// NEO: PP - [PasswordProtection]
			AddString(GetResString(IDS_X_PWPROT_SHOW));
			AddString(GetResString(IDS_X_PWPROT_HIDE));
			AddString(GetResString(IDS_X_PWPROT_SET));
			AddString(GetResString(IDS_X_PWPROT_CHANGE));
			AddString(GetResString(IDS_X_PWPROT_UNSET));
			// NEO: PP END
			// NEO: FCFG - [FileConfiguration]
			AddString(GetResString(IDS_DL_INFO));
			AddString(GetResString(IDS_X_FILE_TWEAKS));
			// NEO: FCFG END
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			AddString(GetResString(IDS_X_VOODOO_LIST));
#endif // VOODOO // NEO: VOODOO END
			AddString(GetResString(IDS_X_SEP));
			break;
		default:
			ASSERT(0);
	}

	return 0;
}

CString GetNeoCmdLStr(uint8 uNeoCmdL);
void SetTreeNeoToolbarButton(CTreeOptionsCtrl &hCtrl, tNeoButton* hItem, HTREEITEM Parent, int i, int *iImgNeoBtn)
{
	SetTreeGroup(hCtrl,hItem->m_htiButton,StrLine(GetResString(IDS_X_NEO_BTN),i),iImgNeoBtn[i%8],Parent,_T(""));
	for (int j=0; j<ARRSIZE(hItem->m_htiGroups); j++)
	{
		uint8 uNeoCmdL = 0;
		switch(j)
		{
			case 0: uNeoCmdL = INST_COLLECT; break;
			case 1: uNeoCmdL = INST_DROP; break;
			case 2: uNeoCmdL = INST_STORAGE; break;
			case 3: uNeoCmdL = INST_OTHER; break;
			default:
				ASSERT(0);
		}
		SetTreeRadio(hCtrl,hItem->m_htiGroups[j],GetNeoCmdLStr(uNeoCmdL),hItem->m_htiButton,_T(""),TRUE,FALSE);
		hCtrl.AddComboBox(hItem->m_htiGroups[j],RUNTIME_CLASS(CNeoToolbarButton),0,(DWORD)j);
		//hCtrl.SetItemSize(hCtrl,hItem->m_htiGroups[j],10);
	}
}

void DeleteTreeNeoToolbarButton(CTreeOptionsCtrl &hCtrl, tNeoButton* hItem){
	for (int j=0; j<ARRSIZE(hItem->m_htiGroups); j++){
		CTreeOptionsItemData* pItem = (CTreeOptionsItemData*)hCtrl.GetItemData(hItem->m_htiGroups[j]);
		if (pItem) 
			delete pItem; 
		hCtrl.SetItemData(hItem->m_htiGroups[j], 0);  

		hCtrl.DeleteItem(hItem->m_htiGroups[j]);
	}

	CTreeOptionsItemData* pItem = (CTreeOptionsItemData*)hCtrl.GetItemData(hItem->m_htiButton);
	if (pItem) 
		delete pItem; 
	hCtrl.SetItemData(hItem->m_htiButton, 0);  

	hCtrl.DeleteItem(hItem->m_htiButton);
}

void DDX_TreeNeoToolbarButton(CTreeOptionsCtrl* hCtrl, CDataExchange* pDX, int /*nIDC*/, tNeoButton* hItem, UINT& nValue)
{
	if (pDX->m_bSaveAndValidate){
		int iNeoCmdL = 0;
		DDX_TreeRadio(pDX, IDC_MOD_OPTS, hItem->m_htiButton, iNeoCmdL);
		CString SNeoCmdW;
		if(iNeoCmdL != -1)
			SNeoCmdW = hCtrl->GetComboText(hItem->m_htiGroups[iNeoCmdL]);
		if(SNeoCmdW.IsEmpty())
			iNeoCmdL = -1;

		uint8 uNeoCmdL = 0;
		uint8 uNeoCmdW = 0;
		switch(iNeoCmdL){
			case -1: 
				uNeoCmdL = 0; 
				uNeoCmdW = 0;
				break;
			case 0: uNeoCmdL = INST_COLLECT; 
				for(uint8 j = 0xA1; j <= 0xAB; j++){
					if(SNeoCmdW == GetNeoCmdWStr(uNeoCmdL,j)){
						uNeoCmdW = j;
						break;
					}
				}
				break;
			case 1: uNeoCmdL = INST_DROP; 
				for(uint8 j = 0xB1; j <= 0xBF; j++){
					if(SNeoCmdW == GetNeoCmdWStr(uNeoCmdL,j)){
						uNeoCmdW = j;
						break;
					}
				}
				break;
			case 2: uNeoCmdL = INST_STORAGE; 
				for(uint8 j = 0xD1; j <= 0xD5; j++){
					if(SNeoCmdW == GetNeoCmdWStr(uNeoCmdL,j)){
						uNeoCmdW = j;
						break;
					}
				}
				break;
			case 3: uNeoCmdL = INST_OTHER; 
				for(uint8 j = 0xE1; j <= 0xE8; j++){
					if(SNeoCmdW == GetNeoCmdWStr(uNeoCmdL,j)){
						uNeoCmdW = j;
						break;
					}
				}
				if(SNeoCmdW == GetNeoCmdWStr(uNeoCmdL,INST_SEP))
					uNeoCmdW = INST_SEP;
				break;
			default:
				ASSERT(0);
		}

		if(uNeoCmdL != 0 && uNeoCmdW != 0)
		{
			nValue = uNeoCmdL;
			nValue |= uNeoCmdW << 16;
		}else
			nValue = 0;
	}
	else
	{
		uint8 uNeoCmdL = (uint8)(nValue);
		uint8 uNeoCmdW = (uint8)(nValue >> 16);

		int iNeoCmdL = -1;
		switch(uNeoCmdL)
		{
			case 0: iNeoCmdL = -1; break;
			case INST_COLLECT: iNeoCmdL = 0; break;
			case INST_DROP: iNeoCmdL = 1; break;
			case INST_STORAGE: iNeoCmdL = 2; break;
			case INST_OTHER: iNeoCmdL = 3; break;
			default:
				ASSERT(0);
		}
		DDX_TreeRadio(pDX, IDC_MOD_OPTS, hItem->m_htiButton, iNeoCmdL);

		CString SNeoCmdW = _T("");
		for(uint8 j=0; j<ARRSIZE(hItem->m_htiGroups); j++)
			hCtrl->SetComboText(hItem->m_htiGroups[j], SNeoCmdW);
		SNeoCmdW = GetNeoCmdWStr(uNeoCmdL,uNeoCmdW); 
		if(iNeoCmdL != -1)
			hCtrl->SetComboText(hItem->m_htiGroups[iNeoCmdL], SNeoCmdW);
	}
}
// NEO: NTB END

// NEO: IM - [InvisibelMode]
class CInvisibelModeHotKeyModifier : public CTreeOptionsCombo
{
public:
	CInvisibelModeHotKeyModifier();
	virtual ~CInvisibelModeHotKeyModifier();

protected:
	//{{AFX_VIRTUAL(CInvisibelModeHotKeyModifier)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CInvisibelModeHotKeyModifier)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

  DECLARE_DYNCREATE(CInvisibelModeHotKeyModifier)
};


IMPLEMENT_DYNCREATE(CInvisibelModeHotKeyModifier, CTreeOptionsCombo)

CInvisibelModeHotKeyModifier::CInvisibelModeHotKeyModifier()
{
}

CInvisibelModeHotKeyModifier::~CInvisibelModeHotKeyModifier()
{
}

BEGIN_MESSAGE_MAP(CInvisibelModeHotKeyModifier, CTreeOptionsCombo)
	//{{AFX_MSG_MAP(CInvisibelModeHotKeyModifier)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CInvisibelModeHotKeyModifier::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeOptionsCombo::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  //Add strings to the combo
	AddString(GetResString(IDS_X_CTRLKEY));
	AddString(GetResString(IDS_X_ALTKEY));
	AddString(GetResString(IDS_X_SHIFTKEY));
	AddString(GetResString(IDS_X_CTRLKEY) + _T(" + ") + GetResString(IDS_X_ALTKEY));
	AddString(GetResString(IDS_X_CTRLKEY) + _T(" + ") + GetResString(IDS_X_SHIFTKEY));
	AddString(GetResString(IDS_X_ALTKEY) + _T(" + ") + GetResString(IDS_X_SHIFTKEY));
	AddString(GetResString(IDS_X_CTRLKEY) + _T(" + ") + GetResString(IDS_X_ALTKEY) + _T(" + ") + GetResString(IDS_X_SHIFTKEY));

	return 0;
}


class CInvisibelModeHotKey : public CTreeOptionsCombo
{
public:
	CInvisibelModeHotKey();
	virtual ~CInvisibelModeHotKey();

protected:
	//{{AFX_VIRTUAL(CInvisibelModeHotKey)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CInvisibelModeHotKey)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

  DECLARE_DYNCREATE(CInvisibelModeHotKey)
};


IMPLEMENT_DYNCREATE(CInvisibelModeHotKey, CTreeOptionsCombo)

CInvisibelModeHotKey::CInvisibelModeHotKey()
{
}

CInvisibelModeHotKey::~CInvisibelModeHotKey()
{
}

BEGIN_MESSAGE_MAP(CInvisibelModeHotKey, CTreeOptionsCombo)
	//{{AFX_MSG_MAP(CInvisibelModeHotKey)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CInvisibelModeHotKey::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeOptionsCombo::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  //Add strings to the combo
	for(int i='A'; i<='Z'; i++)
		AddString(CString((char)(i)));
	for(int i='0'; i<='9'; i++)
		AddString(CString((char)(i)));
	
	return 0;
}
// NEO: IM END

///////////////////////////////////////////////////////////////////////////////
// CPPgInterface dialog

IMPLEMENT_DYNAMIC(CPPgInterface, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgInterface, CPropertyPage)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_MESSAGE(WM_TREEITEM_HELP, DrawTreeItemHelp)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgInterface::CPPgInterface()
	: CPropertyPage(CPPgInterface::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	// NEO: NTB - [NeoToolbarButtons]
	m_htiNeoToolbarButtons = NULL;
	m_cntNeoToolbarButtons = 0;
	// NEO: NTB END
	ClearAllMembers();
	m_ctrlTreeOptions.SetNeoStyle();
}

CPPgInterface::~CPPgInterface()
{
	ClearNeoToolbarMembers(); // NEO: NTB - [NeoToolbarButtons]
}

void CPPgInterface::ClearAllMembers()
{
	m_bInitializedTreeOpts = false;

	// NEO: NXC - [NewExtendedCategories]
	m_htiMorphCats = NULL;
		m_htiShowCatNames = NULL;
		m_htiShowCategoryFlags = NULL;
		m_htiActiveCatDefault = NULL;
		m_htiSelCatOnAdd = NULL;
		m_htiAutoSetResumeOrder = NULL;
		m_htiSmallFileDLPush = NULL;
		m_htiStartDLInEmptyCats = NULL;
		m_htiUseAutoCat = NULL;
		m_htiCheckAlreadyDownloaded = NULL;
		m_htiStartNextFileByPriority = NULL;
	// NEO: NXC END
	// NEO: NTB - [NeoToolbarButtons]
	m_htiNeoToolbar = NULL;
		ClearNeoToolbarMembers();
	// NEO: NTB END
	m_htiShowBanner = NULL; // NEO: NPB - [PrefsBanner]
	m_htiCollorShareFiles = NULL; // NEO: NSC - [NeoSharedCategories]
	m_htiSmoothStatisticsGraphs = NULL; // NEO: NBC - [NeoBandwidthControl]
	m_htiDisableAutoSort = NULL; // NEO: SE - [SortExtension]
	// NEO: NSTI - [NewSystemTrayIcon]
	m_htiNewSystemTrayIcon = NULL;
		m_htiShowSystemTrayUpload = NULL;
		m_htiThinSystemTrayBars = NULL;
		m_htiTrayBarsMaxCollor = NULL;
	// NEO: NSTI END
	m_htiStaticTrayIcon = NULL; // NEO: STI - [StaticTray]
	// NEO: IM - [InvisibelMode]
	m_htiInvisibelMode = NULL;
		m_htiInvisibelModeHotKeyModifier = NULL;
		m_htiInvisibelModeHotKey = NULL;
	// NEO: IM END
	m_htiTrayPasswordProtection = NULL; // NEO: TPP - [TrayPasswordProtection]
	m_htiUsePlusSpeedMeter = NULL; // NEO: PSM - [PlusSpeedMeter]
	m_htiUseRelativeChunkDisplay = NULL; // NEO: MOD - [RelativeChunkDisplay]
	// NEO: SI - [SysInfo]
	m_htiDrawSysInfoGraph = NULL;
	m_htiShowSysInfoOnTitle = NULL;
	// NEO: SI END
	m_htiUseChunkDots = NULL;	// NEO: MOD - [ChunkDots]
	m_htiUseTreeStyle = NULL; // NEO: NTS - [NeoTreeStyle]
	m_htiShowClientPercentage = NULL; // NEO: MOD - [Percentage]
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	m_htiIP2Country = NULL;
		m_htiIP2CountryNameDisable = NULL; 
		m_htiIP2CountryNameShort = NULL; 
		m_htiIP2CountryNameMidle = NULL; 
		m_htiIP2CountryNameLong = NULL; 
		m_htiIP2CountryShowFlag = NULL; 
#endif // IP2COUNTRY // NEO: IP2C END
	// NEO: NMX - [NeoMenuXP]
	m_htiXPMenuStyle = NULL;
		m_htiXPMenuStyleOffice = NULL;
		m_htiXPMenuStyleStartMenu = NULL;
		m_htiXPMenuStyleXP = NULL;
		m_htiShowXPSideBar = NULL;
		m_htiShowXPBitmap = NULL;
		m_htiGrayMenuIcon = NULL;
	// NEO: NMX END
}

// NEO: NTB - [NeoToolbarButtons]
void CPPgInterface::ClearNeoToolbarMembers(bool bTree)
{
	if(m_htiNeoToolbarButtons){
		if(bTree){
			for (int i=0; i<m_cntNeoToolbarButtons; i++)
				DeleteTreeNeoToolbarButton(m_ctrlTreeOptions,&(m_htiNeoToolbarButtons[i]));
		}
		delete [] m_htiNeoToolbarButtons;
		m_htiNeoToolbarButtons = NULL;
		m_cntNeoToolbarButtons = 0;
	}
}

void CPPgInterface::CreateNeoToolbarMembers(int *iImgNeoBtn)
{
	ASSERT(m_htiNeoToolbarButtons == NULL);
	m_cntNeoToolbarButtons = m_iNeoToolbarButtonCount;
	m_htiNeoToolbarButtons = new tNeoButton[m_cntNeoToolbarButtons];
	for (int i=0; i<m_cntNeoToolbarButtons; i++)
		SetTreeNeoToolbarButton(m_ctrlTreeOptions,&(m_htiNeoToolbarButtons[i]),m_htiNeoToolbar,i,iImgNeoBtn);
}
// NEO: NTB END

void CPPgInterface::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);

	static int iImgNeoBtn[8] = {8,8,8,8,8,8,8,8}; // NEO: NTB - [NeoToolbarButtons]
	if (!m_bInitializedTreeOpts)
	{
		int iImgCat = 8;
		int iImgSysTray = 8; // NEO: NSTI - [NewSystemTrayIcon]
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
		int iImgIP2C = 8;
#endif // IP2COUNTRY // NEO: IP2C END
		int iImgXP = 0; // NEO: NMX - [NeoMenuXP]
        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgCat = piml->Add(CTempIconLoader(_T("CATEGORY")));
			// NEO: NTB - [NeoToolbarButtons]
			iImgNeoBtn[0] = piml->Add(CTempIconLoader(_T("NEO_BTN_BLUE")));
			iImgNeoBtn[1] = piml->Add(CTempIconLoader(_T("NEO_BTN_CYNA")));
			iImgNeoBtn[2] = piml->Add(CTempIconLoader(_T("NEO_BTN_GREEN")));
			iImgNeoBtn[3] = piml->Add(CTempIconLoader(_T("NEO_BTN_OLIVE")));
			iImgNeoBtn[4] = piml->Add(CTempIconLoader(_T("NEO_BTN_MAGENTA")));
			iImgNeoBtn[5] = piml->Add(CTempIconLoader(_T("NEO_BTN_RED")));
			iImgNeoBtn[6] = piml->Add(CTempIconLoader(_T("NEO_BTN_YELOW")));
			iImgNeoBtn[7] = piml->Add(CTempIconLoader(_T("NEO_BTN_ORANGE")));
			// NEO: NTB END
			iImgSysTray = piml->Add(CTempIconLoader(_T("TRAYCONNECTED"))); // NEO: NSTI - [NewSystemTrayIcon]
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
			iImgIP2C = piml->Add(CTempIconLoader(_T("SEARCHMETHOD_GLOBAL")));
#endif // IP2COUNTRY // NEO: IP2C END
			iImgXP = piml->Add(CTempIconLoader(_T("SPLITWINDOW"))); // NEO: NMX - [NeoMenuXP]
		}

		// NEO: NXC - [NewExtendedCategories]
		SetTreeGroup(m_ctrlTreeOptions,m_htiMorphCats,GetResString(IDS_X_MORPH_CATS),iImgCat, TVI_ROOT, GetResString(IDS_X_MORPH_CATS_INFO));
			SetTreeCheck(m_ctrlTreeOptions,m_htiShowCatNames,GetResString(IDS_X_SHOW_CAT_NAMES),m_htiMorphCats,GetResString(IDS_X_SHOW_CAT_NAMES_INFO),FALSE,m_bShowCatNames);
			SetTreeCheck(m_ctrlTreeOptions,m_htiShowCategoryFlags,GetResString(IDS_X_SHOW_CAT_FLAGS),m_htiMorphCats,GetResString(IDS_X_SHOW_CAT_FLAGS_INFO),TRUE,m_uShowCategoryFlags);
			SetTreeCheck(m_ctrlTreeOptions,m_htiSelCatOnAdd,GetResString(IDS_X_SEL_CAT_ON_ADD),m_htiMorphCats,GetResString(IDS_X_SEL_CAT_ON_ADD_INFO),FALSE,m_bSelCatOnAdd);
			SetTreeCheck(m_ctrlTreeOptions,m_htiActiveCatDefault,GetResString(IDS_X_ACTIVE_CAT_DEFAULT),m_htiMorphCats,GetResString(IDS_X_ACTIVE_CAT_DEFAULT_INFO),FALSE,m_bActiveCatDefault);
			SetTreeCheck(m_ctrlTreeOptions,m_htiUseAutoCat,GetResString(IDS_X_USE_AUTO_CAT),m_htiMorphCats,GetResString(IDS_X_USE_AUTO_CAT_INFO),FALSE,m_bUseAutoCat);
			SetTreeCheck(m_ctrlTreeOptions,m_htiCheckAlreadyDownloaded,GetResString(IDS_X_CHECK_ALREADY_DOWNLOADED),m_htiMorphCats,GetResString(IDS_X_CHECK_ALREADY_DOWNLOADED_INFO),FALSE,m_bCheckAlreadyDownloaded);
			SetTreeCheck(m_ctrlTreeOptions,m_htiStartNextFileByPriority,GetResString(IDS_X_START_NEXT_FILE_BY_PRIORITY),m_htiMorphCats,GetResString(IDS_X_START_NEXT_FILE_BY_PRIORITY_INFO),FALSE,m_bStartNextFileByPriority);
			SetTreeCheck(m_ctrlTreeOptions,m_htiAutoSetResumeOrder,GetResString(IDS_X_AUTO_SET_RESUME_ORDER),m_htiMorphCats,GetResString(IDS_X_AUTO_SET_RESUME_ORDER_INFO),FALSE,m_bAutoSetResumeOrder);
			SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiSmallFileDLPush,GetResString(IDS_X_SMALL_FILE_DL_PUSH),m_htiMorphCats,GetResString(IDS_X_SMALL_FILE_DL_PUSH_INFO),FALSE,m_bSmallFileDLPush);
			SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiStartDLInEmptyCats,GetResString(IDS_X_START_DL_IN_EMPTY_CATS),m_htiMorphCats,GetResString(IDS_X_START_DL_IN_EMPTY_CATS_INFO),FALSE,m_bStartDLInEmptyCats);
		// NEO: NXC END
		// NEO: NTB - [NeoToolbarButtons]
		SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiNeoToolbar,GetResString(IDS_X_SHOW_NEO_TOOLBAR),TVI_ROOT,GetResString(IDS_X_SHOW_NEO_TOOLBAR_INFO),TRUE,m_uNeoToolbar);
			CreateNeoToolbarMembers(iImgNeoBtn);
		// NEO: NTB END
		SetTreeCheck(m_ctrlTreeOptions,m_htiShowBanner,GetResString(IDS_X_PREFS_BANNER),TVI_ROOT,GetResString(IDS_X_PREFS_BANNER_INFO),FALSE,m_bShowBanner); // NEO: NPB - [PrefsBanner]
		SetTreeCheck(m_ctrlTreeOptions,m_htiCollorShareFiles,GetResString(IDS_X_COLLOR_SHARE_FILES),TVI_ROOT,GetResString(IDS_X_COLLOR_SHARE_FILES_INFO),FALSE,m_bCollorShareFiles); // NEO: NSC - [NeoSharedCategories]
		SetTreeCheck(m_ctrlTreeOptions,m_htiSmoothStatisticsGraphs,GetResString(IDS_X_SMOOTH_STATISTICS_GRAPHS),TVI_ROOT,GetResString(IDS_X_SMOOTH_STATISTICS_GRAPHS_INFO),FALSE,m_bSmoothStatisticsGraphs); // NEO: NBC - [NeoBandwidthControl] 
		SetTreeCheck(m_ctrlTreeOptions,m_htiDisableAutoSort,GetResString(IDS_X_DISABLE_AUTOSORT),TVI_ROOT,GetResString(IDS_X_DISABLE_AUTOSORT_INFO),TRUE,m_uDisableAutoSort);

		// NEO: NSTI - [NewSystemTrayIcon]
		SetTreeGroup(m_ctrlTreeOptions,m_htiNewSystemTrayIcon,GetResString(IDS_X_NSTI),iImgSysTray, TVI_ROOT, GetResString(IDS_X_NSTI_INFO));
			SetTreeCheck(m_ctrlTreeOptions,m_htiShowSystemTrayUpload,GetResString(IDS_X_NSTI_UL),m_htiNewSystemTrayIcon,GetResString(IDS_X_NSTI_UL_INFO),FALSE,m_bShowSystemTrayUpload);
			SetTreeCheck(m_ctrlTreeOptions,m_htiThinSystemTrayBars,GetResString(IDS_X_NSTI_UL_THIN),m_htiNewSystemTrayIcon,GetResString(IDS_X_NSTI_UL_THIN_INFO),FALSE,m_bThinSystemTrayBars);
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiTrayBarsMaxCollor,GetResString(IDS_X_NSTI_MAX),m_htiNewSystemTrayIcon,GetResString(IDS_X_NSTI_MAX_INFO));
		// NEO: NSTI END
		SetTreeCheck(m_ctrlTreeOptions,m_htiStaticTrayIcon,GetResString(IDS_X_STATIC_TRAY_ICON),TVI_ROOT,GetResString(IDS_X_STATIC_TRAY_ICON_INFO),FALSE,m_bStaticTrayIcon); // NEO: STI - [StaticTray]
		// NEO: IM - [InvisibelMode]
		SetTreeCheck(m_ctrlTreeOptions,m_htiInvisibelMode,GetResString(IDS_X_INVISIBLE_MODE),TVI_ROOT,GetResString(IDS_X_INVISIBLE_MODE_INFO),FALSE,m_bInvisibleMode);

			m_htiInvisibelModeHotKeyModifier = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_INVISIBLE_KEY_MOD), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiInvisibelMode);
			m_ctrlTreeOptions.AddComboBox(m_htiInvisibelModeHotKeyModifier,RUNTIME_CLASS(CInvisibelModeHotKeyModifier));
			//m_ctrlTreeOptions.SetItemSize(m_htiInvisibelModeHotKeyModifier,15);
			m_ctrlTreeOptions.SetItemInfo(m_htiInvisibelModeHotKeyModifier,GetResString(IDS_X_INVISIBLE_KEY_MOD_INFO));

			m_htiInvisibelModeHotKey = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_INVISIBLE_KEY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiInvisibelMode);
			m_ctrlTreeOptions.AddComboBox(m_htiInvisibelModeHotKey,RUNTIME_CLASS(CInvisibelModeHotKey));
			//m_ctrlTreeOptions.SetItemSize(m_htiInvisibelModeHotKey,5);
			m_ctrlTreeOptions.SetItemInfo(m_htiInvisibelModeHotKey,GetResString(IDS_X_INVISIBLE_KEY_INFO));
		// NEO: IM END
		SetTreeCheckEdit(m_ctrlTreeOptions,m_htiTrayPasswordProtection,GetResString(IDS_X_TRAY_PASSWORD_PROTECTION),TVI_ROOT,GetResString(IDS_X_TRAY_PASSWORD_PROTECTION_INFO),FALSE,m_bTrayPasswordProtection);// NEO: TPP - [TrayPasswordProtection]
		SetTreeCheck(m_ctrlTreeOptions,m_htiUsePlusSpeedMeter,GetResString(IDS_X_PLUS_METER),TVI_ROOT,GetResString(IDS_X_PLUS_METER_INFO),FALSE,m_bUsePlusSpeedMeter); // NEO: PSM - [PlusSpeedMeter]
		SetTreeCheck(m_ctrlTreeOptions,m_htiUseRelativeChunkDisplay,GetResString(IDS_X_USE_RCD),TVI_ROOT,GetResString(IDS_X_USE_RCD_INFO),FALSE,m_bUseRelativeChunkDisplay); // NEO: MOD - [RelativeChunkDisplay]
		// NEO: SI - [SysInfo]
		SetTreeCheck(m_ctrlTreeOptions,m_htiDrawSysInfoGraph,GetResString(IDS_X_USE_SYS_INFO),TVI_ROOT,GetResString(IDS_X_USE_SYS_INFO_INFO),FALSE,m_bDrawSysInfoGraph);
		SetTreeCheck(m_ctrlTreeOptions,m_htiShowSysInfoOnTitle,GetResString(IDS_X_USE_SYS_INFO_TITLE),TVI_ROOT,GetResString(IDS_X_USE_SYS_INFO_TITLE_INFO),FALSE,m_bShowSysInfoOnTitle);
		// NEO: SI END	
		SetTreeCheck(m_ctrlTreeOptions,m_htiUseChunkDots,GetResString(IDS_X_USE_DOTS),TVI_ROOT,GetResString(IDS_X_USE_DOTS_INFO),FALSE,m_bUseChunkDots); // NEO: MOD - [ChunkDots]
		SetTreeCheck(m_ctrlTreeOptions,m_htiUseTreeStyle,GetResString(IDS_X_USE_NTS),TVI_ROOT,GetResString(IDS_X_USE_NTS_INFO),FALSE,m_bUseTreeStyle); // NEO: NTS - [NeoTreeStyle]
		SetTreeCheck(m_ctrlTreeOptions,m_htiShowClientPercentage,GetResString(IDS_X_SHOW_CLIENT_PERCENTAGE),TVI_ROOT,GetResString(IDS_X_SHOW_CLIENT_PERCENTAGE_INFO),FALSE,m_bShowClientPercentage); // NEO: MOD - [Percentage]
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
		SetTreeGroup(m_ctrlTreeOptions,m_htiIP2Country,GetResString(IDS_X_IP2COUNTRY_MAIN),iImgIP2C, TVI_ROOT, GetResString(IDS_X_IP2COUNTRY_MAIN_INFO));
			SetTreeRadio(m_ctrlTreeOptions,m_htiIP2CountryNameDisable,GetResString(IDS_X_IP2COUNTRY_DISABLED),m_htiIP2Country,GetResString(IDS_X_IP2COUNTRY_DISABLED_INFO),FALSE,m_iIP2CountryNameMode == IP2CountryName_DISABLE);
			SetTreeRadio(m_ctrlTreeOptions,m_htiIP2CountryNameShort,GetResString(IDS_X_IP2COUNTRY_SHORT),m_htiIP2Country,GetResString(IDS_X_IP2COUNTRY_SHORT_INFO),FALSE,m_iIP2CountryNameMode == IP2CountryName_SHORT);
			SetTreeRadio(m_ctrlTreeOptions,m_htiIP2CountryNameMidle,GetResString(IDS_X_IP2COUNTRY_MIDLE),m_htiIP2Country,GetResString(IDS_X_IP2COUNTRY_MIDLE_INFO),FALSE,m_iIP2CountryNameMode == IP2CountryName_MID);
			SetTreeRadio(m_ctrlTreeOptions,m_htiIP2CountryNameLong,GetResString(IDS_X_IP2COUNTRY_LONG),m_htiIP2Country,GetResString(IDS_X_IP2COUNTRY_LONG_INFO),FALSE,m_iIP2CountryNameMode == IP2CountryName_LONG);
			SetTreeCheck(m_ctrlTreeOptions,m_htiIP2CountryShowFlag,GetResString(IDS_X_IP2COUNTRY_FLAGS),m_htiIP2Country,GetResString(IDS_X_IP2COUNTRY_FLAGS_INFO),TRUE,m_uIP2CountryShowFlag);
#endif // IP2COUNTRY // NEO: IP2C END
		// NEO: NMX - [NeoMenuXP]
		SetTreeGroup(m_ctrlTreeOptions,m_htiXPMenuStyle,GetResString(IDS_X_XP_MENU_STYLE),iImgXP, TVI_ROOT, GetResString(IDS_X_XP_MENU_STYLE_INFO));
			SetTreeRadio(m_ctrlTreeOptions,m_htiXPMenuStyleOffice,GetResString(IDS_X_OFFICE_MENUS),m_htiXPMenuStyle,GetResString(IDS_X_OFFICE_MENUS_INFO),FALSE,m_iXPMenuStyle == 0);
			SetTreeRadio(m_ctrlTreeOptions,m_htiXPMenuStyleStartMenu,GetResString(IDS_X_START_MENUS),m_htiXPMenuStyle,GetResString(IDS_X_START_MENUS_INFO),FALSE,m_iXPMenuStyle == 1);
			SetTreeRadio(m_ctrlTreeOptions,m_htiXPMenuStyleXP,GetResString(IDS_X_XP_MENUS),m_htiXPMenuStyle,GetResString(IDS_X_XP_MENUS_INFO),FALSE,m_iXPMenuStyle == 2);
			SetTreeCheck(m_ctrlTreeOptions,m_htiShowXPSideBar,GetResString(IDS_X_SHOW_XP_SIDEBAR),m_htiXPMenuStyle,GetResString(IDS_X_SHOW_XP_SIDEBAR_INFO),FALSE,m_bShowXPSideBar);
			SetTreeCheck(m_ctrlTreeOptions,m_htiShowXPBitmap,GetResString(IDS_X_SHOW_XP_BITMAP),m_htiXPMenuStyle,GetResString(IDS_X_SHOW_XP_BITMAP_INFO),FALSE,m_bShowXPBitmap);
			SetTreeCheck(m_ctrlTreeOptions,m_htiGrayMenuIcon,GetResString(IDS_X_GRAY_MENU_ICON),m_htiXPMenuStyle,GetResString(IDS_X_GRAY_MENU_ICON_INFO),FALSE,m_bGrayMenuIcon);
		// NEO: NMX END

		// NEO: NTB - [NeoToolbarButtons]
		UINT bCheck;
		m_ctrlTreeOptions.GetCheckBox(m_htiNeoToolbar, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiNeoToolbar, bCheck,TRUE,TRUE);
		//m_ctrlTreeOptions.SetGroupEnable(m_htiNeoToolbar, bCheck);
		// NEO: NTB END

		// NEO: IM - [InvisibelMode]
		m_ctrlTreeOptions.SetItemEnable(m_htiInvisibelModeHotKeyModifier, m_bInvisibleMode);
		m_ctrlTreeOptions.SetItemEnable(m_htiInvisibelModeHotKey, m_bInvisibleMode);
		// NEO: IM END

		//m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}

	// NEO: NXC - [NewExtendedCategories]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiShowCatNames,m_bShowCatNames);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiShowCategoryFlags,m_uShowCategoryFlags);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiActiveCatDefault,m_bActiveCatDefault);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSelCatOnAdd,m_bSelCatOnAdd);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiAutoSetResumeOrder,m_bAutoSetResumeOrder);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSmallFileDLPush,m_bSmallFileDLPush);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiSmallFileDLPush, m_iSmallFileDLPush);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiStartDLInEmptyCats,m_bStartDLInEmptyCats);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiStartDLInEmptyCats, m_iStartDLInEmptyCats);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseAutoCat,m_bUseAutoCat);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCheckAlreadyDownloaded,m_bCheckAlreadyDownloaded);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiStartNextFileByPriority,m_bStartNextFileByPriority);
	// NEO: NXC END
	// NEO: NTB - [NeoToolbarButtons]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiNeoToolbar,m_uNeoToolbar);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiNeoToolbar, m_iNeoToolbarButtonCount);
	if(m_iNeoToolbarButtonCount != m_cntNeoToolbarButtons){
		ClearNeoToolbarMembers(true);
		CreateNeoToolbarMembers(iImgNeoBtn);

		CArray<UINT> tmpNeoToolbarButton;
		int tmpNeoToolbarButtons = m_NeoToolbarButtons.GetSize();
		tmpNeoToolbarButton.SetSize(tmpNeoToolbarButtons);
		for (int i=0; i<tmpNeoToolbarButtons; i++)
			tmpNeoToolbarButton[i] = m_NeoToolbarButtons[i];
		m_NeoToolbarButtons.RemoveAll();
		m_NeoToolbarButtons.SetSize(m_iNeoToolbarButtonCount);
		tmpNeoToolbarButtons = min(m_NeoToolbarButtons.GetSize(),tmpNeoToolbarButton.GetSize());
		CDataExchange pDXtmp = *pDX;
		pDXtmp.m_bSaveAndValidate = FALSE;
		for (int i=0; i<tmpNeoToolbarButtons; i++){
			m_NeoToolbarButtons[i] = tmpNeoToolbarButton[i];
			DDX_TreeNeoToolbarButton(&m_ctrlTreeOptions, &pDXtmp, IDC_MOD_OPTS, &(m_htiNeoToolbarButtons[i]), m_NeoToolbarButtons[i]);
		}
	}
	else{
		for (int i=0; i<m_cntNeoToolbarButtons; i++)
			DDX_TreeNeoToolbarButton(&m_ctrlTreeOptions, pDX, IDC_MOD_OPTS, &(m_htiNeoToolbarButtons[i]), m_NeoToolbarButtons[i]);
	}
	// NEO: NTB END
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiShowBanner,m_bShowBanner); // NEO: NPB - [PrefsBanner]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCollorShareFiles,m_bCollorShareFiles); // NEO: NSC - [NeoSharedCategories]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSmoothStatisticsGraphs,m_bSmoothStatisticsGraphs); // NEO: NBC - [NeoBandwidthControl] 
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiDisableAutoSort,m_uDisableAutoSort); // NEO: SE - [SortExtension]
	// NEO: NSTI - [NewSystemTrayIcon]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiShowSystemTrayUpload,m_bShowSystemTrayUpload);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiThinSystemTrayBars,m_bThinSystemTrayBars);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiTrayBarsMaxCollor,m_iTrayBarsMaxCollor);
	// NEO: NSTI END
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiStaticTrayIcon,m_bStaticTrayIcon); // NEO: STI - [StaticTray]
	// NEO: IM - [InvisibelMode]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS,m_htiInvisibelMode,m_bInvisibleMode);
	if (pDX->m_bSaveAndValidate)
	{
		CString sKeyMod;
		sKeyMod = m_ctrlTreeOptions.GetComboText(m_htiInvisibelModeHotKeyModifier);

		m_iInvisibleModeHotKeyModifier = 0;
		if (sKeyMod.Find(GetResString(IDS_X_CTRLKEY))!=-1)
			m_iInvisibleModeHotKeyModifier |= MOD_CONTROL;
		if (sKeyMod.Find(GetResString(IDS_X_ALTKEY))!=-1)
			m_iInvisibleModeHotKeyModifier |= MOD_ALT;
		if (sKeyMod.Find(GetResString(IDS_X_SHIFTKEY))!=-1)
			m_iInvisibleModeHotKeyModifier |= MOD_SHIFT;

		CString sKey;
		sKey = m_ctrlTreeOptions.GetComboText(m_htiInvisibelModeHotKey);

		m_cInvisibleModeHotKey = sKey[0];
	}
	else
	{
		m_ctrlTreeOptions.SetComboText(m_htiInvisibelModeHotKey,CString(m_cInvisibleModeHotKey));

		CString sKeyMod;
		bool bPlus = false;

		if(m_iInvisibleModeHotKeyModifier & MOD_CONTROL){
			sKeyMod.Format(GetResString(IDS_X_CTRLKEY));
			bPlus = true;
		}
		if(m_iInvisibleModeHotKeyModifier & MOD_ALT){
			if(bPlus)
				sKeyMod.Append(_T(" + "));
			sKeyMod.Append(GetResString(IDS_X_ALTKEY));
			bPlus = true;
		}
		if(m_iInvisibleModeHotKeyModifier & MOD_SHIFT){
			if(bPlus)
				sKeyMod.Append(_T(" + "));
			sKeyMod.Append(GetResString(IDS_X_SHIFTKEY));
			bPlus = true;
		}

		m_ctrlTreeOptions.SetComboText(m_htiInvisibelModeHotKeyModifier,sKeyMod);

		m_ctrlTreeOptions.SetComboText(m_htiInvisibelModeHotKey,CString(m_cInvisibleModeHotKey));
	}
	// NEO: IM END
	// NEO: TPP - [TrayPasswordProtection]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS,m_htiTrayPasswordProtection, m_bTrayPasswordProtection);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiTrayPasswordProtection, m_sTrayPassword);
	// NEO: TPP END
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUsePlusSpeedMeter,m_bUsePlusSpeedMeter); // NEO: PSM - [PlusSpeedMeter]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseRelativeChunkDisplay,m_bUseRelativeChunkDisplay);  // NEO: MOD - [RelativeChunkDisplay]
	// NEO: SI - [SysInfo]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiDrawSysInfoGraph,m_bDrawSysInfoGraph);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiShowSysInfoOnTitle,m_bShowSysInfoOnTitle);
	// NEO: SI END	
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseChunkDots,m_bUseChunkDots);  // NEO: MOD - [ChunkDots]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseTreeStyle,m_bUseTreeStyle);  // NEO: NTS - [NeoTreeStyle]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiShowClientPercentage,m_bShowClientPercentage); // NEO: MOD - [Percentage]
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiIP2Country, m_iIP2CountryNameMode);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiIP2CountryShowFlag,m_uIP2CountryShowFlag);
#endif // IP2COUNTRY // NEO: IP2C END
	// NEO: NMX - [NeoMenuXP]
	DDX_TreeRadio(pDX, IDC_MOD_OPTS,m_htiXPMenuStyle,m_iXPMenuStyle);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS,m_htiShowXPSideBar,m_bShowXPSideBar);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS,m_htiShowXPBitmap,m_bShowXPBitmap);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS,m_htiGrayMenuIcon,m_bGrayMenuIcon);
	// NEO: NMX END
}

BOOL CPPgInterface::OnInitDialog()
{
	LoadSettings();

	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgInterface::LoadSettings()
{
	// NEO: NXC - [NewExtendedCategories]
	m_bShowCatNames = NeoPrefs.m_bShowCatNames;
	m_uShowCategoryFlags = NeoPrefs.m_uShowCategoryFlags;
	m_bActiveCatDefault = NeoPrefs.m_bActiveCatDefault;
	m_bSelCatOnAdd = NeoPrefs.m_bSelCatOnAdd;
	m_bAutoSetResumeOrder = NeoPrefs.m_bAutoSetResumeOrder;
	m_bSmallFileDLPush = NeoPrefs.m_bSmallFileDLPush;
	m_iSmallFileDLPush = NeoPrefs.m_iSmallFileDLPush;
	m_bStartDLInEmptyCats = NeoPrefs.m_bStartDLInEmptyCats;
	m_iStartDLInEmptyCats = NeoPrefs.m_iStartDLInEmptyCats;
	m_bUseAutoCat = NeoPrefs.m_bUseAutoCat;
	m_bCheckAlreadyDownloaded = NeoPrefs.m_bCheckAlreadyDownloaded;
	m_bStartNextFileByPriority = NeoPrefs.m_bStartNextFileByPriority;
	// NEO: NXC END
	// NEO: NTB - [NeoToolbarButtons]
	m_uNeoToolbar = NeoPrefs.m_uNeoToolbar;
	m_iNeoToolbarButtonCount = NeoPrefs.m_iNeoToolbarButtonCount;
	m_NeoToolbarButtons.SetSize(m_iNeoToolbarButtonCount);
	for(int i=0; i<m_iNeoToolbarButtonCount; i++)
		m_NeoToolbarButtons[i] = NeoPrefs.m_NeoToolbarButtons[i];
	// NEO: NTB END
	m_bShowBanner = NeoPrefs.m_bShowBanner; // NEO: NPB - [PrefsBanner]
	m_bCollorShareFiles = NeoPrefs.m_bCollorShareFiles; // NEO: NSC - [NeoSharedCategories]
	m_bSmoothStatisticsGraphs = NeoPrefs.m_bSmoothStatisticsGraphs; // NEO: NBC - [NeoBandwidthControl] 
	m_uDisableAutoSort = NeoPrefs.m_uDisableAutoSort; // NEO: SE - [SortExtension]
	// NEO: NSTI - [NewSystemTrayIcon]
	m_bShowSystemTrayUpload = NeoPrefs.m_bShowSystemTrayUpload;
	m_bThinSystemTrayBars = NeoPrefs.m_bThinSystemTrayBars;
	m_iTrayBarsMaxCollor = NeoPrefs.m_iTrayBarsMaxCollor;
	// NEO: NSTI END
	m_bStaticTrayIcon = NeoPrefs.m_bStaticTrayIcon; // NEO: STI - [StaticTray]
	// NEO: IM - [InvisibelMode]
	m_bInvisibleMode = NeoPrefs.m_bInvisibleMode;
	m_iInvisibleModeHotKeyModifier = NeoPrefs.m_iInvisibleModeHotKeyModifier;
	m_cInvisibleModeHotKey = NeoPrefs.m_cInvisibleModeHotKey;
	// NEO: IM END
	// NEO: TPP - [TrayPasswordProtection]
	m_bTrayPasswordProtection = NeoPrefs.m_bTrayPasswordProtection;
	m_sTrayPassword = NeoPrefs.m_sTrayPassword;
	// NEO: TPP END
	m_bUsePlusSpeedMeter = NeoPrefs.m_bUsePlusSpeedMeter; // NEO: PSM - [PlusSpeedMeter]
	m_bUseRelativeChunkDisplay = NeoPrefs.m_bUseRelativeChunkDisplay; // NEO: MOD - [RelativeChunkDisplay]
	// NEO: SI - [SysInfo]
	m_bDrawSysInfoGraph = NeoPrefs.m_bDrawSysInfoGraph;
	m_bShowSysInfoOnTitle = NeoPrefs.m_bShowSysInfoOnTitle;
	// NEO: SI END
	m_bUseChunkDots = NeoPrefs.m_bUseChunkDots;	// NEO: MOD - [ChunkDots]
	m_bUseTreeStyle = NeoPrefs.m_bUseTreeStyle; 	// NEO: NTS - [NeoTreeStyle]
	m_bShowClientPercentage = NeoPrefs.m_bShowClientPercentage; // NEO: MOD - [Percentage]
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	m_iIP2CountryNameMode = NeoPrefs.m_iIP2CountryNameMode; 
	m_uIP2CountryShowFlag = NeoPrefs.m_uIP2CountryShowFlag; 
#endif // IP2COUNTRY // NEO: IP2C END
	// NEO: NMX - [NeoMenuXP]
	m_bShowXPSideBar = NeoPrefs.m_bShowXPSideBar;
	m_bShowXPBitmap = NeoPrefs.m_bShowXPBitmap;
	m_iXPMenuStyle = NeoPrefs.m_iXPMenuStyle;
	m_bGrayMenuIcon = NeoPrefs.m_bGrayMenuIcon;
	// NEO: NMX END
}

BOOL CPPgInterface::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	// NEO: NXC - [NewExtendedCategories]
	NeoPrefs.m_bShowCatNames = m_bShowCatNames;
	if(NeoPrefs.m_uShowCategoryFlags != m_uShowCategoryFlags){
		NeoPrefs.m_uShowCategoryFlags = m_uShowCategoryFlags;
		theApp.emuledlg->transferwnd->UpdateCatTabIcons(true);
		theApp.emuledlg->transferwnd->VerifyCatTabSize();
	}
	NeoPrefs.m_bActiveCatDefault = m_bActiveCatDefault;
	NeoPrefs.m_bSelCatOnAdd = m_bSelCatOnAdd;
	NeoPrefs.m_bAutoSetResumeOrder = m_bAutoSetResumeOrder;
	NeoPrefs.m_bSmallFileDLPush = m_bSmallFileDLPush;
	NeoPrefs.m_iSmallFileDLPush = m_iSmallFileDLPush;
	NeoPrefs.m_bStartDLInEmptyCats = m_bStartDLInEmptyCats;
	NeoPrefs.m_iStartDLInEmptyCats = m_iStartDLInEmptyCats;
	NeoPrefs.m_bUseAutoCat = m_bUseAutoCat;
	NeoPrefs.m_bCheckAlreadyDownloaded = m_bCheckAlreadyDownloaded;
	NeoPrefs.m_bStartNextFileByPriority = m_bStartNextFileByPriority;
	// NEO: NXC END
	// NEO: NTB - [NeoToolbarButtons]
	bool bUpdateToolbar = false;
	if(NeoPrefs.m_uNeoToolbar != m_uNeoToolbar){
		NeoPrefs.m_uNeoToolbar = m_uNeoToolbar;
		bUpdateToolbar = true;
	}
	if(NeoPrefs.m_iNeoToolbarButtonCount != m_iNeoToolbarButtonCount){
		NeoPrefs.m_iNeoToolbarButtonCount = m_iNeoToolbarButtonCount;
		bUpdateToolbar = true;
		NeoPrefs.m_NeoToolbarButtons.RemoveAll();
		NeoPrefs.m_NeoToolbarButtons.SetSize(m_iNeoToolbarButtonCount);
		for(int i=0; i<m_iNeoToolbarButtonCount; i++){
			NeoPrefs.m_NeoToolbarButtons[i] = m_NeoToolbarButtons[i];
		}
	}
	else{
		for(int i=0; i<m_iNeoToolbarButtonCount; i++){
			if(NeoPrefs.m_NeoToolbarButtons[i] != m_NeoToolbarButtons[i]){
				NeoPrefs.m_NeoToolbarButtons[i] = m_NeoToolbarButtons[i];
				bUpdateToolbar = true;
			}
		}
	}
	if(bUpdateToolbar)
		theApp.emuledlg->transferwnd->ResetTransToolbar(thePrefs.IsTransToolbarEnabled());
	// NEO: NTB END
	NeoPrefs.m_bShowBanner = m_bShowBanner; // NEO: NPB - [PrefsBanner]
	NeoPrefs.m_bCollorShareFiles = m_bCollorShareFiles; // NEO: NSC - [NeoSharedCategories]
	// NEO: NBC - [NeoBandwidthControl] 
	if(NeoPrefs.m_bSmoothStatisticsGraphs != m_bSmoothStatisticsGraphs)
	{
		NeoPrefs.m_bSmoothStatisticsGraphs = m_bSmoothStatisticsGraphs;
#ifdef NEO_BC 
		theApp.bandwidthControl->InitStats(false);
#endif // NEO_BC
	}
	// NEO: NBC END
	NeoPrefs.m_uDisableAutoSort = m_uDisableAutoSort; // NEO: SE - [SortExtension]
	// NEO: NSTI - [NewSystemTrayIcon]
	NeoPrefs.m_bShowSystemTrayUpload = m_bShowSystemTrayUpload;
	NeoPrefs.m_bThinSystemTrayBars = m_bThinSystemTrayBars;
	NeoPrefs.m_iTrayBarsMaxCollor = m_iTrayBarsMaxCollor;
	// NEO: NSTI END
	NeoPrefs.m_bStaticTrayIcon = m_bStaticTrayIcon; // NEO: STI - [StaticTray]
	// NEO: IM - [InvisibelMode]
	NeoPrefs.m_bInvisibleMode = m_bInvisibleMode;
	NeoPrefs.m_iInvisibleModeHotKeyModifier = m_iInvisibleModeHotKeyModifier;
	NeoPrefs.m_cInvisibleModeHotKey = m_cInvisibleModeHotKey;
	//Always unregister, the keys could be different.
	theApp.emuledlg->UnRegisterInvisibleHotKey();
	if(m_bInvisibleMode)
		theApp.emuledlg->RegisterInvisibleHotKey();
	// NEO: IM END
	// NEO: TPP - [TrayPasswordProtection]
	NeoPrefs.m_bTrayPasswordProtection = m_bTrayPasswordProtection;
	NeoPrefs.m_sTrayPassword = m_sTrayPassword;
	// NEO: TPP END
	// NEO: PSM - [PlusSpeedMeter]
	NeoPrefs.m_bUsePlusSpeedMeter = m_bUsePlusSpeedMeter;
	theApp.emuledlg->toolbar->ShowSpeedMeter(NeoPrefs.UsePlusSpeedMeter()); 
	// NEO: PSM END
	NeoPrefs.m_bUseRelativeChunkDisplay = m_bUseRelativeChunkDisplay; // NEO: MOD - [RelativeChunkDisplay]
	// NEO: SI - [SysInfo]
	NeoPrefs.m_bDrawSysInfoGraph = m_bDrawSysInfoGraph;
	NeoPrefs.m_bShowSysInfoOnTitle = m_bShowSysInfoOnTitle;
	if (!NeoPrefs.ShowSysInfoOnTitle())
		theApp.emuledlg->SetWindowText(theApp.GetAppTitle(true)); // NEO: NV - [NeoVersion]
	// NEO: SI END
	NeoPrefs.m_bUseChunkDots = m_bUseChunkDots;	// NEO: MOD - [ChunkDots]
	NeoPrefs.m_bUseTreeStyle = m_bUseTreeStyle; 	// NEO: NTS - [NeoTreeStyle]
	NeoPrefs.m_bShowClientPercentage = m_bShowClientPercentage; // NEO: MOD - [Percentage]

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	if(	(NeoPrefs.m_iIP2CountryNameMode != IP2CountryName_DISABLE || NeoPrefs.m_uIP2CountryShowFlag) !=
		(m_iIP2CountryNameMode != IP2CountryName_DISABLE || m_uIP2CountryShowFlag)	){
		//check if need to load or unload DLL and ip table
		if(m_iIP2CountryNameMode != IP2CountryName_DISABLE || m_uIP2CountryShowFlag)
			theApp.ip2country->Load();
		else
			theApp.ip2country->Unload();
		if(m_iIP2CountryNameMode != IP2CountryName_DISABLE){
			theApp.emuledlg->serverwnd->serverlistctrl.ShowColumn(16);
			theApp.emuledlg->transferwnd->clientlistctrl.ShowColumn(8);
			theApp.emuledlg->transferwnd->queuelistctrl.ShowColumn(13);
			theApp.emuledlg->transferwnd->uploadlistctrl.ShowColumn(11);
			theApp.emuledlg->transferwnd->downloadclientsctrl.ShowColumn(8);
		}else{
			theApp.emuledlg->serverwnd->serverlistctrl.ShowColumn(16);
			theApp.emuledlg->transferwnd->clientlistctrl.HideColumn(8);
			theApp.emuledlg->transferwnd->queuelistctrl.HideColumn(12);
			theApp.emuledlg->transferwnd->uploadlistctrl.HideColumn(11);
			theApp.emuledlg->transferwnd->downloadclientsctrl.HideColumn(8);
		}
	}
	NeoPrefs.m_iIP2CountryNameMode = m_iIP2CountryNameMode;
	NeoPrefs.m_uIP2CountryShowFlag = m_uIP2CountryShowFlag;
#endif // IP2COUNTRY // NEO: IP2C END

	// NEO: NMX - [NeoMenuXP]
	if( NeoPrefs.m_bShowXPSideBar != m_bShowXPSideBar
	 || NeoPrefs.m_bShowXPBitmap != m_bShowXPBitmap
	 || NeoPrefs.m_iXPMenuStyle != m_iXPMenuStyle
	 || NeoPrefs.m_bGrayMenuIcon != m_bGrayMenuIcon)
	{
		NeoPrefs.m_bShowXPSideBar = m_bShowXPSideBar;
		NeoPrefs.m_bShowXPBitmap = m_bShowXPBitmap;
		NeoPrefs.m_iXPMenuStyle = m_iXPMenuStyle;
		NeoPrefs.m_bGrayMenuIcon = m_bGrayMenuIcon;

		theApp.emuledlg->transferwnd->downloadlistctrl.CreateMenues();
		theApp.emuledlg->searchwnd->CreateMenus();
		theApp.emuledlg->searchwnd->m_pwndResults->searchlistctrl.CreateMenues();
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.CreateMenues();
		theApp.emuledlg->sharedfileswnd->m_ctlSharedDirTree.CreateMenues();
	}
	// NEO: NMX END

	NeoPrefs.CheckNeoPreferences();
	LoadSettings();


	SetModified(FALSE);

	return CPropertyPage::OnApply();
}

BOOL CPPgInterface::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgInterface::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	ClearAllMembers();
	CPropertyPage::OnDestroy();
}

LRESULT CPPgInterface::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code == EN_KILLFOCUS){
			// NEO: NTB - [NeoToolbarButtons]
			if(m_htiNeoToolbar && pton->hItem == m_htiNeoToolbar){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiNeoToolbar, 1, 8, 100)) SetModified();
			}
			// NEO: NTB END
		}else if(pton->nmhdr.code == BN_CLICKED){
			UINT bCheck;
			// NEO: NTB - [NeoToolbarButtons]
			if(m_htiNeoToolbar && pton->hItem == m_htiNeoToolbar){
				m_ctrlTreeOptions.GetCheckBox(m_htiNeoToolbar, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiNeoToolbar, bCheck,TRUE,TRUE);
				//m_ctrlTreeOptions.SetGroupEnable(m_htiNeoToolbarButtons, bCheck);
			}
			// NEO: NTB END
			// NEO: IM - [InvisibelMode]
			else if(m_htiInvisibelMode && pton->hItem == m_htiInvisibelMode){
				m_ctrlTreeOptions.GetCheckBox(m_htiInvisibelMode, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiInvisibelModeHotKeyModifier, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiInvisibelModeHotKey, bCheck);
			}
			// NEO: IM END
			SetModified();
		}else{
			SetModified();
		}
	}
	return 0;
}

LRESULT CPPgInterface::DrawTreeItemHelp(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		CString* sInfo = (CString*)lParam;
		SetDlgItemText(IDC_MOD_OPTS_INFO, *sInfo);
	}
	return FALSE;
}

void CPPgInterface::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgInterface::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgInterface::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
