//EastShare Start - added by AndCycle, IP to Country

/*
the IP to country data is provided by http://ip-to-country.webhosting.info/

"IP2Country uses the IP-to-Country Database
 provided by WebHosting.Info (http://www.webhosting.info),
 available from http://ip-to-country.webhosting.info."

 */

// by Superlexx, based on IPFilter by Bouc7

#include "StdAfx.h"
#include <share.h>
#include "IP2Country.h"
#include "emule.h"
#include "otherfunctions.h"
#include <flag/resource.h>
#include "Log.h"
#include "Preferences.h" //Xman

//refresh list
//#include "serverlist.h"
//#include "clientlist.h"

//refresh server list ctrl
//#include "emuledlg.h"
//#include "serverwnd.h"
//#include "serverlistctrl.h"

//#include "HttpDownloadDlg.h"//MORPH - Added by SiRoB, IP2Country auto-updating
//#include "ZipFile.h"//MORPH - Added by SiRoB, ZIP File download decompress
// ==> Advanced Updates [MorphXT/Stulle] - Stulle
#include "HttpDownloadDlg.h"
#include "ZipFile.h"
#include "serverlist.h"
#include "clientlist.h"
// <== Advanced Updates [MorphXT/Stulle] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// N/A flag is the first Res, so it should at index zero
#define NO_FLAG 0

CImageList CIP2Country::CountryFlagImageList;
Country_Struct CIP2Country::defaultCountry;

void FirstCharCap(CString *pstrTarget)
{
	pstrTarget->TrimRight();//clean out the space at the end, prevent exception for index++
	if(!pstrTarget->IsEmpty())
	{
		pstrTarget->MakeLower();
		for (int iIdx = 0;;)
		{
			pstrTarget->SetAt(iIdx, pstrTarget->Mid(iIdx, 1).MakeUpper().GetAt(0));
			iIdx = pstrTarget->Find(_T(' '), iIdx) + 1;
			if (iIdx == 0)
				break;
		}
	}
}

CIP2Country::CIP2Country(){

	m_bRunning = false;

	//defaultCountry.ShortCountryName = GetResString(IDS_IP2COUNTRY_NASHORT);
	//defaultCountry.MidCountryName = GetResString(IDS_IP2COUNTRY_NASHORT);
	defaultCountry.LongCountryName = GetResString(IDS_IP2COUNTRY_NALONG);

	defaultCountry.FlagIndex = NO_FLAG;

	EnableIP2Country = false;
	EnableCountryFlag = false;

	//Xman always load
	//if(thePrefs.GetIP2CountryNameMode() != IP2CountryName_DISABLE || 
	//	thePrefs.IsIP2CountryShowFlag()){
		Load();
	//}

	AddLogLine(false, GetResString(IDS_IP2COUNTRY_MSG1));
	AddLogLine(false, GetResString(IDS_IP2COUNTRY_MSG2));
	m_bRunning = true;
}

CIP2Country::~CIP2Country(){

	m_bRunning = false;

	Unload();
}

void CIP2Country::Load(){

	EnableCountryFlag = LoadCountryFlagLib();//flag lib first, so ip range can map to flag
	EnableIP2Country = LoadFromFile();

	//if(m_bRunning) Reset(); //Xman not needed
	if(m_bRunning) Reset(); // Advanced Updates [MorphXT/Stulle] - Stulle
	CountryIDtoFlagIndex.RemoveAll(); //Xman after loading /mapping we don't need it anymore

	AddLogLine(false, GetResString(IDS_IP2COUNTRY_LOADED));
}

void CIP2Country::Unload(){

	EnableIP2Country = false;
	EnableCountryFlag = false;

	//if(m_bRunning) Reset(); //Xman not needed
	if(m_bRunning) Reset(); // Advanced Updates [MorphXT/Stulle] - Stulle

	RemoveAllIPs();
	RemoveAllFlags();

	AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_UNLOADED));
}

/* //Xman not needed
Advanced Updates [MorphXT/Stulle] - Stulle */
void CIP2Country::Reset(){
	theApp.serverlist->ResetIP2Country();
	theApp.clientlist->ResetIP2Country();
}

/* Advanced Updates [MorphXT/Stulle] - Stulle
void CIP2Country::Refresh(){
	theApp.emuledlg->serverwnd->serverlistctrl.RefreshAllServer();
}
*/
static int __cdecl CmpIP2CountryByStartAddr(const void* p1, const void* p2)
{
	const IPRange_Struct2* rng1 = *(IPRange_Struct2**)p1;
	const IPRange_Struct2* rng2 = *(IPRange_Struct2**)p2;
	return CompareUnsigned(rng1->IPstart, rng2->IPstart);
}

