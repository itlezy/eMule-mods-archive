//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "eMule.h"
#include "FileDetailDialogInfo.h"
#include "UserMsgs.h"
#include "OtherFunctions.h"
#include "PartFile.h"
#include "Preferences.h"
#include "shahashset.h"
#include "UpDownClient.h"
#include "FileVerify.h"// X: [FV] - [FileVerify]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// CFileDetailDialogInfo dialog

LPCTSTR CFileDetailDialogInfo::sm_pszNotAvail = _T("-");

IMPLEMENT_DYNAMIC(CFileDetailDialogInfo, CResizablePage)

BEGIN_MESSAGE_MAP(CFileDetailDialogInfo, CResizablePage)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

CFileDetailDialogInfo::CFileDetailDialogInfo()
	: CResizablePage(CFileDetailDialogInfo::IDD, 0)
{
	m_paFiles = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_FD_GENERAL);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
	m_timer = 0;
}

CFileDetailDialogInfo::~CFileDetailDialogInfo()
{
}

void CFileDetailDialogInfo::OnTimer(UINT_PTR /*nIDEvent*/)
{
	RefreshData();
}

BOOL CFileDetailDialogInfo::OnInitDialog()
{
	CResizablePage::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDC_FD_X0, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_FD_X6, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_FD_X8, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_FD_X11, TOP_LEFT, TOP_RIGHT);
	
	AddAnchor(IDC_FNAME, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_METFILE, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_FHASH, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_FD_AICHHASH, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_FSIZE, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_PARTCOUNT, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_HASHSET, TOP_LEFT, TOP_RIGHT);

	AddAnchor(IDC_SOURCECOUNT, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_DATARATE, TOP_LEFT);

	AddAnchor(IDC_FILECREATED, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_DL_ACTIVE_TIME, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_LASTSEENCOMPL, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_LASTRECEIVED, TOP_LEFT, TOP_RIGHT);

	Localize();

	// no need to explicitly call 'RefreshData' here, 'OnSetActive' will be called right after 'OnInitDialog'

	// start time for calling 'RefreshData'
	VERIFY( (m_timer = SetTimer(301, 5000, 0)) != NULL );

	return TRUE;
}

