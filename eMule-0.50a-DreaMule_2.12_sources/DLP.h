

#pragma once



class CDLP
{
public:
	CDLP(CString appdir_in);
	~CDLP();
	bool IsDLPavailable()		{return dlpavailable;}
	void Reload();

	typedef DWORD (__cdecl *GETDLPVERSION)();
	GETDLPVERSION GetDLPVersion;

	typedef LPCTSTR (__cdecl *DLPCHECKMODSTRING_HARD)(LPCTSTR modversion, LPCTSTR clientversion);
	DLPCHECKMODSTRING_HARD DLPCheckModstring_Hard;
	typedef LPCTSTR (__cdecl *DLPCHECKMODSTRING_SOFT)(LPCTSTR modversion, LPCTSTR clientversion);
	DLPCHECKMODSTRING_SOFT DLPCheckModstring_Soft;

	typedef LPCTSTR (__cdecl *DLPCHECKUSERNAME_HARD)(LPCTSTR username);
	DLPCHECKUSERNAME_HARD DLPCheckUsername_Hard;
	typedef LPCTSTR (__cdecl *DLPCHECKUSERNAME_SOFT)(LPCTSTR username);
	DLPCHECKUSERNAME_SOFT DLPCheckUsername_Soft;
	
	typedef LPCTSTR (__cdecl *DLPCHECKNAMEANDHASHANDMOD)(CString username, CString& userhash, CString& modversion);
	DLPCHECKNAMEANDHASHANDMOD DLPCheckNameAndHashAndMod;

	typedef LPCTSTR (__cdecl *DLPCHECKMESSAGESPAM)(LPCTSTR messagetext);
	DLPCHECKMESSAGESPAM DLPCheckMessageSpam;


	typedef LPCTSTR (__cdecl *DLPCHECKHELLOTAG)(UINT tagnumber);
	DLPCHECKHELLOTAG DLPCheckHelloTag;
	typedef LPCTSTR (__cdecl *DLPCHECKINFOTAG)(UINT tagnumber);
	DLPCHECKINFOTAG DLPCheckInfoTag;

	//typedef void (WINAPI*TESTFUNC)();
	//TESTFUNC testfunc;

	

private:
	HINSTANCE dlpInstance;
	bool	dlpavailable;
	CString appdir;
};