bool CIP2Country::LoadFromFile(){
	DWORD startMesure = GetTickCount();
	TCHAR* szbuffer = new TCHAR[88];
	CString ip2countryCSVfile = GetDefaultFilePath();
	FILE* readFile = _tfsopen(ip2countryCSVfile, _T("r"), _SH_DENYWR);
	try{
		if (readFile != NULL) {
			int iCount = 0;
			int iLine = 0;
			int iDuplicate = 0;
			int iMerged = 0;
			bool error = false;
			TCHAR *szIPStart,*szIPEnd,*sz2L,*sz3L,*szCountry;
			/*
			http://ip-to-country.webhosting.info/node/view/54

			This is a sample of how the CSV file is structured:

			0033996344,0033996351,GB,GBR,"UNITED KINGDOM"
			"0050331648","0083886079","US","USA","UNITED STATES"
			"0094585424","0094585439","SE","SWE","SWEDEN"

			FIELD  			DATA TYPE		  	FIELD DESCRIPTION
			IP_FROM 		NUMERICAL (DOUBLE) 	Beginning of IP address range.
			IP_TO			NUMERICAL (DOUBLE) 	Ending of IP address range.
			COUNTRY_CODE2 	CHAR(2)				Two-character country code based on ISO 3166.
			COUNTRY_CODE3 	CHAR(3)				Three-character country code based on ISO 3166.
			COUNTRY_NAME 	VARCHAR(50) 		Country name based on ISO 3166
			*/
			// we assume that the ip-to-country.csv is valid and doesn't cause any troubles
			// Since dec 2007 the file is provided without " so we tokenize on ,
			// get & process IP range

			while (!feof(readFile)) {
				error = false;
				if (_fgetts(szbuffer, 88,readFile)==0) break;
				++iLine;
				
				if (*szbuffer != _T('"'))
					continue;
				szIPStart=++szbuffer;
				#pragma warning(disable:4555) //Xman
				for ( szbuffer ; *szbuffer != 0 && *szbuffer != '"'; szbuffer++ );
				*szbuffer = '\0';
				szIPEnd=szbuffer+=3;
				for ( szbuffer ; *szbuffer != 0 && *szbuffer != '"'; szbuffer++ );
				*szbuffer = '\0';
				sz2L = szbuffer+=3;
				for ( szbuffer ; *szbuffer != 0 && *szbuffer != '"'; szbuffer++ );
				*szbuffer = '\0';
				sz3L = szbuffer+=3;
				for ( szbuffer ; *szbuffer != 0 && *szbuffer != '"'; szbuffer++ );
				*szbuffer = '\0';
				szCountry = szbuffer+=3;
				++szbuffer;
				#pragma warning(disable:4245) //Xman
				for ( szbuffer ; *szbuffer != 0 && *szbuffer != '"'; szbuffer++ )
					if ( (*szbuffer >= (TCHAR)L'A') && (*szbuffer <= (TCHAR)L'Z') )
						*szbuffer -= L'A' - L'a';
					else if (*szbuffer == (TCHAR)L' ')
						++szbuffer;
				*szbuffer= '\0';
				szbuffer=szIPStart-1;
				++iCount;
				#pragma warning(default:4245)
				#pragma warning(default:4555)
				//zz_fly :: VS2005 compatibility :: thanks Stulle/DolphinX
				/*
				AddIPRange((uint32)_tstoi(szIPStart),(uint32)_tstoi(szIPEnd), sz2L, sz3L, szCountry);
				*/
				AddIPRange(_tcstoul(szIPStart,NULL,10), _tcstoul(szIPEnd,NULL,10), sz2L, sz3L, szCountry);
				//zz_fly :: VS2005 compatibility :: end
			}
			fclose(readFile);

			// sort the IP2Country list by IP range start addresses
			qsort(m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpIP2CountryByStartAddr);
			if (m_iplist.GetCount() >= 2)
			{
				IPRange_Struct2* pPrv = m_iplist[0];
				int i = 1;
				while (i < m_iplist.GetCount())
				{
					IPRange_Struct2* pCur = m_iplist[i];
					if (   pCur->IPstart >= pPrv->IPstart && pCur->IPstart <= pPrv->IPend	 // overlapping
						//Xman: Xtreme only uses the long names... use it here too
						|| pCur->IPstart == pPrv->IPend+1 && pCur->country->LongCountryName == pPrv->country->LongCountryName) // adjacent
					{
						if (pCur->IPstart != pPrv->IPstart || pCur->IPend != pPrv->IPend) // don't merge identical entries
						{
							//TODO: not yet handled, overlapping entries with different 'level'
							if (pCur->IPend > pPrv->IPend)
								pPrv->IPend = pCur->IPend;
							//pPrv->desc += _T("; ") + pCur->desc; // this may create a very very long description string...
							++iMerged;
						}
						else
						{
							// if we have identical entries, use the lowest 'level'
							/*if (pCur->level < pPrv->level)
								pPrv->level = pCur->level;
							*/
							iDuplicate++;
						}
						delete pCur;
						m_iplist.RemoveAt(i);
						continue;
					}
					pPrv = pCur;
					++i;
				}
			}

			if (thePrefs.GetVerbose())
			{
				AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_LOADED2), ip2countryCSVfile, GetTickCount()-startMesure);
				AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_INFO), iLine, iCount, iDuplicate, iMerged);
			}

		}
		else{
			throw CString(GetResString(IDS_IP2COUNTRY_ERROR3));
		}
	}
	catch(CString strerror){
		AddLogLine(false, _T("%s %s"), strerror, ip2countryCSVfile);
		RemoveAllIPs();
		delete[] szbuffer;
		return false;
	}
	delete[] szbuffer;
	return true;

}

