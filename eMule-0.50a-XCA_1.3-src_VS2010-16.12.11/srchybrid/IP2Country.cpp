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
#include <algorithm>

#undef _OPENMP
#ifdef _OPENMP
#include "omp.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define INITIP2COUNTRYCOUNT 110000

// N/A flag is the first Res, so it should at index zero
#define NO_FLAG 0

CImageList CIP2Country::CountryFlagImageList;
Country_Struct CIP2Country::defaultCountry;
Location_Struct CIP2Country::defaultLocation;// X: [IP2L] - [IP2Location]
/*
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
*/
CIP2Country::CIP2Country()
#ifdef HAVE_UNORDERED
:locationList(257), CountryIDtoFlagIndex(257)
#endif
{

	//m_bRunning = false;

	//defaultCountry.ShortCountryName = GetResString(IDS_IP2COUNTRY_NASHORT);
	//defaultCountry.MidCountryName = GetResString(IDS_IP2COUNTRY_NASHORT);
	defaultCountry.LongCountryName = GetResString(IDS_IP2COUNTRY_NALONG);

	defaultCountry.FlagIndex = NO_FLAG;
	defaultLocation.country = &defaultCountry;// X: [IP2L] - [IP2Location]

	EnableIP2Country = false;
	EnableCountryFlag = false;

	_hCountryFlagDll = NULL; //Enig123::WiZaRd

	//Xman always load
	//if(thePrefs.GetIP2CountryNameMode() != IP2CountryName_DISABLE || 
	//	thePrefs.IsIP2CountryShowFlag()){
		Load();
	//}

	AddLogLine(false, GetResString(IDS_IP2COUNTRY_MSG1));
	AddLogLine(false, GetResString(IDS_IP2COUNTRY_MSG2));
	//m_bRunning = true;
}

CIP2Country::~CIP2Country(){

	//m_bRunning = false;

	Unload();
}

void CIP2Country::Load(){

	EnableCountryFlag = LoadCountryFlagLib();//flag lib first, so ip range can map to flag

	if(EnableIP2Country = LoadFromFile()){
		CountryIDtoFlagIndex.clear(); //Xman after loading /mapping we don't need it anymore
		AddLogLine(false, GetResString(IDS_IP2COUNTRY_LOADED));
	}
}

void CIP2Country::Unload(){
	if(EnableCountryFlag||EnableIP2Country){
		RemoveAllIPs();
		RemoveAllFlags();

		AddLogLine(false, GetResString(IDS_IP2COUNTRY_UNLOADED));
	}
}

/* //Xman not needed
void CIP2Country::Reset(){
	theApp.serverlist->ResetIP2Country();
	theApp.clientlist->ResetIP2Country();
}

void CIP2Country::Refresh(){
	theApp.emuledlg->serverwnd->serverlistctrl.RefreshAllServer();
}
*/
/*
static int __cdecl CmpIP2CountryByStartAddr(const void* p1, const void* p2)
{
	const IPRange_Struct2* rng1 = *(IPRange_Struct2**)p1;
	const IPRange_Struct2* rng2 = *(IPRange_Struct2**)p2;
	return CompareUnsigned(rng1->IPstart, rng2->IPstart);
}*/
bool CmpIP2CountryByStartAddr(const IPRange_Struct2& rng1, const IPRange_Struct2& rng2){
	return rng1.IPstart < rng2.IPstart;
}

