
// by Superlexx, based on IPFilter by Bouc7
#pragma once
#include <map>
#include <vector>
using namespace std;
#define NO_FLAG 0 // N/A flag is the first Res, so it should at index zero

struct IPRange_Struct2{
	uint32          IPstart;
	uint32          IPend;
	CString			ShortCountryName;
	CString			MidCountryName;
	CString			LongCountryName;
	WORD			FlagIndex;
};

enum IP2CountryNameSelection
{
	IP2CountryName_SHORT = 0,
	IP2CountryName_MID,
	IP2CountryName_LONG
};

#define DFLT_IP2COUNTRY_FILENAME  _T("ip-to-country.csv")

typedef std::vector<IPRange_Struct2*> CIP2CountryArray;

class CIP2Country
{
public:
	CIP2Country();
	~CIP2Country();

	void	Load();
	void	Unload();

	//reset ip2country reference
	void	Reset();

	//refresh passive windows
	void	Refresh();

	bool	IsIP2Country() const			{return m_bEnableIP2Country;}
	bool	ShowCountryFlag() const;
	bool	LoadFromFile();
	bool	LoadCountryFlagLib();
	void	RemoveAllIPs();
	void	RemoveAllFlags();

	void	AddIPRange(uint32 IPfrom,uint32 IPto, CString& shortCountryName, CString& midCountryName, CString& longCountryName);

	IPRange_Struct2*	GetCountryFromIP(uint32 IP);
	CString	GetCountryNameFromRef(IPRange_Struct2* m_structServerCountry, bool longname=false);
	WORD	GetFlagResIDfromCountryCode(CString shortCountryName);

	CImageList* GetFlagImageList() {return &CountryFlagImageList;}
	//void    UpdateIP2CountryURL();//Commander - Added: IP2Country auto-updating
	CString GetDefaultFilePath() const;

private:
	//check is program current running, if it's under init or shutdown, set to false
	bool	m_bRunning;

	HINSTANCE _hCountryFlagDll;
	CImageList	CountryFlagImageList;

	bool	m_bEnableIP2Country;
	bool	m_bEnableCountryFlag;
	struct	IPRange_Struct2 defaultIP2Country;

	CIP2CountryArray m_iplist;
#ifdef _DEBUG  
	struct flaginfostruct  
	{  
		flaginfostruct(const uint16 index)	{flagindex = index; usedcount = 0;}  
		uint16	flagindex;  
		UINT	usedcount;  
	};  
	CRBMap<CString, flaginfostruct>	CountryIDtoFlagIndex;  
#else  
	CRBMap<CString, uint16>	CountryIDtoFlagIndex;  
#endif
};
