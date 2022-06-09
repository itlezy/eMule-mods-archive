//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

//	NOTE: MSLU & RichEditCtrl
//	If you have a RichEdit control supporting Unicode (using the RichEd20W class name)
//	and you attempt to use the GetWindowTextW API or the WM_GETTEXT message to retrieve
//	the text it contains, the text will be garbled. A fix is not currently under consideration
//	because the EM_GETTEXTEX message is available and will work properly, and it will not convert
//	the text via the default system codepage the way that MSLU would for WM_GETTEXT or GetWindowTextW.
//	(see http://www.trigeminal.com/usenet/usenet035.asp?01000000)

#include "stdafx.h"
#include "eMule.h"
#include "HTRichEditCtrl.h"
#include "TitleMenu.h"
#include "otherfunctions.h"
#include <share.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

LinkDetect_Table _apszSchemes[LINKDETECT_TABLESZ] =
{
	{ _T("ed2k://"), CSTRLEN(_T("ed2k://")) },
	{ _T("ftp."), CSTRLEN(_T("ftp.")) },
	{ _T("ftp://"), CSTRLEN(_T("ftp://")) },
	{ _T("http://"), CSTRLEN(_T("http://")) },
	{ _T("https://"), CSTRLEN(_T("https://")) },
	{ _T("mailto:"), CSTRLEN(_T("mailto:")) },
	{ _T("www."), CSTRLEN(_T("www.")) },
	{ _T("www1."), CSTRLEN(_T("www1.")) },
	{ _T("www2."), CSTRLEN(_T("www2.")) },
	{ _T("www3."), CSTRLEN(_T("www3.")) },
	{ _T("www4."), CSTRLEN(_T("www4.")) },
	{ _T("www5."), CSTRLEN(_T("www5.")) }
};

IMPLEMENT_DYNAMIC(CHTRichEditCtrl, CRichEditCtrl)

BEGIN_MESSAGE_MAP(CHTRichEditCtrl, CRichEditCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT_EX(EN_LINK, OnEnLink)
	ON_WM_CREATE()
END_MESSAGE_MAP()

CHTRichEditCtrl::CHTRichEditCtrl() : m_strTitle(_T("")), m_strURL(_T(""))
{
	m_dwFlags = HTC_ISAUTOSCROLL | HTC_ISDEFAULTLINKS;
	m_crDefaultForeground = GetSysColor(COLOR_WINDOWTEXT);
	m_crDefaultBackground = GetSysColor(COLOR_WINDOW);

	if (g_App.m_pPrefs->GetWindowsVersion() == _WINVER_95_)
		m_lMaxBufSize = 64 * 1024 - 1;
	else
		m_lMaxBufSize = 128 * 1024;
	m_hArrowCursor = ::LoadCursor(NULL, IDC_ARROW);
}

CHTRichEditCtrl::~CHTRichEditCtrl()
{
	//	Shared cursors mustn't be destroyed
}

void CHTRichEditCtrl::SetTitle(LPCTSTR pszTitle)
{
	m_strTitle = g_App.StripInvalidFilenameChars(pszTitle);
}

int CHTRichEditCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CRichEditCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	VERIFY( SendMessage(EM_SETUNDOLIMIT, 0, 0) == 0 );

#ifdef _UNICODE
//	Win9x: Explicitly set to Unicode to receive Unicode notifications.
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);
#endif

	LimitText(m_lMaxBufSize);
	SetTargetDevice(NULL, (m_dwFlags & HTC_ISWORDWRAP) == 0);	// 1: off, 0: on word wrap
	SetEventMask(GetEventMask() | ENM_LINK);

	return 0;
}

void CHTRichEditCtrl::AppendText(const CString &strMsg, COLORREF crTextColor/*=CLR_DEFAULT*/, COLORREF crBackColor/*=CLR_DEFAULT*/, DWORD dwFlags/*=0*/)
{
	AppendText(strMsg.GetString(), strMsg.GetLength(), crTextColor, crBackColor, dwFlags);
}