bool CIP2Country::LoadFromFile(){// X: [SUL] - [SpeedUpLoading]
	DWORD startMesure = GetTickCount();
	CString ip2countryCSVfile = GetDefaultFilePath();
	FILE* readFile = _tfsopen(ip2countryCSVfile, _T("rS"), _SH_DENYWR);
	if (readFile == NULL) {
		LogError(_T("%s %s"), GetResString(IDS_IP2COUNTRY_ERROR3), ip2countryCSVfile);
		RemoveAllFlags();
		return false;
	}
	DWORD wBOM = 0;// X: ensure file is ANSI
	if(fread(&wBOM, sizeof(wBOM), 1, readFile) == 1 && (((WORD)wBOM == 0xFEFF) || ((WORD)wBOM == 0xFFFE) || ((wBOM & 0xFFFFFF) == 0xBFBBEF))){
		fclose(readFile);
		LogError(_T("Failed to load %s, the file is not ANSI"), ip2countryCSVfile);
		RemoveAllFlags();
		return false;
	}
	(void)fseek(readFile, 0, SEEK_SET);
	setvbuf(readFile, NULL, _IOFBF, 32768);
	m_countryList.reserve(242);
	m_iplist.reserve(INITIP2COUNTRYCOUNT);
	uint_ptr iCount = 0;
	uint_ptr iLine = 0;
//#ifndef _OPENMP
	uint32 lastip=0;// X: check file is sorted
	bool isSorted = true;// X: check file is sorted
//#endif
#ifdef _OPENMP
#pragma omp parallel
{
#endif
	char szbuffer[512];
	bool isOld;
	uint32 ip[2];
	char *szIPRange[2];
	while (fgets(szbuffer, _countof(szbuffer), readFile) != NULL) {
#ifdef _OPENMP
#pragma omp atomic
#endif
		++iLine;
#ifndef _OPENMP
		if((iLine & 0x3fff) == 0 && iLine > 2*0x3fff && theApp.IsSplash()){// X: [MSI] - [More Splash Info]
			CString strLine;
			strLine.Format(_T("Loading %u"),iLine);
			theApp.UpdateSplash2(strLine);
		}
#endif
		size_t len = strlen(szbuffer);
		if(len<5) continue;
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
		char* pbuffer = szbuffer;
		if(pbuffer[len-1] == 10)
			pbuffer[len-1] = 0;

		if (*pbuffer == '"'){
			++pbuffer;
			isOld=true;
		}
		else
			isOld=false;
		bool isValid = true;
		for(INT_PTR i=0;i<2;++i){
			char * pnext;
			ip[i] = (uint32)strtoul(pbuffer,(const char**)&pnext/*,10*/);
			if(pbuffer == pnext){
				isValid = false;
				break;
			}
			if(isOld)
				pbuffer = pnext + 3;
			else
				pbuffer = pnext + 1;
		}
		if(!isValid)
			continue;
		char sep = isOld?'"':',';
		for(INT_PTR i=0;i<2;++i){
			if(*pbuffer == 0){
				isValid = false;
				break;
			}
			szIPRange[i]=pbuffer;
			do{
				if(*pbuffer == sep){
					*pbuffer = 0;
					if(isOld)
						pbuffer+=3;
					else
						++pbuffer;
					break;
				}
			}while(*++pbuffer != 0);
		}
		if(!isValid)
			continue;

		szIPRange[1]=pbuffer;
		for (; *pbuffer != 0; ){
			if (*pbuffer == ' ' || *pbuffer == '.')
				++pbuffer;
			else if(isOld && *pbuffer == '"'){
				*pbuffer = 0;
				break;
			}
			else{
				++pbuffer;
				if ( (*pbuffer >= 'A') && (*pbuffer <= 'Z') )
					*pbuffer -= 'A' - 'a';
				else if(*pbuffer >= 256){
					if(isOld){
						pbuffer = strchr(pbuffer+1,'"');
						if(pbuffer)
							*pbuffer=0;
					}
					break;
				}
			}
		}

#ifdef _OPENMP
#pragma omp critical
#endif
		AddIPRange(ip[0],ip[1], szIPRange[0], szIPRange[1]);
#ifdef _OPENMP
#pragma omp atomic
#endif
		++iCount;
//#ifndef _OPENMP
		if(lastip > ip[0])// X: check file is sorted
			isSorted = false;
		lastip = ip[0];
//#endif
#ifdef _OPENMP
}
#endif
	}
	fclose(readFile);
	/*if(theApp.IsSplash()){// X: [MSI] - [More Splash Info]
		CString strLine;
		strLine.Format(_T("%u Lines Loaded"),iLine);
		theApp.UpdateSplash2(strLine);
	}*/
    for(MAP<LPSTR, Location_Struct*, LPSTR_Pred>::const_iterator it1=locationList.begin();it1!=locationList.end();++it1 )// X: [SUL] - [SpeedUpLoading]
    {
		delete [] it1->first;
	}

//#ifndef _OPENMP
	if(!isSorted)// X: don't sort if file is sorted
//#endif
		// sort the IP2Country list by IP range start addresses
		//qsort(m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpIP2CountryByStartAddr);
		std::sort(m_iplist.begin(), m_iplist.end(), CmpIP2CountryByStartAddr);
	uint_ptr iDuplicate = 0;
	uint_ptr iMerged = 0;
	if (m_iplist.size() >= 2)
	{
		// On large IP2Country lists there is a noticeable performance problem when merging the list.
		// The 'CIPFilterArray::RemoveAt' call is way too expensive to get called during the merging,
		// thus we use temporary helper arrays to copy only the entries into the final list which
		// are not get deleted.

		// Reserve a byte array (its used as a boolean array actually) as large as the current 
		// IP2Country list, so we can set a 'to delete' flag for each entry in the current IP2Country list.
		char* pcToDelete = NULL;
		uint_ptr iNumToDelete = 0;

		IPRange_Struct2* pPrv = &m_iplist[0];
		for(size_t i = 1;i < m_iplist.size(); ++i){
			IPRange_Struct2* pCur = &m_iplist[i];
			if (   pCur->IPstart >= pPrv->IPstart && pCur->IPstart <= pPrv->IPend	 // overlapping
				//Xman: Xtreme only uses the long names... use it here too
				|| pCur->IPstart == pPrv->IPend+1 && pCur->location->country == pPrv->location->country && pCur->location->locationName == pPrv->location->locationName) // adjacent// X: [IP2L] - [IP2Location]
			{
				if (pCur->IPstart != pPrv->IPstart || pCur->IPend != pPrv->IPend) // don't merge identical entries
				{
					//TODO: not yet handled, overlapping entries with different 'level'
					if (pCur->IPend > pPrv->IPend)
						pPrv->IPend = pCur->IPend;
					++iMerged;
				}
				else
				{
					// if we have identical entries, use the lowest 'level'
					++iDuplicate;
				}
				//m_iplist.RemoveAt(i);	// way too expensive (read above)
				if(pcToDelete == NULL){
					pcToDelete = new char[m_iplist.size()];
					memset(pcToDelete, 0, m_iplist.size());
				}
				pcToDelete[i] = 1;		// mark this entry as 'to delete'
				++iNumToDelete;
			}
			else
				pPrv = pCur;
		}
		// Create new IP-filter list which contains only the entries from the original IP-filter list
		// which are not to be deleted.
		if (iNumToDelete > 0)
		{
			//CIPFilterArray newList;
			int iNewListIndex = 0;
			for (size_t i = 0; i < m_iplist.size(); i++) {
				if (!pcToDelete[i]){
					if(iNewListIndex != i)
						m_iplist[iNewListIndex] = m_iplist[i];
					++iNewListIndex;
				}
			}
			m_iplist.erase(m_iplist.end() - iNumToDelete, m_iplist.end());
			ASSERT( iNewListIndex == m_iplist.size() );

			// Replace current list with new list. Dump, but still fast enough (only 1 memcpy)
			//m_iplist.RemoveAll();
			//m_iplist.Append(newList);
			//newList.RemoveAll();
			delete[] pcToDelete;
		}
	}

	if(m_iplist.capacity() != m_iplist.size())
		CIP2CountryArray(m_iplist).swap(m_iplist);

	if (thePrefs.GetVerbose())
	{
		AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_LOADED2), ip2countryCSVfile, GetTickCount()-startMesure);
		AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_INFO), iLine, iCount, iDuplicate, iMerged);
	}
	return true;

}

