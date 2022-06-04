#if _MSC_VER>=1600
//
// CButtonVE Owner Drawn WinXP/Vista themes aware button implementation...
//
// Rev 1.0 - Apr 24th - Original Release
// Rev 1.1 - Apr 27th - Hans Dietrich noticed problem with default state on WinXP.
//                      Jerry Evans pointed out inadequate handling of uistate.
//                      Also improved transitions.
// Rev 1.2 -          - Added support for checkbox style pushbutton mode if BS_PUSHLIKE style set.
//                      Fixed rare & hard to reproduce screen flicker in animated default state.
//
#ifndef _BUTTONVE_H_
#define _BUTTONVE_H_
#include "VisualStylesXP.h"
#include "ButtonVE_Helper.h"

// Need library containing AlphaBlend function...
#pragma comment (lib,"msimg32.lib")

// Add few defines if missing from SDK...
#ifndef WM_QUERYUISTATE
#define WM_QUERYUISTATE 0x129
#define WM_UPDATEUISTATE 0x128
#define UISF_HIDEFOCUS 0x1
#define UISF_HIDEACCEL 0x2
#define UISF_ACTIVE    0x4
#define DT_HIDEPREFIX 0x00100000
#endif

#ifndef BUTTONVE_TIMER
#define BUTTONVE_TIMER 200
#endif

class CButtonVE_Menu;
class CButtonVE_Image;
class CButtonVE_Task;



/////////////////////////////////////////////////////////////////////////////
//
// CButtonVE owner-drawn button class...
//
class CButtonVE : public CButton, public CButtonVE_Helper
{
  DECLARE_DYNAMIC(CButtonVE)
  friend CButtonVE_Menu;
  friend CButtonVE_Image;
  friend CButtonVE_Task;

//
// Attributes...
//
public:
  BOOL m_vista_test;                    // If TRUE, running on Vista.
  BOOL m_timer_active;                  // Timer has been initialized.
  BOOL m_self_delete;                   // Track if we should self-delete.
  BOOL m_button_hot;                    // Mouse hovering over button.
  BOOL m_button_default;                // Button is the current default.
  CSize m_content_margin;               // Minimum margin between border & text.
  HWND m_owner;                         // Receives control messages from menu

  int m_stateid;                        // Current button state.
  int m_defaultbutton_tickcount;        // # of timer ticks remaining for transition.
  int m_check_state;                    // If pushbutton checkbox, our current state.

  CBitmap *m_oldstate_bitmap;           // Bitmap of previous button state.
  int m_transition_tickcount;           // # of timer ticks remaining for transition.
  int m_transition_tickscale;           // Alpha multipler for timer ticks.

  BOOL m_bgcolor_valid;                 // User specified a desired background color.
  COLORREF m_bgcolor;
  HBRUSH m_parent_hBrush;


//
// Constructor/Destructor...
//
public:
  CButtonVE() : CButton() {
    m_vista_test=-1;
    m_timer_active=FALSE;
    m_self_delete=FALSE;
    m_button_hot = FALSE;
    m_button_default = FALSE;
    m_content_margin = CSize(0,0);
    m_owner = NULL;
    m_stateid = -1;
    m_defaultbutton_tickcount=0;
    m_oldstate_bitmap = NULL;
    m_transition_tickcount=0;
    m_transition_tickscale=25;
    m_check_state = BST_UNCHECKED;
    m_bgcolor_valid = FALSE;
    m_bgcolor = ::GetSysColor(COLOR_BTNFACE);
    m_parent_hBrush = NULL;
  }
  virtual ~CButtonVE(){}


//
// Configuration Operations...
//
public:
  // Indicate who receives command messages...
  void SetOwner(CWnd *owner) {m_owner = owner->GetSafeHwnd();}
  void SetOwner(HWND owner) {m_owner = owner;}

