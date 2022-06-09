//
// Owner Drawn WinXP/Vista themes aware button implementation...
//
#include "stdafx.h"
#include "ButtonVE.h"

#if _MSC_VER>=1600
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CButtonVE, CButton)

BEGIN_MESSAGE_MAP(CButtonVE, CButton)
  ON_WM_GETDLGCODE()
  ON_WM_KILLFOCUS()
  ON_WM_TIMER()
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_NCDESTROY()
  ON_WM_SYSCOLORCHANGE()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_LBUTTONUP()
  ON_MESSAGE(BM_SETSTYLE, OnSetStyle)
  ON_MESSAGE(BM_SETCHECK, OnSetCheck)
  ON_MESSAGE(BM_GETCHECK, OnGetCheck)
  ON_MESSAGE(WM_UPDATEUISTATE, OnUpdateUIState)
END_MESSAGE_MAP()
#endif