bool CIP2Country::LoadCountryFlagLib(){

	CString ip2countryCountryFlag;

	try{

		//detect windows version
		if(thePrefs.GetWindowsVersion() >= _WINVER_XP_){
			//it's XP, we can use beautiful 32bits flags with alpha channel :)
			ip2countryCountryFlag = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("countryflag32.dll");
		}
		else{
			//oh~ it's not XP, but we still can load the 24bits flags
			ip2countryCountryFlag = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("countryflag.dll");
		}

		_hCountryFlagDll = LoadLibrary(ip2countryCountryFlag); 
		if (_hCountryFlagDll == NULL) 
		{ 
			throw CString(GetResString(IDS_IP2COUNTRY_ERROR4));
		} 

		uint16	resID[] = {
			IDI_COUNTRY_FLAG_NOFLAG,//first res in image list should be N/A

			IDI_COUNTRY_FLAG_AD, IDI_COUNTRY_FLAG_AE, IDI_COUNTRY_FLAG_AF, IDI_COUNTRY_FLAG_AG, 
			IDI_COUNTRY_FLAG_AI, IDI_COUNTRY_FLAG_AL, IDI_COUNTRY_FLAG_AM, IDI_COUNTRY_FLAG_AN, 
			IDI_COUNTRY_FLAG_AO, IDI_COUNTRY_FLAG_AR, IDI_COUNTRY_FLAG_AS, IDI_COUNTRY_FLAG_AT, 
			IDI_COUNTRY_FLAG_AU, IDI_COUNTRY_FLAG_AW, IDI_COUNTRY_FLAG_AZ, IDI_COUNTRY_FLAG_BA, 
			IDI_COUNTRY_FLAG_BB, IDI_COUNTRY_FLAG_BD, IDI_COUNTRY_FLAG_BE, IDI_COUNTRY_FLAG_BF, 
			IDI_COUNTRY_FLAG_BG, IDI_COUNTRY_FLAG_BH, IDI_COUNTRY_FLAG_BI, IDI_COUNTRY_FLAG_BJ, 
			IDI_COUNTRY_FLAG_BM, IDI_COUNTRY_FLAG_BN, IDI_COUNTRY_FLAG_BO, IDI_COUNTRY_FLAG_BR, 
			IDI_COUNTRY_FLAG_BS, IDI_COUNTRY_FLAG_BT, IDI_COUNTRY_FLAG_BW, IDI_COUNTRY_FLAG_BY, 
			IDI_COUNTRY_FLAG_BZ, IDI_COUNTRY_FLAG_CA, IDI_COUNTRY_FLAG_CC, IDI_COUNTRY_FLAG_CD, 
			IDI_COUNTRY_FLAG_CF, IDI_COUNTRY_FLAG_CG, IDI_COUNTRY_FLAG_CH, IDI_COUNTRY_FLAG_CI, 
			IDI_COUNTRY_FLAG_CK, IDI_COUNTRY_FLAG_CL, IDI_COUNTRY_FLAG_CM, IDI_COUNTRY_FLAG_CN, 
			IDI_COUNTRY_FLAG_CO, IDI_COUNTRY_FLAG_CR, IDI_COUNTRY_FLAG_CU, IDI_COUNTRY_FLAG_CV, 
			IDI_COUNTRY_FLAG_CX, IDI_COUNTRY_FLAG_CY, IDI_COUNTRY_FLAG_CZ, IDI_COUNTRY_FLAG_DE, 
			IDI_COUNTRY_FLAG_DJ, IDI_COUNTRY_FLAG_DK, IDI_COUNTRY_FLAG_DM, IDI_COUNTRY_FLAG_DO, 
			IDI_COUNTRY_FLAG_DZ, IDI_COUNTRY_FLAG_EC, IDI_COUNTRY_FLAG_EE, IDI_COUNTRY_FLAG_EG, 
			IDI_COUNTRY_FLAG_EH, IDI_COUNTRY_FLAG_ER, IDI_COUNTRY_FLAG_ES, IDI_COUNTRY_FLAG_ET, 
			IDI_COUNTRY_FLAG_FI, IDI_COUNTRY_FLAG_FJ, IDI_COUNTRY_FLAG_FK, IDI_COUNTRY_FLAG_FM, 
			IDI_COUNTRY_FLAG_FO, IDI_COUNTRY_FLAG_FR, IDI_COUNTRY_FLAG_GA, IDI_COUNTRY_FLAG_GB, 
			IDI_COUNTRY_FLAG_GD, IDI_COUNTRY_FLAG_GE, IDI_COUNTRY_FLAG_GG, IDI_COUNTRY_FLAG_GH, 
			IDI_COUNTRY_FLAG_GI, IDI_COUNTRY_FLAG_GK, IDI_COUNTRY_FLAG_GL, IDI_COUNTRY_FLAG_GM, 
			IDI_COUNTRY_FLAG_GN, IDI_COUNTRY_FLAG_GP, IDI_COUNTRY_FLAG_GQ, IDI_COUNTRY_FLAG_GR, 
			IDI_COUNTRY_FLAG_GS, IDI_COUNTRY_FLAG_GT, IDI_COUNTRY_FLAG_GU, IDI_COUNTRY_FLAG_GW, 
			IDI_COUNTRY_FLAG_GY, IDI_COUNTRY_FLAG_HK, IDI_COUNTRY_FLAG_HN, IDI_COUNTRY_FLAG_HR, 
			IDI_COUNTRY_FLAG_HT, IDI_COUNTRY_FLAG_HU, IDI_COUNTRY_FLAG_ID, IDI_COUNTRY_FLAG_IE, 
			IDI_COUNTRY_FLAG_IL, IDI_COUNTRY_FLAG_IM, IDI_COUNTRY_FLAG_IN, IDI_COUNTRY_FLAG_IO, 
			IDI_COUNTRY_FLAG_IQ, IDI_COUNTRY_FLAG_IR, IDI_COUNTRY_FLAG_IS, IDI_COUNTRY_FLAG_IT, 
			IDI_COUNTRY_FLAG_JE, IDI_COUNTRY_FLAG_JM, IDI_COUNTRY_FLAG_JO, IDI_COUNTRY_FLAG_JP, 
			IDI_COUNTRY_FLAG_KE, IDI_COUNTRY_FLAG_KG, IDI_COUNTRY_FLAG_KH, IDI_COUNTRY_FLAG_KI, 
			IDI_COUNTRY_FLAG_KM, IDI_COUNTRY_FLAG_KN, IDI_COUNTRY_FLAG_KP, IDI_COUNTRY_FLAG_KR, 
			IDI_COUNTRY_FLAG_KW, IDI_COUNTRY_FLAG_KY, IDI_COUNTRY_FLAG_KZ, IDI_COUNTRY_FLAG_LA, 
			IDI_COUNTRY_FLAG_LB, IDI_COUNTRY_FLAG_LC, IDI_COUNTRY_FLAG_LI, IDI_COUNTRY_FLAG_LK, 
			IDI_COUNTRY_FLAG_LR, IDI_COUNTRY_FLAG_LS, IDI_COUNTRY_FLAG_LT, IDI_COUNTRY_FLAG_LU, 
			IDI_COUNTRY_FLAG_LV, IDI_COUNTRY_FLAG_LY, IDI_COUNTRY_FLAG_MA, IDI_COUNTRY_FLAG_MC, 
			IDI_COUNTRY_FLAG_MD, IDI_COUNTRY_FLAG_MG, IDI_COUNTRY_FLAG_MH, IDI_COUNTRY_FLAG_MK, 
			IDI_COUNTRY_FLAG_ML, IDI_COUNTRY_FLAG_MM, IDI_COUNTRY_FLAG_MN, IDI_COUNTRY_FLAG_MO, 
			IDI_COUNTRY_FLAG_MP, IDI_COUNTRY_FLAG_MQ, IDI_COUNTRY_FLAG_MR, IDI_COUNTRY_FLAG_MS, 
			IDI_COUNTRY_FLAG_MT, IDI_COUNTRY_FLAG_MU, IDI_COUNTRY_FLAG_MV, IDI_COUNTRY_FLAG_MW, 
			IDI_COUNTRY_FLAG_MX, IDI_COUNTRY_FLAG_MY, IDI_COUNTRY_FLAG_MZ, IDI_COUNTRY_FLAG_NA, 
			IDI_COUNTRY_FLAG_NC, IDI_COUNTRY_FLAG_NE, IDI_COUNTRY_FLAG_NF, IDI_COUNTRY_FLAG_NG, 
			IDI_COUNTRY_FLAG_NI, IDI_COUNTRY_FLAG_NL, IDI_COUNTRY_FLAG_NO, IDI_COUNTRY_FLAG_NP, 
			IDI_COUNTRY_FLAG_NR, IDI_COUNTRY_FLAG_NU, IDI_COUNTRY_FLAG_NZ, IDI_COUNTRY_FLAG_OM, 
			IDI_COUNTRY_FLAG_PA, IDI_COUNTRY_FLAG_PC, IDI_COUNTRY_FLAG_PE, IDI_COUNTRY_FLAG_PF, 
			IDI_COUNTRY_FLAG_PG, IDI_COUNTRY_FLAG_PH, IDI_COUNTRY_FLAG_PK, IDI_COUNTRY_FLAG_PL, 
			IDI_COUNTRY_FLAG_PM, IDI_COUNTRY_FLAG_PN, IDI_COUNTRY_FLAG_PR, IDI_COUNTRY_FLAG_PS, 
			IDI_COUNTRY_FLAG_PT, IDI_COUNTRY_FLAG_PW, IDI_COUNTRY_FLAG_PY, IDI_COUNTRY_FLAG_QA, 
			IDI_COUNTRY_FLAG_RO, IDI_COUNTRY_FLAG_RU, IDI_COUNTRY_FLAG_RW, IDI_COUNTRY_FLAG_SA, 
			IDI_COUNTRY_FLAG_SB, IDI_COUNTRY_FLAG_SC, IDI_COUNTRY_FLAG_SD, IDI_COUNTRY_FLAG_SE, 
			IDI_COUNTRY_FLAG_SG, IDI_COUNTRY_FLAG_SH, IDI_COUNTRY_FLAG_SI, IDI_COUNTRY_FLAG_SK, 
			IDI_COUNTRY_FLAG_SL, IDI_COUNTRY_FLAG_SM, IDI_COUNTRY_FLAG_SN, IDI_COUNTRY_FLAG_SO, 
			IDI_COUNTRY_FLAG_SR, IDI_COUNTRY_FLAG_ST, IDI_COUNTRY_FLAG_SU, IDI_COUNTRY_FLAG_SV, 
			IDI_COUNTRY_FLAG_SY, IDI_COUNTRY_FLAG_SZ, IDI_COUNTRY_FLAG_TC, IDI_COUNTRY_FLAG_TD, 
			IDI_COUNTRY_FLAG_TF, IDI_COUNTRY_FLAG_TG, IDI_COUNTRY_FLAG_TH, IDI_COUNTRY_FLAG_TJ, 
			IDI_COUNTRY_FLAG_TK, IDI_COUNTRY_FLAG_TL, IDI_COUNTRY_FLAG_TM, IDI_COUNTRY_FLAG_TN, 
			IDI_COUNTRY_FLAG_TO, IDI_COUNTRY_FLAG_TR, IDI_COUNTRY_FLAG_TT, IDI_COUNTRY_FLAG_TV, 
			IDI_COUNTRY_FLAG_TW, IDI_COUNTRY_FLAG_TZ, IDI_COUNTRY_FLAG_UA, IDI_COUNTRY_FLAG_UG, 
			IDI_COUNTRY_FLAG_UM, IDI_COUNTRY_FLAG_US, IDI_COUNTRY_FLAG_UY, IDI_COUNTRY_FLAG_UZ, 
			IDI_COUNTRY_FLAG_VA, IDI_COUNTRY_FLAG_VC, IDI_COUNTRY_FLAG_VE, IDI_COUNTRY_FLAG_VG, 
			IDI_COUNTRY_FLAG_VI, IDI_COUNTRY_FLAG_VN, IDI_COUNTRY_FLAG_VU, IDI_COUNTRY_FLAG_WF, 
			IDI_COUNTRY_FLAG_WS, IDI_COUNTRY_FLAG_YE, IDI_COUNTRY_FLAG_YU, IDI_COUNTRY_FLAG_ZA, 
			IDI_COUNTRY_FLAG_ZM, IDI_COUNTRY_FLAG_ZW, 
			IDI_COUNTRY_FLAG_UK, //by tharghan
			IDI_COUNTRY_FLAG_CS, //by propaganda
			IDI_COUNTRY_FLAG_TP, //by commander
			IDI_COUNTRY_FLAG_AQ, IDI_COUNTRY_FLAG_AX, IDI_COUNTRY_FLAG_BV, IDI_COUNTRY_FLAG_GF,
			IDI_COUNTRY_FLAG_ME, IDI_COUNTRY_FLAG_MF, IDI_COUNTRY_FLAG_RE, IDI_COUNTRY_FLAG_RS,
			IDI_COUNTRY_FLAG_YT, //by tomchen1989
			IDI_COUNTRY_FLAG_AP, //by tomchen1989
			IDI_COUNTRY_FLAG_EU, //by tomchen1989

			65535//the end
		};

		CString countryID[] = {
			_T("N/A"),//first res in image list should be N/A

			_T("AD"), _T("AE"), _T("AF"), _T("AG"), _T("AI"), _T("AL"), _T("AM"), _T("AN"), _T("AO"), _T("AR"), _T("AS"), _T("AT"), _T("AU"), _T("AW"), _T("AZ"), 
			_T("BA"), _T("BB"), _T("BD"), _T("BE"), _T("BF"), _T("BG"), _T("BH"), _T("BI"), _T("BJ"), _T("BM"), _T("BN"), _T("BO"), _T("BR"), _T("BS"), _T("BT"), 
			_T("BW"), _T("BY"), _T("BZ"), _T("CA"), _T("CC"), _T("CD"), _T("CF"), _T("CG"), _T("CH"), _T("CI"), _T("CK"), _T("CL"), _T("CM"), _T("CN"), _T("CO"), 
			_T("CR"), _T("CU"), _T("CV"), _T("CX"), _T("CY"), _T("CZ"), _T("DE"), _T("DJ"), _T("DK"), _T("DM"), _T("DO"), _T("DZ"), _T("EC"), _T("EE"), _T("EG"), 
			_T("EH"), _T("ER"), _T("ES"), _T("ET"), _T("FI"), _T("FJ"), _T("FK"), _T("FM"), _T("FO"), _T("FR"), _T("GA"), _T("GB"), _T("GD"), _T("GE"), _T("GG"), 
			_T("GH"), _T("GI"), _T("GK"), _T("GL"), _T("GM"), _T("GN"), _T("GP"), _T("GQ"), _T("GR"), _T("GS"), _T("GT"), _T("GU"), _T("GW"), _T("GY"), _T("HK"), 
			_T("HN"), _T("HR"), _T("HT"), _T("HU"), _T("ID"), _T("IE"), _T("IL"), _T("IM"), _T("IN"), _T("IO"), _T("IQ"), _T("IR"), _T("IS"), _T("IT"), _T("JE"), 
			_T("JM"), _T("JO"), _T("JP"), _T("KE"), _T("KG"), _T("KH"), _T("KI"), _T("KM"), _T("KN"), _T("KP"), _T("KR"), _T("KW"), _T("KY"), _T("KZ"), _T("LA"), 
			_T("LB"), _T("LC"), _T("LI"), _T("LK"), _T("LR"), _T("LS"), _T("LT"), _T("LU"), _T("LV"), _T("LY"), _T("MA"), _T("MC"), _T("MD"), _T("MG"), _T("MH"), 
			_T("MK"), _T("ML"), _T("MM"), _T("MN"), _T("MO"), _T("MP"), _T("MQ"), _T("MR"), _T("MS"), _T("MT"), _T("MU"), _T("MV"), _T("MW"), _T("MX"), _T("MY"), 
			_T("MZ"), _T("NA"), _T("NC"), _T("NE"), _T("NF"), _T("NG"), _T("NI"), _T("NL"), _T("NO"), _T("NP"), _T("NR"), _T("NU"), _T("NZ"), _T("OM"), _T("PA"), 
			_T("PC"), _T("PE"), _T("PF"), _T("PG"), _T("PH"), _T("PK"), _T("PL"), _T("PM"), _T("PN"), _T("PR"), _T("PS"), _T("PT"), _T("PW"), _T("PY"), _T("QA"), 
			_T("RO"), _T("RU"), _T("RW"), _T("SA"), _T("SB"), _T("SC"), _T("SD"), _T("SE"), _T("SG"), _T("SH"), _T("SI"), _T("SK"), _T("SL"), _T("SM"), _T("SN"), 
			_T("SO"), _T("SR"), _T("ST"), _T("SU"), _T("SV"), _T("SY"), _T("SZ"), _T("TC"), _T("TD"), _T("TF"), _T("TG"), _T("TH"), _T("TJ"), _T("TK"), _T("TL"), 
			_T("TM"), _T("TN"), _T("TO"), _T("TR"), _T("TT"), _T("TV"), _T("TW"), _T("TZ"), _T("UA"), _T("UG"), _T("UM"), _T("US"), _T("UY"), _T("UZ"), _T("VA"), 
			_T("VC"), _T("VE"), _T("VG"), _T("VI"), _T("VN"), _T("VU"), _T("WF"), _T("WS"), _T("YE"), _T("YU"), _T("ZA"), _T("ZM"), _T("ZW"), 
			_T("UK"), //by tharghan
			_T("CS"), //by propaganda
			_T("TP"), //by commander
			_T("AQ"), _T("AX"), _T("BV"), _T("GF"), _T("ME"), _T("MF"), _T("RE"), _T("RS"), _T("YT"), //by tomchen1989
			_T("AP"), _T("EU") //by tomchen1989
		};

		//HICON iconHandle;

		CountryFlagImageList.DeleteImageList();
		CountryFlagImageList.Create(18,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
		CountryFlagImageList.SetBkColor(CLR_NONE);

		//the res Array have one element to be the STOP
		//NOTE: Wiz provided us with a fix so let's use this!
		/*
		for(int cur_pos = 0; resID[cur_pos] != 65535; cur_pos++){

			CountryIDtoFlagIndex.SetAt(countryID[cur_pos], (uint16)cur_pos);

			iconHandle = LoadIcon(_hCountryFlagDll, MAKEINTRESOURCE(resID[cur_pos]));
			if(iconHandle == NULL) throw CString(GetResString(IDS_IP2COUNTRY_ERROR5));
			
			CountryFlagImageList.Add(iconHandle);
		}
		*/
		//>>> FiX for IP2Country and other custom lists
		HICON iconHandle = NULL;
		int iconIndex = -1;
		//<<< FiX for IP2Country and other custom lists
		for(int i = 0; resID[i] != 65535; i++)
		{
			//>>> FiX for IP2Country and other custom lists
			iconHandle = (HICON)::LoadImage(_hCountryFlagDll, MAKEINTRESOURCE(resID[i]), IMAGE_ICON, 18, 16, LR_DEFAULTCOLOR);
			if(iconHandle) 
			//<<< FiX for IP2Country and other custom lists
			{
				//>>> FiX for IP2Country and other custom lists
				iconIndex = CountryFlagImageList.Add(iconHandle);
				if(iconIndex != -1)
					CountryIDtoFlagIndex.SetAt(countryID[i], (uint16)iconIndex);
				::DestroyIcon(iconHandle);
				//<<< FiX for IP2Country and other custom lists
			}
			else
				theApp.QueueDebugLogLineEx(LOG_WARNING, GetResString(IDS_IP2COUNTRY_ERROR5), resID[i]);
		}
	}
	catch(CString error){
		AddLogLine(false, _T("%s in %s"), error, ip2countryCountryFlag);
		RemoveAllFlags();
		//free lib
		if(_hCountryFlagDll != NULL) FreeLibrary(_hCountryFlagDll);
		return false;
	}

	//free lib
	if(_hCountryFlagDll != NULL) FreeLibrary(_hCountryFlagDll);

	AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_FLAGLOAD));
	return true;

}

