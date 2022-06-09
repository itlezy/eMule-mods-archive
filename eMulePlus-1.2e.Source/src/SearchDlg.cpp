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
#include "SearchDlg.h"
#include "packets.h"
#include "server.h"
#include "opcodes.h"
#include "ServerList.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "ED2KLink.h"
#include "CustomAutoComplete.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define	SEARCH_STRINGS_PROFILE	_T("AC_SearchStrings.dat")

//	Search control masks to enable/disable controls
#define EP_SCHCTRL_EXCL		0x01
#define EP_SCHCTRL_TYPE		0x02
#define EP_SCHCTRL_MIN		0x04
#define EP_SCHCTRL_MAX		0x08
#define EP_SCHCTRL_AVAIL	0x10
#define EP_SCHCTRL_EXT		0x20

//	Search type indexes
enum
{
	EP_SCHTYPE_ANY = 0,
	EP_SCHTYPE_ARC,
	EP_SCHTYPE_AUDIO,
	EP_SCHTYPE_CDIMG,
	EP_SCHTYPE_DOC,
	EP_SCHTYPE_PICS,
	EP_SCHTYPE_PRG,
	EP_SCHTYPE_VIDEO,

	EP_SCHTYPE_COUNT
};

//	Search server parameter types
#define SCH_SRV_PARAMTYPE_BOOL			0	// boolean operator
#define SCH_SRV_PARAMTYPE_STR			1	// string
#define SCH_SRV_PARAMTYPE_TAGSTR		2	// string with tag
#define SCH_SRV_PARAMTYPE_TAGNUM		3	// number with tag
#define SCH_SRV_PARAMTYPE_TAGNUM64	8	// 64bit number with tag

#define SCH_SRV_BOOLPARAM_AND			0
#define SCH_SRV_BOOLPARAM_OR			1
#define SCH_SRV_BOOLPARAM_NOT			2

class CSearchExprTarget
{
public:
	CSearchExprTarget(CMemFile *pStream1, CMemFile *pStream2)
	{
		m_pStrm1 = pStream1;
		m_pStrm2 = pStream2;
		m_bContains64bitData = false;
	}

	void AddBooleanAND()
	{
		static const byte s_aBoolPar[] = { SCH_SRV_PARAMTYPE_BOOL, SCH_SRV_BOOLPARAM_AND };

		m_pStrm1->Write(&s_aBoolPar, sizeof(s_aBoolPar));
		m_pStrm2->Write(&s_aBoolPar, sizeof(s_aBoolPar));
	}

	void AddBooleanOR()
	{
		static const byte s_aBoolPar[] = { SCH_SRV_PARAMTYPE_BOOL, SCH_SRV_BOOLPARAM_OR };

		m_pStrm1->Write(&s_aBoolPar, sizeof(s_aBoolPar));
		m_pStrm2->Write(&s_aBoolPar, sizeof(s_aBoolPar));
	}

	void AddBooleanNOT()
	{
		static const byte s_aBoolPar[] = { SCH_SRV_PARAMTYPE_BOOL, SCH_SRV_BOOLPARAM_NOT };

		m_pStrm1->Write(&s_aBoolPar, sizeof(s_aBoolPar));
		m_pStrm2->Write(&s_aBoolPar, sizeof(s_aBoolPar));
	}

	void AddStringParam(const CString &strVal)
	{
		byte abytePar[3];
		CStringA strEncoded;

		Str2MB(cfUTF8, &strEncoded, strVal);

		abytePar[0] = SCH_SRV_PARAMTYPE_STR;
		POKE_WORD(&abytePar[1], static_cast<uint16>(strEncoded.GetLength()));	// string length

		m_pStrm1->Write(&abytePar, sizeof(abytePar));
		m_pStrm1->Write(strEncoded, strEncoded.GetLength());
		m_pStrm2->Write(&abytePar, sizeof(abytePar));
		m_pStrm2->Write(strEncoded, strEncoded.GetLength());
	}

	void AddStringParam(byte byteTagID, const CString &strVal, CMemFile *pStrm)
	{
		byte abytePar[6];
		CStringA strEncoded;

		Str2MB(cfUTF8, &strEncoded, strVal);

		abytePar[0] = SCH_SRV_PARAMTYPE_TAGSTR;
		POKE_WORD(&abytePar[1], static_cast<uint16>(strEncoded.GetLength()));	// string length
		POKE_WORD(&abytePar[3], 1);			// meta tag ID length
		abytePar[5] = byteTagID;			// meta tag ID name

		pStrm->Write(&abytePar, 3);
		pStrm->Write(strEncoded, strEncoded.GetLength());
		pStrm->Write(&abytePar[3], 3);
	}

	void AddStringParam(byte byteTagID, const CString &strVal)
	{
		AddStringParam(byteTagID, strVal, m_pStrm1);
		AddStringParam(byteTagID, strVal, m_pStrm2);
	}

	void AddNumParam(byte byteTagID, byte byteOperator, uint64 qwVal, CMemFile *pStrm)
	{
		byte	abytePar[9 + 4];
		uint32	dwSz, dwVal;
		bool	b64BitValue = (qwVal > 0xFFFFFFFFui64);

		if (b64BitValue && (pStrm == m_pStrm2))
		{
			m_bContains64bitData = true;
			abytePar[0] = SCH_SRV_PARAMTYPE_TAGNUM64;
			POKE_QWORD(&abytePar[1], qwVal);	// numeric value
			abytePar[9] = byteOperator;			// comparison operator
			POKE_WORD(&abytePar[10], 1);		// meta tag ID length
			abytePar[12] = byteTagID;			// meta tag ID name
			dwSz = sizeof(abytePar);
		}
		else
		{
			if (b64BitValue)
				dwVal = 0xFFFFFFFF;
			else
				dwVal = static_cast<uint32>(qwVal);
			abytePar[0] = SCH_SRV_PARAMTYPE_TAGNUM;
			POKE_DWORD(&abytePar[1], dwVal);	// numeric value
			abytePar[5] = byteOperator;			// comparison operator
			POKE_WORD(&abytePar[6], 1);			// meta tag ID length
			abytePar[8] = byteTagID;			// meta tag ID name
			dwSz = sizeof(abytePar) - 4;
		}
		pStrm->Write(&abytePar, dwSz);
	}

	void AddNumParam(byte byteTagID, byte byteOperator, uint64 qwVal)
	{
		AddNumParam(byteTagID, byteOperator, qwVal, m_pStrm1);
		AddNumParam(byteTagID, byteOperator, qwVal, m_pStrm2);
	}

	void AddNumParamMin(byte byteTagID, uint64 qwVal)
	{
		AddNumParam(byteTagID, ED2K_SEARCH_OP_GREATER, qwVal - 1ui64);
	}

	void AddNumParamMax(byte byteTagID, uint64 qwVal)
	{
		AddNumParam(byteTagID, ED2K_SEARCH_OP_LESS, qwVal + 1ui64);
	}

	bool Contains64bitData() const		{ return m_bContains64bitData; }

private:
	CMemFile	*m_pStrm1;
	CMemFile	*m_pStrm2;
	bool		m_bContains64bitData;	//	2nd scream contains 64bit numbers
};

// CSearchDlg dialog

