// TitleMenu.h: interface for the CTitleMenu class.
// Based on the code of Per Fikse(1999/06/16) on codeguru.earthweb.com
// Author: Arthur Westerman
// Bug reports by : Brian Pearson 
//////////////////////////////////////////////////////////////////////


#pragma once

class CTitleMenu : public CMenu
{
	typedef UINT (CALLBACK* LPFNDLLFUNC1)(HDC,CONST PTRIVERTEX,DWORD,CONST PVOID,DWORD,DWORD);
// Construction
public:
	CTitleMenu();

// Attributes
protected:
	CFont m_Font;
	CString m_strTitle;

	LPFNDLLFUNC1 dllfunc_GradientFill;
	HINSTANCE hinst_msimg32;
	long clRight;
	long clLeft;
	long clText;
	bool bDrawEdge;
	UINT flag_edge;

// Operations
public:
	void AddMenuTitle(CString strTitle);

protected:
	bool m_bCanDoGradientFill;
	HFONT CreatePopupMenuTitleFont();
	BOOL GradientFill(	HDC hdc,
						CONST PTRIVERTEX pVertex,
						DWORD dwNumVertex,
						CONST PVOID pMesh,
						DWORD dwNumMesh,
						DWORD dwMode);
	


// Implementation
public:
	void SetColor(long cl) {clLeft=cl;};
	void SetGradientColor(long cl) {clRight=cl;};
	void SetTextColor(long cl) {clText=cl;};
	// See ::DrawEdge for flag values
	void SetEdge(bool shown,UINT remove=0,UINT add=0) {bDrawEdge=shown; (flag_edge^=remove)|=add;};

	long GetColor() {return clLeft;};
	long GetGradientColor() {return clRight;};
	long GetTextColor() {return clText;};
	long GetEdge() {return flag_edge;};

	virtual ~CTitleMenu();
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

private:

 
};


