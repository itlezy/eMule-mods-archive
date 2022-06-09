// EmMt.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARLMT_H__6886ED1D_C47B_48B7_9467_47E10EF205E7__INCLUDED_)
#define AFX_ARLMT_H__6886ED1D_C47B_48B7_9467_47E10EF205E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EmWinNT.h"

#ifndef CCriticalSection_INL_implemented
#define CCriticalSection_INL_implemented
class CCriticalSection_INL : public CRITICAL_SECTION {

	DWORD m_dwSafeValue;
	static const DWORD s_dwSafeValue;
public:
	inline CCriticalSection_INL() { InitializeCriticalSection(this); m_dwSafeValue = s_dwSafeValue; }
	inline ~CCriticalSection_INL() { m_dwSafeValue = 0; DeleteCriticalSection(this); }
	inline void Enter() { if (m_dwSafeValue == s_dwSafeValue) EnterCriticalSection(this); }
	inline void Leave() { if (m_dwSafeValue == s_dwSafeValue) LeaveCriticalSection(this); }

	// The following function will just return FALSE unless we are running in WinNT
	inline BOOL TryEnter() { return
		(m_dwSafeValue == s_dwSafeValue) &&
		CEmWinNT::s_stWinNT.IsInitialized() &&
		CEmWinNT::s_stWinNT.m_pfnTryEnterCriticalSection(this);
	}

	class CScope {
		CCriticalSection_INL& m_csLock;
	public:
		inline CScope(CCriticalSection_INL& csLock) : m_csLock(csLock) { m_csLock.Enter(); }
		inline CScope(CScope& stOther) : m_csLock(stOther.m_csLock) { m_csLock.Enter(); }
		inline ~CScope()	{ m_csLock.Leave(); }
	};

	inline CScope zget_Scope() { CScope stScope(*this); return stScope; }
	__declspec (property(get=zget_Scope)) CScope _Scope;
};

#endif // CCriticalSection_INL_implemented

#endif // !defined(AFX_ARLMT_H__6886ED1D_C47B_48B7_9467_47E10EF205E7__INCLUDED_)