bool CIP2Country::LoadCountryFlagLib(){// X: [SUL] - [SpeedUpLoading]

		//it's XP, we can use beautiful 32bits flags with alpha channel :)
	CString ip2countryCountryFlag = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+
		//detect windows version
		(thePrefs.GetWindowsVersion() != _WINVER_2K_/*(thePrefs.GetWindowsVersion() == _WINVER_XP_ || thePrefs.GetWindowsVersion() == _WINVER_2003_ || thePrefs.GetWindowsVersion() == _WINVER_VISTA_ || thePrefs.GetWindowsVersion() == _WINVER_7_)*/?
		_T("countryflag32.dll")
		//oh~ it's not XP, but we still can load the 24bits flags
		:_T("countryflag.dll"));

	_hCountryFlagDll = LoadLibrary(ip2countryCountryFlag); 
	if (_hCountryFlagDll == NULL){ 
		LogError(_T("%s in %s"), GetResString(IDS_IP2COUNTRY_ERROR4), ip2countryCountryFlag);
		return false;
	}

	const uint16 resID[] = {
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
		IDI_COUNTRY_FLAG_YT, IDI_COUNTRY_FLAG_AP, IDI_COUNTRY_FLAG_EU //by tomchen1989
	};

	static LPSTR countryID[] = {
		"N/A",//first res in image list should be N/A

		"AD", "AE", "AF", "AG", "AI", "AL", "AM", "AN", "AO", "AR", "AS", "AT", "AU", "AW", "AZ", 
		"BA", "BB", "BD", "BE", "BF", "BG", "BH", "BI", "BJ", "BM", "BN", "BO", "BR", "BS", "BT", 
		"BW", "BY", "BZ", "CA", "CC", "CD", "CF", "CG", "CH", "CI", "CK", "CL", "CM", "CN", "CO", 
		"CR", "CU", "CV", "CX", "CY", "CZ", "DE", "DJ", "DK", "DM", "DO", "DZ", "EC", "EE", "EG", 
		"EH", "ER", "ES", "ET", "FI", "FJ", "FK", "FM", "FO", "FR", "GA", "GB", "GD", "GE", "GG", 
		"GH", "GI", "GK", "GL", "GM", "GN", "GP", "GQ", "GR", "GS", "GT", "GU", "GW", "GY", "HK", 
		"HN", "HR", "HT", "HU", "ID", "IE", "IL", "IM", "IN", "IO", "IQ", "IR", "IS", "IT", "JE", 
		"JM", "JO", "JP", "KE", "KG", "KH", "KI", "KM", "KN", "KP", "KR", "KW", "KY", "KZ", "LA", 
		"LB", "LC", "LI", "LK", "LR", "LS", "LT", "LU", "LV", "LY", "MA", "MC", "MD", "MG", "MH", 
		"MK", "ML", "MM", "MN", "MO", "MP", "MQ", "MR", "MS", "MT", "MU", "MV", "MW", "MX", "MY", 
		"MZ", "NA", "NC", "NE", "NF", "NG", "NI", "NL", "NO", "NP", "NR", "NU", "NZ", "OM", "PA", 
		"PC", "PE", "PF", "PG", "PH", "PK", "PL", "PM", "PN", "PR", "PS", "PT", "PW", "PY", "QA", 
		"RO", "RU", "RW", "SA", "SB", "SC", "SD", "SE", "SG", "SH", "SI", "SK", "SL", "SM", "SN", 
		"SO", "SR", "ST", "SU", "SV", "SY", "SZ", "TC", "TD", "TF", "TG", "TH", "TJ", "TK", "TL", 
		"TM", "TN", "TO", "TR", "TT", "TV", "TW", "TZ", "UA", "UG", "UM", "US", "UY", "UZ", "VA", 
		"VC", "VE", "VG", "VI", "VN", "VU", "WF", "WS", "YE", "YU", "ZA", "ZM", "ZW", 
		"UK", //by tharghan
		"CS", //by propaganda
		"TP", //by commander
		"AQ", "AX", "BV", "GF", "ME", "MF", "RE", "RS", "YT", "AP", "EU" //by tomchen1989
	};

	//Enig123::Reduce GDI handles
	// original idea from Rapid_Mule @ http://forum.emule-project.net/index.php?showtopic=132369
	HICON iconHandle = NULL;
	//HANDLE iconHandle; //Reduce GDI handles

	CountryFlagImageList.DeleteImageList();
	CountryFlagImageList.Create(18,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	//CountryFlagImageList.SetBkColor(CLR_NONE);

	ASSERT(_countof(countryID) == _countof(resID));

	bool failed = false;
	//Xman Code Improcement
	//the res Array have one element to be the STOP
	//for(uint_ptr cur_pos = 0; resID[cur_pos] != 253; cur_pos++){
	for(uint_ptr cur_pos = 0; cur_pos < _countof(countryID); cur_pos++){

		//Enig123::Reduce GDI handles
		//iconHandle = LoadIcon(_hCountryFlagDll, MAKEINTRESOURCE(resID[cur_pos]));
		iconHandle = (HICON)::LoadImage(_hCountryFlagDll, MAKEINTRESOURCE(resID[cur_pos]), IMAGE_ICON, 18, 16, LR_DEFAULTCOLOR);
		if(iconHandle)
		{
			int iconIndex = CountryFlagImageList.Add(iconHandle);
			if(iconIndex != -1)
				CountryIDtoFlagIndex[countryID[cur_pos]] = (uint16)iconIndex;// X: use static countryID as key
			::DestroyIcon(iconHandle);
		}
		//Enig123::Reduce GDI handles
		else
			failed = true;
	}
	if(failed)
		LogError(_T("%s in %s"), GetResString(IDS_IP2COUNTRY_ERROR5), ip2countryCountryFlag);
	//Xman end

	//free lib
	FreeLibrary(_hCountryFlagDll);

	AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_FLAGLOAD));
	return true;

}

