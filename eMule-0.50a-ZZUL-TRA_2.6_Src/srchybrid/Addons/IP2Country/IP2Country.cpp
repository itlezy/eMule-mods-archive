//this file is part of eMule
//Copyright (C)2010 Superlexx based on IPFilter by Bouc7
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "StdAfx.h"
#include <share.h>
#include "IP2Country.h"
#include "emule.h"
#include "otherfunctions.h"
#include <flag/resource.h>
#include "log.h"
#include <algorithm>
#include "serverlist.h"
#include "clientlist.h"
#include "emuledlg.h"
#include "serverwnd.h"
#include "serverlistctrl.h"
//#include "kademliawnd.h"
#include "HttpDownloadDlg.h"
#include "ZipFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int __cdecl CmpIP2CountryByAddr(const void* pvKey, const void* pvElement)
{
	UINT ip = *(UINT*)pvKey;
	const IPRange_Struct2* pIP2Country = *(IPRange_Struct2**)pvElement;

	if (ip < pIP2Country->IPstart)
		return -1;
	if (ip > pIP2Country->IPend)
		return 1;
	return 0;
}

static int __cdecl CmpIP2CountryByStartAddr(const void* p1, const void* p2)
{
	const IPRange_Struct2* rng1 = *(IPRange_Struct2**)p1;
	const IPRange_Struct2* rng2 = *(IPRange_Struct2**)p2;
	return CompareUnsigned(rng1->IPstart, rng2->IPstart);
}

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

CIP2Country::CIP2Country()
{
	m_bRunning = false;

	defaultIP2Country.IPstart = 0;
	defaultIP2Country.IPend = 0;
	defaultIP2Country.ShortCountryName = GetResString(IDS_IP2COUNTRY_NASHORT);
	defaultIP2Country.MidCountryName = GetResString(IDS_IP2COUNTRY_NASHORT);
	defaultIP2Country.LongCountryName = GetResString(IDS_IP2COUNTRY_NALONG);
	defaultIP2Country.FlagIndex = NO_FLAG;

	m_bEnableIP2Country = false;
	m_bEnableCountryFlag = false;
	_hCountryFlagDll = NULL;

	Load();

	theApp.QueueLogLineEx(LOG_WARNING, GetResString(IDS_IP2COUNTRY_MSG1));
	theApp.QueueLogLineEx(LOG_WARNING, GetResString(IDS_IP2COUNTRY_MSG2));
	m_bRunning = m_bEnableIP2Country && m_bEnableCountryFlag;
}

CIP2Country::~CIP2Country()
{
	m_bRunning = false;
	Unload();
}

void CIP2Country::Load()
{
	m_bEnableCountryFlag = LoadCountryFlagLib();//flag lib first, so ip range can map to flag
	if(m_bEnableCountryFlag)
		m_bEnableIP2Country = LoadFromFile();
	else
		m_bEnableIP2Country = false;

	if(!m_bEnableIP2Country || !m_bEnableCountryFlag)
		Unload();
	else //if(m_bRunning)  //not necessary to reset twice :)
		Reset();

	if(m_bEnableIP2Country && m_bEnableCountryFlag)
		theApp.QueueLogLineEx(LOG_SUCCESS, GetResString(IDS_IP2COUNTRY_LOADED));
}

void CIP2Country::Unload()
{
	m_bEnableIP2Country = false;
	m_bEnableCountryFlag = false;

	RemoveAllIPs();
	RemoveAllFlags();

	if(m_bRunning){
		Reset();
		theApp.QueueLogLineEx(LOG_WARNING, GetResString(IDS_IP2COUNTRY_UNLOADED));
	}
}

void CIP2Country::Reset()
{
	theApp.serverlist->ResetIP2Country();
	theApp.clientlist->ResetIP2Country();
	//theApp.emuledlg->kademliawnd->ResetIP2Country();
}

