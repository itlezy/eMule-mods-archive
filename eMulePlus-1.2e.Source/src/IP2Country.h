// by Superlexx, based on IPFilter by Bouc7
#pragma once
#include "Loggable.h"
#include <atlcoll.h>

// N/A flag/country has zero index
#define NO_COUNTRY_INFO		0

class CIP2Country : public CLoggable
{
public:
	CIP2Country(void);
	~CIP2Country(void);

	void	Load(bool bReset = true);
	void	Unload(bool bReset = true);

	//refresh passive windows
	void	Refresh();

	bool	IsIP2Country() const			{return m_bEnableIP2Country;}
	bool	ShowCountryFlag() const			{return m_bEnableCountryFlag;}

	CString	GetCountryNameByIndex(uint16 uCountryIndex);

	uint16	CIP2Country::GetCountryFromIP(uint32 dwClientIP);

	CImageList *GetFlagImageList()			{return &m_lstCountryFlagImage;}

private:
	struct IPRange_Struct
	{
		uint32		dwIPend;
		uint16		uIndex;
	};
//	Reset ip2country reference
	void	Reset();

	CImageList	m_lstCountryFlagImage;

	bool	m_bEnableIP2Country;
	bool	m_bEnableCountryFlag;
	CRBMap<uint32, IPRange_Struct> m_rbmapIpList;
	CArray<CString> m_astrCountryName;
};
