// XMessageBox.h  Version 1.10
//
// This software is released into the public domain.  You are free to use it
// in any way you like, except that you may not sell this source code.
//
// This software is provided "as is" with no expressed or implied warranty.
// I accept no liability for any damage or loss of business that this software
// may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XMESSAGEBOX_H
#define XMESSAGEBOX_H

// MessageBox() Flags
/*
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L
*/

#ifndef MB_CANCELTRYCONTINUE
#define MB_CANCELTRYCONTINUE        0x00000006L	// adds three buttons, "Cancel", "Try Again", "Continue"
#endif

#define MB_CONTINUEABORT			0x00000007L // adds two buttons, "Continue", "Abort"
#define MB_SKIPSKIPALLCANCEL		0x00000008L // adds three buttons, "Skip", "Skip All", "Cancel"
#define MB_IGNOREIGNOREALLCANCEL	0x00000009L // adds three buttons, "Ignore", "Ignore All", "Cancel"

#define MB_DONOTASKAGAIN			0x01000000L // add checkbox "Do not ask me again"
#define MB_DONOTTELLAGAIN			0x02000000L // add checkbox "Do not tell me again"
#define MB_DONOTSHOWAGAIN			0x04000000L // add checkbox "Do not show again"
#define MB_YESTOALL					0x08000000L // must be used with either MB_YESNO or MB_YESNOCANCEL
#define MB_NOTOALL					0x10000000L // must be used with either MB_YESNO or MB_YESNOCANCEL
#define MB_NORESOURCE				0x20000000L	// do not try to load button strings from resources
#define MB_NOSOUND					0x40000000L // do not play sound when mb is displayed
#define MB_TIMEOUT			        0x80000000L	// returned if timeout expired

#define MB_DEFBUTTON5				0x00000400L
#define MB_DEFBUTTON6				0x00000500L

// Dialog Box Command IDs
/*
#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
#if(WINVER >= 0x0400)
#define IDCLOSE             8
#define IDHELP              9
#endif
*/

#ifndef IDTRYAGAIN
#define IDTRYAGAIN          10
#endif

#ifndef IDCONTINUE
#define IDCONTINUE          11
#endif

#define IDSKIP				14
#define IDSKIPALL			15
#define IDIGNOREALL			16

#define IDYESTOALL			19
#define IDNOTOALL			20

// following 4 ids MUST be sequential
#define IDCUSTOM1			23
#define IDCUSTOM2			24
#define IDCUSTOM3			25
#define IDCUSTOM4			26

#define ID_XMESSAGEBOX_LAST_ID	26
///////////////////////////////////////////////////////////////////////////////
//
// If you want to save the state of "Do Not Ask/Tell" checkbox to profile (ini)
// file instead of registry, uncomment the following line:
//
//#define XMESSAGEBOX_USE_PROFILE_FILE


///////////////////////////////////////////////////////////////////////////////
//
// If you do not want automatic saving of "Do Not Ask/Tell" checkbox, 
// uncomment the following line:
//
#define XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX


///////////////////////////////////////////////////////////////////////////////
//
// If you have chosen to automatically save "Do Not Ask/Tell" checkbox to ini:
//
// Normally the lpszModule and nLine data are encoded, since it might not be
// desirable to allow users to be able to see the path and module name of
// your source file.  If you do not want encoding of "Do Not Ask/Tell" module
// info in the registry (or ini file), uncomment the following line:
//
//#define XMESSAGEBOX_DO_NOT_ENCODE


///////////////////////////////////////////////////////////////////////////////
//
// This identifier specifies the format of the text displayed for the timeout 
// key, which by default is "%s = %d".  You may change this to anything you 
// wish, as long as 1) there is both a %s and a %d;  and 2) the %s precedes 
// the %d.
//
#define XMESSAGEBOX_TIMEOUT_TEXT_FORMAT	_T("%s [%d]")
//#define XMESSAGEBOX_TIMEOUT_TEXT_FORMAT	_T("%s = %d")


///////////////////////////////////////////////////////////////////////////////
//
// This identifier specifies the name of the ini file, which by default 
// is "XMessageBox.ini".
//
//#define XMESSAGEBOX_INI_FILE			_T("XMessageBox.ini")


///////////////////////////////////////////////////////////////////////////////
//
// This identifier specifies the registry key used to store checkbox values.  
// By default it is "XMessageBox".
//
//#define XMESSAGEBOX_REGISTRY_KEY		_T("XMessageBox")

///////////////////////////////////////////////////////////////////////////////
//
// If you do not want to display the disabled timer countdown in the caption,
// uncomment the following line:
//
//#define XMESSAGEBOX_NO_DISABLED_COUNTDOWN

///////////////////////////////////////////////////////////////////////////////
//+++1.8
// If you do not want to use automatic Vista detection and style, comment out 
// the following line:
//
#define XMESSAGEBOX_AUTO_VISTA_STYLE


///////////////////////////////////////////////////////////////////////////////
// 
// Report Callback Function
// 
// Purpose:     Optional function that is called when user clicks on Report
//              button.
// 
// Parameters:  lpszMessageText - message text from message box
//              dwUserData      - user-supplied data
// 
// Returns:     None
//
typedef void (* XMESSAGEBOX_REPORT_FUNCTION)(LPCTSTR lpszMessageText, DWORD dwUserData); 


