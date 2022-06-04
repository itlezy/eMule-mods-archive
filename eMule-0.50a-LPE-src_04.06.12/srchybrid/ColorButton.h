//***************************************************************************
//
// AUTHOR:  James White (feel free to remove or otherwise mangle any part)
//
// DESCRIPTION: This class is alarmingly similar to the CColourPicker control
//	created by Chris Maunder of www.codeproject.com. It is so as it was blatantly
//	copied from that class and is entirely dependant on his other great work
//  in CColourPopup. I was hoping for (cough.. gag..) a more Microsoft look
//  and I think this is pretty close. Hope you like it.
//
// ORIGINAL: http://www.codeproject.com/miscctrl/colour_picker.asp
//
//***************************************************************************
#pragma once

void AFXAPI DDX_ColorButton(CDataExchange *pDX, int nIDC, COLORREF& crColour);

class CColorButton : public CButton
{
public:
	//***********************************************************************
	// Name:		CColorButton
	// Description:	Default constructor.
	// Parameters:	None.
	// Return:		None.	
	// Notes:		None.
	//***********************************************************************
	CColorButton(void);

	//***********************************************************************
	// Name:		CColorButton
	// Description:	Destructor.
	// Parameters:	None.
	// Return:		None.		
	// Notes:		None.
	//***********************************************************************
	virtual ~CColorButton(void);

	//***********************************************************************
	// Name:		GetColor
	// Description:	Returns the current color selected in the control.
	// Parameters:	void
	// Return:		COLORREF 
	// Notes:		None.
	//***********************************************************************
	COLORREF GetColor(void) const{return m_Color;}

	//***********************************************************************
	// Name:		SetColor
	// Description:	Sets the current color selected in the control.
	// Parameters:	COLORREF Color
	// Return:		None. 
	// Notes:		None.
	//***********************************************************************
	void SetColor(COLORREF Color);


	//***********************************************************************
	// Name:		GetDefaultColor
	// Description:	Returns the color associated with the 'default' selection.
	// Parameters:	void
	// Return:		COLORREF 
	// Notes:		None.
	//***********************************************************************
	COLORREF GetDefaultColor(void) const{return m_DefaultColor;}

	//***********************************************************************
	// Name:		SetDefaultColor
	// Description:	Sets the color associated with the 'default' selection.
	//				The default value is COLOR_APPWORKSPACE.
	// Parameters:	COLORREF Color
	// Return:		None. 
	// Notes:		None.
	//***********************************************************************
	void SetDefaultColor(COLORREF Color){m_DefaultColor = Color;}

	//{{AFX_VIRTUAL(CColorButton)

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
    virtual void PreSubclassWindow();
    //}}AFX_VIRTUAL

	//{{AFX_MSG(CColorButton)
    afx_msg BOOL OnClicked();
    //afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	COLORREF m_Color;
	COLORREF m_DefaultColor;
	BOOL	m_bPopupActive;

private:

	typedef CButton _Inherited;

};
