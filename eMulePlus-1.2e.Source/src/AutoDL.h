//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

#define DLURLMAX 512

struct CAutoDLData
{
	TCHAR	acUrl[DLURLMAX];
	ULONG	ulInterval;	// in minutes
	long	lLastCheck;	// time() value
};

class CAutoDL : public CLoggable
{
public:
	CAutoDL();
	virtual ~CAutoDL();

	bool LoadPrefs();
	bool SavePrefs();

	void Restart(){ Start(); }

	__declspec(property(get=get_UseIt,put=put_UseIt))	bool UseIt;
	__declspec(property(get=get_UrlCount))				long UrlCount;
	__declspec(property(get=get_UrlItem))				CAutoDLData UrlItem[];

	bool get_UseIt();
	void put_UseIt(bool bUseIt);
	long get_UrlCount();
	CAutoDLData get_UrlItem(long nIndex);
	void ClearUrlList();
	void AddUrlItem(CAutoDLData& data);

private:
	static void WorkerThread(void *);
	void Start();
	void Finish();
	void GetConfigFilename(CString *pstrOut);
	ULONG CheckUrl(const TCHAR *pcUrl);
	void DownloadLink(CString sLink, CString sCategory);

private:
	HANDLE	m_hWorkerThread;
	HANDLE	m_hFinish;

	CArray<CAutoDLData, CAutoDLData> m_UrlList;
	bool	m_bUseIt;
};
