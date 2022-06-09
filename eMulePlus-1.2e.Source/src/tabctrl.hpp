#ifndef _TABCTRL_HPP
#define _TABCTRL_HPP

#pragma once

#include <afxwin.h>
#include <afxcmn.h>
#include <windows.h>

#define NM_TABMOVED WM_USER+0x101+0x12

// Standard C++ includes
#include <vector>

/************************************************
*
*                   TAB CONTROL
*
************************************************/

class TabControl: public CTabCtrl
{
public:
  TabControl();
  virtual ~TabControl();
  UINT GetLastMovementSource()	{return m_nSrcTab;}
	UINT		GetLastMovementDestination()	{return m_nDstTab;}
  bool ReorderTab( unsigned int nSrcTab, unsigned int nDstTab );

  // Overriden CTabCtrl/CWnd members
//  virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

  // Command/Notification Handlers
  afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
  afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
  afx_msg void OnMouseMove( UINT nFlags, CPoint point );
  afx_msg void OnCaptureChanged( CWnd* );
//  afx_msg void OnPaint();
  
private:
  bool  m_bDragging;     // Specifies that whether drag 'n drop is in progress.
  UINT  m_nSrcTab;       // Specifies the source tab that is going to be moved.
  UINT  m_nDstTab;       // Specifies the destination tab (drop position).
  bool  m_bHotTracking;  // Specifies the state of whether the tab control has hot tracking enabled.

  BOOL  DragDetectPlus(CWnd* Handle, CPoint p);

  CRect m_InsertPosRect;
  CPoint m_lclickPoint;

  CSpinButtonCtrl * m_pSpinCtrl;
  
  // Utility members
  bool DrawIndicator( CPoint point );
  
  DECLARE_MESSAGE_MAP()
};

#endif _TABCTRL_HPP

// END TABCTRL.HPP
