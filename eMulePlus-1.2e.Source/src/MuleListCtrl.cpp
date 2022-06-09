//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "memdc.h"
#include "MuleListCtrl.h"
#include "otherfunctions.h"
#include "TitleMenu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


#define MLC_BLEND(A, B, X) ((A + B * (X-1) + ((X+1)/2)) / X)

#define MLC_RGBBLEND(A, B, X) (                   \
	RGB(MLC_BLEND(GetRValue(A), GetRValue(B), X), \
	MLC_BLEND(GetGValue(A), GetGValue(B), X),     \
	MLC_BLEND(GetBValue(A), GetBValue(B), X))     \
)

#define MLC_DT_TEXT (DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS)

#define MLC_IDC_MENU	4875
#define MLC_IDC_UPDATE	(MLC_IDC_MENU - 1)

//a value that's not a multiple of 4 and uncommon
#define MLC_MAGIC 0xFEEBDEEF

//used for very slow assertions
//#define MLC_ASSERT(f)	ASSERT(f)
#define MLC_ASSERT(f)	((void)0)

#ifndef HDM_SETBITMAPMARGIN
#define HDM_SETBITMAPMARGIN	(HDM_FIRST + 20)
#define HDM_GETBITMAPMARGIN	(HDM_FIRST + 21)
#endif

BEGIN_MESSAGE_MAP(CMuleListCtrl, CListCtrl)
	ON_WM_DRAWITEM()
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

//////////////////////////////////
//	CMuleListCtrl

