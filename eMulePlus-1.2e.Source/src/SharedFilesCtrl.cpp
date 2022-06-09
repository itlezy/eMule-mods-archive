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
#include "SharedFilesCtrl.h"
#include "ShellContextMenu.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "CommentDialog.h"
#include "TitleMenu.h"
#include "MemDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define sortcmp(a, b) ((a)==(b))?0: (((a)<(b))?-1:1)

#define GrayIt(gray, color) gray?(0x444444+0x010101*((GetRValue(color)*30+GetGValue(color)*59+GetBValue(color)*11)/0xFF)):color

IMPLEMENT_DYNAMIC(CSharedFilesCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSharedFilesCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
END_MESSAGE_MAP()

bool	CSharedFilesCtrl::m_sortParts = false;
uint32	CSharedFilesCtrl::m_p2p[5];

static const int s_aiDoubleLevelCols[][2] = {
	{ SFL_COLUMN_REQUESTS,		SFL_ALT_REQUESTS },
	{ SFL_COLUMN_ACCEPTED,		SFL_ALT_ACCEPTED },
	{ SFL_COLUMN_TRANSFERRED,	SFL_ALT_TRANSFERRED },
	{ SFL_COLUMN_PARTTRAFFIC,	SFL_ALT_PARTTRAFFIC },
	{ SFL_COLUMN_UPLOADS,		SFL_ALT_UPLOADS },
	{ SFL_COLUMN_COMPLETESRC,	SFL_ALT_COMPLETESRC }
};

CSharedFilesCtrl::CSharedFilesCtrl() : m_pSCM(NULL)
{
	EMULE_TRY

	m_statusWidth = -1;

	SetColoring(0);
	SetDisplay(0, false);

	SetGeneralPurposeFind(true);

	m_allYaKnow = false;

	memset(&m_bSortAscending, true, sizeof(m_bSortAscending));
	m_bSortAscending[SFL_COLUMN_FILESIZE] = false;
	m_bSortAscending[SFL_COLUMN_PRIORITY] = false;
	m_bSortAscending[SFL_COLUMN_REQUESTS] = false;
	m_bSortAscending[SFL_COLUMN_ACCEPTED] = false;
	m_bSortAscending[SFL_COLUMN_TRANSFERRED] = false;
	m_bSortAscending[SFL_COLUMN_PARTTRAFFIC] = false;
	m_bSortAscending[SFL_COLUMN_UPLOADS] = false;
	m_bSortAscending[SFL_COLUMN_COMPLETESRC] = false;

//	Fast priority converter, needed because priorities are messed up a little bit
	m_p2p[PR_VERYLOW]	= 0 << 2;
	m_p2p[PR_LOW]		= 1 << 2;
	m_p2p[PR_NORMAL]	= 2 << 2;
	m_p2p[PR_HIGH]		= 3 << 2;
	m_p2p[PR_RELEASE]	= 4 << 2;

	EMULE_CATCH
}

CSharedFilesCtrl::~CSharedFilesCtrl()
{
	delete m_pSCM;
}

void CSharedFilesCtrl::Init()
{
	static const uint16 s_auIconResID[] =
	{
		IDI_RATING_NO,
		IDI_RATING_FAKE,
		IDI_RATING_POOR,
		IDI_RATING_GOOD,
		IDI_RATING_FAIR,
		IDI_RATING_EXCELLENT,
		IDI_RATING_NONE
	};
	static const uint16 s_auColHdr[][2] =
	{
		{ LVCFMT_LEFT,  250 },	//SFL_COLUMN_FILENAME
		{ LVCFMT_RIGHT, 100 },	//SFL_COLUMN_FILESIZE
		{ LVCFMT_LEFT,   50 },	//SFL_COLUMN_TYPE
		{ LVCFMT_LEFT,   70 },	//SFL_COLUMN_PRIORITY
		{ LVCFMT_LEFT,  100 },	//SFL_COLUMN_PERMISSION
		{ LVCFMT_LEFT,  220 },	//SFL_COLUMN_FILEID
		{ LVCFMT_LEFT,  100 },	//SFL_COLUMN_REQUESTS
		{ LVCFMT_LEFT,  100 },	//SFL_COLUMN_ACCEPTED
		{ LVCFMT_LEFT,  120 },	//SFL_COLUMN_TRANSFERRED
		{ LVCFMT_LEFT,  120 },	//SFL_COLUMN_PARTTRAFFIC
		{ LVCFMT_LEFT,  120 },	//SFL_COLUMN_UPLOADS
		{ LVCFMT_LEFT,  100 },	//SFL_COLUMN_COMPLETESRC
		{ LVCFMT_LEFT,  150 }	//SFL_COLUMN_FOLDER
	};

	EMULE_TRY

	SetColoring(g_App.m_pPrefs->GetUpbarColor());
	SetDisplay(g_App.m_pPrefs->GetUpbarStyle(), false);

	//--- that seems to be the way to make taller item :) (i hate mfc... honestly i don't understand mfc :) ---
	CImageList ilDummyImageList;
	ilDummyImageList.Create(1, 17, ILC_COLOR, 1, 1);
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	ModifyStyle(LVS_SINGLESEL, 0);

	//--- in the official emule source the is a unlogical value at the end (for me :) ---
	for (unsigned ui = 0; ui < ARRSIZE(s_auColHdr); ui++)
		InsertColumn(ui, _T(""), static_cast<int>(s_auColHdr[ui][0]), static_cast<int>(s_auColHdr[ui][1]));

	m_imageList.Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, ARRSIZE(s_auIconResID), 0);
	m_imageList.SetBkColor(CLR_NONE);
	FillImgLstWith16x16Icons(&m_imageList, s_auIconResID, ARRSIZE(s_auIconResID));

	LoadSettings(CPreferences::TABLE_SHARED);

	if (g_App.m_pPrefs->DoUseSort())
	{
		SortInit(g_App.m_pPrefs->GetFileSortCol());
	}
	else
	{
		uint32		dwSortCode = g_App.m_pPrefs->GetColumnSortItem(CPreferences::TABLE_SHARED);

		dwSortCode |= g_App.m_pPrefs->GetColumnSortAscending(CPreferences::TABLE_SHARED) ? MLC_SORTASC : MLC_SORTDESC;

		SortInit(dwSortCode);
	}

	EMULE_CATCH
}

void CSharedFilesCtrl::SortInit(int iSortCode)
{
	int		iSortColumn = (iSortCode & MLC_COLUMNMASK);			// The sort column
	bool	bSortAscending = (iSortCode & MLC_SORTDESC) == 0;	// The sort order
	int		iSortAltFlag = (iSortCode & MLC_SORTALT);			// The alternate sort
	unsigned	ui;

	for (ui = 0; ui < ARRSIZE(s_aiDoubleLevelCols); ui++)
	{
		if (iSortColumn == s_aiDoubleLevelCols[ui][0])
			break;
	}
	if (ui == ARRSIZE(s_aiDoubleLevelCols))
		iSortAltFlag = 0;

	m_bSortAscending[iSortColumn] = bSortAscending;

	for (ui = 0; ui < ARRSIZE(s_aiDoubleLevelCols); ui++)
	{
		if (iSortColumn == s_aiDoubleLevelCols[ui][0])
		{
			m_bSortAscending[s_aiDoubleLevelCols[ui][1]] = (iSortAltFlag == 0);
			break;
		}
	}
	if (iSortAltFlag == 0)
		SetSortArrow(iSortColumn, bSortAscending);
	else
		SetSortArrow(iSortColumn, (bSortAscending) ? arrowDoubleUp : arrowDoubleDown);
	SortItems(SortProc, iSortCode);
}

void CSharedFilesCtrl::Localize()
{
	static const uint16 s_auResTbl[] =
	{
		IDS_DL_FILENAME,	//SFL_COLUMN_FILENAME
		IDS_DL_SIZE,		//SFL_COLUMN_FILESIZE
		IDS_TYPE,			//SFL_COLUMN_TYPE
		IDS_PRIORITY,		//SFL_COLUMN_PRIORITY
		IDS_PERMISSION,		//SFL_COLUMN_PERMISSION
		IDS_FILEHASH,		//SFL_COLUMN_FILEID
		IDS_SF_REQUESTS,	//SFL_COLUMN_REQUESTS
		IDS_SF_ACCEPTS,		//SFL_COLUMN_ACCEPTED
		IDS_SF_TRANSFERRED,	//SFL_COLUMN_TRANSFERRED
		IDS_SF_PARTTRAFFIC,	//SFL_COLUMN_PARTTRAFFIC
		IDS_SF_COLUPLOADS,	//SFL_COLUMN_UPLOADS
		IDS_SF_COMPLETESRC,	//SFL_COLUMN_COMPLETESRC
		IDS_SF_FOLDER		//SFL_COLUMN_FOLDER
	};

	EMULE_TRY

	if (GetSafeHwnd())
	{
		CHeaderCtrl	*pHeaderCtrl = GetHeaderCtrl();
		CString		strRes;
		HDITEM		hdi;

		hdi.mask = HDI_TEXT;

		for (unsigned ui = 0; ui < ARRSIZE(s_auResTbl); ui++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[ui]));
			hdi.pszText = const_cast<LPTSTR>(strRes.GetString());
			pHeaderCtrl->SetItem(static_cast<int>(ui), &hdi);
		}

	//	Don't update empty list, also it avoids unrequired update on startup,
	//	which can cause problems because this method is called inside dialog initialization
		if (GetItemCount())
		{
		// Localization
			if(m_allYaKnow)
				ShowKnownList();
			else
				ShowFileList(g_App.m_pSharedFilesList);
		}
		else
		{
			ShowFilesCount();
			g_App.m_pMDlg->m_wndSharedFiles.SetDlgItemText( IDC_BN_SWITCHALLKNOWN,
				GetResString((m_allYaKnow) ? IDS_SF_SHOWALLSF : IDS_SF_SHOWALLKF) );
		}
	}

	EMULE_CATCH
}