BOOL CFileDetailDialogInfo::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;
	if (m_bDataChanged)
	{
		RefreshData();
		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CFileDetailDialogInfo::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CFileDetailDialogInfo::RefreshData()
{
	CString str;
	int uFiles = m_paFiles->GetSize();

	if (uFiles == 1)
	{
		//CPartFile* file = STATIC_DOWNCAST(CPartFile, (*m_paFiles)[0]);
		CPartFile* file = (CPartFile*)(*m_paFiles)[0];

		// if file is completed, we output the 'file path' and not the 'part.met file path'
		if (file->GetStatus(true) == PS_COMPLETE)
			SetDlgItemText(IDC_FD_X2,GetResString(IDS_DL_FILENAME)+_T(':'));
		else
			SetDlgItemText(IDC_FD_X2,GetResString(IDS_FD_MET)); // ducho


		SetDlgItemText(IDC_FNAME, file->GetFileName());
		SetDlgItemText(IDC_METFILE, file->GetFullName());
		SetDlgItemText(IDC_FHASH, md4str(file->GetFileHash()));

		if (file->GetTransferringSrcCount() > 0)
			str.Format(GetResString(IDS_PARTINFOS2), file->GetTransferringSrcCount());
		else
			str = file->getPartfileStatus();
		SetDlgItemText(IDC_PFSTATUS, str);

		str.Format(_T("%u;  %s: %u (%.1f%%)"), file->GetPartCount(), GetResString(IDS_AVAILABLE) , file->GetAvailablePartCount(), (float)((file->GetAvailablePartCount()*100)/file->GetPartCount()));
		SetDlgItemText(IDC_PARTCOUNT, str);

		// date created
		if (file->GetCrFileDate() != 0) {
			str.Format(_T("%s   ") + GetResString(IDS_TIMEBEFORE),
						file->GetCrCFileDate().Format(thePrefs.GetDateTimeFormat()),
						CastSecondsToLngHM((uint_ptr)(time(NULL) - file->GetCrFileDate())));
		}
		else
			str = _T('-');
		SetDlgItemText(IDC_FILECREATED, str);

		// active download time
		uint_ptr nDlActiveTime = file->GetDlActiveTime();
		if (nDlActiveTime)
			str = CastSecondsToLngHM(nDlActiveTime);
		else
			str = _T('-');
		SetDlgItemText(IDC_DL_ACTIVE_TIME, str);

		// last seen complete
		struct tm tmTemp;
		struct tm* ptimLastSeenComplete = file->lastseencomplete.GetLocalTm(&tmTemp);
		if (file->lastseencomplete == NULL || ptimLastSeenComplete == NULL)
			str.Format(GetResString(IDS_NEVER));
		else {
			str.Format(_T("%s   ") + GetResString(IDS_TIMEBEFORE),
						file->lastseencomplete.Format(thePrefs.GetDateTimeFormat()),
						CastSecondsToLngHM((uint_ptr)(time(NULL) - safe_mktime(ptimLastSeenComplete))));
		}
		SetDlgItemText(IDC_LASTSEENCOMPL, str);

		// last receive
		if (file->GetFileDate() != 0 && file->GetRealFileSize() > (uint64)0)
		{
			// 'Last Modified' sometimes is up to 2 seconds greater than the current time ???
			// If it's related to the FAT32 seconds time resolution the max. failure should still be only 1 sec.
			// Happens at least on FAT32 with very high download speed.
			uint64 tLastModified = file->GetFileDate();
			uint64 tNow = time(NULL);
			uint_ptr tAgo;
			if (tNow >= tLastModified)
				tAgo = (uint_ptr)(tNow - tLastModified);
			else{
				TRACE("tNow = %s\n", CTime(tNow).Format(_T("%X")));
				TRACE("tLMd = %s, +%u\n", CTime(tLastModified).Format(_T("%X")), tLastModified - tNow);
				TRACE("\n");
				tAgo = 0;
			}
			str.Format(_T("%s   ") + GetResString(IDS_TIMEBEFORE),
						(tLastModified ? file->GetCFileDate().Format(thePrefs.GetDateTimeFormat()) : GetResString(IDS_NEVER)),
						CastSecondsToLngHM(tAgo));
		}
		else
			str = GetResString(IDS_NEVER);
		SetDlgItemText(IDC_LASTRECEIVED, str);

		// AICH Hash
		switch (file->GetAICHRecoveryHashSet()->GetStatus()) {
			case AICH_TRUSTED:
			case AICH_VERIFIED:
			case AICH_HASHSETCOMPLETE:
				if (file->GetAICHRecoveryHashSet()->HasValidMasterHash()) {
					SetDlgItemText(IDC_FD_AICHHASH, file->GetAICHRecoveryHashSet()->GetMasterHash().GetString());
					break;
				}
			default:
				SetDlgItemText(IDC_FD_AICHHASH, _T("-"));
				break;
		}

		// file type
		FileFmt_Struct* fmt = file->GetFileFormat();// X: [FV] - [FileVerify]
		if(fmt)
			str = fmt->label;
			else {
				// not verified
				int pos=file->GetFileName().ReverseFind(_T('.'));
				if (file->GetFileName().ReverseFind(_T('\\'))<pos){
					str=file->GetFileName().Mid(pos+1);
					str.MakeUpper();
					str.Append(_T("  (") );
					str.Append( GetResString(IDS_UNKNOWN) +_T(')') );
				}
				else
					str=GetResString(IDS_UNKNOWN);
			}
		if(file->verifystatus > VS_NONE){
			str.Append(_T(" ("));
			if(file->verifystatus == VS_UTF16)
				str.Append(_T("UTF-16"));
			else if(file->verifystatus == VS_UTF8)
				str.Append(_T("UTF-8"));
			else{
				static const UINT vsStrID[]={
					IDS_FV_SIZESMALL,
					IDS_FV_SIZELARGE,
					IDS_FV_SIZENA
				};
				str.Append(GetResString(vsStrID[file->verifystatus-VS_SIZESMALL]));
				str.AppendChar(_T(')'));
			}
		}
		SetDlgItemText(IDC_FD_X11,str);
	}
	else
	{
		SetDlgItemText(IDC_FNAME, sm_pszNotAvail);
		SetDlgItemText(IDC_METFILE, sm_pszNotAvail);
		SetDlgItemText(IDC_FHASH, sm_pszNotAvail);

		SetDlgItemText(IDC_PFSTATUS, sm_pszNotAvail);
		SetDlgItemText(IDC_PARTCOUNT, sm_pszNotAvail);
		SetDlgItemText(IDC_FD_X11, sm_pszNotAvail);

		SetDlgItemText(IDC_FILECREATED, sm_pszNotAvail);
		SetDlgItemText(IDC_DL_ACTIVE_TIME, sm_pszNotAvail);
		SetDlgItemText(IDC_LASTSEENCOMPL, sm_pszNotAvail);
		SetDlgItemText(IDC_LASTRECEIVED, sm_pszNotAvail);
		SetDlgItemText(IDC_FD_AICHHASH, sm_pszNotAvail);
	}

	uint64 uFileSize = 0;
	uint64 uRealFileSize = 0;
	uint64 uTransferred = 0;
	uint64 uCorrupted = 0;
	uint32 uRecoveredParts = 0;
	uint64 uCompression = 0;
	uint64 uCompleted = 0;
	size_t iMD4HashsetAvailable = 0;
	size_t iAICHHashsetAvailable = 0;
	uint32 uDataRate = 0;
	size_t uSources = 0;
	UINT uValidSources = 0;
	UINT uNNPSources = 0;
	size_t uA4AFSources = 0;
	double dAvgDlSpeed = 0; // Average download speed - Stulle
	for (int i = 0; i < uFiles; i++)
	{
		//const CPartFile* file = STATIC_DOWNCAST(CPartFile, (*m_paFiles)[i]);
		const CPartFile* file = (CPartFile*)(*m_paFiles)[i];

		uFileSize += (uint64)file->GetFileSize();
		uRealFileSize += (uint64)file->GetRealFileSize();
		uTransferred += (uint64)file->GetTransferred();
		uCorrupted += file->GetCorruptionLoss();
		uRecoveredParts += file->GetRecoveredPartsByICH();
		uCompression += file->GetCompressionGain();
		uDataRate += file->GetDownloadDatarate10(); //Xman // Maella -Accurate measure of bandwidth
		uCompleted += (uint64)file->GetCompletedSize();
		iMD4HashsetAvailable += (file->GetFileIdentifierC().HasExpectedMD4HashCount()) ? 1 : 0;
		iAICHHashsetAvailable += (file->GetFileIdentifierC().HasExpectedAICHHashCount()) ? 1 : 0;

		// ==> Average download speed - Stulle
		if(file->GetDlActiveTime() > 0)
			dAvgDlSpeed += (double)(file->GetTransferred()/file->GetDlActiveTime());
		// <== Average download speed - Stulle

		if (file->IsPartFile())
		{
			uSources += file->GetSourceCount();
			uValidSources += file->GetValidSourcesCount();
			uNNPSources += file->GetSrcStatisticsValue(DS_NONEEDEDPARTS);
			uA4AFSources += file->GetSrcA4AFCount();
		}
	}

	str.Format(_T("%s  (%s B);  %s %s"), CastItoXBytes(uFileSize, false, false), GetFormatedUInt64(uFileSize), GetResString(IDS_ONDISK), CastItoXBytes(uRealFileSize, false, false));
	SetDlgItemText(IDC_FSIZE, str);

	if (m_paFiles->GetSize() == 1)
	{
		if (iAICHHashsetAvailable == 0 && iMD4HashsetAvailable == 0)
			SetDlgItemText(IDC_HASHSET, GetResString(IDS_NO));
		else if (iAICHHashsetAvailable == 1 && iMD4HashsetAvailable == 1)
			SetDlgItemText(IDC_HASHSET, GetResString(IDS_YES) + _T(" (eD2K + AICH)"));
		else if (iAICHHashsetAvailable == 1)
			SetDlgItemText(IDC_HASHSET, GetResString(IDS_YES) + _T(" (AICH)"));
		else if (iMD4HashsetAvailable == 1)
			SetDlgItemText(IDC_HASHSET, GetResString(IDS_YES) + _T(" (eD2K)"));
	}
	else
	{
		if (iAICHHashsetAvailable == 0 && iMD4HashsetAvailable == 0)
		SetDlgItemText(IDC_HASHSET, GetResString(IDS_NO));
		else if (iMD4HashsetAvailable == m_paFiles->GetSize() && iAICHHashsetAvailable == m_paFiles->GetSize())
			SetDlgItemText(IDC_HASHSET, GetResString(IDS_YES) +  + _T(" (eD2K + AICH)"));
	else
		SetDlgItemText(IDC_HASHSET, _T(""));
	}

	str.Format(GetResString(IDS_SOURCESINFO), uSources, uValidSources, uNNPSources, uA4AFSources);
	SetDlgItemText(IDC_SOURCECOUNT, str);

	// ==> Average download speed - Stulle
	str.Format(_T("%s(%s)"), CastItoXBytes(uDataRate, false, true), CastItoXBytes(dAvgDlSpeed, false, true));
	SetDlgItemText(IDC_DATARATE, str);
	// <== Average download speed - Stulle

	SetDlgItemText(IDC_TRANSFERRED, CastItoXBytes(uTransferred, false, false));

	str.Format(_T("%s (%.2f%%)"), CastItoXBytes(uCompleted, false, false), uFileSize!=0 ? (uCompleted * 100.0 / uFileSize) : 0.0);// X: [UIC] - [UIChange]
	SetDlgItemText(IDC_COMPLSIZE, str);

	str.Format(_T("%s (%.2f%%)"), CastItoXBytes(uCorrupted, false, false), uTransferred!=0 ? (uCorrupted * 100.0 / uTransferred) : 0.0);// X: [UIC] - [UIChange]
	SetDlgItemText(IDC_CORRUPTED, str);

	str.Format(_T("%s (%.2f%%)"), CastItoXBytes(uFileSize - uCompleted, false, false), uFileSize!=0 ? ((uFileSize - uCompleted) * 100.0 / uFileSize) : 0.0);// X: [UIC] - [UIChange]
	SetDlgItemText(IDC_REMAINING, str);

	str.Format(_T("%u %s"), uRecoveredParts, GetResString(IDS_FD_PARTS));
	SetDlgItemText(IDC_RECOVERED, str);

	str.Format(_T("%s (%.2f%%)"), CastItoXBytes(uCompression, false, false), uCompleted!=0 ? (uCompression * 100.0 / uCompleted) : 0.0);// X: [UIC] - [UIChange] // Tux: Fix: accurate compression percentage indication fix [BlueSonicBoy]: changed line
	SetDlgItemText(IDC_COMPRESSION, str);
}

void CFileDetailDialogInfo::OnDestroy()
{
	if (m_timer){
		KillTimer(m_timer);
		m_timer = 0;
	}
}

void CFileDetailDialogInfo::Localize()
{
	SetDlgItemText(IDC_FD_X0,GetResString(IDS_FD_GENERAL));
	SetDlgItemText(IDC_FD_X1,GetResString(IDS_SW_NAME)+_T(':'));
	SetDlgItemText(IDC_FD_X2,GetResString(IDS_FD_MET));
	SetDlgItemText(IDC_FD_X3,GetResString(IDS_FD_HASH));
	SetDlgItemText(IDC_FD_X4,GetResString(IDS_DL_SIZE)+_T(':'));
	SetDlgItemText(IDC_FD_X9,GetResString(IDS_FD_PARTS)+_T(':'));
	SetDlgItemText(IDC_FD_X5,GetResString(IDS_STATUS)+_T(':'));
	SetDlgItemText(IDC_FD_X6,GetResString(IDS_FD_TRANSFER));
	SetDlgItemText(IDC_FD_X7,GetResString(IDS_DL_SOURCES)+_T(':'));
	SetDlgItemText(IDC_FD_X14,GetResString(IDS_FD_TRANS));
	SetDlgItemText(IDC_FD_X12,GetResString(IDS_FD_COMPSIZE));
	SetDlgItemText(IDC_FD_X13,GetResString(IDS_FD_DATARATE));
	SetDlgItemText(IDC_FD_X15,GetResString(IDS_LASTSEENCOMPL));
	SetDlgItemText(IDC_FD_LASTCHANGE,GetResString(IDS_FD_LASTCHANGE));
	SetDlgItemText(IDC_FD_X8,GetResString(IDS_FD_TIMEDATE));
	SetDlgItemText(IDC_FD_X16,GetResString(IDS_FD_DOWNLOADSTARTED));
	SetDlgItemText(IDC_DL_ACTIVE_TIME_LBL,GetResString(IDS_DL_ACTIVE_TIME)+_T(':'));
	SetDlgItemText(IDC_HSAV,GetResString(IDS_HSAV)+_T(':'));
	SetDlgItemText(IDC_FD_CORR,GetResString(IDS_FD_CORR)+_T(':'));
	SetDlgItemText(IDC_FD_RECOV,GetResString(IDS_FD_RECOV)+_T(':'));
	SetDlgItemText(IDC_FD_COMPR,GetResString(IDS_FD_COMPR)+_T(':'));
	SetDlgItemText(IDC_FD_XAICH,GetResString(IDS_AICHHASH)+_T(':'));
	SetDlgItemText(IDC_REMAINING_TEXT, GetResString(IDS_DL_REMAINS)+_T(':'));
	SetDlgItemText(IDC_FD_X10, GetResString(IDS_TYPE)+_T(':') );
}