void CIP2Country::Refresh()
{
	theApp.emuledlg->serverwnd->serverlistctrl.RefreshAllServer();
	//theApp.emuledlg->kademliawnd->RedrawWindow();
}

bool CIP2Country::LoadFromFile()
{
	CString ip2countryCSVfile = GetDefaultFilePath();
	DWORD startMesure = GetTickCount();
	TCHAR szBuffer[1024];
	int	lenBuf = ARRSIZE(szBuffer);
	bool bRet = true;
	FILE* readFile = _tfsopen(ip2countryCSVfile, _T("r"), _SH_DENYWR);
	try{
		if (readFile != NULL) {
			int iCount = 0;
			int iLine = 0;
			int iDuplicate = 0;
			int iMerged = 0;
			uint32 uLastIP = 0;
			bool bError = false;
			bool bIsSorted = true;
			while (!feof(readFile)) {
				bError = false;
				if (_fgetts(szBuffer, lenBuf, readFile)==0) 
					break;
				CString sbuffer = szBuffer;
				++iLine;
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
				// we assume that the ip-to-country.csv is valid and doesn't cause any troubles
				// get & process IP range
				sbuffer.Remove(L'"'); // get rid of the " signs
				
				CString tempStr[5];
				int curPos = 0;
				for(int forCount = 0; forCount < 5; forCount++)
				{
					tempStr[forCount] = sbuffer.Tokenize(_T(","), curPos);
					if(tempStr[forCount].IsEmpty()) 
					{
						if(forCount == 0 || forCount == 1) 
						{
							bError = true; //no empty ip field
							break;
						}
						//no need to throw an exception, keep reading in next line
						//throw CString(_T("error line in"));
					}
				}
				if(bError)
				{
					theApp.QueueLogLineEx(LOG_ERROR, GetResString(IDS_IP2COUNTRY_ERROR1), iCount+1);
					theApp.QueueLogLineEx(LOG_ERROR, _T("%s %s"), GetResString(IDS_IP2COUNTRY_ERROR2), ip2countryCSVfile);
					continue;
				}
				//tempStr[4] is full country name, capitalize country name from rayita
				FirstCharCap(&tempStr[4]);

				++iCount;

				AddIPRange((uint32)_tstoi(tempStr[0]),(uint32)_tstoi(tempStr[1]), tempStr[2], tempStr[3], tempStr[4]);

				if (uLastIP > (uint32)_tstoi(tempStr[0]))
					bIsSorted = false;
				uLastIP = (uint32)_tstoi(tempStr[0]);
			}
			fclose(readFile);

#ifdef _DEBUG  
			for(POSITION pos = CountryIDtoFlagIndex.GetHeadPosition(); pos;){  
				const CRBMap<CString, flaginfostruct>::CPair* pair = CountryIDtoFlagIndex.GetNext(pos);  
				if(pair->m_value.usedcount == 0)  
					theApp.QueueLogLineEx(LOG_WARNING, L"Country name \"%s\" is not used in .csv", pair->m_key);  
			}  
#endif

			// sort the IP2Country list by IP range start addresses
			if(!bIsSorted)
				//qsort(m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpIP2CountryByStartAddr);
				std::sort(m_iplist.begin(), m_iplist.end(), CmpIP2CountryByStartAddr);

			if (m_iplist.size() >= 2)
			{
				// On large IP2Country lists there is a noticeable performance problem when merging the list.
				// The 'CIPFilterArray::RemoveAt' call is way too expensive to get called during the merging,
				// thus we use temporary helper arrays to copy only the entries into the final list which
				// are not get deleted.

				// Reserve a byte array (its used as a boolean array actually) as large as the current 
				// IP2Country list, so we can set a 'to delete' flag for each entry in the current IP2Country list.
				char* pcToDelete = NULL;
				int iNumToDelete = 0;

				IPRange_Struct2* pPrv = m_iplist[0];
				for(size_t i = 1; i < m_iplist.size(); ++i)
				{
					IPRange_Struct2* pCur = m_iplist[i];
					if (   pCur->IPstart >= pPrv->IPstart && pCur->IPstart <= pPrv->IPend	 // overlapping
						|| pCur->IPstart == pPrv->IPend+1 && &pCur->ShortCountryName == &pPrv->ShortCountryName) // adjacent
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
						//m_iplist.RemoveAt(i); // way too expensive (read above)
						if(pcToDelete == NULL)
						{
							pcToDelete = new char[m_iplist.size()];
							memset(pcToDelete, 0, m_iplist.size());
						}
						pcToDelete[i] = 1;		// mark this entry as 'to delete'
						++iNumToDelete;
						continue;
					}
					pPrv = pCur;
					++i;
				}

				// Create new IP-filter list which contains only the entries from the original IP-filter list
				// which are not to be deleted.
				if (iNumToDelete > 0)
				{
					//CIPFilterArray newList;
					int iNewListIndex = 0;
					for (size_t i = 0; i < m_iplist.size(); i++)
					{
						if (!pcToDelete[i])
						{
							if(iNewListIndex != (int)i)
								m_iplist[iNewListIndex] = m_iplist[i];
							++iNewListIndex;
						}
					}
					m_iplist.erase(m_iplist.end() - iNumToDelete, m_iplist.end());
					ASSERT( iNewListIndex == (int)m_iplist.size() );

					// Replace current list with new list. Dump, but still fast enough (only 1 memcpy)
					//m_iplist.RemoveAll();
					//m_iplist.Append(newList);
					//newList.RemoveAll();
					delete[] pcToDelete;
				}
			}

			if (thePrefs.GetVerbose())
			{
				theApp.QueueLogLineEx(LOG_SUCCESS, GetResString(IDS_IP2COUNTRY_LOADED2), ip2countryCSVfile, CastSecondsToHM((::GetTickCount()-startMesure)/1000));
				theApp.QueueLogLineEx(LOG_SUCCESS, GetResString(IDS_IP2COUNTRY_INFO), iLine, iCount, iDuplicate, iMerged);
			}

		}
		else{
			theApp.QueueLogLineEx(LOG_WARNING, _T("%s %s"), GetResString(IDS_IP2COUNTRY_ERROR3), ip2countryCSVfile);
			bRet = false;
		}
	}
	catch(...)
	{
		theApp.QueueLogLineEx(LOG_ERROR, _T("%s %s"), GetErrorMessage(GetLastError()), ip2countryCSVfile);
		bRet = false;
	}
	return bRet;
}

