/*	CStatisticsTree Class Header File by Khaos
	Copyright (C) 2003

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	This file is a part of the KX mod, and more
	specifically, it is a part of my statistics
	add-on.

	The purpose of deriving a custom class from CTreeCtrl
	was to provide another level of customization and control.
	This allows us to easily code complicated parsing features
	and a context menu.
*/
#pragma once

// -khaos--+++> Items for our stat's tree context menu.
#define	MP_STATTREE_RESET		10950
#define	MP_STATTREE_RESTORE		10951
#define	MP_STATTREE_COPYSEL		10952
#define	MP_STATTREE_COPYVIS		10953
#define MP_STATTREE_EXPANDALL	10960
#define MP_STATTREE_COLLAPSEALL 10961
#define MP_STATTREE_EXPANDMAIN	10962

class CStatisticsTree : public CTreeCtrl
{
	DECLARE_DYNAMIC(CStatisticsTree)
public:
	CStatisticsTree();
	~CStatisticsTree();
	virtual		BOOL	OnCommand( WPARAM wParam, LPARAM lParam );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg		void	OnLButtonDown( UINT nFlags, CPoint point );// X: [AC] - [ActionChange] don't refresh when collapse an item
	afx_msg		void	OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg		void	OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg		void	OnContextMenu( CWnd* pWnd, CPoint point );
	afx_msg		void	OnItemExpanded( NMHDR* pNMHDR, LRESULT* pResult );

public:
	void				Init();
	bool				CopyText( uint_ptr copyMode = MP_STATTREE_COPYSEL );
	CString				GetText( bool onlyVisible = true, HTREEITEM theItem = NULL, int theItemLevel = 0, bool firstItem = true );
	BOOL				IsBold( HTREEITEM theItem );
	BOOL				IsExpanded( HTREEITEM theItem );
	BOOL				CheckState( HTREEITEM hItem, UINT state );
	void				DoMenu();
	void				DoMenu(CPoint doWhere);
	void				DoMenu(CPoint doWhere, UINT nFlags);
	void				ExpandAll(bool onlyBold = false, HTREEITEM theItem = NULL);
	void				CollapseAll(HTREEITEM theItem = NULL);
	void				DeleteChildItems(HTREEITEM parentItem); //Xman extended stats (taken from emule plus)
	void				DeleteRestChildItems (HTREEITEM hChildItem);
	void				DeleteRestChildItemsR (HTREEITEM hChildItem);
	CString				GetExpandedMask(HTREEITEM theItem = NULL);
	int					ApplyExpandedMask(CString theMask, HTREEITEM theItem = NULL, int theStringIndex = 0);
private:
	CMenu			mnuContext;
	bool				m_bExpandingAll;
	bool				m_bRefresh;// X: [AC] - [ActionChange] don't refresh when collapse an item
};