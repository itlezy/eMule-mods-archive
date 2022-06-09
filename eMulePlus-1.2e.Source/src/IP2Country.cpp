/*
the IP to country data is provided by http://ip-to-country.webhosting.info/

"IP2Country uses the IP-to-Country Database
 provided by WebHosting.Info (http://www.webhosting.info),
 available from http://ip-to-country.webhosting.info."
*/

/*
flags are from http://sf.net/projects/flags/
*/

// by Superlexx, based on IPFilter by Bouc7

#include "stdafx.h"
#include "IP2Country.h"
#include "emule.h"
#include "otherfunctions.h"
#include "flag/resource.h"

//refresh list
#include "ServerList.h"
#include "ClientList.h"

//refresh server list ctrl
#include "emuleDlg.h"
#include "ServerWnd.h"
#include "ServerListCtrl.h"
#include <share.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COUNTRY_FLAG_DLL_NAME		_T("countryflag.dll")

static void FirstCharCap(CString *pstrTarget)
{
	pstrTarget->TrimRight(); //remove whitespaces at the end (including carriage return)
	if (!pstrTarget->IsEmpty())
	{
		TCHAR	*pcBuf = const_cast<TCHAR*>(pstrTarget->GetString());
		TCHAR	acCh[2];

		acCh[1] = _T('\0');
		pstrTarget->MakeLower();
		for (int iIdx = 0;;)
		{
			acCh[0] = pcBuf[iIdx];
			_tcsupr(acCh);	// convert string (first word's char) to the upper case
			pcBuf[iIdx] = acCh[0];
			iIdx = pstrTarget->Find(_T(' '), iIdx) + 1;
			if (iIdx <= 0)
				break;
		}
	}
}

CIP2Country::CIP2Country() : m_rbmapIpList(144)
{
	m_bEnableIP2Country = false;
	m_bEnableCountryFlag = false;

	if (g_App.m_pPrefs->GetShowCountryFlag())
	{
		AddLogLine(LOG_FL_DBG | LOG_FL_EMBEDFMT, _T("IP-to-Country Database provided by ") RGB_STEEL_BLUE_TXT _T("WEBH") RGB_DARK_ORANGE_TXT _T("O") RGB_STEEL_BLUE_TXT _T("STING") RGB_DARK_ORANGE_TXT _T(".INFO") RGB_DEFAULT_TXT _T(", available from http://ip-to-country.webhosting.info."));
		Load(false);
	}
}

CIP2Country::~CIP2Country()
{
	Unload(false);
}