void CIP2Country::RemoveAllIPs(){
	EnableIP2Country = false;

	//first remove all location structs// X: [IP2L] - [IP2Location]
    for(MAP<LPSTR, Location_Struct*, LPSTR_Pred>::const_iterator it1=locationList.begin();it1!=locationList.end();++it1 )// X: [SUL] - [SpeedUpLoading]
    {
		//delete [] it1->first;
		delete it1->second;
	}
	locationList.clear();

	//first remove all country structs
	for (size_t i = 0; i < m_countryList.size(); i++)
		delete m_countryList[i];
	CCountryArray().swap(m_countryList);

	//now the ip structs
	CIP2CountryArray().swap(m_iplist);

	AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_FILELOAD));
}

void CIP2Country::RemoveAllFlags(){
	EnableCountryFlag = false;

	//destory all image
	CountryFlagImageList.DeleteImageList();

	//also clean out the map table
	CountryIDtoFlagIndex.clear();

	AddDebugLogLine(false, GetResString(IDS_IP2COUNTRY_FLAGUNLD));
}
Country_Struct* CIP2Country::AddCountry(const LPSTR shortCountryName, const LPSTR longCountryName){// X: [IP2L] - [IP2Location]
	//AddCountry
	Country_Struct* newCountry = new Country_Struct;
	//newCountry->MidCountryName = midCountryName;
	newCountry->LongCountryName = longCountryName;
	m_countryList.push_back(newCountry);

	//Add Flag
	if(EnableCountryFlag){

		MAP<LPSTR, uint16, LPSTR_Pred>::const_iterator it2 = CountryIDtoFlagIndex.find(shortCountryName);
		newCountry->FlagIndex = ((it2 != CountryIDtoFlagIndex.end()) ? it2->second : NO_FLAG);
	}
	return newCountry;
}

