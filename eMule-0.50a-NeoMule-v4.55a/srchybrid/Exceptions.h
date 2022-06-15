#pragma once

#ifdef _DEBUG
#define	CATCH_DFLT_ALL(fname)
#else
#define	CATCH_DFLT_ALL(fname) \
	catch(...){ \
		if (thePrefs.GetVerbose()) \
			DebugLogError(LOG_STATUSBAR, _T("Unknown exception in ") fname); \
		ASSERT(0); \
	}
#endif

// This type of "last chance" exception handling is to be used at least in several callback functions to avoid memory leaks.
// It is *not* thought as a proper handling of exceptions in general! 
// -> Use explicit exception handlers where needed!

#define CATCH_MFC_EXCEPTION(fname) \
	catch(CException* e){ \
		TCHAR szError[1024]; \
		e->GetErrorMessage(szError, _countof(szError)); \
		const CRuntimeClass* pRuntimeClass = e->GetRuntimeClass(); \
		LPCSTR pszClassName = (pRuntimeClass) ? pRuntimeClass->m_lpszClassName : NULL; \
		if (!pszClassName) \
			pszClassName = "CException"; \
		if (thePrefs.GetVerbose()) \
			DebugLogError(LOG_STATUSBAR, _T("Unknown %hs exception in ") fname _T(" - %s"), pszClassName, szError); \
		e->Delete(); \
	}

#define	CATCH_STR_EXCEPTION(fname) \
	catch(CString strError){ \
		if (thePrefs.GetVerbose()) \
			DebugLogError(LOG_STATUSBAR, _T("Unknown CString exception in ") fname _T(" - %s"), strError); \
	}

//#define CATCH_DFLT_EXCEPTIONS(fname) \
//	CATCH_MFC_EXCEPTION(fname) \
//	CATCH_STR_EXCEPTION(fname)

// NEO: ND - [NeoDebug] -- Xanatos -->
// This code is intended for use in release builds only, it catch all kinds of exceptions 
// And it resetz possible FPU error states
#define CATCH_DFLT_EXCEPTIONS(fname) \
	CATCH_MFC_EXCEPTION(fname) \
	CATCH_STR_EXCEPTION(fname) \
	catch(...){ \
		int fperr= _status87(); \
		_clear87();  \
		if (thePrefs.GetVerbose()){ \
			if(fperr == 0) \
				DebugLogError(LOG_STATUSBAR, _T("Unknown exception in ") fname); \
			else \
				DebugLogError(LOG_STATUSBAR, _T("FPU exception in ") fname _T(" - %i"), fperr); \
		} \
		ASSERT(0); \
	}
// NEO: ND END<-- Xanatos --

class CMsgBoxException : public CException
{
	DECLARE_DYNAMIC(CMsgBoxException)
public:
	explicit CMsgBoxException(LPCTSTR pszMsg, UINT uType = MB_ICONWARNING, UINT uHelpID = 0)
	{
		m_strMsg = pszMsg;
		m_uType = uType;
		m_uHelpID = uHelpID;
	}

	CString m_strMsg;
	UINT m_uType;
	UINT m_uHelpID;
};

class CClientException : public CException
{
	DECLARE_DYNAMIC(CClientException)
public:
	CClientException(LPCTSTR pszMsg, bool bDelete)
	{
		m_strMsg = pszMsg;
		m_bDelete = bDelete;
	}

	CString m_strMsg;
	bool m_bDelete;
};