struct XMSGBOXPARAMS 
{
	XMSGBOXPARAMS()
	{
		nTimeoutSeconds               = 0;
		//nDisabledSeconds              = 0;
		hInstanceIcon                 = NULL;
		//hInstanceStrings              = NULL;
		//lpReportFunc                  = NULL;
		//dwReportUserData              = 0;
		//nIdHelp                       = 0;
		nIdIcon                       = 0;
		//nIdCustomButtons              = 0;
		//nIdReportButtonCaption        = 0;
		x                             = 0;
		y                             = 0;
		dwOptions                     = 0;
#ifndef XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX
		lpszModule                    = NULL;
		nLine                         = 0;
#endif
		//bUseUserDefinedButtonCaptions = FALSE;			//+++1.5
		crText                        = CLR_INVALID;	//+++1.8
		crBackground                  = CLR_INVALID;	//+++1.8

		//memset(szIcon, 0, sizeof(szIcon));
		//memset(szCustomButtons, 0, sizeof(szCustomButtons));
		//memset(szReportButtonCaption, 0, sizeof(szReportButtonCaption));
#ifndef XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX
		memset(szCompanyName, 0, sizeof(szCompanyName));
#endif
		//memset(&UserDefinedButtonCaptions, 0, sizeof(UserDefinedButtonCaptions));	//+++1.5
	}

	//UINT		nIdHelp;						// help context ID for message;
												// 0 indicates the application’s 
												// default Help context will 
												// be used
	int			nTimeoutSeconds;				// number of seconds before the
												// default button will be selected
	//int			nDisabledSeconds;				// number of seconds that all the 
												// buttons will be disabled - after
												// nDisabledSeconds, all buttons
												// will be enabled
	int			x, y;							// initial x,y screen coordinates
	enum										// these are bit flags for dwOptions
	{
		None				= 0x0000,
		RightJustifyButtons	= 0x0001,			// causes buttons to be right-justified
		VistaStyle			= 0x0002,			// setting this option bit will cause the 
												// message background to be painted with 
												// the current window color (typically 
												// white), and the buttons to be 
												// right-justified.    +++1.8
		Narrow				= 0x0004			// uses a narrow width for message box -
												// SM_CXSCREEN / 3

	};
	DWORD		dwOptions;						// options flags
	//HINSTANCE	hInstanceStrings;				// if specified, will be used to
												// load strings
	HINSTANCE	hInstanceIcon;					// if specified, will be used to
												// load custom icon
	UINT		nIdIcon;						// custom icon resource id
	//TCHAR		szIcon[MAX_PATH];				// custom icon name
	//UINT		nIdCustomButtons;				// custom buttons resource id
	//TCHAR		szCustomButtons[MAX_PATH];		// custom buttons string
	//UINT		nIdReportButtonCaption;			// report button resource id
	//TCHAR		szReportButtonCaption[MAX_PATH];// report button string
#ifndef XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX
	TCHAR		szCompanyName[MAX_PATH];		// used when saving checkbox state in registry
	LPCTSTR		lpszModule;						// module name (for saving DoNotAsk state)
	int			nLine;							// line number (for saving DoNotAsk state)
#endif
	//DWORD		dwReportUserData;				// data sent to report callback function
	//XMESSAGEBOX_REPORT_FUNCTION lpReportFunc;	// report function
	COLORREF	crText;							// message text color		+++1.8
	COLORREF	crBackground;					// message background color	+++1.8
/*
	//-[UK
	// For not loading from resource but passing directly,
	// Use the following code.
	struct CUserDefinedButtonCaptions
	{
		TCHAR	szAbort				[MAX_PATH];
		TCHAR	szCancel			[MAX_PATH];
		TCHAR	szContinue			[MAX_PATH];
		TCHAR	szDoNotAskAgain		[MAX_PATH];
		TCHAR	szDoNotTellAgain	[MAX_PATH];
		TCHAR	szDoNotShowAgain	[MAX_PATH];
		TCHAR	szHelp				[MAX_PATH];
		TCHAR	szIgnore			[MAX_PATH];
		TCHAR	szIgnoreAll			[MAX_PATH];
		TCHAR	szNo				[MAX_PATH];
		TCHAR	szNoToAll			[MAX_PATH];
		TCHAR	szOK				[MAX_PATH];
		TCHAR	szReport			[MAX_PATH];
		TCHAR	szRetry				[MAX_PATH];
		TCHAR	szSkip				[MAX_PATH];
		TCHAR	szSkipAll			[MAX_PATH];
		TCHAR	szTryAgain			[MAX_PATH];
		TCHAR	szYes				[MAX_PATH];
		TCHAR	szYesToAll			[MAX_PATH];
	};
	BOOL						bUseUserDefinedButtonCaptions;	//+++1.5
	CUserDefinedButtonCaptions	UserDefinedButtonCaptions;		//+++1.5
	//-]UK
	*/
};

int XMessageBox(HWND hwnd, 
				LPCTSTR lpszMessage,
				LPCTSTR lpszCaption = NULL, 
				UINT nStyle = MB_OK | MB_ICONEXCLAMATION,
				XMSGBOXPARAMS * pXMB = NULL);


#ifndef XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX
DWORD XMessageBoxGetCheckBox(LPCTSTR lpszCompanyName, LPCTSTR lpszModule, int nLine);

DWORD XMessageBoxGetCheckBox(XMSGBOXPARAMS& xmb);
#endif

#endif //XMESSAGEBOX_H