void CSharedFilesCtrl::ShowFileList(CSharedFileList* in_sflist)
{
	EMULE_TRY

	SetRedraw(FALSE);

	DeleteAllItems();
	CCKey bufKey;
	CKnownFile* cur_file;
	for(POSITION pos = in_sflist->m_mapSharedFiles.GetStartPosition(); pos != NULL;)
	{
		in_sflist->m_mapSharedFiles.GetNextAssoc(pos, bufKey, cur_file);
		ShowFile(cur_file, false);
	}

	m_allYaKnow = false;

	ShowFilesCount();
	g_App.m_pMDlg->m_wndSharedFiles.SetDlgItemText(IDC_BN_SWITCHALLKNOWN, GetResString(IDS_SF_SHOWALLKF));

	SetRedraw(TRUE);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFilesCtrl::ShowKnownList()
{
	EMULE_TRY

	CKnownFileList* list=g_App.m_pKnownFilesList;

	SetRedraw(FALSE);

	DeleteAllItems();

	uint32 records = list->GetCount();

	m_allYaKnow = true;

	for (uint32 i = 0; i < records; i++)
	{
		CKnownFile* pKnownFile = list->ElementAt(i);

		if (pKnownFile != NULL)
			ShowFile(pKnownFile, false);
	}
	ShowFilesCount();
	g_App.m_pMDlg->m_wndSharedFiles.SetDlgItemText(IDC_BN_SWITCHALLKNOWN, GetResString(IDS_SF_SHOWALLSF));

	SetRedraw(TRUE);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFilesCtrl::UpdateFile(CKnownFile *pSharedFile, uint32 itemnr, bool resort)
{
	if (!g_App.m_pMDlg->IsRunning() || (pSharedFile == NULL))
		return;

	EMULE_TRY

//	Disable list sorting, position will be updated either by 'resort' or by last SetItemText()
	SetSortProcedure(NULL);

	SetItemText(itemnr, SFL_COLUMN_FILESIZE, CastItoXBytes(pSharedFile->GetFileSize()));
	SetItemText(itemnr, SFL_COLUMN_TYPE, pSharedFile->GetFileTypeString());
	SetItemText(itemnr, SFL_COLUMN_PRIORITY, pSharedFile->GetKnownFilePriorityString());
	SetItemText(itemnr, SFL_COLUMN_PERMISSION, pSharedFile->GetPermissionString());
	SetItemText(itemnr, SFL_COLUMN_FILEID, HashToString(pSharedFile->GetFileHash()));

	const TCHAR	*pcTmp;
	POSITION	pos;
	CString	strBuffer;
	uint64	a = pSharedFile->statistic.GetRequests();
	uint64	b = pSharedFile->statistic.GetAllTimeRequests();
	bool	bPartFile;
	uint16	uPart;

	if (a != 0 || b != 0)
	{
		strBuffer.Format(_T("%I64u (%I64u)"), a, b);
		pcTmp = strBuffer.GetString();
	}
	else
		pcTmp = _T("");
	SetItemText(itemnr, SFL_COLUMN_REQUESTS, pcTmp);

	a = pSharedFile->statistic.GetAccepts();
	b = pSharedFile->statistic.GetAllTimeAccepts();
	if (a != 0 || b != 0)
	{
		strBuffer.Format(_T("%I64u (%I64u)"), a, b);
		pcTmp = strBuffer.GetString();
	}
	else
		pcTmp = _T("");
	SetItemText(itemnr, SFL_COLUMN_ACCEPTED, pcTmp);

	a = pSharedFile->statistic.GetTransferred();
	b = pSharedFile->statistic.GetAllTimeTransferred();
	if (a != 0 || b != 0)
	{
		strBuffer.Format(_T("%s (%s)"), CastItoXBytes(a), CastItoXBytes(b));
		pcTmp = strBuffer.GetString();
	}
	else
		pcTmp = _T("");
	SetItemText(itemnr, SFL_COLUMN_TRANSFERRED, pcTmp);

	//--- upload column text, or no text if 0 ---
	a = pSharedFile->GetFileSize();
	b = pSharedFile->statistic.GetAllTimeTransferred();

	double	dCompRel = pSharedFile->statistic.GetCompleteReleases();

	if (a != 0 && (dCompRel + b != 0.0))
	{
		strBuffer.Format(_T("%0.2f (%0.2f)"), dCompRel, (double)b/a);
		pcTmp = strBuffer.GetString();
	}
	else
		pcTmp = _T("");
	SetItemText(itemnr, SFL_COLUMN_UPLOADS, pcTmp);

	SetItemText(itemnr, SFL_COLUMN_FOLDER, pSharedFile->GetPath());

	//--- kids/parts open? ---
	sfl_itemdata* itemdataParent=(sfl_itemdata*)GetItemData(itemnr);
	if(itemdataParent->isOpen)
	{
	//	Update all parts
		bPartFile = pSharedFile->IsPartFile();
		pos = GetItemDataPos(itemnr + 1);
		for (uint16 part = 0; part < itemdataParent->parts; part++)
		{
			int				iSubItemIdx = itemnr + 1 + part;
			sfl_itemdata	*itemdata = reinterpret_cast<sfl_itemdata*>(GetItemDataByPos(pos, iSubItemIdx));

			uPart = itemdata->part;
			if (pSharedFile->IsPartShared(uPart))
				pcTmp = _T("");
			else
			{
				GetResString(&strBuffer, IDS_HIDDEN);
				pcTmp = strBuffer.GetString();
			}
			SetItemText(iSubItemIdx, SFL_COLUMN_PRIORITY, pcTmp);

			//--- column size (completed part?) ---
			if (bPartFile && !reinterpret_cast<CPartFile*>(pSharedFile)->IsPartComplete(uPart))
				pcTmp = _T("");
			else
				pcTmp = _T("*");
			SetItemText(iSubItemIdx, SFL_COLUMN_FILESIZE, pcTmp);

			//--- column accepted ---
			uint32	as = pSharedFile->statistic.GetPartAccepted(uPart, true);
			uint32	a = pSharedFile->statistic.GetPartAccepted(uPart, false);
			if (as != 0 || a != 0)
			{
				strBuffer.Format(_T("%u (%u)"), as, a);
				pcTmp = strBuffer.GetString();
			}
			else
				pcTmp = _T("");
			SetItemText(iSubItemIdx, SFL_COLUMN_ACCEPTED, pcTmp);

			//--- column transferred ---
			uint32 ts = pSharedFile->GetPartTraffic(uPart, true);
			uint32 t = pSharedFile->GetPartTraffic(uPart);
			if (ts != 0 || t != 0)
			{
				strBuffer.Format(_T("%s (%s)"), CastItoXBytes(ts), CastItoXBytes(t));
				pcTmp = strBuffer.GetString();
			}
			else
				pcTmp = _T("");
			SetItemText(iSubItemIdx, SFL_COLUMN_TRANSFERRED, pcTmp);

			//--- column complete releases ---
			double	dCompPartRel = pSharedFile->statistic.GetCompletePartReleases(uPart);
			if (ts != 0 || dCompPartRel != 0.0)
			{
				strBuffer.Format(_T("%0.2f (%0.2f)"), dCompPartRel, (double)t/pSharedFile->GetPartSize(uPart));
				pcTmp = strBuffer.GetString();
			}
			else
				pcTmp = _T("");
			SetItemText(iSubItemIdx, SFL_COLUMN_UPLOADS, pcTmp);
		}
	}

	if (pSharedFile->IsPartFile())
	{
		strBuffer.Format(_T("%u"), ((CPartFile*)pSharedFile)->GetCompleteSourcesCount());
	}
	else
	{
		uint16 nCompleteSourcesCountLo, nCompleteSourcesCountHi;
		pSharedFile->GetCompleteSourcesRange(&nCompleteSourcesCountLo, &nCompleteSourcesCountHi);
		if (nCompleteSourcesCountLo == 0)
		{
			if (nCompleteSourcesCountHi == 0)
				strBuffer = _T("");
			else
				strBuffer.Format(_T("< %u"), nCompleteSourcesCountHi);
		}
		else if (nCompleteSourcesCountLo == nCompleteSourcesCountHi)
			strBuffer.Format(_T("%u"), nCompleteSourcesCountLo);
		else
			strBuffer.Format(_T("%u - %u"), nCompleteSourcesCountLo, nCompleteSourcesCountHi);
	}
	SetSortProcedure(SortProc);	//	Enable list sorting
	SetItemText(itemnr, SFL_COLUMN_COMPLETESRC, strBuffer);

	//--- good time for a resort ---

	//	MOREVIT - Ok, a little kludgy but this should prevent resorts during extended selection
	//		causing the selection to be cleared.
	bool	isShift = GetAsyncKeyState(VK_SHIFT) < 0;

	if (resort && !isShift)
		SortItems(SortProc, m_dwParamSort);

	EMULE_CATCH
}

void CSharedFilesCtrl::AddFile(CKnownFile *file)
{
	ShowFile(file);
	ShowFilesCount();
}

void CSharedFilesCtrl::ShowFile(CKnownFile* file, bool resort)
{
	ShowFile(file, GetItemCount(), resort);
}

void CSharedFilesCtrl::ShowFile(CKnownFile* file, uint32 itemnr, bool resort)
{
	EMULE_TRY

	if(file == NULL)
		return;

	sfl_itemdata	*itemdata = new sfl_itemdata;

	itemdata->isFile = true;
	itemdata->isOpen = false;
	itemdata->knownFile = file;
	itemdata->part = 0;
	itemdata->parts = 0;

	itemnr=InsertItem(LVIF_TEXT|LVIF_PARAM, itemnr, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)itemdata);
	UpdateFile(file, itemnr, resort);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFilesCtrl::RemoveFile(CKnownFile *toRemove)
{
	EMULE_TRY

	if(toRemove == NULL)
		return;

	sfl_itemdata*	itemdata;
	for (int nItem = 0; nItem < GetItemCount(); nItem++)
	{
		itemdata = (sfl_itemdata*)GetItemData(nItem);
		if (itemdata->isFile && itemdata->knownFile == toRemove)
		{
			//--- delete all part-childs ---
			if(itemdata->isOpen)
			{
				for (uint16 part = 0; part < itemdata->parts; part++)
				{
					sfl_itemdata	*itemdataPart=(sfl_itemdata*)GetItemData(nItem+1);

					if (itemdataPart->isFile)
						break;
					DeleteItem(nItem+1);
					delete itemdataPart;
				}
			}

			DeleteItem(nItem);
			delete itemdata;
		}
	}
	ShowFilesCount();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSharedFilesCtrl::DeleteAllItems()
{
	BOOL			bResult = FALSE;
	sfl_itemdata	*pItem;
	POSITION		pos;
	int				iIdx;

	EMULE_TRY

	for (iIdx = 0, pos = GetItemDataHeadPos(); pos != NULL; iIdx++)
	{
		pItem = reinterpret_cast<sfl_itemdata*>(GetItemDataByPos(pos, iIdx));
		delete pItem;
	}
	bResult = CListCtrl::DeleteAllItems();

	EMULE_CATCH

	return bResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFilesCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
	NOPRM(pWnd);
	EMULE_TRY

	delete m_pSCM;
	m_pSCM = NULL;

	sfl_itemdata	*itemdata;
	UINT			uFlag;
	CTitleMenu		SharedFilesMenu;
	CMenu			menuPriority, PermMenu, ed2kMenu, ShellContextMenu, menuWeb;

	SharedFilesMenu.CreatePopupMenu();
	SharedFilesMenu.AddMenuTitle(GetResString((m_allYaKnow) ? IDS_KNOWNFILES : IDS_SHAREDFILES));

	int				iSel = GetSelectionMark();

	if (iSel != -1)
		itemdata = reinterpret_cast<sfl_itemdata*>(GetItemData(iSel));

	UINT		dwSelectedCnt = GetSelectedCount();
	bool		bJustOne = (dwSelectedCnt == 1);
	bool		bNone = (dwSelectedCnt == 0);

	if (!bNone)
	{
		if (itemdata->isFile)
		{
		//	Add priority switcher
			menuPriority.CreateMenu();

			bool		bTmpFlag = (bJustOne && !itemdata->knownFile->IsULAutoPrioritized());
			bool		bIsJumpStart = (itemdata->knownFile->GetJumpstartEnabled());

			menuPriority.AppendMenu( MF_STRING |
				((bTmpFlag && itemdata->knownFile->GetULPriority() == PR_VERYLOW) ? MF_CHECKED : MF_UNCHECKED),
				MP_PRIOVERYLOW, GetResString(IDS_PRIOVERYLOW) );
			menuPriority.AppendMenu( MF_STRING |
				((bTmpFlag && itemdata->knownFile->GetULPriority() == PR_LOW) ? MF_CHECKED : MF_UNCHECKED),
				MP_PRIOLOW, GetResString(IDS_PRIOLOW) );
			menuPriority.AppendMenu( MF_STRING |
				((bTmpFlag && itemdata->knownFile->GetULPriority() == PR_NORMAL) ? MF_CHECKED : MF_UNCHECKED),
				MP_PRIONORMAL, GetResString(IDS_PRIONORMAL) );
			menuPriority.AppendMenu( MF_STRING |
				((bTmpFlag && itemdata->knownFile->GetULPriority() == PR_HIGH) ? MF_CHECKED : MF_UNCHECKED),
				MP_PRIOHIGH, GetResString(IDS_PRIOHIGH) );
			menuPriority.AppendMenu( MF_STRING |
				((bTmpFlag && itemdata->knownFile->GetULPriority() == PR_RELEASE) ? MF_CHECKED : MF_UNCHECKED),
				MP_PRIORELEASE, GetResString(IDS_PRIORELEASE) );
			menuPriority.AppendMenu( MF_STRING |
				((bJustOne && itemdata->knownFile->IsULAutoPrioritized()) ? MF_CHECKED : MF_UNCHECKED),
				MP_PRIOAUTO, GetResString(IDS_PRIOAUTO) );

			if ( ( !itemdata->knownFile->IsPartFile() && !m_allYaKnow &&
				((itemdata->knownFile->GetFileSize() > PARTSIZE) || bIsJumpStart) ) ||	//don't allow to enable JumpStart for small files
				(bIsJumpStart && m_allYaKnow) )
			{
				menuPriority.AppendMenu(MF_SEPARATOR);
				menuPriority.AppendMenu(MF_STRING | (bIsJumpStart ? MF_CHECKED : MF_UNCHECKED), MP_JUMPSTART, _T("JumpStart"));
			}
			SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP|MF_ENABLED,(UINT_PTR)menuPriority.m_hMenu, GetResString(IDS_PRIORITY));

		//	add permission switcher
			PermMenu.CreateMenu();
			PermMenu.AppendMenu(MF_STRING | 
				((bJustOne && itemdata->knownFile->GetPermissions() == PERM_NOONE) ? MF_CHECKED : MF_UNCHECKED),
				MP_PERMNONE, GetResString(IDS_HIDDEN));
			PermMenu.AppendMenu(MF_STRING | 
				((bJustOne && itemdata->knownFile->GetPermissions() == PERM_FRIENDS) ? MF_CHECKED : MF_UNCHECKED),
				MP_PERMFRIENDS, GetResString(IDS_FSTATUS_FRIENDSONLY));
			PermMenu.AppendMenu(MF_STRING | 
				((bJustOne && itemdata->knownFile->GetPermissions() == PERM_ALL) ? MF_CHECKED : MF_UNCHECKED),
				MP_PERMALL, GetResString(IDS_FSTATUS_PUBLIC));

			SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)PermMenu.m_hMenu, GetResString(IDS_PERMISSION));
			SharedFilesMenu.AppendMenu(MF_SEPARATOR);

			if (m_allYaKnow == false)
			{
				ShellContextMenu.CreateMenu();
				uFlag = MF_STRING | MF_GRAYED;
				if (bJustOne && !itemdata->knownFile->IsPartFile())
				{
					uFlag &= ~MF_GRAYED;

					CString	strBuffer = ConcatFullPath(itemdata->knownFile->GetPath(), itemdata->knownFile->GetFileName());

					m_pSCM = new CShellContextMenu(m_hWnd, strBuffer);
					m_pSCM->SetMenu(&ShellContextMenu);
				}
				SharedFilesMenu.AppendMenu(uFlag | MF_POPUP, (UINT_PTR)ShellContextMenu.m_hMenu, GetResString(IDS_SHELLCONTEXT));
				SharedFilesMenu.AppendMenu( MF_STRING |
					((itemdata->knownFile->IsPartFile() || !(bJustOne)) ? MF_GRAYED : MF_ENABLED),
					MP_OPENFOLDER, GetStringFromShortcutCode(IDS_OPENFOLDER, SCUT_FILE_OPENDIR, SSP_TAB_PREFIX) );

				SharedFilesMenu.AppendMenu( MF_STRING |
					((itemdata->knownFile->IsPartFile() || !g_App.m_pPrefs->IsAVEnabled() || (g_App.m_pPrefs->GetAVPath().IsEmpty())) ? MF_GRAYED : MF_ENABLED),
					MP_AV_SCAN, GetResString(IDS_AV_SCAN));
			}
			SharedFilesMenu.AppendMenu( MF_STRING |
				((bJustOne) ? MF_ENABLED : MF_GRAYED) |
				((itemdata->knownFile->GetFileComment().IsEmpty()) ? MF_UNCHECKED : MF_CHECKED),
				MP_CMT, GetStringFromShortcutCode(IDS_EDIT_FILE_COMMENT, SCUT_FILE_EDITCOMMENTS, SSP_TAB_PREFIX) );
		}
		else
		{
			uFlag = MF_STRING | (itemdata->knownFile->IsPartFile() ? MF_GRAYED : MF_ENABLED);

			SharedFilesMenu.AppendMenu(uFlag, MP_SFL_PARTON, GetResString(IDS_SF_PARTON));
			SharedFilesMenu.AppendMenu(uFlag, MP_SFL_PARTHIDDEN, GetResString(IDS_SF_PARTHIDDEN));
		}

		SharedFilesMenu.AppendMenu(MF_SEPARATOR);
		SharedFilesMenu.AppendMenu(MF_STRING, MP_SFL_CLEARALLSTATS, GetResString(IDS_SF_RESETALLSTATS));
		SharedFilesMenu.AppendMenu(MF_STRING, MP_SFL_CLEARSTATS, GetResString(IDS_SF_RESETSESSTATS));

		SharedFilesMenu.AppendMenu(MF_SEPARATOR);

	//	Delete is possible only for a known file
		if (m_allYaKnow && itemdata->isFile)
		{
			SharedFilesMenu.AppendMenu(MF_STRING, MP_SFL_DELKNOWN, GetResString(IDS_SF_DELETE));
			SharedFilesMenu.AppendMenu(MF_SEPARATOR);
		}
	}

	if(m_allYaKnow)
	{
		SharedFilesMenu.AppendMenu(MF_STRING, MP_SFL_MERGEKNOWN, GetResString(IDS_SF_MERGE));
		SharedFilesMenu.AppendMenu(MF_SEPARATOR);
		SharedFilesMenu.AppendMenu(MF_STRING, MP_SFL_ALLYAKNOW, GetResString(IDS_SF_SHOWALLSF));
	}
	else
		SharedFilesMenu.AppendMenu(MF_STRING, MP_SFL_ALLYAKNOW, GetResString(IDS_SF_SHOWALLKF));

	if (!bNone)
	{
		SharedFilesMenu.AppendMenu(MF_SEPARATOR);

	//	ED2K link can't be obtained for a part only for a file
		if (itemdata->isFile)
		{
			ed2kMenu.CreateMenu();
			ed2kMenu.AppendMenu(MF_STRING, MP_GETED2KLINK, GetStringFromShortcutCode(IDS_DL_LINK1, SCUT_LINK, SSP_TAB_PREFIX));
			ed2kMenu.AppendMenu(MF_STRING, MP_GETHTMLED2KLINK, GetStringFromShortcutCode(IDS_DL_LINK2, SCUT_LINK_HTML, SSP_TAB_PREFIX));
			ed2kMenu.AppendMenu(MF_STRING, MP_GETSOURCEED2KLINK, GetStringFromShortcutCode(IDS_CREATESOURCELINK, SCUT_LINK_SOURCE, SSP_TAB_PREFIX));
			ed2kMenu.AppendMenu(MF_STRING, MP_GETHASH, GetStringFromShortcutCode(IDS_COPYHASH, SCUT_LINK_HASH, SSP_TAB_PREFIX));
			SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP|MF_ENABLED, (UINT_PTR)ed2kMenu.m_hMenu, GetResString(IDS_ED2KLINKFIX));
		}

		menuWeb.CreateMenu();
		uFlag = ((UpdateURLMenu(menuWeb) == 0) ? MF_GRAYED : MF_STRING) | (bJustOne ? MF_ENABLED : MF_GRAYED);
		SharedFilesMenu.AppendMenu(uFlag | MF_POPUP, (UINT_PTR)menuWeb.m_hMenu, GetResString(IDS_WEBSERVICES));
	}

	SharedFilesMenu.TrackPopupMenuEx(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this, NULL);

	if (m_pSCM != NULL)
		m_pSCM->CleanUp();