bool CIP2Country::LoadCountryFlagLib()
{
	CString ip2countryCountryFlag;
	bool bRet = true;
	try{
		ASSERT( CountryIDtoFlagIndex.GetCount() == 0);
		CountryIDtoFlagIndex.RemoveAll();		
		//shouldn't contain images either...
		ASSERT( CountryFlagImageList.GetSafeHandle() == NULL || CountryFlagImageList.GetImageCount() == 0);
		_hCountryFlagDll = NULL; //just to be sure
		//detect windows version
		if(IsRunning32BitOS()){
			//it's XP, we can use beautiful 32bits flags with alpha channel :)
			ip2countryCountryFlag = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("countryflag32.dll");
			_hCountryFlagDll = LoadLibrary(ip2countryCountryFlag);
		}
		//Fail safe - if 32bit not possible proceed with 24bit
		if (_hCountryFlagDll == NULL){
			//oh~ it's not XP, but we still can load the 24bits flags
			ip2countryCountryFlag = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("countryflag.dll");
			_hCountryFlagDll = LoadLibrary(ip2countryCountryFlag); 
		}
		if (_hCountryFlagDll == NULL) { 
			throw CString(GetResString(IDS_IP2COUNTRY_ERROR4));
		} 

		const uint16 resIDs[] = {
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
			IDI_COUNTRY_FLAG_ML, IDI_COUNTRY_FLAG_MM, IDI_COUNTRY_FLAG_MN, IDI_COUNTRY_FLAG_MR, 
			IDI_COUNTRY_FLAG_MP, IDI_COUNTRY_FLAG_MQ, IDI_COUNTRY_FLAG_MR, IDI_COUNTRY_FLAG_MS, 
			IDI_COUNTRY_FLAG_MT, IDI_COUNTRY_FLAG_MU, IDI_COUNTRY_FLAG_MV, IDI_COUNTRY_FLAG_MW, 
			IDI_COUNTRY_FLAG_MX, IDI_COUNTRY_FLAG_MY, IDI_COUNTRY_FLAG_MZ, IDI_COUNTRY_FLAG_NA, 
			IDI_COUNTRY_FLAG_NC, IDI_COUNTRY_FLAG_NE, IDI_COUNTRY_FLAG_NF, IDI_COUNTRY_FLAG_NG, 
			IDI_COUNTRY_FLAG_NI, IDI_COUNTRY_FLAG_NL, IDI_COUNTRY_FLAG_NO, IDI_COUNTRY_FLAG_NP, 
			IDI_COUNTRY_FLAG_NR, IDI_COUNTRY_FLAG_NU, IDI_COUNTRY_FLAG_NZ, IDI_COUNTRY_FLAG_OM, 
			IDI_COUNTRY_FLAG_PA, IDI_COUNTRY_FLAG_PE, IDI_COUNTRY_FLAG_PF, 
			IDI_COUNTRY_FLAG_PG, IDI_COUNTRY_FLAG_PH, IDI_COUNTRY_FLAG_PK, IDI_COUNTRY_FLAG_PL, 
			IDI_COUNTRY_FLAG_PM, IDI_COUNTRY_FLAG_PN, IDI_COUNTRY_FLAG_PR, IDI_COUNTRY_FLAG_PS, 
			IDI_COUNTRY_FLAG_PT, IDI_COUNTRY_FLAG_PW, IDI_COUNTRY_FLAG_PY, IDI_COUNTRY_FLAG_QA, 
			IDI_COUNTRY_FLAG_RO, IDI_COUNTRY_FLAG_RU, IDI_COUNTRY_FLAG_RW, IDI_COUNTRY_FLAG_SA, 
			IDI_COUNTRY_FLAG_SB, IDI_COUNTRY_FLAG_SC, IDI_COUNTRY_FLAG_SD, IDI_COUNTRY_FLAG_SE, 
			IDI_COUNTRY_FLAG_SG, IDI_COUNTRY_FLAG_SH, IDI_COUNTRY_FLAG_SI, IDI_COUNTRY_FLAG_SK, 
			IDI_COUNTRY_FLAG_SL, IDI_COUNTRY_FLAG_SM, IDI_COUNTRY_FLAG_SN, IDI_COUNTRY_FLAG_SO, 
			IDI_COUNTRY_FLAG_SR, IDI_COUNTRY_FLAG_ST, IDI_COUNTRY_FLAG_SV, 
			IDI_COUNTRY_FLAG_SY, IDI_COUNTRY_FLAG_SZ, IDI_COUNTRY_FLAG_TC, IDI_COUNTRY_FLAG_TD, 
			IDI_COUNTRY_FLAG_TF, IDI_COUNTRY_FLAG_TG, IDI_COUNTRY_FLAG_TH, IDI_COUNTRY_FLAG_TJ, 
			IDI_COUNTRY_FLAG_TK, IDI_COUNTRY_FLAG_TL, IDI_COUNTRY_FLAG_TM, IDI_COUNTRY_FLAG_TN, 
			IDI_COUNTRY_FLAG_TO, IDI_COUNTRY_FLAG_TR, IDI_COUNTRY_FLAG_TT, IDI_COUNTRY_FLAG_TV, 
			IDI_COUNTRY_FLAG_TW, IDI_COUNTRY_FLAG_TZ, IDI_COUNTRY_FLAG_UA, IDI_COUNTRY_FLAG_UG, 
			IDI_COUNTRY_FLAG_US, IDI_COUNTRY_FLAG_US, IDI_COUNTRY_FLAG_UY, IDI_COUNTRY_FLAG_UZ, 
			IDI_COUNTRY_FLAG_VA, IDI_COUNTRY_FLAG_VC, IDI_COUNTRY_FLAG_VE, IDI_COUNTRY_FLAG_VG, 
			IDI_COUNTRY_FLAG_VI, IDI_COUNTRY_FLAG_VN, IDI_COUNTRY_FLAG_VU, IDI_COUNTRY_FLAG_WF, 
			IDI_COUNTRY_FLAG_WS, IDI_COUNTRY_FLAG_YE, IDI_COUNTRY_FLAG_ZA, 
			IDI_COUNTRY_FLAG_ZM, IDI_COUNTRY_FLAG_ZW,
			IDI_COUNTRY_FLAG_RS, //by JvA (eMule+)
			IDI_COUNTRY_FLAG_AX, //by JvA (eMule+)
			IDI_COUNTRY_FLAG_ME, //by JvA (eMule+)
			IDI_COUNTRY_FLAG_FR, //by JvA (WiZaRd)
			IDI_COUNTRY_FLAG_FR, //by JvA (WiZaRd)
			IDI_COUNTRY_FLAG_FR, //by JvA (WiZaRd)
			IDI_COUNTRY_FLAG_AQ, //by JvA (WiZaRd)
			IDI_COUNTRY_FLAG_CS, //by JvA
			IDI_COUNTRY_FLAG_GB,	// by JvA
			IDI_COUNTRY_FLAG_FR,	// by JvA
			IDI_COUNTRY_FLAG_NO  //by JvA
		};

		CString strIDs[] = {
			L"N/A",//first res in image list should be N/A

			L"AD", L"AE", L"AF", L"AG", L"AI", L"AL", L"AM", L"AN", L"AO", L"AR", L"AS", L"AT", L"AU", L"AW", L"AZ", 
			L"BA", L"BB", L"BD", L"BE", L"BF", L"BG", L"BH", L"BI", L"BJ", L"BM", L"BN", L"BO", L"BR", L"BS", L"BT", 
			L"BW", L"BY", L"BZ", L"CA", L"CC", L"CD", L"CF", L"CG", L"CH", L"CI", L"CK", L"CL", L"CM", L"CN", L"CO", 
			L"CR", L"CU", L"CV", L"CX", L"CY", L"CZ", L"DE", L"DJ", L"DK", L"DM", L"DO", L"DZ", L"EC", L"EE", L"EG", 
			L"EH", L"ER", L"ES", L"ET", L"FI", L"FJ", L"FK", L"FM", L"FO", L"FR", L"GA", L"GB", L"GD", L"GE", L"GG", 
			L"GH", L"GI", L"GK", L"GL", L"GM", L"GN", L"GP", L"GQ", L"GR", L"GS", L"GT", L"GU", L"GW", L"GY", L"HK", 
			L"HN", L"HR", L"HT", L"HU", L"ID", L"IE", L"IL", L"IM", L"IN", L"IO", L"IQ", L"IR", L"IS", L"IT", L"JE", 
			L"JM", L"JO", L"JP", L"KE", L"KG", L"KH", L"KI", L"KM", L"KN", L"KP", L"KR", L"KW", L"KY", L"KZ", L"LA", 
			L"LB", L"LC", L"LI", L"LK", L"LR", L"LS", L"LT", L"LU", L"LV", L"LY", L"MA", L"MC", L"MD", L"MG", L"MH", 
			L"MK", L"ML", L"MM", L"MN", L"MO", L"MP", L"MQ", L"MR", L"MS", L"MT", L"MU", L"MV", L"MW", L"MX", L"MY", 
			L"MZ", L"NA", L"NC", L"NE", L"NF", L"NG", L"NI", L"NL", L"NO", L"NP", L"NR", L"NU", L"NZ", L"OM", L"PA", 
			L"PE", L"PF", L"PG", L"PH", L"PK", L"PL", L"PM", L"PN", L"PR", L"PS", L"PT", L"PW", L"PY", L"QA", 
			L"RO", L"RU", L"RW", L"SA", L"SB", L"SC", L"SD", L"SE", L"SG", L"SH", L"SI", L"SK", L"SL", L"SM", L"SN", 
			L"SO", L"SR", L"ST", L"SV", L"SY", L"SZ", L"TC", L"TD", L"TF", L"TG", L"TH", L"TJ", L"TK", L"TL", 
			L"TM", L"TN", L"TO", L"TR", L"TT", L"TV", L"TW", L"TZ", L"UA", L"UG", L"UM", L"US", L"UY", L"UZ", L"VA", 
			L"VC", L"VE", L"VG", L"VI", L"VN", L"VU", L"WF", L"WS", L"YE", L"ZA", L"ZM", L"ZW",
			L"RS", //by JvA (eMule+)
			L"AX", //by JvA (eMule+)
			L"ME", //by JvA (eMule+)
			L"YT", //by JvA (WiZaRd)
			L"GF", //by JvA (WiZaRd)
			L"RE", //by JvA (WiZaRd)
			L"AQ", //by JvA (WiZaRd)
			L"CS", //by JvA
			L"UK",	// by JvA
			L"MF",	// by JvA
			L"BV"  //by JvA

		};

		CountryFlagImageList.DeleteImageList();
		CountryFlagImageList.Create(18, 16, theApp.m_iDfltImageListColorFlags|ILC_MASK, 0, 1);
		CountryFlagImageList.SetBkColor(CLR_NONE);

		HICON iconHandle = NULL; 
		int iconIndex = -1;
		for(int i = 0; i != _countof(resIDs); i++){
			iconHandle = (HICON)::LoadImage(_hCountryFlagDll, MAKEINTRESOURCE(resIDs[i]), IMAGE_ICON, 18, 16, LR_DEFAULTCOLOR);
			if(iconHandle) {
				iconIndex = CountryFlagImageList.Add(iconHandle);
				if(iconIndex != -1)
					CountryIDtoFlagIndex.SetAt(strIDs[i], (uint16)iconIndex);
				::DestroyIcon(iconHandle);
			} else
				theApp.QueueLogLineEx(LOG_WARNING, GetResString(IDS_IP2COUNTRY_ERROR5), resIDs[i]);
		}
		theApp.QueueLogLineEx(LOG_SUCCESS, GetResString(IDS_IP2COUNTRY_FLAGLOAD));
	}
	catch(CString error)
	{
		theApp.QueueLogLineEx(LOG_ERROR, L"IP2COUNTRY: error %s in %s", error, ip2countryCountryFlag);
		bRet = false;
	}
	catch(...)
	{
		theApp.QueueLogLineEx(LOG_ERROR, L"IP2COUNTRY: error %s in %s", GetErrorMessage(GetLastError()), ip2countryCountryFlag);
		bRet = false;
	}

	//free lib
	if(_hCountryFlagDll != NULL) 
		FreeLibrary(_hCountryFlagDll);

	return bRet;
}