  // Set content horizontal position.  Valid param: BS_LEFT, BS_RIGHT, BS_CENTER
  void SetContentHorz (DWORD horzpos) {
    ModifyStyle(BS_CENTER, horzpos & BS_CENTER);
    Invalidate();
  }

  // Set content vertical position.  Valid param: BS_TOP, BS_BOTTOM, BS_VCENTER
  void SetContentVert (DWORD vertpos) {
    ModifyStyle(BS_VCENTER, vertpos & BS_VCENTER);
    Invalidate();
  }

  // Set spacing between button content & border...
  void SetContentMargin(CSize margin)
    {m_content_margin = margin;}

  // Permit easy setting of non-theme default background color...
  void SetBackgroundColor(COLORREF bgcolor) {
    m_bgcolor_valid = (bgcolor != -1);
    m_bgcolor = (m_bgcolor_valid) ? bgcolor : ::GetSysColor(COLOR_BTNFACE);
  }


//
// Internal Operations...
//
private:
  // Return TRUE if running on Vista...
  BOOL UseVistaEffects() 
  {
    if (m_vista_test<0) {
      OSVERSIONINFO osvi;
      memset (&osvi,0,sizeof(osvi));
      osvi.dwOSVersionInfoSize = sizeof(osvi);
      m_vista_test = GetVersionEx(&osvi) ? (osvi.dwPlatformId==VER_PLATFORM_WIN32_NT) && (osvi.dwMajorVersion>=6) : FALSE;
    }
    return m_vista_test;
  }


  // Detect if mouse floating over button...
  BOOL HitTest() {
    HWND hWnd = GetSafeHwnd();
    if (hWnd==NULL) return FALSE;
    // If mouse captured by someone other than us, indicate no hit...
    CWnd *capWnd = GetCapture();
    if ((capWnd != NULL) && (capWnd != this)) return FALSE;
    // Quit if mouse not over us...
    CPoint pt; ::GetCursorPos(&pt);
    CRect rw; GetWindowRect(&rw);
    if (!rw.PtInRect(pt)) return FALSE;
    // Get the top-level window that is under the mouse cursor...
    HWND hWndMouse = ::WindowFromPoint(pt);
    if (!::IsWindowEnabled(hWndMouse)) return FALSE;
    if (hWndMouse==hWnd) return TRUE;
    // Convert (x,y) from screen to parent window's client coordinates...
    ::ScreenToClient(hWndMouse, &pt);
    // Verify child window (if any) is under the mouse cursor is us...
    return (::ChildWindowFromPointEx(hWndMouse, pt, CWP_ALL)==hWnd);
  }


  // Return pixel size of text (single or multiline)...
  virtual CSize GetTextSize(CDC &dc, CString button_text, CRect textrc, int text_format)
  {
    // Remove any non-visible ampersand characters...
    CString filter_text;
    int textlen = button_text.GetLength();
    BOOL got_ampersand = FALSE;
    for (int i=0; i<textlen; i++) {
      TCHAR newchar = button_text[i];
      if (newchar == _T('&')) {
        if (got_ampersand) {filter_text += newchar;}
        got_ampersand = !got_ampersand;
      }
      else {
        filter_text += newchar;
        got_ampersand = FALSE;
      }
    }

    CSize textsize(0,0);
    if (!filter_text.IsEmpty())
      if ((filter_text.Find(_T("\n"))<0) || (text_format & DT_SINGLELINE)) // single line string...
        textsize = dc.GetTextExtent(filter_text);
      else {
        CString temp;
        int textlen = filter_text.GetLength();
        for (int i=0; i<textlen; i++) {
          TCHAR newchar = filter_text.GetAt(i);
          if (newchar == _T('\n')) {
            if (!temp.IsEmpty()) {
              CSize tempsize = dc.GetTextExtent(temp);
              if (tempsize.cx > textsize.cx) textsize.cx = tempsize.cx;
            }
            temp.Empty();
          }
          else temp += newchar;
        }

        if (!temp.IsEmpty()) {
          CSize tempsize = dc.GetTextExtent(temp);
          if (tempsize.cx > textsize.cx) textsize.cx = tempsize.cx;
        }

        CRect sizerc(textrc);
        dc.DrawText(filter_text, sizerc, DT_CALCRECT);
        textsize.cy = sizerc.Height();        
      }
    return textsize;
  }


//
// Overrideable...
//
private:

  // Draw button text content...
  virtual void DrawTextContent (
    CDC &dc,              // Drawing context
    HTHEME hTheme,        // XP/Vista theme (if available).  Use g_xpStyle global var for drawing.
    int uistate,          // Windows keyboard/mouse ui styles.  If UISF_HIDEACCEL set, should hide underscores.
    CRect /*rclient*/,        // Button outline rectangle.
    CRect /*border*/,         // Content rectangle (specified by theme API).
    CRect textrc,         // Text rectangle.
    int text_format,      // DrawText API formatting.
    BOOL enabled,         // Set if button enabled
    CString button_text)
  {
    int oldmode = dc.SetBkMode(TRANSPARENT);

    // Adjust coordinates for multiline text...
    CRect textsize(textrc);
    dc.DrawText(button_text, textsize, DT_CALCRECT | (DT_SINGLELINE & text_format));
    if (text_format & DT_VCENTER) {
      textrc.top += (textrc.Height()-textsize.Height())/2;
      text_format = (text_format & ~DT_VCENTER) | DT_TOP;
    }
    if (text_format & DT_BOTTOM) {
      textrc.top = textrc.bottom - textsize.Height();
      text_format = (text_format & ~DT_BOTTOM) | DT_TOP;
    }

    if (text_format & DT_SINGLELINE) // remove any newlines
      button_text.Replace(_T("\n"),_T(" "));

    if (uistate & UISF_HIDEACCEL) text_format |= DT_HIDEPREFIX;

    if (hTheme) {
      int button_textlen = button_text.GetLength();
      if (button_textlen) {
#ifdef UNICODE
        g_xpStyle.DrawThemeText(hTheme, dc, BP_PUSHBUTTON, m_stateid, button_text, button_textlen, 
          text_format, NULL, &textrc);
#else
        int widelen = MultiByteToWideChar(CP_ACP, 0, button_text, button_textlen+1, NULL, 0);
        WCHAR *pszWideText = new WCHAR[widelen+1];
        if (pszWideText) {
          MultiByteToWideChar(CP_ACP, 0, button_text, button_textlen, pszWideText, widelen);
          g_xpStyle.DrawThemeText(hTheme, dc, BP_PUSHBUTTON, m_stateid, pszWideText, button_textlen, 
            text_format, NULL, &textrc);
          delete [] pszWideText;
        }
#endif
      }
    }
    else {
      dc.SetBkColor(m_bgcolor);

      if (enabled) {
        dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
        dc.DrawText(button_text, textrc, text_format);
      }
      else {
        dc.SetTextColor(::GetSysColor(COLOR_BTNHIGHLIGHT));
        dc.DrawText(button_text, textrc+CPoint(1,1), text_format);
        dc.SetTextColor(::GetSysColor(COLOR_BTNSHADOW));
        dc.DrawText(button_text, textrc, text_format);
      }
    }

    dc.SetBkMode(oldmode);
  }


  // Draw button content (after background prepared)...
  virtual void DrawContent (
    CDC &dc,         // Drawing context
    HTHEME hTheme,   // XP/Vista theme (if available).  Use g_xpStyle global var for drawing.
    int uistate,     // Windows keyboard/mouse ui styles.  If UISF_HIDEACCEL set, should hide underscores.
    CRect rclient,   // Button outline rectangle.
    CRect border,    // Content rectangle (specified by theme API).
    CRect textrc,    // Text rectangle.
    int text_format, // DrawText API formatting.
    BOOL enabled)    // Set if button enabled.
  {
    CString button_text;
    GetWindowText(button_text);
    CFont *oldfont = (CFont*)dc.SelectObject(GetFont());
    DrawTextContent (dc, hTheme, uistate, rclient, border, textrc, text_format, enabled, button_text);
    dc.SelectObject(oldfont);
  }


