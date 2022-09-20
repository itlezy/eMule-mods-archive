//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "MediaPlayerWnd.h"
#include "otherfunctions.h"
#include "MPlayer.h"
#include "Log.h"
#include "PartFile.h"
#include "KnownFile.h"
#include "emuleDlg.h"
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CONTROL_OFFSET	37
CString GetMillisecondsIntoString(const int ms)
{
	if(ms <= 0)
		return L"--:--";

	CString buffer = L"";

#define _SECOND	(1000)
#define _MINUTE	(60*_SECOND)
#define _HOUR	(60*_MINUTE)

		{
			const UINT uMin = (ms)/_MINUTE;
			const UINT uSec = (ms - uMin*_MINUTE)/_SECOND;
			buffer.Format(L"%02u:%02u", uMin, uSec);
		}

	return buffer;
}

CString m_strCurrentFileShort = L"";
CString GetProgressString(const int ms, const int ms2)
{
	CString buffer = L"";
	buffer.Format(L"%s/%s - %s", GetMillisecondsIntoString(ms), GetMillisecondsIntoString(ms2), m_strCurrentFileShort);
	return buffer;
	
}

CString GetZoomString(CMPlayerDisplaySize size)
{
	switch(size)
	{
		case MPHalfSize:	return L"x0.5";
		case MPDefaultSize:	return L"x1.0";
		case MPDoubleSize:	return L"x2.0";
		case MPFitToWindow:	return L"Toda";
		case MPFullscreen:	return L"Cheia";
	}
	return L"ERROR";
}

// CMediaPlayerWnd dialog

IMPLEMENT_DYNAMIC(CMediaPlayerWnd, CDialog)

BEGIN_MESSAGE_MAP(CMediaPlayerWnd, CResizableDialog)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_VLC_PLAY, OnBnClickedPlay)
	ON_BN_CLICKED(IDC_VLC_STOP, OnBnClickedStop)
	ON_BN_CLICKED(IDC_VLC_PAUSE, OnBnClickedPause)
	ON_BN_CLICKED(IDC_VLC_MUTE, OnBnClickedMute)
	ON_WM_HSCROLL()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SVLC_CALLBACK, OnPlayerMessage)
END_MESSAGE_MAP()

CMediaPlayerWnd::CMediaPlayerWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMediaPlayerWnd::IDD, pParent)
{
	m_strCurrentFile = L"";
	m_strCurrentFileShort = L"";
	m_bInited = false;
	m_pPlayer = new CMPlayer;
}

CMediaPlayerWnd::~CMediaPlayerWnd()
{
	CMPlayerState state = m_pPlayer ? m_pPlayer->GetState() : MPStopped;
	//if "shutdown" is called with a wrong state or the m_player object gets deleted it will lockup!?
	//for now we skip the shutdown/deletion... will create a (small) memleak but better than the lockup
	ASSERT(state == MPClosed || state == MPStopped || state == MPError); 
	if(state == MPClosed || state == MPStopped || state == MPError)
	delete m_pPlayer;
}

void CMediaPlayerWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VLC_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_VLC_STOP, m_btnStop);
	DDX_Control(pDX, IDC_VLC_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_VLC_MUTE, m_btnMute);
	DDX_Control(pDX, IDC_VLC_SEEK, m_ctrlSeekBar);
	DDX_Control(pDX, IDC_VLC_VOLUME, m_ctrlVolumeBar);
	DDX_Control(pDX, IDC_VLC_WINDOWSIZE, m_ctrlWindowSize);
}

void CMediaPlayerWnd::ToggleControls()
{
	CMPlayerState state = m_pPlayer ? m_pPlayer->GetState() : MPStopped;	
	m_ctrlVolumeBar.EnableWindow(m_pPlayer ? TRUE : FALSE);
	m_btnMute.EnableWindow(m_pPlayer ? TRUE : FALSE);
	switch(state)
	{			
		case MPPlaying:
			m_btnPlay.EnableWindow(FALSE);
			m_btnStop.EnableWindow(TRUE);
			m_btnPause.EnableWindow(TRUE);
			break;

		case MPPaused:
			m_btnPlay.EnableWindow(TRUE);
			m_btnStop.EnableWindow(TRUE);
			m_btnPause.EnableWindow(TRUE);
			break;

		case MPOpen:	//file opened
			m_ctrlSeekBar.EnableWindow(m_pPlayer->IsSeekable() ? TRUE : FALSE);
			m_ctrlSeekBar.SetRange(0, m_pPlayer->GetDuration());
			m_ctrlSeekBar.SetPos(0);
			GetDlgItem(IDC_VLC_DURATION)->SetWindowText(GetProgressString(m_ctrlSeekBar.GetPos(), m_ctrlSeekBar.GetRangeMax()));
			//flow over...
		case MPStopped:
			m_btnPlay.EnableWindow(TRUE);
			m_btnStop.EnableWindow(FALSE);
			m_btnPause.EnableWindow(FALSE);
			break;
		case MPLoading:	//undefined...
		case MPClosed:	//undefined...
		case MPError:
			m_ctrlSeekBar.EnableWindow(FALSE);
			m_btnPlay.EnableWindow(FALSE);
			m_btnStop.EnableWindow(FALSE);
			m_btnPause.EnableWindow(FALSE);
			break;
	}
}