void CIP2Country::RemoveAllIPs()
{
	for (size_t i = 0; i < m_iplist.size(); i++)
		delete m_iplist[i];
	m_iplist.clear();

	if(m_bRunning)
		theApp.QueueLogLineEx(LOG_WARNING, GetResString(IDS_IP2COUNTRY_FILELOAD));
}

void CIP2Country::RemoveAllFlags()
{
	//destroy all images
	CountryFlagImageList.DeleteImageList();

	//also clean out the map table
	CountryIDtoFlagIndex.RemoveAll();

	if(m_bRunning)
		theApp.QueueLogLineEx(LOG_WARNING, GetResString(IDS_IP2COUNTRY_FLAGUNLD));
}

void CIP2Country::AddIPRange(uint32 IPfrom,uint32 IPto, CString& shortCountryName, CString& midCountryName, CString& longCountryName)
{
	IPRange_Struct2* newRange = new IPRange_Struct2();
	newRange->IPstart = IPfrom;
	newRange->IPend = IPto;
	newRange->ShortCountryName = shortCountryName;
	newRange->MidCountryName = midCountryName;
	newRange->LongCountryName = longCountryName;

	if(m_bEnableCountryFlag){
#ifdef _DEBUG
		CRBMap<CString, flaginfostruct>::CPair* pair = CountryIDtoFlagIndex.Lookup(shortCountryName);
#else
		const CRBMap<CString, uint16>::CPair* pair = CountryIDtoFlagIndex.Lookup(shortCountryName);
#endif
		if(pair != NULL){
#ifdef _DEBUG
			newRange->FlagIndex = pair->m_value.flagindex;
			pair->m_value.usedcount++;
		} else
			theApp.QueueLogLineEx(LOG_WARNING, L"Country name \"%s\" (%s) was not found in icon library (maybe too new?)", longCountryName, shortCountryName);
#else
			newRange->FlagIndex = pair->m_value;
		}
#endif
	}
	m_iplist.push_back(newRange);
}

