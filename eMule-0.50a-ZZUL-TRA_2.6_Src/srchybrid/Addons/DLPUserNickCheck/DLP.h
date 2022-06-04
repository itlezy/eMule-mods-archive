
#pragma once

class CDLP
{
public:
	CDLP(CString configdir_in);
	~CDLP();
	bool IsDLPavailable()		{return dlpavailable;}
	void Reload();

	typedef DWORD (__cdecl *GETDLPVERSION)();
	GETDLPVERSION GetDLPVersion;

	typedef LPCTSTR (__cdecl *DLPCHECKUSERNAME_HARD)(LPCTSTR username);
	DLPCHECKUSERNAME_HARD DLPCheckUsername_Hard;
	typedef LPCTSTR (__cdecl *DLPCHECKUSERNAME_SOFT)(LPCTSTR username);
	DLPCHECKUSERNAME_SOFT DLPCheckUsername_Soft;

private:
	HINSTANCE dlpInstance;
	bool	dlpavailable;
	CString configdir;
};
