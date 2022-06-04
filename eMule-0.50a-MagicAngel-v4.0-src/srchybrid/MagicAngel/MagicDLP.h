

#pragma once



class CMagicDLP
{ 
public: 
	CMagicDLP(CString appdir_in);
	~CMagicDLP();
	bool IsMagicDLPavailable()		{return Magicdlpavailable;}
	void Reload();

	typedef DWORD (__cdecl *GETMAGICDLPVERSION)();
	GETMAGICDLPVERSION GetMagicDLPVersion;

	typedef LPCTSTR (__cdecl *DLPCHECKMODSTRING_GPL)(LPCTSTR modversion, LPCTSTR clientversion);
	DLPCHECKMODSTRING_GPL DLPCheckModstring_GPL;
	typedef LPCTSTR (__cdecl *DLPCHECKUSERNAME_GPL)(LPCTSTR username);
	DLPCHECKUSERNAME_GPL DLPCheckUsername_GPL;

	typedef LPCTSTR (__cdecl *DLPCHECKMODSTRING_HARD)(LPCTSTR modversion, LPCTSTR clientversion);
	DLPCHECKMODSTRING_HARD DLPCheckModstring_Hard;
	typedef LPCTSTR (__cdecl *DLPCHECKMODSTRING_SOFT)(LPCTSTR modversion, LPCTSTR clientversion);
	DLPCHECKMODSTRING_SOFT DLPCheckModstring_Soft;

	typedef LPCTSTR (__cdecl *DLPCHECKUSERNAME_HARD)(LPCTSTR username);
	DLPCHECKUSERNAME_HARD DLPCheckUsername_Hard;
	typedef LPCTSTR (__cdecl *DLPCHECKUSERNAME_SOFT)(LPCTSTR username);
	DLPCHECKUSERNAME_SOFT DLPCheckUsername_Soft;

	typedef LPCTSTR (__cdecl *DLPCHECKMODSTRING_BAD)(LPCTSTR modversion, LPCTSTR clientversion);
	DLPCHECKMODSTRING_BAD DLPCheckModstring_Bad;

	//typedef void (WINAPI*TESTFUNC)();
	//TESTFUNC testfunc;

	

private:
	HINSTANCE MagicdlpInstance;
	bool	Magicdlpavailable;
	CString appdir;
};