void CIP2Country::RemoveAllIPs(){

	//first remove all country structs
	CString key;
	Country_Struct* value;
	POSITION pos = countryList.GetHeadPosition();
	while(pos)
	{
		countryList.GetNextAssoc(pos, key, value);
		delete value;
	}
	countryList.RemoveAll();

	//now the ip structs
	for (int i = 0; i < m_iplist.GetCount(); i++)
		delete m_iplist[i];
	m_iplist.RemoveAll();

	AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_FILELOAD));
}

void CIP2Country::RemoveAllFlags(){

	//destory all image
	CountryFlagImageList.DeleteImageList();

	//also clean out the map table
	CountryIDtoFlagIndex.RemoveAll();

	AddLogLine(false, GetResString(IDS_IP2COUNTRY_FLAGUNLD));
}

void CIP2Country::AddIPRange(uint32 IPfrom,uint32 IPto, TCHAR* shortCountryName, TCHAR* /*midCountryName*/, TCHAR* longCountryName){
	IPRange_Struct2* newRange = new IPRange_Struct2();
	newRange->IPstart = IPfrom;
	newRange->IPend = IPto;

	const CRBMap<CString, Country_Struct*>::CPair* pair;
	pair = countryList.Lookup(longCountryName);
	if (pair == NULL) {
		//AddCountry
		Country_Struct* newCountry = new Country_Struct();
		newCountry->LongCountryName = longCountryName;
		//newCountry->MidCountryName = midCountryName;
		//newCountry->LongCountryName = longCountryName;
		countryList.SetAt(longCountryName, newCountry);

		//Add Flag
		if(EnableCountryFlag){

			const CRBMap<CString, uint16>::CPair* pair2;
			pair2 = CountryIDtoFlagIndex.Lookup(shortCountryName);

			if(pair2 != NULL){
				newCountry->FlagIndex = pair2->m_value;
			}
			else{
				newCountry->FlagIndex = NO_FLAG;
			}
		}

		pair = countryList.Lookup(longCountryName);
	}
	ASSERT(pair!=NULL);
	newRange->country = pair->m_value;
	
	m_iplist.Add(newRange);
}

