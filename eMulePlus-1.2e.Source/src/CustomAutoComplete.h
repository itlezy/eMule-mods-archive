//--------------------------------------------------------------------------------------------
//  Author:         Klaus H. Probst [kprobst@vbbox.com]
// 
//--------------------------------------------------------------------------------------------

#pragma once

#include <initguid.h>
#include <shldisp.h>
#include <shlguid.h>

class CCustomAutoComplete : 
	public IEnumString
{
public:
	CCustomAutoComplete();
	CCustomAutoComplete(const CStringArray &astrItemList);
	~CCustomAutoComplete();

	BOOL	Bind(HWND hWndEdit, DWORD dwOptions = 0, LPCTSTR pcFormatString = NULL);
	VOID	Unbind();
	BOOL	IsBound() const { return m_bBound; }

	BOOL	SetList(const CStringArray &astrItemList);
	int		GetItemCount();
	const CStringArray&	GetList() const;

	BOOL	AddItem(const CString &strItem, int iPos);
	BOOL	RemoveItem(const CString &strItem);
	CString	GetItem(int iPos);

	BOOL	Clear();
	BOOL	Disable();
	BOOL	Enable(VOID);
	
	BOOL	LoadList(LPCTSTR pcFileName);
	BOOL	SaveList(LPCTSTR pcFileName);

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);

	STDMETHOD(Next)(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(IEnumString **ppenum);

private:
	void	InternalInit();
	HRESULT	EnDisable(BOOL bEnable);
	int		FindItem(const CString &strItem);

	CStringArray	m_asList;
	CComPtr<IAutoComplete>	m_pac;

	ULONG	m_uiCurrentElement;
	ULONG	m_uiRefCount;
	int		m_iMaxItemCount;
	BOOL	m_bBound;
};
