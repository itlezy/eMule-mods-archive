//***************************************************************************
//
// AUTHOR:  James White (feel free to remove or otherwise mangle any part)
//
//***************************************************************************
#include "stdafx.h"
#include "ColorButton.h"
#include "UserMsgs.h"

//***********************************************************************
//**                         MFC Debug Symbols                         **
//***********************************************************************
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//***********************************************************************
//**                            DDX Method                            **
//***********************************************************************

void AFXAPI DDX_ColorButton(CDataExchange *pDX, int nIDC, COLORREF& crColour)
{
    HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
    ASSERT (hWndCtrl != NULL);                
    
    CColorButton* pColourButton = (CColorButton*) CWnd::FromHandle(hWndCtrl);
    if (pDX->m_bSaveAndValidate)
		crColour = pColourButton->GetColor();
    else // initializing
		pColourButton->SetColor(crColour);
}

//***********************************************************************
//**                             Constants                             **
//***********************************************************************
const int g_ciArrowSizeX = 4 ;
const int g_ciArrowSizeY = 2 ;

//***********************************************************************
// Method:	CColorButton::CColorButton(void)
// Notes:	Default Constructor.
//***********************************************************************
CColorButton::CColorButton(void):
	_Inherited(),
	m_Color(CLR_DEFAULT),
	m_DefaultColor(::GetSysColor(COLOR_APPWORKSPACE)),
	m_bPopupActive(FALSE)
{
}

//***********************************************************************
// Method:	CColorButton::~CColorButton(void)
// Notes:	Destructor.
//***********************************************************************
CColorButton::~CColorButton(void)
{
}

//***********************************************************************
// Method:	CColorButton::SetColor()
// Notes:	None.
//***********************************************************************
void CColorButton::SetColor(COLORREF Color)
{
	m_Color = Color;

	if (::IsWindow(m_hWnd)) 
        RedrawWindow();
}

//***********************************************************************
//**                         CButton Overrides                         **
//***********************************************************************
void CColorButton::PreSubclassWindow() 
{
    ModifyStyle(0, BS_OWNERDRAW);      

    _Inherited::PreSubclassWindow();
}

//***********************************************************************
//**                         Message Handlers                         **
//***********************************************************************
BEGIN_MESSAGE_MAP(CColorButton, CButton)
    //{{AFX_MSG_MAP(CColorButton)
    ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
    //ON_WM_CREATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*
//***********************************************************************
// Method:	CColorButton::OnCreate()
// Notes:	None.
//***********************************************************************
int CColorButton::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CButton::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}
*/
//***********************************************************************
// Method:	CColorButton::OnClicked()
// Notes:	None.
//***********************************************************************
BOOL CColorButton::OnClicked()
{
	m_bPopupActive = TRUE;

	CColorDialog dlg(m_Color, CC_FULLOPEN | CC_ANYCOLOR, this);

	if (dlg.DoModal() == IDOK)
		m_Color = dlg.GetColor();

	m_bPopupActive = FALSE;
	RedrawWindow();
    return TRUE;
}



//***********************************************************************
// Method:	CColorButton::DrawItem()
// Notes:	None.
//***********************************************************************
void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	ASSERT(lpDrawItemStruct);

	CDC*    pDC      = CDC::FromHandle(lpDrawItemStruct->hDC);
	UINT    state    = lpDrawItemStruct->itemState;
    CRect   rDraw    = lpDrawItemStruct->rcItem;
	CRect	rArrow;

	if (m_bPopupActive)
		state |= ODS_SELECTED|ODS_FOCUS;

	//******************************************************
	//**                  Draw Outer Edge
	//******************************************************
	UINT uFrameState = DFCS_BUTTONPUSH|DFCS_ADJUSTRECT;

	if (state & ODS_SELECTED)
		uFrameState |= DFCS_PUSHED;

	if (state & ODS_DISABLED)
		uFrameState |= DFCS_INACTIVE;
	
	pDC->DrawFrameControl(&rDraw,
						  DFC_BUTTON,
						  uFrameState);


	if (state & ODS_SELECTED)
		rDraw.OffsetRect(1,1);

	//******************************************************
	//**                     Draw Focus
	//******************************************************
	if (state & ODS_FOCUS) 
    {
		RECT rFocus = {rDraw.left,
					   rDraw.top,
					   rDraw.right - 1,
					   rDraw.bottom};
  
        pDC->DrawFocusRect(&rFocus);
    }

	rDraw.DeflateRect(::GetSystemMetrics(SM_CXEDGE),
					  ::GetSystemMetrics(SM_CYEDGE));

	//******************************************************
	//**                     Draw Color
	//******************************************************
	if ((state & ODS_DISABLED) == 0)
	{
		pDC->FillSolidRect(&rDraw,
						   (m_Color == CLR_DEFAULT)
						   ? m_DefaultColor
						   : m_Color);

		::FrameRect(pDC->m_hDC,
					&rDraw,
					(HBRUSH)::GetStockObject(BLACK_BRUSH));
	}
}