void CIP2Country::AddIPRange(uint32 IPfrom,uint32 IPto, const LPSTR shortCountryName/*, const CHAR* midCountryName*/, const LPSTR longCountryName){// X: [SUL] - [SpeedUpLoading]
	IPRange_Struct2 newRange;
	newRange.IPstart = IPfrom;
	newRange.IPend = IPto;

	MAP<LPSTR, Location_Struct*, LPSTR_Pred>::const_iterator it1 = locationList.find(longCountryName);// X: [IP2L] - [IP2Location]
	if (it1 == locationList.end()) {
		Location_Struct* newLocation = new Location_Struct;
		LPSTR locationEnd, coutryEnd;
		LPSTR longname = nstrdup(longCountryName);
		if((locationEnd=strrchr(longCountryName, ']')) && (coutryEnd=strchr(longCountryName, '['))){
			*locationEnd = 0;
			*coutryEnd = 0;
			newLocation->locationName = coutryEnd+1;
			it1 = locationList.find(longCountryName);
			if (it1 == locationList.end()){
				Location_Struct* newLocationParent = new Location_Struct;
				newLocationParent->country = AddCountry(shortCountryName, longCountryName);
				LPSTR longnameParent = nstrdup(longCountryName);
				locationList[longnameParent] = newLocationParent;// X: use LPSTR as key
				it1 = locationList.find(longnameParent);
			}
			newLocation->country = it1->second->country;
		}
		else{
			newLocation->country = AddCountry(shortCountryName, longCountryName);
		}
		locationList[longname] = newLocation;// X: use LPSTR as key
		it1 = locationList.find(longname);

	}
	ASSERT(it1!=locationList.end());
	newRange.location = it1->second;

	m_iplist.push_back(newRange);
}

static int __cdecl CmpIP2CountryByAddr(const void* pvKey, const void* pvElement)
{
	uint32 ip = *(uint32*)pvKey;
	const IPRange_Struct2* pIP2Country = (IPRange_Struct2*)pvElement;

	if (ip < pIP2Country->IPstart)
		return -1;
	if (ip > pIP2Country->IPend)
		return 1;
	return 0;
}

//struct 
Location_Struct* CIP2Country::GetLocationFromIP(uint32 ClientIP) const// X: [IP2L] - [IP2Location]
{

	if(EnableIP2Country == false || ClientIP == 0 || m_iplist.size() == 0)
		return &defaultLocation;

	ClientIP = htonl(ClientIP);
	IPRange_Struct2* ppFound = (IPRange_Struct2*)bsearch(&ClientIP, &m_iplist.at(0), m_iplist.size(), sizeof(m_iplist[0]), CmpIP2CountryByAddr);
	if (ppFound)
		return ppFound->location;

	return &defaultLocation;
}
CString& CIP2Country::GetCountryNameFromRef(Country_Struct* m_structCountry/*, bool longName*/){
	//if(EnableIP2Country)
	//{
		//if(longName)
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
	//}
	//else if(longName)
	//return defaultCountry.LongCountryName;	
	//return _T("");
}
bool CIP2Country::ShowCountryFlag() const
{

	return 
		//user wanna see flag,
		(// thePrefs.IsIP2CountryShowFlag() && //Xman if we wan't to make it optional
		//flag have been loaded
		EnableCountryFlag
		//ip table have been loaded
		//&& EnableIP2Country
		);
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