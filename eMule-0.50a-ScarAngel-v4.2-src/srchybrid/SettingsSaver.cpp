//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#include "StdAfx.h"
#include "SettingsSaver.h"
#include "PartFile.h"
#include "emule.h"
#include "Preferences.h" // for thePrefs
#include "emuledlg.h" // for theApp.emuledlg
#include "log.h" // for log
#include "Ini2.h"
#include "DownloadQueue.h"

/* This class has been rewritten by Stulle */

CSettingsSaver::CSettingsSaver(void){}
CSettingsSaver::~CSettingsSaver(void){}

void CSettingsSaver::LoadSettings()
{
	CPartFile* cur_file ;
	int iTryImport = 0; // 0 = no; 1 = import; 2 = delete

	CString IniFilePath;
	IniFilePath.Format(L"%sFileSettings.ini", thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	CIni ini(IniFilePath);

	CString OldFilePath;
	OldFilePath.Format(_T("%s\\%s\\"), thePrefs.GetTempDir(), _T("Extra Lists"));

	if(!PathFileExists(IniFilePath))
		iTryImport = 1;
	else if(PathFileExists(OldFilePath)) // we assume if the default contains this all else do.
		iTryImport = 2;

	for (POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition();pos != 0;){
		cur_file = theApp.downloadqueue->filelist.GetNext(pos);
		if(iTryImport == 1)
		{
			ImportOldSettings(cur_file);
			continue; // next item
		}
		else if(iTryImport == 2)
		{
			DeleteOldSettings(cur_file);
		}

		ini.SetSection(cur_file->GetPartMetFileName());

		cur_file->SetEnableAutoDropNNS(ini.GetBool(L"NNS",thePrefs.GetEnableAutoDropNNSDefault()));
		cur_file->SetAutoNNS_Timer(ini.GetInt(L"NNSTimer",thePrefs.GetAutoNNS_TimerDefault()));
		cur_file->SetMaxRemoveNNSLimit((uint16)ini.GetInt(L"NNSLimit",thePrefs.GetMaxRemoveNNSLimitDefault()));

		cur_file->SetEnableAutoDropFQS(ini.GetBool(L"FQS",thePrefs.GetEnableAutoDropFQSDefault()));
		cur_file->SetAutoFQS_Timer(ini.GetInt(L"FQSTimer",thePrefs.GetAutoFQS_TimerDefault()));
		cur_file->SetMaxRemoveFQSLimit((uint16)ini.GetInt(L"FQSLimit",thePrefs.GetMaxRemoveFQSLimitDefault()));

		cur_file->SetEnableAutoDropQRS(ini.GetBool(L"QRS",thePrefs.GetEnableAutoDropQRSDefault()));
		cur_file->SetAutoHQRS_Timer(ini.GetInt(L"QRSTimer",thePrefs.GetAutoHQRS_TimerDefault()));
		cur_file->SetMaxRemoveQRS((uint16)ini.GetInt(L"MaxQRS",thePrefs.GetMaxRemoveQRSDefault()));
		cur_file->SetMaxRemoveQRSLimit((uint16)ini.GetInt(L"QRSLimit",thePrefs.GetMaxRemoveQRSLimitDefault()));

		cur_file->SetGlobalHL(ini.GetBool(L"GlobalHL",thePrefs.GetGlobalHlDefault()));
		cur_file->SetHQRXman(ini.GetBool(L"XmanHQR",thePrefs.GetHQRXmanDefault()));
		cur_file->SetFollowTheMajority(ini.GetInt(L"FTM",-1));
	}

	if(iTryImport > 0)
	{
		for (int i=0;i<thePrefs.tempdir.GetCount();i++) {
			CString sSivkaFileSettingsPath = CString(thePrefs.GetTempDir(i)) + _T("\\") + SIVKAFOLDER;
			if (PathFileExists(sSivkaFileSettingsPath.GetBuffer()) && !::RemoveDirectory(sSivkaFileSettingsPath.GetBuffer())) {
				CString strError;
				strError.Format(_T("Failed to delete sivka extra lists directory \"%s\" - %s"), sSivkaFileSettingsPath, GetErrorMessage(GetLastError()));
				AfxMessageBox(strError, MB_ICONERROR);
			}
		}
	}
}

bool CSettingsSaver::SaveSettings()
{
	CPartFile* cur_file ;
	bool bAborted = false;

	CString strFileSettingsIniFilePath;
	strFileSettingsIniFilePath.Format(L"%sFileSettings.ini", thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	(void)_tremove(strFileSettingsIniFilePath);
	if(theApp.downloadqueue->filelist.GetCount()<=0) // nothing to save here
		return true; // everything's fine
	CStdioFile ini;
	CString buffer;

	if(ini.Open(strFileSettingsIniFilePath,CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive | CFile::typeText))
	{
		ini.WriteString(_T("[General]\r\n"));
		buffer.Format(_T("FileSettingsVersion=%i\r\n"),1);
		ini.WriteString(buffer);

		for (POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition();pos != 0;){
			cur_file = theApp.downloadqueue->filelist.GetNext(pos);

			if(!cur_file) // NULL-pointer? we deleted the file, break
			{
				AddDebugLogLine(false,_T("CSettingsSaver cur_file == NULL? wtf?!"));
				bAborted = true;
				break;
			}

			try // i just hope we don't need this
			{
				buffer.Format(_T("[%s]\r\n"),cur_file->GetPartMetFileName());
				ini.WriteString(buffer);

				buffer.Format(_T("NNS=%i\r\n"),cur_file->GetEnableAutoDropNNS()?1:0);
				ini.WriteString(buffer);
				buffer.Format(_T("NNSTimer=%u\r\n"),cur_file->GetAutoNNS_Timer());
				ini.WriteString(buffer);
				buffer.Format(_T("NNSLimit=%u\r\n"),cur_file->GetMaxRemoveNNSLimit());
				ini.WriteString(buffer);

				buffer.Format(_T("FQS=%i\r\n"),cur_file->GetEnableAutoDropFQS()?1:0);
				ini.WriteString(buffer);
				buffer.Format(_T("FQSTimer=%u\r\n"),cur_file->GetAutoFQS_Timer());
				ini.WriteString(buffer);
				buffer.Format(_T("FQSLimit=%u\r\n"),cur_file->GetMaxRemoveFQSLimit());
				ini.WriteString(buffer);

				buffer.Format(_T("QRS=%i\r\n"),cur_file->GetEnableAutoDropQRS()?1:0);
				ini.WriteString(buffer);
				buffer.Format(_T("QRSTimer=%u\r\n"),cur_file->GetAutoHQRS_Timer());
				ini.WriteString(buffer);
				buffer.Format(_T("MaxQRS=%u\r\n"),cur_file->GetMaxRemoveQRS());
				ini.WriteString(buffer);
				buffer.Format(_T("QRSLimit=%u\n"),cur_file->GetMaxRemoveQRSLimit());
				ini.WriteString(buffer);

				buffer.Format(_T("GlobalHL=%i\r\n"),cur_file->GetGlobalHL()?1:0);
				ini.WriteString(buffer);
				buffer.Format(_T("XmanHQR=%i\r\n"),cur_file->GetHQRXman()?1:0);
				ini.WriteString(buffer);
				buffer.Format(_T("FTM=%i\r\n"),cur_file->GetFollowTheMajority());
				ini.WriteString(buffer);
			}
			catch(...) // and if we do we break and log
			{
				bAborted = true;
				AddDebugLogLine(true,_T("We had to catch an error in SettingsSaver::SaveSettings()! Report this please!"));
				break;
			}
		}
		ini.Close();
	}
	else
		bAborted = true;

	return !bAborted; // report if we had to abort, this is no good
}

/* IMPORT OLD */
#pragma warning(disable:4296) // expression is always true
void CSettingsSaver::ImportOldSettings(CPartFile* file)
{
	SettingsList daten;
	CString datafilepath;
	datafilepath.Format(_T("%s\\%s\\%s.sivka"), file->GetTempPath(), _T("Extra Lists"), file->GetPartMetFileName());

	CString strLine;
	CStdioFile f;
	if (!f.Open(datafilepath, CFile::modeReadWrite | CFile::typeText))
		return;
	while(f.ReadString(strLine))
	{
		if (strLine.GetAt(0) == _T('#'))
			continue;
		int pos = strLine.Find(_T('\n'));
		if (pos == -1)
			continue;
		CString strData = strLine.Left(pos);
		CSettingsData* newdata = new CSettingsData(_tstol(strData));
		daten.AddTail(newdata);
	}
	f.Close();

	POSITION pos = daten.GetHeadPosition();
	if(!pos)
		return;

	if( ((CSettingsData*)daten.GetAt(pos))->dwData == 0 || ((CSettingsData*)daten.GetAt(pos))->dwData == 1)
	{
		file->SetEnableAutoDropNNS((((CSettingsData*)daten.GetAt(pos))->dwData)!=0);
	}
	else
		file->SetEnableAutoDropNNS(thePrefs.GetEnableAutoDropNNSDefault());

	daten.GetNext(pos);
	if( ((CSettingsData*)daten.GetAt(pos))->dwData >= 0 && ((CSettingsData*)daten.GetAt(pos))->dwData <= 60000)
		file->SetAutoNNS_Timer(((CSettingsData*)daten.GetAt(pos))->dwData);
	else
		file->SetAutoNNS_Timer(thePrefs.GetAutoNNS_TimerDefault());

	daten.GetNext(pos);
	if( ((CSettingsData*)daten.GetAt(pos))->dwData >= 50 && ((CSettingsData*)daten.GetAt(pos))->dwData <= 100)
		file->SetMaxRemoveNNSLimit((uint16)((CSettingsData*)daten.GetAt(pos))->dwData);
	else
		file->SetMaxRemoveNNSLimit(thePrefs.GetMaxRemoveNNSLimitDefault());

	daten.GetNext(pos);
	if( ((CSettingsData*)daten.GetAt(pos))->dwData == 0 || ((CSettingsData*)daten.GetAt(pos))->dwData == 1)
	{
		file->SetEnableAutoDropFQS((((CSettingsData*)daten.GetAt(pos))->dwData)!=0);
	}
	else
		file->SetEnableAutoDropFQS(thePrefs.GetEnableAutoDropFQSDefault());

	daten.GetNext(pos);
	if( ((CSettingsData*)daten.GetAt(pos))->dwData >= 0 && ((CSettingsData*)daten.GetAt(pos))->dwData <= 60000)
		file->SetAutoFQS_Timer(((CSettingsData*)daten.GetAt(pos))->dwData);
	else
		file->SetAutoFQS_Timer(thePrefs.GetAutoFQS_TimerDefault());

	daten.GetNext(pos);
	if( ((CSettingsData*)daten.GetAt(pos))->dwData >= 50 && ((CSettingsData*)daten.GetAt(pos))->dwData <= 100)
		file->SetMaxRemoveFQSLimit((uint16)((CSettingsData*)daten.GetAt(pos))->dwData);
	else
		file->SetMaxRemoveFQSLimit(thePrefs.GetMaxRemoveFQSLimitDefault());

	daten.GetNext(pos);
	if( ((CSettingsData*)daten.GetAt(pos))->dwData == 0 || ((CSettingsData*)daten.GetAt(pos))->dwData == 1)
	{
		file->SetEnableAutoDropQRS((((CSettingsData*)daten.GetAt(pos))->dwData)!=0);
	}
	else
		file->SetEnableAutoDropQRS(thePrefs.GetEnableAutoDropQRSDefault());

	daten.GetNext(pos);
	if( ((CSettingsData*)daten.GetAt(pos))->dwData >= 0 && ((CSettingsData*)daten.GetAt(pos))->dwData <= 60000)
		file->SetAutoHQRS_Timer(((CSettingsData*)daten.GetAt(pos))->dwData);
	else
		file->SetAutoHQRS_Timer(thePrefs.GetAutoHQRS_TimerDefault());

	daten.GetNext(pos);
	if( ((CSettingsData*)daten.GetAt(pos))->dwData >= 1000 && ((CSettingsData*)daten.GetAt(pos))->dwData <= 10000)
		file->SetMaxRemoveQRS((uint16)((CSettingsData*)daten.GetAt(pos))->dwData);
	else
		file->SetMaxRemoveQRS(thePrefs.GetMaxRemoveQRSDefault());

	daten.GetNext(pos);
	if( ((CSettingsData*)daten.GetAt(pos))->dwData >= 50 && ((CSettingsData*)daten.GetAt(pos))->dwData <= 100)
		file->SetMaxRemoveQRSLimit((uint16)((CSettingsData*)daten.GetAt(pos))->dwData);
	else
		file->SetMaxRemoveQRSLimit(thePrefs.GetMaxRemoveQRSLimitDefault());

	if(daten.GetCount() > 10) // emulate StulleMule <= v2.2 files
	{
		// ==> Global Source Limit (customize for files) - Stulle
		daten.GetNext(pos);
		if( ((CSettingsData*)daten.GetAt(pos))->dwData == 0 || ((CSettingsData*)daten.GetAt(pos))->dwData == 1)
		{
			file->SetGlobalHL((((CSettingsData*)daten.GetAt(pos))->dwData)!=0);
		}
		else
			file->SetGlobalHL(thePrefs.GetGlobalHlDefault());
		// <== Global Source Limit (customize for files) - Stulle
	}
	else
		file->SetGlobalHL(thePrefs.GetGlobalHlDefault());

	while (!daten.IsEmpty()) 
		delete daten.RemoveHead();

	if (_tremove(datafilepath)) if (errno != ENOENT)
		AddLogLine(true, _T("Failed to delete %s, you will need to do this manually"), datafilepath);
}
#pragma warning(default:4296) // expression is always true

void CSettingsSaver::DeleteOldSettings(CPartFile* file)
{
	CString datafilepath;
	datafilepath.Format(_T("%s\\%s\\%s.sivka"), file->GetTempPath(), _T("Extra Lists"), file->GetPartMetFileName());

	if (_tremove(datafilepath)) if (errno != ENOENT)
		AddLogLine(true, _T("Failed to delete %s, you will need to do this manually"), datafilepath);
}