void CHTRichEditCtrl::AppendText(LPCTSTR pszMsg, int iMsgLen, COLORREF crTextColor/*=CLR_DEFAULT*/, COLORREF crBackColor/*=CLR_DEFAULT*/, DWORD dwFlags/*=0*/)
{
//	When actual length is passed, the string has to be zero terminated right after it
	if (iMsgLen < 0)
		iMsgLen = _tcslen(pszMsg);

	if (iMsgLen == 0 || !::IsWindow(m_hWnd))
		return;

	long		lStartChar, lEndChar, lTextEnd = GetTextLengthEx(GTL_PRECISE | GTL_NUMCHARS);

	GetSel(lStartChar, lEndChar);

	BOOL			bIsVisible = IsWindowVisible();
	bool			bIsEndSel = (lStartChar == lEndChar && lStartChar == lTextEnd);
	CHARFORMAT2		cf2;
	POINT			ptOldPos;

	if (bIsVisible)
		SetRedraw(false);

	if (lTextEnd >= m_lMaxBufSize)
	{
		int iLineStart = LineIndex(LineFromChar(m_lMaxBufSize >> 3));

		SetSel(0, iLineStart);
		ReplaceSel(_T(""));
	}
	SendMessage(EM_GETSCROLLPOS, 0, (LPARAM)&ptOldPos);
	SetSel(-1, -1);
	memzero(&cf2, sizeof(cf2));
	cf2.cbSize = sizeof(cf2);
	GetSelectionCharFormat(cf2);
	cf2.crTextColor = (crTextColor == CLR_DEFAULT) ? m_crDefaultForeground : crTextColor;
	cf2.crBackColor = (crBackColor == CLR_DEFAULT) ? m_crDefaultBackground : crBackColor;
	cf2.dwEffects &= ~(CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR | CFE_BOLD | CFE_ITALIC | CFE_LINK | CFE_STRIKEOUT | CFE_UNDERLINE);
	cf2.dwEffects |= (dwFlags & HTC_BOLD) ? CFE_BOLD : 0;
	cf2.dwEffects |= (dwFlags & HTC_ITALIC) ? CFE_ITALIC : 0;
	cf2.dwEffects |= (dwFlags & HTC_LINK) ? CFE_LINK : 0;
	cf2.dwEffects |= (dwFlags & HTC_STRIKEOUT) ? CFE_STRIKEOUT : 0;
	cf2.dwEffects |= (dwFlags & HTC_UNDERLINE) ? CFE_UNDERLINE : 0;
	SetSelectionCharFormat(cf2);
	ReplaceSel(pszMsg);

	if ((dwFlags & (HTC_LINK | HTC_HAVENOLINK)) == 0 && (m_dwFlags & HTC_ISDEFAULTLINKS) != 0)
	{
		CString		strLine(pszMsg, iMsgLen);
		int			iIndex = 0, iScheme, iLen, iLinkEnd, iLnkIdx;
		const TCHAR	*pcLine, *pcTmp, *pcLink;

		lTextEnd = GetTextLengthEx(GTL_PRECISE | GTL_NUMCHARS, 1200/*Unicode CodePage*/) - iMsgLen;
		cf2.dwEffects |= CFE_LINK;

		//	Convert string to lower case only once
		strLine.MakeLower();
		pcLine = strLine.GetString();
		while (iMsgLen > iIndex)
		{
			pcLink = reinterpret_cast<const TCHAR*>(~0);
			for (iScheme = 0, iLnkIdx = -1; iScheme < ARRSIZE(_apszSchemes); iScheme++)
			{
				iLen = _apszSchemes[iScheme].uiLen;
				iLinkEnd = iIndex + iLen;

				if (iMsgLen <= iLinkEnd)
					continue;
				pcTmp = _tcsstr(pcLine + iIndex, _apszSchemes[iScheme].pszScheme);
			//	Choose a link closest to the beginning of the string
				if ((pcTmp != NULL) && (pcTmp < pcLink))
				{
					pcLink = pcTmp;
					iLnkIdx = iScheme;
				}
			}
			if (iLnkIdx < 0)	//no links were found
				break;
			iLen = _apszSchemes[iLnkIdx].uiLen;
			iIndex = pcLink - pcLine;
			iLinkEnd = iIndex + iLen;

		//	Find the first occurence of any following character
			pcTmp = _tcspbrk(pcLine + iLinkEnd, _T(" \n\\\r\t"));
			iLinkEnd = (pcTmp == NULL) ? iMsgLen : (pcTmp - pcLine);

			while ((iLinkEnd > iIndex + iLen) && 
				!((pcLine[iLinkEnd - 1] >= _T('/') && pcLine[iLinkEnd - 1] <= _T('9'))
				||(pcLine[iLinkEnd - 1] >= _T('a') && pcLine[iLinkEnd - 1] <= _T('z'))
				|| (pcLine[iLinkEnd - 1] == _T('|'))))
			{
				iLinkEnd--;
			}

			if (iLinkEnd > iIndex + iLen)
			{
				SetSel(lTextEnd + iIndex, lTextEnd + iLinkEnd);
				SetSelectionCharFormat(cf2);
				iIndex = iLinkEnd;
			}
			else
				iIndex += iLen;
		}
	}

	if (bIsEndSel)
		SetSel(-1, -1);
	else
		SetSel(lStartChar, lEndChar);

	if ((m_dwFlags & HTC_ISAUTOSCROLL) != 0 && (_tcschr(pszMsg, _T('\n')) != NULL))
	{
		ScrollToLastLine();

		POINT			ptNewPos;

		SendMessage(EM_GETSCROLLPOS, 0, (LPARAM)&ptNewPos);
		ptOldPos.y = ptNewPos.y;
	}

	if ((m_dwFlags & HTC_ISWORDWRAP) != 0)
		ptOldPos.x = 0;

	SendMessage(EM_SETSCROLLPOS, 0, (LPARAM)&ptOldPos);

	if (bIsVisible)
	{
		SetRedraw();
		Invalidate();
	}
}

