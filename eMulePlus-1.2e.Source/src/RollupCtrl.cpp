//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// CRollupCtrl & CRollupHeader
// (c) 2002 by FoRcHa (a.k.a. NO)  [seppforcher38@hotmail.com]
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
///////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "otherfunctions.h"
#include "RollupCtrl.h"
#include "RollupGripper.h"
#include "TransferWnd.h"
#include "DeferPos.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRollupHeader
CRollupHeader::CRollupHeader()
{
#ifndef NEW_LOOK
	m_crBackColor = GetSysColor(COLOR_3DDKSHADOW);
	m_crTextColor =  GetSysColor(COLOR_3DHIGHLIGHT);
#else
	m_crBackColor = GetSysColor(COLOR_BTNFACE);
	m_crTextColor =  GetSysColor(COLOR_WINDOWTEXT);
#endif NEW_LOOK
	m_crBorderColor = GetSysColor(COLOR_WINDOWFRAME);

	CFont	*pDefGuiFont = CFont::FromHandle(static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)));
	LOGFONT	lf;

	pDefGuiFont->GetLogFont(&lf);
	m_cfTextFont.CreateFontIndirect(&lf);
	m_cpBorderPen.CreatePen(PS_SOLID, 1, m_crBorderColor);
	m_cpArrowPen.CreatePen(PS_SOLID, 1, m_crTextColor);
	
	m_MemDC.m_hDC = NULL;
	m_MemBMP.m_hObject = NULL;
	m_pOldMemBMP = NULL;

	m_rClientRect = m_rArrowRect = m_rTextRect = CRect(0,0,0,0);

	m_bInit = TRUE;	
	m_bExpanded = FALSE;

	m_iHeight = RUP_HEADERHEIGHT;
}

CRollupHeader::~CRollupHeader()
{
	if(m_MemDC.m_hDC)
	{
		if(m_pOldMemBMP != NULL)
			m_MemDC.SelectObject(m_pOldMemBMP);
	}
}