static int __cdecl CmpIP2CountryByAddr(const void* pvKey, const void* pvElement)
{
	uint32 ip = *(uint32*)pvKey;
	const IPRange_Struct2* pIP2Country = *(IPRange_Struct2**)pvElement;

	if (ip < pIP2Country->IPstart)
		return -1;
	if (ip > pIP2Country->IPend)
		return 1;
	return 0;
}

//struct 
Country_Struct* CIP2Country::GetCountryFromIP(uint32 ClientIP) const
{

	if(EnableIP2Country == false || ClientIP == 0){
		return &defaultCountry;
	}
	if(m_iplist.GetCount() == 0){
		AddDebugLogLine(false, _T("CIP2Country::GetCountryFromIP iplist doesn't exist"));
		return &defaultCountry;
	}
	ClientIP = htonl(ClientIP);
	IPRange_Struct2** ppFound = (IPRange_Struct2**)bsearch(&ClientIP, m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpIP2CountryByAddr);
	if (ppFound)
	{
		return (*ppFound)->country;
	}

	return &defaultCountry;
}
CString CIP2Country::GetCountryNameFromRef(Country_Struct* m_structCountry, bool longName){
	if(EnableIP2Country)
	{
		if(longName)
			return m_structCountry->LongCountryName;
		/* //Xman only the long name
		switch(thePrefs.GetIP2CountryNameMode()){
			case IP2CountryName_SHORT:
				return m_structCountry->ShortCountryName;
			case IP2CountryName_MID:
				return m_structCountry->MidCountryName;
			case IP2CountryName_LONG:
				return m_structCountry->LongCountryName;
		}
		*/
	}
	else if(longName)
		return GetResString(IDS_DISABLED);	
	return _T("");
}
bool CIP2Country::ShowCountryFlag() const
{

	return 
		//user wanna see flag,
		(// thePrefs.IsIP2CountryShowFlag() && //Xman if we wan't to make it optional
		//flag have been loaded
		EnableCountryFlag && 
		//ip table have been loaded
		EnableIP2Country);
}