void CIP2Country::Load(bool bReset)
{
	static const uint16	s_auResID[] =
	{
		IDI_FLAG_AD, IDI_FLAG_AE, IDI_FLAG_AF, IDI_FLAG_AG, IDI_FLAG_AI, IDI_FLAG_AL,
		IDI_FLAG_AM, IDI_FLAG_AN, IDI_FLAG_AO, IDI_NO_FLAG, IDI_FLAG_AR, IDI_FLAG_AS,
		IDI_FLAG_AT, IDI_FLAG_AU, IDI_FLAG_AW, IDI_FLAG_AX, IDI_FLAG_AZ, IDI_FLAG_BA,
		IDI_FLAG_BB, IDI_FLAG_BD, IDI_FLAG_BE, IDI_FLAG_BF, IDI_FLAG_BG, IDI_FLAG_BH,
		IDI_FLAG_BI, IDI_FLAG_BJ, IDI_FLAG_BM, IDI_FLAG_BN, IDI_FLAG_BO, IDI_FLAG_BR,
		IDI_FLAG_BS, IDI_FLAG_BT, IDI_FLAG_NO, IDI_FLAG_BW, IDI_FLAG_BY, IDI_FLAG_BZ,
		IDI_FLAG_CA, IDI_FLAG_CC, IDI_FLAG_CD, IDI_FLAG_CF, IDI_FLAG_CG, IDI_FLAG_CH,
		IDI_FLAG_CI, IDI_FLAG_CK, IDI_FLAG_CL, IDI_FLAG_CM, IDI_FLAG_CN, IDI_FLAG_CO,
		IDI_FLAG_CR, IDI_FLAG_YU, IDI_FLAG_CU, IDI_FLAG_CV, IDI_FLAG_CX, IDI_FLAG_CY,
		IDI_FLAG_CZ, IDI_FLAG_DE, IDI_FLAG_DJ, IDI_FLAG_DK, IDI_FLAG_DM, IDI_FLAG_DO,
		IDI_FLAG_DZ, IDI_FLAG_EC, IDI_FLAG_EE, IDI_FLAG_EG, IDI_FLAG_EH, IDI_FLAG_ER,
		IDI_FLAG_ES, IDI_FLAG_ET, IDI_FLAG_FI, IDI_FLAG_FJ, IDI_FLAG_FK, IDI_FLAG_FM,
		IDI_FLAG_FO, IDI_FLAG_FR, IDI_FLAG_GA, IDI_FLAG_GB, IDI_FLAG_GD, IDI_FLAG_GE,
		IDI_FLAG_FR, IDI_FLAG_GG, IDI_FLAG_GH, IDI_FLAG_GI, IDI_FLAG_GL, IDI_FLAG_GM,
		IDI_FLAG_GN, IDI_FLAG_GP, IDI_FLAG_GQ, IDI_FLAG_GR, IDI_FLAG_GS, IDI_FLAG_GT,
		IDI_FLAG_GU, IDI_FLAG_GW, IDI_FLAG_GY, IDI_FLAG_HK, IDI_FLAG_HN, IDI_FLAG_HR,
		IDI_FLAG_HT, IDI_FLAG_HU, IDI_FLAG_ID, IDI_FLAG_IE, IDI_FLAG_IL, IDI_FLAG_IM,
		IDI_FLAG_IN, IDI_FLAG_IO, IDI_FLAG_IQ, IDI_FLAG_IR, IDI_FLAG_IS, IDI_FLAG_IT,
		IDI_FLAG_JE, IDI_FLAG_JM, IDI_FLAG_JO, IDI_FLAG_JP, IDI_FLAG_KE, IDI_FLAG_KG,
		IDI_FLAG_KH, IDI_FLAG_KI, IDI_FLAG_KM, IDI_FLAG_KN, IDI_FLAG_KP, IDI_FLAG_KR,
		IDI_FLAG_KW, IDI_FLAG_KY, IDI_FLAG_KZ, IDI_FLAG_LA, IDI_FLAG_LB, IDI_FLAG_LC,
		IDI_FLAG_LI, IDI_FLAG_LK, IDI_FLAG_LR, IDI_FLAG_LS, IDI_FLAG_LT, IDI_FLAG_LU,
		IDI_FLAG_LV, IDI_FLAG_LY, IDI_FLAG_MA, IDI_FLAG_MC, IDI_FLAG_MD, IDI_FLAG_ME,
		IDI_FLAG_MG, IDI_FLAG_MH, IDI_FLAG_MK, IDI_FLAG_ML, IDI_FLAG_MM, IDI_FLAG_MN,
		IDI_FLAG_MO, IDI_FLAG_MP, IDI_FLAG_MQ, IDI_FLAG_MR, IDI_FLAG_MS, IDI_FLAG_MT,
		IDI_FLAG_MU, IDI_FLAG_MV, IDI_FLAG_MW, IDI_FLAG_MX, IDI_FLAG_MY, IDI_FLAG_MZ,
		IDI_FLAG_NA, IDI_FLAG_FR, IDI_FLAG_NE, IDI_FLAG_NF, IDI_FLAG_NG, IDI_FLAG_NI,
		IDI_FLAG_NL, IDI_FLAG_NO, IDI_FLAG_NP, IDI_FLAG_NR, IDI_FLAG_NU, IDI_FLAG_NZ,
		IDI_FLAG_OM, IDI_FLAG_PA, IDI_FLAG_PE, IDI_FLAG_PF, IDI_FLAG_PG, IDI_FLAG_PH,
		IDI_FLAG_PK, IDI_FLAG_PL, IDI_FLAG_PM, IDI_FLAG_PN, IDI_FLAG_PR, IDI_FLAG_PS,
		IDI_FLAG_PT, IDI_FLAG_PW, IDI_FLAG_PY, IDI_FLAG_QA, IDI_FLAG_FR, IDI_FLAG_RO,
		IDI_FLAG_RS, IDI_FLAG_RU, IDI_FLAG_RW, IDI_FLAG_SA, IDI_FLAG_SB, IDI_FLAG_SC,
		IDI_FLAG_SD, IDI_FLAG_SE, IDI_FLAG_SG, IDI_FLAG_SH, IDI_FLAG_SI, IDI_FLAG_SK,
		IDI_FLAG_SL, IDI_FLAG_SM, IDI_FLAG_SN, IDI_FLAG_SO, IDI_FLAG_SR, IDI_FLAG_ST,
		IDI_FLAG_SV, IDI_FLAG_SY, IDI_FLAG_SZ, IDI_FLAG_TC, IDI_FLAG_TD, IDI_FLAG_TF,
		IDI_FLAG_TG, IDI_FLAG_TH, IDI_FLAG_TJ, IDI_FLAG_TK, IDI_FLAG_TL, IDI_FLAG_TM,
		IDI_FLAG_TN, IDI_FLAG_TO, IDI_FLAG_TR, IDI_FLAG_TT, IDI_FLAG_TV, IDI_FLAG_TW,
		IDI_FLAG_TZ, IDI_FLAG_UA, IDI_FLAG_UG, IDI_FLAG_US, IDI_FLAG_US, IDI_FLAG_UY,
		IDI_FLAG_UZ, IDI_FLAG_VA, IDI_FLAG_VC, IDI_FLAG_VE, IDI_FLAG_VG, IDI_FLAG_VI,
		IDI_FLAG_VN, IDI_FLAG_VU, IDI_FLAG_WF, IDI_FLAG_WS, IDI_FLAG_YE, IDI_FLAG_FR,
		IDI_FLAG_ZA, IDI_FLAG_ZM, IDI_FLAG_ZW
	};
	static const uint16 s_auCountryID[] =
	{
		'AD', 'AE', 'AF', 'AG', 'AI', 'AL',
		'AM', 'AN', 'AO', 'AQ', 'AR', 'AS',
		'AT', 'AU', 'AW', 'AX', 'AZ', 'BA',
		'BB', 'BD', 'BE', 'BF', 'BG', 'BH',
		'BI', 'BJ', 'BM', 'BN', 'BO', 'BR',
		'BS', 'BT', 'BV', 'BW', 'BY', 'BZ',
		'CA', 'CC', 'CD', 'CF', 'CG', 'CH',
		'CI', 'CK', 'CL', 'CM', 'CN', 'CO',
		'CR', 'CS', 'CU', 'CV', 'CX', 'CY',
		'CZ', 'DE', 'DJ', 'DK', 'DM', 'DO',
		'DZ', 'EC', 'EE', 'EG', 'EH', 'ER',
		'ES', 'ET', 'FI', 'FJ', 'FK', 'FM',
		'FO', 'FR', 'GA', 'GB', 'GD', 'GE',
		'GF', 'GG', 'GH', 'GI', 'GL', 'GM',
		'GN', 'GP', 'GQ', 'GR', 'GS', 'GT',
		'GU', 'GW', 'GY', 'HK', 'HN', 'HR',
		'HT', 'HU', 'ID', 'IE', 'IL', 'IM',
		'IN', 'IO', 'IQ', 'IR', 'IS', 'IT',
		'JE', 'JM', 'JO', 'JP', 'KE', 'KG',
		'KH', 'KI', 'KM', 'KN', 'KP', 'KR',
		'KW', 'KY', 'KZ', 'LA', 'LB', 'LC',
		'LI', 'LK', 'LR', 'LS', 'LT', 'LU',
		'LV', 'LY', 'MA', 'MC', 'MD', 'ME',
		'MG', 'MH', 'MK', 'ML', 'MM', 'MN',
		'MO', 'MP', 'MQ', 'MR', 'MS', 'MT',
		'MU', 'MV', 'MW', 'MX', 'MY', 'MZ',
		'NA', 'NC', 'NE', 'NF', 'NG', 'NI',
		'NL', 'NO', 'NP', 'NR', 'NU', 'NZ',
		'OM', 'PA', 'PE', 'PF', 'PG', 'PH',
		'PK', 'PL', 'PM', 'PN', 'PR', 'PS',
		'PT', 'PW', 'PY', 'QA', 'RE', 'RO',
		'RS', 'RU', 'RW', 'SA', 'SB', 'SC',
		'SD', 'SE', 'SG', 'SH', 'SI', 'SK',
		'SL', 'SM', 'SN', 'SO', 'SR', 'ST',
		'SV', 'SY', 'SZ', 'TC', 'TD', 'TF',
		'TG', 'TH', 'TJ', 'TK', 'TL', 'TM',
		'TN', 'TO', 'TR', 'TT', 'TV', 'TW',
		'TZ', 'UA', 'UG', 'UM', 'US', 'UY',
		'UZ', 'VA', 'VC', 'VE', 'VG', 'VI',
		'VN', 'VU', 'WF', 'WS', 'YE', 'YT',
		'ZA', 'ZM', 'ZW'
	};

//	Protect from consecutive loadings
	if (m_bEnableIP2Country)
		return;

//	Temporary resource just for fast lookup
	CRBMap<uint32, uint32>	rbmapCountryIDtoFlagIndex(ARRSIZE(s_auResID));
	CRBMap<uint32, uint32>::CPair	*pPair;

	for (unsigned ui = 0; ui < ARRSIZE(s_auResID); ui++)
	{
		rbmapCountryIDtoFlagIndex.SetAt(
			((s_auCountryID[ui] & 0xFF00) << 8) | (s_auCountryID[ui] & 0xFF), s_auResID[ui] );
	}

	FILE	*pReadFile;
	CString	strIP2CountryCSVfile = g_App.m_pPrefs->GetConfigDir();

	strIP2CountryCSVfile += _T("ip-to-country.csv");

	if ((pReadFile = _tfsopen(strIP2CountryCSVfile, _T("r"), _SH_DENYWR)) != NULL)
	{
		TCHAR			acBuffer[512];
		CString			strBuffer, strTemp[5];
		unsigned		uiLineNum = 0;
		HINSTANCE		hCountryFlagDll;
		IPRange_Struct	ipRangeData;
		CStringList		countryNameTempList;
		uint32			dwIdx, dwKey;
		HICON			hIcon;
		bool			bEnableCountryFlag, bEnableIP2Country;

		setvbuf(pReadFile, NULL, _IOFBF, 64*1024);

		bEnableCountryFlag = true;
		if ((hCountryFlagDll = ::LoadLibrary(COUNTRY_FLAG_DLL_NAME)) == NULL)
		{
			bEnableCountryFlag = false;
	
			AddLogLine(LOG_RGB_ERROR, IDS_ERR_FILEOPEN2, CString(g_App.m_pPrefs->GetAppDir() + COUNTRY_FLAG_DLL_NAME));
		}
		else
		{
			m_lstCountryFlagImage.Create(18, 12, g_App.m_iDfltImageListColorFlags | ILC_MASK, 200, 1);
			m_lstCountryFlagImage.SetBkColor(CLR_NONE);

		//	Load flag for unknown country
			hIcon = reinterpret_cast<HICON>(::LoadImage(hCountryFlagDll, MAKEINTRESOURCE(IDI_NO_FLAG), IMAGE_ICON, 0, 0, 0));
			if (hIcon == NULL)
			{
				bEnableCountryFlag = false;
				AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Invalid ResID, maybe upgrade is required ") COUNTRY_FLAG_DLL_NAME);
			}
			m_lstCountryFlagImage.Add(hIcon);
			::DestroyIcon(hIcon);
		}
	//	Add unknown country name
		countryNameTempList.AddTail(_T("Not Applicable"));

		bEnableIP2Country = true;
		while (feof(pReadFile) == 0)
		{
			if (_fgetts(acBuffer, ARRSIZE(acBuffer), pReadFile) == 0)
				break;
			strBuffer = acBuffer;
			uiLineNum++;
			/*
				http://ip-to-country.webhosting.info/node/view/54

				This is a sample of how the CSV file is structured:

				"0033996344","0033996351","GB","GBR","UNITED KINGDOM"
				"0050331648","0083886079","US","USA","UNITED STATES"
				"0094585424","0094585439","SE","SWE","SWEDEN"

				FIELD  			DATA TYPE		  	FIELD DESCRIPTION
				IP_FROM 		NUMERICAL (DOUBLE) 	Beginning of IP address range.
				IP_TO			NUMERICAL (DOUBLE) 	Ending of IP address range.
				COUNTRY_CODE2 	CHAR(2)				Two-character country code based on ISO 3166.
				COUNTRY_CODE3 	CHAR(3)				Three-character country code based on ISO 3166.
				COUNTRY_NAME 	VARCHAR(50) 		Country name based on ISO 3166
			*/
		//	As full country name is at the end of the string,
		//	'\n' trimming is not required here, as it's done inside FirstCharCap()
		//	by TrimRight() which removes all whitespaces including carriage return
			//strBuffer.TrimRight(_T('\n'));
			strBuffer.Remove(_T('"')); // get rid of the " signs

			int iCurPos = 0;

			for (unsigned ui = 0; ui < ARRSIZE(strTemp); ui++)
			{
				strTemp[ui] = strBuffer.Tokenize(_T(","), iCurPos);
				if (strTemp[ui].IsEmpty())
				{
					bEnableIP2Country = false;
					AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T(__FUNCTION__) _T(": %s error (line: %u)"), strIP2CountryCSVfile, uiLineNum);
					break;
				}
			}
			if (!bEnableIP2Country)
				break;
			
			ipRangeData.dwIPend = _tstoi(strTemp[1]);
			ipRangeData.uIndex = NO_COUNTRY_INFO;

			if (strTemp[2].GetLength() == 2)	// 2 character country code
			{
				dwKey = strTemp[2].GetString()[0] << 16 | strTemp[2].GetString()[1];
				pPair = rbmapCountryIDtoFlagIndex.Lookup(dwKey);	// short country name
			}
			else
				pPair = NULL;
			if (pPair != NULL)
			{
				if ((dwIdx = (pPair->m_value >> 16)) == NO_COUNTRY_INFO)
				{
				//	Country name and flag weren't added yet

					if (bEnableCountryFlag)
					{
						hIcon = reinterpret_cast<HICON>(::LoadImage(hCountryFlagDll, MAKEINTRESOURCE(pPair->m_value & 0xffff), IMAGE_ICON, 0, 0, 0));
						if (hIcon == NULL)
						{
							bEnableCountryFlag = false;
							AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Invalid ResID, maybe upgrade is required ") COUNTRY_FLAG_DLL_NAME);
							break;
						}
						m_lstCountryFlagImage.Add(hIcon);
						::DestroyIcon(hIcon);
					}
				//	Capitalize words in country name
					FirstCharCap(&strTemp[4]);

					dwIdx = countryNameTempList.GetCount();
					countryNameTempList.AddTail(strTemp[4]);

				//	Associate flag/country index with short country name
					pPair->m_value |= (dwIdx << 16);
				}
				ipRangeData.uIndex = static_cast<uint16>(dwIdx);
			}
			else
			{
				AddLogLine( LOG_FL_DBG | LOG_RGB_WARNING, _T("Unsupported country name found %s/%s (line: %u)"),
								 strTemp[2], strTemp[4].TrimRight(), uiLineNum );
			}
		//	Start of IP range is a key
			m_rbmapIpList.SetAt(_tstoi(strTemp[0]), ipRangeData);
		}	//while
		fclose(pReadFile);

		if (m_rbmapIpList.IsEmpty())
			bEnableIP2Country = false;

		if (!bEnableIP2Country)
		{
			bEnableCountryFlag = false;
			m_rbmapIpList.RemoveAll();
		}
		else
		{
		//	Allocate array memory
			m_astrCountryName.SetSize(countryNameTempList.GetCount());

			POSITION pos;

			for (pos = countryNameTempList.GetHeadPosition(), dwIdx = 0; pos != NULL; dwIdx++)
			{
				m_astrCountryName[dwIdx] = countryNameTempList.GetNext(pos);
			}
			m_bEnableIP2Country = bEnableIP2Country;
			m_bEnableCountryFlag = bEnableCountryFlag;

			if (bReset)
				Reset();
		}
		if (hCountryFlagDll != NULL)
		{
			if (!bEnableCountryFlag)
			{
			//	Destory image list
				m_lstCountryFlagImage.DeleteImageList();
			}
			::FreeLibrary(hCountryFlagDll);
		}
	}
	else
		AddLogLine(LOG_RGB_ERROR, IDS_ERR_FILEOPEN2, strIP2CountryCSVfile);
}