//	Menu objects are destroyed in their destructor

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSharedFilesCtrl::OnWndMsg(UINT iMessage, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	BOOL		bHandled = FALSE;

	EMULE_TRY

	switch (iMessage)
	{
		case WM_SFL_UPDATEITEM:
		{
			CKnownFile		*pKnownFile = reinterpret_cast<CKnownFile*>(lParam);
			bool			bResort = B2b(wParam);

			if (pKnownFile == NULL)	//	Special request to redraw all items without actual update
				RedrawItems(0, GetItemCount());
			else
				UpdateItem(pKnownFile, bResort);
			bHandled = TRUE;
			break;
		}
	}

	EMULE_CATCH

	if (!bHandled)
		bHandled = CMuleListCtrl::OnWndMsg(iMessage, wParam, lParam, pResult);

	return bHandled;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSharedFilesCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);
	EMULE_TRY

//	Commands which don't depend on selection
	switch (wParam)
	{
		case MP_SFL_ALLYAKNOW:
		{
			if(m_allYaKnow)
				ShowFileList(g_App.m_pSharedFilesList);
			else
				ShowKnownList();
			return true;
		}
		case MP_SFL_MERGEKNOWN:
		{
			SetRedraw(FALSE);
			g_App.m_pKnownFilesList->merge();
			ShowKnownList();
			SetRedraw(TRUE);
			return true;
		}
	}

	int iSel = GetSelectionMark();
	if (iSel == -1)
		return false;

	sfl_itemdata   *itemdata = (sfl_itemdata*)GetItemData(iSel);
	CKnownFile	   *file = itemdata->knownFile;

	if (wParam >= MP_WEBURL && wParam <= MP_WEBURL + 64)
	{
		RunURL(file, g_App.m_strWebServiceURLArray.GetAt(wParam - MP_WEBURL));
		return true;
	}

