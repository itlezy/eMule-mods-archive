//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "resource.h"
#include "opcodes.h"
#include "memcpy_amd.h"
#include "AboutDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)
CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutDlg::IDD, pParent)
{
	m_bLButtonDown = false;
}

CAboutDlg::~CAboutDlg()
{
	m_imgBackground.DeleteObject();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Layered.AddLayeredStyle(m_hWnd);
	m_Layered.SetLayeredWindowAttributes(m_hWnd, 0, 0, LWA_ALPHA);

	VERIFY(m_imgBackground.LoadImage(IDR_LOGO, _T("JPG")));
	(static_cast<CStatic*>(GetDlgItem(IDC_BACKGROUND)))->SetBitmap(m_imgBackground);

#ifndef _DEBUG
	#define VERSION_FONTSIZE _T("22")
#else
	#define VERSION_FONTSIZE _T("14")
#endif

	CString s =
		_T("<font face='arial' size='10' style='b' color='0,0,0' background='255,255,255' align='center' valign='bottom'>")
		_T("<br><br><br>")
		_T("<font face='Arial Bold' size='22' style='b' color='0,0,0' background='255,255,255' align='center' valign='bottom'>") CLIENT_NAME _T("</font>")
		_T("<br>")
		_T("<font face='Arial Bold' size='") VERSION_FONTSIZE _T("' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>v") CURRENT_VERSION_LONG;
	switch (get_cpu_type())
	{
		case 2:
			s += _T(" MMX");
			break;
		case 3:
			s += _T(" AMD");
			break;
		case 4:
		case 5:
			s += _T(" SSE");
			break;
	}
	s += _T("</font><br>")
		_T("<font face='Verdana Regular' size='11' style='u | -b' color='0,0,255' background='255,255,255' align='center' valign='bottom'><a href='http://emuleplus.info'>emuleplus.info</a></font>")
		_T("<br><br><br><br><br><br><br><br>")

	// uses default font
		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>Programming</font>")
		_T("<br>")
		_T("Aw3")
		_T("<br>")
		_T("DonGato")
		_T("<br>")
		_T("Eklmn")
		_T("<br>")
		_T("kuchin")
		_T("<br>")
		_T("KuSh")
		_T("<br><br>")

		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>Layout Design and Graphics</font>")
		_T("<br>")
		_T("DrSiRiUs")
		_T("<br>")
		_T("Psy")
		_T("<br><br>")

#if 0
		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>Coordinators</font>")
		_T("<br>")
		_T("[Translation] ")
		_T("<br>")
		_T("[Stand-alone FAQ] ")
		_T("<br><br>")
#endif

		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>Betatesters</font>")
		_T("<br>")
		_T("DopeFish")
		_T("<br>")
		_T("Fuxie - DK")
		_T("<br>")
		_T("muleteer")
		_T("<br>")
		_T("Psy")
		_T("<br>")
		_T("reanimated838uk")
		_T("<br>")
		_T("Vladimir (SV)")
		_T("<br>")
		_T("glaskrug")
		_T("<br><br>")

		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>Translators</font>")
		_T("<br>")
		_T("[Basque] orokulus")
		_T("<br>")
		_T("[Belarusian] Kryvich")
		_T("<br>")
		_T("[Catalan] ernzzz")
		_T("<br>")
		_T("[Chinese Simplified] xyes")
		_T("<br>")
		_T("[Chinese Traditional] roytam1")
		_T("<br>")
		_T("[Croatian] lhay")
		_T("<br>")
		_T("[Czech] rocky.intel")
		_T("<br>")
		_T("[Danish] Bro-DK")
		_T("<br>")
		_T("[Dutch] Sleepy")
		_T("<br>")
		_T("[Extremaduran] purgossu")
		_T("<br>")
		_T("[Finnish] Nikerabbit")
		_T("<br>")
		_T("[French] KuSh")
		_T("<br>")
		_T("[German] DoubleT")
		_T("<br>")
		_T("[Greek] geodimo")
		_T("<br>")
		_T("[Hebrew] Tcl")
		_T("<br>")
		_T("[Italian] Efix, Nazgul")
		_T("<br>")
		_T("[Korean] p5657587")
		_T("<br>")
		_T("[Lithuanian] Gliz")
		_T("<br>")
		_T("[Malay] xyes")
		_T("<br>")
		_T("[Norwegian] True Neo")
		_T("<br>")
		_T("[Polish] ElrondDzidkins")
		_T("<br>")
		_T("[Portuguese Brazil] Jhonnatta")
		_T("<br>")
		_T("[Portuguese Portugal] Brun0")
		_T("<br>")
		_T("[Romanian] Tcl")
		_T("<br>")
		_T("[Russian] iMiKE")
		_T("<br>")
		_T("[Serbian] DarkMan")
		_T("<br>")
		_T("[Slovenian] krojc")
		_T("<br>")
		_T("[Spanish] gprina")
		_T("<br>")
		_T("[Turkish] BouRock")
		_T("<br>")
		_T("[Ukrainian] Punk")
		_T("<br><br>")

		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>FAQ Translators</font>")
		_T("<br>")
		_T("[Italian] KerneL")
		_T("<br><br>")

		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>Retired Developers</font>")
		_T("<br>")
		_T("BavarianSnail, bond006")
		_T("<br>")
		_T("Cax2, DropF, EC, FoRcHa")
		_T("<br>")
		_T("Lord KiRon, moosetea")
		_T("<br>")
		_T("morevit, netwolf, obaldin")
		_T("<br>")
		_T("Purity, reCDVst, katsyonak")
		_T("<br>")
		_T("TwoBottle Mod, zegzav")
		_T("<br>")
		_T("SyruS, morphis")
		_T("<br><br>")

		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>Retired Supporters</font>")
		_T("<br>")
		_T("eMWu, LeChuck, Mr. Bean")
		_T("<br>")
		_T("bymike, erein, uffowich")
		_T("<br>")
		_T("Daan, giorgosman, pooz")
		_T("<br>")
		_T("Dr.Slump, janes bong, hiei")
		_T("<br>")
		_T("Bazzik, Blackstaff, LF_")
		_T("<br>")
		_T("thisIsRandom, ChatNoir, k`s")
		_T("<br>")
		_T("n@boleo, sw54rus, krhung")
		_T("<br>")
		_T("project-xt, koizo, Beltxo")
		_T("<br>")
		_T("andrerib")
		_T("<br><br>")

		_T("<font face='arial' size='12' style='b' color='0,0,0' background='255,255,255' align='center' valign='bottom'>Additional thanks go to:</font>")
		_T("<br><br>")
		_T("BavarianSnail")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Code snippets)</font>")
		_T("<br>")
		_T("DoubleT")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Code snippets)</font>")
		_T("<br>")
		_T("Eugene Pustovoyt")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Advanced tooltips)</font>")
		_T("<br>")
		_T("Ultras")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Code snippets)</font>")
		_T("<br>")
		_T("zz")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Upload system)</font>")
		_T("<br>")
		_T("Matthew R. Miller")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(OptionTree class)</font>")
		_T("<br>")
		_T("xrmb")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(GUI goodies)</font>")
		_T("<br>")
		_T("Amdribant")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Color Coded Search Results)</font>")
		_T("<br>")
		_T("Tarod")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Too much code to name it)</font>")
		_T("<br>")
		_T("Vorlost")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(chunk transfer and preview)</font>")
		_T("<br><br>")
		_T("<font face='arial' size='11' style='b' color='0,0,0' background='255,255,255' align='center' valign='bottom'>And to everyone we forgot</font>")
		_T("<br><br><br>")


		_T("<font face='arial' size='14' style='b' color='0,0,0' background='255,255,255' align='center' valign='bottom'>eMule Official</font>")
		_T("<br>")
		_T("<font face='arial' size='14' style='b' color='0,0,0' background='255,255,255' align='center' valign='bottom'>Code Credits</font>")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>Copyright (c) 2002-2009 Merkur</font>")
		_T("<br><br>")
		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>Development</font>")
		_T("<br>")
		_T("Unknown1")
		_T("<br>")
		_T("Ornis")
		_T("<br>")
		_T(".")
		_T("<br><br>")

		_T("<font face='arial' size='10' style='' color='0,115,250' background='255,255,255' align='center' valign='bottom'>Retired Members</font>")
		_T("<br>")
		_T("Merkur, tecxx, Pach2")
		_T("<br>")
		_T("Juanjo, Barry, Dirus")
		_T("<br>")
		_T("bluecow")
		_T("<br><br>")

		_T("Thanks to these programmers")
		_T("<br>")
		_T("for publishing useful codeparts")
		_T("<br><br>")
		_T("Paolo Messina")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(ResizableDialog class)</font>")
		_T("<br>")
		_T("PJ Naughter")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(HttpDownload Dialog)</font>")
		_T("<br>")
		_T("Jim Connor")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Scrolling Credits)</font>")
		_T("<br>")
		_T("Yury Goltsman")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(extended Progressbar)</font>")
		_T("<br>")
		_T("Arthur Westerman")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Titled menu)</font>")
		_T("<br>")
		_T("Davide Calabro'")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Layered Windows)</font>")
		_T("<br>")
		_T("Tim Kosse")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(AsyncSocket-Proxysupport)</font>")
		_T("<br>")
		_T("Keith Rule")
		_T("<br>")
		_T("<font face='arial' size='10' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>(Memory DC)</font>")
		_T("</font>")
		_T("<br><br><br><br><br><br>");

	m_ctrlCredits.SetDataString(s);
	m_ctrlCredits.SetDefaultBkColor(RGB(255,255,255));

