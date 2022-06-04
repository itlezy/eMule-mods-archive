#include "stdafx.h"
#include "PartStatusCtrl.h"
#include "DownloadClientsCtrl.h"
#include "PartFile.h"
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define DLC_BARUPDATE 512


// CPartStatusCtrl
IMPLEMENT_DYNAMIC(CPartStatusCtrl, CStatic)

BEGIN_MESSAGE_MAP(CPartStatusCtrl, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

CPartStatusCtrl::~CPartStatusCtrl(){
	status.DeleteObject();
}

void CPartStatusCtrl::Refresh(bool force)
{
	if(force)
		dwUpdated = 0;
	Invalidate();
	//UpdateWindow();
}

void CPartStatusCtrl::Refresh(CPartFile*file)
{
	if(file == pdcl->curPartfile && IsWindowVisible()){
		Invalidate();
		//UpdateWindow();
	}
}

void CPartStatusCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rect;
	GetClientRect(rect);
	if(pdcl->curPartfile){
		CDC cdcStatus;
		cdcStatus.CreateCompatibleDC(&dc);
		int cx = 0;
		if (status != (HBITMAP)NULL)
			cx = status.GetBitmapDimension().cx; 
		DWORD dwTicks = GetTickCount();
		int iWidth = rect.Width();
		int iHeight = rect.Height();
		if(status == (HBITMAP)NULL || !dwUpdated || dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth) { 
			status.DeleteObject(); 
			status.CreateCompatibleBitmap(&dc,  iWidth, iHeight); 
			status.SetBitmapDimension(iWidth,  iHeight); 
			cdcStatus.SelectObject(status); 

			pdcl->curPartfile->DrawStatusBar(&cdcStatus,  &rect, thePrefs.UseFlatBar());

			dwUpdated = dwTicks + (rand() % 128); 
		} else 
			cdcStatus.SelectObject(status);

		dc.BitBlt(rect.left, rect.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
	}
	else
		dc.FillSolidRect(rect, GetSysColor(COLOR_MENU));

}