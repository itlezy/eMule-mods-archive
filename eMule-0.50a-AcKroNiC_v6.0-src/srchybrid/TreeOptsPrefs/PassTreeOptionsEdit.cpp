// emulEspaña: Added by MoNKi [ MoNKi: -Pass Edit on TreeOptionsCtrl- ]

#include "StdAfx.h"
#include "PassTreeOptionsEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPassTreeOptionsEdit, CTreeOptionsEdit)

CPassTreeOptionsEdit::CPassTreeOptionsEdit(void)
{
}

CPassTreeOptionsEdit::~CPassTreeOptionsEdit(void)
{
}
BEGIN_MESSAGE_MAP(CPassTreeOptionsEdit, CTreeOptionsEdit)
	ON_WM_CREATE()
END_MESSAGE_MAP()

int CPassTreeOptionsEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeOptionsEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

DWORD CPassTreeOptionsEdit::GetWindowStyle()
{
	return CTreeOptionsEdit::GetWindowStyle() | ES_PASSWORD;
}