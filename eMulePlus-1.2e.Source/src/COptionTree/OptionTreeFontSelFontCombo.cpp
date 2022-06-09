// COptionTree
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgement.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 1999-2002 
// ComputerSmarts.net 
// mattrmiller@computersmarts.net

#include "stdafx.h"
#include "OptionTreeFontSelFontCombo.h"

// Added Headers
#include "OptionTreeFontSelGlobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeFontSelFontCombo

COptionTreeFontSelFontCombo::COptionTreeFontSelFontCombo()
{
	// Initialize variables
	m_bmpTrueType.LoadBitmap(AFX_IDB_TRUETYPE);
	_AfxInitMaskFromBitmap(&m_bmpTrueType, &m_bmpMask);
}

COptionTreeFontSelFontCombo::~COptionTreeFontSelFontCombo()
{
	// Reallocate
	if (m_bmpTrueType.GetSafeHandle() != NULL)
	{
		m_bmpTrueType.DeleteObject();
	}
	if (m_bmpMask.GetSafeHandle() != NULL)
	{
		m_bmpMask.DeleteObject();
	}
}


BEGIN_MESSAGE_MAP(COptionTreeFontSelFontCombo, CComboBox)
	//{{AFX_MSG_MAP(COptionTreeFontSelFontCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionTreeFontSelFontCombo message handlers

int COptionTreeFontSelFontCombo::AddFont(LOGFONT *pLF, DWORD FontType)
{
	int nEntry;
	OT_FONTSEL_ITEM* pFontItem = NULL;

	// Font already in the combobox
	if (FindString(-1, (LPCTSTR) pLF->lfFaceName) != CB_ERR)
		return CB_ERR;

	// allocate some memory for the OT_FONTSEL_ITEM structure
	TRY
	{
		pFontItem = new OT_FONTSEL_ITEM;
	}
	CATCH(CMemoryException, e)
	{
		return CB_ERR;
	}
	END_CATCH

	ASSERT(pFontItem);
	pFontItem->lf = *pLF;
	pFontItem->dwFontType = FontType;

	nEntry = AddString((LPCTSTR) pFontItem->lf.lfFaceName);

	if (nEntry == CB_ERR)
		delete pFontItem;
	else
		SetItemData(nEntry, (DWORD) pFontItem);

	return nEntry;
}

OT_FONTSEL_ITEM* COptionTreeFontSelFontCombo::GetFontItem(int nSel)
{
	if (nSel == -1)
		nSel = GetCurSel();

	if (nSel == -1)
	{
		CString str;

		GetWindowText(str);
		nSel = FindString(-1, str);
		if (nSel == CB_ERR)
			nSel = 0;
	}

	ASSERT(GetItemData(nSel));
	return (OT_FONTSEL_ITEM*) GetItemData(nSel);
}

LPLOGFONT COptionTreeFontSelFontCombo::GetLogFont(int nSel)
{
	return &GetFontItem(nSel)->lf;
}

DWORD COptionTreeFontSelFontCombo::GetFontType(int nSel)
{
	return GetFontItem(nSel)->dwFontType;
}

CString COptionTreeFontSelFontCombo::GetCurrentName()
{
	CString str;
	GetWindowText(str);
	return str;
}

void COptionTreeFontSelFontCombo::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	ASSERT(lpDIS->CtlType == ODT_COMBOBOX);

	// Make sure this is a *real* item
	if (lpDIS->itemID == -1)
		return;

	// Get drawing device
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	// Pointer to a FONTITEM storied in item data
	OT_FONTSEL_ITEM* pFI = (OT_FONTSEL_ITEM*)lpDIS->itemData;    
	LOGFONT* pLF = &pFI->lf;
	COLORREF crBk, crText;
	TEXTMETRIC tm;
	int x, y;

	// Calculate the colors to use
	crBk = pDC->SetBkColor(GetSysColor(lpDIS->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));
	crText = pDC->SetTextColor(GetSysColor(lpDIS->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

	// Calculate the position of the text
	pDC->GetTextMetrics(&tm);
	x = LOWORD(GetDialogBaseUnits()) / 4;
	y = (lpDIS->rcItem.bottom + lpDIS->rcItem.top - tm.tmHeight) / 2;

	// Draw the text
	pDC->ExtTextOut(lpDIS->rcItem.left + DX_BITMAP + 2 * x, y, ETO_CLIPPED | ETO_OPAQUE,
		&lpDIS->rcItem,(LPCTSTR) pLF->lfFaceName,
		lstrlen((LPCTSTR) pLF->lfFaceName), NULL);

	// Put the colors back as they were
	pDC->SetTextColor(crText);
	pDC->SetBkColor(crBk);

	// Draw the TrueType bitmap
	if (pFI->dwFontType & TRUETYPE_FONTTYPE)
	{
		int dy;
		dy = ((lpDIS->rcItem.bottom - lpDIS->rcItem.top) - DY_BITMAP) / 2;
		_AfxDrawMaskedBitmap(pDC, &m_bmpTrueType, &m_bmpMask,
			x, lpDIS->rcItem.top + dy, DX_BITMAP, DY_BITMAP);
	}

	// Draw the focus rect if needed
	if (lpDIS->itemState & ODS_FOCUS)
		pDC->DrawFocusRect(&lpDIS->rcItem);
}

void COptionTreeFontSelFontCombo::DeleteItem(LPDELETEITEMSTRUCT lpDIS)
{
	OT_FONTSEL_ITEM* pFI;

	if (lpDIS->itemID == -1)
		return;

	ASSERT(lpDIS->CtlType == ODT_COMBOBOX);

	pFI = GetFontItem(lpDIS->itemID);

	// Free the OT_FONTSEL_ITEM created in COptionTreeFontSelFontCombo::AddFont()
	ASSERT(pFI);
	delete pFI;
}

BOOL COptionTreeFontSelFontCombo::SelectFontName(CString strName)
{
	// Validate
	if (!::IsWindow(GetSafeHwnd()))
	{
		return FALSE;
	}
	
	// Declare variables
	int nSel;

	// Find string
	nSel = FindString(-1, strName);

	// Set selection
	if (nSel != CB_ERR)
	{	
		if (SetCurSel(nSel) != CB_ERR)
		{
			return TRUE;
		}
	}

	return FALSE;
}
