//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "SpeedGraphWnd.h"
#include "preferences.h"
#include "MemDC.h"
#include "DropTarget.h"
#include "Addons/GDIPlusUtil/GDIPlusUtil.h" // X: [GPUI] - [GDI Plus UI]
#include "UploadQueue.h"
#include "DownloadQueue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSpeedGraph::CSpeedGraph()
{
	m_ui_MaxAmount = 0;
	TrafficStats = NULL;
}

CSpeedGraph::~CSpeedGraph()
{
	if(TrafficStats)
		delete[] TrafficStats; 
}

void CSpeedGraph::Set_TrafficValue(uint32 value) 
{
	// Shift whole array 1 step to left and calculate local maximum
	for(uint32 x=0; x < TrafficEntries-1; x++)
		TrafficStats[x] = TrafficStats[x+1];

	TrafficStats[TrafficEntries-1] = (value >= m_ui_MaxAmount)?m_ui_MaxAmount:value;
}

void CSpeedGraph::InitArray()
{
	memset(TrafficStats, 0, TrafficEntries*sizeof(TrafficStats[0]));
}

void CSpeedGraph::Init(uint32 trafficEntries) 
{
	TrafficEntries = trafficEntries;
	delete[] TrafficStats;
	TrafficStats = new uint32[TrafficEntries];
	InitArray();
}

// CSpeedGraphWnd dialog
//IMPLEMENT_DYNAMIC(CSpeedGraphWnd, CDialog)

BEGIN_MESSAGE_MAP(CSpeedGraphWnd, CModDialog)
ON_WM_CONTEXTMENU()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONDBLCLK()
ON_WM_PAINT()
ON_WM_DESTROY()
ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()
#define WINWIDTH 48
#define WINBORDER 2
#define DRAWWIDTH (WINWIDTH - 2*WINBORDER)

CSpeedGraphWnd::CSpeedGraphWnd()
	: CModDialog(CSpeedGraphWnd::IDD) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	lastvalidi = 0;
	lastvalido = 0;
	m_pDropTarget = new CMainFrameDropTarget();
	//ColorText = RGB(0, 0, 0);
}

CSpeedGraphWnd::~CSpeedGraphWnd()
{
	delete m_pDropTarget;
}

BOOL CSpeedGraphWnd::OnInitDialog()
{
	CModDialog::OnInitDialog();
	int nScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	int left = (thePrefs.speedgraphwindowLeft * nScreenWidth + nScreenWidth/2) / 100;
	int top = (thePrefs.speedgraphwindowTop * nScreenHeight + nScreenHeight/2) / 100;
	SetWindowPos(NULL, left,top, WINWIDTH, WINWIDTH, SWP_NOACTIVATE);
	m_co_UpTrafficGraph.Init(DRAWWIDTH);
	m_co_DownTrafficGraph.Init(DRAWWIDTH);

	// Traffic graph
      SetUSpeedMeterRange((uint32)thePrefs.GetMaxGraphUploadRate(true));
	  SetDSpeedMeterRange((uint32)thePrefs.GetMaxGraphDownloadRate());

	VERIFY( m_pDropTarget->Register(this) );
	DrawWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// X-Ray :: Speedgraph :: Start
void CSpeedGraphWnd::Update_TrafficGraph()
{
	if(!theApp.emuledlg->IsRunning())
		return;
	uint32 eMuleIn = theApp.downloadqueue->GetDatarate();
	uint32 eMuleOut = theApp.uploadqueue->GetDatarate();

	bool refresho = true, refreshi = true;
	if(eMuleOut)
		lastvalido = DRAWWIDTH;
	else if(lastvalido)
		--lastvalido;
	else
		refresho = false;

	if(eMuleIn)
		lastvalidi = DRAWWIDTH;
	else if(lastvalidi)
		--lastvalidi;
	else
		refreshi = false;
	m_co_UpTrafficGraph.Set_TrafficValue(eMuleOut);
	m_co_DownTrafficGraph.Set_TrafficValue(eMuleIn);
	/*if (thePrefs.ShowOverhead())
		AllTraffic.Format(L"%.1f(%.1f)", traffic / 1024.0, (float)(valueOverall-value) / 1024.0);
	else
		AllTraffic.Format(L"%.1f", traffic / 1024.0);*/
	if(refresho || refreshi)
		DrawWindow();
}
// X-Ray :: Speedgraph :: End

void CSpeedGraphWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	CModDialog::OnLButtonDown(nFlags, point);
	PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}

void CSpeedGraphWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point){
	theApp.emuledlg->ShowToolPopup(true, true);
}

