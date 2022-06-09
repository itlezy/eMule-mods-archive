//--------------------------------------------------------------------------------------------
//  Name:           CCustomAutoComplete (CCUSTOMAUTOCOMPLETE.H)
//  Type:           Wrapper class
//  Description:    Matches IAutoComplete, IEnumString and the registry (optional) to provide
//					custom auto-complete functionality for EDIT controls - including those in
//					combo boxes - in WTL projects.
//
//  Author:         Klaus H. Probst [kprobst@vbbox.com]
//  URL:            http://www.vbbox.com/
//  Copyright:      This work is copyright © 2002, Klaus H. Probst
//  Usage:          You may use this code as you see fit, provided that you assume all
//                  responsibilities for doing so.
//  Distribution:   Distribute freely as long as you maintain this notice as part of the
//					file header.
//
//
//  Updates:        09-Mai-2003 [bluecow]:
//						- changed original string list code to deal with a LRU list
//						  and auto cleanup of list entries according 'iMaxItemCount'.
//						- splitted original code into cpp/h file
//						- removed registry stuff
//						- added file stuff
//					15-Jan-2004 [Ornis]:
//						- changed adding strings to replace existing ones on a new position
//
//
//  Notes:
//
//
//  Dependencies:
//
//					The usual ATL/WTL headers for a normal EXE, plus <atlmisc.h>
//
//--------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "CustomAutoComplete.h"
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include "otherfunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CCustomAutoComplete::CCustomAutoComplete()
{
	InternalInit();
}

CCustomAutoComplete::CCustomAutoComplete(const CStringArray &astrItemList)
{
	InternalInit();
	SetList(astrItemList);
}

CCustomAutoComplete::~CCustomAutoComplete()
{
	if (m_pac != NULL)
		m_pac.Release();
}

BOOL CCustomAutoComplete::Bind(HWND hWndEdit, DWORD dwOptions, LPCTSTR pcFormatString)
{
	ATLASSERT(::IsWindow(hWndEdit));

	if (m_bBound || m_pac != NULL)
		return FALSE;

	if (SUCCEEDED(m_pac.CoCreateInstance(CLSID_AutoComplete)))
	{
		if (dwOptions != 0)
		{
			CComQIPtr<IAutoComplete2> pAC2(m_pac);

			if (pAC2 != NULL)
			{
				pAC2->SetOptions(dwOptions);
				pAC2.Release();
			}
		}

		USES_CONVERSION;
		if (SUCCEEDED(m_pac->Init(hWndEdit, this, NULL, CT2CW(pcFormatString))))
		{
			m_bBound = TRUE;

			return TRUE;
		}
	}

	return FALSE;
}

VOID CCustomAutoComplete::Unbind()
{
	if (!m_bBound)
		return;

	if (m_pac != NULL)
	{
		m_pac.Release();
		m_bBound = FALSE;
	}
}

BOOL CCustomAutoComplete::SetList(const CStringArray &astrItemList)
{
	ATLASSERT(astrItemList.GetSize() != 0);

	Clear();
	m_asList.Append(astrItemList);

	return TRUE;
}

int CCustomAutoComplete::FindItem(const CString &strItem)
{
	for (int i = 0; i < m_asList.GetCount(); i++)
		if (m_asList[i].Compare(strItem) == 0)
			return i;
	return -1;
}

BOOL CCustomAutoComplete::AddItem(const CString &strItem, int iPos)
{
	if (!strItem.IsEmpty())
	{
		int	iOldPos = FindItem(strItem);

		if (iOldPos == -1)
		{
		//	Use a LRU list
			if (iPos == -1)
				m_asList.Add(strItem);
			else
				m_asList.InsertAt(iPos, strItem);

			while (m_asList.GetSize() > m_iMaxItemCount)
				m_asList.RemoveAt(m_asList.GetSize() - 1);

			return TRUE;
		}
		else if (iPos != -1)
		{
			m_asList.RemoveAt(iOldPos);
			if (iOldPos < iPos)
				--iPos;
			m_asList.InsertAt(iPos, strItem);

			while (m_asList.GetSize() > m_iMaxItemCount)
				m_asList.RemoveAt(m_asList.GetSize() - 1);

			return TRUE;
		}
	}
	return FALSE;
}

int CCustomAutoComplete::GetItemCount()
{
	return static_cast<int>(m_asList.GetCount());
}