//	Multiple selections
	CString							strBuffer;
	UINT							selectedCount = GetSelectedCount();
	CTypedPtrList<CPtrList,CKnownFile*>	selectedList;
	POSITION						pos = GetFirstSelectedItemPosition();

	while (pos)
	{
		iSel = GetNextSelectedItem(pos);

		itemdata = reinterpret_cast<sfl_itemdata*>(GetItemData(iSel));

		//--- file selection? ---
		if (itemdata && itemdata->isFile)
		{
			selectedList.AddTail(itemdata->knownFile);
		}
	}

	switch (wParam)
	{
		case MP_GETED2KLINK:
			while (!selectedList.IsEmpty())
			{
				strBuffer += selectedList.RemoveHead()->CreateED2kLink();
				if (!selectedList.IsEmpty())
					strBuffer += _T("\r\n");
			}
			g_App.CopyTextToClipboard(strBuffer);
			break;

		case MP_GETHTMLED2KLINK:
			while (!selectedList.IsEmpty())
			{
				strBuffer += selectedList.RemoveHead()->CreateHTMLED2kLink();
				if(!selectedList.IsEmpty())
					strBuffer += _T("\r\n");
			}
			g_App.CopyTextToClipboard(strBuffer);
			break;

		case MP_GETSOURCEED2KLINK:
			while (!selectedList.IsEmpty())
			{
				CKnownFile	*pKnownFile = selectedList.RemoveHead();

				strBuffer += pKnownFile->GetSharedFile() ? pKnownFile->CreateED2kSourceLink() : pKnownFile->CreateED2kLink();
				if(!selectedList.IsEmpty())
					strBuffer += _T("\r\n");
			}
			g_App.CopyTextToClipboard(strBuffer);
			break;

		case MP_GETHASH:
			while (!selectedList.IsEmpty())
			{
				strBuffer += HashToString(selectedList.RemoveHead()->GetFileHash());
				if (!selectedList.IsEmpty())
					strBuffer += _T("\r\n");
			}
			g_App.CopyTextToClipboard(strBuffer);
			break;

		case MP_OPENFOLDER:
			ShellOpenFile(file->GetPath());
			break;

		case MP_AV_SCAN:
			strBuffer = g_App.m_pPrefs->GetAVParams();
			while (!selectedList.IsEmpty())
			{
				CKnownFile	*pKnownFile = selectedList.RemoveHead();

				if (!pKnownFile->IsPartFile())
				{
					strBuffer += _T(" \"");
					strBuffer += ConcatFullPath(pKnownFile->GetPath(), pKnownFile->GetFileName());
					strBuffer += _T('\"');
				}
			}
			ShellExecute(NULL, _T("open"), g_App.m_pPrefs->GetAVPath(), strBuffer, NULL, SW_SHOW);
			break;

		case MP_CMT:
		{
			if(selectedCount > 1)
				break;
			CCommentDialog dialog(file);

			if ((dialog.DoModal() == IDOK) && g_App.m_pPrefs->ShowRatingIcons())
				UpdateItem(file, false);	// Update file rating icon
			break;
		}
		case MP_SFL_DELKNOWN:
			while (!selectedList.IsEmpty())
			{
				CKnownFile		*pKnownFile = selectedList.RemoveHead();

			//	Try to clear file from download list if it's where as complete file not to crash
			//	during download list update, because file object will be destroyed now.
				g_App.m_pDownloadList->ClearCompleted(pKnownFile->GetFileHash());

				g_App.m_pSharedFilesList->RemoveFile(pKnownFile);
			}
			break;

		case MP_SFL_CLEARSTATS:
		case MP_SFL_CLEARALLSTATS:
		{
			//--- TODO: ask if really erase ---

			bool			all = (wParam == MP_SFL_CLEARALLSTATS);
			CKnownFile	   *lastfile = NULL;

			POSITION		pos = GetFirstSelectedItemPosition();

			while (pos)
			{
				iSel = GetNextSelectedItem(pos);

				itemdata = (sfl_itemdata*)GetItemData(iSel);
				file = itemdata->knownFile;

				//--- file selection? ---
				if (itemdata->isFile)
					itemdata->knownFile->statistic.resetStats(all);
				//--- part selection ---
				else
					itemdata->knownFile->statistic.resetPartTraffic(itemdata->part, all);

				if (file != lastfile)
				{
					if (lastfile)
						UpdateItem(lastfile, false);
					lastfile = file;
				}
			}

			if (lastfile)
				UpdateItem(lastfile, true);
			RedrawItems(0, GetItemCount());
			break;
		}
		case MP_PRIOVERYLOW:
		case MP_PRIOLOW:
		case MP_PRIONORMAL:
		case MP_PRIOHIGH:
		case MP_PRIORELEASE:
		case MP_PRIOAUTO:
		{
			POSITION	pos = GetFirstSelectedItemPosition();

			while (pos != NULL)
			{
				iSel = this->GetNextSelectedItem(pos);

				itemdata = (sfl_itemdata*)GetItemData(iSel);
				file = itemdata->knownFile;

				//--- is there a part in selection? ---
				if (itemdata->isFile)
				{
					file->SetAutoULPriority(false);

					switch (wParam)
					{
						default:
						case MP_PRIOVERYLOW:
							file->SetULPriority(PR_VERYLOW);
							break;

						case MP_PRIOLOW:
							file->SetULPriority(PR_LOW);
							break;

						case MP_PRIONORMAL:
							file->SetULPriority(PR_NORMAL);
							break;

						case MP_PRIOHIGH:
							file->SetULPriority(PR_HIGH);
							break;

						case MP_PRIORELEASE:
							file->SetULPriority(PR_RELEASE);
							break;

						case MP_PRIOAUTO:
							file->SetAutoULPriority(true);
							file->UpdateUploadAutoPriority();
							break;
					}
					SetItemText(iSel, SFL_COLUMN_PRIORITY, file->GetKnownFilePriorityString());
				}
			}
			break;
		}
		case MP_SFL_PARTON:
		case MP_SFL_PARTHIDDEN:
		{
			POSITION	pos = GetFirstSelectedItemPosition();
			bool			bIsItemsUpdated = false;

			while (pos)
			{
				iSel = GetNextSelectedItem(pos);

				itemdata = reinterpret_cast<sfl_itemdata*>(GetItemData(iSel));
				if (itemdata)
				{
					file = itemdata->knownFile;
					if (itemdata->isFile == false)
					{
						if (file->IsPartFile() == false)
						{
							switch(wParam)
							{
								case MP_SFL_PARTON:
									file->SharePart(itemdata->part);
									SetItemText(iSel, SFL_COLUMN_PRIORITY, _T(""));
									bIsItemsUpdated = true;
									break;

								case MP_SFL_PARTHIDDEN:
									file->UnsharePart(itemdata->part);
									GetResString(&strBuffer, IDS_HIDDEN);
									SetItemText(iSel, SFL_COLUMN_PRIORITY, strBuffer);
									bIsItemsUpdated = true;
									break;
							}
						}
					}
				}
			}

			if (bIsItemsUpdated)
				RedrawItems(0, GetItemCount());	// Update part traffic status bars
			break;
		}
		case MP_PERMNONE:
		case MP_PERMFRIENDS:
		case MP_PERMALL:
		{
			POSITION	pos = GetFirstSelectedItemPosition();

			while (pos != NULL)
			{
				iSel = this->GetNextSelectedItem(pos);

				itemdata = (sfl_itemdata*)GetItemData(iSel);
				if (itemdata->isFile)
				{
					file = itemdata->knownFile;
					switch (wParam)
					{
						case MP_PERMNONE:
							file->SetPermissions(PERM_NOONE);
							break;

						case MP_PERMFRIENDS:
							file->SetPermissions(PERM_FRIENDS);
							break;

						case MP_PERMALL:
							file->SetPermissions(PERM_ALL);
							break;
					}
					SetItemText(iSel, SFL_COLUMN_PERMISSION, file->GetPermissionString());
				}
			}
			break;
		}
		case MP_JUMPSTART:
		{
			BOOL alreadyasked = FALSE;
			BOOL alreadynotified = FALSE;
			INT msgyes = 0;
			POSITION pos = GetFirstSelectedItemPosition();
			while(pos != NULL)
			{
				iSel = this->GetNextSelectedItem(pos);

				itemdata=reinterpret_cast<sfl_itemdata*>(GetItemData(iSel));
				file=itemdata->knownFile;

				//--- is there a part in selection? ---
				if(itemdata->isFile && !itemdata->knownFile->IsPartFile())
				{
					if(file->GetJumpstartEnabled())
					{
						if(alreadyasked != TRUE)
						{
							GetResString(&strBuffer, IDS_JS_DISABLE);
							msgyes = AfxMessageBox(strBuffer, MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2);
							alreadyasked = TRUE;
						}

						if(msgyes == IDYES)
						{
							file->SetJumpstartEnabled(false);
							SetItemText(iSel, SFL_COLUMN_PRIORITY, file->GetKnownFilePriorityString());
						}
					}
					else if (file->GetFileSize() > PARTSIZE)	//don't allow to enable JumpStart for small files
					{
						if (file->IsJsComplete())
						{
							if (alreadynotified != TRUE)
							{
								GetResString(&strBuffer, IDS_JS_COMPLETE);
								AfxMessageBox(strBuffer, MB_OK | MB_ICONSTOP);
								alreadynotified = TRUE;
							}
						}
						else
						{
							file->SetJumpstartEnabled(true);
							SetItemText(iSel, SFL_COLUMN_PRIORITY, file->GetKnownFilePriorityString());
						}
					}
				}
			}
			break;
		}
		default:
		{
			if ((m_pSCM != NULL) && m_pSCM->IsMenuCommand(wParam))
				m_pSCM->InvokeCommand(wParam, file);
			break;
		}
	}

	return true;

	EMULE_CATCH

	return false;
}

void CSharedFilesCtrl::OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW	*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int			iFlags = 0, iSubItem = pNMListView->iSubItem;
	bool		bSortOrder = m_bSortAscending[iSubItem];