IMPLEMENT_DYNAMIC(CSearchDlg, CDialog)
CSearchDlg::CSearchDlg(CWnd* pParent /*=NULL*/)
: CResizableDialog(CSearchDlg::IDD, pParent)
{
	m_nSearchID = 0;
	m_bCancelled = true;
	m_bMoreEnabled = false;
	m_pGlobSearchPck = NULL;
	m_pGlobSearchPck2 = NULL;
	m_b64BitSearchPacket = false;
	m_pacSearchString = NULL;
	m_bGuardCBPrompt = false;
	m_nMoreRequestCount = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSearchDlg::~CSearchDlg()
{
	EMULE_TRY

	safe_delete(m_pGlobSearchPck);
	safe_delete(m_pGlobSearchPck2);
	if (m_pacSearchString != NULL)
	{
		m_pacSearchString->Unbind();
		m_pacSearchString->Release();
	}

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSearchDlg::OnInitDialog()
{
	static const uint16 s_auIconResID[] =
	{
		IDI_SMALLSERVER,	//EP_SEARCHMETHOD_SERV
		IDI_GLOBAL,			//EP_SEARCHMETHOD_GLOB
		IDI_SCH_WEB2,		//EP_SEARCHMETHOD_WEB1
		IDI_TRAYICON_GREY	//EP_SEARCHMETHOD_WEB2
	};

	EMULE_TRY

	CResizableDialog::OnInitDialog();
	g_App.m_pSearchList->SetOutputWnd(&m_ctlSearchList);
	m_ctlSearchList.Init(g_App.m_pSearchList);

	m_ctlGlobalSearchProgress.SetStep(1);
	global_search_timer = 0;
	m_bGlobalSearch = false;
	m_bGuardCBPrompt = false;

	m_ctrlSearchFrm.Init(IDI_NORMALSEARCH, &g_App.m_pMDlg->m_themeHelper);
	m_ctrlDirectDlFrm.Init(IDI_DIRECTDOWNLOAD, &g_App.m_pMDlg->m_themeHelper);
	oldSearchLstIcon = GetSearchLstStatic()->SetIcon(GetSearchLstIcon());

	m_BoxImageList.Create(16, 16, g_App.m_iDfltImageListColorFlags|ILC_MASK, ARRSIZE(s_auIconResID), 0);
	m_BoxImageList.SetBkColor(CLR_NONE);
	FillImgLstWith16x16Icons(&m_BoxImageList, s_auIconResID, ARRSIZE(s_auIconResID));

	AddAnchor(IDC_SEARCH_FRM, TOP_LEFT);
	AddAnchor(IDC_SNAME_LBL, TOP_LEFT);
	AddAnchor(IDC_SEARCHNAME, TOP_LEFT);
	AddAnchor(IDC_SEARCHNAME_NOT, TOP_LEFT);
	AddAnchor(IDC_STYPE_LBL, TOP_LEFT);
	AddAnchor(IDC_TYPESEARCH, TOP_LEFT);
	AddAnchor(IDC_SMETHOD_LBL, TOP_LEFT);
	AddAnchor(IDC_METHOD, TOP_LEFT);

	AddAnchor(IDC_DDOWN_FRM, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_ED2KLINK_LBL, TOP_LEFT);
	AddAnchor(IDC_ELINK, TOP_LEFT, TOP_RIGHT);

	AddAnchor(IDC_STARTS, TOP_RIGHT);
	AddAnchor(IDC_MORES, TOP_RIGHT);
	AddAnchor(IDC_CANCELS, TOP_RIGHT);
	AddAnchor(IDC_CLEARALL, TOP_RIGHT);

	AddAnchor(IDC_SEARCHLST_ICO, TOP_LEFT);
	AddAnchor(IDC_RESULTS_LBL, TOP_LEFT);
	AddAnchor(IDC_SEARCHLIST, TOP_LEFT, BOTTOM_RIGHT);

	AddAnchor(IDC_SDOWNLOAD, BOTTOM_LEFT);
	AddAnchor(IDC_PROGRESS1, BOTTOM_LEFT, BOTTOM_RIGHT);

	AddAnchor(m_ctlSearchTabs.m_hWnd,TOP_LEFT,TOP_RIGHT);
	AddAnchor(IDC_DOWNLOAD_ALL_ED2K, BOTTOM_LEFT);
	AddAnchor(IDC_STATIC_DLTOof, BOTTOM_LEFT);
	AddAnchor(IDC_CATTAB2, BOTTOM_LEFT, BOTTOM_RIGHT);

	ShowSearchTabs(SW_HIDE);

	m_pacSearchString = new CCustomAutoComplete();
	m_pacSearchString->AddRef();
	m_strLastClipBoard = _T("");

	CoInitialize(NULL);
	if (m_pacSearchString->Bind(::GetDlgItem(m_hWnd, IDC_SEARCHNAME), ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST))
	{
		if (g_App.m_pPrefs->GetKeepSearchHistory())
			m_pacSearchString->LoadList(g_App.m_pPrefs->GetConfigDir() + _T("\\") SEARCH_STRINGS_PROFILE);
	}

//	Max. Length of search expression
	((CEdit*)GetDlgItem(IDC_SEARCHNAME))->LimitText(512);
	((CEdit*)GetDlgItem(IDC_EDITSEARCHEXTENSION))->LimitText(32);

	Localize();

	OnEnChangeMethod();

	m_ttip.Create(this);
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 15000);
	m_ttip.SetDelayTime(TTDT_INITIAL, g_App.m_pPrefs->GetToolTipDelay()*1000);
	m_ttip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
	m_ttip.SetBehaviour(PPTOOLTIP_MULTIPLE_SHOW);
	m_ttip.SetNotify(m_hWnd);
	m_ttip.AddTool(&m_ctlSearchList, _T(""));

	return true;
	EMULE_CATCH
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SEARCHLIST, m_ctlSearchList);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctlGlobalSearchProgress);
	DDX_Control(pDX, IDC_METHOD, methodbox);
	DDX_Control(pDX, IDC_TYPESEARCH, Stypebox);
	DDX_Control(pDX, IDC_TAB1, m_ctlSearchTabs);
	DDX_Control(pDX, IDC_SEARCH_FRM, m_ctrlSearchFrm);
	DDX_Control(pDX, IDC_DDOWN_FRM, m_ctrlDirectDlFrm);
	DDX_Control(pDX, IDC_CATTAB2, m_ctrlCatTabs);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSearchDlg, CResizableDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SEARCH_RESET, OnBnClickedSearchReset)
	ON_BN_CLICKED(IDC_STARTS, OnBnClickedStarts)
	ON_BN_CLICKED(IDC_MORES, OnBnClickedMores)
	ON_BN_CLICKED(IDC_CANCELS, OnBnClickedCancels)
	ON_BN_CLICKED(IDC_CLEARALL, OnBnClickedClearall)
	ON_BN_CLICKED(IDC_SDOWNLOAD, OnBnClickedSdownload)
	ON_BN_CLICKED(IDC_DOWNLOAD_ALL_ED2K, OnBnClickedDownloadAllEd2k)
	ON_EN_KILLFOCUS(IDC_EDITSEARCHMIN, OnChangeMin)
	ON_EN_KILLFOCUS(IDC_EDITSEARCHMAX, OnChangeMax)
	ON_NOTIFY(NM_DBLCLK, IDC_SEARCHLIST, OnNMDblclkSearchlist)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnTcnSelchangeTab1)
	ON_EN_CHANGE(IDC_SEARCHNAME, OnEnChangeSearchname)
	ON_CBN_SELCHANGE(IDC_TYPESEARCH, OnEnChangeSearchname)
	ON_CBN_SELCHANGE(IDC_METHOD, OnEnChangeMethod)
	ON_MESSAGE(WM_CLOSETAB, OnCloseTab)
	ON_MESSAGE(WM_SHOWWINDOW, OnShowWindow)
	ON_WM_DESTROY()