struct IPRange_Struct2* CIP2Country::GetCountryFromIP(uint32 ClientIP)
{
	if(m_bEnableIP2Country == false || ClientIP == 0)
		return &defaultIP2Country;
	if(m_iplist.size() == 0)
	{
		theApp.QueueLogLineEx(LOG_ERROR, L"CIP2Country::GetCountryFromIP iplist doesn't exist");
		return &defaultIP2Country;
	}
	ClientIP = htonl(ClientIP);
	IPRange_Struct2** ppFound = (IPRange_Struct2**)bsearch(&ClientIP, &m_iplist.at(0), m_iplist.size(), sizeof(m_iplist[0]), CmpIP2CountryByAddr);
	if (ppFound)
		return *ppFound;

	return &defaultIP2Country;
}

CString CIP2Country::GetCountryNameFromRef(IPRange_Struct2* m_structCountry, bool longName)
{
	if(m_bEnableIP2Country)
	{
		if(longName)
			return m_structCountry->LongCountryName;
		switch(thePrefs.GetIP2CountryNameMode()){
			case IP2CountryName_SHORT:
				return m_structCountry->ShortCountryName;
			case IP2CountryName_MID:
				return m_structCountry->MidCountryName;
			case IP2CountryName_LONG:
				return m_structCountry->LongCountryName;
		}
	}
	else if(longName)
		return GetResString(IDS_DISABLED);	
	return _T("");
}