BEGIN_MESSAGE_MAP(CRollupHeader, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRollupHeader message handlers
void CRollupHeader::OnPaint() 
{
	CPaintDC dc(this);
	CRect rClientRect;
	
	if(m_bInit)
	{
		GetClientRect(&rClientRect);
		CreateMemDC(&dc, &rClientRect);
//		m_MemDC.SelectObject(&m_cpBorderPen);
//		m_MemDC.SetTextColor(m_crTextColor);
//		m_MemDC.SetBkColor(m_crBackColor);
//		m_MemDC.SetBkMode(OPAQUE);

		m_rClientRect = rClientRect;
		m_rArrowRect = rClientRect;
		m_rArrowRect.right = m_rArrowRect.left + m_iHeight;
		m_rArrowRect.DeflateRect(2,2);
		m_cpArrowPoint.x = m_rArrowRect.left + 5;
		m_cpArrowPoint.y = m_rArrowRect.top + 5;
		m_rTextRect = rClientRect;
		m_rTextRect.DeflateRect(m_iHeight + 2, 2, 4+5, 2);	// space(s) between border and text

		m_bInit = FALSE;
	}

	CPen *pOldPen = m_MemDC.SelectObject(&m_cpBorderPen);
	COLORREF crOldTxtColor = m_MemDC.SetTextColor(m_crTextColor);
	COLORREF crOldBckColor = m_MemDC.SetBkColor(m_crBackColor);
	int iOldBkMode = m_MemDC.SetBkMode(OPAQUE);

	m_MemDC.FillSolidRect(m_rClientRect, m_crBackColor);
	m_MemDC.MoveTo(m_rClientRect.left, m_rClientRect.top);
	m_MemDC.LineTo(m_rClientRect.right, m_rClientRect.top);

#ifndef NEW_LOOK
	m_MemDC.MoveTo(m_rClientRect.left, m_rClientRect.bottom-1);
	m_MemDC.LineTo(m_rClientRect.right, m_rClientRect.bottom-1);

	m_MemDC.MoveTo(m_rClientRect.left, m_rClientRect.top);
	m_MemDC.LineTo(m_rClientRect.left, m_rClientRect.bottom);

	m_MemDC.MoveTo(m_rClientRect.right-1, m_rClientRect.top);
	m_MemDC.LineTo(m_rClientRect.right-1, m_rClientRect.bottom);
#endif NEW_LOOK

	CPoint cpArrowPt(m_cpArrowPoint);
	if(m_bExpanded)
		cpArrowPt.y+=3;
	else
		cpArrowPt.x++;
	
	DrawArrow(&m_MemDC, &cpArrowPt, m_bExpanded);
	
	CFont *pOldFont = m_MemDC.SelectObject(&m_cfTextFont);
	m_MemDC.DrawText(m_strRightText, m_rTextRect,
		DT_RIGHT | DT_NOPREFIX | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
	m_MemDC.DrawText(m_strLeftText, m_rTextRect,
		DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

	dc.BitBlt(m_rClientRect.left, m_rClientRect.top, m_rClientRect.Width(), m_rClientRect.Height(),
				&m_MemDC, 0, 0, SRCCOPY);

	m_MemDC.SelectObject(pOldFont);
	m_MemDC.SetBkMode(iOldBkMode);
	m_MemDC.SetBkColor(crOldBckColor);
	m_MemDC.SetTextColor(crOldTxtColor);
	m_MemDC.SelectObject(pOldPen);
}

void CRollupHeader::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(point.x >= m_rArrowRect.left && point.x <= m_rArrowRect.right &&
	   point.y >= m_rArrowRect.top  && point.y <= m_rArrowRect.bottom)
	{
		m_bExpanded = !m_bExpanded;
		Invalidate();
		GetParent()->SendMessage(WM_COMMAND, USRMSG_STATECHANGED, (LPARAM)m_hWnd);
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CRollupHeader::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if(point.x >= m_rTextRect.left && point.x <= m_rTextRect.right &&
		point.y >= m_rTextRect.top && point.y <= m_rTextRect.bottom)
	{
		m_bExpanded = !m_bExpanded;
		Invalidate();
		GetParent()->SendMessage(WM_COMMAND, USRMSG_STATECHANGED, (LPARAM)m_hWnd);
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}

void CRollupHeader::OnRButtonUp(UINT nFlags, CPoint point)
{
	GetParent()->SendMessage(WM_COMMAND, USRMSG_RIGHTCLICK, (LPARAM)m_hWnd);
	CWnd::OnRButtonUp(nFlags, point);
}

void CRollupHeader::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	m_bInit = TRUE;
}

void CRollupHeader::OnSysColorChange()
{
	CWnd::OnSysColorChange();
#ifndef NEW_LOOK
	m_crBackColor = GetSysColor(COLOR_3DDKSHADOW);
	m_crTextColor = GetSysColor(COLOR_3DHIGHLIGHT);
#else
	m_crBackColor = GetSysColor(COLOR_BTNFACE);
	m_crTextColor = GetSysColor(COLOR_WINDOWTEXT);
#endif NEW_LOOK
	m_crBorderColor = GetSysColor(COLOR_WINDOWFRAME);

	if(m_cpBorderPen.GetSafeHandle())
		m_cpBorderPen.DeleteObject();
	m_cpBorderPen.CreatePen(PS_SOLID, 1, m_crBorderColor);
	if(m_cpArrowPen.GetSafeHandle())
		m_cpArrowPen.DeleteObject();
	m_cpArrowPen.CreatePen(PS_SOLID, 1, m_crTextColor);
	Invalidate();
}

void CRollupHeader::DrawArrow(CDC* pDC, const CPoint* pTopLeft, bool bDown)
{
	CPen *pOldPen;
	pOldPen = pDC->SelectObject(&m_cpArrowPen);
	
	if(bDown)
	{
		int xs = pTopLeft->x;
		int xe = pTopLeft->x + 7;
		
		for(int y = pTopLeft->y; y < pTopLeft->y + 4; y++)
		{	
			pDC->MoveTo(xs,y);
			pDC->LineTo(xe,y);
			xs++;
			xe--;
		}
	}
	else
	{
		int ys = pTopLeft->y;
		int ye = pTopLeft->y + 7;

		for(int x = pTopLeft->x; x < pTopLeft->x + 4; x++)
		{
			pDC->MoveTo(x,ys);
			pDC->LineTo(x,ye);
			ys++;
			ye--;
		}
	}
	
	pDC->SelectObject(pOldPen);
}

BOOL CRollupHeader::OnEraseBkgnd(CDC* pDC)
{
	NOPRM(pDC);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl
CRollupCtrl::CRollupCtrl() : m_iHeaderHeight(0)
{
	m_iHeaderHeight = RUP_HEADERHEIGHT;
	m_iExpandedItems = 0;
	m_iExpandedMsk = 0x00;
}

CRollupCtrl::~CRollupCtrl()
{
	int iCount = m_List.GetSize();

	for(int i = 0; i < iCount; i++)
	{
		RollupEntry *pEntry = m_List.GetAt(i);
		if(pEntry != NULL)
		{
			if(pEntry->pHeader != NULL)
			{
				if(pEntry->pHeader->m_hWnd)
					pEntry->pHeader->DestroyWindow();
				delete pEntry->pHeader;
			}
			if(pEntry->pGripper != NULL)
			{
				if(pEntry->pGripper->m_hWnd)
					pEntry->pGripper->DestroyWindow();
				delete pEntry->pGripper;
			}
			delete pEntry;
		}
	}

	m_List.RemoveAll();
}

BEGIN_MESSAGE_MAP(CRollupCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl message handlers
int CRollupCtrl::InsertItem(LPCTSTR strLeft, LPCTSTR strRight, CWnd *pClient, int iIndex, bool bExpanded)
{
	ASSERT(pClient != NULL);
	ASSERT(pClient->m_hWnd != NULL);
	
	CRollupHeader *pHeader = new CRollupHeader;
	pHeader->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, rand());
	pHeader->SetHeight(m_iHeaderHeight);
	pHeader->Expand(bExpanded);
	pHeader->SetLeftText(strLeft);
	pHeader->SetRightText(strRight);
	
	CRollupGripper *pGripper = new CRollupGripper;
	pGripper->Create(NULL, NULL, WS_CHILD, CRect(0,0,60,15), this, rand());
	
	RollupEntry *pEntry = new RollupEntry;
	pEntry->pHeader	 = pHeader;
	pEntry->pGripper = pGripper;
	pEntry->pClient  = pClient;	
	pEntry->iMinHeight = RUP_ENTRYMINHEIGHT;
	
	int iPos;
	if(iIndex < 0)
		iPos = (int)m_List.Add(pEntry);
	else
	{
		m_List.InsertAt(iIndex, pEntry);
		iPos = iIndex;
	}

	if(bExpanded)
	{	
		m_iExpandedMsk |= (1 << iPos);
		m_iExpandedItems++;
	}

	return iPos;
}

void CRollupCtrl::OnPaint() 
{	
	CPaintDC dc(this);
	
	CRect rClient;
	GetClientRect(&rClient);
	
	int iExpandedItems = 0;
	int iYpos = 0;
	int iCount = (int)m_List.GetSize();

	CDeferPos dp(iCount * 3 - 1);

	for(int i = 0; i < iCount; i++)
	{	
		RollupEntry *pEntry = m_List.GetAt(i);
		ASSERT(pEntry != NULL);
	
		dp.MoveWindow(pEntry->pHeader, rClient.left, rClient.top + iYpos,
						rClient.Width(), m_iHeaderHeight, TRUE); 
	
		iYpos += m_iHeaderHeight;

		if(pEntry->pHeader->IsExpanded())
		{	
			iExpandedItems++;

			double dblHeight = static_cast<double>(rClient.Height() - (m_iHeaderHeight * m_List.GetSize() + RUP_BORDERSIZES * m_iExpandedItems)) * pEntry->adSizes[m_iExpandedMsk] / 100.0;
			int iHeight = static_cast<int>(dblHeight + .5);
			bool bShowGripper;
			
			if(i >= iCount-1) // lastitem?
			{
				iHeight += RUP_BORDERSIZES;

				if(pEntry->pGripper->IsWindowVisible())
				{
					pEntry->pGripper->ShowWindow(SW_HIDE);
					pEntry->pGripper->EnableWindow(FALSE);
				}
				bShowGripper = false;
			}
			else
			{	
				if(m_iExpandedItems > iExpandedItems)
				{
					iHeight -= RUP_GRIPPERHEIGHT + RUP_BORDERSIZES;
					bShowGripper = true;
				}
				else
				{
					bShowGripper = false;
					if(pEntry->pGripper->IsWindowVisible())
					{
						pEntry->pGripper->ShowWindow(SW_HIDE);
						pEntry->pGripper->EnableWindow(FALSE);
					}
				}
			}
	
			CRect rPClient = rClient;
			rPClient.top += iYpos;
			rPClient.bottom = rPClient.top + iHeight - 1;
			CWnd *pClientParent = pEntry->pClient->GetParent();
			if(pClientParent != NULL && pClientParent != this)
			{
				ClientToScreen(&rPClient);
				pClientParent->ScreenToClient(&rPClient);
			}

			pEntry->pClient->EnableWindow();
			dp.MoveWindow(pEntry->pClient, rPClient.left, rPClient.top, rPClient.Width(), rPClient.Height(), TRUE);
			pEntry->pClient->ShowWindow(SW_SHOW); // need OnShowWindow in CInfoListCtrl
			
			CRect rFill = rClient;
			rFill.top = rPClient.bottom;

			iYpos += iHeight + RUP_BORDERSIZES;
			
			if(bShowGripper)
			{
				dp.SetWindowPos(pEntry->pGripper, NULL, 
								rClient.left + rClient.Width() / 2 - RUP_GRIPPERWIDTH / 2,
								rClient.top + iYpos, RUP_GRIPPERWIDTH, RUP_GRIPPERHEIGHT, 
								SWP_NOZORDER|SWP_NOSIZE|SWP_SHOWWINDOW);
				pEntry->pGripper->EnableWindow();

				iYpos += RUP_GRIPPERHEIGHT + RUP_BORDERSIZES;
			}

			rFill.bottom = rClient.top + iYpos;
			dc.FillSolidRect(rFill, GetSysColor(COLOR_BTNFACE));
		}
		else
		{
			if(pEntry->pGripper->IsWindowEnabled())
				pEntry->pGripper->EnableWindow(FALSE);
			if(pEntry->pGripper->IsWindowVisible())
				pEntry->pGripper->ShowWindow(SW_HIDE);
			if(pEntry->pClient->IsWindowEnabled())
				pEntry->pClient->EnableWindow(FALSE);
			if(pEntry->pClient->IsWindowVisible())
				pEntry->pClient->ShowWindow(SW_HIDE);
		}
	}

	if(iExpandedItems == 0)
	{
		CRect rFill = rClient;
		rFill.top = rClient.top + iYpos;
		dc.FillSolidRect(rFill, GetSysColor(COLOR_BTNFACE));
	}
}

void CRollupCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
}

BOOL CRollupCtrl::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam)
	{
		case USRMSG_STATECHANGED:
		{		
			ChildStateChanged((HWND)lParam, true);
			break;
		}
		case USRMSG_GRIPPERMOVE:
		{
			if(Recalc((HWND)lParam))
				Invalidate();			
			break;
		}
		case USRMSG_RIGHTCLICK:
		{
			if(m_List.GetAt(1)->pHeader->m_hWnd == (HWND)lParam)
				GetParent()->SendMessage(WM_COMMAND, USRMSG_SWITCHUPLOADLIST, 0);
			else if(m_List.GetAt(0)->pHeader->m_hWnd == (HWND)lParam)
				GetParent()->SendMessage(WM_COMMAND, USRMSG_CLEARCOMPLETED, 0);
		}
		default: 
		{
			//GetParent()->PostMessage(WM_COMMAND, wParam, lParam);
			break;
		}
	}
	
	return CWnd::OnCommand(wParam, lParam);
}

int CRollupCtrl::SetText(int iItem, const CString &strText, bool bLeft)
{
	if (static_cast<unsigned int>(iItem) >= static_cast<unsigned int>(m_List.GetSize()))
		return -1;
	
	if(bLeft)
		m_List.GetAt(iItem)->pHeader->SetLeftText(strText);
	else
		m_List.GetAt(iItem)->pHeader->SetRightText(strText);

	return 0;
}

int CRollupCtrl::SetHeaderColor(int iItem, int iColor, COLORREF crColor)
{
	if (static_cast<unsigned int>(iItem) >= static_cast<unsigned int>(m_List.GetSize()))
		return -1;

	switch(iColor%3)
	{
		case 0:
			m_List.GetAt(iItem)->pHeader->SetBackColor(crColor);
			break;
		case 1:
			m_List.GetAt(iItem)->pHeader->SetTextColor(crColor);
			break;
		case 2:
			m_List.GetAt(iItem)->pHeader->SetBorderColor(crColor);
			break;
	}

	return 0;
}

BOOL CRollupCtrl::OnEraseBkgnd(CDC* pDC)
{
	NOPRM(pDC);
	return FALSE;
}

int CRollupCtrl::SetItemHeights(int iItem, double *pHeights, unsigned uiSize)
{
	RollupEntry *pEntry;

	if ((static_cast<unsigned>(iItem) >= static_cast<unsigned>(m_List.GetSize())) || (uiSize > sizeof(pEntry->adSizes)))
		return -1;

	pEntry = m_List.GetAt(iItem);
	memcpy2(pEntry->adSizes, pHeights, uiSize);
	return 0;
}

int CRollupCtrl::Recalc(HWND hWnd)
{
	int iCount = m_List.GetSize();

	for(int i = 0; i < iCount; i++)
	{
		RollupEntry *pEntry = m_List.GetAt(i);
		if(pEntry->pGripper->m_hWnd == hWnd)
		{	
			CRect rClientRect;
			GetClientRect(&rClientRect);
						
			RollupEntry *pNextEntry = m_List.GetAt(i+1);		// ToDO: Error-Handling
			if(!pNextEntry->pHeader->IsExpanded())
			{
				if(i+2 < iCount)
					pNextEntry = m_List.GetAt(i+2);
				else
					return 0;
			}
			
			int iMove = pEntry->pGripper->GetLastMove();
			double dCurrEntry = pEntry->adSizes[m_iExpandedMsk];
			double dNextEntry = pNextEntry->adSizes[m_iExpandedMsk];
			double dblTotal = dCurrEntry + dNextEntry;

			// get window heights:
			double dblSize = static_cast<double>(rClientRect.Height() - (m_iHeaderHeight * m_List.GetSize() + RUP_BORDERSIZES * m_iExpandedItems));
			double dblHeight1 = dblSize / 100.0 * dCurrEntry - RUP_GRIPPERHEIGHT - RUP_BORDERSIZES;
			double dblHeight2 = dblSize / 100.0 * dNextEntry;
				
			if(pNextEntry->pHeader->IsExpanded() && i+1 < iCount-1)
				dblHeight2 -= (RUP_GRIPPERHEIGHT + RUP_BORDERSIZES);
			
			// change the heights:
			if(iMove >= 0)
			{
				dblHeight1 -= iMove;
				if(dblHeight1 <= pEntry->iMinHeight)
				{	
					iMove -= pEntry->iMinHeight - static_cast<int>(dblHeight1);
					dblHeight1 = pEntry->iMinHeight;
				}												
				dblHeight2 += iMove;
			}
			else
			{
				iMove = -iMove;
				dblHeight2 -= iMove;
				if(dblHeight2 <= pNextEntry->iMinHeight)
				{
					iMove -= pNextEntry->iMinHeight - static_cast<int>(dblHeight2);
					dblHeight2 = pNextEntry->iMinHeight;
				}
				dblHeight1 += iMove;
			}

			if(pNextEntry->pHeader->IsExpanded() && i+1 < iCount-1)
				dblHeight2 += RUP_GRIPPERHEIGHT + RUP_BORDERSIZES;
			// calc the percentages
			dCurrEntry = (dblHeight1 + RUP_GRIPPERHEIGHT+RUP_BORDERSIZES) / dblSize * 100.0;
			dNextEntry = dblTotal - dCurrEntry;

			if(dCurrEntry < 10)	// should be changed sometime
			{
				dNextEntry -= (10 - dCurrEntry);
				dCurrEntry = 10;
			}
			pEntry->adSizes[m_iExpandedMsk] = dCurrEntry;
			pNextEntry->adSizes[m_iExpandedMsk] = dNextEntry;
		}
	}

	return 1;
}

int CRollupCtrl::SetItemClient(int iItem, CWnd *pClient)
{
	if (static_cast<unsigned int>(iItem) >= static_cast<unsigned int>(m_List.GetSize()))
		return -1;

	m_List.GetAt(iItem)->pClient = pClient;
	return 0;
}

CWnd* CRollupCtrl::GetItemClient(int iItem)
{
	if (static_cast<unsigned int>(iItem) >= static_cast<unsigned int>(m_List.GetSize()))
		return NULL;

	return m_List.GetAt(iItem)->pClient;
}

RollupEntry* CRollupCtrl::GetItem(int iItem)
{
	if (static_cast<unsigned int>(iItem) >= static_cast<unsigned int>(m_List.GetSize()))
		return NULL;

	return m_List.GetAt(iItem);
}

int CRollupCtrl::ExpandItem(int iItem, bool bExpand)
{
	if (static_cast<unsigned int>(iItem) >= static_cast<unsigned int>(m_List.GetSize()))
		return -1;
	
	RollupEntry *pEntry = m_List.GetAt(iItem);

	if (pEntry->pHeader->IsExpanded() != bExpand)
	{
		pEntry->pHeader->Expand(bExpand, false);
		ChildStateChanged(pEntry->pHeader->m_hWnd, false);
	}
	return 0;
}

void CRollupCtrl::ChildStateChanged(HWND hChild, bool bInvalidate)
{
	int iEntry, iCount = m_List.GetSize();

	for(int i = 0; i < iCount; i++)
	{	
		RollupEntry *pEntry = m_List.GetAt(i);

		if(pEntry->pHeader->m_hWnd == hChild)
		{
			iEntry = (1 << i);
			if(pEntry->pHeader->IsExpanded())
			{
				m_iExpandedMsk |= iEntry;
				m_iExpandedItems++;
			}
			else
			{
				m_iExpandedMsk &= ~iEntry;
				m_iExpandedItems--;
			}
			break;
		}
	}

	if(bInvalidate)
		Invalidate();
}

// redirect notify messages to the owner
BOOL CRollupCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	CWnd *pParent = GetParent();

	if(pParent)
	{
		*pResult = pParent->SendMessage(WM_NOTIFY, wParam, lParam);
		return TRUE;
	}
	return CWnd::OnNotify(wParam, lParam, pResult);
}