void CIP2Country::Unload(bool bReset)
{
//	Protect from consecutive unloadings
	if (!m_bEnableIP2Country)
		return;

	m_bEnableIP2Country = false;
	m_bEnableCountryFlag = false;

	if (bReset)
		Reset();

//	Release resources	
	m_astrCountryName.RemoveAll();
	m_rbmapIpList.RemoveAll();
//	Destory image list
	m_lstCountryFlagImage.DeleteImageList();
}

void CIP2Country::Reset()
{
	g_App.m_pServerList->ResetIP2Country();
	g_App.m_pClientList->ResetIP2Country();
}

void CIP2Country::Refresh()
{
	g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshAllServer();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetCountryFromIP() obtains country information based on IP.
//		Params:
//			dwClientIP - IP address.
//		Return:
//			index of Country Name and flag.
uint16 CIP2Country::GetCountryFromIP(uint32 dwClientIP)
{
	if (m_bEnableIP2Country && (dwClientIP != 0))
	{
		dwClientIP = fast_htonl(dwClientIP);

		POSITION pos = m_rbmapIpList.FindFirstKeyAfter(dwClientIP);

		if (pos == NULL)
			pos = m_rbmapIpList.GetTailPosition();
		else
			m_rbmapIpList.GetPrev(pos);

		while(pos)
		{
			const CRBMap<uint32, IPRange_Struct>::CPair *pPair = m_rbmapIpList.GetPrev(pos);

			if (dwClientIP > pPair->m_value.dwIPend)
				break;
			if (dwClientIP >= pPair->m_key)
				return pPair->m_value.uIndex;
		}
	}
	return NO_COUNTRY_INFO;
}

CString CIP2Country::GetCountryNameByIndex(uint16 uCountryIndex)
{
	return (uCountryIndex < m_astrCountryName.GetSize()) ? m_astrCountryName[uCountryIndex] : CString(_T(""));
}