//	For the double level columns, toggle the Alt column flag when the arrow is facing up
	for (unsigned ui = 0; ui < ARRSIZE(s_aiDoubleLevelCols); ui++)
	{
		if (iSubItem == s_aiDoubleLevelCols[ui][0])
		{
			if ((static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem) && bSortOrder)
				m_bSortAscending[s_aiDoubleLevelCols[ui][1]] = !m_bSortAscending[s_aiDoubleLevelCols[ui][1]];
			iFlags = m_bSortAscending[s_aiDoubleLevelCols[ui][1]] ? 0 : MLC_SORTALT;
			break;
		}
	}

// Reverse sorting direction for the same column and keep the same if column was changed
	if (static_cast<int>(m_dwParamSort & MLC_COLUMNMASK) == iSubItem)
		m_bSortAscending[iSubItem] = bSortOrder = !bSortOrder;

	if (iFlags == 0)
		SetSortArrow(iSubItem, bSortOrder);
	else
		SetSortArrow(iSubItem, (bSortOrder) ? arrowDoubleUp : arrowDoubleDown);
	m_sortParts = (GetAsyncKeyState(VK_SHIFT) < 0);
	SortItems(SortProc, iSubItem | iFlags | ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));

	g_App.m_pPrefs->SetColumnSortItem(CPreferences::TABLE_SHARED, iSubItem | iFlags);	// Allow to save alternate criterion
	g_App.m_pPrefs->SetColumnSortAscending(CPreferences::TABLE_SHARED, bSortOrder);

	*pResult = 0;
}

int CSharedFilesCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (lParam1 == NULL || lParam2 == NULL)
		return 0;

	EMULE_TRY

	sfl_itemdata	*item1 = reinterpret_cast<sfl_itemdata*>(lParam1);
	sfl_itemdata	*item2 = reinterpret_cast<sfl_itemdata*>(lParam2);
	CKnownFile		*knownfile1 = item1->knownFile;
	CKnownFile		*knownfile2 = item2->knownFile;

	if (knownfile1 == NULL || knownfile2 == NULL)
		return 0;

	uint32	dwVal1, dwVal2;
	double	d1, d2;
	int		iCompare = 0;
	int		iSortColumn = (lParamSort & MLC_COLUMNMASK);
	int		iSortAltFlag = (lParamSort & MLC_SORTALT);
	int		iSortMod = ((lParamSort & MLC_SORTDESC) == 0) ? 1 : -1;

	//--- some parts to compare? and from same file ---
	if (item1->isFile == false && item2->isFile == false && item1->knownFile == item2->knownFile)
	{
		//--- shiftkey down ---
		if (m_sortParts && g_App.m_pPrefs->DoUsePT())
		{
			switch(iSortColumn)
			{
				case SFL_COLUMN_TRANSFERRED:
					if (iSortAltFlag == 0)
					{
						dwVal1 = knownfile1->GetPartTraffic(item1->part, true);
						dwVal2 = knownfile2->GetPartTraffic(item2->part, true);

						if ((dwVal1 != 0) || (dwVal2 != 0))
						{
							if (dwVal1 == 0)
								iCompare = -1;
							else if(dwVal2 == 0)
								iCompare = 1;
							else
								iCompare = sortcmp(dwVal1, dwVal2);
							break;
						}
					}

					dwVal1 = knownfile1->GetPartTraffic(item1->part);
					dwVal2 = knownfile2->GetPartTraffic(item2->part);

					if (dwVal1 == 0)
						iCompare = (dwVal2 == 0) ? (item1->part - item2->part) : -1;
					else if (dwVal2 == 0)
						iCompare = 1;
					else
						iCompare = sortcmp(dwVal1, dwVal2);
					break;

				case SFL_COLUMN_ACCEPTED:
					if (iSortAltFlag == 0)
					{
						dwVal1 = knownfile1->statistic.GetPartAccepted(item1->part, true);
						dwVal2 = knownfile2->statistic.GetPartAccepted(item2->part, true);

						if ((dwVal1 != 0) || (dwVal2 != 0))
						{
							if (dwVal1 == 0)
								iCompare = -1;
							else if(dwVal2 == 0)
								iCompare = 1;
							else
								iCompare = sortcmp(dwVal1, dwVal2);
							break;
						}
					}

					dwVal1 = knownfile1->statistic.GetPartAccepted(item1->part, false);
					dwVal2 = knownfile2->statistic.GetPartAccepted(item2->part, false);

					if (dwVal1 == 0)
						iCompare = (dwVal2 == 0) ? (item1->part - item2->part) : -1;
					else if (dwVal2 == 0)
						iCompare = 1;
					else
						iCompare = sortcmp(dwVal1, dwVal2);
					break;

				case SFL_COLUMN_PARTTRAFFIC:
				case SFL_COLUMN_UPLOADS:
					if (iSortAltFlag == 0)
					{
						iCompare = sortcmp(knownfile1->statistic.GetCompletePartReleases(item1->part),
									    knownfile2->statistic.GetCompletePartReleases(item2->part));
					}
					else
					{
						d1 = (double)knownfile1->GetPartTraffic(item1->part) / knownfile1->GetPartSize(item1->part);
						d2 = (double)knownfile2->GetPartTraffic(item2->part) / knownfile2->GetPartSize(item2->part);
						iCompare = sortcmp(d1, d2);
					}
					break;

				default:
					iCompare = item1->part - item2->part;
			}
		}
		else	//--- no shift key down, no sorting ---
		{
			iCompare = item1->part - item2->part;
			iSortMod = 1;		//	sort always in ascending order
		}
	}
	else	//--- some other compare ---
	{
		for (;;)
		{
			switch (iSortColumn)
			{
				case SFL_COLUMN_FILENAME:
					iCompare = knownfile1->CmpFileNames(knownfile2->GetFileName());
					break;

				case SFL_COLUMN_FILESIZE:
					iCompare = CompareInt64(knownfile1->GetFileSize(), knownfile2->GetFileSize());
					break;

				case SFL_COLUMN_TYPE:
					iCompare = knownfile1->CmpFileTypes(knownfile2->GetFileType());
					if (iCompare == 0)
					{
						iSortMod = 1;		//sort always in ascending order
						if ((iCompare = knownfile1->GetFileExtension().Compare(knownfile2->GetFileExtension())) == 0)
						{
							iSortColumn = SFL_COLUMN_FILENAME;
							continue;
						}
					}
					break;

				case SFL_COLUMN_PRIORITY:
					dwVal1 = m_p2p[knownfile1->GetULPriority()] | (knownfile1->GetJumpstartEnabled() ? 2 : 0) | (knownfile1->IsULAutoPrioritized() ? 1 : 0);
					dwVal2 = m_p2p[knownfile2->GetULPriority()] | (knownfile2->GetJumpstartEnabled() ? 2 : 0) | (knownfile2->IsULAutoPrioritized() ? 1 : 0);
					iCompare = dwVal1 - dwVal2;

					if (iCompare == 0)
					{
						iSortColumn = SFL_COLUMN_FILENAME;
						iSortMod = 1;		//sort always in ascending order
						continue;
					}
					break;

				case SFL_COLUMN_PERMISSION:
					iCompare = knownfile1->GetPermissions() - knownfile2->GetPermissions();
					break;

				case SFL_COLUMN_FILEID:
					iCompare = memcmp(knownfile1->GetFileHash(), knownfile2->GetFileHash(), 16);
					break;

				case SFL_COLUMN_REQUESTS:
					if (iSortAltFlag == 0)
					{
						iCompare = sortcmp(knownfile1->statistic.GetRequests(), knownfile2->statistic.GetRequests());
						if (iCompare != 0)
							break;
					}
					iCompare = sortcmp(knownfile1->statistic.GetAllTimeRequests(), knownfile2->statistic.GetAllTimeRequests());
					break;

				case SFL_COLUMN_ACCEPTED:
					if (iSortAltFlag == 0)
					{
						iCompare = sortcmp(knownfile1->statistic.GetAccepts(), knownfile2->statistic.GetAccepts());
						if (iCompare != 0)
							break;
					}
					iCompare = sortcmp(knownfile1->statistic.GetAllTimeAccepts(), knownfile2->statistic.GetAllTimeAccepts());
					break;

				case SFL_COLUMN_TRANSFERRED:
					if (iSortAltFlag == 0)
					{
						iCompare = sortcmp(knownfile1->statistic.GetTransferred(), knownfile2->statistic.GetTransferred());
						if (iCompare != 0)
							break;
					}
					iCompare = sortcmp(knownfile1->statistic.GetAllTimeTransferred(), knownfile2->statistic.GetAllTimeTransferred());
					break;

				case SFL_COLUMN_COMPLETESRC:
					d1 = CompleteSourcesCmpValue(knownfile1, (iSortAltFlag != 0));
					d2 = CompleteSourcesCmpValue(knownfile2, (iSortAltFlag != 0));
					iCompare = sortcmp(d1, d2);
					break;

				case SFL_COLUMN_PARTTRAFFIC:
				case SFL_COLUMN_UPLOADS:
					if (iSortAltFlag == 0)
					{
						iCompare = sortcmp(knownfile1->statistic.GetCompleteReleases(), knownfile2->statistic.GetCompleteReleases());
						if (iCompare != 0)
							break;
					}

					d1 = (double)knownfile1->statistic.GetAllTimeTransferred() / knownfile1->GetFileSize();
					d2 = (double)knownfile2->statistic.GetAllTimeTransferred() / knownfile2->GetFileSize();
					iCompare = sortcmp(d1, d2);
					break;

				case SFL_COLUMN_FOLDER:
					iCompare = _tcsicmp(knownfile1->GetPath(), knownfile2->GetPath());
					if (iCompare == 0)
					{
						iSortColumn = SFL_COLUMN_FILENAME;
						iSortMod = 1;		//sort always in ascending order
						continue;
					}
					break;
			}
			break;
		}
	}

	return iCompare * iSortMod;

	EMULE_CATCH

	return 0;
}

void CSharedFilesCtrl::UpdateItem(CKnownFile *toupdate, bool resort)
{
	if (!::IsWindow(m_hWnd) || (toupdate == NULL))
		return;

	EMULE_TRY

	sfl_itemdata	*pItem;
	int				iIdx;
	POSITION		pos;

//	Disable list sorting to preserve list positions
	SetSortProcedure(NULL);

	for (iIdx = 0, pos = GetItemDataHeadPos(); pos != NULL; iIdx++)
	{
		pItem = reinterpret_cast<sfl_itemdata*>(GetItemDataByPos(pos, iIdx));
		if ((pItem->knownFile == toupdate) && pItem->isFile)
		{
			SetSortProcedure(SortProc);	//	Enable list sorting

			UpdateFile(toupdate, iIdx, resort);
			Update(iIdx);
			return;
		}
	}
	SetSortProcedure(SortProc);	//	Enable list sorting

	EMULE_CATCH
}

void CSharedFilesCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	EMULE_TRY

	if (!lpDrawItemStruct->itemData)
		return;

	sfl_itemdata*	itemdata=(sfl_itemdata*)lpDrawItemStruct->itemData;
	CKnownFile*		file=itemdata->knownFile;
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	int iODC = odc->SaveDC();
	UINT			iCalcFlag = (DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
	COLORREF		crBk, crWinBk;

	if (IsRightToLeftLanguage())
		iCalcFlag |= DT_RTLREADING;

	crWinBk = crBk = GetBkColor();
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		crBk = (GetFocus() == this) ? m_crHighlight : m_crNoHighlight;

	CMemDC dc(odc, &lpDrawItemStruct->rcItem, crWinBk, crBk);
	int iDC = dc.SaveDC();
	CFont	*oldFont = dc->SelectObject(GetFont());

	if(m_allYaKnow)
	{
		if(file->GetSharedFile() == false)
			dc->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		else
			dc->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	}
	else
	{
		int	iSysColor = COLOR_WINDOWTEXT;

		if (file->IsPartFile())
			iSysColor = COLOR_GRAYTEXT;
		else if (file->GetJumpstartEnabled())
			iSysColor = COLOR_HIGHLIGHT;

		dc->SetTextColor(::GetSysColor(iSysColor));
	}

	RECT cur_rec;

	cur_rec = lpDrawItemStruct->rcItem;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	int iImage;

	//--- all 0.. columns ---
	for(int c = 0; c < iCount; c++)
	{
		//--- get real column, may its moved ---
		int cc = pHeaderCtrl->OrderToIndex(c);

		//--- if the column is hidden, dont do anything ---
		if(IsColumnHidden(cc))
			continue;

 		if(cc)
			cur_rec.left = cur_rec.right;
		cur_rec.right = cur_rec.left + CListCtrl::GetColumnWidth(cc);
		cur_rec.left += 4;

		//--- is it a file? ---
		if(itemdata->isFile)
		{
			switch(cc)
			{
				case SFL_COLUMN_FILENAME:

					// File Type
					if (g_App.m_pPrefs->ShowFileTypeIcon())
					{
						iImage = g_App.GetFileTypeSystemImageIdx(file->GetFileName());
						if (g_App.GetSystemImageList() != NULL)
							::ImageList_Draw(g_App.GetSystemImageList(), iImage, dc->GetSafeHdc(), cur_rec.left, cur_rec.top + 1, ILD_TRANSPARENT);
						cur_rec.left += 19;
					}
					// File Rating
					if (g_App.m_pPrefs->ShowRatingIcons())
					{
						int	iImgIdx = file->GetFileRating();

						if (file->GetFileComment().IsEmpty() && (iImgIdx == PF_RATING_NONE))
							iImgIdx = 6;
						m_imageList.Draw(dc, iImgIdx, CPoint(cur_rec.left - 4, cur_rec.top + 2), ILD_TRANSPARENT);

						cur_rec.left += 10;
					}

				case SFL_COLUMN_TYPE:
				case SFL_COLUMN_PRIORITY:
				case SFL_COLUMN_PERMISSION:
				case SFL_COLUMN_FILEID:
				case SFL_COLUMN_REQUESTS:
				case SFL_COLUMN_COMPLETESRC:
				case SFL_COLUMN_ACCEPTED:
				case SFL_COLUMN_TRANSFERRED:
				case SFL_COLUMN_UPLOADS:
				case SFL_COLUMN_FOLDER:
					dc->DrawText(GetItemText(lpDrawItemStruct->itemID, cc), &cur_rec, DT_LEFT | iCalcFlag);
					break;

				case SFL_COLUMN_FILESIZE:
					dc->DrawText(GetItemText(lpDrawItemStruct->itemID, cc), &cur_rec, DT_RIGHT | iCalcFlag);
					break;

				case SFL_COLUMN_PARTTRAFFIC:	//--- part traffic ---
					if (g_App.m_pPrefs->DoUsePT())
					{
						cur_rec.bottom--;
						cur_rec.top++;

						// added
						int iWidth = cur_rec.right - cur_rec.left-4;
						int iHeight = cur_rec.bottom - cur_rec.top;

						if (iWidth > 0)
						{
							//--- old bar draw ---
							cdcStatus.CreateCompatibleDC(&dc);

							//--- the bar is not initalized yet? ---
							if (status == (HBITMAP)NULL)
							{
								status.CreateCompatibleBitmap(&dc, iWidth, iHeight);
								m_statusWidth=iWidth;
							}
							else
							{
								//--- the we do it this way ---
								if(iWidth!=m_statusWidth)
								{
									status.DeleteObject();
									status.CreateCompatibleBitmap(&dc, iWidth, iHeight);
									m_statusWidth=iWidth;
								}
							}

							CBitmap *oldBitmap=cdcStatus.SelectObject(&status);

							RECT rec_status;
							rec_status.left = 0;
							rec_status.top = 0;
							rec_status.bottom = iHeight;
							rec_status.right = iWidth;
							if(DrawStatusBarFile(&cdcStatus, &rec_status, file->statistic))
								dc->BitBlt(cur_rec.left, cur_rec.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY);

							cdcStatus.SelectObject(oldBitmap);

							cdcStatus.DeleteDC();
						}

						//added end
						cur_rec.bottom++;
						cur_rec.top--;
					}
					break;
			}
		}

		//--- it is a part of a file ---
		else
		{
			switch(cc)
			{
				case SFL_COLUMN_FILENAME:
					dc->DrawText(GetItemText(lpDrawItemStruct->itemID, cc), &cur_rec, DT_RIGHT | iCalcFlag);
					break;

				case SFL_COLUMN_FILESIZE:
				case SFL_COLUMN_PRIORITY:
				case SFL_COLUMN_ACCEPTED:
				case SFL_COLUMN_TRANSFERRED:
				case SFL_COLUMN_UPLOADS:
				case SFL_COLUMN_COMPLETESRC:
					dc->DrawText(GetItemText(lpDrawItemStruct->itemID, cc), &cur_rec, /*DT_RIGHT*/DT_LEFT | iCalcFlag);
					break;

				case SFL_COLUMN_PARTTRAFFIC:	//--- part traffic ---
					if (g_App.m_pPrefs->DoUsePT())
					{
						cur_rec.bottom--;
						cur_rec.top++;

						int iWidth = cur_rec.right - cur_rec.left-4;
						int iHeight = cur_rec.bottom - cur_rec.top;

						if (iWidth > 0)
						{
							//--- old bar draw ---
							cdcStatus.CreateCompatibleDC(&dc);

							//--- the bar is not initalized yet? ---
							if (status == (HBITMAP)NULL)
							{
								status.CreateCompatibleBitmap(&dc, iWidth, iHeight);
								m_statusWidth=iWidth;
							}
							else
							{
								//--- the we do it this way ---
								if (iWidth != m_statusWidth)
								{
									status.DeleteObject();
									status.CreateCompatibleBitmap(&dc, iWidth, iHeight);
									m_statusWidth=iWidth;
								}
							}
							CBitmap *oldBitmap=cdcStatus.SelectObject(&status);

							RECT rec_status;
							rec_status.left = 0;
							rec_status.top = 0;
							rec_status.bottom = iHeight;
							rec_status.right = iWidth;
							if (DrawStatusBarPart(&cdcStatus, &rec_status, file->statistic, itemdata->part))
								dc->BitBlt(cur_rec.left, cur_rec.top, iWidth, iHeight, &cdcStatus, 0, 0, SRCCOPY);

							cdcStatus.SelectObject(oldBitmap);

							cdcStatus.DeleteDC();
						}

						cur_rec.bottom++;
						cur_rec.top--;
					}
					break;
			}
		}
	}

	//--- draw rectangle around selected item ---
	if(lpDrawItemStruct->itemState & ODS_FOCUS)
	{
		RECT	rOutline = lpDrawItemStruct->rcItem;
		CBrush	FrmBrush((GetFocus() == this) ? m_crFocusLine : m_crNoFocusLine);

		rOutline.left++;
		rOutline.right--;
		dc->FrameRect(&rOutline, &FrmBrush);
	}

	dc->SelectObject(oldFont);
	dc.RestoreDC(iDC);
	odc->RestoreDC(iODC);

	EMULE_CATCH
}

bool CSharedFilesCtrl::DrawStatusBarFile(CDC* dc, RECT* rect, const CFileStatistic& statistic)
{
	EMULE_TRY

	int	available, use;
	available=use=(rect->right-rect->left+1);
	uint64	fs = statistic.fileParent->GetFileSize();
	double	dFileSz = static_cast<double>(fs);
	if(fs < PARTSIZE)
		use = max(1, static_cast<uint32>(available * (dFileSz / PARTSZ32)));

	RECT		r = *rect;
	COLORREF	crOldBckColor;

	crOldBckColor = dc->SetBkColor(RGB(0x00, 0x00, 0x00));	// save background color
	if (available == use)
		dc->ExtTextOut(0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);	// fast fill with black
	else
	{
		r.right = r.left+use;
		dc->ExtTextOut(0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);	// fast fill with black

		r.left = r.right;
		r.right = available;
		dc->FillSolidRect(&r, RGB(0xCC, 0xCC, 0xCC));	// fast fill with grey
	}
	dc->SetBkColor(crOldBckColor);	// restore previuos background color

	double bpp = dFileSz / use;

	//--- traffigram? ---
	if(m_trafficgram)
	{
		if ((statistic.GetAllTimeTransferred() == 0) || (statistic.partTraffic == NULL))
			return true;

		TraffiGram(dc, rect, statistic, use, bpp);
	}
	//--- old boring bar style :) ---
	else
	{
		COLORREF	colr;
		uint32	t;	// traffic
		CPen	*pOldPen, pen;
		double	dTmp, dInvBpp = 1.0 / bpp;
		bool	boh;

		for(int x = 0; x < use; x++)
		{
			dTmp = x * bpp;
			boh = !statistic.fileParent->IsPartShared(static_cast<uint32>(dTmp / PARTSZ32));

			//--- draw alltime parttraffic bar? ---
			t = statistic.fileParent->GetTrafficPart(static_cast<uint64>(dTmp), static_cast<uint64>(dTmp + bpp));
			if (t || boh)
			{
				colr = (GetTrafficColor)(t * dInvBpp);
				pen.CreatePen(PS_SOLID, 0, GrayIt(boh, colr));
				pOldPen = dc->SelectObject(&pen);
				dc->MoveTo(x, 0);
				dc->LineTo(x, m_display_atpte);
				dc->SelectObject(pOldPen);	// set previous object to make pen pointer always correct in GDI object
				pen.DeleteObject();
			}

			if (m_display_atbte)	//	Draw alltime blocktraffic bar?
			{
				t = statistic.fileParent->GetTrafficBlock(static_cast<uint64>(dTmp), static_cast<uint64>(dTmp + bpp));
				if (t || boh)
				{
					colr = (GetTrafficColor)(t * dInvBpp);
					pen.CreatePen(PS_SOLID, 0, GrayIt(boh, colr));
					pOldPen = dc->SelectObject(&pen);
					dc->MoveTo(x, m_display_atbts);
					dc->LineTo(x, m_display_atbte);
					dc->SelectObject(pOldPen);	// set previous object to make pen pointer always correct in GDI object
					pen.DeleteObject();
				}

				if (m_display_sbte)	//	Draw session blocktraffic bar?
				{
					t = statistic.fileParent->GetTrafficBlock(static_cast<uint64>(dTmp), static_cast<uint64>(dTmp + bpp), true);
					if (t || boh)
					{
						colr = (GetTrafficColor)(t * dInvBpp);
						pen.CreatePen(PS_SOLID, 0, GrayIt(boh, colr));
						pOldPen = dc->SelectObject(&pen);
						dc->MoveTo(x, m_display_sbts);
						dc->LineTo(x, m_display_sbte);
						dc->SelectObject(pOldPen);	// set previous object to make pen pointer always correct in GDI object
						pen.DeleteObject();
					}
				}
			}
		}
	}

	return true;

	EMULE_CATCH

	return false;
}

