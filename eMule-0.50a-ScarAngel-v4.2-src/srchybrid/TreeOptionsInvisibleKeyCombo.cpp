#include "stdafx.h"
#include "TreeOptionsInvisibleKeyCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTreeOptionsInvisibleKeyCombo, CTreeOptionsCombo)

CTreeOptionsInvisibleKeyCombo::CTreeOptionsInvisibleKeyCombo()
{
}

CTreeOptionsInvisibleKeyCombo::~CTreeOptionsInvisibleKeyCombo()
{
}

BEGIN_MESSAGE_MAP(CTreeOptionsInvisibleKeyCombo, CTreeOptionsCombo)
	ON_WM_CREATE()
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnCbnSelchange)
END_MESSAGE_MAP()


int CTreeOptionsInvisibleKeyCombo::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	m_bSelf = true;
	if (CTreeOptionsCombo::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_bSelf = false;
	
	for(TCHAR i=_T('A'); i<=_T('Z'); i++)
		AddString(CString(i));
	for(TCHAR i=_T('0'); i<=_T('9'); i++)
		AddString(CString(i));

	return 0;
}

void CTreeOptionsInvisibleKeyCombo::OnCbnSelchange()
{
	if (!m_bSelf)
		((CTreeOptionsCtrlEx*)m_pTreeCtrl)->NotifyParent((UINT)this, m_hTreeCtrlItem);
}