  // Draw themed button background...
  virtual void DrawThemedButtonBackground(HDC /*hDC*/, CDC &mDC, HTHEME hTheme, int stateid, CRect &rc, CRect &border)
  {
	// Fill background with button face to avoid nasty border around buttons
	mDC.FillSolidRect(rc, m_bgcolor);

    // Draw themed button background...
    if (g_xpStyle.IsThemeBackgroundPartiallyTransparent(hTheme, BP_PUSHBUTTON, stateid))
      g_xpStyle.DrawThemeParentBackground(m_hWnd, mDC, &rc);
    g_xpStyle.DrawThemeBackground (hTheme, mDC, BP_PUSHBUTTON, stateid, &rc, NULL);

    // Get rect for content...
    border = rc;
    g_xpStyle.GetThemeBackgroundContentRect (hTheme, mDC, BP_PUSHBUTTON, stateid, &border, &border);
  }


  // Modify button background into transition state...
  virtual void DrawThemedVistaEffects(HDC hDC, CDC &mDC, HTHEME hTheme, int stateid, CRect &rc, CRect &border)
  {
    // Simulate Vista button effects...
    if (UseVistaEffects() && (m_transition_tickcount>0)) { // transitioning into new state...
      CDCBitmap tempDC(hDC,m_oldstate_bitmap);
      AlphaBlt (mDC, rc, tempDC, rc, m_transition_tickcount*m_transition_tickscale);
      m_defaultbutton_tickcount=0;
    }

    // Simulate pulsing Vista default button...
    if (UseVistaEffects() && (m_transition_tickcount==0) && (stateid == PBS_DEFAULTED)) {
      // Copy "default" button state to temp bitmap...
      CBitmap tempbmp;
      tempbmp.CreateCompatibleBitmap(CDC::FromHandle(hDC),rc.Width(),rc.Height());
      CDCBitmap tempDC(mDC,tempbmp);
      AlphaBlt (tempDC, rc, mDC, rc, 255);

      // Compute "glow" alpha...  0->250->0 over 40 ticks.
      int alpha = (int)(m_defaultbutton_tickcount*12.5);
      if (m_defaultbutton_tickcount>=20) alpha = 500-alpha;

      // Thicken content border...
      CRect rect(border);
      rect.InflateRect(1,1);
      AlphaBlt (mDC, border, tempDC, rect, alpha/2);

      // Render hot button state...
      CRect temprc;
      DrawThemedButtonBackground(hDC, tempDC, hTheme, PBS_HOT, rc, temprc);

      // Blend the hot-state content area...
      border.DeflateRect(1,1);
      AlphaBlt (mDC, border, tempDC, border, alpha);
      border.InflateRect(1,1);
    }
  }


  // Draw non-themed button background...
  virtual void DrawNonThemedButtonBackground(HDC /*hDC*/, CDC &mDC, BOOL button_focus, UINT state, CRect rc, CRect &border)
  {
    CRect rcback(border);
    if (!m_bgcolor_valid && m_parent_hBrush)
      mDC.FillRect(border, CBrush::FromHandle(m_parent_hBrush));
    else mDC.FillSolidRect(border,m_bgcolor);

    if (button_focus & ((state & DFCS_PUSHED)==0)) rcback.DeflateRect(1,1);
    mDC.DrawFrameControl(rcback,DFC_BUTTON,state);

	// Hardcoded the edge style to match other parts of eMule on no theme
	mDC.DrawEdge(rcback, EDGE_ETCHED, BF_FLAT | BF_MONO | BF_RECT);

    COLORREF btntext = ::GetSysColor(COLOR_BTNTEXT);
    if (button_focus) mDC.Draw3dRect(rc,btntext,btntext);
    border.DeflateRect(4,4);
  }