BOOL CMediaPlayerWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);
	
	Localize();
	
	AddAnchor(IDC_VLC_PLAY, BOTTOM_LEFT);
	AddAnchor(IDC_VLC_STOP, BOTTOM_LEFT);
	AddAnchor(IDC_VLC_PAUSE, BOTTOM_LEFT);
	AddAnchor(IDC_VLC_MUTE, BOTTOM_RIGHT);
	//
	AddAnchor(IDC_STATIC4, BOTTOM_RIGHT);
	AddAnchor(IDC_STATIC5, BOTTOM_RIGHT);
	//
	AddAnchor(IDC_VLC_VOLUME, BOTTOM_RIGHT);
	AddAnchor(IDC_VLC_WINDOWSIZE_LBL, BOTTOM_RIGHT);
	AddAnchor(IDC_VLC_WINDOWSIZE, BOTTOM_RIGHT);
	AddAnchor(IDC_VLC_DURATION, BOTTOM_CENTER, BOTTOM_RIGHT);
	AddAnchor(IDC_VLC_SEEK, BOTTOM_LEFT, BOTTOM_CENTER);
	AddAnchor(IDC_STATICA1, BOTTOM_LEFT, BOTTOM_RIGHT);

	m_btnPlay.SetWindowText(L"");
	m_btnStop.SetWindowText(L"");
	m_btnPause.SetWindowText(L"");
	m_btnMute.SetWindowText(L"");
	m_btnPlay.SetIcon(L"RESUME");
	m_btnStop.SetIcon(L"STOP");
	m_btnPause.SetIcon(L"PAUSE");
	m_btnMute.SetIcon(L"SPEAKER_OFF");

	m_ctrlWindowSize.SetRange(0, 3); //no "fullscreen" for now
	m_ctrlWindowSize.SetPos(1);
	GetDlgItem(IDC_VLC_WINDOWSIZE_LBL)->SetWindowText(GetZoomString(MPDefaultSize));
	m_ctrlSeekBar.SetRange(0, 100);
	m_ctrlVolumeBar.SetRange(0, 100);
	GetDlgItem(IDC_VLC_DURATION)->SetWindowText(GetProgressString(-1, -1));
	
	ToggleControls();
	Init(true);

	return TRUE;
}


BOOL CMediaPlayerWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;
	}
	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CMediaPlayerWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
}

void CMediaPlayerWnd::Localize()
{
}

//BuGFiX:
//if you switch to another window and come back to mediaplayer page then no vid will be shown
void CMediaPlayerWnd::Refresh()
{
	if(m_bInited)
		m_pPlayer->SetDisplaySize((CMPlayerDisplaySize)m_ctrlWindowSize.GetPos());
}

void CMediaPlayerWnd::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);
	if(m_bInited)
	{
		if(cx || cy)
		{
			CRect rect;
			GetWindowRect(&rect);
			m_pPlayer->Height = cy-CONTROL_OFFSET; //bottom row;
			m_pPlayer->Width = cx;
			m_pPlayer->Handle = GetSafeHwnd();
			m_pPlayer->Resize();
		}
		Refresh();
	}
}

BOOL CMediaPlayerWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

bool	CMediaPlayerWnd::CanPlayFileType(const CKnownFile* file)
{
	// check the file extension
	switch(GetED2KFileTypeID(file->GetFileName()))
	{
		case ED2KFT_AUDIO:		//clear...
		case ED2KFT_VIDEO:		//clear...
		//My Build not do this !
		//		case ED2KFT_ARCHIVE:	//handle with care... archives containing multimedia files *should* be playable... I love the VLC for that
		//My Build not do this !		
			return true;
		default:
			return false;
	}		
}