void CHTRichEditCtrl::ScrollToLastLine()
{
	long		lFirstVisible = GetFirstVisibleLine();

	if (lFirstVisible > 0)
		LineScroll(-lFirstVisible, 0);

	// WM_VSCROLL does not work correctly under Win98 (or older versions of comctl.dll)
	SendMessage(WM_VSCROLL, SB_BOTTOM);
	SendMessage(WM_VSCROLL, SB_LINEUP);
	if (g_App.m_pPrefs->GetWindowsVersion() == _WINVER_95_)
	{
		// older versions of comctl.dll seem to need this to properly update the display
		SendMessage(WM_VSCROLL, MAKELONG(SB_THUMBPOSITION, GetScrollPos(SB_VERT)));
		SendMessage(WM_VSCROLL, SB_ENDSCROLL);
	}
}

void CHTRichEditCtrl::Reset()
{
	SetRedraw(false);
	SetWindowText(_T(""));
	SetRedraw();
	Invalidate();
}

void CHTRichEditCtrl::OnContextMenu(CWnd *pWnd, CPoint point)
{
	long			lSelStart, lSelEnd;
	CTitleMenu		menuLog;
	NOPRM(pWnd);

	GetSel(lSelStart, lSelEnd);
	m_dwFlags |= HTC_ISARROWCURSOR;
	menuLog.CreatePopupMenu();
	if ((m_dwFlags & HTC_ISLIMITED) == 0)
		menuLog.AddMenuTitle(GetResString(IDS_LOGENTRY));
	menuLog.AppendMenu(MF_STRING | ((lSelEnd > lSelStart) ? MF_ENABLED : MF_GRAYED), MP_COPYSELECTED, GetResString(IDS_COPY));

	UINT			dwFlags = MF_STRING | ((GetWindowTextLength() > 0) ? MF_ENABLED : MF_GRAYED);
	CString			strBuffer = GetResString(IDS_SAVETOFILE);

	if ((m_dwFlags & HTC_ISLIMITED) == 0)
	{
		menuLog.AppendMenu(dwFlags, MP_SAVELOG, strBuffer + _T(" (.log)"));
		menuLog.AppendMenu(dwFlags, MP_SAVERTF, strBuffer + _T(" (.rtf)"));
	}
	menuLog.AppendMenu(MF_SEPARATOR);
	menuLog.AppendMenu(dwFlags, MP_SELECTALL, GetResString(IDS_SELECTALL));

	if ((m_dwFlags & HTC_ISLIMITED) == 0)
	{
		menuLog.AppendMenu(dwFlags, MP_REMOVEALL, GetResString(IDS_PW_RESET));
		menuLog.AppendMenu(MF_SEPARATOR);
		menuLog.AppendMenu(MF_STRING | ((m_dwFlags & HTC_ISAUTOSCROLL) != 0 ? MF_CHECKED : MF_UNCHECKED), MP_AUTOSCROLL, GetResString(IDS_AUTOSCROLL));
		menuLog.AppendMenu(MF_STRING | ((m_dwFlags & HTC_ISWORDWRAP) != 0 ? MF_CHECKED : MF_UNCHECKED), MP_WORDWRAP, GetResString(IDS_WORDWRAP));
	}

	menuLog.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
	m_dwFlags &= ~HTC_ISARROWCURSOR;
}