void CSpeedGraphWnd::DrawWindow(){ // X: [GPUI] - [GDI Plus UI]
	HDC hdc = ::GetDC(HWND_DESKTOP);
	CDC* pdc = CDC::FromHandle(hdc);
	CDC dc;
	dc.CreateCompatibleDC(pdc);
	CBitmap bgbitmap;
	bgbitmap.CreateCompatibleBitmap(pdc, WINWIDTH, WINWIDTH);
	dc.SelectObject(bgbitmap);
	Graphics g(dc.GetSafeHdc());
	g.SetCompositingMode(CompositingModeSourceCopy);
	SolidBrush borderbrush(Color(0xccd8d8ff));
	Pen pen(&borderbrush);
	g.DrawRectangle(&pen, 0, 0, WINWIDTH - 1, WINWIDTH - 1);
	LinearGradientBrush brush(
		Point(0, 0),
		Point(0, WINWIDTH - WINBORDER),
		Color(0x7fffffff),
		Color(0x3fcccccc));

	g.FillRectangle(&brush, WINBORDER, WINBORDER, DRAWWIDTH, DRAWWIDTH);
	if(lastvalido || lastvalidi){
		brush.SetLinearColors(Color(210, 255, 130, 0),
			Color(210, 255, 255, 80));
		Point*p = NULL;
		if(lastvalido){
			p = new Point[lastvalido + 2];
			p[lastvalido].X = WINBORDER + lastvalido;
			p[lastvalido].Y = WINWIDTH - WINBORDER;
			p[lastvalido+1].X = WINBORDER;
			p[lastvalido+1].Y = WINWIDTH - WINBORDER;
			for (int i = 0;i < lastvalido; ++i){
				p[i].X = WINBORDER + i;
				p[i].Y = WINWIDTH - WINBORDER - m_co_UpTrafficGraph.TrafficStats[i] * DRAWWIDTH / m_co_UpTrafficGraph.m_ui_MaxAmount;
			}
			g.FillPolygon(&brush, p, lastvalido + 2);
		}

		if(lastvalidi){
			brush.SetLinearColors(Color(210, 0, 130, 255),
				Color(210, 80, 255, 255));
			Point*p2;
			p2 = new Point[lastvalidi + 2];
			p2[lastvalidi].X = WINBORDER + lastvalidi;
			p2[lastvalidi].Y = WINWIDTH - WINBORDER;
			p2[lastvalidi+1].X = WINBORDER;
			p2[lastvalidi+1].Y = WINWIDTH - WINBORDER;
			for (int i = 0;i < lastvalidi; ++i){
				p2[i].X = WINBORDER + i;
				p2[i].Y = WINWIDTH - WINBORDER - m_co_DownTrafficGraph.TrafficStats[i] * DRAWWIDTH / m_co_DownTrafficGraph.m_ui_MaxAmount;
			}
			g.SetCompositingMode(CompositingModeSourceCopy);
			g.FillPolygon(&brush, p2, lastvalidi + 2);
			delete [] p2;
			if(lastvalido){
				brush.SetLinearColors(Color(105, 255, 130, 0),
					Color(105, 255, 255, 80));
				g.SetCompositingMode(CompositingModeSourceOver);
				g.FillPolygon(&brush, p, lastvalido + 2);
			}
		}

		delete [] p;
	}
	POINT ptSrc = {0,0};
	SIZE szWindow = {WINWIDTH, WINWIDTH};
	BLENDFUNCTION blendPixelFunction= { AC_SRC_OVER, 0, 255,AC_SRC_ALPHA };
	UpdateLayeredWindow(NULL, NULL, &szWindow, &dc,&ptSrc, 0, &blendPixelFunction, ULW_ALPHA);
	::ReleaseDC(HWND_DESKTOP, hdc);
}

void CSpeedGraphWnd::OnDestroy()
{
	m_pDropTarget->Revoke();
	RECT rcDlg;
	GetWindowRect(&rcDlg);
	int nScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	thePrefs.speedgraphwindowLeft = rcDlg.left * 100 / nScreenWidth;
	thePrefs.speedgraphwindowTop = rcDlg.top * 100 / nScreenHeight;
	CModDialog::OnDestroy();
}

void CSpeedGraphWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CModDialog::OnLButtonDblClk(nFlags, point);
	if (theApp.emuledlg->IsIconic())
		theApp.emuledlg->ShowWindow(SW_RESTORE);
	else if(!theApp.emuledlg->IsWindowVisible())
		theApp.emuledlg->RestoreWindow();
	else
		theApp.emuledlg->SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

void CSpeedGraphWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CModDialog::OnShowWindow(bShow, nStatus);
	if (bShow == FALSE){
		m_co_UpTrafficGraph.InitArray();
		m_co_DownTrafficGraph.InitArray();
		lastvalido = 0;
		lastvalidi = 0;
		DrawWindow();
	}
}