bool CMediaPlayerWnd::StartPlayback(const CKnownFile* file)
{
	ASSERT(file != NULL);
	if(!Init() || file == NULL)
		return false;
	
	CString strCurrentFile = L""; 
	if(file->IsPartFile())
		strCurrentFile = RemoveFileExtension(((CPartFile*)file)->GetFullName());
	else
		strCurrentFile = file->GetFilePath();
//	if(_tcsicmp(m_strCurrentFile, strCurrentFile) != 0) //already playing ;)
	//{ Dream not check if the file is playing

	OnBnClickedStop();
	m_strCurrentFile = strCurrentFile;
		const int pos = m_strCurrentFile.ReverseFind(L'\\');
		if(pos != -1)
			m_strCurrentFileShort = m_strCurrentFile.Mid(pos+1);
		else
			m_strCurrentFileShort = m_strCurrentFile;

		if(file->IsPartFile())
			((CPartFile*)file)->FlushBuffer(true);
	if(!m_pPlayer->Play(m_strCurrentFile))
	{		
		theApp.QueueLogLineEx(LOG_ERROR, L"VLC: Não pude rodar \"%s\"", m_strCurrentFile);
		OnBnClickedStop();
		return false;
	}
	//update seek controls
//		if(m_pPlayer->IsSeekable())
//			m_ctrlSeekBar.EnableWindow(TRUE);
//		else
//			m_ctrlSeekBar.EnableWindow(FALSE);
//		m_ctrlSeekBar.SetRange(0, m_pPlayer->GetDuration());
//		m_ctrlSeekBar.SetPos(0);
//		GetDlgItem(IDC_VLC_DURATION)->SetWindowText(GetProgressString(m_ctrlSeekBar.GetPos(), m_ctrlSeekBar.GetRangeMax()));

	m_btnPlay.EnableWindow(FALSE);
	m_btnStop.EnableWindow(TRUE);
	m_btnPause.EnableWindow(TRUE);
		if(GetED2KFileTypeID(file->GetFileName()) != ED2KFT_AUDIO)
	theApp.emuledlg->SetActiveDialog(this);
	//} Dream not check if the file is playing
	return true;
}

void CMediaPlayerWnd::StopFile(const CKnownFile* file)
{
	ASSERT(file != NULL);
	if(!Init() || file == NULL || m_strCurrentFile.IsEmpty())
		return;

	CString strCurrentFile = L""; 
	if(file->IsPartFile())
		strCurrentFile = RemoveFileExtension(((CPartFile*)file)->GetFullName());
	else
		strCurrentFile = file->GetFilePath();
	if(_tcsicmp(m_strCurrentFile, strCurrentFile) == 0) //playing that file?
	{
		m_strCurrentFile = L"";
		OnBnClickedStop();
	}
}

bool CMediaPlayerWnd::Init(const bool bTryToLoad /*= false*/)
{
	if(!bTryToLoad)
		return m_bInited;

	Uninit();

	CRect rect;
	GetWindowRect(&rect);
	m_pPlayer->Height = rect.Height()-CONTROL_OFFSET; //bottom row
	m_pPlayer->Width = rect.Width();
	m_pPlayer->Handle = GetSafeHwnd();
	m_pPlayer->Resize();
	if(m_pPlayer->Initialize())
	{		
		//toggle mute
		//boizaum no mute at start
		/*if(thePrefs.VLC_IsMuted())
		{
			//I know it's ugly... but it works :P
			thePrefs.VLC_SetMuted(!thePrefs.VLC_IsMuted());
			OnBnClickedMute(); 
		}
		*/
		m_ctrlVolumeBar.SetPos(thePrefs.VLC_GetVolume());
		m_bInited = true;
		Refresh(); //update "zoom"
		ToggleControls();
	}
	else
		theApp.QueueLogLineEx(LOG_ERROR, GetResString(IDS_VLC_INIT_ERROR));

	return m_bInited;
}

bool CMediaPlayerWnd::Uninit()
{
	if(!m_bInited)
		return false;

	CMPlayerState state = m_pPlayer->GetState();
	int i = 0;
	//dunno why... this state is (sometimes) not reached?! but stop() command does not fail either!?
	while(i < 10 && state != MPClosed && state != MPStopped && state != MPError)
	{
		++i;
	OnBnClickedStop();
		Sleep(100); //wait for close... hmmm
		state = m_pPlayer->GetState();
	}	
	m_bInited = false;
	ToggleControls();
	//if "shutdown" is called with a wrong state or the m_player object gets deleted it will lockup!?
	//for now we skip the shutdown/deletion... will create a (small) memleak but better than the lockup
	ASSERT(state == MPClosed || state == MPStopped || state == MPError); 
	if(state == MPClosed || state == MPStopped || state == MPError)
	m_pPlayer->Shutdown();

	return true;
}

void CMediaPlayerWnd::OnBnClickedPlay()
{
	if(!Init())
		return;

	if(m_pPlayer->IsPaused())
		m_pPlayer->Pause(false);
	else
		m_pPlayer->Play(m_strCurrentFile);
	m_btnPlay.EnableWindow(FALSE);
	m_btnPause.EnableWindow(TRUE); 
	m_btnStop.EnableWindow(TRUE);
}

