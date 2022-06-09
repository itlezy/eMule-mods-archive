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
#include "OptionTreeFontSel.h"

// Added Headers
#include "CommonRes.h"
#include <afximpl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeFontSel dialog


COptionTreeFontSel::COptionTreeFontSel(DWORD dwFlags, LOGFONT lfFont, LOGFONT lfDefault, CWnd* pParent /*=NULL*/): CDialog(COptionTreeFontSel::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionTreeFontSel)
	//}}AFX_DATA_INIT
	
	// Initialize variables
	m_strCustomSample = _T("");
	m_bStrikeOut = FALSE;
	m_bUnderline = FALSE;
	m_lfFont = lfFont;
	m_lfDefault = lfDefault;
	m_dwDialogFlags = dwFlags;
	m_uControlID = 0;
}

COptionTreeFontSel::COptionTreeFontSel(DWORD dwFlags, LOGFONT lfFont, CWnd* pParent /*=NULL*/): CDialog(COptionTreeFontSel::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionTreeFontSel)
	//}}AFX_DATA_INIT
	
	// Initialize variables
	m_strCustomSample = _T("");
	m_bStrikeOut = FALSE;
	m_bUnderline = FALSE;
	m_lfFont = lfFont;
	m_lfDefault = lfFont;
	m_dwDialogFlags = dwFlags;
	m_uControlID = 0;
}

COptionTreeFontSel::COptionTreeFontSel(DWORD dwFlags, CHARFORMAT cfFont, CHARFORMAT cfDefault, CWnd* pParent /*=NULL*/): CDialog(COptionTreeFontSel::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionTreeFontSel)
	//}}AFX_DATA_INIT
	
	// Initialize variables
	m_strCustomSample = _T("");
	m_bStrikeOut = FALSE;
	m_bUnderline = FALSE;
	ConvertFont(m_lfFont, cfFont);
	ConvertFont(m_lfDefault, cfDefault);
	m_dwDialogFlags = dwFlags;
	m_uControlID = 0;

	// Set text color
	SetTextColor(cfFont.crTextColor);
	SetDefaultTextColor(cfDefault.crTextColor);
}

COptionTreeFontSel::COptionTreeFontSel(DWORD dwFlags, CHARFORMAT cfFont, CWnd* pParent /*=NULL*/): CDialog(COptionTreeFontSel::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionTreeFontSel)
	//}}AFX_DATA_INIT
	
	// Initialize variables
	m_strCustomSample = _T("");
	m_bStrikeOut = FALSE;
	m_bUnderline = FALSE;
	ConvertFont(m_lfFont, cfFont);
	ConvertFont(m_lfDefault, cfFont);
	m_dwDialogFlags = dwFlags;
	m_uControlID = 0;

	// Set text color
	SetTextColor(cfFont.crTextColor);
	SetDefaultTextColor(cfFont.crTextColor);
}

COptionTreeFontSel::~COptionTreeFontSel()
{
	// Reallocate
	if (m_fSampleFont.GetSafeHandle() != NULL)
	{
		m_fSampleFont.DeleteObject();
	}

}