bool CSharedFilesCtrl::DrawStatusBarPart(CDC* dc, RECT* rect, const CFileStatistic& statistic, uint16 part)
{
	EMULE_TRY

	uint32		ps = statistic.fileParent->GetPartSize(part);
	RECT		r = *rect;
	COLORREF	crOldBckColor;
	int			use = max(1, (int)((int)(rect->right - rect->left + 1) * (double)ps / PARTSZ32));

	crOldBckColor = dc->SetBkColor(RGB(0x00, 0x00, 0x00));	// save background color
	if (ps != PARTSZ32)
	{
		r.right = r.left + use;
		dc->ExtTextOut(0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);	// fast fill with black

		r.left = r.right;
		r.right = rect->right;
		dc->FillSolidRect(&r, RGB(0xCC, 0xCC, 0xCC));	// fast fill with grey
	}
	else
		dc->ExtTextOut(0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);	// fast fill with black

	if ((statistic.partTraffic != NULL) && (statistic.partTraffic[part] != 0))
	{
		uint64	qwStart = static_cast<uint64>(part) * PARTSIZE;
		double	bpp = (double)ps / use;

		//--- traffigram? ---
		if(m_trafficgram)
			TraffiGram(dc, rect, statistic, use, bpp, qwStart);
		else	//	old boring bar style :)
		{
			COLORREF	colr;
			CPen	*pOldPen, pen;
			bool	bHidden = !statistic.fileParent->IsPartShared(part);
			uint32	t;
			double	dTmp, dInvBpp = 1.0 / bpp;

			//--- draw alltime parttraffic bar? ---
			t = statistic.fileParent->GetPartTraffic(part);
			if (t || bHidden)
			{
				r.top = 0;
				r.bottom = m_display_atpte;
				r.left = rect->left;
				r.right = rect->left+use;
				colr = (GetTrafficColor)(static_cast<double>(t) / ps);
				dc->FillSolidRect(&r, GrayIt(bHidden, colr));
				r.bottom = rect->bottom;
			}

			if (m_display_atbte)	//	Draw alltime blocktraffic bar?
			{
				for(int x = 0; x < use; x++)
				{
					dTmp = x * bpp;
					t = statistic.fileParent->GetTrafficBlock(qwStart + static_cast<uint64>(dTmp), qwStart + static_cast<uint64>(dTmp + bpp));
					if (t || bHidden)
					{
						colr = (GetTrafficColor)(t * dInvBpp);
						pen.CreatePen(PS_SOLID, 0, GrayIt(bHidden, colr));
						pOldPen = dc->SelectObject(&pen);
						dc->MoveTo(x, m_display_atbts);
						dc->LineTo(x, m_display_atbte);
						dc->SelectObject(pOldPen);	// set previous object to make pen pointer always correct in GDI object
						pen.DeleteObject();
					}

					if (m_display_sbte)	//	Draw session blocktraffic bar?
					{
						t = statistic.fileParent->GetTrafficBlock(qwStart + static_cast<uint64>(dTmp), qwStart + static_cast<uint64>(dTmp + bpp), true);
						if (t || bHidden)
						{
							colr = (GetTrafficColor)(t * dInvBpp);
							pen.CreatePen(PS_SOLID, 0, GrayIt(bHidden, colr));
							pOldPen = dc->SelectObject(&pen);
							dc->MoveTo(x, m_display_sbts);
							dc->LineTo(x, m_display_sbte);
							dc->SelectObject(pOldPen);	// set previous object to make pen pointer always correct in GDI object
							pen.DeleteObject();
						}
					}
				}
			}
		}
	}
	dc->SetBkColor(crOldBckColor);	// restore previuos background color

	return true;

	EMULE_CATCH

	return false;
}

void CSharedFilesCtrl::OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	EMULE_TRY

	NMITEMACTIVATE *i=(NMITEMACTIVATE*)pNMHDR;

	if(i->iItem == -1)
		return;

	//--- if shift+dblclk hide all no traffic parts ---
	bool	onlyWithTraffic = (i->uKeyFlags & LVKF_SHIFT) != 0;
	bool	onlyWithSessionTraffic = ((i->uKeyFlags & (LVKF_SHIFT | LVKF_CONTROL)) == (LVKF_SHIFT | LVKF_CONTROL));

	//--- get data from double clicked item ---
	sfl_itemdata	*itemdataParent = (sfl_itemdata*)GetItemData(i->iItem);
	CKnownFile		*file = itemdataParent->knownFile;

	//--- is it a file? ---
	if (itemdataParent->isFile == false)
	{
		switch(i->iSubItem)
		{
			case SFL_COLUMN_PRIORITY:
			//	No blocking and hidding for partfiles
				if(file->IsPartFile())
					break;

				if (file->IsPartShared(itemdataParent->part))
					file->UnsharePart(itemdataParent->part);
				else
					file->SharePart(itemdataParent->part);

				UpdateItem(file, false);
				break;
		}
		return;
	}

	//--- toggle open ---
	itemdataParent->isOpen=!itemdataParent->isOpen;

	//--- is this file already open? ---
	//!!! ATTN i toggled isOpen already... you will not understand why I did this :) ---
	if(itemdataParent->isOpen==false)
	{
		SetRedraw(false);

		//--- delete all part-childs ---
		for(uint16 part = 0; part < itemdataParent->parts; part++)
		{
			sfl_itemdata *itemdata = (sfl_itemdata*)GetItemData(i->iItem + 1);
			if (itemdata->isFile)
				break;
			DeleteItem(i->iItem + 1);
			delete itemdata;
		}

		SetRedraw(true);
	}
	//--- it is not open ---
	else
	{
		if(g_App.m_pPrefs->DoUsePT() && file->GetPartCount()>1)
		{
			CString	buffer;
			uint16	parts=0;

			SetRedraw(false);

			for(uint16 part=0; part<file->GetPartCount(); part++)
			{
				//--- no no traffic items? ---
				if(onlyWithTraffic && file->GetPartTraffic(part, onlyWithSessionTraffic)==0)
					continue;

				parts++;

				sfl_itemdata	*itemdata = new sfl_itemdata;
				itemdata->isFile = false;
				itemdata->isOpen = false;
				itemdata->knownFile = file;
				itemdata->part = part;
				itemdata->parts = 0;

				buffer.Format(_T("%u"), part);
				InsertItem(LVIF_TEXT|LVIF_PARAM, i->iItem+parts, buffer, 0, 0, 0, (LPARAM)itemdata);
			}

			itemdataParent->parts=parts;

			//--- no parts added? then we can close it ---
			if(parts==0)
				itemdataParent->isOpen=false;

			//--- ok, thats why i toggled the isOpen before :) ---
			UpdateFile(file, i->iItem);

			SetRedraw(true);
		}
	}

	*pResult=0;

	EMULE_CATCH
}

void CSharedFilesCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	EMULE_TRY

	*pResult = 0;

	POSITION		posSelClient = GetFirstSelectedItemPosition();

	if (posSelClient == NULL)
		return;

	int				iItemIndex = GetNextSelectedItem(posSelClient);
	sfl_itemdata	*pItemDataParent = (sfl_itemdata*)GetItemData(iItemIndex);

	if (pItemDataParent == NULL)
		return;

	CKnownFile		*pKnownFile = pItemDataParent->knownFile;

	if (pKnownFile == NULL)
		return;

	g_App.m_pMDlg->m_wndSharedFiles.ShowDetails(pKnownFile);

	if (g_App.m_pPrefs->ShowRatingIcons())
	{
		DWORD		dwPos = GetMessagePos();
		CPoint		pt((int)(short)LOWORD(dwPos), (int)(short)HIWORD(dwPos));
		CRect		rRect;

		ScreenToClient(&pt);
		GetItemRect(iItemIndex, &rRect, LVIR_BOUNDS);
		if (rRect.PtInRect(pt))
		{
			if (g_App.m_pPrefs->ShowFileTypeIcon())
				rRect.left += 19;

			CRect		rTestedArea(rRect.left + 3, rRect.top + 2, rRect.left + 11, rRect.top + 16);

			if (rTestedArea.PtInRect(pt))
			{
				CCommentDialog dialog(pKnownFile);

				if (dialog.DoModal() == IDOK)
					UpdateItem(pKnownFile, false);	// Update file rating icon
			}
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFilesCtrl::OnNMDividerDoubleClick(NMHEADER *pHeader)
{
	int		iWidth, iColumn = pHeader->iItem;

	SetRedraw(false);
	CMuleListCtrl::OnNMDividerDoubleClick(pHeader);
	if ((iColumn == SFL_COLUMN_FILENAME) && (g_App.m_pPrefs->ShowRatingIcons() || g_App.m_pPrefs->ShowFileTypeIcon()))
	{
	//	Base class sets max width for a label, adjusting according to used icons
		iWidth = GetColumnWidth(iColumn);
		if (g_App.m_pPrefs->ShowFileTypeIcon())
			iWidth += 19;
		if (g_App.m_pPrefs->ShowRatingIcons())
			iWidth += 10;
		SetColumnWidth(iColumn, iWidth);
	}
	SetRedraw(true);
}

static COLORREF GetTrafficColor0(double f)	/// rainbow (exp)
{
	//--- COLORING MODE 1 --- RAINBOW ---
	//--- 0..1   black to red ---
	if(f<1)
		return RGB((BYTE)(255*f), 0, 0);

	//--- 1..3   red to yellow ---
	if(f<3)
		return RGB(255, (byte)(255*(f-1)/2.0), 0);

	//--- 3..6   yellow to green ---
	if(f<6)
		return RGB((byte)(255*((6-f)/3.0)), 255, 0);

	//--- 6..10   green to cyan ---
	if(f<10)
		return RGB(0, 255, (byte)(255*((f-6)/4.0)));

	//--- 10..15   cyan to blue ---
	if(f<15)
		return RGB(0, (byte)(255*((15-f)/5.0)), 255);

	//--- 15..21   blue to pink ---
	if(f<21)
		return RGB((byte)(255*((f-15)/6.0)), 0, 255);

	//--- 21..28   pink to white ---
	if(f<28)
		return RGB(255, (byte)(255*((28-f)/7.0)), 255);

	return RGB(255, 255, 255);
}

static COLORREF GetTrafficColor1(double f)	/// rainbow (lin)
{
	//--- COLORING MODE 1 --- RAINBOW ---
	//--- 0..1   black to red ---
	if(f<1)
		return RGB((BYTE)(255*f), 0, 0);

	//--- 1..2   red to yellow ---
	if(f<2)
		return RGB(255, (byte)(255*(f-1)), 0);

	//--- 2..3   yellow to green ---
	if(f<3)
		return RGB((byte)(255*((3-f))), 255, 0);

	//--- 3..4   green to cyan ---
	if(f<4)
		return RGB(0, 255, (byte)(255*((f-3))));

	//--- 4..5   cyan to blue ---
	if(f<5)
		return RGB(0, (byte)(255*((5-f))), 255);

	//--- 5..6   blue to pink ---
	if(f<6)
		return RGB((byte)(255*((f-5))), 0, 255);

	//--- 6..7   pink to white ---
	if(f<7)
		return RGB(255, (byte)(255*((7-f))), 255);

	return RGB(255, 255, 255);
}

static COLORREF GetTrafficColor2(double f)	/// blue
{
	//--- COLORING MODE 2 --- like in download ---
	return RGB(0, (210-(22*(f-1)) < 0)? 0:210-(22*(f-1)), 255);
}

static COLORREF GetTrafficColor3(double f)	/// pink
{
	//--- girlie mode ---
	return RGB(255, 0, (210-(22*(f-1)) < 0)? 0:210-(22*(f-1)));
}

void CSharedFilesCtrl::SetColoring(byte mode)
{
	switch (mode)
	{
		case 0:
		default:
			GetTrafficColor = GetTrafficColor0;
			break;

		case 1:
			GetTrafficColor = GetTrafficColor1;
			break;

		case 2:
			GetTrafficColor = GetTrafficColor2;
			break;

		case 3:
			GetTrafficColor = GetTrafficColor3;
			break;
	}
}

void CSharedFilesCtrl::SetDisplay(byte mode, bool redraw)
{
	EMULE_TRY

	m_display=mode;

	//--- should be 17 pixel high, i hope that never changes, but i try to save cpu where i can ---

	switch(m_display)
	{
	case 1:
		m_display_atpte	= 4;
		m_display_atbts	= 5;
		m_display_atbte	= 16;
		m_display_sbts	= 0;
		m_display_sbte	= 0;
		m_trafficgram	= false;
		break;

	case 2:
		m_display_atpte	= 4;
		m_display_atbts	= 5;
		m_display_atbte	= 11;
		m_display_sbts	= 12;
		m_display_sbte	= 16;
		m_trafficgram	= false;
		break;

	case 3:	// traffigram
		m_trafficgram=true;
		break;

	case 0:
	default:
		m_display_atpte	= 16;
		m_display_atbts	= 0;
		m_display_atbte	= 0;
		m_display_sbts	= 0;
		m_display_sbte	= 0;

		m_trafficgram	= false;
	}

	if(redraw)
		RedrawItems(0, GetItemCount());

	EMULE_CATCH
}

void CSharedFilesCtrl::TraffiGram(CDC* dc, RECT* rect, const CFileStatistic& statistic, int use, double &bpp, uint64 qwStart)
{
	EMULE_TRY

	std::vector<uint32>	adwBlocks(use);	// Cache traffic block statistics
	uint32	t, tt = 0;
	double	dTmp;

	for (int x = 0; x < use; x++)	//	Find highest value
	{
		dTmp = x * bpp;
		t = statistic.fileParent->GetTrafficBlock( qwStart + static_cast<uint64>(dTmp),
			qwStart + static_cast<uint64>(dTmp + bpp) );
		adwBlocks[x] = t;
		tt = max(tt, t);
	}

	int h=rect->bottom-rect->top+1;	// height
	int	lh=0;		// last height
	int	ch=0;		// current height
	uint32	lt=0;	// last traffic
	uint32	tph=tt/h;	// traffic per height pixel
	double	dInvBpp = 1.0 / bpp;
	double	dHdivTT = static_cast<double>(h) / static_cast<double>(tt);

	for (int x = 0; x < use; x++)
	{
		t = adwBlocks[x];
		ch = h - static_cast<int>(dHdivTT * static_cast<double>(t) + 0.5);
		if(t)
			dc->SetPixel(x, ch, (GetTrafficColor)(t * dInvBpp));

		//--- fill space between last and now ---
		if(x && lh!=ch)
		{
			int	d = (lh > ch) ? -1 : 1;
			int	iLmt = abs(lh - ch) / 2 + 1;

			for (int i = 1; i < iLmt; i++)
			{
				dc->SetPixel(x-1,	lh+i*d, (GetTrafficColor)((tt - tph * (lh+i*d)) * dInvBpp));
				dc->SetPixel(x,		ch-i*d, (GetTrafficColor)((tt - tph * (ch-i*d)) * dInvBpp));
			}
		}
		lh=ch;
		lt=t;
	}

	EMULE_CATCH
}

void CSharedFilesCtrl::ShowFilesCount()
{
	CString	strCounter;
	uint32	dwCnt;

	if (!m_allYaKnow)
	{
		GetResString(&strCounter, IDS_SHAREDFILES);
		if ((dwCnt = g_App.m_pSharedFilesList->GetWaitingForHashCount()) != 0)
			strCounter.AppendFormat(GetResString(IDS_STILLTOHASH), GetItemCount(), dwCnt);
		else
			strCounter.AppendFormat(_T(" (%u)"), GetItemCount());
	}
	else
		strCounter.Format(_T("%s (%u)"), GetResString(IDS_KNOWNFILES), GetItemCount());
	g_App.m_pMDlg->m_wndSharedFiles.SetDlgItemText(IDC_TRAFFIC_TEXT, strCounter);
}

double CSharedFilesCtrl::CompleteSourcesCmpValue(CKnownFile* item, bool second)
{
	uint16 nCountLo, nCountHi;
	bool bPartFile= item->IsPartFile();

	if (bPartFile)
		((CPartFile*)item)->GetCompleteSourcesRange(&nCountLo, &nCountHi);
	else
		item->GetCompleteSourcesRange(&nCountLo, &nCountHi);

	return ((nCountLo == 0) ? ((nCountHi == 0) ? ((bPartFile) ? 2.0 : 0.0) : (2.0 - (1.0 / static_cast<double>(nCountHi)))) : (second) ? static_cast<double>(nCountHi) + 3.0 - (1.0 / static_cast<double>(nCountLo)) : static_cast<double>(nCountLo) + 3.0 - (1.0 / static_cast<double>(nCountHi)));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFilesCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	EMULE_TRY
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (g_App.m_pMDlg->IsRunning())
	{
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, no *NOT* put any time consuming code here in.

		if (pDispInfo->item.mask & (LVIF_TEXT | LVIF_PARAM))
		{
			sfl_itemdata* pData = reinterpret_cast<sfl_itemdata*>(pDispInfo->item.lParam);
			if(pData)
			{
				const CKnownFile* pFile = pData->knownFile;
				if (pFile != NULL)
				{
					switch (pDispInfo->item.iSubItem)
					{
						case SFL_COLUMN_FILENAME:
							if (pDispInfo->item.cchTextMax > 0)
							{
								int		iSz = pDispInfo->item.cchTextMax;
								TCHAR	*pcBuf = pDispInfo->item.pszText;

								if (!m_allYaKnow && !pFile->GetPublishedED2K())
								{
								//	Mark shared files which should be published on the server
									*pcBuf++ = _T('*');
									iSz--;
								}
								_tcsncpy(pcBuf, pFile->GetFileName(), iSz);
								pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
							}
							break;
						default:
							// shouldn't happen
							pDispInfo->item.pszText[0] = _T('\0');
							break;
					}
				}
			}
		}
	}
	*pResult = 0;
	EMULE_CATCH
}

BOOL CSharedFilesCtrl::PreTranslateMessage(MSG *pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN))
	{
		int iMessage = 0;
		POSITION posSelClient = GetFirstSelectedItemPosition();

		if (posSelClient != NULL)
		{
			bool	bJustOne = (GetSelectedCount() == 1);
			short	nCode = GetCodeFromPressedKeys(pMsg);

			if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_FILE_OPENDIR))
			{
				sfl_itemdata *pItemData = reinterpret_cast<sfl_itemdata*>(GetItemData(GetNextSelectedItem(posSelClient)));

				if (!pItemData->knownFile->IsPartFile() && bJustOne)
					iMessage = MP_OPENFOLDER;
			}
			else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_FILE_EDITCOMMENTS))
			{
				if (bJustOne)
					iMessage = MP_CMT;
			}
			else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK))
				iMessage = MP_GETED2KLINK;
			else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK_HTML))
				iMessage = MP_GETHTMLED2KLINK;
			else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK_SOURCE))
				iMessage = MP_GETSOURCEED2KLINK;
			else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_LINK_HASH))
				iMessage = MP_GETHASH;

			if (iMessage > 0)
			{
				PostMessage(WM_COMMAND, iMessage);
				return TRUE;
			}
		}
	}
	else if (pMsg->message == WM_MBUTTONUP)
	{
		POINT point;

		::GetCursorPos(&point);

		CPoint p = point;

		ScreenToClient(&p);

		int it = HitTest(p);

		if (it == -1)
			return false;

		sfl_itemdata   *itemdata = reinterpret_cast<sfl_itemdata*>(GetItemData(it));
		CKnownFile	   *pFile = itemdata->knownFile;

		if (pFile != NULL)
		{
			CCommentDialog dialog(pFile);

			if ((dialog.DoModal() == IDOK) && g_App.m_pPrefs->ShowRatingIcons())
				UpdateItem(pFile, false);	// Update file rating icon
		}
		return true;
	}

	return CMuleListCtrl::PreTranslateMessage(pMsg);
}