  // Called by CButton::OnChildNotify handler for owner drawn buttons.   Works great,
  // as long as parent window reflects notify messages back to the button.
  virtual void DrawItem(LPDRAWITEMSTRUCT di)
  {
    if (!m_timer_active) {
      m_timer_active = TRUE;
      ::SetTimer(m_hWnd,BUTTONVE_TIMER,50,NULL);
    }

    if (di->CtlType != ODT_BUTTON) return;

    // Create memory drawing context...
    CRect rc, textrc;
    GetClientRect(&rc);
    CDCBitmap mDC(di->hDC,rc);
    CRect border(rc);

    // Colors...
    //COLORREF btntext = ::GetSysColor(COLOR_BTNTEXT); // unused
    COLORREF btnface = m_bgcolor;

    // Query parent for background color with WM_CTLCOLOR message...
    m_parent_hBrush = NULL;
    CWnd *parent = GetParent();
    if (parent) m_parent_hBrush = (HBRUSH)(parent->SendMessage(
      WM_CTLCOLORBTN, (WPARAM)di->hDC, (LPARAM)GetSafeHwnd()));

    // Get Win2k/XP UI state...  Controls if focus rectangle is 
    // hidden, making things cleaner unless keyboard nav used.
    int uistate = SendMessage(WM_QUERYUISTATE,0,0);
    if (uistate<0) uistate=0;

    UINT btnstate = GetState();
    DWORD btn_style = GetStyle();
    m_button_hot = HitTest();

    BOOL button_pressed = (btnstate & 4);
    BOOL button_focus = GetFocus()==this;
    BOOL draw_pressed = button_pressed || ((btn_style & BS_PUSHLIKE) && m_check_state);

    // If not enabled, make sure to cleanup any left-over "pressed" state indications...
    BOOL enabled = IsWindowEnabled();
    if (!enabled) {
      button_pressed = FALSE;
      draw_pressed = FALSE;
      m_button_default = FALSE;
      m_button_hot = FALSE;
    }

    // Get theme handle...
    HTHEME hTheme = NULL;
    if (g_xpStyle.IsAppThemed() && g_xpStyle.IsThemeActive())
      hTheme = g_xpStyle.OpenThemeData (m_hWnd, L"Button");

    if (hTheme) {
      int old_stateid = m_stateid;
      if (!enabled)
        m_stateid = PBS_DISABLED;
      else if (draw_pressed)
        m_stateid = PBS_PRESSED;
      else if (m_button_hot)
        m_stateid = PBS_HOT;
      else if (m_button_default)
        m_stateid = PBS_DEFAULTED;
      else m_stateid = PBS_NORMAL;
      if (old_stateid<0) old_stateid = m_stateid;

      // Create bitmap of button in its "hot" state.  This is used to simulate
      // a pulsing default button (ala Vista)...
      if (UseVistaEffects() && (m_stateid != old_stateid)) {
        // Get snapshot of button in old state...
        CBitmap *new_snapshot = new CBitmap;
        new_snapshot->CreateBitmap(rc.Width(),rc.Height(),1,32,NULL);
        CDCBitmap tempDC(di->hDC, new_snapshot);
        DrawThemedButtonBackground(di->hDC, tempDC, hTheme, old_stateid, rc, border);
        DrawThemedVistaEffects(di->hDC, tempDC, hTheme, old_stateid, rc, border);
        if (m_oldstate_bitmap) delete m_oldstate_bitmap;
        m_oldstate_bitmap = new_snapshot;

        // Select number of 50ms ticks transition should occur over.  Some are faster than others...
        switch (m_stateid) {
          case PBS_HOT : m_transition_tickcount = (old_stateid==PBS_PRESSED) ? 4 : 2; break;
          case PBS_NORMAL : m_transition_tickcount = (old_stateid==PBS_HOT) ? 20 : 4; break;
          case PBS_PRESSED : m_transition_tickcount = 2; break;
          case PBS_DEFAULTED : m_transition_tickcount = (old_stateid==PBS_HOT) ? 20 : 2; break;
          default : m_transition_tickcount = 4; break; 
        }
        m_transition_tickscale = 250/m_transition_tickcount;
      }
      DrawThemedButtonBackground(di->hDC, mDC, hTheme, m_stateid, rc, border);
      DrawThemedVistaEffects(di->hDC, mDC, hTheme, m_stateid, rc, border);
      textrc = border;
    }
    else { // draw non-themed background...
      UINT state = DFCS_BUTTONPUSH;
      if (!enabled) state |= DFCS_INACTIVE;
      else if (draw_pressed) state |= DFCS_PUSHED;
      if (!button_pressed && (btn_style & BS_PUSHLIKE) && m_check_state) state |= DFCS_CHECKED;

      // Draw button frame...
      DrawNonThemedButtonBackground(di->hDC, mDC, button_focus, state, rc, border);

      textrc = border;
      if (button_pressed) textrc += CPoint(1,1);
      textrc.top--;
    }

    textrc.DeflateRect(m_content_margin.cx, m_content_margin.cy);

    // Convert button style into text-format...
    int text_format=0; 
    switch (btn_style & BS_CENTER) {
      case BS_LEFT : text_format |= DT_LEFT; break;
      case BS_RIGHT : text_format |= DT_RIGHT; break;
      default : text_format |= DT_CENTER; break;
    }
    switch (btn_style & BS_VCENTER) {
      case BS_TOP : text_format |= DT_TOP; break;
      case BS_BOTTOM : text_format |= DT_BOTTOM; break;
      default : text_format |= DT_VCENTER; break;
    }
    if ((btn_style & BS_MULTILINE)==0)
      text_format |= DT_SINGLELINE;

    // Draw contents of button...
    DrawContent (mDC, hTheme, uistate, rc, border, textrc, text_format, enabled);

    // Draw focus rectangle...
    if ((uistate & UISF_HIDEFOCUS)==0)
      if (button_focus) {
        mDC.SetBkColor(btnface);
        if (hTheme)
          mDC.DrawFocusRect(border);
        else {
          rc.DeflateRect(4,4);
          mDC.DrawFocusRect(rc);
        }
      }

    // Done with themes...
    if (hTheme) g_xpStyle.CloseThemeData(hTheme);
  }


//
// Message map functions...
//
protected:
  // Creating button window.  Not called if created by dialog resource.
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) return -1;
    m_self_delete = TRUE;
    return 0;
  }

  // Cleanup button window...
  afx_msg void OnDestroy() {
    if (m_timer_active) {::KillTimer(m_hWnd,BUTTONVE_TIMER); m_timer_active = FALSE;}
    if (m_oldstate_bitmap) {delete m_oldstate_bitmap; m_oldstate_bitmap=NULL;}
    CButton::OnDestroy();
  }
  afx_msg void OnNcDestroy() {
    CWnd::OnNcDestroy();
    // Make sure the window was destroyed.
    ASSERT(m_hWnd == NULL);
    // Delete object because it won't be destroyed otherwise.
    if (m_self_delete) delete this;  
  }


  // Redraw if the colors change...
  afx_msg void OnSysColorChange() {
    CWnd::OnSysColorChange();
    if (m_oldstate_bitmap) {delete m_oldstate_bitmap; m_oldstate_bitmap=NULL;}
    if (!m_bgcolor_valid) m_bgcolor = ::GetSysColor(COLOR_BTNFACE);
    Invalidate();  
    UpdateWindow();
  }


  // Timer events...
  afx_msg void OnTimer(UINT nIDEvent)                                                                     
  {
    // Track if mouse floating over button...
    BOOL hit = HitTest();
    if (hit != m_button_hot) {
      m_button_hot = hit;
      Invalidate();
    }

    // Timer support for Vista transition effects...
    if (UseVistaEffects())
      if (m_transition_tickcount>0) {
        m_transition_tickcount--;
        Invalidate();
      }
      else if (m_stateid == PBS_DEFAULTED) {
        m_defaultbutton_tickcount++;
        if (m_defaultbutton_tickcount>40) m_defaultbutton_tickcount=0;
        if ((m_defaultbutton_tickcount>=0) && ((m_defaultbutton_tickcount%2)==0)) Invalidate();
      }

    CButton::OnTimer(nIDEvent);
  }


  // Stop Windows erasing the background...
  afx_msg BOOL OnEraseBkgnd(CDC* /*pDC*/)
    {return TRUE;}


  // Let Windows know we're a push button (so default button status works as expected)...
  afx_msg UINT OnGetDlgCode() {
    UINT result = DLGC_BUTTON;
    if (IsWindowEnabled()) result |= (m_button_default) ? DLGC_DEFPUSHBUTTON : DLGC_UNDEFPUSHBUTTON;
    return result;
  }


  // Hack to stop Windows reseting BS_OWNERDRAW style...
  afx_msg LRESULT OnSetStyle(WPARAM wParam, LPARAM lParam) {
    BOOL new_button_default = (wParam & 0xF) == BS_DEFPUSHBUTTON;
    if (new_button_default != m_button_default) Invalidate();
    m_button_default = new_button_default;
    return DefWindowProc(BM_SETSTYLE, (wParam & 0xFFFFFFF0) | BS_OWNERDRAW, lParam);
  }


  // Redraw when we loose focus...
  afx_msg void OnKillFocus(CWnd* pNewWnd) {
    Invalidate();
    CButton::OnKillFocus(pNewWnd);
  }


  // Toggle check state if BS_PUSHLIKE style...
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point) {
    CButton::OnLButtonDown(nFlags, point);
  }


  // Reroute double clicks...
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point) {
    SendMessage(WM_LBUTTONDOWN, (WPARAM)nFlags, (LPARAM)((point.y<<16)|point.x));
    CWnd::OnLButtonDblClk(nFlags, point);
  }


  // Send message clicks to override owner...
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point) {
    BOOL button_pressed = GetState()&4;
    if (button_pressed) m_check_state = (GetStyle() & BS_PUSHLIKE) ? !m_check_state : 0;
    if (m_owner)
      if ((m_owner != ::GetParent(GetSafeHwnd())) && button_pressed)
        if ((GetStyle() & BS_NOTIFY)>0)
          ::SendMessage(m_owner, WM_COMMAND,(WPARAM)((BN_DOUBLECLICKED<<16)|GetDlgCtrlID()),(LPARAM)GetSafeHwnd());
        else ::SendMessage(m_owner, WM_COMMAND,(WPARAM)((BN_CLICKED<<16)|GetDlgCtrlID()),(LPARAM)GetSafeHwnd());
    CWnd::OnLButtonUp(nFlags, point);
  }

  // Update button when ALT key clicked...
  virtual LRESULT OnUpdateUIState(WPARAM uistate, LPARAM lParam) {
    Invalidate();
    return DefWindowProc(WM_UPDATEUISTATE,uistate,lParam);
  }

  // Checkbox style button states...
  afx_msg LRESULT OnSetCheck(WPARAM wParam, LPARAM /*lParam*/) {
    m_check_state = wParam;
    Invalidate();
    return 0;
  }

  afx_msg LRESULT OnGetCheck(WPARAM /*wParam*/, LPARAM /*lParam*/) {
    return m_check_state;
  }

  DECLARE_MESSAGE_MAP()
};

#endif // define _BUTTONVE_H_
#endif