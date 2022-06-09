#pragma once

class MiniDumper
{
private:
	static bool		m_bWritten;
	static LPCTSTR	m_szKeepDumpFile;
	static LPCTSTR	m_szCantUseDBGHelp;
	static LPCTSTR	m_szDownloadDBGHelp;
	static LPCTSTR	m_szSavedDump;
	static LPCTSTR	m_szFailedToSave;

	static LONG WINAPI TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo );

public:
	MiniDumper();
	~MiniDumper();
	void LoadStrings();
};