//EastShare End - added by AndCycle, IP to Country
/*
//Commander - Added: IP2Country auto-updating - Start
void CIP2Country::UpdateIP2CountryURL()
{   
	CString sbuffer;
	CString strURL = thePrefs.GetUpdateVerURLIP2Country(); //Version URL to keep it separated

	TCHAR szTempFilePath[_MAX_PATH];
	_tmakepath(szTempFilePath, NULL, thePrefs.GetAppDir(), DFLT_IP2COUNTRY_FILENAME, _T("tmp"));
	FILE* readFile= _tfsopen(szTempFilePath, _T("r"), _SH_DENYWR);

	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = GetResString(IDS_IP2COUNTRY_VERFILE);
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = szTempFilePath;
	if (dlgDownload.DoModal() != IDOK)
	{
		_tremove(szTempFilePath);
		AddLogLine(true, GetResString(IDS_LOG_ERRDWN), strURL);
		return;
	}
	readFile = _tfsopen(szTempFilePath, _T("r"), _SH_DENYWR);

	char buffer[9]; //Versionformat: Ymmdd -> 20040101
	int lenBuf = 9;
	fgets(buffer,lenBuf,readFile);
	sbuffer = buffer;
	sbuffer = sbuffer.Trim();
	fclose(readFile);
	_tremove(szTempFilePath);

    // Compare the Version numbers
	if ((thePrefs.GetIP2CountryVersion()< (uint32) _tstoi(sbuffer)) || !PathFileExists(GetDefaultFilePath())) {
		
		CString IP2CountryURL = thePrefs.GetUpdateURLIP2Country();
		
		_tmakepath(szTempFilePath, NULL, thePrefs.GetConfigDir(), DFLT_IP2COUNTRY_FILENAME, _T("tmp"));

		CHttpDownloadDlg dlgDownload;
		dlgDownload.m_strTitle = GetResString(IDS_IP2COUNTRY_DWNFILE);
		dlgDownload.m_sURLToDownload = IP2CountryURL;
		dlgDownload.m_sFileToDownloadInto = szTempFilePath;
		if (dlgDownload.DoModal() != IDOK)
		{
			_tremove(szTempFilePath);
			LogError(LOG_STATUSBAR, GetResString(IDS_IP2COUNTRY_ERROR6));
			return;
		}
        
		bool bIsZipFile = false;
		bool bUnzipped = false;
		CZIPFile zip;
		if (zip.Open(szTempFilePath))
		{
			bIsZipFile = true;

			CZIPFile::File* zfile = zip.GetFile(DFLT_IP2COUNTRY_FILENAME); // It has to be a zip-file which includes a file called: ip-to-country.csv
			if (zfile)
			{
				TCHAR szTempUnzipFilePath[MAX_PATH];
				_tmakepath(szTempUnzipFilePath, NULL, thePrefs.GetConfigDir(), DFLT_IP2COUNTRY_FILENAME, _T(".unzip.tmp"));
				if (zfile->Extract(szTempUnzipFilePath))
				{
					zip.Close();
					zfile = NULL;

					if (_tremove(GetDefaultFilePath()) != 0)
						TRACE("*** Error: Failed to remove default IP to Country file \"%s\" - %s\n", GetDefaultFilePath(), _tcserror(errno));
					if (_trename(szTempUnzipFilePath, GetDefaultFilePath()) != 0)
						TRACE("*** Error: Failed to rename uncompressed IP to Country file \"%s\" to default IP to Country file \"%s\" - %s\n", szTempUnzipFilePath, GetDefaultFilePath(), _tcserror(errno));
					if (_tremove(szTempFilePath) != 0)
						TRACE("*** Error: Failed to remove temporary IP to Country file \"%s\" - %s\n", szTempFilePath, _tcserror(errno));
					bUnzipped = true;
				}
				else
					LogError(LOG_STATUSBAR, GetResString(IDS_IP2COUNTRY_ERROR7), szTempFilePath);
			}
			else
				LogError(LOG_STATUSBAR, GetResString(IDS_IP2COUNTRY_ERROR8), szTempFilePath); //File not found inside the zip-file

			zip.Close();
		}
        
		if (!bIsZipFile && !bUnzipped)
		{
			_tremove(GetDefaultFilePath());
			_trename(szTempFilePath, GetDefaultFilePath());
		}

		if(bIsZipFile && !bUnzipped){
			return;
		}

		if(thePrefs.GetIP2CountryNameMode() != IP2CountryName_DISABLE || thePrefs.IsIP2CountryShowFlag()){
			theApp.ip2country->Unload();
			AddLogLine(false,GetResString(IDS_IP2COUNTRY_UPUNLOAD));
			theApp.ip2country->Load();
			AddLogLine(false,GetResString(IDS_IP2COUNTRY_UPLOAD));
		}

		thePrefs.SetIP2CountryVersion(_tstoi(sbuffer)); //Commander - Added: Update version number
		thePrefs.Save();
	}
}
//Commander - Added: IP2Country auto-updating - End
*/
CString CIP2Country::GetDefaultFilePath() const
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_IP2COUNTRY_FILENAME;
}