BOOL CHTRichEditCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);

	switch (wParam)
	{
		case MP_COPYSELECTED:
			Copy();
			break;
		case MP_SAVELOG:
			SaveLogToDisk();
			break;
		case MP_SAVERTF:
			SaveRtfToDisk();
			break;
		case MP_SELECTALL:
			SetSel(0, -1);
			break;
		case MP_REMOVEALL:
			Reset();
			break;
		case MP_AUTOSCROLL:
			m_dwFlags ^= HTC_ISAUTOSCROLL;
			break;
		case MP_WORDWRAP:
			m_dwFlags ^= HTC_ISWORDWRAP;
			SetTargetDevice(NULL, (m_dwFlags & HTC_ISWORDWRAP) == 0);	// 1: off, 0: on word wrap
			break;
	}
	return true;
}

BOOL CHTRichEditCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ((m_dwFlags & HTC_ISARROWCURSOR) != 0)
	{
		::SetCursor(m_hArrowCursor);
		return true;
	}

	return CRichEditCtrl::OnSetCursor(pWnd, nHitTest, message);
}

void CHTRichEditCtrl::GetLastLogEntry(CString *pstrOut)
{
	int		iLastLine = GetLineCount() - 2;

	if (iLastLine >= 0)
	{
		pstrOut->ReleaseBufferSetLength(GetLine(iLastLine, pstrOut->GetBuffer(1024), 1024));
		pstrOut->TrimRight(_T("\r\n"));
	}
	else
		pstrOut->Truncate(0);
}

void CHTRichEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		if (nChar == _T('A'))
			//////////////////////////////////////////////////////////////////
			// Ctrl+A: Select all items
			SetSel(0, -1);
		else if (nChar == _T('C'))
			//////////////////////////////////////////////////////////////////
			// Ctrl+C: Copy listview items to clipboard
			Copy();
	}
	else if (nChar == VK_ESCAPE)
		return;	// don't minimize CHTRichEditCtrl

	CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CHTRichEditCtrl::OnEnLink(NMHDR *pNMHDR, LRESULT *pResult)
{
	bool	bHandled = false;
	ENLINK	*pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);

	if ((pEnLink != NULL) && (pEnLink->msg == WM_LBUTTONDOWN))
	{
		CString		strURL;

		GetTextRange(pEnLink->chrg.cpMin, pEnLink->chrg.cpMax, strURL);

		for (int iScheme = 0; iScheme < ARRSIZE(_apszSchemes); iScheme++)
		{
			if (_tcsnicmp(strURL.GetString(), _apszSchemes[iScheme].pszScheme, _apszSchemes[iScheme].uiLen) == 0)
			{
				ShellExecute(NULL, NULL, strURL, NULL, NULL, SW_SHOWDEFAULT);
				bHandled = true;
				break;
			}
		}
	}

	*pResult = bHandled;
	return bHandled;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHTRichEditCtrl::SaveLogToDisk()
{
	static const TCHAR *s_apcExtension = _T(".log");
	CString			strLog, strTemp;

	if (DialogBrowseFile(strTemp, _T("Log (*.log)|*.log||"), m_strTitle + s_apcExtension, 0, false, g_App.m_pPrefs->GetAppDir()))
	{
		if (strTemp.Right(4) != s_apcExtension)
			strTemp.Append(s_apcExtension);

		FILE	*pLogFile = _tfsopen(strTemp, _T("wb"), _SH_DENYWR);

		if (pLogFile != NULL)
		{
			EDITSTREAM		es;

			es.dwCookie = (DWORD)&strLog;
#ifdef _UNICODE
			es.pfnCallback = MEditStreamOutCallbackW;
			StreamOut(SF_UNICODE | SF_TEXT, es);
			fputwc(0xFEFF, pLogFile);
#else
			es.pfnCallback = MEditStreamOutCallbackA;
			StreamOut(SF_TEXT, es);
#endif
			fwrite(strLog, sizeof(TCHAR), strLog.GetLength(), pLogFile);
			fclose(pLogFile);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHTRichEditCtrl::SaveRtfToDisk()
{
	static const TCHAR *s_apcExtension = _T(".rtf");
	CString			strTemp;

	if (DialogBrowseFile(strTemp, _T("Rich Text Format (RTF)|*.rtf||"), m_strTitle + s_apcExtension, 0, false, g_App.m_pPrefs->GetAppDir()))
	{
		if (strTemp.Right(4) != s_apcExtension)
			strTemp.Append(s_apcExtension);

		FILE	*pRtfFile = _tfsopen(strTemp, _T("wb"), _SH_DENYWR);

		if (pRtfFile != NULL)
		{
			EDITSTREAM		es;
			CStringA			strRTF;

			es.dwCookie = (DWORD)&strRTF;
			es.pfnCallback = MEditStreamOutCallbackA;
			StreamOut(SF_RTF, es);
			fwrite(strRTF.GetString(), strRTF.GetLength(), 1, pRtfFile);
			fclose(pRtfFile);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CHTRichEditCtrl::GetHtml()
{
	CStringA		strRtfUtf8;
	CString			strTemp, strText;
	EDITSTREAM		es;

	es.dwCookie = (DWORD)&strRtfUtf8;
	es.pfnCallback = MEditStreamOutCallbackA;
	StreamOut((CP_UTF8 << 16) | SF_USECODEPAGE | SF_RTF, es);
	MB2Str(cfUTF8, &strTemp, strRtfUtf8);

	strTemp.Replace(_T("&"), _T("&amp;"));
	strTemp.Replace(_T("<"), _T("&lt;"));
	strTemp.Replace(_T(">"), _T("&gt;"));
	strTemp.Replace(_T("\""), _T("&quot;"));
	strTemp.Replace(_T("\\\'"), _T(" \\\'"));

	int				iTableStart = strTemp.Find(_T("\n{\\colortbl ;"));
	int				iTableEnd = strTemp.Find(_T(";}"), iTableStart + 1);

	if ((iTableStart != -1) && (iTableEnd != -1))
	{
		CStringArray	astrColorsRtf, astrColorsHtml;
		int				iRtfColor = 0, iColorEnd;
		long			lHtmlColor = 0;
		CString			strColor;

		while ((iTableStart < iTableEnd) && (iTableStart != -1))
		{
			iTableStart = strTemp.Find(_T("\\red"), iTableStart + 1);
			strColor.Format(_T("\\cf%i"), iRtfColor);
			astrColorsRtf.Add(strColor);
			strColor.Format(_T(" </font><font color=\"#%06x\">"), lHtmlColor);
			astrColorsHtml.Add(strColor);
			iRtfColor++;
			if (iTableStart >= 0)
			{
				iColorEnd = strTemp.Find(_T('\\'), iTableStart + 1);
				if (iColorEnd >= 0)
				{
					lHtmlColor = (_tstoi(strTemp.Mid(iTableStart + 4, iColorEnd - (iTableStart + 4))) << 16);
					iTableStart = iColorEnd + 6;
					iColorEnd = strTemp.Find(_T('\\'), iTableStart + 1);
					if (iColorEnd >= 0)
					{
						lHtmlColor += (_tstoi(strTemp.Mid(iTableStart, iColorEnd - iTableStart)) << 8);
						iTableStart = iColorEnd + 5;
						iColorEnd = strTemp.Find(_T(';'), iTableStart + 1);
						if (iColorEnd >= 0)
							lHtmlColor += _tstoi(strTemp.Mid(iTableStart, iColorEnd - iTableStart));
					}
				}
			}
		}
		for (int i = 0; i < astrColorsRtf.GetSize(); i++)
		{
			strTemp.Replace(astrColorsRtf[i] + _T(" \\\'"), astrColorsHtml[i] + _T(" \\\'"));
			strTemp.Replace(astrColorsRtf[i] + _T(' '), astrColorsHtml[i]);
			strTemp.Replace(astrColorsRtf[i] + _T('\\'), astrColorsHtml[i] + _T('\\'));
		}

		LPCTSTR			psz = strTemp.GetString();

		psz += strTemp.Find(_T('\\'), iTableEnd);

		LPCTSTR			pszStart = psz;
		TCHAR			strChar[2];
		long			lLen;

		while (*psz != _T('\0'))
		{
			if (*psz == _T('\\'))
			{
				if (*(psz + 1) == _T('\\'))
				{
					strText.Append(pszStart, 1 + psz - pszStart);
					psz += 2;
				}
				else if (*(psz + 1) == _T('\''))
				{
					if ((psz - pszStart) > 0)
						strText.Append(pszStart, psz - pszStart - 1);

					strChar[0] = *(psz + 2);
					strChar[1] = *(psz + 3);
					strText += (TCHAR)_tcstol(strChar, NULL, 16);
					psz += 4;
				}
				else
				{
					if ((psz - pszStart) > 0)
						strText.Append(pszStart, psz - pszStart);

					lLen = _tcscspn(psz, _T(" \n"));
					if (lLen == 0)
						psz += _tcslen(psz);
					else
					{
						psz += lLen;
						if (*psz == _T(' '))
							psz++;
					}
				}
				pszStart = psz;
			}
			else
				psz++;
		}
		if (pszStart != psz)
			strText += pszStart;
	}

	iTableEnd = strText.ReverseFind(_T('}'));
	if (iTableEnd != -1)
		strText.Truncate(iTableEnd);
	strText.TrimRight(_T('\n'));
	strText.Replace(_T(" </font><font color"), _T("</font><font color"));

	return strText;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CHTRichEditCtrl::GetToolTip()
{
	CStringA		strRtfUtf8;
	CString			strTemp, strText;
	EDITSTREAM		es;

	es.dwCookie = (DWORD)&strRtfUtf8;
	es.pfnCallback = MEditStreamOutCallbackA;
	StreamOut((CP_UTF8 << 16) | SF_USECODEPAGE | SF_RTF, es);
	MB2Str(cfUTF8, &strTemp, strRtfUtf8);

	strTemp.Replace(_T("<"), _T("<<"));
	strTemp.Replace(_T("\\\'"), _T(" \\\'"));

	int				iTableStart = strTemp.Find(_T("\n{\\colortbl ;"));
	int				iTableEnd = strTemp.Find(_T(";}"), iTableStart + 1);

	if ((iTableStart >= 0) && (iTableEnd >= 0))
	{
		CStringArray	astrColorsRtf, astrColorsTT;
		int				iTTColor = 0, iRtfColor = 0, iColorEnd;
		CString			strColor;

		while ((iTableStart < iTableEnd) && (iTableStart != -1))
		{
			iTableStart = strTemp.Find(_T("\\red"), iTableStart + 1);
			strColor.Format(_T("\\cf%i"), iRtfColor);
			astrColorsRtf.Add(strColor);
			if ((iRtfColor == 0) || (iTTColor == static_cast<int>(GetSysColor(COLOR_WINDOWTEXT))))
				strColor = _T(" <ct>");
			else
				strColor.Format(_T(" <ct=0x%06x>"), iTTColor);
			astrColorsTT.Add(strColor);
			iRtfColor++;
			if (iTableStart != -1)
			{
				iColorEnd = strTemp.Find(_T('\\'), iTableStart + 1);
				if (iColorEnd >= 0)
				{
					iTTColor = _tstoi(strTemp.Mid(iTableStart + 4, iColorEnd - (iTableStart + 4)));
					iTableStart = iColorEnd + 6;
					iColorEnd = strTemp.Find(_T('\\'), iTableStart + 1);
					if (iColorEnd >= 0)
					{
						iTTColor += (_tstoi(strTemp.Mid(iTableStart, iColorEnd - iTableStart)) << 8);
						iTableStart = iColorEnd + 5;
						iColorEnd = strTemp.Find(_T(';'), iTableStart + 1);
						if (iColorEnd >= 0)
							iTTColor += (_tstoi(strTemp.Mid(iTableStart, iColorEnd - iTableStart)) << 16);
					}
				}
			}
		}
		for (int i = 0; i < astrColorsRtf.GetSize(); i++)
		{
			strTemp.Replace(astrColorsRtf[i] + _T(" \\\'"), astrColorsTT[i] + _T(" \\\'"));
			strTemp.Replace(astrColorsRtf[i] + _T(' '), astrColorsTT[i]);
			strTemp.Replace(astrColorsRtf[i] + _T('\\'), astrColorsTT[i] + _T('\\'));
		}

		int			iLines = 0, iStrEnd = strTemp.ReverseFind(_T('}'));

		if (iStrEnd != -1)
			strTemp.Truncate(iStrEnd);
		strTemp.TrimRight(_T('\n'));

		LPCTSTR		pszStart = strTemp;
		LPCTSTR		psz = pszStart;

		pszStart += strTemp.Find(_T('\\'), iTableEnd);
		psz += strTemp.GetLength() - 1;

		CString		strLog, strStatus = g_App.m_pMDlg->m_ctlStatusBar.GetText(SB_MESSAGETEXT, 0);
		int			iLastFound, iNextFound, iCount = 11;

#ifdef _UNICODE
		es.dwCookie = (DWORD)&strLog;
		es.pfnCallback = MEditStreamOutCallbackW;
		StreamOut(SF_UNICODE | SF_TEXT, es);
#else
		GetWindowText(strLog);
#endif
		iLastFound = strLog.Find(strStatus);
		iNextFound = iLastFound;
		while (iNextFound >= 0)
		{
			iLastFound = iNextFound;
			iNextFound = strLog.Find(strStatus, iLastFound + 1);
		}
		
		if (iLastFound >= 0)
		{
			iCount = 0;
			iNextFound = strLog.Find(_T("\n"), iLastFound + 1);
			while (iNextFound >= 0)
			{
				iCount++;
				iNextFound = strLog.Find(_T("\n"), iNextFound + 1);
			}
			
			while (iCount > 0 && psz > pszStart)
			{
				if (*psz == _T('\n'))
					iCount--;
				psz--;
			}
			iCount = 10 - iCount;
			strTemp.Truncate(psz - strTemp.GetString());
		}

		while (iLines < iCount && psz > pszStart)
		{
			if (*psz == _T('\n'))
				iLines++;
			psz--;
		}

		if (psz > pszStart)
			psz++;

		pszStart = psz;

		TCHAR		strChar[2];
		long		lLen;

		while (*psz != _T('\0'))
		{
			if (*psz == _T('\\'))
			{
				if (*(psz + 1) == _T('\\'))
				{
					strText.Append(pszStart, (1 + psz - pszStart));
					psz += 2;
				}
				else if (*(psz + 1) == _T('\''))
				{
					if ((psz - pszStart) > 0)
						strText.Append(pszStart, (psz - pszStart - 1));

					strChar[0] = *(psz + 2);
					strChar[1] = *(psz + 3);
					strText += (TCHAR)_tcstol(strChar, NULL, 16);
					psz += 4;
				}
				else
				{
					if ((psz - pszStart) > 0)
						strText.Append(pszStart, (psz - pszStart));

					lLen = _tcscspn(psz, _T(" \n"));
					if (lLen == 0)
						psz += _tcslen(psz);
					else
					{
						psz += lLen;
						if (*psz == _T(' '))
							psz++;
					}
				}
				pszStart = psz;
			}
			else
				psz++;
		}
		if (pszStart != psz)
			strText.Append(pszStart);
	}

	strText.Trim(_T('\n'));
	strText.Replace(_T(" <ct"), _T("<ct"));
	while (strText.Right(4) == _T("<ct>"))
		strText.Truncate(strText.GetLength() - CSTRLEN(_T("<ct>")));
	strText.TrimRight(_T('\n'));
	strText.Replace(_T("\n"), _T("<br>"));
	if (strText[0] != _T('<'))
		strText.Insert(0, _T("<"));

	return strText;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHTRichEditCtrl::SetFont(CFont* pFont, bool bRedraw /*= true*/)
{
	SetBackgroundColor(false, m_crDefaultBackground);
	
	LOGFONT			lf = {0};
	CHARFORMAT2		cf2;

	pFont->GetLogFont(&lf);
	memzero(&cf2, sizeof(cf2));
	cf2.cbSize = sizeof(cf2);
	cf2.dwMask = CFM_FACE | CFM_SIZE /*| CFM_CHARSET*/;

	if (GetWindowTextLength() == 0)
		cf2.dwMask |= CFM_BACKCOLOR | CFM_BOLD | CFM_COLOR | CFM_ITALIC | CFM_LINK | CFM_STRIKEOUT | CFM_UNDERLINE;

	cf2.crTextColor = m_crDefaultForeground;
	cf2.crBackColor = m_crDefaultBackground;

	HDC				hDC = ::GetDC(NULL);

	cf2.yHeight = (-MulDiv(lf.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY))) * 20;
	::ReleaseDC(NULL, hDC);

	cf2.bPitchAndFamily = lf.lfPitchAndFamily;
	_tcsncpy(cf2.szFaceName, lf.lfFaceName, ARRSIZE(cf2.szFaceName));
	cf2.szFaceName[ARRSIZE(cf2.szFaceName) - 1] = _T('\0');

	// although this should work correctly (according SDK) it may give false results (e.g. the "click here..." text
	// which is shown in the server info window may not be entirely used as a hyperlink???)
	//cf2.bCharSet = lf.lfCharSet;
	//cf2.yOffset = 0;
	VERIFY( SetDefaultCharFormat(cf2) );

	if (bRedraw)
	{
		Invalidate();
		UpdateWindow();
	}
}