//	Resize dialog window to the picture size to look well for any system font size
	CRect rWndRect, rBmpRect;

	GetWindowRect(rWndRect);
	GetDlgItem(IDC_BACKGROUND)->GetWindowRect(rBmpRect);
	MoveWindow(rWndRect.left, rWndRect.top, rBmpRect.Width(), rBmpRect.Height(), FALSE);
	
	CRect rCredits;
	GetClientRect(rCredits);
	rCredits.top = 147;
	rCredits.left += 10;
	rCredits.bottom = rCredits.top + 120;
	rCredits.right = rCredits.left + 220;

	int nOldSpeed = m_ctrlCredits.m_nTimerSpeed;
	m_ctrlCredits.m_nTimerSpeed = 500;
//	m_ctrlCredits.m_crInternalTransparentColor = RGB(255,255,255);			// "transparent" background
//	m_ctrlCredits.m_pBackgroundPaint = CAboutDlg::DrawCreditsBackground;	// -||-
//	m_ctrlCredits.m_dwBackgroundPaintLParam = (DWORD)this;					// -||-
	m_ctrlCredits.Create(NULL, WS_CHILD | WS_VISIBLE, rCredits, this);
	m_ctrlCredits.m_nTimerSpeed = nOldSpeed;

	m_iTranslucency = 0;
	SetTimer(1, (256/7)+2, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnTimer(UINT nIDEvent)
{
	if (m_iTranslucency < 255)
	{
		m_Layered.SetLayeredWindowAttributes(m_hWnd, 0, static_cast<BYTE>(m_iTranslucency), LWA_ALPHA);
		m_iTranslucency += 7;
	}
	else
	{
		m_Layered.SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);
		KillTimer(nIDEvent);
	}
	CDialog::OnTimer(nIDEvent);
}

void CAboutDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDblClk(nFlags, point);
	CDialog::OnOK();
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLButtonDown = true;
	SetCapture();
	ClientToScreen(&point);
	m_cpLastPoint = point;
	CDialog::OnLButtonDown(nFlags, point);
}

void CAboutDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bLButtonDown)
	{
		m_bLButtonDown = false;
		ReleaseCapture();
		m_Layered.SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bLButtonDown)
	{
		CRect r;

		GetClientRect(&r);
		ClientToScreen(&r);
		ClientToScreen(&point);
		if (r.PtInRect(point))
		{
			m_Layered.SetLayeredWindowAttributes(m_hWnd, 0, 128, LWA_ALPHA);
			GetWindowRect(&r);
			MoveWindow(r.left - (m_cpLastPoint.x - point.x), r.top - (m_cpLastPoint.y - point.y), r.Width(), r.Height());
			m_cpLastPoint = point;
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CAboutDlg::OnCancelMode()
{
	if (m_bLButtonDown)
	{
		m_bLButtonDown = false;
		m_Layered.SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);
	}
	CWnd::OnCancelMode();
}
