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
#include "OptionTreeItemFont.h"

// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionTreeItemFont::COptionTreeItemFont()
{
	// Initialize variables
	m_crDefTextColor = RGB(0, 0, 0);
	m_crTextColor = RGB(0, 0, 0);
	m_dwOptions = NULL;
	m_strCustomSample = _T("");
	m_pApplyWindow = NULL;
	m_lDefaultHeight = 0;

	// Set item type
	SetItemType(OT_ITEM_FONT);
}

COptionTreeItemFont::~COptionTreeItemFont()
{

}

void COptionTreeItemFont::DrawAttribute(CDC *pDC, const RECT &rcRect)
{
	// Make sure options aren't NULL
	if (m_otOption == NULL)
	{
		return;
	}

	// Declare variables
	CFont *pOldFont;
	COLORREF crOld;
	int nOldBack;
	CRect rcText, rcClient;
	CString strText;
	CString strSample;
	LOGFONT lf;
	CFont fFont;
	COLORREF crOldBack;

	// Get window rect
	rcClient = rcRect;

	// Create font
	if (fFont.CreateFontIndirect(&m_lfFont) == FALSE)
	{
		return;
	}

	// Select font
	pOldFont = pDC->SelectObject(&fFont);
	
	// Set text color
	if (IsReadOnly() == TRUE || m_otOption->IsWindowEnabled() == FALSE)
	{
		crOld = pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}
	else
	{
		crOld = pDC->SetTextColor(m_crTextColor);
	}

	// Set background mode
	nOldBack = pDC->SetBkMode(TRANSPARENT);

	// Set background color
	crOldBack = pDC->SetBkColor(GetBackgroundColor());	

	// Load sample text
	if (GetOption(OT_FS_CUSTOMSAMPLE) == TRUE)
	{
		strSample = m_strCustomSample;
	}
	else if (GetOption(OT_FS_FONTNAMESAMPLE) == TRUE)
	{
		fFont.GetLogFont(&lf);
		strSample.Format("%s", lf.lfFaceName);
	}
	else
	{
		strSample.LoadString(AFX_IDS_SAMPLETEXT);	
	}

	// Get text rectangle
	rcText.left  = rcRect.left + 1;
	rcText.right = rcRect.right;
	rcText.top = rcRect.top;
	rcText.bottom = rcRect.bottom;

	// Draw text
	pDC->DrawText(strSample, rcText, DT_SINGLELINE | DT_VCENTER);
	pDC->DrawText(strSample, rcText, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
	
	// Restore GDI ojects
	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(crOld);
	pDC->SetBkMode(nOldBack);
	pDC->SetBkColor(crOldBack);
}

void COptionTreeItemFont::OnCommit()
{
}

void COptionTreeItemFont::OnRefresh()
{
}

void COptionTreeItemFont::OnMove()
{
}

void COptionTreeItemFont::OnActivate()
{
	// Declare variables
	CWnd *pWnd;

	// Set apply window
	if (m_pApplyWindow != NULL)
	{
		pWnd = m_pApplyWindow;
	}
	else
	{
		pWnd = m_otOption;
	}

	// Show dialog
	COptionTreeFontSel fDialog(m_dwOptions, m_lfFont, m_lfDefaultFont, pWnd);
	// -- Default Font Color
	fDialog.SetDefaultTextColor(m_crDefTextColor);
	// -- Font Color
	fDialog.SetTextColor(m_crTextColor);
	// -- Control ID
	fDialog.SetControlID(GetCtrlID());

	// Show dialog
	if (fDialog.DoModal() == ID_FONTSEL_OK)	
	{
		// -- Get font
		fDialog.GetLogFont(m_lfFont);

		// -- Get font color
		m_crTextColor = fDialog.GetTextColor();

		// -- Set item height
		if ((abs(m_lfFont.lfHeight) + (OT_SPACE * 2)) > m_lDefaultHeight)
		{
			SetItemHeight(abs(m_lfFont.lfHeight) + (OT_SPACE * 2));
			SetDrawMultiline(TRUE);
		}
		else
		{
			SetItemHeight(m_lDefaultHeight);
			SetDrawMultiline(FALSE);
		}
	}

	// Update items
	if (m_otOption != NULL)
	{
		m_otOption->UpdatedItems();
	}
}

void COptionTreeItemFont::CleanDestroyWindow()
{
}

void COptionTreeItemFont::OnDeSelect()
{
}

void COptionTreeItemFont::OnSelect()
{
}

BOOL COptionTreeItemFont::CreateFontItem(CHARFORMAT cfFont, COLORREF crFontColor, CHARFORMAT cfDefaultFont, COLORREF crDefaultFontColor, DWORD dwOptions)
{
	// Declare variables
	LOGFONT lfFont, lfDefaultFont;

	// Make sure options is not NULL
	if (m_otOption == NULL)
	{
		return FALSE;
	}

	// Save options
	m_dwOptions = dwOptions;

	// Set font
	ConvertFont(lfFont, cfFont);
	SetCurFont(lfFont);

	// Set default font
	ConvertFont(lfDefaultFont, cfDefaultFont);
	SetDefaultFont(lfDefaultFont);

	// Set color
	SetTextColor(crFontColor);

	// Set default font color
	SetDefaultTextColor(crDefaultFontColor);

	// Get default height
	m_lDefaultHeight = GetHeight();

	return TRUE;
}

BOOL COptionTreeItemFont::CreateFontItem(CHARFORMAT cfFont, COLORREF crFontColor, DWORD dwOptions)
{
	// Declare variables
	LOGFONT lfFont;

	// Make sure options is not NULL
	if (m_otOption == NULL)
	{
		return FALSE;
	}	

	// Save options
	m_dwOptions = dwOptions;

	// Set font
	ConvertFont(lfFont, cfFont);
	SetCurFont(lfFont);

	// Set color
	SetTextColor(crFontColor);

	// Get default height
	m_lDefaultHeight = GetHeight();

	return TRUE;
}

BOOL COptionTreeItemFont::CreateFontItem(LOGFONT lfFont, COLORREF crFontColor, DWORD dwOptions)
{
	// Declare variables
	CRect rcButton;

	// Make sure options is not NULL
	if (m_otOption == NULL)
	{
		return FALSE;
	}

	// Save options
	m_dwOptions = dwOptions;

	// Set font
	SetCurFont(lfFont);

	// Set color
	SetTextColor(crFontColor);

	// Get default height
	m_lDefaultHeight = GetHeight();

	return TRUE;
}

BOOL COptionTreeItemFont::CreateFontItem(LOGFONT lfFont, COLORREF crFontColor, LOGFONT lfDefaultFont, COLORREF crDefaultFontColor, DWORD dwOptions)
{
	// Make sure options is not NULL
	if (m_otOption == NULL)
	{
		return FALSE;
	}

	// Save options
	m_dwOptions = dwOptions;

	// Set font
	SetCurFont(lfFont);

	// Set default font
	SetDefaultFont(lfDefaultFont);

	// Set color
	SetTextColor(crFontColor);

	// Set default font color
	SetDefaultTextColor(crDefaultFontColor);

	// Get default height
	m_lDefaultHeight = GetHeight();

	return TRUE;
}

void COptionTreeItemFont::SetCurFont(LOGFONT lf)
{
	m_lfFont = lf;
}

void COptionTreeItemFont::SetDefaultFont(LOGFONT lf)
{
	m_lfDefaultFont = lf;
}

void COptionTreeItemFont::ConvertFont(CHARFORMAT &cfFont, LOGFONT &lf)
{
	// Declare variables
	CDC dc;
	dc.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	long yPerInch = dc.GetDeviceCaps(LOGPIXELSY);

	// Convert Font
	cfFont.dwMask = CFM_COLOR | CFM_SIZE | CFM_FACE | CFM_UNDERLINE | CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT;
	cfFont.dwEffects = 0;
	// -- Face
	strcpy(cfFont.szFaceName, lf.lfFaceName);
	// -- Pitch and family
	cfFont.bPitchAndFamily = lf.lfPitchAndFamily;
	// -- Size
	cfFont.yHeight = ((1440 / yPerInch) * abs(lf.lfHeight));
	// -- Bold
	if (lf.lfWeight >= FW_BOLD)
	{
		cfFont.dwEffects |= CFE_BOLD;
	}
	// -- Italic
	if (lf.lfItalic == TRUE)
	{
		cfFont.dwEffects |= CFE_ITALIC;
	}
	// -- Underline
	if (lf.lfUnderline == TRUE)
	{
		cfFont.dwEffects |= CFE_UNDERLINE;
	}
	// -- Strikeout
	if (lf.lfStrikeOut == TRUE)
	{
		cfFont.dwEffects |= CFE_STRIKEOUT;
	}
	// -- Character set
	cfFont.bCharSet = lf.lfCharSet;
	cfFont.dwEffects |= CFM_CHARSET;
	// -- Text color
	cfFont.crTextColor = GetTextColor();
}

void COptionTreeItemFont::ConvertFont(LOGFONT &lf, CHARFORMAT &cfFont)
{
	// Declare variables
	CDC dc;
	dc.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	long yPerInch = dc.GetDeviceCaps(LOGPIXELSY);


	// Create log font
	// -- Font name
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = cfFont.bPitchAndFamily;
	strcpy(lf.lfFaceName, cfFont.szFaceName);
	// -- Size
	lf.lfHeight = abs(((cfFont.yHeight * yPerInch) / 1440));
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	// -- Bold
	if (cfFont.dwEffects & CFE_BOLD)
	{
		lf.lfWeight = FW_BOLD;
	}
	else
	{
		lf.lfWeight = FW_NORMAL;
	}
	// -- Strikeout
	if (cfFont.dwEffects & CFE_STRIKEOUT)
	{
		lf.lfStrikeOut = TRUE;
	}
	else
	{
		lf.lfStrikeOut = FALSE;
	}
	// -- Underline
	if (cfFont.dwEffects & CFE_UNDERLINE)
	{
		lf.lfUnderline = TRUE;
	}
	else
	{
		lf.lfUnderline = FALSE;
	}
	// -- Italic
	if (cfFont.dwEffects & CFE_ITALIC)
	{
		lf.lfItalic = TRUE;
	}
	else
	{
		lf.lfItalic = FALSE;
	}
}

void COptionTreeItemFont::SetCurFont(CHARFORMAT cfFont)
{
	// Declare variables
	LOGFONT lf = {0};

	// Copy font
	ConvertFont(lf, cfFont);

	// Save font
	m_lfFont = lf;
}

void COptionTreeItemFont::SetDefaultFont(CHARFORMAT cfFont)
{
	// Declare variables
	LOGFONT lf = {0};

	// Copy font
	ConvertFont(lf, cfFont);

	// Save font
	m_lfDefaultFont = lf;
}

void COptionTreeItemFont::SetDefaultTextColor(COLORREF crColor)
{
	m_crDefTextColor = crColor;
}

void COptionTreeItemFont::SetTextColor(COLORREF crColor)
{
	m_crTextColor = crColor;
}

COLORREF COptionTreeItemFont::GetTextColor()
{
	return m_crTextColor;
}

COLORREF COptionTreeItemFont::GetDefaultTextColor()
{
	return m_crDefTextColor;
}

void COptionTreeItemFont::SetCustomSample(CString strSample)
{
	m_strCustomSample = strSample;
}

void COptionTreeItemFont::SetApplyWindow(CWnd *pWnd)
{
	m_pApplyWindow = pWnd;
}

BOOL COptionTreeItemFont::GetOption(DWORD dwOption)
{
	// Return option
	return (m_dwOptions & dwOption) ? TRUE : FALSE;
}

void COptionTreeItemFont::SetOption(DWORD dwOption, BOOL bSet)
{
	// Set option
	if (bSet == TRUE)
	{
		m_dwOptions |= dwOption;
	}
	else
	{
		m_dwOptions &= ~dwOption;
	}
}
