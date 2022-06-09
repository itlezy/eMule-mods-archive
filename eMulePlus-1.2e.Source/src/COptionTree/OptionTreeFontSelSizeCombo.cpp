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
#include "OptionTreeFontSelSizeCombo.h"

// Added Headers
#include "OptionTreeFontSelGlobals.h"
#include <afximpl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeFontSelSizeCombo

COptionTreeFontSelSizeCombo::COptionTreeFontSelSizeCombo()
{
}

COptionTreeFontSelSizeCombo::~COptionTreeFontSelSizeCombo()
{
}


BEGIN_MESSAGE_MAP(COptionTreeFontSelSizeCombo, CComboBox)
	//{{AFX_MSG_MAP(COptionTreeFontSelSizeCombo)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionTreeFontSelSizeCombo message handlers

int COptionTreeFontSelSizeCombo::AddSize(int nPointSize, long lfHeight)
{
	if (lfHeight == 0)
	{
		lfHeight = MulDiv(-afxData.cyPixelsPerInch, nPointSize, 72);
	}

	CString str;
	wsprintf(str.GetBuffer(16), _T("%d"), nPointSize);
	str.ReleaseBuffer();

	int nMaxEntries = GetCount();
	int nEntry;

	// We use positive height values for non-truetype fonts, negitive for true type
	if (lfHeight > 0)
	{
		for (nEntry = 0; nEntry < nMaxEntries; nEntry++)
		{
			int iComp = (int)(lfHeight - GetHeight(nEntry));
			if (!iComp)
				return CB_ERR;
			if (iComp < 0)
				break;
		}
	}
	else
	{
		for (nEntry = 0; nEntry < nMaxEntries; nEntry++)
		{
			int iComp = (int)(lfHeight - GetHeight(nEntry));
			if (!iComp)
				return CB_ERR;
			if (iComp > 0)
				break;
		}
	}

	if (nEntry == nMaxEntries)
		nEntry = -1;
	nEntry = InsertString(nEntry, str);
	if (nEntry != CB_ERR)
		SetItemData(nEntry, (DWORD)lfHeight);

	return nEntry;
}

void COptionTreeFontSelSizeCombo::GetPointSize(CY& cy)
{
	TCHAR szText[20];
	GetWindowText(szText, 20);
	cy.Lo = 0;
	cy.Hi = 0;
	_AfxCyFromString(cy, szText);
}

long COptionTreeFontSelSizeCombo::GetHeight(int nSel)
{
	if (nSel == -1)
		nSel = GetCurSel();

	if (nSel == -1)
	{
		TCHAR szText[20];
		GetWindowText(szText, 20);
		nSel = FindString(-1, szText);
		if (nSel == CB_ERR)
		{
			CY cyTmp;
			cyTmp.Lo = 0;
			cyTmp.Hi = 0;
			_AfxCyFromString(cyTmp, szText);
			int PointSize = (int)((cyTmp.Lo + 5000) / 10000);
			if (PointSize != 0)
				return MulDiv(-afxData.cyPixelsPerInch, PointSize, 72);
			else
				nSel = 0;
		}
	}

	return (long) GetItemData(nSel);
}

void COptionTreeFontSelSizeCombo::UpdateLogFont(LPLOGFONT lpLF, int nSel)
{
	ASSERT(lpLF);

	lpLF->lfHeight = (int)GetHeight(nSel);
	lpLF->lfWidth = 0;
}