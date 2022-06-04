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
private:
	CAtlArray<CString> m_asList;
	CComPtr<IAutoComplete> m_pac;

	size_t m_nCurrentElement;
	size_t m_iMaxItemCount;
	ULONG m_nRefCount;
	BOOL m_fBound;

	// Constructors/destructors
public:
	CCustomAutoComplete();
	CCustomAutoComplete(const CAtlArray<CString>& p_sItemList);
	~CCustomAutoComplete();

	// Implementation
	BOOL Bind(HWND p_hWndEdit, DWORD p_dwOptions = 0, LPCTSTR p_lpszFormatString = NULL);
	VOID Unbind();
	BOOL IsBound() const { return m_fBound; }

	BOOL SetList(const CAtlArray<CString>& p_sItemList);
	const CAtlArray<CString>& GetList() const;
	size_t GetItemCount();

	BOOL AddItem(const CString& p_sItem, size_t iPos);
	BOOL RemoveItem(const CString& p_sItem);
	BOOL RemoveSelectedItem();
	CString GetItem(size_t pos);
	
	BOOL Clear();
	BOOL Disable();
	BOOL Enable(VOID);
	
	BOOL LoadList(LPCTSTR pszFileName);
	BOOL SaveList(LPCTSTR pszFileName);

	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);

	STDMETHOD(Next)(ULONG celt, LPOLESTR* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(IEnumString** ppenum);

	// Internal implementation
private:
	void InternalInit();
	HRESULT EnDisable(BOOL p_fEnable);
	size_t FindItem(const CString& rstr);
};
