/****************************************************************************
 *	class		:	CWndUtil
 *	author		:	Peter Mares / kinkycode.com (gui@kinkycode.com)
 *	base class	:	none
 *	notes		:	Helper class for custom CWnd registration (Should derived from here)
 *
 *	Blurb		:	Its free, it feels good and its from South Africa :)
 ****************************************************************************
 *	Version History:
 *
 *	v0.1 (2003-05-10)
 *
 *	- First public release
 *
 ****************************************************************************/

#if !defined(AFX_WNDUTIL_H__979E2102_DC7C_461D_AA76_40F06C0FBF87__INCLUDED_)
#define AFX_WNDUTIL_H__979E2102_DC7C_461D_AA76_40F06C0FBF87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CWndUtil  
{
protected:
	CWndUtil(LPCTSTR lpszClassName)
		: m_strClassName(lpszClassName)
	{
	}

	virtual ~CWndUtil()
	{
	}

public:

	virtual BOOL			RegisterWndClass()
	{
		WNDCLASS			wnd;
		HINSTANCE			hInst = AfxGetInstanceHandle();

		if ( !(::GetClassInfo( hInst, m_strClassName, &wnd )) )
		{
			wnd.style				= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
			wnd.lpfnWndProc			= ::DefWindowProc;
			wnd.cbClsExtra			= wnd.cbWndExtra = 0;
			wnd.hInstance			= hInst;
			wnd.hIcon				= NULL;
			wnd.hCursor				= AfxGetApp()->LoadStandardCursor(IDC_ARROW);
			wnd.hbrBackground		= NULL;
			wnd.lpszMenuName		= NULL;
			wnd.lpszClassName		= m_strClassName;

			if ( !AfxRegisterClass(&wnd) )
			{
				AfxThrowResourceException();
				return FALSE;
			}
		}

		return TRUE;
	}

protected:
	// class registration/information
	CString					m_strClassName;
};

#endif // !defined(AFX_WNDUTIL_H__979E2102_DC7C_461D_AA76_40F06C0FBF87__INCLUDED_)