void CMediaPlayerWnd::OnBnClickedStop()
{
	if(!Init())
		return;

	m_pPlayer->Stop();	

	m_ctrlSeekBar.SetPos(0);
	if(GetSafeHwnd()) //during exit...
		GetDlgItem(IDC_VLC_DURATION)->SetWindowText(GetProgressString(-1, -1));
	m_btnStop.EnableWindow(FALSE);
	m_btnPause.EnableWindow(FALSE); 
	m_btnPlay.EnableWindow(TRUE); 
}

void CMediaPlayerWnd::OnBnClickedPause()
{
	if(!Init())
		return;

	m_pPlayer->TogglePause();
//	m_btnPause.EnableWindow(FALSE);
	m_btnPlay.EnableWindow(TRUE); 
	m_btnStop.EnableWindow(TRUE); 
}

void CMediaPlayerWnd::OnBnClickedMute()
{
	if(thePrefs.VLC_IsMuted())
	{
		m_btnMute.SetIcon(L"SPEAKER_OFF");
		m_pPlayer->Mute(false);
	}
	else
	{
		m_btnMute.SetIcon(L"SPEAKER_ON");
		m_pPlayer->Mute(true);
	}
	thePrefs.VLC_SetMuted(!thePrefs.VLC_IsMuted());
}

void CMediaPlayerWnd::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar)
{
	if (pScrollBar->GetSafeHwnd() == m_ctrlVolumeBar.m_hWnd)
	{
		const int vol = m_ctrlVolumeBar.GetPos();
		m_pPlayer->SetVolume((float)vol);
		thePrefs.VLC_SetVolume(vol);
	}
	else if (pScrollBar->GetSafeHwnd() == m_ctrlSeekBar.m_hWnd)
	{
		if(m_ctrlSeekBar.GetRangeMax() != 0)
		{			
			float pos = (float)( (double)m_ctrlSeekBar.GetPos()/(double)m_ctrlSeekBar.GetRangeMax() );
			m_pPlayer->SetPosition(pos);
		}
	}
	else if(pScrollBar->GetSafeHwnd() == m_ctrlWindowSize.m_hWnd)
	{
		CMPlayerDisplaySize size = (CMPlayerDisplaySize)m_ctrlWindowSize.GetPos();
		m_pPlayer->SetDisplaySize(size);
		GetDlgItem(IDC_VLC_WINDOWSIZE_LBL)->SetWindowText(GetZoomString(size));
	}

//	CResizableDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}



LRESULT CMediaPlayerWnd::OnPlayerMessage(WPARAM wParam, LPARAM lParam)
{
	SVlcCallbackEvent event = (SVlcCallbackEvent)wParam;
    switch(event) 
	{
	    case SVLC_CB_STATE_CHANGE: 
		{
	        //// call user
	        //if(FOnStateChange)
	        //    FOnStateChange(this, ConvertVlcState(((SVlcCbStateData*)Msg.LParam)->new_state));
			ToggleControls();
	        break;
	    }
	    
		case SVLC_CB_POSITION_CHANGE: 
		{
			ASSERT(m_pPlayer && m_pPlayer->IsSeekable());
			const int pos = (int) (((SVlcCbPositionChangeData*)lParam)->position * m_ctrlSeekBar.GetRangeMax());
			m_ctrlSeekBar.SetPos(pos);
			GetDlgItem(IDC_VLC_DURATION)->SetWindowText(GetProgressString(m_ctrlSeekBar.GetPos(), m_ctrlSeekBar.GetRangeMax()));
	        break;
	    }

	    case SVLC_CB_DISPLAY_POPUP: 
		{
			ASSERT(0); //not requested
	        //// call user
	        //if(FOnDisplayPopup)
	        //    FOnDisplayPopup(this, ((SVlcCbDisplayPopupData*)Msg.LParam)->show);
	        break;
	    }

	    case SVLC_CB_KEY_PRESSED: 
		{
			ASSERT(0); //not requested
	        //if(!FOnKeyPress)
	        //    break;
	
	        //int VlcKey = ((SVlcCbKeyPressedData*)Msg.LParam)->key;
	        //char Key = 0;
	        //// do some converting
	        //if(VlcKey & SVLC_KEY_ASCII) {
	        //    Key = (char)(VlcKey & SVLC_KEY_ASCII);
	        //} else {
	        //    switch(VlcKey) {
	        //    case SVLC_KEY_SPACE:     Key = ' ';
	        //    case SVLC_KEY_ENTER:     Key = '\n';
	        //    case SVLC_KEY_ESC:       Key = 27;
	        //    case SVLC_KEY_TAB:       Key = '\t';
	        //    case SVLC_KEY_BACKSPACE: Key = '\r';
	        //    }
	        //}
	        //// call user
	        //if(Key > 0)
	        //    FOnKeyPress(this, Key);
	        break;
	    }
	}

	return 0;
}
