//EastShare Start - added by AndCycle, IP to Country

// by Superlexx, based on IPFilter by Bouc7
#pragma once
#include <atlcoll.h>

//Code Improvements by Xman
struct Country_Struct {
	//CString			ShortCountryName;
	//CString			MidCountryName;
	CString			LongCountryName;
	WORD			FlagIndex;
};
//Xman Xtreme only uses the long names

struct IPRange_Struct2{
	uint32          IPstart;
	uint32          IPend;
	Country_Struct*	country;
	IPRange_Struct2() {  }
};


#define DFLT_IP2COUNTRY_FILENAME  _T("ip-to-country.csv")//Commander - Added: IP2Country auto-updating

typedef CTypedPtrArray<CPtrArray, IPRange_Struct2*> CIP2CountryArray;

class CIP2Country
{
	public:
		CIP2Country(void);
		~CIP2Country(void);
		
		void	Load();
		void	Unload();

		//reset ip2country referense
		//void	Reset();
		void	Reset(); // Advanced Updates [MorphXT/Stulle] - Stulle

		//refresh passive windows
		void	Refresh();

		bool	IsIP2Country()			{return EnableIP2Country;}
		bool	ShowCountryFlag() const;

		Country_Struct*	GetDefaultIP2Country() {return &defaultCountry;}

		bool	LoadFromFile();
		bool	LoadCountryFlagLib();
		void	RemoveAllIPs();
		void	RemoveAllFlags();

		void	AddIPRange(uint32 IPfrom,uint32 IPto, TCHAR* shortCountryName, TCHAR* midCountryName, TCHAR* longCountryName);

		Country_Struct*	GetCountryFromIP(uint32 IP) const;
		CString	GetCountryNameFromRef(Country_Struct* m_structServerCountry, bool longname=false);
		//WORD	GetFlagResIDfromCountryCode(CString shortCountryName);

		CImageList* GetFlagImageList() const {return &CountryFlagImageList;}
		IMAGELISTDRAWPARAMS GetFlagImageDrawParams(CDC* dc,int iIndex,POINT point) const;
		//void    UpdateIP2CountryURL();//Commander - Added: IP2Country auto-updating
		CString GetDefaultFilePath() const;
	private:

		//check is program current running, if it's under init or shutdown, set to false
		bool	m_bRunning;

		HINSTANCE _hCountryFlagDll;
		static CImageList	CountryFlagImageList;

		bool	EnableIP2Country;
		bool	EnableCountryFlag;
		static 	Country_Struct defaultCountry;

		CIP2CountryArray m_iplist;
		CRBMap<CString, Country_Struct*> countryList;
		CRBMap<CString, uint16>	CountryIDtoFlagIndex;

		// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	public:
		void    UpdateIP2CountryURL();
		// <== Advanced Updates [MorphXT/Stulle] - Stulle
};