bool CIP2Country::ShowCountryFlag() const
{
	return 
		//user wanna see flag,
		(thePrefs.IsIP2CountryShowFlag()
		//flag have been loaded
		&& m_bEnableCountryFlag
		//ip table have been loaded
		&& m_bEnableIP2Country);
}
/*
void CIP2Country::UpdateIP2CountryURL()
{   
	CString sbuffer;
	CString strURL = thePrefs.GetIP2CountryUpdateVerURL(); //Version URL to keep it separated

	TCHAR szTempFilePath[_MAX_PATH];
	_tmakepath(szTempFilePath, NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IP2COUNTRY_FILENAME, _T("tmp"));

	//>> pP: fix -- delete old tmp files
	if (PathFileExists(szTempFilePath))
	{
		theApp.QueueLogLineEx(LOG_ERROR | LOG_STATUSBAR, L"Deleted orphaned ip2c tmp file.");
		_tremove(szTempFilePath);
	}
	//<< pP: fix -- delete old tmp files

	FILE* readFile= _tfsopen(szTempFilePath, _T("r"), _SH_DENYWR);

	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = GetResString(IDS_IP2COUNTRY_DWNFILE);
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = szTempFilePath;
	if (dlgDownload.DoModal() != IDOK){
		_tremove(szTempFilePath);
		theApp.QueueLogLineEx(LOG_ERROR, GetResString(IDS_LOG_ERRDWN), strURL);
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
	if ((thePrefs.GetIP2CountryVersion() < (uint32)_tstoi(sbuffer)) || !PathFileExists(GetDefaultFilePath())) {
		
		CString IP2CountryURL = thePrefs.GetIP2CountryUpdateURL();
		
		_tmakepath(szTempFilePath, NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IP2COUNTRY_FILENAME, _T("tmp"));

		CHttpDownloadDlg dlgDownload;
		dlgDownload.m_strTitle = GetResString(IDS_IP2COUNTRY_DWNFILE);
		dlgDownload.m_sURLToDownload = IP2CountryURL;
		dlgDownload.m_sFileToDownloadInto = szTempFilePath;
		SYSTEMTIME SysTime;
		if (PathFileExists(GetDefaultFilePath()))
			memcpy(&SysTime, thePrefs.GetIP2CountryVersion2(), sizeof(SYSTEMTIME));
		else
			memset(&SysTime, 0, sizeof(SYSTEMTIME));
		dlgDownload.m_pLastModifiedTime = &SysTime;

		if (dlgDownload.DoModal() != IDOK){
			_tremove(szTempFilePath);
			theApp.QueueLogLineEx(LOG_ERROR | LOG_STATUSBAR, GetResString(IDS_IP2COUNTRY_ERROR6));
			return;
		}

		if (dlgDownload.m_pLastModifiedTime == NULL)
			return;
        
		bool bIsZipFile = false;
		bool bUnzipped = false;
		CZIPFile zip;
		if (zip.Open(szTempFilePath)){
			bIsZipFile = true;

			CZIPFile::File* zfile = zip.GetFile(DFLT_IP2COUNTRY_FILENAME); // It has to be a zip-file which includes a file called: ip-to-country.csv
			if (zfile){
				TCHAR szTempUnzipFilePath[MAX_PATH];
				_tmakepath(szTempUnzipFilePath, NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IP2COUNTRY_FILENAME, _T(".unzip.tmp"));
				if (zfile->Extract(szTempUnzipFilePath)){
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
					theApp.QueueLogLineEx(LOG_ERROR | LOG_STATUSBAR, GetResString(IDS_IP2COUNTRY_ERROR7), szTempFilePath);
			}
			else
				theApp.QueueLogLineEx(LOG_ERROR | LOG_STATUSBAR, GetResString(IDS_IP2COUNTRY_ERROR8), szTempFilePath); //File not found inside the zip-file

			zip.Close();
		}
        
		if (!bIsZipFile && !bUnzipped){
			_tremove(GetDefaultFilePath());
			_trename(szTempFilePath, GetDefaultFilePath());
		}

		if(bIsZipFile && !bUnzipped)
			return;

		theApp.ip2country->Unload();
		theApp.QueueLogLineEx(LOG_WARNING,GetResString(IDS_IP2COUNTRY_UPUNLOAD));
		theApp.ip2country->Load();
		theApp.QueueLogLineEx(LOG_WARNING,GetResString(IDS_IP2COUNTRY_UPLOAD));

		thePrefs.SetIP2CountryVersion(_tstoi(sbuffer));
		memcpy(thePrefs.GetIP2CountryVersion2(), &SysTime, sizeof SysTime);
		thePrefs.Save();
	}
}
*/
CString CIP2Country::GetDefaultFilePath() const
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_IP2COUNTRY_FILENAME;
}