BOOL CCustomAutoComplete::RemoveItem(const CString &strItem)
{
	if (!strItem.IsEmpty())
	{
		int	iPos = FindItem(strItem);

		if (iPos != -1)
		{
			m_asList.RemoveAt(iPos);
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CCustomAutoComplete::Clear()
{
	if (!m_asList.IsEmpty())
	{
		m_asList.RemoveAll();

		return TRUE;
	}

	return FALSE;
}

BOOL CCustomAutoComplete::Disable()
{
	if (m_pac == NULL || !m_bBound)
		return FALSE;

	return SUCCEEDED(EnDisable(FALSE));
}

BOOL CCustomAutoComplete::Enable(VOID)
{
	if (m_pac == NULL || m_bBound)
		return FALSE;

	return SUCCEEDED(EnDisable(TRUE));
}

const CStringArray& CCustomAutoComplete::GetList() const
{
	return m_asList;
}

STDMETHODIMP_(ULONG) CCustomAutoComplete::AddRef()
{
	ULONG	uiCount = ::InterlockedIncrement(reinterpret_cast<LONG*>(&m_uiRefCount));

	return uiCount;
}

STDMETHODIMP_(ULONG) CCustomAutoComplete::Release()
{
	ULONG	uiCount = static_cast<ULONG>(::InterlockedDecrement(reinterpret_cast<LONG*>(&m_uiRefCount)));

	if (uiCount == 0)
		delete this;

	return uiCount;
}

STDMETHODIMP CCustomAutoComplete::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT	hr = E_NOINTERFACE;

	if (ppvObject != NULL)
	{
		*ppvObject = NULL;

		if (IID_IUnknown == riid)
			*ppvObject = static_cast<IUnknown*>(this);
		else if (IID_IEnumString == riid)
			*ppvObject = static_cast<IEnumString*>(this);
		if (*ppvObject != NULL)
		{
			hr = S_OK;
			static_cast<LPUNKNOWN>(*ppvObject)->AddRef();
		}
	}
	else
		hr = E_POINTER;

	return hr;
}

STDMETHODIMP CCustomAutoComplete::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
	USES_CONVERSION;

	HRESULT	hr = S_FALSE;
	ULONG	ui;

	if (celt == 0)
		celt = 1;

	for (ui = 0; ui < celt; ui++)
	{
		if (m_uiCurrentElement == static_cast<ULONG>(m_asList.GetSize()))
			break;

		rgelt[ui] = (LPWSTR)::CoTaskMemAlloc((ULONG) sizeof(WCHAR) * (m_asList[m_uiCurrentElement].GetLength() + 1));
		wcscpy(rgelt[ui], T2CW(m_asList[m_uiCurrentElement]));

		if (pceltFetched != NULL)
			*pceltFetched++;

		m_uiCurrentElement++;
	}

	if (ui == celt)
		hr = S_OK;

	return hr;
}

STDMETHODIMP CCustomAutoComplete::Skip(ULONG celt)
{
	m_uiCurrentElement += celt;

	if (m_uiCurrentElement > static_cast<ULONG>(m_asList.GetSize()))
		m_uiCurrentElement = 0;

	return S_OK;
}

STDMETHODIMP CCustomAutoComplete::Reset(void)
{
	m_uiCurrentElement = 0;

	return S_OK;
}

STDMETHODIMP CCustomAutoComplete::Clone(IEnumString **ppenum)
{
	if (ppenum == NULL)
		return E_POINTER;

	CCustomAutoComplete	*pnew = new CCustomAutoComplete();

	pnew->AddRef();
	*ppenum = pnew;

	return S_OK;
}

void CCustomAutoComplete::InternalInit()
{
	m_uiCurrentElement = 0;
	m_uiRefCount = 0;
	m_bBound = FALSE;
	m_iMaxItemCount = 30;
}

HRESULT CCustomAutoComplete::EnDisable(BOOL bEnable)
{
	ATLASSERT(m_pac);

	HRESULT	hr = m_pac->Enable(bEnable);

	if (SUCCEEDED(hr))
		m_bBound = bEnable;

	return hr;
}

BOOL CCustomAutoComplete::LoadList(LPCTSTR pcFileName)
{
#ifdef _UNICODE
	FILE	*fp = _tfsopen(pcFileName, _T("rb"), _SH_DENYWR);
#else
	FILE	*fp = _tfsopen(pcFileName, _T("r"), _SH_DENYWR);
#endif

	if (fp == NULL)
		return FALSE;

#ifdef _UNICODE
//	Verify Unicode byte-order mark 0xFEFF
	WORD	wBOM = fgetwc(fp);

//	If not UNICODE file, set reading to ASCII mode
	if (wBOM != 0xFEFF)
	{
		_setmode(_fileno(fp), _O_TEXT);
		fseek(fp, 0, SEEK_SET);
	}

#endif
	CString strItem;

	while (_fgetts(strItem.GetBufferSetLength(256), 256, fp) != NULL)
	{
		strItem.ReleaseBuffer();
		strItem.Trim(_T(" \r\n"));
		AddItem(strItem, -1);
	}

	fclose(fp);

	return TRUE;
}

BOOL CCustomAutoComplete::SaveList(LPCTSTR pcFileName)
{
#ifdef _UNICODE
	FILE	*fp = _tfsopen(pcFileName, _T("wb"), _SH_DENYWR);
#else
	FILE	*fp = _tfsopen(pcFileName, _T("w"), _SH_DENYWR);
#endif

	if (fp == NULL)
		return FALSE;

#ifdef _UNICODE
//	Write Unicode byte-order mark 0xFEFF
	fputwc(0xFEFF, fp);

#endif
	for (int i = 0; i < m_asList.GetCount(); i++)
#ifdef _UNICODE
		_ftprintf(fp, _T("%s\r\n"), m_asList[i]);
#else
		_ftprintf(fp, _T("%s\n"), m_asList[i]);
#endif
	fclose(fp);

	return (ferror(fp) == 0);
}

CString CCustomAutoComplete::GetItem(int iPos)
{
	return (iPos < m_asList.GetCount()) ? m_asList.GetAt(iPos) : NULL;
}