END_MESSAGE_MAP()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSearchDlg message handlers
void CSearchDlg::OnChangeMin()
{
	OnChangeSize(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnChangeMax()
{
	OnChangeSize(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnChangeSize(bool bChangedMin)
{
	EMULE_TRY

	CString	strNum;
	int		iMin, iMax;

	GetDlgItemText(IDC_EDITSEARCHMIN, strNum);
	iMin = _tstoi(strNum);
	GetDlgItemText(IDC_EDITSEARCHMAX, strNum);
	iMax = _tstoi(strNum);
	if (iMin > 524287)	// 512 * 1024 - 1
	{
		iMin = 524287;
		SetDlgItemText(IDC_EDITSEARCHMIN, _T("524287"));
	}
	if (iMax > 524288)	// 512 * 1024
	{
		iMax = 524288;
		SetDlgItemText(IDC_EDITSEARCHMAX, _T("524288"));
	}
	if ((iMax <= iMin) && !strNum.IsEmpty())
	{
		SetDlgItemText((bChangedMin) ? IDC_EDITSEARCHMAX : IDC_EDITSEARCHMIN, _T(""));
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnBnClickedStarts()
{
	EMULE_TRY

	if (!GetDlgItem(IDC_STARTS)->IsWindowEnabled())
		return;

	m_ctlGlobalSearchProgress.SetPos(0);

	CString		strTemp;

//	ED2k-links
	GetDlgItemText(IDC_ELINK, strTemp);
	if (!strTemp.IsEmpty())
	{
		SetDlgItemText(IDC_ELINK, _T(""));

		int				iCatIdx = m_ctrlCatTabs.GetCurSel();
		EnumCategories	eCatID = (iCatIdx < 1 || iCatIdx > CCat::GetNumUserCats()) ? CAT_NONE : CCat::GetCatIDByUserIndex(iCatIdx);

		AddEd2kLinksToDownload(strTemp, eCatID);
	}
//	Search by name
	else if (GetDlgItem(IDC_SEARCHNAME)->GetWindowTextLength())
	{
		if (m_pacSearchString != NULL)
		{
			if (GetDlgItemText(IDC_SEARCHNAME, strTemp) && g_App.m_pPrefs->GetKeepSearchHistory())
				m_pacSearchString->AddItem(strTemp, 0);
		}
		switch (methodbox.GetCurSel())
		{
			case EP_SEARCHMETHOD_SERV:
			case EP_SEARCHMETHOD_GLOB:
				StartNewSearch();
				break;
			case EP_SEARCHMETHOD_WEB1:
			case EP_SEARCHMETHOD_WEB2:
				ShellOpenFile(CreateWebQuery());
				break;
			default:
				break;
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnTimer(UINT nIDEvent)
{
	EMULE_TRY

	CResizableDialog::OnTimer(nIDEvent);

#ifdef OLD_SOCKETS_ENABLED
	if (g_App.m_pServerConnect->IsConnected())
	{
		CServer		*pNextServer = g_App.m_pServerList->GetNextSearchServer();
		CServer		*pCurServer = g_App.m_pServerConnect->GetCurrentServer();

		if (pNextServer == g_App.m_pServerList->GetServerByAddress(pCurServer->GetAddress(), pCurServer->GetPort()))
			pNextServer = g_App.m_pServerList->GetNextSearchServer();

	//	If we got the next server and we haven't wrapped all the way around the server list...
		if (pNextServer != NULL && g_App.m_pServerList->GetServerCount() - 1 != m_uServerCount)
		{
			m_uServerCount++;
		//	Don't send search requests to the dead servers (dead static servers aren't removed automatically)
			if (pNextServer->GetFailedCount() >= g_App.m_pPrefs->GetDeadserverRetries())
				return;

			uint32	dwSrvUDPFlags = pNextServer->GetUDPFlags();
			Packet	*pSearchPck, *pExtSearchPck;

#define EPUDP_ADVFORM	(SRV_UDPFLG_LARGEFILES | SRV_UDPFLG_TYPETAGINTEGER | SRV_UDPFLG_EXT_GETFILES | SRV_UDPFLG_NEWTAGS)
			if ((dwSrvUDPFlags & EPUDP_ADVFORM) == EPUDP_ADVFORM)
			{
				static const uint32		dwTagCnt = 1;
				CMemFile	pckStrm(16);
				uint32		dwHdrSz;
				CWrTag		tagWr;

				pckStrm.Write(&dwTagCnt, sizeof(uint32));
				tagWr.WriteNewEd2kTag(CT_SERVER_UDPSEARCH_FLAGS, static_cast<uint32>(SRVCAP_UDP_NEWTAGS_LARGEFILES), pckStrm);

				pSearchPck = (m_pGlobSearchPck2 != NULL) ? m_pGlobSearchPck2 : m_pGlobSearchPck;

				dwHdrSz = static_cast<uint32>(pckStrm.GetLength());
				pExtSearchPck = new Packet(OP_GLOBSEARCHREQ3, pSearchPck->m_dwSize + dwHdrSz);
				pckStrm.SeekToBegin();
				pckStrm.Read(pExtSearchPck->m_pcBuffer, dwHdrSz);
				memcpy(pExtSearchPck->m_pcBuffer + dwHdrSz, pSearchPck->m_pcBuffer, pSearchPck->m_dwSize);

				g_App.m_pUploadQueue->AddUpDataOverheadServer(pExtSearchPck->m_dwSize);
				g_App.m_pServerConnect->SendUDPPacket(pExtSearchPck, pNextServer, true);
			}
			else if (!m_b64BitSearchPacket)	//	Skipped large file search request when the server doesn't support it
			{
			//	Send integer search type if it's among operands and server supports integer search type
				pSearchPck = ( (m_pGlobSearchPck2 != NULL) &&
					(dwSrvUDPFlags & SRV_UDPFLG_TYPETAGINTEGER) ) ? m_pGlobSearchPck2 : m_pGlobSearchPck;

			//	If the server supports extended Get Files...
				if (dwSrvUDPFlags & SRV_UDPFLG_EXT_GETFILES)
					pSearchPck->m_eOpcode = OP_GLOBSEARCHREQ2;
				else
					pSearchPck->m_eOpcode = OP_GLOBSEARCHREQ;

				g_App.m_pUploadQueue->AddUpDataOverheadServer(pSearchPck->m_dwSize);
				g_App.m_pServerConnect->SendUDPPacket(pSearchPck, pNextServer, false);
			}
#if 1
			else
			{
				g_App.m_pMDlg->AddLogLine( LOG_FL_DBG | LOG_RGB_CHOCOLATE, _T("Skipped large file UDP search request to server %s (%s, Flags %#x)"),
					pNextServer->GetFullIP(), pNextServer->GetVersion(), dwSrvUDPFlags );
			}
#endif
			m_ctlGlobalSearchProgress.StepIt();
		}
		else
		{
			OnBnClickedCancels();
		}
	}
	else
#endif //OLD_SOCKETS_ENABLED
		OnBnClickedCancels();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnBnClickedCancels()
{
	EMULE_TRY

	safe_delete(m_pGlobSearchPck);
	safe_delete(m_pGlobSearchPck2);
	m_b64BitSearchPacket = false;

	m_bCancelled = true;
	m_bGlobalSearch = false;
	if (global_search_timer)
	{
		KillTimer(global_search_timer);
		global_search_timer = 0;
		m_ctlGlobalSearchProgress.SetPos(0);
	}
	GetDlgItem(IDC_STARTS)->EnableWindow(true);
	GetDlgItem(IDC_MORES)->EnableWindow(false);
	GetDlgItem(IDC_CANCELS)->EnableWindow(false);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::LocalSearchEnd(uint16 count, bool bIsMoreResultsAvailable)
{
	EMULE_TRY

	if (!m_bCancelled)
	{
		if (count >= MAX_RESULTS)
		{
			OnBnClickedCancels();
		}
		else if (!m_bGlobalSearch)
		{
			GetDlgItem(IDC_STARTS)->EnableWindow(true);
			GetDlgItem(IDC_CANCELS)->EnableWindow(false);
			m_bCancelled = true; // DonGato: if we do not set this ENTER doesn't work
		}
		else
		{
			global_search_timer = SetTimer(1, 750, 0);
		}
	}

	if (bIsMoreResultsAvailable && m_nMoreRequestCount < MAX_MORE_SEARCH_REQ)
	{
		GetDlgItem(IDC_MORES)->EnableWindow(true);
		GetDlgItem(IDC_STARTS)->EnableWindow(true);
		GetDlgItem(IDC_CANCELS)->EnableWindow(false);
		m_bMoreEnabled = true;
	}
	else
	{
		GetDlgItem(IDC_MORES)->EnableWindow(false);
		if (!m_bGlobalSearch)
		{
			GetDlgItem(IDC_STARTS)->EnableWindow(true);
			GetDlgItem(IDC_CANCELS)->EnableWindow(false);
		}
		m_bMoreEnabled = false;
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::AddGlobalEd2kSearchResults(uint16 count)
{
	if (!m_bCancelled && (count >= MAX_RESULTS))
		OnBnClickedCancels();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnBnClickedSdownload()
{
	DownloadSelected(g_App.m_pPrefs->StartDownloadPaused());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSearchDlg::PreTranslateMessage(MSG *pMsg)
{
	EMULE_TRY

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			return FALSE;
		}
		else if ( pMsg->wParam == VK_DELETE
					&& GetAsyncKeyState(VK_CONTROL) < 0
					&& pMsg->hwnd == GetDlgItem(IDC_SEARCHNAME)->m_hWnd )
		{
			m_pacSearchString->Clear();
		}
		else if (pMsg->wParam == VK_RETURN)
		{
			if (pMsg->hwnd == GetDlgItem(IDC_SEARCHLIST)->m_hWnd)
			{
				OnBnClickedSdownload();
			}
			else if ( pMsg->hwnd == GetDlgItem(IDC_SEARCHNAME)->m_hWnd
						&& m_pacSearchString && m_pacSearchString->IsBound() )
			{
				CString strText;

				GetDlgItemText(IDC_SEARCHNAME, strText);
				if (!strText.IsEmpty())
				{
					SetDlgItemText(IDC_SEARCHNAME, _T("")); // this seems to be the only way to hide the dropdown
					SetDlgItemText(IDC_SEARCHNAME, strText);
				//	Remove selection
					((CEdit*)GetDlgItem(IDC_SEARCHNAME))->SetSel(~0ul);
				}
				if (m_bCancelled)
					OnBnClickedStarts();
			}
			else if ( m_bCancelled
						&& ( pMsg->hwnd == GetDlgItem(IDC_TYPESEARCH)->m_hWnd
							|| pMsg->hwnd == GetDlgItem(IDC_EDITSEARCHMIN)->m_hWnd
							|| pMsg->hwnd == GetDlgItem(IDC_EDITSEARCHMAX)->m_hWnd
							|| pMsg->hwnd == GetDlgItem(IDC_EDITSEARCHAVAILABILITY)->m_hWnd
							|| pMsg->hwnd == GetDlgItem(IDC_SEARCHNAME_NOT)->m_hWnd
							|| pMsg->hwnd == GetDlgItem(IDC_METHOD)->m_hWnd
							|| pMsg->hwnd == GetDlgItem(IDC_EDITSEARCHEXTENSION)->m_hWnd )
						|| pMsg->hwnd == GetDlgItem(IDC_ELINK)->m_hWnd )
			{
				OnBnClickedStarts();
			}
		}
	}

	if (g_App.m_pPrefs->GetToolTipDelay() != 0)
		m_ttip.RelayEvent(pMsg);

	return CResizableDialog::PreTranslateMessage(pMsg);

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSearchDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;

	switch(pNMHDR->code)
	{
		case UDM_TOOLTIP_DISPLAY:
		{
			NM_PPTOOLTIP_DISPLAY *pNotify = (NM_PPTOOLTIP_DISPLAY*)lParam;

			GetInfo4ToolTip(pNotify);
			return TRUE;
		}
		case UDM_TOOLTIP_POP:
		{
			m_ttip.Pop();
			return TRUE;
		}
	}

	return CResizableDialog::OnNotify(wParam, lParam, pResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY *pNotify)
{
	int	iControlId = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	if (iControlId == IDC_SEARCHLIST)
	{
		if (m_ctlSearchTabs.GetItemCount() < 1)
			return;

		int				iRating, iSel = GetItemUnderMouse(&m_ctlSearchList);

		if (iSel < 0 || iSel == 65535)
			return;

		CSearchFile		*pSearchFile = reinterpret_cast<CSearchFile*>(m_ctlSearchList.GetItemData(iSel));

		if (pSearchFile == NULL)
			return;

		CKnownFile		*pKnownFile = g_App.m_pSharedFilesList->GetFileByID(pSearchFile->GetFileHash());
		CString			strTemp, strInfo;

		if (pKnownFile == NULL)
			pKnownFile = g_App.m_pDownloadQueue->GetFileByID(pSearchFile->GetFileHash());

		if (pKnownFile != NULL)
		{
			if (pKnownFile->IsPartFile())
				strTemp.Format(_T("%s (%s)"), GetResString(IDS_TREE_DL), ((CPartFile*)pKnownFile)->GetPartFileStatus());
			else
				GetResString(&strTemp, IDS_SHAREDFILES);
		}
		else
		{
			pKnownFile = g_App.m_pKnownFilesList->FindKnownFileByID(pSearchFile->GetFileHash());
			GetResString(&strTemp, (pKnownFile != NULL) ? IDS_KNOWNFILES : IDS_UNKNOWN);
		}

		strInfo.Format(_T("<t=2><b>%s</b><br><t=2>%s<br><t=2>%s<br><hr=100%%><br><b>%s:<t></b>%s (%s %s)<br><b>%s:<t></b>%u<br><b>%s:<t></b>%u<br><b>%s:<t></b>%s"),
			pSearchFile->GetFileName(),
			pSearchFile->GetFileTypeString(),
			strTemp,
			GetResString(IDS_DL_SIZE), CastItoXBytes(pSearchFile->GetFileSize()), CastItoThousands(pSearchFile->GetFileSize()), GetResString(IDS_BYTES),
			GetResString(IDS_DL_SOURCES), pSearchFile->GetSourceCount(),
			GetResString(IDS_SF_COMPLETESRC), pSearchFile->GetCompleteSourceCount(), 
			GetResString(IDS_FILEHASH), HashToString(pSearchFile->GetFileHash()));

		g_App.m_pFakeCheck->GetFakeComment(HashToString(pSearchFile->GetFileHash()), pSearchFile->GetFileSize(), &strTemp);
		strTemp.Replace(_T("<"), _T("<<"));

		if (!strTemp.IsEmpty())
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_FAKE_CHECK_HEADER), strTemp);

		strTemp = m_ctlSearchList.GetItemText(iSel, SL_COLUMN_LASTSEENCOMPLETE);
		if (!strTemp.IsEmpty())
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_LASTSEENCOMPLETE), strTemp);

		strTemp = pSearchFile->GetSearchFileDir();
		if (!strTemp.IsEmpty())
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_SF_FOLDER), strTemp);

		if ((iRating = pSearchFile->GetSrvFileRating()) != PF_RATING_NONE)
		{
			double	dRating;
			uint32	dwUsers;

		//	Convert it to the original crazy format for proper string display
			if (iRating == PF_RATING_FAIR)
				iRating = PF_RATING_GOOD;
			else if (iRating == PF_RATING_GOOD)
				iRating = PF_RATING_FAIR;
			pSearchFile->GetSrvFileRatingEx(&dRating, &dwUsers);
			strInfo.AppendFormat( _T("<br><b>%s:<t></b>%s (%.2f) "), GetResString(IDS_TT_CMT_RATING),
				GetRatingString(static_cast<_EnumPartFileRating>(iRating)), dRating );
			strInfo.AppendFormat(GetResString(IDS_BY_USERS), dwUsers);
		}

		SHFILEINFO		shfi;

		memzero(&shfi, sizeof(shfi));
		SHGetFileInfo(pSearchFile->GetFileName(), FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
		pNotify->ti->hIcon = shfi.hIcon;
		pNotify->ti->sTooltip = strInfo;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnNMDblclkSearchlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	EMULE_TRY

	OnBnClickedSdownload();
	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString	CSearchDlg::CreateWebQuery()
{
	EMULE_TRY

	CString	strQuery, strToSearch;
	uint32	dwVal;

	GetDlgItemText(IDC_SEARCHNAME, strToSearch);
	switch (methodbox.GetCurSel())
	{
		case EP_SEARCHMETHOD_WEB1:
		{
		//	The only one required field is q
			strQuery.Format(_T("http://www.emugle.com/search.php?q=%s"), URLEncode(strToSearch));

			dwVal = Stypebox.GetCurSel();

			const TCHAR *pcType = NULL;

			switch (dwVal)
			{
				case EP_SCHTYPE_ARC:
					pcType = _T("Archive");
					break;
				case EP_SCHTYPE_AUDIO:
					pcType = _T("Audio");
					break;
				case EP_SCHTYPE_CDIMG:
					pcType = _T("CD+Image");
					break;
				case EP_SCHTYPE_PICS:
					pcType = _T("Image");
					break;
				case EP_SCHTYPE_VIDEO:
					pcType = _T("Video");
					break;
			}
			if (pcType != NULL)
				strQuery.AppendFormat(_T("&t=%s"), pcType);

			GetDlgItemText(IDC_SEARCHNAME_NOT, strToSearch);
			if (!strToSearch.IsEmpty())
				strQuery.AppendFormat(_T("&ex=%s"), URLEncode(strToSearch));	// multiple items allowed

			GetDlgItemText(IDC_EDITSEARCHMIN, strToSearch);
			if ((dwVal = _tstol(strToSearch)) != 0)
				strQuery.AppendFormat(_T("&mins=%u"), dwVal);

			GetDlgItemText(IDC_EDITSEARCHMAX, strToSearch);
			if ((dwVal = _tstol(strToSearch)) != 0)
				strQuery.AppendFormat(_T("&maxs=%u"), dwVal);

			GetDlgItemText(IDC_EDITSEARCHAVAILABILITY, strToSearch);
			if ((dwVal = _tstol(strToSearch)) != 0)
				strQuery.AppendFormat(_T("&avai=%u"), dwVal);
			break;
		}
		case EP_SEARCHMETHOD_WEB2:
			strQuery.Format(_T("http://www.filedonkey.com/fdsearch/index.php?pattern=%s"),
				URLEncode(strToSearch));
			GetDlgItemText(IDC_EDITSEARCHMIN, strToSearch);
			dwVal = _tstol(strToSearch);
			GetDlgItemText(IDC_EDITSEARCHMAX, strToSearch);
			strQuery.AppendFormat( _T("&min_size=%I64u&max_size=%I64u"),
				static_cast<uint64>(dwVal) * 1048576ui64,
				static_cast<uint64>(_tstol(strToSearch)) * 1048576ui64 );
			break;
	}
	return strQuery;

	EMULE_CATCH

	return _T("");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::DownloadSelected(bool bPaused)
{
	EMULE_TRY
	int index = -1;
	POSITION pos = m_ctlSearchList.GetFirstSelectedItemPosition();
	while(pos != NULL)
	{
		index = m_ctlSearchList.GetNextSelectedItem(pos);

		if(index > -1)
		{
			int					iCatIdx = m_ctrlCatTabs.GetCurSel();
			EnumCategories		eCatID = (iCatIdx < 1 || iCatIdx > CCat::GetNumUserCats()) ? CAT_NONE : CCat::GetCatIDByUserIndex(iCatIdx);
			CSearchFile			*pSearchFile = (CSearchFile*)m_ctlSearchList.GetItemData(index);

			g_App.m_pDownloadQueue->AddSearchToDownload(pSearchFile, eCatID, bPaused);

			if (pSearchFile->GetLastSeenCompleteValue() == 0x7FFFFFFF && pSearchFile->GetType() == SFT_SERVER)
			{
				g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_SOURCESWARNING, pSearchFile->GetFileName());
			}

			m_ctlSearchList.RedrawItems(index,index);
		}
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::Localize()
{
	static const int s_aiResTbl[][2] =
	{
		{ IDC_SNAME_LBL, IDS_SW_NAME },
		{ IDC_SNAME_LBL2, IDS_SCH_EXCEPT },
		{ IDC_STYPE_LBL, IDS_SW_TYPE },
		{ IDC_SMETHOD_LBL, IDS_SW_METHOD },
		{ IDC_ED2KLINK_LBL, IDS_SW_LINK },
		{ IDC_STARTS, IDS_SW_START },
		{ IDC_MORES, IDS_SW_MORE },
		{ IDC_CANCELS, IDS_CANCEL },
		{ IDC_CLEARALL, IDS_REMOVEALLSEARCH },
		{ IDC_RESULTS_LBL, IDS_SW_RESULT },
		{ IDC_SDOWNLOAD, IDS_SW_DOWNLOAD },
		{ IDC_SEARCHAVAIL, IDS_SEARCHAVAIL },
		{ IDC_SEARCHEXT, IDS_SEARCHEXTENSION_LBL },
		{ IDC_SEARCH_RESET, IDS_PW_RESET },
		{ IDC_DOWNLOAD_ALL_ED2K, IDS_DOWNLOAD_ALL_ED2K }
	};

	EMULE_TRY
	m_ctlSearchList.Localize();

	for (uint32 i = 0; i < ARRSIZE(s_aiResTbl); i++)
		SetDlgItemText(s_aiResTbl[i][0], GetResString(static_cast<UINT>(s_aiResTbl[i][1])));

	m_ctrlSearchFrm.SetText(GetResString(IDS_SEARCH_NOUN));
	m_ctrlDirectDlFrm.SetText(GetResString(IDS_SW_DIRECTDOWNLOAD));

	SetDlgItemText(IDC_SEARCHMINSIZE, GetResString(IDS_SEARCHMINSIZE) + _T(" (") + GetResString(IDS_MBYTES) + _T(')'));
	SetDlgItemText(IDC_SEARCHMAXSIZE, GetResString(IDS_SEARCHMAXSIZE) + _T(" (") + GetResString(IDS_MBYTES) + _T(')'));

	methodbox.ResetContent();
	methodbox.SetImageList(&m_BoxImageList);
	COMBOBOXEXITEM     cbi;
	cbi.mask = CBEIF_IMAGE | CBEIF_OVERLAY | CBEIF_SELECTEDIMAGE | CBEIF_TEXT;

	CString str[EP_SEARCHMETHOD_COUNT];

	GetResString(&str[EP_SEARCHMETHOD_SERV], IDS_PW_SERVER);
	GetResString(&str[EP_SEARCHMETHOD_GLOB], IDS_SW_GLOBAL);
	str[EP_SEARCHMETHOD_WEB1] = _T("eMugle (Web)");
	str[EP_SEARCHMETHOD_WEB2] = _T("FileDonkey (Web)");
	for (int i = 0; i < EP_SEARCHMETHOD_COUNT; i++)
	{
		cbi.iItem = i;
		cbi.pszText = const_cast<LPTSTR>(str[i].GetString());
		cbi.cchTextMax = str[i].GetLength();
		cbi.iImage = i;
		cbi.iSelectedImage = i;
		cbi.iOverlay = i;
		methodbox.InsertItem(&cbi);
	}
	methodbox.SetCurSel(g_App.m_pPrefs->GetSearchMethod());

	Stypebox.ResetContent();
	Stypebox.AddString(GetResString(IDS_SEARCH_ANY));
	Stypebox.AddString(GetResString(IDS_SEARCH_ARC)+_T(" (.zip .rar .ace...)"));
	Stypebox.AddString(GetResString(IDS_SEARCH_AUDIO)+_T(" (.mp3 .ogg .wav...)"));
	Stypebox.AddString(GetResString(IDS_SEARCH_CDIMG)+_T(" (.iso .bin .nrg...)"));
	Stypebox.AddString(GetResString(IDS_SEARCH_DOC)+_T(" (.doc .txt .pdf...)"));
	Stypebox.AddString(GetResString(IDS_SEARCH_PICS)+_T(" (.jpg .gif .png...)"));
	Stypebox.AddString(GetResString(IDS_SEARCH_PRG)+_T(" (.exe .zip .rar...)"));
	Stypebox.AddString(GetResString(IDS_SEARCH_VIDEO)+_T(" (.avi .mpg .ogm...)"));
	Stypebox.SetCurSel(EP_SCHTYPE_ANY);

	UpdateCatTabs(); //Localization Fix

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnBnClickedClearall()
{
	OnBnClickedCancels();
	DeleteAllSearches();
	SetDlgItemText(IDC_SEARCHNAME, _T(""));
	GetDlgItem(IDC_SEARCHNAME)->SetFocus();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::DoNewEd2kSearch(const CString &strSearch, const CString &strTypeText, uint64 qwMinSize, uint64 qwMaxSize,
	uint32 dwAvailability, const CString &strExtension, bool bDoGlobal, const CString &strSearchExclusion, uint16 nSearchID /*=0*/)
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
	if (!g_App.m_pServerConnect->IsConnected())
		return;
#endif //OLD_SOCKETS_ENABLED

//	KuSh: perhaps a messagebox or a message in the log/debuglog could be great
	if (strSearch.IsEmpty() && strExtension.IsEmpty())
	{
		return;
	}

	if (nSearchID == 0)
	{
		m_nSearchID++;
		g_App.m_pSearchList->NewSearch(&m_ctlSearchList, strTypeText, m_nSearchID);
	}
	else
	{
		m_nSearchID = nSearchID;
	}

//	Related files search is allowed only for server search type
	if (bDoGlobal && (_tcsnicmp(strSearch, _T("related:"), 8) == 0))
		bDoGlobal = false;

	m_bCancelled = false;
	m_nMoreRequestCount = 0;

	CString	strType;
	uint32	dwType = ED2KFT_ANY;

	if (GetResString(IDS_SEARCH_AUDIO) == strTypeText)
	{
		strType = _T("Audio");
		dwType = ED2KFT_AUDIO;
	}
	else if (GetResString(IDS_SEARCH_VIDEO) == strTypeText)
	{
		strType = _T("Video");
		dwType = ED2KFT_VIDEO;
	}
//	Archives, CD images and programs are treated the same by servers
	else if (GetResString(IDS_SEARCH_ARC) == strTypeText || GetResString(IDS_SEARCH_PRG) == strTypeText || GetResString(IDS_SEARCH_CDIMG) == strTypeText)
	{
		strType = _T("Pro");
		dwType = ED2KFT_PROGRAM;
	}
	else if (GetResString(IDS_SEARCH_PICS) == strTypeText)
	{
		strType = _T("Image");
		dwType = ED2KFT_IMAGE;
	}
	else if (GetResString(IDS_SEARCH_DOC) == strTypeText)
	{
		strType = _T("Doc");
		dwType = ED2KFT_DOCUMENT;
	}

//	The 2nd stream is used to create a new packet format,
//	as a packet can be sent to different servers we need to prepare 2 different ones
	CMemFile			pckStrm1(100), pckStrm2(100);
	CSearchExprTarget	Targ(&pckStrm1, &pckStrm2);

	uint32	dwTermsCount = 0;

	if (!strExtension.IsEmpty())
		dwTermsCount++;
	if (dwAvailability > 0)
		dwTermsCount++;
	if (qwMaxSize > 0)
		dwTermsCount++;
	if (qwMinSize > 0)
		dwTermsCount++;
	if (!strType.IsEmpty())
		dwTermsCount++;

	uint32			dwParameterCount = 0;

	//	New request creation method that can be processed by the servers with less load (lugdunummaster request)
	//
	//	input:      "a" AND min=1 AND max=2
	//	instead of: AND AND "a" min=1 max=2
	//	we use:     AND "a" AND min=1 max=2

	//	KuSh: all spaces should be parsed !!!
	if (!strSearch.IsEmpty())
	{
		int			iCurPos = 0;
		CString		strResToken = strSearch.Tokenize(_T(" "),iCurPos);
		CString		strOldResToken;

		while (!strResToken.IsEmpty())
		{
			strOldResToken = strResToken;
			strResToken = strSearch.Tokenize(_T(" "), iCurPos);
			if (!strResToken.IsEmpty() || dwTermsCount > 0)
				Targ.AddBooleanAND();
			Targ.AddStringParam(strOldResToken);
		}
	}
	if (!strType.IsEmpty())
	{
		if (++dwParameterCount < dwTermsCount)
			Targ.AddBooleanAND();
		Targ.AddStringParam(FT_FILETYPE, strType, &pckStrm1);
		Targ.AddNumParam(FT_FILETYPE, ED2K_SEARCH_OP_EQUAL, dwType, &pckStrm2);
	}
	if (qwMinSize > 0)
	{
		if (++dwParameterCount < dwTermsCount)
			Targ.AddBooleanAND();
		Targ.AddNumParamMin(FT_FILESIZE, qwMinSize);
	}
	if (qwMaxSize > 0)
	{
		if (++dwParameterCount < dwTermsCount)
			Targ.AddBooleanAND();
		Targ.AddNumParamMax(FT_FILESIZE, qwMaxSize);
	}
	if (dwAvailability > 0)
	{
		if (++dwParameterCount < dwTermsCount)
			Targ.AddBooleanAND();
		Targ.AddNumParamMin(FT_SOURCES, dwAvailability);
	}
	if (!strExtension.IsEmpty())
	{
		if (++dwParameterCount < dwTermsCount)
			Targ.AddBooleanAND();
		Targ.AddStringParam(FT_FILEFORMAT, strExtension);
	}
	ASSERT( dwParameterCount == dwTermsCount );

//	Send new packet format if it's different from the old one and
//	the server supports corresponding features
	uint32	dwSrvTCPFlags = g_App.m_pServerConnect->GetCurrentServer()->GetTCPFlags();
	bool	bHas64bData = Targ.Contains64bitData();
	bool	bNewPckFmt = ( ( bHas64bData &&
		((dwSrvTCPFlags & (SRV_TCPFLG_LARGEFILES | SRV_TCPFLG_TYPETAGINTEGER)) ==
			(SRV_TCPFLG_LARGEFILES | SRV_TCPFLG_TYPETAGINTEGER)) ) ||
		( !bHas64bData && (dwType != ED2KFT_ANY) && (dwSrvTCPFlags & SRV_TCPFLG_TYPETAGINTEGER)) );
	Packet	*pPacket = new Packet((bNewPckFmt) ? &pckStrm2 : &pckStrm1);

	pPacket->m_eOpcode = OP_SEARCHREQUEST;

	g_App.m_pUploadQueue->AddUpDataOverheadServer(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
	g_App.m_pServerConnect->SendPacket(pPacket, false);
#endif //OLD_SOCKETS_ENABLED

	if (bDoGlobal)
	{
	//	Get rid of any old global search packets
		safe_delete(m_pGlobSearchPck);
		safe_delete(m_pGlobSearchPck2);
		m_b64BitSearchPacket = bHas64bData;
	//	Keep packets to use them for UDP search
		if (bNewPckFmt)
		{
			m_pGlobSearchPck = new Packet(&pckStrm1);
			m_pGlobSearchPck2 = pPacket;
		}
		else
		{
			m_pGlobSearchPck = pPacket;
			if (m_b64BitSearchPacket || (dwType != ED2KFT_ANY))
				m_pGlobSearchPck2 = new Packet(&pckStrm2);
		}

		if (g_App.m_pPrefs->GetUseServerPriorities())
			g_App.m_pServerList->ResetSearchServerPos();
		m_uServerCount = 0;
		m_ctlGlobalSearchProgress.SetRange32(0, g_App.m_pServerList->GetServerCount() - 1);
		m_bGlobalSearch = true;
	}
	else
	{
		m_bGlobalSearch = false;
		delete pPacket;
	}

	SearchData		sd;

	sd.sNotSearch = strSearchExclusion;
	sd.bDocumentSearch = (GetResString(IDS_SEARCH_DOC) == strTypeText);
	m_SearchMap.SetAt(m_nSearchID, sd);
	CreateNewTab(strSearch, m_nSearchID);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::StartNewSearch()
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
//	If we're not connected to a server...
	if (!g_App.m_pServerConnect->IsConnected())
	{
		MessageBox(GetResString(IDS_ERR_NOTCONNECTED), GetResString(IDS_NOTCONNECTED), MB_ICONASTERISK);
	}
	else
#endif //OLD_SOCKETS_ENABLED
	{
		CString			strSearch, strExtension;

		GetDlgItemText(IDC_SEARCHNAME, strSearch);
		strSearch.Trim();
		GetDlgItemText(IDC_EDITSEARCHEXTENSION, strExtension);
		strExtension.Trim();
		if (!strExtension.IsEmpty() && (strExtension[0] == _T('.')))
		{
			strExtension = strExtension.Mid(1);
			SetDlgItemText(IDC_EDITSEARCHEXTENSION, strExtension);	// update GUI
		}

	//	KuSh: perhaps a messagebox or a message in the log/debuglog could be great
		if (strSearch.IsEmpty() && strExtension.IsEmpty())
		{
			return;
		}

		CString			strTypeText, strNum, strSearchExclusion;
		bool			bIsThisGlobal = (methodbox.GetCurSel() == EP_SEARCHMETHOD_GLOB);

		GetDlgItemText(IDC_TYPESEARCH, strTypeText);

		int				idx = strTypeText.Find(_T(" (."));

		if (idx > 0)
			strTypeText.Truncate(idx);

		GetDlgItemText(IDC_EDITSEARCHMIN, strNum);

		uint64	qwMinSize = static_cast<uint64>(_tstol(strNum)) * 0x100000ui64;

		GetDlgItemText(IDC_EDITSEARCHMAX, strNum);

		uint64	qwMaxSize = static_cast<uint64>(_tstol(strNum)) * 0x100000ui64;

	//	If no maximum size is specified or MaxSize < MinSize, don't send maximum size limit
		if (qwMaxSize < qwMinSize)
			qwMaxSize = 0;

		GetDlgItemText(IDC_EDITSEARCHAVAILABILITY, strNum);

		uint32	dwAvailability = _tstoi(strNum);

		GetDlgItemText(IDC_SEARCHNAME_NOT, strSearchExclusion);
		strSearchExclusion.Trim();
		strSearchExclusion.MakeLower();

		if (GetFocus() == GetDlgItem(IDC_STARTS))
			GetDlgItem(IDC_SEARCHNAME)->SetFocus();
		GetDlgItem(IDC_STARTS)->EnableWindow(false);
		GetDlgItem(IDC_MORES)->EnableWindow(false);
		GetDlgItem(IDC_CANCELS)->EnableWindow(true);

		DoNewEd2kSearch(strSearch, strTypeText, qwMinSize, qwMaxSize, dwAvailability,
						strExtension, bIsThisGlobal, strSearchExclusion);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::CreateNewTab(const CString &strSearchString, uint32 dwSearchID)
{
	EMULE_TRY
//	Add new tab
	TCITEM	tci;
	CString	strBuff(strSearchString);

	tci.mask = TCIF_PARAM | TCIF_TEXT | TCIF_IMAGE;
	tci.lParam = static_cast<LPARAM>(dwSearchID);
	tci.pszText = (strBuff.IsEmpty()) ? _T("-") : const_cast<LPTSTR>(strBuff.GetString());
	tci.iImage = 0;

	int	iItemIdx = m_ctlSearchTabs.InsertItem(m_ctlSearchTabs.GetItemCount(), &tci);

	if (!m_ctlSearchTabs.IsWindowVisible())
		ShowSearchTabs(SW_SHOW);
	m_ctlSearchTabs.SetCurSel(iItemIdx);
	m_ctlSearchList.ShowResults(dwSearchID);
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::DeleteSearch(uint32 nSearchID)
{
	EMULE_TRY

	int		i, iItems;
	TCITEM	item;

	item.mask = TCIF_PARAM;
	item.lParam = -1;

	for (i = 0, iItems = m_ctlSearchTabs.GetItemCount(); iItems != i; ++i)
	{
		m_ctlSearchTabs.GetItem(i,&item);
		if (static_cast<uint32>(item.lParam) == nSearchID)
			break;
	}

	if (item.lParam == -1 || item.lParam == 0 || static_cast<uint32>(item.lParam) != nSearchID)
		return;

	if ((!m_bCancelled) && (nSearchID == m_nSearchID))
		OnBnClickedCancels();

	g_App.m_pSearchList->RemoveResults(nSearchID);
	m_ctlSearchTabs.DeleteItem(i);

	if (--iItems)
	{
		if (i >= iItems)
			i = iItems - 1;
		m_ctlSearchTabs.SetCurSel(i);
		item.mask = TCIF_PARAM;
		m_ctlSearchTabs.GetItem(i,&item);
		m_ctlSearchList.ShowResults(item.lParam);
	}
	else
	{
		m_ctlSearchList.DeleteAllItems();
		ShowSearchTabs(SW_HIDE);
		m_ctlSearchList.NoTabs();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::DeleteAllSearches()
{
	EMULE_TRY

	g_App.m_pSearchList->Clear();
	m_ctlSearchList.DeleteAllItems();
	ShowSearchTabs(SW_HIDE);
	m_ctlSearchTabs.DeleteAllItems();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	EMULE_TRY

	const int iCurrentTab = m_ctlSearchTabs.GetCurSel();

	if (iCurrentTab == -1)
		return;

	TCITEM item;
	item.mask = TCIF_PARAM;
	m_ctlSearchTabs.GetItem(iCurrentTab, &item);
	m_ctlSearchList.ShowResults(item.lParam);
	GetDlgItem(IDC_MORES)->EnableWindow(false);

	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CSearchDlg::OnCloseTab(WPARAM wparam, LPARAM lparam)
{
	NOPRM(lparam);
	EMULE_TRY

	TCITEM item;
	item.mask = TCIF_PARAM;
	int i = (int)wparam;
	m_ctlSearchTabs.GetItem(i,&item);
	uint32	dwSearchID = static_cast<uint32>(item.lParam);

	if ((!m_bCancelled) && ((dwSearchID == m_nSearchID) || (m_ctlSearchTabs.GetItemCount() == 1)))
		OnBnClickedCancels();  // Cancel active search before closing active tab

	DeleteSearch(dwSearchID);
	GetDlgItem(IDC_MORES)->EnableWindow(false);

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnEnChangeSearchname()
{
	GetDlgItem(IDC_CANCELS)->EnableWindow(!m_bCancelled);
	GetDlgItem(IDC_MORES)->EnableWindow(false);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnBnClickedSearchReset()
{
	SetDlgItemText(IDC_EDITSEARCHAVAILABILITY, _T(""));
	SetDlgItemText(IDC_EDITSEARCHEXTENSION, _T(""));
	SetDlgItemText(IDC_EDITSEARCHMIN, _T(""));
	SetDlgItemText(IDC_EDITSEARCHMAX, _T(""));
	SetDlgItemText(IDC_SEARCHNAME, _T(""));
	SetDlgItemText(IDC_SEARCHNAME_NOT, _T(""));
	Stypebox.SetCurSel(EP_SCHTYPE_ANY);
	GetDlgItem(IDC_SEARCHNAME)->SetFocus();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnEnChangeMethod()
{
	static const uint16 s_auResID[] = {
		IDC_SEARCHNAME_NOT,	IDC_TYPESEARCH,	IDC_EDITSEARCHMIN,
		IDC_EDITSEARCHMAX,	IDC_EDITSEARCHAVAILABILITY,	IDC_EDITSEARCHEXTENSION
	};
	unsigned uiMask = EP_SCHCTRL_EXT | EP_SCHCTRL_AVAIL | EP_SCHCTRL_MAX | EP_SCHCTRL_MIN | EP_SCHCTRL_TYPE | EP_SCHCTRL_EXCL;
	int	iCurSel = methodbox.GetCurSel();

	g_App.m_pPrefs->SetSearchMethod(static_cast<byte>(iCurSel));
	switch (iCurSel)
	{
		case EP_SEARCHMETHOD_WEB2:
			uiMask = EP_SCHCTRL_MAX | EP_SCHCTRL_MIN;
			break;
		case EP_SEARCHMETHOD_WEB1:
			uiMask = EP_SCHCTRL_AVAIL | EP_SCHCTRL_MAX | EP_SCHCTRL_MIN | EP_SCHCTRL_TYPE | EP_SCHCTRL_EXCL;
			break;
	}
	for(unsigned ui = 0; ui < ARRSIZE(s_auResID); ui++, uiMask >>= 1)
		GetDlgItem(s_auResID[ui])->EnableWindow(static_cast<BOOL>(uiMask & 1));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::SearchClipBoard()
{
	EMULE_TRY

	if (m_bGuardCBPrompt)
		return;

	CString	strClipBoard;

	if (!g_App.CopyTextFromClipboard(&strClipBoard))
		return;

	strClipBoard.Trim();
	if (strClipBoard.IsEmpty() || (strClipBoard.Compare(m_strLastClipBoard) == 0))
		return;

	if (strClipBoard.Left(13).CompareNoCase(_T("ed2k://|file|")) == 0)
	{
		m_bGuardCBPrompt = true;

		if (AfxMessageBox(GetResString(IDS_ADDDOWNLOADSFROMCB) + strClipBoard, MB_YESNO | MB_SETFOREGROUND | MB_ICONQUESTION) == IDYES)
		{
			EnumCategories	eCatID = CAT_NONE;

			AddEd2kLinksToDownload(strClipBoard, eCatID);
		}
	}
	m_strLastClipBoard = strClipBoard;
	m_bGuardCBPrompt = false;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::AddEd2kLinksToDownload(CString strLink, EnumCategories eCatID)
{
	EMULE_TRY

	CString	resToken;
	int		curPos = 0;

	for (;;)
	{
		resToken = strLink.Tokenize(_T("\t\n\r"), curPos);
		resToken.Trim();
		if (resToken.IsEmpty())
			break;

		if (resToken.Right(1) != _T("/"))
			resToken += _T("/");

		try
		{
			CED2KLink	   *pLink = CED2KLink::CreateLinkFromUrl(resToken);

			_ASSERT( pLink !=0 );
			if (pLink->GetKind() == CED2KLink::kFile)
			{
				g_App.m_pDownloadQueue->AddFileLinkToDownload(pLink->GetFileLink(),eCatID);
			}
			else if (pLink->GetKind() == CED2KLink::kServer)
			{
				CString 			strName;
				CED2KServerLink* 	pSrvLink = pLink->GetServerLink();
				CServer* 			pSrv = new CServer(pSrvLink->GetPort(), pSrvLink->GetIP());

				pSrvLink->GetDefaultName(strName);
				pSrv->SetListName(strName);
				if (g_App.m_pPrefs->GetManuallyAddedServerHighPrio())
					pSrv->SetPreference(PR_HIGH);

				if (!g_App.m_pMDlg->m_wndServer.m_ctlServerList.AddServer(pSrv, true))
					delete pSrv;
				else
					g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR, IDS_SERVERADDED, pSrv->GetListName());
			}
			else
			{
				delete pLink;
				throw CString(_T("bad link"));
			}
			delete pLink;
		}
		catch(CString error)
		{
			OUTPUT_DEBUG_TRACE();
			g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_INVALIDLINK, error);
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnBnClickedDownloadAllEd2k()
{
	EMULE_TRY

	CString			strClipboard;

	if (!g_App.CopyTextFromClipboard(&strClipboard))
		return;

	const TCHAR		*text = strClipboard.GetString();
	CString			newlink;
	CString			msglinks;
	CTypedPtrList<CPtrList, CED2KLink*>	links;

	while (_tcsnextc(text))
	{
		if (_tcsnicmp(text, _T("ed2k://"), CSTRLEN(_T("ed2k://"))) == 0)
		{
			bool bLinkEnded = false;
			unsigned prevchar = 0;
			while (_tcsnextc(text))
			{
				if (_tcsnextc(text) == _T('\n'))
				{
					text = _tcsinc(text);
					continue;
				}
				else if (_tcsnextc(text) == _T('\r'))
				{
					text = _tcsinc(text);
					continue;
				}
				else if (_tcsncmp(text, _T("> "), CSTRLEN(_T("> "))) == 0)
				{
					text += _tcslen(_T("> "));
					continue;
				}
				else if (_tcsnextc(text) == _T('>'))
				{
					text = _tcsinc(text);
					continue;
				}
				newlink.AppendChar(static_cast<TCHAR>(_tcsnextc(text)));

				TCHAR nextchar = static_cast<TCHAR>(_tcsnextc(_tcsinc(text)));

				// Link ends with "|/"
				if ((_tcsnextc(text) == _T('/')) && ((nextchar != _T('|')) && (nextchar != _T('/'))))
					bLinkEnded = true;

				// Link ends with "|"
				if ( (_tcsnextc(text) == _T('|')) && (prevchar != _T('/'))
				    && (_tcschr(_T(" \""), nextchar) || _tcschr(_T("\r\n"), nextchar)) )
				{
					newlink.AppendChar(_T('/'));
					bLinkEnded = true;
				}

				if (bLinkEnded)
				{
					try
					{
						CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(newlink);
						_ASSERT( pLink != 0 );
						if ( pLink->GetKind() == CED2KLink::kFile)
						{
							msglinks += newlink;
							msglinks += _T('\n');
							links.AddTail(pLink);
						}
						else
						{
							delete pLink;
						}
					}
					catch(...)
					{
					}

					newlink = _T("");
					break;
				}
				prevchar = _tcsnextc(text);
				text = _tcsinc(text);
			}
		}
		text = _tcsinc(text);
	}
	if (links.IsEmpty())
	{
		MessageBox(GetResString(IDS_ED2KNOLINKFROMCLIPBOARD)); //"No ED2K links found in clipboard"
	}
	else
	{
		CString	msg;

		msg.Format(GetResString(IDS_ED2KCONFIRMFROMCLIPBOARD), msglinks); // "Add to downloads these links?\n%s", msglinks

		int		retvalue = MessageBox(msg, NULL, MB_YESNO);

		if (retvalue == IDYES)
		{
			int					iCatIdx = m_ctrlCatTabs.GetCurSel();
			EnumCategories		eCatID = (iCatIdx < 1 || iCatIdx > CCat::GetNumUserCats()) ? CAT_NONE : CCat::GetCatIDByUserIndex(iCatIdx);

			while (!links.IsEmpty())
			{
				g_App.m_pDownloadQueue->AddFileLinkToDownload(links.GetHead()->GetFileLink(),eCatID);
				delete links.RemoveHead();
			}
		}
		else
		{
			while (!links.IsEmpty())
				delete links.RemoveHead();
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnDestroy()
{
	EMULE_TRY

	CResizableDialog::OnDestroy();
	//Resources cleanup - [TwoBottle Mod]
	DestroyIcon(GetSearchLstStatic()->SetIcon(oldSearchLstIcon));
	m_BoxImageList.DeleteImageList();	// eklmn: bugfix(01): resource cleanup due to ImageLists recreation

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ultras - to set a focus in the search field
LRESULT CSearchDlg::OnShowWindow(WPARAM wparam, LPARAM lparam)
{
	const LRESULT	ret = DefWindowProc( WM_SHOWWINDOW, wparam, lparam );

	if( !ret && wparam && !lparam )
		GetDlgItem(IDC_SEARCHNAME)->SetFocus();

	return ret;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString	CSearchDlg::GetNotSearch(int nSearchId)
{
	SearchData	sd;

	if(m_SearchMap.Lookup(nSearchId, sd))
		return sd.sNotSearch;
	else
		return _T("");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSearchDlg::IsDocumentSearch(int nSearchId)
{
	SearchData	sd;

	if(m_SearchMap.Lookup(nSearchId, sd))
		return sd.bDocumentSearch;
	else
		return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::UpdateCatTabs()
{
	CString	strBuff;
	int		oldsel = m_ctrlCatTabs.GetCurSel();

	m_ctrlCatTabs.DeleteAllItems();
	m_ctrlCatTabs.InsertItem(0, ::GetResString(IDS_CAT_UNCATEGORIZED));
	for (int ix = 1; ix <= CCat::GetNumUserCats(); ix++)
	{
		strBuff = CCat::GetCatByUserIndex(ix)->GetTitle();
		strBuff.Replace(_T("&"), _T("&&"));
		m_ctrlCatTabs.InsertItem(ix, strBuff);
	}

	if (oldsel == -1 || oldsel >= m_ctrlCatTabs.GetItemCount())
		oldsel = 0;

	m_ctrlCatTabs.SetCurSel(oldsel);

	int		flag = (m_ctrlCatTabs.GetItemCount() > 1) ? SW_SHOW : SW_HIDE;

	GetDlgItem(IDC_CATTAB2)->ShowWindow(flag);
	GetDlgItem(IDC_STATIC_DLTOof)->ShowWindow(flag);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::KeepHistoryChanged()
{
	if (m_pacSearchString != NULL)
		m_pacSearchString->Clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::SaveSearchStrings()
{
	if (m_pacSearchString == NULL)
		return;
	m_pacSearchString->SaveList(g_App.m_pPrefs->GetConfigDir() + _T("\\") SEARCH_STRINGS_PROFILE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::OnBnClickedMores()
{
	EMULE_TRY

	if (!GetDlgItem(IDC_MORES)->IsWindowEnabled())
		return;

	if (!g_App.m_pServerConnect->IsConnected())
	{
		MessageBox(GetResString(IDS_ERR_NOTCONNECTED), GetResString(IDS_NOTCONNECTED), MB_ICONASTERISK);
		return;
	}

	if (GetFocus() == GetDlgItem(IDC_MORES))
		GetDlgItem(IDC_SEARCHNAME)->SetFocus();
	GetDlgItem(IDC_STARTS)->EnableWindow(false);
	GetDlgItem(IDC_MORES)->EnableWindow(false);
	GetDlgItem(IDC_CANCELS)->EnableWindow(true);

	Packet		*pPacket = new Packet(OP_EDONKEYPROT);

	pPacket->m_eOpcode = OP_QUERY_MORE_RESULT;

	g_App.m_pUploadQueue->AddUpDataOverheadServer(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
	g_App.m_pServerConnect->SendPacket(pPacket,true);
#endif //OLD_SOCKETS_ENABLED

	m_nMoreRequestCount++;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSearchDlg::ShowSearchTabs(int iCmdShow)
{
	EMULE_TRY

	WINDOWPLACEMENT	wpSearchListWinPos;
	WINDOWPLACEMENT	wpSearchTabsWinPos;

	m_ctlSearchTabs.GetWindowPlacement(&wpSearchTabsWinPos);
	m_ctlSearchList.GetWindowPlacement(&wpSearchListWinPos);

	if (iCmdShow == SW_SHOW)
	{
		wpSearchListWinPos.rcNormalPosition.top = wpSearchTabsWinPos.rcNormalPosition.bottom;
	}
	else
	{
		wpSearchListWinPos.rcNormalPosition.top = wpSearchTabsWinPos.rcNormalPosition.top;
	}

	m_ctlSearchTabs.ShowWindow(iCmdShow);
	RemoveAnchor(m_ctlSearchList);
	m_ctlSearchList.SetWindowPlacement(&wpSearchListWinPos);
	AddAnchor(m_ctlSearchList,TOP_LEFT,BOTTOM_RIGHT);
	GetDlgItem(IDC_CLEARALL)->ShowWindow(iCmdShow);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
