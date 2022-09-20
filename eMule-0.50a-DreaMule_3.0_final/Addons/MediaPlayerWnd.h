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
#pragma once
#include "ResizableLib\ResizableDialog.h"
#include "BtnST.h"

class CMPlayer;
class CKnownFile;
class CMediaPlayerWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CMediaPlayerWnd)

public:
	CMediaPlayerWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMediaPlayerWnd();

	void Localize();

// Dialog Data
	enum { IDD = IDD_MEDIAPLAYER };

	bool	StartPlayback(const CKnownFile* file);
	bool	Init(const bool bTryToLoad = false);
	bool	CanPlayFileType(const CKnownFile* file);
	void	Refresh();
	bool	Uninit();
	void	StopFile(const CKnownFile* file);
	
private:
	CString m_strCurrentFile;
	bool	m_bInited;
	CMPlayer* m_pPlayer;

protected:
	void		ToggleControls();

	bool		m_bMuted;
	CButtonST	m_btnPlay;
	CButtonST	m_btnStop;
	CButtonST	m_btnPause;
	CButtonST	m_btnMute;
	CSliderCtrl	m_ctrlSeekBar;
	CSliderCtrl	m_ctrlVolumeBar;
	CSliderCtrl	m_ctrlWindowSize;

	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedMute();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnPlayerMessage(WPARAM wParam,LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};