void COptionTreeFontSel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionTreeFontSel)
	DDX_Control(pDX, ID_STATIC_SAMPLE, m_ctlSample);
	DDX_Control(pDX, ID_COMBO_STYLE, m_ctlFontStyles);
	DDX_Control(pDX, ID_COMBO_SIZE, m_ctlFontSizes);
	DDX_Control(pDX, ID_COMBO_FONT, m_ctlFontNames);
	DDX_Control(pDX, ID_CHECK_UNDERLINE, m_ctlUnderline);
	DDX_Control(pDX, ID_CHECK_STRIKEOUT, m_ctlStrikeout);
	DDX_Control(pDX, ID_FONTSEL_DEFAULT, m_ctlButtonDefault);
	DDX_Control(pDX, ID_FONTSEL_APPLY, m_ctlButtonApply);
	DDX_Control(pDX, ID_FONTSEL_COLOR, m_ctlButtonColor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionTreeFontSel, CDialog)
	//{{AFX_MSG_MAP(COptionTreeFontSel)
	ON_WM_PAINT()
	ON_BN_CLICKED(ID_FONTSEL_OK, OnButtonOK)
	ON_BN_CLICKED(ID_FONTSEL_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(ID_FONTSEL_DEFAULT, OnButtonDefault)
	ON_BN_CLICKED(ID_FONTSEL_APPLY, OnButtonApply)
	ON_CBN_EDITUPDATE(ID_COMBO_FONT, OnEditUpdateFontNames)
	ON_CBN_EDITUPDATE(ID_COMBO_SIZE, OnEditUpdateFontSizes)
	ON_CBN_SELCHANGE(ID_COMBO_FONT, OnSelChangeFontNames)
	ON_CBN_SELCHANGE(ID_COMBO_SIZE, OnSelChangeFontSizes)
	ON_CBN_SELCHANGE(ID_COMBO_STYLE, OnSelChangeFontStyles)
	ON_CBN_EDITCHANGE(ID_COMBO_STYLE, OnEditChangeFontStyles)
	ON_CBN_SELCHANGE(ID_COMBO_SCRIPT, OnSelChangeFontScript)	
	ON_BN_CLICKED(ID_CHECK_STRIKEOUT, OnStrikeout)
	ON_BN_CLICKED(ID_CHECK_UNDERLINE, OnUnderline)
	ON_MESSAGE(OT_COLOR_SELENDOK, OnSelChangeColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionTreeFontSel message handlers

BOOL COptionTreeFontSel::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Microsoft makes it really hard to customize the Font dialog, so I created my own.

	// Declare variables
	CRect rcTemp;

	// Enable buttons
	// -- Default
	if (m_dwDialogFlags & OT_FS_USEDEFAULT)
	{
		m_ctlButtonDefault.ModifyStyle(0, WS_VISIBLE);

		// -- Move default button uo
		if (!(m_dwDialogFlags & OT_FS_USEAPPLY))
		{
			m_ctlButtonApply.GetWindowRect(rcTemp);
			ScreenToClient(rcTemp);
			m_ctlButtonDefault.MoveWindow(rcTemp);
		}
	}
	// -- Apply
	if (m_dwDialogFlags & OT_FS_USEAPPLY)
	{
		m_ctlButtonApply.ModifyStyle(0, WS_VISIBLE);
	}
	// -- Text Color
	if (m_dwDialogFlags & OT_FS_NOTEXTCOLOR)
	{
		m_ctlButtonColor.EnableWindow(FALSE);
	}
	// -- Effects
	if (m_dwDialogFlags & OT_FS_NOEFFECTS)
	{
		m_ctlUnderline.EnableWindow(FALSE);
		m_ctlStrikeout.EnableWindow(FALSE);
	}
	// -- Styles
	if (m_dwDialogFlags & OT_FS_NOSTYLES)
	{
		m_ctlFontStyles.EnableWindow(FALSE);
	}
	// -- Face
	if (m_dwDialogFlags & OT_FS_NOFACE)
	{
		m_ctlFontNames.EnableWindow(FALSE);
	}

	// Load everything
	OnObjectsChanged();

	// Select current font
	SelectCurFont(m_lfFont);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionTreeFontSel::FillFaceNameList()
{
	// Declare variables
	CClientDC dc(NULL);

	// Validate controls
	if (ValidateControls() == FALSE)
	{
		return;
	}
	
	// Clear the list
	m_ctlFontNames.ResetContent();

	// Create a DC to enumerate
	EnumFontFamilies(dc.GetSafeHdc(), (LPCTSTR) NULL, (FONTENUMPROC)COptionTreeFontSel::EnumFontFamiliesCallBack, (LPARAM) this);

	// Select the first one
	if (m_ctlFontNames.SetCurSel(0) != CB_ERR)
	{
		// Fill the size list
		FillSizeStyleList();
	}
	else
	{
		m_ctlFontNames.EnableWindow(FALSE);
		m_ctlFontSizes.EnableWindow(FALSE);
		m_ctlFontStyles.EnableWindow(FALSE);
		m_ctlUnderline.EnableWindow(FALSE);
		m_ctlStrikeout.EnableWindow(FALSE);
	}
}

void COptionTreeFontSel::FillSizeStyleList()
{
	// Declare variables
	CClientDC dc(NULL);
	CString strFaceName;
	int nSel;

	// Validate controls
	if (ValidateControls() == FALSE)
	{
		return;
	}
	
	// Clear the size list
	m_ctlFontSizes.ResetContent();
	m_ctlFontStyles.ResetContent();
	m_nStyles = 0L;

	// Fill with "real" sizes
	nSel = m_ctlFontNames.GetCurSel();
	if (nSel == CB_ERR)
	{
		return;
	}
	m_ctlFontNames.GetLBText(nSel, strFaceName);

	// Enumerate
	EnumFontFamilies(dc.GetSafeHdc(), (LPCTSTR) strFaceName, (FONTENUMPROC) COptionTreeFontSel::EnumFontFamiliesCallBack2, (LPARAM) this);

	// Check if we have a font that is either a vector or Truettype font
	if (m_ctlFontNames.GetFontType() != RASTER_FONTTYPE)
	{
		// -- Fill with "common" sizes
		for (int i = 0; i < _countof(_afxTTDefaults); i++)
		{
			m_ctlFontSizes.AddSize(_afxTTDefaults[i], 0);
		}
	}

	// See what fonts are native
	BOOL bRegular = (BOOL)(m_nStyles & NTM_REGULAR);
	BOOL bBold = (BOOL)(m_nStyles & NTM_BOLD);
	BOOL bItalic = (BOOL)(m_nStyles & NTM_ITALIC);
	BOOL bBoldItalic = (BOOL)((m_nStyles & NTM_BOLD) && (m_nStyles & NTM_ITALIC));

	// Allow for "synthesized" italic && bold variants
	if (bRegular)
	{
		bBold = bItalic = TRUE;
	}
	if (bBold || bItalic)
	{
		bBoldItalic = TRUE;
	}

	// Fill the styles list box
	CString strStyle;
	int nEntry;
	if (bRegular)
	{
		strStyle.LoadString(AFX_IDS_REGULAR);
		nEntry = m_ctlFontStyles.AddString(strStyle);
		m_ctlFontStyles.SetItemData(nEntry, (DWORD)NTM_REGULAR);
	}
	if (bBold)
	{
		strStyle.LoadString(AFX_IDS_BOLD);
		nEntry = m_ctlFontStyles.AddString(strStyle);
		m_ctlFontStyles.SetItemData(nEntry, (DWORD)NTM_BOLD);
	}
	if (bItalic)
	{
		strStyle.LoadString(AFX_IDS_ITALIC);
		nEntry = m_ctlFontStyles.AddString(strStyle);
		m_ctlFontStyles.SetItemData(nEntry, (DWORD)NTM_ITALIC);
	}
	if (bBoldItalic)
	{
		strStyle.LoadString(AFX_IDS_BOLDITALIC);
		nEntry = m_ctlFontStyles.AddString(strStyle);
		m_ctlFontStyles.SetItemData(nEntry, (DWORD)NTM_ITALIC | NTM_BOLD);
	}

	// Set the point size
	if (m_ctlFontSizes.FindString(-1, m_strFontSize) != CB_ERR)
	{
		nEntry = m_ctlFontSizes.SelectString(-1, m_strFontSize);
		if (nEntry == CB_ERR)
		{
			return;
		}
	}
	else
	{
		// -- Point size is not in the list so just fill the edit box
		// and don't select anything from the list
		m_ctlFontSizes.SetCurSel(-1);
		m_ctlFontSizes.SetWindowText(m_strFontSize);
	}

	// Set the styles combo box selection
	BOOL bFound = FALSE;
	int nMaxEntries = m_ctlFontStyles.GetCount();
	for (int nEntry3 = 0; nEntry3 < nMaxEntries; nEntry3++)
	{
		if (m_ctlFontStyles.GetItemData(nEntry3) == m_nActualStyle)
		{
			m_ctlFontStyles.SetCurSel(nEntry3);
			bFound = TRUE;
		}
	}

	if (bFound == FALSE)
	{
		// -- Set style to regular
		m_ctlFontStyles.SetCurSel(0);      
		m_nCurrentStyle = NTM_REGULAR;
	}
	else
	{
		m_nCurrentStyle = m_nActualStyle;
	}

	// Redraw the sample
	UpdateSampleFont();
}

int CALLBACK COptionTreeFontSel::EnumFontFamiliesCallBack(ENUMLOGFONT *lpelf, NEWTEXTMETRIC *, int nFontType, LPARAM lParam)
{
	// Declare variables
	COptionTreeFontSel *pDlg = (COptionTreeFontSel *)lParam;
	
	// Validate
	ASSERT(pDlg);

	// Setup styles
	// -- True type
	if (nFontType & TRUETYPE_FONTTYPE)
	{
		// -- Add font
		pDlg->m_ctlFontNames.AddFont(&lpelf->elfLogFont, nFontType);
	}
	// -- Raster
	else
	{
		// -- True type only
		if (pDlg->GetDialogFlags() & OT_FS_TTONLY)
		{
			return 1;
		}

		// -- Add font
		pDlg->m_ctlFontNames.AddFont(&lpelf->elfLogFont, nFontType);
	}


	return 1;
}

int CALLBACK COptionTreeFontSel::EnumFontFamiliesCallBack2(ENUMLOGFONT* lpelf, NEWTEXTMETRIC* lpntm, int nFontType, LPARAM lParam)
{
	// Declare variables
	COptionTreeFontSel *pDlg = (COptionTreeFontSel *)lParam;
	
	// Validate
	ASSERT(pDlg);
	
	// Setup styles
	// -- True type
	if (nFontType & TRUETYPE_FONTTYPE)
	{
		if (!(lpntm->ntmFlags & (NTM_BOLD | NTM_ITALIC)))
		{
			pDlg->m_nStyles |= NTM_REGULAR;
		}

		if (lpntm->ntmFlags & NTM_ITALIC)
		{
			pDlg->m_nStyles |= NTM_ITALIC;
		}

		if (lpntm->ntmFlags & NTM_BOLD)
		{
			pDlg->m_nStyles |= NTM_BOLD;
		}
	}
	// -- Raster
	else
	{
		// -- True type only
		if (pDlg->GetDialogFlags() & OT_FS_TTONLY)
		{
			return 1;
		}

		if (nFontType & RASTER_FONTTYPE)
		{
			int nHeight = lpntm->tmHeight - lpntm->tmInternalLeading;
			pDlg->m_ctlFontSizes.AddSize(MulDiv(nHeight, 72, afxData.cyPixelsPerInch), nHeight);
		}

		if (lpelf->elfLogFont.lfWeight >= FW_BOLD && lpelf->elfLogFont.lfItalic)
		{
			pDlg->m_nStyles |= NTM_BOLD | NTM_ITALIC;
		}
		else if (lpelf->elfLogFont.lfWeight >= FW_BOLD)
		{
			pDlg->m_nStyles |= NTM_BOLD;
		}
		else if (lpelf->elfLogFont.lfItalic)
		{
			pDlg->m_nStyles |= NTM_ITALIC;
		}
		else
		{
			pDlg->m_nStyles |= NTM_REGULAR;
		}
	}


	return 1;
}

void COptionTreeFontSel::UpdateSampleFont()
{
	// Validate
	ASSERT(m_ctlFontNames.GetFontItem());

	// Declare variables
	LOGFONT lf;
	CRect rcSample;
	CString strCharSet;
    memset(&lf, 0, sizeof(LOGFONT));

	// Validate controls
	if (ValidateControls() == FALSE)
	{
		return;
	}

	// Get log font
	lf = *m_ctlFontNames.GetLogFont();

	// Get size
	m_ctlFontSizes.UpdateLogFont(&lf);

	// Handle styles
	if (m_nCurrentStyle & NTM_BOLD)
	{
		lf.lfWeight = FW_BOLD;
	}
	else
	{
		lf.lfWeight = FW_REGULAR;
	}
	if (m_nCurrentStyle & NTM_ITALIC)
	{
		lf.lfItalic = TRUE;
	}
	else
	{
		lf.lfItalic = FALSE;
	}
	lf.lfStrikeOut = (unsigned char)m_bStrikeOut;
	lf.lfUnderline = (unsigned char)m_bUnderline;

	// Create font
	if (m_fSampleFont.GetSafeHandle() != NULL)
	{
		m_fSampleFont.DeleteObject();
	}
	m_fSampleFont.CreateFontIndirect(&lf);

	// Redraw window
	m_ctlSample.GetWindowRect(&rcSample);
	ScreenToClient(&rcSample);

	InvalidateRect(rcSample);
}

void COptionTreeFontSel::OnPaint() 
{
	// Declare variables
	CPaintDC dc(this);
	CRect rcText;
	CFont *oldFont;
	CSize sTextExtent;
	COLORREF crText;
	TEXTMETRIC tm;
	int nBkMode, nLen, x, y;
	CString strSample;
	LOGFONT lf;

	// Load sample text
	if (m_dwDialogFlags & OT_FS_CUSTOMSAMPLE)
	{
		strSample = m_strCustomSample;
	}
	else if (m_dwDialogFlags & OT_FS_FONTNAMESAMPLE)
	{
		m_fSampleFont.GetLogFont(&lf);
		strSample.Format("%s", lf.lfFaceName);
	}
	else
	{
		strSample.LoadString(AFX_IDS_SAMPLETEXT);	
	}

	// If there is no sample font abort
	if (!m_fSampleFont.GetSafeHandle())
	{
		return;
	}

	// Get the bounding box
	m_ctlSample.GetWindowRect(&rcText);
	ScreenToClient(&rcText);

	// Select the new font and colors into the dc
	oldFont = dc.SelectObject(&m_fSampleFont);
	crText = dc.SetTextColor(m_ctlButtonColor.GetColor());
	nBkMode = dc.SetBkMode(TRANSPARENT);

	// Calculate the position of the text
	dc.GetTextMetrics(&tm);

	nLen = strSample.GetLength();
	sTextExtent = dc.GetTextExtent(strSample, nLen);
	sTextExtent.cy = tm.tmAscent - tm.tmInternalLeading;

	if ((sTextExtent.cx >= (rcText.right - rcText.left)) || (sTextExtent.cx <= 0))
	{
		x = rcText.left;
	}
	else
	{
		x = rcText.left + ((rcText.right - rcText.left) - sTextExtent.cx) / 2;
	}

	y = min(rcText.bottom, rcText.bottom - ((rcText.bottom - rcText.top) - sTextExtent.cy) / 2);

	// Draw it
	dc.ExtTextOut(x, y - (tm.tmAscent), ETO_CLIPPED, &rcText, strSample, nLen, NULL);

	// Put the DC back the way it was
	dc.SetBkMode(nBkMode);
	dc.SetTextColor(crText);
	dc.SelectObject(oldFont);
}

void COptionTreeFontSel::OnButtonOK() 
{
	// Get current font
	GetSelectedFont(m_lfFont);

	// End dialog
	EndDialog(ID_FONTSEL_OK);
}

void COptionTreeFontSel::OnButtonCancel() 
{
	// Get current font
	m_lfFont = *m_ctlFontNames.GetLogFont();

	// Update size
	m_ctlFontSizes.UpdateLogFont(&m_lfFont);

	// Close dialog
	EndDialog(ID_FONTSEL_CANCEL);
}


void COptionTreeFontSel::OnButtonDefault() 
{
	// Select default font
	SelectCurFont(m_lfDefault);

	// Select color
	m_ctlButtonColor.SetColor(m_ctlButtonColor.GetDefaultColor());
	
	// Update sample
	UpdateSampleFont();
}

void COptionTreeFontSel::OnObjectsChanged()
{
	// Fill font names
	FillFaceNameList();

	// Update sample
	UpdateSampleFont();
}

void COptionTreeFontSel::OnEditUpdateFontNames()
{
	// When the users entry matches an entry in the list, select it
	CString str;
	m_ctlFontNames.GetWindowText(str);
	int nEntry = m_ctlFontNames.FindStringExact(-1, str);
	if (nEntry != CB_ERR)
	{
		m_ctlFontNames.SetCurSel(nEntry);
		m_ctlFontNames.SetEditSel(-1, -1);

		// -- Re-fill the size list
		FillSizeStyleList();
	}
}

void COptionTreeFontSel::OnEditUpdateFontSizes()
{
	// When the users entry matches an entry in the list, select it
	m_ctlFontSizes.GetWindowText(m_strFontSize);
	int nEntry = m_ctlFontSizes.FindStringExact(-1, m_strFontSize);
	if (nEntry != CB_ERR)
	{
		m_ctlFontSizes.SetCurSel(nEntry);
		m_ctlFontSizes.SetEditSel(-1, -1);

		// -- Update the sample text
		UpdateSampleFont();
	}
}

void COptionTreeFontSel::OnSelChangeFontNames()
{
	FillSizeStyleList();
}

void COptionTreeFontSel::OnSelChangeFontSizes()
{
	int nSel = m_ctlFontSizes.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_ctlFontSizes.GetLBText(nSel, m_strFontSize);
		UpdateSampleFont();
	}
}

void COptionTreeFontSel::OnSelChangeFontStyles()
{
	int nSel = m_ctlFontStyles.GetCurSel();
	m_nCurrentStyle = m_ctlFontStyles.GetItemData(nSel);
	m_nActualStyle = m_nCurrentStyle;

	// Update the sample font
	UpdateSampleFont();
}

void COptionTreeFontSel::OnEditChangeFontStyles()
{
	// when the users entry matches an entry in the list, select it
	CString str;
	m_ctlFontStyles.GetWindowText(str);
	int nEntry = m_ctlFontStyles.FindStringExact(-1, str);
	if (nEntry != CB_ERR)
	{
		m_ctlFontStyles.SetCurSel(nEntry);
		m_ctlFontStyles.SetEditSel(-1, -1);

		// Update the sample text
		m_nCurrentStyle = m_ctlFontStyles.GetItemData(nEntry);
		m_nActualStyle = m_nCurrentStyle;
		UpdateSampleFont();
	}
}

void COptionTreeFontSel::OnStrikeout()
{
	if (m_ctlStrikeout.GetCheck() == 1)
		m_bStrikeOut = TRUE;
	else
		m_bStrikeOut = FALSE;

	UpdateSampleFont();
}

void COptionTreeFontSel::OnUnderline()
{
	if (m_ctlUnderline.GetCheck() == 1)
		m_bUnderline = TRUE;
	else
		m_bUnderline = FALSE;

	UpdateSampleFont();
}

void COptionTreeFontSel::SetDefaultTextColor(COLORREF crColor)
{
	// Set default color
	m_ctlButtonColor.SetDefaultColor(crColor);
}

void COptionTreeFontSel::SetTextColor(COLORREF crColor)
{
	// Set color
	m_ctlButtonColor.SetColor(crColor);
}

COLORREF COptionTreeFontSel::GetDefaultTextColor()
{
	return m_ctlButtonColor.GetDefaultColor();
}

COLORREF COptionTreeFontSel::GetTextColor()
{
	return m_ctlButtonColor.GetColor();
}

long COptionTreeFontSel::OnSelChangeColor(UINT lParam, long wParam)
{
	// Update the sample font
	UpdateSampleFont();

	return TRUE;
}

void COptionTreeFontSel::SelectCurFont(LOGFONT lf)
{
	// Declare variables
	CPaintDC dc(this);
	int nEntry;
	BOOL bFound = FALSE;
	int nMaxEntries;
	CString strFace;

	// Validate controls
	if (ValidateControls() == FALSE)
	{
		return;
	}

	// Select font name
	strFace.Format("%s", lf.lfFaceName);
	if (m_ctlFontNames.SelectFontName(strFace) == FALSE)
	{
		m_ctlFontNames.SetCurSel(0);
	}

	// Fill size list for this font name
	FillSizeStyleList();
	
	// Size
	m_strFontSize.Format("%d", abs(MulDiv(lf.lfHeight, 72, dc.GetDeviceCaps(LOGPIXELSY))));

	// Underline
	if (lf.lfUnderline == TRUE)
	{
		m_bUnderline = TRUE;
	}
	else
	{
		m_bUnderline = FALSE;
	}
	m_ctlUnderline.SetCheck(m_bUnderline);

	// Strikeout
	if (lf.lfStrikeOut == TRUE)
	{
		m_bStrikeOut = TRUE;
	}
	else
	{
		m_bStrikeOut = FALSE;
	}
	m_ctlStrikeout.SetCheck(m_bStrikeOut);

	// Styles
	if (lf.lfWeight >= FW_BOLD && lf.lfItalic)
	{
		m_nActualStyle = NTM_BOLD | NTM_ITALIC;
	}
	else if (lf.lfWeight >= FW_BOLD)
	{
		m_nActualStyle = NTM_BOLD;
	}
	else if (lf.lfItalic)
	{
		m_nActualStyle = NTM_ITALIC;
	}
	else
	{
		m_nActualStyle = NTM_REGULAR;
	}

	// Select the size
	// -- Set the point size
	if (m_ctlFontSizes.FindString(-1, m_strFontSize) != CB_ERR)
	{
		nEntry = m_ctlFontSizes.SelectString(-1, m_strFontSize);
		if (nEntry == CB_ERR)
		{
			return;
		}
	}
	else
	{
		// -- -- Point size is not in the list so just fill the edit box
		// and don't select anything from the list
		m_ctlFontSizes.SetCurSel(-1);
		m_ctlFontSizes.SetWindowText(m_strFontSize);
	}


	// Set the styles combo box selection
	bFound = FALSE;
	nMaxEntries = m_ctlFontStyles.GetCount();
	for (int nEntry3 = 0; nEntry3 < nMaxEntries; nEntry3++)
	{
		if (m_ctlFontStyles.GetItemData(nEntry3) == m_nActualStyle)
		{
			m_ctlFontStyles.SetCurSel(nEntry3);
			bFound = TRUE;
		}
	}

	if (!bFound)
	{
		// -- Set style to regular
		m_ctlFontStyles.SetCurSel(0);      
		m_nCurrentStyle = NTM_REGULAR;
	}
	else
	{
		m_nCurrentStyle = m_nActualStyle;
	}

	// Update sample
	UpdateSampleFont();
}

void COptionTreeFontSel::SetDefaultFont(LOGFONT lf)
{
	// Save font
	m_lfDefault = lf;
}

void COptionTreeFontSel::SetCurFont(LOGFONT lf)
{
	// Save font
	m_lfFont = lf;

	// Objects changes
	OnObjectsChanged();
}

void COptionTreeFontSel::SetDefaultFont(CHARFORMAT cfFont)
{
	// Declare variables
	LOGFONT lf = {0};

	// Copy font
	ConvertFont(lf, cfFont);

	// Save font
	m_lfDefault = lf;
}

void COptionTreeFontSel::SetCurFont(CHARFORMAT cfFont)
{
	// Declare variables
	LOGFONT lf = {0};

	// Copy font
	ConvertFont(lf, cfFont);

	// Save font
	m_lfFont = lf;

	// Objects changes
	OnObjectsChanged();
}

BOOL COptionTreeFontSel::ValidateControls()
{
	// Validate
	if (!IsWindow(m_ctlFontStyles.GetSafeHwnd()))
	{
		return FALSE;
	}
	if (!IsWindow(m_ctlFontSizes.GetSafeHwnd()))
	{
		return FALSE;
	}
	if (!IsWindow(m_ctlFontNames.GetSafeHwnd()))
	{
		return FALSE;
	}
	if (!IsWindow(m_ctlUnderline.GetSafeHwnd()))
	{
		return FALSE;
	}
	if (!IsWindow(m_ctlStrikeout.GetSafeHwnd()))
	{
		return FALSE;
	}
	if (!IsWindow(m_ctlButtonDefault.GetSafeHwnd()))
	{
		return FALSE;
	}
	if (!IsWindow(m_ctlButtonColor.GetSafeHwnd()))
	{
		return FALSE;
	}

	return TRUE;
}

void COptionTreeFontSel::OnSelChangeFontScript()
{
	// Update sample
	UpdateSampleFont();
}

void COptionTreeFontSel::OnButtonApply()
{
	// Declare variables
	CHARFORMAT cfFont = {0};
	LOGFONT lf;
	CWnd *pParent = GetParent();

	// Validate parent window
	if (pParent == NULL)
	{
		return;
	}
	if (!::IsWindow(pParent->GetSafeHwnd()))
	{
		return;
	}

	// Get fonts
	GetSelectedFont(lf);
	ConvertFont(cfFont, lf);

	// Notify parent
	if (m_dwDialogFlags & OT_FS_USECHARFORMAT)
	{
		::SendMessage(pParent->GetSafeHwnd(), OT_FS_NOTIFY_APPLY, (WPARAM) m_uControlID, (LPARAM) &cfFont);
	}
	else
	{
		::SendMessage(pParent->GetSafeHwnd(), OT_FS_NOTIFY_APPLY, (WPARAM) m_uControlID, (LPARAM) &lf);
	}

}

void COptionTreeFontSel::ConvertFont(CHARFORMAT &cfFont, LOGFONT &lf)
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

void COptionTreeFontSel::ConvertFont(LOGFONT &lf, CHARFORMAT &cfFont)
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


void COptionTreeFontSel::GetSelectedFont(LOGFONT &lf)
{
	// Validate controls
	if (ValidateControls() == FALSE)
	{
		return;
	}

	// Get log font
	lf = *m_ctlFontNames.GetLogFont();

	// Get size
	m_ctlFontSizes.UpdateLogFont(&lf);

	// Handle styles
	if (m_nCurrentStyle & NTM_BOLD)
	{
		lf.lfWeight = FW_BOLD;
	}
	else
	{
		lf.lfWeight = FW_REGULAR;
	}
	if (m_nCurrentStyle & NTM_ITALIC)
	{
		lf.lfItalic = TRUE;
	}
	else
	{
		lf.lfItalic = FALSE;
	}
	lf.lfStrikeOut = (unsigned char)m_bStrikeOut;
	lf.lfUnderline = (unsigned char)m_bUnderline;
}

DWORD COptionTreeFontSel::GetDialogFlags()
{
	return m_dwDialogFlags;
}

void COptionTreeFontSel::SetCustomSample(CString strSample)
{
	// Save variable
	m_strCustomSample = strSample;
}

void COptionTreeFontSel::GetLogFont(LOGFONT &lf)
{
	// Get selected font
	lf = m_lfFont;
}

void COptionTreeFontSel::GetCharFormat(CHARFORMAT &cf)
{	
	// Convert font
	ConvertFont(cf, m_lfFont);
}

void COptionTreeFontSel::SetControlID(UINT uID)
{
	m_uControlID = uID;
}
