#pragma once

class CThemeHelperST;

/////////////////////////////////////////////////////////////////////////////
class CIconStatic : public CStatic
{
public:
	CIconStatic();
	virtual ~CIconStatic();

	void	Init(UINT nIconID, CThemeHelperST *pTheme);
	void	SetText(const CString &strText);

protected:
	afx_msg	void OnSysColorChange();

	DECLARE_MESSAGE_MAP()

private:
	void	Draw();

	CBitmap	m_MemBMP;
	CStatic	m_wndPicture;
	UINT	m_uiIconID;
	CString	m_strText;
	CThemeHelperST	*m_pTheme;
};
