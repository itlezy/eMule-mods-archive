//EastShare Start - added by AndCycle, IP to Country

// by Superlexx, based on IPFilter by Bouc7
#pragma once
#include <vector>
#include "map_inc.h"

//Code Improvements by Xman
struct Country_Struct {
	//CString			ShortCountryName;
	//CString			MidCountryName;
	CString			LongCountryName;
	WORD			FlagIndex;
};
//Xman Xtreme only uses the long names

struct Location_Struct { // X: [IP2L] - [IP2Location]
	Country_Struct*	country;
	CString			locationName;
};

struct IPRange_Struct2{
	uint32          IPstart;
	uint32          IPend;
	Location_Struct*location; // X: [IP2L] - [IP2Location]
	IPRange_Struct2() {  }
};


#define DFLT_IP2COUNTRY_FILENAME  _T("ip-to-country.csv")

typedef std::vector<IPRange_Struct2> CIP2CountryArray;
typedef std::vector<Country_Struct*> CCountryArray;// X: [IP2L] - [IP2Location]

class CIP2Country
{
	public:
		CIP2Country();
		~CIP2Country();
		
		void	Load();
		void	Unload();

		//reset ip2country referense
		//void	Reset();

		//refresh passive windows
		void	Refresh();

		bool	IsIP2Country() const			{return EnableIP2Country;}
		bool	ShowCountryFlag() const;

		//Country_Struct*	GetDefaultIP2Country() {return &defaultCountry;}

		bool	LoadFromFile();
		bool	LoadCountryFlagLib();
		void	RemoveAllIPs();
		void	RemoveAllFlags();

		void	AddIPRange(uint32 IPfrom,uint32 IPto, const LPSTR shortCountryName/*, const CHAR* midCountryName*/, const LPSTR longCountryName);

		Location_Struct*	GetLocationFromIP(uint32 IP) const;// X: [IP2L] - [IP2Location]
		CString&	GetCountryNameFromRef(Country_Struct* m_structServerCountry/*, bool longname=false*/);
		//WORD	GetFlagResIDfromCountryCode(CString shortCountryName);

		CImageList* GetFlagImageList() const {return &CountryFlagImageList;}
		CString GetDefaultFilePath() const;
		static 	Country_Struct defaultCountry;// X: [AL] - [Additional Localize] move up
		static 	Location_Struct defaultLocation;// X: [IP2L] - [IP2Location]
	private:
		Country_Struct*	AddCountry(const LPSTR shortCountryName, const LPSTR longCountryName);// X: [IP2L] - [IP2Location]

		//check is program current running, if it's under init or shutdown, set to false
		//bool	m_bRunning;

		HINSTANCE _hCountryFlagDll;
		static CImageList	CountryFlagImageList;

		CIP2CountryArray m_iplist;
		CCountryArray m_countryList;// X: [IP2L] - [IP2Location]
		MAP<LPSTR, Location_Struct*, LPSTR_Pred> locationList;// X: [SUL] - [SpeedUpLoading]
		MAP<LPSTR, uint16, LPSTR_Pred>	CountryIDtoFlagIndex;

		bool	EnableIP2Country;
		bool	EnableCountryFlag;
};