IMAGELISTDRAWPARAMS CIP2Country::GetFlagImageDrawParams(CDC* dc,int iIndex, POINT point) const
{
	IMAGELISTDRAWPARAMS imldp;
	imldp.cbSize   = sizeof(IMAGELISTDRAWPARAMS);
	imldp.himl     = CountryFlagImageList.m_hImageList;
	imldp.i        = iIndex;
	imldp.hdcDst   = dc->m_hDC;
	imldp.x        = point.x;
	imldp.y        = point.y;
	imldp.cx       = 18;
	imldp.cy       = 16;
	imldp.xBitmap  = 0;
	imldp.yBitmap  = 0;
	imldp.rgbBk    = CLR_DEFAULT;
	imldp.rgbFg    = CLR_DEFAULT;
	imldp.fStyle   = ILD_NORMAL;
	imldp.dwRop    = SRCCOPY;
#if (_WIN32_WINNT >= 0x501)
	imldp.fState   = ILS_NORMAL;
	imldp.Frame    = 0;
	imldp.crEffect = CLR_DEFAULT;
#endif

	return imldp;
}

// ==> Advanced Updates [MorphXT/Stulle] - Stulle
void CIP2Country::UpdateIP2CountryURL()
{   
	CString sbuffer;
	CString strURL = thePrefs.GetUpdateURLIP2Country();
	TCHAR szTempFilePath[_MAX_PATH];
	_tmakepath(szTempFilePath, NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IP2COUNTRY_FILENAME, _T("tmp"));

	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = GetResString(IDS_IP2COUNTRY_DWNFILE);
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = szTempFilePath;
	SYSTEMTIME SysTime;
	if (PathFileExists(GetDefaultFilePath()))
		memcpy(&SysTime, thePrefs.GetIP2CountryVersion(), sizeof(SYSTEMTIME));
	else
		memset(&SysTime, 0, sizeof(SYSTEMTIME));
	dlgDownload.m_pLastModifiedTime = &SysTime;

	if (dlgDownload.DoModal() != IDOK)
	{
		LogError(LOG_STATUSBAR, GetResString(IDS_LOG_ERRDWN), strURL);
		return;
	}
    if (dlgDownload.m_pLastModifiedTime == NULL)
		return;

	bool bIsZipFile = false;
	bool bUnzipped = false;
	CZIPFile zip;
	if (zip.Open(szTempFilePath))
	{
		bIsZipFile = true;

		CZIPFile::File* zfile = zip.GetFile(DFLT_IP2COUNTRY_FILENAME); // It has to be a zip-file which includes a file called: ip-to-country.csv
		if (zfile)
		{
			CString strTempUnzipFilePath;
			_tmakepath(strTempUnzipFilePath.GetBuffer(_MAX_PATH), NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IP2COUNTRY_FILENAME, _T(".unzip.tmp"));
			strTempUnzipFilePath.ReleaseBuffer();
			if (zfile->Extract(strTempUnzipFilePath))
			{
				zip.Close();
				zfile = NULL;

				if (_tremove(GetDefaultFilePath()) != 0)
					TRACE("*** Error: Failed to remove default IP to Country file \"%s\" - %s\n", GetDefaultFilePath(), _tcserror(errno));
				if (_trename(strTempUnzipFilePath, GetDefaultFilePath()) != 0)
					TRACE("*** Error: Failed to rename uncompressed IP to Country file \"%s\" to default IP to Country file \"%s\" - %s\n", strTempUnzipFilePath, GetDefaultFilePath(), _tcserror(errno));
				if (_tremove(szTempFilePath) != 0)
					TRACE("*** Error: Failed to remove temporary IP to Country file \"%s\" - %s\n", szTempFilePath, _tcserror(errno));
				bUnzipped = true;
			}
			else
				LogError(LOG_STATUSBAR, GetResString(IDS_IP2COUNTRY_ERROR7), szTempFilePath);
		}
		else
			LogError(LOG_STATUSBAR, GetResString(IDS_IP2COUNTRY_ERROR8), szTempFilePath); //File not found inside the zip-file

		zip.Close();
	}

	if (!bIsZipFile && !bUnzipped)
	{
		_tremove(GetDefaultFilePath());
		_trename(szTempFilePath, GetDefaultFilePath());
	}

	//Moved up to not retry if archive don't contain the awaited file
	memcpy(thePrefs.GetIP2CountryVersion(), &SysTime, sizeof SysTime);
	thePrefs.Save();

	if(bIsZipFile && !bUnzipped){
		return;
	}

//	if(thePrefs.GetIP2CountryNameMode() != IP2CountryName_DISABLE || thePrefs.IsIP2CountryShowFlag()){
		theApp.ip2country->Unload();
		AddLogLine(false,GetResString(IDS_IP2COUNTRY_UPUNLOAD));
		theApp.ip2country->Load();
		AddLogLine(false,GetResString(IDS_IP2COUNTRY_UPLOAD));
//	}
}
// <== Advanced Updates [MorphXT/Stulle] - Stulle