IMPLEMENT_DYNAMIC(CMuleListCtrl, CListCtrl)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMuleListCtrl::CMuleListCtrl(PFNLVCOMPARE pfnCompare, DWORD dwParamSort) : m_Params(50)
{
	m_SortProc = pfnCompare;
	m_dwParamSort = dwParamSort;

	m_bCustomDraw = false;
	m_iCurrentSortItem[0] = -1;
	m_iCurrentSortItem[1] = -1;
	m_iCurrentSortItem[2] = -1;
	m_iColumnsTracked = 0;
	m_aColumns = NULL;
	m_lRedrawCount = 0;
	m_bMovingItem = false;
	m_pHeaderCtrl = NULL;
	::InitializeCriticalSection(&m_csRedraw);

//	just in case
	m_crWindow = 0;
	m_crWindowText = 0;
	m_crWindowTextBk = 0;
	m_crHighlight = 0;
	m_crGlow = 0;
	m_crGlowText = 0;
	m_crDimmedText = 0;
	m_crFocusLine = 0;
	m_crNoHighlight = 0;
	m_crNoFocusLine = 0;
	m_bGeneralPurposeFind = false;
	m_iFindDirection = 1;
	m_iFindColumn = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMuleListCtrl::~CMuleListCtrl()
{
	if (m_aColumns != NULL)
		delete[] m_aColumns;

	::DeleteCriticalSection(&m_csRedraw);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMuleListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	NOPRM(lParam1); NOPRM(lParam2); NOPRM(lParamSort);

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::PreSubclassWindow()
{
	SetColors();
	CListCtrl::PreSubclassWindow();
	ModifyStyle(LVS_LIST | LVS_ICON | LVS_SMALLICON, LVS_REPORT | LVS_SINGLESEL);
#ifdef _UNICODE
//	Win9x: Explicitly set to Unicode to receive Unicode notifications.
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);
#endif
	SetExtendedStyle(LVS_EX_HEADERDRAGDROP);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMuleListCtrl::IndexToOrder(CHeaderCtrl* pHeader, int iIndex)
{
	int			iCount = pHeader->GetItemCount();
	int			*piArray = new int[iCount];

	Header_GetOrderArray(pHeader->m_hWnd, iCount, piArray);
	for (int i = 0; i < iCount; i++)
	{
		if (piArray[i] == iIndex)
		{
			delete[] piArray;
			return i;
		}
	}
	delete[] piArray;
	return -1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::HideColumn(int iColumn)
{
	CHeaderCtrl			*pHeaderCtrl = GetHeaderCtrl();
	int					iCount = pHeaderCtrl->GetItemCount();

	if (iColumn < 1 || iColumn >= iCount || m_aColumns[iColumn].bHidden)
		return;

//	stop it from redrawing
	SetRedraw(FALSE);

//	shrink width to 0
	HDITEM		item;

	item.mask = HDI_WIDTH;
	pHeaderCtrl->GetItem(iColumn, &item);
	m_aColumns[iColumn].iWidth = item.cxy;
	item.cxy = 0;
	pHeaderCtrl->SetItem(iColumn, &item);

//	move to front of list
	int			*piArray = new INT[m_iColumnsTracked];

	pHeaderCtrl->GetOrderArray(piArray, m_iColumnsTracked);

	int			iFrom = m_aColumns[iColumn].iLocation;

	for (int i = 0; i < m_iColumnsTracked; i++)
		if (m_aColumns[i].iLocation > m_aColumns[iColumn].iLocation && m_aColumns[i].bHidden)
			iFrom++;

	for (int i = iFrom; i > 0; i--)
		piArray[i] = piArray[i - 1];
	piArray[0] = iColumn;
	pHeaderCtrl->SetOrderArray(m_iColumnsTracked, piArray);
	delete[] piArray;

//	update entry
	m_aColumns[iColumn].bHidden = true;

//	redraw
	SetRedraw(TRUE);
	Invalidate(FALSE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::ShowColumn(int iColumn)
{
	CHeaderCtrl		*pHeaderCtrl = GetHeaderCtrl();
	int				iCount = pHeaderCtrl->GetItemCount();

	if (iColumn < 1 || iColumn >= iCount || !m_aColumns[iColumn].bHidden)
		return;

//	stop it from redrawing
	SetRedraw(FALSE);

//	restore position in list
	int				*piArray = new INT[m_iColumnsTracked];

	pHeaderCtrl->GetOrderArray(piArray, m_iColumnsTracked);

	int				iCurrent = IndexToOrder(pHeaderCtrl, iColumn);

	for (; iCurrent < IndexToOrder(pHeaderCtrl, 0) && iCurrent < m_iColumnsTracked - 1; iCurrent++)
		piArray[iCurrent] = piArray[iCurrent + 1];
	for (; (iCurrent < m_iColumnsTracked - 1) &&
		m_aColumns[iColumn].iLocation > m_aColumns[pHeaderCtrl->OrderToIndex(iCurrent + 1)].iLocation; iCurrent++ )
	{
		piArray[iCurrent] = piArray[iCurrent + 1];
	}
	piArray[iCurrent] = iColumn;
	pHeaderCtrl->SetOrderArray(m_iColumnsTracked, piArray);
	delete[] piArray;

//	and THEN restore original width
	HDITEM			item;

	item.mask = HDI_WIDTH;
	item.cxy = m_aColumns[iColumn].iWidth;
	pHeaderCtrl->SetItem(iColumn, &item);

//	update entry
	m_aColumns[iColumn].bHidden = false;

//	redraw
	SetRedraw(TRUE);
	Invalidate(FALSE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::SaveSettings(CPreferences::EnumTable tID)
{
	if (m_iColumnsTracked > 0)	// save only when something was loaded
	{
		int	i = m_iColumnsTracked - 1, *piArray = new INT[m_iColumnsTracked];

		do
		{
			g_App.m_pPrefs->SetColumnWidth(tID, i, GetColumnWidth(i));
			g_App.m_pPrefs->SetColumnHidden(tID, i, IsColumnHidden(i));
			piArray[i] = m_aColumns[i].iLocation;
		} while(--i >= 0);

		g_App.m_pPrefs->SetColumnOrder(tID, piArray);
		delete[] piArray;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::LoadSettings(CPreferences::EnumTable tID)
{
	CHeaderCtrl			*pHeaderCtrl = GetHeaderCtrl();
	int					*piArray = new INT[m_iColumnsTracked];

	for (int i = 0; i < m_iColumnsTracked; i++)
		piArray[i] = i;

	for (int i = 0; i < m_iColumnsTracked; i++)
	{
		int iWidth = g_App.m_pPrefs->GetColumnWidth(tID, i);

		if (iWidth != 0)
			SetColumnWidth(i, iWidth);
		if (i == 0)
		{
			piArray[0] = 0;
		}
		else
		{
			int iOrder = g_App.m_pPrefs->GetColumnOrder(tID, i);

			if (iOrder > 0 && iOrder < m_iColumnsTracked && iOrder != i)
				piArray[iOrder] = i;
		}
	}

	pHeaderCtrl->SetOrderArray(m_iColumnsTracked, piArray);
	pHeaderCtrl->GetOrderArray(piArray, m_iColumnsTracked);
	for (int i = 0; i < m_iColumnsTracked; i++)
		m_aColumns[piArray[i]].iLocation = i;

	delete[] piArray;

	for (int i = 1; i < m_iColumnsTracked; i++)
	{
		if (g_App.m_pPrefs->GetColumnHidden(tID, i))
			HideColumn(i);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::SetColors()
{
	m_crWindow = ::GetSysColor(COLOR_WINDOW);
	m_crWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
	m_crWindowTextBk = m_crWindow;

	COLORREF		crHighlight = ::GetSysColor(COLOR_HIGHLIGHT);

	m_crFocusLine	= crHighlight;
	m_crNoHighlight = MLC_RGBBLEND(crHighlight, m_crWindow, 8);
	m_crNoFocusLine = MLC_RGBBLEND(crHighlight, m_crWindow, 2);
	m_crHighlight	= MLC_RGBBLEND(crHighlight, m_crWindow, 4);
	m_crGlow		= ::GetSysColor(COLOR_INFOBK);
	m_crGlowText	= ::GetSysColor(COLOR_INFOTEXT);
	m_crDimmedText	= ::GetSysColor(COLOR_GRAYTEXT);
}
void CMuleListCtrl::SetSortArrow(int iColumn, bool bAscending, int iColumnIndex)
{
	ArrowType		arrowType;

	if (bAscending)
	{
		switch (iColumnIndex)
		{
			case 0:
				arrowType = arrowUp;
				break;
			case 1:
				arrowType = arrowUp1;
				break;
			case 2:
				arrowType = arrowUp2;
				break;
			case 3:
				arrowType = arrowUp3;
				break;
			default:
				arrowType = arrowUp;
		}
	}
	else
	{
		switch (iColumnIndex)
		{
			case 1:
				arrowType = arrowDown1;
				break;
			case 2:
				arrowType = arrowDown2;
				break;
			case 3:
				arrowType = arrowDown3;
				break;
			default:
				arrowType = arrowDown;
		}
	}
	SetSortArrow(iColumn, arrowType);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::SetSortArrow(int iColumn, ArrowType atType)
{
	if (::IsWindow(GetSafeHwnd()) == false || g_App.m_app_state == CEmuleApp::APP_STATE_SHUTTINGDOWN)
		return;

	HDITEM		headerItem;

	headerItem.mask = HDI_FORMAT;

	CHeaderCtrl		*pHeaderCtrl = GetHeaderCtrl();
	int				iSortIndex = 0;
	bool			bMultiColumn = false;

	if (atType == arrowUp1 || atType == arrowDown1)
	{
		bMultiColumn = true;
	}
	if (atType == arrowUp2 || atType == arrowDown2)
	{
		iSortIndex = 2 - 1;
		bMultiColumn = true;
	}
	else if (atType == arrowUp3 || atType == arrowDown3)
	{
		iSortIndex = 3 - 1;
		bMultiColumn = true;
	}
//
//	Delete old image if column has changed
	if (iColumn != m_iCurrentSortItem[iSortIndex])
	{
		if (m_iCurrentSortItem[iSortIndex] >= 0)
		{
			pHeaderCtrl->GetItem(m_iCurrentSortItem[iSortIndex], &headerItem);
			headerItem.fmt &= ~(HDF_IMAGE | HDF_BITMAP_ON_RIGHT);
		}
		else
			headerItem.fmt = HDF_STRING;
		pHeaderCtrl->SetItem(m_iCurrentSortItem[iSortIndex], &headerItem);
		m_iCurrentSortItem[iSortIndex] = iColumn;
		m_imlHeaderCtrl.DeleteImageList();
	}
	if (!bMultiColumn)
	{
		headerItem.fmt = HDF_STRING;
		pHeaderCtrl->SetItem(m_iCurrentSortItem[1], &headerItem);
		m_iCurrentSortItem[1] = -1;
		pHeaderCtrl->SetItem(m_iCurrentSortItem[2], &headerItem);
		m_iCurrentSortItem[2] = -1;
	}
//place new arrow unless we were given an invalid column
	if (iColumn >= 0 && pHeaderCtrl->GetItem(iColumn, &headerItem))
	{
		m_atSortArrow[iSortIndex] = atType;

		HINSTANCE		hInstRes = AfxFindResourceHandle(MAKEINTRESOURCE(m_atSortArrow[0]), RT_BITMAP);
		HINSTANCE		hInstRes2 = NULL;
		HINSTANCE		hInstRes3 = NULL;

		if (hInstRes != NULL)
		{
			HBITMAP			hbmSortStates = (HBITMAP)::LoadImage(hInstRes, MAKEINTRESOURCE(m_atSortArrow[0]), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);

			if (hbmSortStates != NULL)
			{
				CBitmap			bmSortStates;

				bmSortStates.Attach(hbmSortStates);

				CImageList		imlSortStates;

				if (imlSortStates.Create(14, 14, ILC_COLOR | ILC_MASK, 1, 0))
				{
					VERIFY(imlSortStates.Add(&bmSortStates, RGB(255, 0, 255)) != -1);
					if (m_iCurrentSortItem[1] >= 0)
					{
						hInstRes2 = AfxFindResourceHandle(MAKEINTRESOURCE(m_atSortArrow[1]), RT_BITMAP);

						if (hInstRes2 != NULL)
						{
							HBITMAP		hbmSortStates2 = (HBITMAP)::LoadImage(hInstRes, MAKEINTRESOURCE(m_atSortArrow[1]), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);

							if (hbmSortStates2 != NULL)
							{
								CBitmap		bmSortStates2;

								bmSortStates2.Attach(hbmSortStates2);
								VERIFY(imlSortStates.Add(&bmSortStates2, RGB(255, 0, 255)) != -1);
							}
						}
					}
					if (m_iCurrentSortItem[2] >= 0)
					{
						hInstRes3 = AfxFindResourceHandle(MAKEINTRESOURCE(m_atSortArrow[2]), RT_BITMAP);

						if (hInstRes3 != NULL)
						{
							HBITMAP		hbmSortStates3 = (HBITMAP)::LoadImage(hInstRes, MAKEINTRESOURCE(m_atSortArrow[2]), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);

							if (hbmSortStates3 != NULL)
							{
								CBitmap		bmSortStates3;

								bmSortStates3.Attach(hbmSortStates3);
								VERIFY(imlSortStates.Add(&bmSortStates3, RGB(255, 0, 255)) != -1);
							}
						}
					}

				//	To avoid drawing problems (which occure only with an image list *with* a mask) while
				//	resizing list view columns which have the header control bitmap right aligned, set
				//	the background color of the image list.
					if (g_App.m_qwComCtrlVer < MAKEDLLVERULL(6, 0, 0, 0))
						imlSortStates.SetBkColor(GetSysColor(COLOR_BTNFACE));

				//	When setting the image list for the header control for the first time we'll get
				//	the image list of the listview control!! So, better store the header control imagelist separate.
					(void)pHeaderCtrl->SetImageList(&imlSortStates);
					m_imlHeaderCtrl.DeleteImageList();
					m_imlHeaderCtrl.Attach(imlSortStates.Detach());

				//	Use smaller bitmap margins -- this saves some pixels which may be required for
				//	rather small column titles.
					if (g_App.m_qwComCtrlVer >= MAKEDLLVERULL(5, 8, 0, 0))
					{
						int iBmpMargin = pHeaderCtrl->SendMessage(HDM_GETBITMAPMARGIN);
						int iNewBmpMargin = GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXEDGE) / 2;
						if (iNewBmpMargin < iBmpMargin)
							pHeaderCtrl->SendMessage(HDM_SETBITMAPMARGIN, iNewBmpMargin);
					}
				}
			}
		}
		headerItem.mask |= HDI_IMAGE;
		headerItem.fmt |= HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
		headerItem.iImage = iSortIndex;
		pHeaderCtrl->SetItem(iColumn, &headerItem);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//move item in list, returns index of new item
int CMuleListCtrl::MoveItem(int iOldIndex, int iNewIndex)
{
	if (iNewIndex > iOldIndex)
		iNewIndex--;

	TCHAR	acText[256];
	LVITEM	lvi;
	DWORD	dwStyle;

//	Get Item
	lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM | LVIF_INDENT | LVIF_IMAGE | LVIF_NORECOMPUTE;
	lvi.stateMask = (UINT) - 1;
	lvi.iItem = iOldIndex;
	lvi.iSubItem = 0;	// Set subitem pointer on Item
	lvi.pszText = acText;
	lvi.cchTextMax = ARRSIZE(acText);
	lvi.iIndent = 0;
	if (!GetItem(&lvi))
		return -1;

//	Get Style an save subsettings if required
	dwStyle = GetStyle();

//	Prevent redraw during moving (delete & inserts)
	SetRedraw(FALSE);

	if ((dwStyle & LVS_OWNERDATA) == 0)
	{
	//	Save substrings
		CSimpleArray<void*>	aSubItems;
		TCHAR	acTextSub[256];
		LVITEM	lviSub;
		void	*pstrSubItem;
		int		i;

		lviSub.mask = LVIF_TEXT | LVIF_NORECOMPUTE;
		lviSub.iItem = iOldIndex;
		for (i = 1; i < m_iColumnsTracked; i++)
		{
			lviSub.iSubItem = i;
			lviSub.cchTextMax = ARRSIZE(acTextSub);
			lviSub.pszText = acTextSub;

			if (GetItem(&lviSub))
			{
				if (lviSub.pszText == LPSTR_TEXTCALLBACK)
					pstrSubItem = LPSTR_TEXTCALLBACK;
				else
					pstrSubItem = new CString(lviSub.pszText);
			}
			else
				pstrSubItem = NULL;
			aSubItems.Add(pstrSubItem);
		}

		m_bMovingItem = true;

		DeleteItem(iOldIndex);
		lvi.iItem = iNewIndex;
		iNewIndex = InsertItem(&lvi);

		m_bMovingItem = false;

	//	Restore substrings
		lviSub.iItem = iNewIndex;
		for (i = 1; i <= aSubItems.GetSize(); i++)
		{
			pstrSubItem = aSubItems[i - 1];
			if (pstrSubItem != NULL)
			{
			//	Several crashes were reporting showing that m_iColumnsTracked changed
			//	while moving row, be very careful with here
				if (i < m_iColumnsTracked)
				{
					lviSub.iSubItem = i;

					if (pstrSubItem == LPSTR_TEXTCALLBACK)
						lviSub.pszText = LPSTR_TEXTCALLBACK;
					else
						lviSub.pszText = const_cast<LPTSTR>(reinterpret_cast<CString*>(pstrSubItem)->GetString());
					DefWindowProc(LVM_SETITEMTEXT, iNewIndex, reinterpret_cast<LPARAM>(&lviSub));
				}
				if (pstrSubItem != LPSTR_TEXTCALLBACK)
					delete static_cast<CString*>(pstrSubItem);
			}
		}
	}
	else
	{
		m_bMovingItem = true;

		DeleteItem(iOldIndex);
		lvi.iItem = iNewIndex;
		iNewIndex = InsertItem(&lvi);

		m_bMovingItem = false;
	}

	SetRedraw(TRUE);

	return iNewIndex;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMuleListCtrl::UpdateLocation(int iItem)
{
	int iNewIndex = GetNewLocation(GetItemData(iItem), iItem, false);

// move if required
	if (iNewIndex != iItem)
		return MoveItem(iItem, iNewIndex);

	return iItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CMuleListCtrl::GetNewLocation(const DWORD_PTR dwpItemData, int iItem, bool bInsertion)
{
	if (dwpItemData == NULL)
		return iItem;

	int iItemCount = GetItemCount();

// note: the maximal index value that can handle following algorithm is equal to the number of the entries
// in the list, this value is always used during insertion
	if (static_cast<uint32>(iItem) > static_cast<uint32>(iItemCount))
		return iItem;

	bool			notLast =  bInsertion ? (iItem < iItemCount) : (iItem < (iItemCount - 1));
	bool			notFirst = iItem > 0;
	uint32			dwLastIndex = static_cast<uint32>(iItemCount - 1);
	uint32			dwLowerBound = 0;
	uint32			dwUpperBound = dwLastIndex;
	uint32			i, dwNewIndex, dwDist;
	POSITION		posItem, posPrevItem, posNextItem, posCmpItem;

// get the position of the element in the list
	if (!notLast)
		posItem = m_Params.GetTailPosition();
	else
		posItem = m_Params.FindIndex(iItem);

//	if it isn't first element, we gonna compare it with previous element
	if (notFirst)
	{
	//	get Index & Position of previous element (Item-1)
		dwNewIndex = static_cast<uint32>(iItem - 1);
		posPrevItem = posItem;
		if (iItem != iItemCount)
			m_Params.GetPrev(posPrevItem);

	//	check if we need to move an Item
		int			iResult = m_SortProc(dwpItemData, GetParamAt(posPrevItem, dwNewIndex), m_dwParamSort);

	//	we need to move it
		if (iResult < 0)
		{
			dwDist = dwNewIndex;
			posCmpItem = m_Params.GetHeadPosition();

			while (dwDist >= 1)
			{
				if (m_SortProc(dwpItemData, GetParamAt(posCmpItem, dwNewIndex - dwDist), m_dwParamSort) < 0)
				{
				//	update an Index & postion
					dwNewIndex = dwNewIndex - dwDist;
					posPrevItem = posCmpItem;
				//	calculate new step based on new Index
					dwDist = dwNewIndex - dwLowerBound + 1u;
				}
				else
				{
				//	set postion to initial one, if we don't need to move
					posCmpItem = posPrevItem;
				//	set new boundary
					dwLowerBound = dwNewIndex - dwDist;
				}
			//	just decrease a step
				dwDist >>= 1u;

				if (dwDist == 0)
					break;
				i = dwDist;
				do
				{
					m_Params.GetPrev(posCmpItem);
				} while(--i > 0u);
			}
			return dwNewIndex;
		}
	}

	if (notLast)
	{
	//	get Index of next element (Item+1)
		dwNewIndex = static_cast<uint32>(iItem);
		posNextItem = posItem;
		if (!bInsertion)
		{
			dwNewIndex++;
			m_Params.GetNext(posNextItem);
		}

	//	check if we need to move an Item
		int iResult = m_SortProc(dwpItemData, GetParamAt(posNextItem, dwNewIndex), m_dwParamSort);

		if (iResult > 0)
		{
			dwDist = dwLastIndex - dwNewIndex;
			posCmpItem = m_Params.GetTailPosition();

			while (dwDist >= 1)
			{
				if (m_SortProc(dwpItemData, GetParamAt(posCmpItem, dwNewIndex + dwDist), m_dwParamSort) > 0)
				{
				//	update an Index & postion
					dwNewIndex = dwNewIndex + dwDist;
					posNextItem = posCmpItem;
				//	calculate new step based on new Index
					dwDist = dwUpperBound - dwNewIndex + 1u;
				}
				else
				{
				//	set postion to initial one, if we don't need to move
					posCmpItem = posNextItem;
				//	set new boundary
					dwUpperBound = dwNewIndex + dwDist;
				}
			//	just decrease a step
				dwDist >>= 1u;

				if (dwDist == 0)
					break;
				i = dwDist;
				do
				{
					m_Params.GetNext(posCmpItem);
				} while(--i > 0u);
			}
			return ++dwNewIndex;
		}
	}

	return iItem;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD_PTR CMuleListCtrl::GetItemData(int iItem)
{
	POSITION		pos = m_Params.FindIndex(iItem);
	LPARAM			lParam = 0;

	if (pos != NULL)
	{
		lParam = GetParamAt(pos, iItem);
		MLC_ASSERT(lParam == CListCtrl::GetItemData(iItem));
	}

	return lParam;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::OnNMDividerDoubleClick(NMHEADER *pHeader)
{
//	As long as we do not handle the HDN_DIVIDERDBLCLICK according the actual
//	listview item contents it's better to resize to the header width instead of
//	resizing to zero width. The complete solution for this would require a lot
//	of rewriting in the owner drawn listview controls...
	SetColumnWidth(pHeader->iItem, LVSCW_AUTOSIZE_USEHEADER);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	lower level than everything else so poorly overriden functions don't break us
BOOL CMuleListCtrl::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
//	lets look for the important messages that are essential to handle
	switch (message)
	{
		case WM_NOTIFY:
		{
			if (wParam == 0)
			{
				if (((NMHDR*)lParam)->code == NM_RCLICK)
				{
				//	Handle right click on headers and show column menu
					POINT		point;
					CTitleMenu	tmColumnMenu;

					GetCursorPos(&point);
					tmColumnMenu.CreatePopupMenu();

					CHeaderCtrl			*pHeaderCtrl = GetHeaderCtrl();
					int					iCount = pHeaderCtrl->GetItemCount();

					for (int iCurrent = 1; iCurrent < iCount; iCurrent++)
					{
						HDITEM		item;
						TCHAR		text[255];

						item.pszText = text;
						item.mask = HDI_TEXT;
						item.cchTextMax = 255;
						pHeaderCtrl->GetItem(iCurrent, &item);

						tmColumnMenu.AppendMenu( MF_STRING | ((m_aColumns[iCurrent].bHidden) ? 0 : MF_CHECKED),
												MLC_IDC_MENU + iCurrent, item.pszText );
					}
					tmColumnMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

				//	Menu objects are destroyed in their destructor
					return *pResult = TRUE;

				}
				else if (((NMHDR*)lParam)->code == HDN_BEGINTRACKA || ((NMHDR*)lParam)->code == HDN_BEGINTRACKW)
				{
				//stop them from changeing the size of anything "before" first column
					if (m_aColumns[((HD_NOTIFY*)lParam)->iItem].bHidden)
						return *pResult = TRUE;
				}
				else if (((NMHDR*)lParam)->code == HDN_ENDDRAG)
				{
				//stop them from moving first column

					NMHEADER		*pHeader = (NMHEADER*)lParam;

					if (pHeader->iItem != 0 && pHeader->pitem->iOrder != 0)
					{
						int			iNewLoc = pHeader->pitem->iOrder - GetHiddenColumnCount();

						if (iNewLoc > 0)
						{
							if (m_aColumns[pHeader->iItem].iLocation != iNewLoc)
							{
								if (m_aColumns[pHeader->iItem].iLocation > iNewLoc)
								{
									int			iMax = m_aColumns[pHeader->iItem].iLocation;
									int			iMin = iNewLoc;

									for (int i = 0; i < m_iColumnsTracked; i++)
									{
										if (m_aColumns[i].iLocation >= iMin && m_aColumns[i].iLocation < iMax)
											m_aColumns[i].iLocation++;
									}
								}

								else if (m_aColumns[pHeader->iItem].iLocation < iNewLoc)
								{
									int			iMin = m_aColumns[pHeader->iItem].iLocation;
									int			iMax = iNewLoc;

									for (int i = 0; i < m_iColumnsTracked; i++)
									{
										if (m_aColumns[i].iLocation > iMin && m_aColumns[i].iLocation <= iMax)
											m_aColumns[i].iLocation--;
									}
								}

								m_aColumns[pHeader->iItem].iLocation = iNewLoc;

								Invalidate(FALSE);
								break;
							}
						}
					}

					return *pResult = 1;
				}
				else if (((NMHDR*)lParam)->code == HDN_DIVIDERDBLCLICKA || ((NMHDR*)lParam)->code == HDN_DIVIDERDBLCLICKW)
				{
					if (GetStyle() & LVS_OWNERDRAWFIXED)
					{
						OnNMDividerDoubleClick(reinterpret_cast<NMHEADER*>(lParam));
						return *pResult = 1;
					}
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			if (wParam == MLC_IDC_UPDATE)
			{
				UpdateLocation(lParam);
				return *pResult = 1;
			}
			else if (wParam >= MLC_IDC_MENU)	//	deal with menu clicks
			{
				CHeaderCtrl			*pHeaderCtrl = GetHeaderCtrl();
				int					iCount = pHeaderCtrl->GetItemCount();
				int					iToggle = wParam - MLC_IDC_MENU;

				if (iToggle >= iCount)
					break;

				if (m_aColumns[iToggle].bHidden)
					ShowColumn(iToggle);
				else
					HideColumn(iToggle);

				return *pResult = 1;
			}
			break;
		}
		case LVM_DELETECOLUMN:
		{
		//book keeping!
			if (m_aColumns != NULL)
			{
				for (int i = 0; i < m_iColumnsTracked; i++)
					if (m_aColumns[i].bHidden)
						ShowColumn(i);

				delete[] m_aColumns;
				m_aColumns = NULL; // 'new' may throw an exception
			}
			m_aColumns = new MULE_COLUMN[--m_iColumnsTracked];
			for (int i = 0; i < m_iColumnsTracked; i++)
			{
				m_aColumns[i].iLocation = i;
				m_aColumns[i].bHidden = false;
			}
			break;
		}
	//	case LVM_INSERTCOLUMN:
		case LVM_INSERTCOLUMNA:
		case LVM_INSERTCOLUMNW:
		{
		//book keeping!
			if (m_aColumns != NULL)
			{
				for (int i = 0; i < m_iColumnsTracked; i++)
					if (m_aColumns[i].bHidden)
						ShowColumn(i);

				delete[] m_aColumns;
				m_aColumns = NULL; // 'new' may throw an exception
			}
			m_aColumns = new MULE_COLUMN[++m_iColumnsTracked];
			for (int i = 0; i < m_iColumnsTracked; i++)
			{
				m_aColumns[i].iLocation = i;
				m_aColumns[i].bHidden = false;
			}
			break;
		}
		case LVM_SETITEM:
		{
		//	book keeping
			POSITION pos = m_Params.FindIndex(((LPLVITEM)lParam)->iItem);
			if (pos != NULL)
			{
				m_Params.SetAt(pos, MLC_MAGIC);
			//	m_SortProc = SortProc when sorting is disabled
			//	Don't post any update messages when sorting is disabled to avoid
			//	flood of messages overflowing message queues, what results in other issues
				if (m_SortProc != SortProc)
					PostMessage(LVM_UPDATE, ((LPLVITEM)lParam)->iItem);
			}
			break;
		}
		case LVN_KEYDOWN:
		{
			break;
		}
		case LVM_SETITEMTEXT:
		{
		//	need to check for movement
			*pResult = DefWindowProc(message, wParam, lParam);
		//	m_SortProc = SortProc when sorting is disabled
		//	Don't post any update messages when sorting is disabled to avoid
		//	flood of messages overflowing message queues, what results in other issues
			if ((m_SortProc != SortProc) && *pResult)
				PostMessage(WM_COMMAND, MLC_IDC_UPDATE, wParam);
			return *pResult;
		}
		case LVM_SORTITEMS:
		{
		//	book keeping...
			m_dwParamSort = (LPARAM)wParam;
			m_SortProc = (PFNLVCOMPARE)lParam;
			for (POSITION pos = m_Params.GetHeadPosition(); pos != NULL; m_Params.GetNext(pos))
				m_Params.SetAt(pos, MLC_MAGIC);
			break;
		}
		case LVM_DELETEALLITEMS:
		{
		//	book keeping...
			if (!CListCtrl::OnWndMsg(message, wParam, lParam, pResult) && DefWindowProc(message, wParam, lParam))
				m_Params.RemoveAll();
			return *pResult = TRUE;
		}
		case LVM_DELETEITEM:
		{
		//	book keeping.....
			MLC_ASSERT(m_Params.GetAt(m_Params.FindIndex(wParam)) == CListCtrl::GetItemData(wParam));
			if (!CListCtrl::OnWndMsg(message, wParam, lParam, pResult) && DefWindowProc(message, wParam, lParam))
				m_Params.RemoveAt(m_Params.FindIndex(wParam));
			return *pResult = TRUE;
		}
	//	case LVM_INSERTITEM:
		case LVM_INSERTITEMA:
		case LVM_INSERTITEMW:
	//	try to fix position of inserted items
		{
			LPLVITEM		pItem = (LPLVITEM)lParam;

		//update location
			if (!m_bMovingItem)
				pItem->iItem = GetNewLocation(pItem->lParam, pItem->iItem, true);

			if (pItem->iItem == 0)
			{
				m_Params.AddHead(pItem->lParam);
				return FALSE;
			}

			LRESULT lResult = DefWindowProc(message, wParam, lParam);
			if (lResult != -1)
			{
				if (lResult >= GetItemCount())
					m_Params.AddTail(pItem->lParam);
				else if (lResult == 0)
					m_Params.AddHead(pItem->lParam);
				else
					m_Params.InsertAfter(m_Params.FindIndex(lResult - 1), pItem->lParam);
			}
			return *pResult = lResult;
			break;
		}
		case LVM_UPDATE:
		{
		//	better fix for old problem... normally Update(int) causes entire list to redraw

			if (wParam == (WPARAM)UpdateLocation(wParam))
			{ //no need to invalidate rect if item moved
				RECT		rcItem;
				BOOL		bResult = GetItemRect(wParam, &rcItem, LVIR_BOUNDS);

				if (bResult)
					InvalidateRect(&rcItem, FALSE);
				return *pResult = bResult;
			}
			return *pResult = TRUE;
		}
	}

	return CListCtrl::OnWndMsg(message, wParam, lParam, pResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_DELETE)
	{
		PostMessage(WM_COMMAND, MP_CANCEL, 0);
	}
	else if (nChar == 'A')
	{
	//	Select everything only on the first press and don't on repeated events
		if (((nFlags & 0x4000) == 0) && (::GetAsyncKeyState(VK_CONTROL) < 0))	//	Ctrl+A: Select all items
			SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
	}
	else if (nChar == VK_ADD)
	{
		if (::GetAsyncKeyState(VK_CONTROL) < 0)
		{
		//	Auto-resize list only on the first press and don't on repeated events
			if ((nFlags & 0x4000) == 0)
			{
				NMHEADER	Hdr;

			//	List redraw can't be disabled as it's required for transfer window lists
			//	to calculate max column width
				for (int i = 0; i < GetHeaderCtrl()->GetItemCount(); i++)
				{
					if (!m_aColumns[i].bHidden)
					{
						Hdr.iItem = i;
						OnNMDividerDoubleClick(&Hdr);
					}
				}
			}
			return;
		}
	}
	else if (m_bGeneralPurposeFind)
	{
		if (nChar == 'F')
		{
			if (GetKeyState(VK_CONTROL) & 0x8000)	//	Ctrl+F: Search item
				OnFindStart();
		}
		else if (nChar == VK_F3)
		{
			if (GetKeyState(VK_SHIFT) & 0x8000)
				OnFindPrev();	//	Shift+F3: Search previous
			else
				OnFindNext();	//	F3: Search next
		}
	}

	return CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CMuleListCtrl::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message != WM_DRAWITEM)
	{
	//	Catch the prepaint and copy struct
		if ( message == WM_NOTIFY && ((NMHDR*)lParam)->code == NM_CUSTOMDRAW
		     && ((LPNMLVCUSTOMDRAW)lParam)->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
		{
			m_bCustomDraw = B2b(CListCtrl::OnChildNotify(message, wParam, lParam, pResult));

			if (m_bCustomDraw)
				memcpy2(&m_lvcd, (void*)lParam, sizeof(NMLVCUSTOMDRAW));

			return m_bCustomDraw;
		}

		return CListCtrl::OnChildNotify(message, wParam, lParam, pResult);
	}

	ASSERT(pResult == NULL); // no return value expected
	UNUSED(pResult);         // unused in release builds

	DrawItem((LPDRAWITEMSTRUCT)lParam);

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::TextDraw(CDC* pDC, CString &strText, LPRECT lpRect, UINT iFormat, COLORREF crForeground, COLORREF crBackground)
{
	int			iIndex = 0;
	COLORREF	crColor;

	while (!strText.IsEmpty())
	{
		switch (strText.GetAt(iIndex))
		{
			case 1:
			case 2:
			{
				if (iIndex > 0)
				{
					pDC->DrawText(strText.Left(iIndex), lpRect, iFormat);
					lpRect->left += pDC->GetTextExtent(strText.Left(iIndex)).cx;
				}
				if (strText.GetLength() > iIndex + 4)
				{
					_stscanf(strText.Mid(iIndex + 1, 6), _T("%06x"), &crColor);
					if (strText.GetAt(iIndex) == 1)
						pDC->SetTextColor(crColor);
					else
						pDC->SetBkColor(crColor);
				}
				strText = strText.Mid(iIndex + 7);
				iIndex = 0;
				break;
			}
			case 3:
			{
				if (iIndex > 0)
				{
					pDC->DrawText(strText.Left(iIndex), lpRect, iFormat);
					lpRect->left += pDC->GetTextExtent(strText.Left(iIndex)).cx;
				}
				pDC->SetTextColor(crForeground);
				pDC->SetBkColor(crBackground);

				strText = strText.Mid(iIndex + 1);
				iIndex = 0;
				break;
			}
			default:
				iIndex++;
		}
		if (iIndex >= strText.GetLength())
		{
			pDC->DrawText(strText, lpRect, iFormat);
			break;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	int					iItem = lpDrawItemStruct->itemID;
	CImageList			*pImageList;
	CHeaderCtrl			*pHeaderCtrl = GetHeaderCtrl();

//	gets the item image and state info
	LV_ITEM				lvi;

	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_GLOW | LVIS_CUT | LVIS_OVERLAYMASK;
	GetItem(&lvi);

//	see if the item be highlighted
	bool			bHighlight = ((lvi.state & LVIS_DROPHILITED) || (lvi.state & LVIS_SELECTED));
	bool			bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));

//	set up our ficker free drawing
	int				iState = SaveDC(lpDrawItemStruct->hDC);
	COLORREF		crForeground, crBackground, crOldTextColor;

//	Select background color
	crBackground = m_crWindow;
	if (bHighlight)
	{
		if (bCtrlFocused)
			crBackground = m_crHighlight;
		else
			crBackground = (lvi.state & LVIS_GLOW) ? m_crGlow : m_crNoHighlight;
	}
	else if (lvi.state & LVIS_GLOW)
		crBackground = m_crGlow;

	CDC				*oDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CMemDC			pDC(oDC, &lpDrawItemStruct->rcItem, m_crWindow, (m_crWindowTextBk == CLR_NONE) ? m_crWindow : crBackground);
	CFont			*pOldFont = pDC->SelectObject(GetFont());

	if (m_bCustomDraw)
		crForeground = m_lvcd.clrText;
	else if (lvi.state & LVIS_CUT)	// Dimmed
		crForeground = m_crDimmedText;
	else if (lvi.state & LVIS_GLOW)	// Glowing
		crForeground = m_crGlowText;
	else
		crForeground = m_crWindowText;

	crOldTextColor = pDC->SetTextColor(crForeground);

	if (m_crWindowTextBk == CLR_NONE)
	{
		DefWindowProc(WM_ERASEBKGND, (WPARAM)pDC->m_hDC, 0);
		pDC->SetBkColor(crBackground);
	}

	const int		iOffset = 2;
//	get rectangles for drawing
	CRect			rcBounds, rcLabel, rcIcon;

	GetItemRect(iItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(iItem, rcLabel, LVIR_LABEL);

	CRect			rcCol(rcBounds);
	CString			sLabel = GetItemText(iItem, 0);

//	update column
	rcCol.right = rcCol.left + CListCtrl::GetColumnWidth(0);

//	draw state icon
	if (lvi.state & LVIS_STATEIMAGEMASK)
	{
		int				nImage = ((lvi.state & LVIS_STATEIMAGEMASK) >> 12) - 1;

		pImageList = GetImageList(LVSIL_STATE);
		if (pImageList)
		{
			COLORREF			crOld = pImageList->SetBkColor(CLR_NONE);

			pImageList->Draw(pDC, nImage, rcCol.TopLeft(), ILD_NORMAL);
			pImageList->SetBkColor(crOld);
		}
	}

//	draw the item's icon
	pImageList = GetImageList(LVSIL_SMALL);
	if (pImageList != NULL)
	{
		COLORREF			crOld = pImageList->SetBkColor(CLR_NONE);

		GetItemRect(iItem, rcIcon, LVIR_ICON);
		pImageList->Draw(pDC, lvi.iImage, rcIcon.TopLeft(), ILD_NORMAL);
		pImageList->SetBkColor(crOld);
		rcLabel.left += 2 * iOffset;
	}
	else
		rcLabel.left = rcBounds.left + iOffset;

	int		iOldBkMode = (m_crWindowTextBk == CLR_NONE) ? pDC->SetBkMode(TRANSPARENT) : OPAQUE;

//	draw item label (column 0)
	rcLabel.right -= iOffset;
	if (lvi.state & LVIS_OVERLAYMASK)
		TextDraw(pDC, sLabel, rcLabel, MLC_DT_TEXT | DT_LEFT | DT_NOCLIP, crForeground, crBackground);
	else
		pDC->DrawText(sLabel, rcLabel, MLC_DT_TEXT | DT_LEFT | DT_NOCLIP);

//	draw labels for remaining columns
	LV_COLUMN			lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	int				iCount = pHeaderCtrl->GetItemCount();

	for (int iCurrent = 1; iCurrent < iCount; iCurrent++)
	{
		int				iColumn = pHeaderCtrl->OrderToIndex(iCurrent);

	//	don't draw column 0 again
		if (iColumn == 0)
			continue;

		GetColumn(iColumn, &lvc);
	//	don't draw anything with 0 width
		if (lvc.cx == 0)
			continue;

		rcCol.left = rcCol.right;
		rcCol.right += lvc.cx;

		sLabel = GetItemText(iItem, iColumn);
		if (sLabel.IsEmpty())
			continue;

	//	get the text justification
		UINT			nJustify = MLC_DT_TEXT | DT_LEFT;

		switch (lvc.fmt & LVCFMT_JUSTIFYMASK)
		{
			case LVCFMT_RIGHT:
				nJustify = MLC_DT_TEXT | DT_RIGHT;
				break;
			case LVCFMT_CENTER:
				nJustify = MLC_DT_TEXT | DT_CENTER;
				break;
		}

		if (IsRightToLeftLanguage())
			nJustify |= DT_RTLREADING;

		rcLabel.left = rcCol.left + iOffset;
		rcLabel.right = rcCol.right - iOffset;

		if (lvi.state & LVIS_OVERLAYMASK)
			TextDraw(pDC, sLabel, rcLabel, nJustify, crForeground, crBackground);
		else
			pDC->DrawText(sLabel, rcLabel, nJustify);
	}

//	draw focus rectangle if item has focus
	if ((lvi.state & LVIS_FOCUSED) && (bCtrlFocused || (lvi.state & LVIS_SELECTED)))
	{
		rcBounds.left++;
		rcBounds.right--;

		CBrush	brFocus((!bCtrlFocused || ((lvi.state & LVIS_SELECTED) == 0)) ? m_crNoFocusLine : m_crFocusLine);

		pDC->FrameRect(rcBounds, &brFocus);
	}

	pDC->Flush();
	if (m_crWindowTextBk == CLR_NONE)
		pDC->SetBkMode(iOldBkMode);
	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(crOldTextColor);
	RestoreDC(lpDrawItemStruct->hDC, iState);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CMuleListCtrl::OnEraseBkgnd(CDC* pDC)
{
	int				itemCount = GetItemCount();

	if (!itemCount)
		return CListCtrl::OnEraseBkgnd(pDC);

	RECT			clientRect;
	RECT			itemRect;
	int				topIndex = GetTopIndex();
	int				maxItems = GetCountPerPage();
	int				drawnItems = itemCount < maxItems ? itemCount : maxItems;
	CRect			rcClip;

//	draw top portion
	GetClientRect(&clientRect);
	rcClip = clientRect;
	GetItemRect(topIndex, &itemRect, LVIR_BOUNDS);
	clientRect.bottom = itemRect.top;
	if (m_crWindowTextBk != CLR_NONE)
		pDC->FillSolidRect(&clientRect, GetBkColor());
	else
		rcClip.top = itemRect.top;

//	draw bottom portion if we have to
	if (topIndex + maxItems >= itemCount)
	{
		GetClientRect(&clientRect);
		GetItemRect(topIndex + drawnItems - 1, &itemRect, LVIR_BOUNDS);
		clientRect.top = itemRect.bottom;
		rcClip.bottom = itemRect.bottom;
		if (m_crWindowTextBk != CLR_NONE)
			pDC->FillSolidRect(&clientRect, GetBkColor());
	}

//	draw right half if we need to
	if (itemRect.right < clientRect.right)
	{
		GetClientRect(&clientRect);
		clientRect.left = itemRect.right;
		rcClip.right = itemRect.right;
		if (m_crWindowTextBk != CLR_NONE)
			pDC->FillSolidRect(&clientRect, GetBkColor());
	}

	if (m_crWindowTextBk == CLR_NONE)
	{
		CRect			rcClipBox;

		pDC->GetClipBox(&rcClipBox);
		rcClipBox.SubtractRect(&rcClipBox, &rcClip);
		if (!rcClipBox.IsRectEmpty())
		{
			pDC->ExcludeClipRect(&rcClip);
			CListCtrl::OnEraseBkgnd(pDC);
			InvalidateRect(&rcClip, FALSE);
		}
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::OnSysColorChange()
{
//	adjust colors
	CListCtrl::OnSysColorChange();
	SetColors();

//	redraw the up/down sort arrow (if it's there)
	if (m_iCurrentSortItem[0] >= 0)
		SetSortArrow(m_iCurrentSortItem[0], (ArrowType)m_atSortArrow[0]);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::ApplyImageList(HIMAGELIST himl)
{
	SendMessage(LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)himl);
	if (m_imlHeaderCtrl.m_hImageList != NULL)
	{
	//	Must *again* set the image list for the header control, because LVM_SETIMAGELIST
	//	always resets any already specified header control image lists!
		GetHeaderCtrl()->SetImageList(&m_imlHeaderCtrl);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//	CDlgListSearchListSearch

class CDlgListSearchListSearch : public CDialog
{
	DECLARE_DYNAMIC(CDlgListSearchListSearch)

public:
	CDlgListSearchListSearch(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_LISTVIEW_SEARCH };

	CMuleListCtrl	*m_pListView;
	CString			m_strFindText;
	int				m_iSearchColumn;

protected:
	CComboBox m_ctlSearchCol;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	afx_msg void OnEnChangeSearchText();
	virtual BOOL OnInitDialog();
};


IMPLEMENT_DYNAMIC(CDlgListSearchListSearch, CDialog)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDlgListSearchListSearch::CDlgListSearchListSearch(CWnd* pParent /*=NULL*/)
		: CDialog(CDlgListSearchListSearch::IDD, pParent)
		, m_strFindText(_T(""))
{
	m_pListView = NULL;
	m_iSearchColumn = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDlgListSearchListSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTVIEW_SEARCH_COLUMN, m_ctlSearchCol);
	DDX_CBIndex(pDX, IDC_LISTVIEW_SEARCH_COLUMN, m_iSearchColumn);
	DDX_Text(pDX, IDC_LISTVIEW_SEARCH_TEXT, m_strFindText);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CDlgListSearchListSearch::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_LISTVIEW_SEARCH_TEXT_LBL, GetResString(IDS_SEARCH_TEXT) + _T(':'));
	SetDlgItemText(IDC_LISTVIEW_SEARCH_COLUMN_LBL, GetResString(IDS_SEARCH_COLUMN) + _T(':'));

	SetDlgItemText(IDCANCEL, GetResString(IDS_CANCEL));

	SetIcon(g_App.m_pMDlg->m_hiconSourceTray, FALSE);
	SetWindowText(GetResString(IDS_SEARCH_NOUN));

	if (m_pListView != NULL)
	{
		TCHAR			szColTitle[256];
		LVCOLUMN		lvc;

		lvc.mask = LVCF_TEXT;
		lvc.cchTextMax = ARRSIZE(szColTitle);
		lvc.pszText = szColTitle;

		int				iCol = 0;

		while (m_pListView->GetColumn(iCol++, &lvc))
			m_ctlSearchCol.AddString(lvc.pszText);
		if ((UINT)m_iSearchColumn >= (UINT)m_ctlSearchCol.GetCount())
			m_iSearchColumn = 0;
	}
	else
	{
		m_ctlSearchCol.EnableWindow(FALSE);
		m_ctlSearchCol.ShowWindow(SW_HIDE);

		m_iSearchColumn = 0;
	}
	m_ctlSearchCol.SetCurSel(m_iSearchColumn);

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::DoFind(int iStartItem, int iDirection /*1=down, 0 = up*/, BOOL bShowError)
{
	if (iStartItem < 0)
	{
		MessageBeep((UINT) - 1);
		return;
	}

	CWaitCursor curHourglass;
	TCHAR		acItemText[ML_SEARCH_SZ];
	int			iItem = iStartItem, iNumItems = GetItemCount();

	iDirection = iDirection ? 1 : -1;
	while ((iItem < iNumItems) && (iItem >= 0))
	{
		if (GetItemText(iItem, m_iFindColumn, acItemText, ARRSIZE(acItemText)) != 0)
		{
			if (stristr(acItemText, m_strFindText) != NULL)
			{
			//	Deselect all listview entries
				SetItemState(-1, 0, LVIS_SELECTED);

			//	Select the found listview entry
				SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				SetSelectionMark(iItem);
				EnsureVisible(iItem, FALSE /*bPartialOK*/);
				SetFocus();

				return;
			}
		}

		iItem += iDirection;
	}

	if (bShowError)
		AfxMessageBox(GetResString(IDS_SEARCH_NORESULT), MB_ICONINFORMATION);
	else
		MessageBeep((UINT) - 1);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::OnFindStart()
{
	CDlgListSearchListSearch		dlg;

	dlg.m_pListView = this;
	dlg.m_strFindText = m_strFindText;
	dlg.m_iSearchColumn = m_iFindColumn;
	if (dlg.DoModal() != IDOK)
		return;
	m_strFindText = dlg.m_strFindText;
	m_iFindColumn = dlg.m_iSearchColumn;

	if (!m_strFindText.IsEmpty())
	{
		DoFindNext(TRUE /*bShowError*/);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::OnFindNext()
{
	if (!m_strFindText.IsEmpty())
		DoFindNext(FALSE /*bShowError*/);
	else
		OnFindStart();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::DoFindNext(BOOL bShowError)
{
	int	iStartItem = GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);

	if (iStartItem == -1)
		iStartItem = 0;
	else
		iStartItem = iStartItem + (m_iFindDirection ? 1 : -1);
	DoFind(iStartItem, m_iFindDirection, bShowError);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleListCtrl::OnFindPrev()
{
	if (!m_strFindText.IsEmpty())
	{
		int			iStartItem = GetNextItem( -1, LVNI_SELECTED | LVNI_FOCUSED);

		if (iStartItem == -1)
			iStartItem = 0;
		else
			iStartItem = iStartItem + (!m_iFindDirection ? 1 : -1);

		DoFind(iStartItem, !m_iFindDirection, FALSE /*bShowError*/);
	}
	else
		OnFindStart();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	PostUniqueMessage() check the existence of message in the list control's message queue & puts it into the queue if message does not exist
void CMuleListCtrl::PostUniqueMessage(UINT uiMsg)
{
	if (::IsWindow(GetSafeHwnd()) && g_App.m_app_state != CEmuleApp::APP_STATE_SHUTTINGDOWN)
	{
		MSG		msg;

	//	If there's no refresh message already in the message queue... (don't want to flood it)
		if (!::PeekMessage(&msg, m_hWnd, uiMsg, uiMsg, PM_NOREMOVE))
		{
			PostMessage(uiMsg,0, 0);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
