//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#include "MuleListCtrl.h"
#include "Loggable.h"


#define SFL_ALT_REQUESTS	(SFL_NUMCOLUMNS + 0)
#define SFL_ALT_ACCEPTED	(SFL_NUMCOLUMNS + 1)
#define SFL_ALT_TRANSFERRED	(SFL_NUMCOLUMNS + 2)
#define SFL_ALT_PARTTRAFFIC	(SFL_NUMCOLUMNS + 3)
#define SFL_ALT_UPLOADS		(SFL_NUMCOLUMNS + 4)
#define SFL_ALT_COMPLETESRC	(SFL_NUMCOLUMNS + 5)

class CKnownFile;
class CSharedFileList;
class CShellContextMenu;

typedef struct
{
	bool		isFile;
	bool		isOpen;
	uint16		part;
	uint16		parts;
	CKnownFile	*knownFile;
} sfl_itemdata;

class CSharedFilesCtrl : public CMuleListCtrl, public CLoggable
{
	DECLARE_DYNAMIC(CSharedFilesCtrl)

public:
	CSharedFilesCtrl();
	virtual ~CSharedFilesCtrl();
	void	Init();
	void	ShowFileList(CSharedFileList* in_sflist);
	void	AddFile(CKnownFile *file);
	void	RemoveFile(CKnownFile* toremove);
	void	UpdateFile(CKnownFile* file, uint32 pos, bool resort=true);
	void	UpdateItem(CKnownFile* toupdate, bool resort=true);
	void	SortInit(int iSortCode);
	void	Localize();
	void	ShowFilesCount();
	void	SetColoring(byte mode);
	void	SetDisplay(byte mode, bool redraw);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL DeleteAllItems();
	void	ShowKnownList();
	bool	IsKnownFilesView() const { return m_allYaKnow; }

protected:
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static double CompleteSourcesCmpValue(CKnownFile* item, bool second);
	afx_msg void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	virtual BOOL	OnWndMsg(UINT iMessage,WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual BOOL	OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL	PreTranslateMessage(MSG *pMsg);
	virtual void	OnNMDividerDoubleClick(NMHEADER *pNMHDR);

	DECLARE_MESSAGE_MAP()
private:
	bool			DrawStatusBarFile(CDC* dc, RECT* rect, const CFileStatistic& statistic);
	bool			DrawStatusBarPart(CDC* dc, RECT* rect, const CFileStatistic& statistic, uint16 part);
	void			ShowFile(CKnownFile *file, bool resort=true);
	void			ShowFile(CKnownFile *file, uint32 itemnr, bool resort=true);
	void			TraffiGram(CDC* dc, RECT* rect, const CFileStatistic& statistic, int use, double &bpp, uint64 qwStart=0);

	CImageList		m_imageList;
	static	bool	m_sortParts;
	COLORREF		(*GetTrafficColor)(double);
	byte			m_display;
	byte			m_display_atpte;
	byte			m_display_atbts;
	byte			m_display_atbte;
	byte			m_display_sbts;
	byte			m_display_sbte;

	bool			m_allYaKnow;
	bool			m_trafficgram;

//	For fast prio-sorting
	static uint32	m_p2p[5];

//	Make it Win9x compatible by using only one bitmap, brush...?
	CDC		cdcStatus;
	CBitmap	status;

	int		m_statusWidth;
	CShellContextMenu *m_pSCM;

	bool			m_bSortAscending[SFL_NUMCOLUMNS + 6];
};
