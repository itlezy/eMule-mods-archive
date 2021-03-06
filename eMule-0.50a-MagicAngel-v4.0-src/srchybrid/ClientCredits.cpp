//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "stdafx.h"
#include <math.h>
#include "emule.h"
#include "ClientCredits.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "Opcodes.h"
#include "Sockets.h"
#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4702) // unreachable code
#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>
#include <cryptopp/files.h>
#include <cryptopp/sha.h>
#pragma warning(default:4702) // unreachable code
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#include "emuledlg.h"
#include "Log.h"
//MORPH START - new includes
#include "ClientList.h"
#include "UpDownClient.h"
//MORPH END   - new includes
#include "ClientList.h" // => Extended credit-table-arragement [Xman] - sFrQlXeRt
// ==> new credit system - Stulle
#include "SharedFileList.h"
// <== new credit system - Stulle
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#define CLIENTS_MET_FILENAME	_T("clients.met")

CClientCredits::CClientCredits(CreditStruct* in_credits)
{
	// Moonlight: Dynamic ClientStruct - adjust structure element offsets.//Morph - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	m_pCredits = in_credits;
	InitalizeIdent();
	m_dwUnSecureWaitTime = 0;
	m_dwSecureWaitTime = 0;
	m_dwWaitTimeIP = 0;
	m_bmarktodelete = false; // => Extended credit-table-arragement [Xman] - sFrQlXeRt

	//Morph Start - Added by AndCycle, reduce a little CPU usage for ratio count
	m_bCheckScoreRatio = true;
	//Removed by SiRoB , for speedup creditfile load
	/*
	m_fLastScoreRatio = 0;
	*/
	//Morph End - Added by AndCycle, reduce a little CPU usage for ratio count
	//Moved by SiRoB , for speedup creditfile load, now in AddClientToQueue
	//InitPayBackFirstStatus();//EastShare - added by AndCycle, Pay Back First
}

CClientCredits::CClientCredits(const uchar* key)
{
	m_pCredits = new CreditStruct;
	memset(m_pCredits, 0, sizeof(CreditStruct));
	md4cpy(m_pCredits->abyKey, key);
	InitalizeIdent();

	// EastShare START - Modified by TAHO, modified SUQWT
	//m_dwUnSecureWaitTime = ::GetTickCount();
	//m_dwSecureWaitTime = ::GetTickCount();
	m_dwUnSecureWaitTime = 0;
	m_dwSecureWaitTime = 0;
	// EastShare END - Modified by TAHO, modified SUQWT

	m_dwWaitTimeIP = 0;
	m_bmarktodelete = false; // => Extended credit-table-arragement [Xman] - sFrQlXeRt

	//Morph Start - Added by AndCycle, reduce a little CPU usage for ratio count
	m_bCheckScoreRatio = true;
	//Removed by SiRoB , for speedup creditfile load
	/*
	m_fLastScoreRatio = 0;
	*/
	//Morph End - Added by AndCycle, reduce a little CPU usage for ratio count

	//Moved by SiRoB , for speedup creditfile load, now in AddClientToQueue
	//InitPayBackFirstStatus();//EastShare - added by AndCycle, Pay Back First
}

CClientCredits::~CClientCredits()
{
	delete m_pCredits;
}

void CClientCredits::AddDownloaded(uint32 bytes, uint32 dwForIP) {
	//MORPH START - Changed by SIRoB, Code Optimization 
	/*
	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
	*/
	if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
	//MORPH END   - Changed by SIRoB, Code Optimization 
		return;
	}

	//encode
	uint64 current = (((uint64)m_pCredits->nDownloadedHi << 32) | m_pCredits->nDownloadedLo) + bytes;

	//recode
	m_pCredits->nDownloadedLo=(uint32)current;
	m_pCredits->nDownloadedHi=(uint32)(current>>32);

	//is it good to refresh PayBackFirst status here?
	TestPayBackFirstStatus();//EastShare - added by AndCycle, Pay Back First

	m_bCheckScoreRatio = true;//Morph - Added by AndCycle, reduce a little CPU usage for ratio count
}

void CClientCredits::AddUploaded(uint32 bytes, uint32 dwForIP) {
	//MORPH START - Changed by SIRoB, Code Optimization 
	/*
	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
	*/
	if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
	//MORPH END   - Changed by SIRoB, Code Optimization 
		return;
	}

	//encode
	uint64 current = (((uint64)m_pCredits->nUploadedHi << 32) | m_pCredits->nUploadedLo) + bytes;

	//recode
	m_pCredits->nUploadedLo=(uint32)current;
	m_pCredits->nUploadedHi=(uint32)(current>>32);

	//is it good to refresh PayBackFirst status here?
	TestPayBackFirstStatus();//EastShare - added by AndCycle, Pay Back First

	m_bCheckScoreRatio = true;//Morph - Added by AndCycle, reduce a little CPU usage for ratio count
}

uint64	CClientCredits::GetUploadedTotal() const{
	return ((uint64)m_pCredits->nUploadedHi << 32) | m_pCredits->nUploadedLo;
}

uint64	CClientCredits::GetDownloadedTotal() const{
	return ((uint64)m_pCredits->nDownloadedHi << 32) | m_pCredits->nDownloadedLo;
}

float CClientCredits::GetScoreRatio(uint32 dwForIP) /*const*/
{

// ==> moved into switch for new credit systems - Stulle
//	// check the client ident status
//	//MORPH START - Changed by SIRoB, Code Optimization 
//	/*
//	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
//	*/
//	if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
//	//MORPH END   - Changed by SIRoB, Code Optimization 
//		// bad guy - no credits for you
//		return 1.0F;
//	}
	EIdentState currentIDstate =  GetCurrentIdentState(dwForIP);
	bool bBadGuy = false;
// <== moved into switch for new credit systems - Stulle

	//Morph Start - Added by AndCycle, reduce a little CPU usage for ratio count
	if(m_bCheckScoreRatio == false){//only refresh ScoreRatio when really need
		return m_fLastScoreRatio;
	}
	m_bCheckScoreRatio = false;
	//Morph End - Added by AndCycle, reduce a little CPU usage for ratio count

	//Morph Start - Modified by AndCycle, reduce a little CPU usage for ratio count

	double result = 0.0;//everybody share one result. leuk_he:doublw to prevent underflow in CS_LOVELACE.
    //EastShare START - Added by linekin, CreditSystem 
	switch (thePrefs.GetCreditSystem())	{

		// EastShare - Added by linekin, lovelace Credit
		case CS_LOVELACE:{
			if ( currentIDstate != IS_IDENTIFIED  && currentIDstate != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
				result = 0.984290578f; // use the CS' default value
				bBadGuy = true;
				break;
			}

			// new creditsystem by [lovelace]
			double cl_up,cl_down; 

			cl_up = GetUploadedTotal()/(double)1048576;
			cl_down = GetDownloadedTotal()/(double)1048576;
			result=(float)(3.0 * cl_down * cl_down - cl_up * cl_up);
			if (fabs(result)>20000.0f) 
				result*=20000.0/fabs(result);
			result=100.0*pow((1-1/(1.0f+exp(result*0.001))),6.6667);
			if (result<0.1) 
				result=0.1;
			if (result>10.0 && IdentState == IS_NOTAVAILABLE)
				result=10.0;
			// end new creditsystem by [lovelace]
		}break;

		//EastShare Start - added by AndCycle, Pawcio credit
		case CS_PAWCIO:{	
			if ( currentIDstate != IS_IDENTIFIED  && currentIDstate != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
				result = 1.0f;
				bBadGuy = true;
				break;
			}

			//Pawcio: Credits
			if ((GetDownloadedTotal() < 1000000)&&(GetUploadedTotal() > 1000000)){
				result = 1.0f;
				break;
			}
			else if ((GetDownloadedTotal() < 1000000)&&(GetUploadedTotal()<1000000)) {
				result = 3.0f;
				break;
			}
			result = 0.0F;
			if (GetUploadedTotal()<1000000)
				result = 10.0f * GetDownloadedTotal()/1000000.0f;
			else
				result = (float)(GetDownloadedTotal()*3)/GetUploadedTotal();
			if ((GetDownloadedTotal() > 100000000)&&(GetUploadedTotal()<GetDownloadedTotal()+8000000)&&(result<50)) result=50;
			else if ((GetDownloadedTotal() > 50000000)&&(GetUploadedTotal()<GetDownloadedTotal()+5000000)&&(result<25)) result=25;
			else if ((GetDownloadedTotal() > 25000000)&&(GetUploadedTotal()<GetDownloadedTotal()+3000000)&&(result<12)) result=12;
			else if ((GetDownloadedTotal() > 10000000)&&(GetUploadedTotal()<GetDownloadedTotal()+2000000)&&(result<5)) result=5;
			if (result > 100.0f){
				result = 100.0f;
				break;
			}
			if (result < 1.0f){
				result = 1.0f;
				break;
			}
		}break;
		//EastShare End - added by AndCycle, Pawcio credit

		// ==> Ratio Creditsystem - sFrQlXeRt
		case CS_RATIO: // RT.10a mod Credit
		{
			if ( currentIDstate != IS_IDENTIFIED  && currentIDstate != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
				result = 1.0f;
				bBadGuy = true;
				break;
			}

			// check the client ident status
			double UploadedTotalMB = (double)GetUploadedTotal() / 1048576.0;
			double DownloadedTotalMB = (double)GetDownloadedTotal() / 1048576.0;
			if (DownloadedTotalMB <= 1){
				if (UploadedTotalMB <= 1)
					result = 1;
				else
					result = (float)(1 / sqrt((double)UploadedTotalMB) );
				break;
			}

			if (UploadedTotalMB > 1){
				double Basic = (double)sqrt( (double)(DownloadedTotalMB + 1) );
				if (DownloadedTotalMB > UploadedTotalMB){
					result = ( Basic + (double)sqrt((double)(DownloadedTotalMB - UploadedTotalMB)) );
				}
				else{
					if ( (UploadedTotalMB - DownloadedTotalMB) <= 1 ){
						result = Basic;
						break;
					}
					double Result = ( Basic / (double)sqrt((double)(UploadedTotalMB - DownloadedTotalMB)) );
					if (DownloadedTotalMB >= 9){
						double Lowest = 0.7f + (Basic / 10);
						if (Result < Lowest)   Result = Lowest;
					}
					else{
						if (Result < 1)   Result = DownloadedTotalMB / 9;
					}
/* RT only
					if ( (thePrefs.GetMaxCredit1Slot() > 0) && (Result > 1) ){
						if ( (UploadedTotalMB - DownloadedTotalMB) > (Basic * 2) )   Result = 1;
					}
*/
					result = Result;
				}
				break;
			}
			else    
				result = DownloadedTotalMB;
		}break;
		// <== Ratio Creditsystem - sFrQlXeRt
		// EastShare START - Added by TAHO, new Credit System //Modified by Pretender
		case CS_EASTSHARE:{
			result = (IdentState == IS_NOTAVAILABLE) ? 80 : 100; // stay with the original - Stulle
			
			result += (float)((double)GetDownloadedTotal()/174762.67 - (double)GetUploadedTotal()/524288); //Modefied by Pretender - 20040120
			
			if ((double)GetDownloadedTotal() > 1048576) {
				result += 100; 
				if (result<50 && ((double)GetDownloadedTotal()*10 > (double)GetUploadedTotal())) result=50;
				} //Modefied by Pretender - 20040330

			if ( result < 10 ) {
				result = 10;
			}else if ( result > 5000 ) {
				result = 5000;
			}
			result = result / 100;

		}break;
		// EastShare END - Added by TAHO, new Credit System

		// ==> Magic Angel Creditsystem - sFrQlXeRt
		case CS_MAGICANGEL:
		{
			if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
				result = 1.0f;
				bBadGuy = true;
				break;
			}

			uint64 uDownloadedTotal = GetDownloadedTotal();
			uint64 uUploadedTotal = GetUploadedTotal();

			if (uDownloadedTotal < 1650000 && uUploadedTotal == 0){
				result = 1.0F;
				break;
			}
			if (!uUploadedTotal)
				result = 50.0F;
			else {
				if (uDownloadedTotal < 1048576) // 1MB
					// We give the client 1 MB upload for the modifier calculation.
					result = (float)(((double)1048576*2.0)/(double)uUploadedTotal);
				else
					result = (float)(((double)uDownloadedTotal*2.0)/(double)uUploadedTotal);
			}

			// exponential calcualtion of the max multiplicator based on uploaded data (9.2MB = 3.34, 100MB = 10.0)
			float result2 = 0.0F;
			result2 = (float)(uDownloadedTotal/1048576.0);
			result2 += 2.0F;
			result2 = (float)sqrt(result2);

			// linear calcualtion of the max multiplicator based on uploaded data for the first chunk (1MB = 1.01, 9.2MB = 3.34)
			float result3 = 10.0F;
			if (uDownloadedTotal < 9646899){
				result3 = (((float)(uDownloadedTotal - 1048576) / 8598323.0F) * 2.34F) + 1.0F;
			}

			// take the smallest result
			result = min(result, min(result2, result3));

			if (result < 0.1F){
				result = 0.1F;
				break;
			}
			else if (result > 50.0F){
				result = 50.0F;
				break;
			}
		}break;
		// <== Magic Angel Creditsystem - sFrQlXeRt

		// ==> Magic Angel+ Creditsystem - sFrQlXeRt
		case CS_MAGICANGELPLUS:
		{
			if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
				result = 1.0f;
				bBadGuy = true;
				break;
			}

			uint64 uDownloadedTotal = GetDownloadedTotal();
			uint64 uUploadedTotal = GetUploadedTotal();

			if (uDownloadedTotal < 1650000 && uUploadedTotal == 0){
				result = 1.0F;
				break;
			}
			if (!uUploadedTotal)
				result = 50.0F;
			else {
				if (uDownloadedTotal < 1048576) // 1MB
					// We give the client 1 MB upload for the modifier calculation.
					result = (float)(((double)1048576*2.0)/(double)uUploadedTotal);
				else
					result = (float)(((double)uDownloadedTotal*2.0)/(double)uUploadedTotal);
			}

			// exponential calcualtion of the max multiplicator based on uploaded data (9.2MB = 3.34, 100MB = 10.0)
			float result2 = 0.0F;
			result2 = (float)(uDownloadedTotal/1048576.0);
			result2 += 2.0F;
			result2 = (float)sqrt(result2);

			// linear calcualtion of the max multiplicator based on uploaded data for the first chunk (1MB = 1.01, 9.2MB = 3.34)
			float result3 = 10.0F;
			if (uDownloadedTotal < 9646899){
				result3 = (((float)(uDownloadedTotal - 1048576) / 8598323.0F) * 2.34F) + 1.0F;
			}

			// take the smallest result
			result = min(result, min(result2, result3));

			// add some bonus factors for Upload - sFrQlXeRt
			if(uDownloadedTotal > uUploadedTotal){ 
				if(uDownloadedTotal - uUploadedTotal < 7340032) // 7MB
					result += 0.3F;
				else if(uDownloadedTotal - uUploadedTotal < 15728640) // 15MB
					result += 1.0F;
				else if(uDownloadedTotal - uUploadedTotal < 31457280) // 30MB
					result += 2.0F;
				else // uDownloadedTotal - uUploadedTotal >= 31457280 // 30MB
					result += 3.0F;
			}

			if (result < 0.1F){
				result = 0.1F;
				break;
			}
			else if (result > 50.0F){
				result = 50.0F;
				break;
			}
		}break;
		// <== Magic Angel+ Creditsystem - sFrQlXeRt
		// ==> new credit system ~ Sivka Credit - Stulle
		case CS_SIVKA:{
			switch(currentIDstate){
				case IS_IDNEEDED: if(theApp.clientcredits->CryptoAvailable()) {
										result = 0.75f;
										bBadGuy = true;
										break;
							  }
				case IS_IDFAILED: {
									result = 0.5f;
									bBadGuy = true;
									break;
								  }
				case IS_IDBADGUY:
					default: {
						result = 0.0f;
						bBadGuy = true;
						break;
							 }
			}

			if(GetDownloadedTotal() > GetUploadedTotal())
			{
				const uint64 diffTransfer = GetDownloadedTotal() - GetUploadedTotal() + 1048576;

				if( diffTransfer >= 1073741824 ) { // >= 1024MB
					result = 32.0f;
					break;
				}
		
				result = sqrtf((float)diffTransfer/1048576.0f);
				break;
			}
			else
				result = 1.0f;
		}break;
		// <== new credit system ~ Sivka Credit - Stulle

		// ==> new credit system ~ S.W.A.T. Credit - Stulle
		case CS_SWAT:{
			// new creditsystem by [Jmijie]
			if ( currentIDstate != IS_IDENTIFIED  && currentIDstate != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
				result = 1.0f;
				bBadGuy = true;
				break;
			}

			//	if (GetDownloadedTotal() < 1000000)
				if (GetDownloadedTotal() < 1048576) { //pcsl999
					/*return*/ result = 1;
				break;
				}
			
				if (!GetUploadedTotal())
					result = 10;
				else
//					result = (float)(((double)GetDownloadedTotal()*2.0)/(double)GetUploadedTotal());
					result = (float)(((double)GetDownloadedTotal()*2.2)/(double)GetUploadedTotal()); //pcsl999

				float result2 = 0;

				result2 = (float)(GetDownloadedTotal()/1048576.0);
				result2 += 2;
				result2 = (float)sqrt((double)result2);

				if (result > result2)
					result = result2;

				if (result < 1)
					result = 1;
				else if (result > 100) //pcsl999
					result = 100; //pcsl999
		}break;
		// <== new credit system ~ S.W.A.T. Credit - Stulle

		// ==> new credit system ~ Tk4 Creditsystem [BlueSonicBoy] - Stulle
		case CS_TK4:
		{
			CUpDownClient* client = theApp.clientlist->FindClientByIP(dwForIP);

			result = 10.0F;
			//if SUI failed then credit starts at 10 as for everyone else but will not go up
			if((currentIDstate == IS_IDFAILED || currentIDstate == IS_IDBADGUY || currentIDstate == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable()){
				//CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwForIP);//Get 'client' so we can get file info
				float dOwnloadedSessionTotal = (float)client->GetTransferredDown();
				float uPloadedSessionTotal = (float)client->GetTransferredUp();
				float allowance = dOwnloadedSessionTotal/4.0F;
				if(uPloadedSessionTotal > (float)(dOwnloadedSessionTotal + allowance + 1048576.0F)){
					CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
					if(file!=NULL){//Are they requesting a file? NULL can be produced when client details calls getscoreratio() without this line eMule will crash.
						if(file->IsPartFile()){//It's a file we are trying to obtain so we want to give to givers so we may get the file quicker.
							float MbSqd =sqrt((float)(uPloadedSessionTotal-(dOwnloadedSessionTotal + allowance))/1048576.0F);
							if(MbSqd > 9.0F) result = 9.0F / MbSqd;  //above 81mb values 1 - 0 9/(9 - x)
							else result = 10.0F - MbSqd; //for the first 81Mb (10 -(0-9))
						}

					}
				}
				bBadGuy = true;
				break; //partfile 10 - 0.14 complete 10
			}
			//float is 1e38 it should be sufficient given 1 Gig is 1e9 hence 1000G is 1e12....
			float dOwnloadedTotal = (float)GetDownloadedTotal();//(Given to us)
			float uPloadedTotal = (float)GetUploadedTotal(); //(Taken from us)
			/* Base allowance for a client that has given us nothing is 1Mb
			But if someone has give us 100Mb and take 130Mb they should not be penalized as someone who has give 0Mb and taken 30Mb?
			So if you've given 100Mb and taken 130Mb you will only be penalized for 5Mb*/
			float allowance = dOwnloadedTotal/4.0F; //reward uploaders with 1 Mb allowance for every 4Mb uploaded over what they have uploaded.
			if(uPloadedTotal>(float)(dOwnloadedTotal + allowance + 1048576.0F)) //If they have taken above (1Mb + 'allowance')
			{/*They may owe us, is it on a file we want or a completed file we are sharing. If it's a completed file progrssively lowering someone score
			who cannot pay us back could make it very difficult for them to complete the file esp. if it's rare and we hold one of the few complete copies, better for everyone if
			we share completed files based on time waited + any credit thay have for giving us stuff.
			If the files a partfile we are trying to get the modifier will start to get smaller -1 to -90Mb range 9 to 1 beyond that 1 to 0 eg: -400Mb = 0.452839 */
				//CUpDownClient* pClient = theApp.clientlist->FindClientByIP(dwForIP);//Get 'client' so we can get file info
				CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
				if(file!=NULL){//Are they requesting a file? NULL can be produced when client details calls getscoreratio() without this line eMule will crash.
					if(file->IsPartFile()){//It's a file we are trying to obtain so we want to give to givers so we may get the file quicker.
						float MbSqd =sqrt((float)(uPloadedTotal-(dOwnloadedTotal + allowance))/1048576.0F);
						if(MbSqd > 9.0F) result = 9.0F / MbSqd;  //above 81mb values 1 - 0 9/(9 - x)
						else		 result = 10.0F - MbSqd; //for the first 81Mb (10 -(0-9))
					}
				}
			} else //We may owe them :o) give a small proportional boost to an uploader
				if(dOwnloadedTotal>uPloadedTotal){ // result =  log(2.72 + (given - taken in Mb * 4)) + given in bytes / 12Mb (eg +1 for every 12Mb +.5  6Mb etc)
					result+=log(2.72F+(float)(dOwnloadedTotal-uPloadedTotal)/262144.0F)+(float)(dOwnloadedTotal/12582912.0F);
				}
		}break;
		// <== new credit system ~ Tk4 Creditsystem [BlueSonicBoy] - Stulle

		// ==> new credit system ~ Xtreme Creditsystem [Xman] - Stulle
		case CS_XTREME:
		{
			if((currentIDstate == IS_IDFAILED || currentIDstate == IS_IDBADGUY || currentIDstate == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() == true){
				// bad guy - no credits for you
				//return 1.0f;
				result = 0.8f; //Xman 80% for non SUI-clients.. (and also bad guys)
				bBadGuy = true;
				break;
			}

			CUpDownClient* client = theApp.clientlist->FindClientByIP(dwForIP);

			#define PENALTY_UPSIZE 8388608 //8 MB
			// Cache value
			const uint64 downloadTotal = GetDownloadedTotal();

			float m_bonusfaktor = 0.0F;
			// Check if this client has any credit (sent >1.65MB)
			const float difference2=(float)client->GetTransferredUp() - client->GetTransferredDown();	
			if(downloadTotal < 1650000)
			{	
				if ( difference2 > (2*PENALTY_UPSIZE))
					m_bonusfaktor=(-0.2f);
				else if (difference2 > PENALTY_UPSIZE)
					m_bonusfaktor=(-0.1f);
				else
					m_bonusfaktor=0;

				result = (1.0f + m_bonusfaktor);
				break;
			}

			// Cache value
			const uint64 uploadTotal = GetUploadedTotal();

			// Bonus Faktor calculation
			float difference = (float)downloadTotal - uploadTotal;
			if (difference>=0)
			{
				m_bonusfaktor=difference/10485760.0f - (1.5f/(downloadTotal/10485760.0f));  //pro MB difference 0.1 - pro MB download 0.1
				if (m_bonusfaktor<0)
					m_bonusfaktor=0;
			}
			else 
			{
				difference=abs(difference);
				if (difference> (2*PENALTY_UPSIZE) && difference2 > (2*PENALTY_UPSIZE))
					m_bonusfaktor=(-0.2f);
				else if (difference>PENALTY_UPSIZE && difference2 > PENALTY_UPSIZE)
					m_bonusfaktor=(-0.1f);
				else
					m_bonusfaktor=0;
			}
			// Factor 1
			result = (uploadTotal == 0) ?
				10.0f : (float)(2*downloadTotal)/(float)uploadTotal;

			// Factor 2
			//Xman slightly changed to use linear function until half of chunk is transferred
			float trunk;
			if(downloadTotal < 4718592)  //half of a chunk and a good point to keep the function consistent
				trunk = (float)(1.0 + (double)downloadTotal/(1048576.0*3.0));
			else
				trunk = (float)sqrt(2.0 + (double)downloadTotal/1048576.0);
			//Xman end


			if(result>10.0f)
			{
				result=10.0f;
				m_bonusfaktor=0;
			}
			else
				result += m_bonusfaktor;
			if(result>10.0f)
			{
				m_bonusfaktor -= (float)(result-10.0f);
				result=10.0f;
			}

			if(result > trunk)
			{
				result = trunk;
				m_bonusfaktor=0;
			}

			// Trunk final result 1..10
			if(result < 1.0f)
			{
				result = (1.0f + m_bonusfaktor );
				break;
			}
			if (result > 10.0f)
			{
				result = 10.0f;
				break;
			}
		}break;
		// <== new credit system ~ Xtreme Creditsystem [Xman] - Stulle

		// ==> new credit system ~ ZZUL [ZZ] - Stulle
		case CS_ZZUL:
		{
			if(currentIDstate != IS_IDENTIFIED  && currentIDstate != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable())
			{
				result = 1.0F;
				bBadGuy = true;
				break;
			}

			if (GetDownloadedTotal() < 1)
			{
				result = 1.0F;
				break;
			}
			result = 0.0F;
			if (!GetUploadedTotal())
				result = 10.0F;
			else
				result = (float)(((double)GetDownloadedTotal()*2.0)/(double)GetUploadedTotal());
			float result2 = 0.0F;
			result2 = (float)(GetDownloadedTotal()/1048576.0);
			result2 += 1.0F;
			result2 = (float)sqrt(result2);

			if (result > result2)
				result = result2;

			if (result < 1.0F)
			{
				result = 1.0F;
				break;
			}
			else if (result > 10.0F)
			{
				result = 10.0F;
				break;
			}
		}break;
		// <== new credit system ~ ZZUL [ZZ] - Stulle


		case CS_OFFICIAL:
		default:{
			if ( currentIDstate != IS_IDENTIFIED  && currentIDstate != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
				result = 1.0f;
				bBadGuy = true;
				break;
			}

			if (GetDownloadedTotal() < 1048576){
				result = 1.0F;
				break;
			}
			if (!GetUploadedTotal())
				result = 10.0F;
			else
				result = (float)(((double)GetDownloadedTotal()*2.0)/(double)GetUploadedTotal());
	
			// exponential calcualtion of the max multiplicator based on uploaded data (9.2MB = 3.34, 100MB = 10.0)
			float result2 = 0.0F;
			result2 = (float)(GetDownloadedTotal()/1048576.0);
			result2 += 2.0F;
			result2 = (float)sqrt(result2);

			// linear calcualtion of the max multiplicator based on uploaded data for the first chunk (1MB = 1.01, 9.2MB = 3.34)
			float result3 = 10.0F;
			if (GetDownloadedTotal() < 9646899){
				result3 = (((float)(GetDownloadedTotal() - 1048576) / 8598323.0F) * 2.34F) + 1.0F;
			}

			// take the smallest result
			result = min(result, min(result2, result3));

			if (result < 1.0F){
				result = 1.0F;
				break;
			}else if (result > 10.0F){
				result = 10.0F;
				break;
			}
		}break;
	}

	// ==> moved into switch for new credit systems - Stulle
	if(bBadGuy)
		m_bCheckScoreRatio = true;
	// <== moved into switch for new credit systems - Stulle

	// ==> Anti Upload Protection - sFrQlXeRt
	// FIXED: Anti Upload Protection when the other client have not yet uploaded to us [evcz]
	if (thePrefs.IsAntiUploadProtection() && thePrefs.TransferFullChunks() && (GetDownloadedTotal() < thePrefs.GetUploadProtectionLimit()*1024))
		//return 1.0F;
		//fixed to handle proper default values depending on the CS... not everytime 1.0... ;)
		//code from StulleMule
		switch (thePrefs.GetCreditSystem())	{
			case CS_LOVELACE:{
				if((float)result > 0.985f)
					return m_fLastScoreRatio = 0.985f;
			}
			case CS_PAWCIO:{
				if((float)result > (float)3.0f)
					return m_fLastScoreRatio = 3.0f;
			}
			case CS_TK4:{
				if((float)result > (float)10.0f)
					return m_fLastScoreRatio = 3.0f;
			}
			case CS_RATIO:
			case CS_EASTSHARE:
			case CS_SIVKA:
			case CS_SWAT:
			case CS_XTREME:
			case CS_ZZUL:
			//==sFrQlXeRt=> Magic Angel Creditsystems
			case CS_MAGICANGEL:
			case CS_MAGICANGELPLUS:
			//<=sFrQlXeRt== Magic Angel Creditsystems
			case CS_OFFICIAL:
			default:{
				if((float)result > 1.0f)
					return m_fLastScoreRatio = 1.0f;
			}
	}
	// <== Anti Upload Protection - sFrQlXeRt

	return m_fLastScoreRatio = (float)result;
	//EastShare END - Added by linekin, CreditSystem 

	//EastShare END - Added by linekin, CreditSystem 
}

//MORPH START - Added by Stulle, fix score display
bool CClientCredits::GetHasScore(uint32 dwForIP)
{
	float modif = GetScoreRatio(dwForIP);
	float m_fDefault;

	// ==> new credit system - Stulle
	/*
	if (thePrefs.GetCreditSystem() == CS_LOVELACE)
		m_fDefault = 0.985f; // this might be a bit more than the result... who care's!?!?!
	else if (thePrefs.GetCreditSystem() == CS_PAWCIO)
		m_fDefault = 3.0f;
	else
		m_fDefault = 1.0f;
	*/
	switch (thePrefs.GetCreditSystem())	{
		case CS_LOVELACE:
		{
			m_fDefault = 0.985f; // this might be a bit more than the result... who care's!?!?!
		}break;
		case CS_PAWCIO:
		{
			m_fDefault = 3.0f;
		}break;
		case CS_TK4:
		{
			m_fDefault = 10.0f;
		}break;
		case CS_OFFICIAL:
		case CS_RATIO:
		case CS_EASTSHARE:
		case CS_SIVKA:
		case CS_SWAT:
		case CS_XTREME:
		case CS_ZZUL:
		default:
		{
			m_fDefault = 1.0f;
		}break;
	}
	// <== new credit system - Stulle

	return (modif > m_fDefault);
}
//MORPH END - Added by Stulle, fix score display

//MORPH START - Added by IceCream, VQB: ownCredits
float CClientCredits::GetMyScoreRatio(uint32 dwForIP) const
{
	// check the client ident status
	//MORPH START - Changed by SIRoB, Code Optimization 
	/*
	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED  || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
	*/
	if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && GetCurrentIdentState(dwForIP) != IS_IDBADGUY && theApp.clientcredits->CryptoAvailable() ){
	//MORPH END   - Changed by SIRoB, Code Optimization 
		// bad guy - no credits for... me?
		return 1.0F;
	}

	CUpDownClient* client = theApp.clientlist->FindClientByIP(dwForIP);
	uint64 uUploadedTotalMin = 1000000;
	bool bNewCredits = false;
	if(client->GetClientSoft() == SO_EMULE && client->GetVersion() >= MAKE_CLIENT_VERSION(0, 48, 0))
	{
		uUploadedTotalMin = 1048576;
		bNewCredits = true;
	}

	if (GetUploadedTotal() < uUploadedTotalMin)
		return 1.0F;
	float result = 0.0F;
	if (!GetDownloadedTotal())
		result = 10.0F;
	else
		result = (float)(((double)GetUploadedTotal()*2.0)/(double)GetDownloadedTotal());
	float result2 = 0.0F;
	result2 = (float)(GetUploadedTotal()/1048576.0);
	result2 += 2.0F;
	result2 = (float)sqrt(result2);

	if(!bNewCredits)
	{
		if (result > result2)
			result = result2;
	}
	else
	{
		// linear calcualtion of the max multiplicator based on uploaded data for the first chunk (1MB = 1.01, 9.2MB = 3.34)
		float result3 = 10.0F;
		if (GetUploadedTotal() < 9646899){
			result3 = (((float)(GetUploadedTotal() - 1048576) / 8598323.0F) * 2.34F) + 1.0F;
		}

		// take the smallest result
		result = min(result, min(result2, result3));
	}

	if (result < 1.0F)
		return 1.0F;
	else if (result > 10.0F)
		return 10.0F;
	return result;
}
//MORPH END   - Added by IceCream, VQB: ownCredits

//Morph Start - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
// Moonlight: SUQWT - Conditions to determine an active record.
// Returns true if the client has been seen recently
bool CClientCredits::IsActive(time_t dwExpired) { //vs2005 (NOTE: some chashdumps on exit with sqyt.met corruption  point here??? 
	return (GetUploadedTotal() || GetDownloadedTotal() || m_pCredits->nSecuredWaitTime || m_pCredits->nUnSecuredWaitTime) &&
			(m_pCredits->nLastSeen >= dwExpired);
}
//Morph End - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)

CClientCreditsList::CClientCreditsList()
{
	m_nLastSaved = ::GetTickCount();
	LoadList();
	
	InitalizeCrypting();
}

CClientCreditsList::~CClientCreditsList()
{
	SaveList();
	CClientCredits* cur_credit;
	CCKey tmpkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	while (pos){
		m_mapClients.GetNextAssoc(pos, tmpkey, cur_credit);
		delete cur_credit;
	}
		delete m_pSignkey;
}

// Moonlight: SUQWT: Change the file import 0.30c format.//Morph - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
void CClientCreditsList::LoadList()
{
	CString strFileName = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + CLIENTS_MET_FILENAME;
	const int iOpenFlags = CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite;
	CSafeBufferedFile file;
	CFileException fexp;

	m_bSaveUploadQueueWaitTime = thePrefs.SaveUploadQueueWaitTime();//Morph - added by AndCycle, Save Upload Queue Wait Time (SUQWT)
	//Morph Start - added by AndCycle, choose .met to load

	CSafeBufferedFile	loadFile;

	const int	totalLoadFile = 9;

	CString		loadFileName[totalLoadFile];
	CFileStatus	loadFileStatus[totalLoadFile];
	bool		successLoadFile[totalLoadFile];

	int	countFile = 0;

	//SUQWTv2.met must have bigger number than original clients.met to have higher prio
	loadFileName[countFile++].Format(_T("%s") CLIENTS_MET_FILENAME, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	loadFileName[countFile++].Format(_T("%s") CLIENTS_MET_FILENAME _T(".bak"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	loadFileName[countFile++].Format(_T("%s") CLIENTS_MET_FILENAME _T(".MSUQWT"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));//Pawcio
	loadFileName[countFile++].Format(_T("%s") CLIENTS_MET_FILENAME _T(".SUQWTv2.met"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	loadFileName[countFile++].Format(_T("%s") CLIENTS_MET_FILENAME _T(".SUQWTv2.met.bak"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	loadFileName[countFile++].Format(_T("%s") CLIENTS_MET_FILENAME, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("Backup\\"));
	loadFileName[countFile++].Format(_T("%s") CLIENTS_MET_FILENAME _T(".SUQWTv2.met"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("Backup\\"));
	loadFileName[countFile++].Format(_T("%s") CLIENTS_MET_FILENAME, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("Backup2\\"));
	loadFileName[countFile++].Format(_T("%s") CLIENTS_MET_FILENAME _T(".SUQWTv2.met"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)+_T("Backup2\\"));
	//totalLoadFile = 9;
	uint8 prioOrderfile[totalLoadFile];

	int	index = 0;
	for(int curFile = 0; curFile < totalLoadFile && (curFile < 255); curFile++){
		//check clients.met status
		successLoadFile[curFile] = loadFile.Open(loadFileName[curFile], iOpenFlags, &fexp)!=0;
		if (successLoadFile[curFile]){
			loadFile.GetStatus(loadFileStatus[curFile]);
			prioOrderfile[index++]=(uint8)curFile;
			loadFile.Close();
		}
	}
	uint8 tmpprioOrderfile;
	uint8 maxavailablefile =(uint8) index;
	for (;index>0;index--){
		for (uint8 i=1; i<index;i++)
		{
			if(loadFileStatus[prioOrderfile[i-1]].m_mtime > loadFileStatus[prioOrderfile[i]].m_mtime)
				continue;
			if(m_bSaveUploadQueueWaitTime && loadFileStatus[prioOrderfile[i-1]].m_mtime == loadFileStatus[prioOrderfile[i]].m_mtime && _tcsstr(loadFileStatus[prioOrderfile[i]].m_szFullName,_T("SUQWT")) == 0)
				continue;
			tmpprioOrderfile = prioOrderfile[i-1];
			prioOrderfile[i-1] = prioOrderfile[i];
			prioOrderfile[i] = tmpprioOrderfile;
		}
	}

	for (uint8 i=0;i<maxavailablefile;i++)
	{
		strFileName = loadFileName[prioOrderfile[i]];
		AddLogLine(false, GetResString(IDS_SUQWT_LOAD), strFileName);
	//Morph End - added by AndCycle, choose .met to load
	//MORPH END  - Changed by SiRoB, Allternative choose .met to load

		if (!file.Open(strFileName, iOpenFlags, &fexp)){
			if (fexp.m_cause != CFileException::fileNotFound){
				CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
					strError += _T(" - ");
					strError += szError;
				}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
			}
			//MORPH START - Changed by SiRoB, Allternative choose .met to load
			/*
            return;
			*/
			continue;
			//MORPH END  - Changed by SiRoB, Allternative choose .met to load
		}
		setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
		
		try{
			uint8 version = file.ReadUInt8();
			//Morph Start - modified by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
			/*
			if (version != CREDITFILE_VERSION && version != CREDITFILE_VERSION_29){
				LogWarning(GetResString(IDS_ERR_CREDITFILEOLD));
				file.Close();
				return;
			}
			*/
			// Moonlight: SUQWT - Import CreditStruct from 0.30c and SUQWTv1
			if (version != CREDITFILE_VERSION_30_SUQWTv1 && version != CREDITFILE_VERSION_30_SUQWTv2 &&
				version != CREDITFILE_VERSION_30 && version != CREDITFILE_VERSION_29){
				LogWarning(GetResString(IDS_ERR_CREDITFILEOLD));
				file.Close();
				//MORPH START - Changed by SiRoB, Allternative choose .met to load
				/*
				return;
				*/
				continue;
				//MORPH END  - Changed by SiRoB, Allternative choose .met to load
			}
			//Morph End - modified by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)

			// everything is ok, lets see if the backup exist...
			CString strBakFileName;
			//Morph start - modify by AndCycle, backup loaded file
			/*
			strBakFileName.Format(_T("%s") CLIENTS_MET_FILENAME _T(".bak"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
			*/
			strBakFileName.Format(_T("%s") _T(".bak"), strFileName);
			//Morph end - modify by AndCycle, backup loaded file

			DWORD dwBakFileSize = 0;
			BOOL bCreateBackup = TRUE;

			HANDLE hBakFile = ::CreateFile(strBakFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
											OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hBakFile != INVALID_HANDLE_VALUE)
			{
				// Ok, the backup exist, get the size
				dwBakFileSize = ::GetFileSize(hBakFile, NULL); //debug
				if (dwBakFileSize > (DWORD)file.GetLength())
				{
					// the size of the backup was larger then the org. file, something is wrong here, don't overwrite old backup..
					bCreateBackup = FALSE;
				}
				//else: backup is smaller or the same size as org. file, proceed with copying of file
				::CloseHandle(hBakFile);
			}
			//else: the backup doesn't exist, create it

			if (bCreateBackup)
			{
				file.Close(); // close the file before copying

				if (!::CopyFile(strFileName, strBakFileName, FALSE))
					LogError(GetResString(IDS_ERR_MAKEBAKCREDITFILE));

				// reopen file
				CFileException fexp;
				if (!file.Open(strFileName, iOpenFlags, &fexp)){
					CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
					TCHAR szError[MAX_CFEXP_ERRORMSG];
					if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
						strError += _T(" - ");
						strError += szError;
					}
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					continue; //MORPH - Continue loading files
				}
				setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
				file.Seek(1, CFile::begin); //set filepointer behind file version byte
			}

			if (m_mapClients.GetCount() > 0) {
				CClientCredits* cur_credit;
				CCKey tmpkey(0);
				POSITION pos = m_mapClients.GetStartPosition();
				while (pos){
					m_mapClients.GetNextAssoc(pos, tmpkey, cur_credit);
					delete cur_credit;
				}
				m_mapClients.RemoveAll();
			}
			UINT count = file.ReadUInt32();
			// ==> Extended credit-table-arragement [Xman] - sFrQlXeRt
			//Morph Start - added by AndCycle, minor tweak - prime
			/*
			m_mapClients.InitHashTable(count+5000); // TODO: should be prime number... and 20% larger
			*/
			//m_mapClients.InitHashTable((int)(count*1.5) > 5003?getPrime((int)(count*1.5)):5003);
			//Morph End - added by AndCycle, minor tweak - prime
			UINT calc=UINT(count*1.2f);
			calc = calc + calc%2 + 1;
			m_mapClients.InitHashTable(calc + 20000); //optimized for 20 000 new contacts
			// <== Extended credit-table-arragement [Xman] - sFrQlXeRt

			const time_t dwExpired = time(NULL) - 12960000; // today - 150 day vs2005
			uint32 cDeleted = 0;
			
			//MORPH START - Changed by SiRoB, Optimization
			//Morph Start - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
			if (version == CREDITFILE_VERSION) {
				for (UINT i = 0; i < count; i++){
					CreditStruct* newcstruct = new CreditStruct;
					file.Read(newcstruct, sizeof(CreditStruct_30c_SUQWTv2));
					if (newcstruct->nLastSeen < dwExpired){
						++cDeleted;
						delete newcstruct;
						continue;
					}
					CClientCredits* newcredits = new CClientCredits(newcstruct);
					m_mapClients.SetAt(CCKey(newcredits->GetKey()), newcredits);
				}		
			} else if (version == CREDITFILE_VERSION_30) {
				for (UINT i = 0; i < count; i++){
					CreditStruct* newcstruct = new CreditStruct;
					newcstruct->nSecuredWaitTime = 0;
					newcstruct->nUnSecuredWaitTime = 0;
					file.Read(((uint8*)newcstruct) + 8, sizeof(CreditStruct_30c));
					if (newcstruct->nLastSeen < dwExpired){
						++cDeleted;
						delete newcstruct;
						continue;
					}
					CClientCredits* newcredits = new CClientCredits(newcstruct);
					m_mapClients.SetAt(CCKey(newcredits->GetKey()), newcredits);
				}		
			} else if (version == CREDITFILE_VERSION_30_SUQWTv1) {
				for (UINT i = 0; i < count; i++){
					CreditStruct* newcstruct = new CreditStruct;
					file.Read(((uint8*)newcstruct) + 8, sizeof(CreditStruct_30c_SUQWTv1) - 8);
					file.Read(((uint8*)newcstruct), 8);
					if (newcstruct->nLastSeen < dwExpired){
						++cDeleted;
						delete newcstruct;
						continue;
					}
					CClientCredits* newcredits = new CClientCredits(newcstruct);
					m_mapClients.SetAt(CCKey(newcredits->GetKey()), newcredits);
				}		
			} else {
				for (UINT i = 0; i < count; i++){
					CreditStruct* newcstruct = new CreditStruct;
					memset(newcstruct, 0, sizeof(CreditStruct));
					file.Read(((uint8*)newcstruct) + 8, sizeof(CreditStruct_29a));
					if (newcstruct->nLastSeen < dwExpired){
						cDeleted++;
						delete newcstruct;
						continue;
					}

					CClientCredits* newcredits = new CClientCredits(newcstruct);
					m_mapClients.SetAt(CCKey(newcredits->GetKey()), newcredits);
				}
			}
			//Morph End   - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
			//MORPH START - Changed by SiRoB, Optimization
			file.Close();

			if (cDeleted>0)
				AddLogLine(false, GetResString(IDS_CREDITFILELOADED) + GetResString(IDS_CREDITSEXPIRED), count-cDeleted,cDeleted);
			else
				AddLogLine(false, GetResString(IDS_CREDITFILELOADED), count);
			
			//We got a valide Credit file so exit now
			break; //Added by SiRoB
		}
		catch(CFileException* error){
			if (error->m_cause == CFileException::endOfFile)
			LogError(LOG_STATUSBAR, GetResString(IDS_CREDITFILECORRUPT));
			else{
				TCHAR buffer[MAX_CFEXP_ERRORMSG];
				error->GetErrorMessage(buffer, ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREDITFILEREAD), buffer);
			}
			error->Delete();
			file.Close();
		}
		//MORPH START - Added by SiRoB, Catch oversized public key in credit.met file
		catch(CString error)
		{
			if (!error.IsEmpty())
				LogWarning(_T("%s - while loading %s"), error, strFileName);
			file.Close();
		}
		//MORPH END   - Added by SiRoB, Catch oversized public key in credit.met file
	}//MORPH - Added by SiRoB, Alternative choose .met to load
}

// Moonlight: SUQWT - Save the wait times before saving the list.//Morph - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
void CClientCreditsList::SaveList()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, _T("Saving clients credit list file \"%s\""), CLIENTS_MET_FILENAME);
	m_nLastSaved = ::GetTickCount();

	CString name = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + CLIENTS_MET_FILENAME;
	CFile file;// no buffering needed here since we swap out the entire array
	CFileException fexp;
	if (!file.Open(name, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(GetResString(IDS_ERR_FAILED_CREDITSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return;
	}

	uint32 count = m_mapClients.GetCount();
	BYTE* pBuffer = NULL;
	pBuffer = new BYTE[count*sizeof(CreditStruct_30c)]; //Morph - modified by AndCycle, original 30c file format
	//Morph Start - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	BYTE* pBufferSUQWT=NULL;
	if (m_bSaveUploadQueueWaitTime)
		pBufferSUQWT = new BYTE[count*sizeof(CreditStruct)];
	const time_t dwExpired = time(NULL) - 12960000; // today - 150 day vs2005
	//Morph End   - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	CClientCredits* cur_credit;
	CCKey tempkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	count = 0;
	while (pos)
	{
		m_mapClients.GetNextAssoc(pos, tempkey, cur_credit);
		//Morph Start - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
		if(m_bSaveUploadQueueWaitTime){
			if (cur_credit->IsActive(dwExpired))	// Moonlight: SUQWT - Also save records if there is wait time.
			{
				cur_credit->SaveUploadQueueWaitTime();	// Moonlight: SUQWT
				memcpy(pBufferSUQWT+(count*sizeof(CreditStruct)), cur_credit->GetDataStruct(), sizeof(CreditStruct));
				memcpy(pBuffer+(count*sizeof(CreditStruct_30c)), (uint8 *)cur_credit->GetDataStruct() + 8, sizeof(CreditStruct_30c));	// Moonlight: SUQWT - Save 0.30c CreditStruct
				count++; 
			}
		}else 
		//Morph End   - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
		if (cur_credit->GetUploadedTotal() || cur_credit->GetDownloadedTotal())
		{
			/*// Moonlight: SUQWT - Save 0.30c CreditStruct
			memcpy(pBuffer+(count*sizeof(CreditStruct)), cur_credit->GetDataStruct(), sizeof(CreditStruct));
			*/
			memcpy(pBuffer+(count*sizeof(CreditStruct_30c)), (uint8 *)cur_credit->GetDataStruct() + 8, sizeof(CreditStruct_30c));
			count++; 
		}
	}

	try{
		uint8 version = CREDITFILE_VERSION_30; //Morph - modified by AndCycle, original 30c file format
		file.Write(&version, 1);
		file.Write(&count, 4);
		file.Write(pBuffer, count*sizeof(CreditStruct_30c)); //Morph - modified by AndCycle, original 30c file format
		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning()))
			file.Flush();
		file.Close();

		//Morph Start - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
		if (m_bSaveUploadQueueWaitTime)
		{
			CString nameSUQWT = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + CString(CLIENTS_MET_FILENAME) + _T(".SUQWTv2.met"); 
			if (!file.Open(nameSUQWT, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
				CString strError(GetResString(IDS_ERR_FAILED_CREDITSAVE));
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
					strError += _T(" - ");
					strError += szError;
				}
				LogError(LOG_STATUSBAR, _T("%s"), strError);
				return;
			}
			uint8 version = CREDITFILE_VERSION;
			file.Write(&version, 1);
			file.Write(&count, 4);
			file.Write(pBufferSUQWT, count*sizeof(CreditStruct)); //save SUQWT buffer
			if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning()))
				file.Flush();
			file.Close();
		}
		//Morph End - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	}
	catch(CFileException* error){
		CString strError(GetResString(IDS_ERR_FAILED_CREDITSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
	}
	delete[] pBuffer;

	//Morph Start - added by SiRoB, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	if(m_bSaveUploadQueueWaitTime)
		delete[] pBufferSUQWT;
	//Morph End   - added by SiRoB, Moonlight's Save Upload Queue Wait Time (MSUQWT)
}

CClientCredits* CClientCreditsList::GetCredit(const uchar* key)
{
	CClientCredits* result;
	CCKey tkey(key);
	if (!m_mapClients.Lookup(tkey, result)){
		result = new CClientCredits(key);
		m_mapClients.SetAt(CCKey(result->GetKey()), result);
	}
	result->SetLastSeen();
	result->UnMarkToDelete(); // => Extended credit-table-arragement [Xman] - sFrQlXeRt
	return result;
}

void CClientCreditsList::Process()
{
#define HOURS_KEEP_IN_MEMORY 6 // => Extended credit-table-arragement [Xman] - sFrQlXeRt
	if (::GetTickCount() - m_nLastSaved > MIN2MS(13))
		// ==> Extended credit-table-arragement [Xman] - sFrQlXeRt
	{
		CClientCredits* cur_credit;
		CCKey tmpkey(0);
		POSITION pos = m_mapClients.GetStartPosition();
		while (pos){
			m_mapClients.GetNextAssoc(pos, tmpkey, cur_credit);

			if(cur_credit->GetMarkToDelete() && (time(NULL) - cur_credit->GetLastSeen() > (3600 * HOURS_KEEP_IN_MEMORY))) //not seen for > 3 hours
			{
				//two security-checks, it can happen that there is a second user using this hash
				if(cur_credit->GetUploadedTotal()==0 && cur_credit->GetDownloadedTotal()==0
					&& theApp.clientlist->FindClientByUserHash(cur_credit->GetKey())==NULL
					//==sFrQlXeRt=> Fix for SUQWT and Extended credit-table-arrangement
					&& cur_credit->GetSecureWaitStartTime(cur_credit->m_dwWaitTimeIP, true)==0)
				{
					//this key isn't longer used
					m_mapClients.RemoveKey(CCKey(cur_credit->GetKey()));
					delete cur_credit;
				}
				else
					cur_credit->UnMarkToDelete();
			}
		}
	// <== Extended credit-table-arragement [Xman] - sFrQlXeRt
		SaveList();
	} // => Extended credit-table-arragement [Xman] - sFrQlXeRt
}

void CClientCredits::InitalizeIdent()
{
	if (m_pCredits->nKeySize == 0 ){
		memset(m_abyPublicKey,0,80); // for debugging
		m_nPublicKeyLen = 0;
		IdentState = IS_NOTAVAILABLE;
	}
	else{
		m_nPublicKeyLen = m_pCredits->nKeySize;
		//MORPH START - Added by SiRoB, Catch oversized public key in credit.met file
		if (m_nPublicKeyLen > MAXPUBKEYSIZE)
			throw CString(_T("Public Key of one client is larger than MAXPUBKEYSIZE"));
		//MORPH END   - Added by SiRoB, Catch oversized public key in credit.met file
		memcpy(m_abyPublicKey, m_pCredits->abySecureIdent, m_nPublicKeyLen);
		IdentState = IS_IDNEEDED;
	}
	m_dwCryptRndChallengeFor = 0;
	m_dwCryptRndChallengeFrom = 0;
	m_dwIdentIP = 0;
}

void CClientCredits::Verified(uint32 dwForIP)
{
	m_dwIdentIP = dwForIP;
	// client was verified, copy the keyto store him if not done already
	if (m_pCredits->nKeySize == 0){
		m_pCredits->nKeySize = m_nPublicKeyLen; 
		memcpy(m_pCredits->abySecureIdent, m_abyPublicKey, m_nPublicKeyLen);
		if (GetDownloadedTotal() > 0){
			// for security reason, we have to delete all prior credits here
			m_pCredits->nDownloadedHi = 0;
			m_pCredits->nDownloadedLo = 1;
			m_pCredits->nUploadedHi = 0;
			m_pCredits->nUploadedLo = 1; // in order to safe this client, set 1 byte
			if (thePrefs.GetVerbose())
				DEBUG_ONLY(AddDebugLogLine(false, _T("Credits deleted due to new SecureIdent")));
		}
	}
	IdentState = IS_IDENTIFIED;
}

bool CClientCredits::SetSecureIdent(const uchar* pachIdent, uint8 nIdentLen)  // verified Public key cannot change, use only if there is not public key yet
{
	if (MAXPUBKEYSIZE < nIdentLen || m_pCredits->nKeySize != 0 )
		return false;
	memcpy(m_abyPublicKey,pachIdent, nIdentLen);
	m_nPublicKeyLen = nIdentLen;
	IdentState = IS_IDNEEDED;
	return true;
}

EIdentState	CClientCredits::GetCurrentIdentState(uint32 dwForIP) const
{
	if (IdentState != IS_IDENTIFIED)
		return IdentState;
	else{
		if (dwForIP == m_dwIdentIP)
			return IS_IDENTIFIED;
		else
			return IS_IDBADGUY; 
			// mod note: clients which just reconnected after an IP change and have to ident yet will also have this state for 1-2 seconds
			//		 so don't try to spam such clients with "bad guy" messages (besides: spam messages are always bad)
	}
}

using namespace CryptoPP;

void CClientCreditsList::InitalizeCrypting()
{
	m_nMyPublicKeyLen = 0;
	memset(m_abyMyPublicKey,0,80); // not really needed; better for debugging tho
	m_pSignkey = NULL;
	if (!thePrefs.IsSecureIdentEnabled())
		return;
	// check if keyfile is there
	bool bCreateNewKey = false;
	HANDLE hKeyFile = ::CreateFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("cryptkey.dat"), GENERIC_READ, FILE_SHARE_READ, NULL,
										OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hKeyFile != INVALID_HANDLE_VALUE)
	{
		if (::GetFileSize(hKeyFile, NULL) == 0)
			bCreateNewKey = true;
		::CloseHandle(hKeyFile);
	}
	else
		bCreateNewKey = true;
	if (bCreateNewKey)
		CreateKeyPair();
	
	// load key
	try{
		// load private key
		FileSource filesource(CStringA(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("cryptkey.dat")), true,new Base64Decoder);
		m_pSignkey = new RSASSA_PKCS1v15_SHA_Signer(filesource);
		// calculate and store public key
		RSASSA_PKCS1v15_SHA_Verifier pubkey(*m_pSignkey);
		ArraySink asink(m_abyMyPublicKey, 80);
		pubkey.DEREncode(asink);
		m_nMyPublicKeyLen = (uint8)asink.TotalPutLength();
		asink.MessageEnd();
	}
	catch(...)
	{
		delete m_pSignkey;
		m_pSignkey = NULL;
		LogError(LOG_STATUSBAR, GetResString(IDS_CRYPT_INITFAILED));
		ASSERT(0);
	}
	ASSERT( Debug_CheckCrypting() );
}

bool CClientCreditsList::CreateKeyPair()
{
	try{
		AutoSeededRandomPool rng;
		InvertibleRSAFunction privkey;
		privkey.Initialize(rng,RSAKEYSIZE);
 
		Base64Encoder privkeysink(new FileSink(CStringA(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("cryptkey.dat"))));
		privkey.DEREncode(privkeysink);
		privkeysink.MessageEnd();

		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("Created new RSA keypair"));
	}
	catch(...)
	{
		// morphend: more logging if config dir is redonly (vista!)
		theApp.QueueLogLineEx(LOG_ERROR, _T("rsa create failed, fialed to create 'cryptkey.dat' in directory %s. Make sure 'Users' has write permmissions." ),thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)); 
		// morphend: more logging if config dir is redonly (vista!)
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Failed to create new RSA keypair"));
		ASSERT ( false );
		return false;
	}
	return true;
}

uint8 CClientCreditsList::CreateSignature(CClientCredits* pTarget, uchar* pachOutput, uint8 nMaxSize, 
										  uint32 ChallengeIP, uint8 byChaIPKind, 
										  CryptoPP::RSASSA_PKCS1v15_SHA_Signer* sigkey)
{
	// sigkey param is used for debug only
	if (sigkey == NULL)
		sigkey = m_pSignkey;

	// create a signature of the public key from pTarget
	ASSERT( pTarget );
	ASSERT( pachOutput );
	uint8 nResult;
	if ( !CryptoAvailable() )
		return 0;
	try{
		
		SecByteBlock sbbSignature(sigkey->SignatureLength());
		AutoSeededRandomPool rng;
		byte abyBuffer[MAXPUBKEYSIZE+9];
		uint32 keylen = pTarget->GetSecIDKeyLen();
		memcpy(abyBuffer,pTarget->GetSecureIdent(),keylen);
		// 4 additional bytes random data send from this client
		uint32 challenge = pTarget->m_dwCryptRndChallengeFrom;
		ASSERT ( challenge != 0 );
		PokeUInt32(abyBuffer+keylen, challenge);
		uint16 ChIpLen = 0;
		if ( byChaIPKind != 0){
			ChIpLen = 5;
			PokeUInt32(abyBuffer+keylen+4, ChallengeIP);
			PokeUInt8(abyBuffer+keylen+4+4, byChaIPKind);
		}
		sigkey->SignMessage(rng, abyBuffer ,keylen+4+ChIpLen , sbbSignature.begin());
		ArraySink asink(pachOutput, nMaxSize);
		asink.Put(sbbSignature.begin(), sbbSignature.size());
		nResult = (uint8)asink.TotalPutLength();			
	}
	catch(...)
	{
		ASSERT ( false );
		nResult = 0;
	}
	return nResult;
}

bool CClientCreditsList::VerifyIdent(CClientCredits* pTarget, const uchar* pachSignature, uint8 nInputSize, 
									 uint32 dwForIP, uint8 byChaIPKind)
{
	ASSERT( pTarget );
	ASSERT( pachSignature );
	if ( !CryptoAvailable() ){
		pTarget->IdentState = IS_NOTAVAILABLE;
		return false;
	}
	bool bResult;
	try{
		StringSource ss_Pubkey((byte*)pTarget->GetSecureIdent(),pTarget->GetSecIDKeyLen(),true,0);
		RSASSA_PKCS1v15_SHA_Verifier pubkey(ss_Pubkey);
		// 4 additional bytes random data send from this client +5 bytes v2
		byte abyBuffer[MAXPUBKEYSIZE+9];
		memcpy(abyBuffer,m_abyMyPublicKey,m_nMyPublicKeyLen);
		uint32 challenge = pTarget->m_dwCryptRndChallengeFor;
		ASSERT ( challenge != 0 );
		PokeUInt32(abyBuffer+m_nMyPublicKeyLen, challenge);
		
		// v2 security improvments (not supported by 29b, not used as default by 29c)
		uint8 nChIpSize = 0;
		if (byChaIPKind != 0){
			nChIpSize = 5;
			uint32 ChallengeIP = 0;
			switch (byChaIPKind){
				case CRYPT_CIP_LOCALCLIENT:
					ChallengeIP = dwForIP;
					break;
				case CRYPT_CIP_REMOTECLIENT:
					if (theApp.serverconnect->GetClientID() == 0 || theApp.serverconnect->IsLowID()){
						if (thePrefs.GetLogSecureIdent())
							AddDebugLogLine(false, _T("Warning: Maybe SecureHash Ident fails because LocalIP is unknown"));
						ChallengeIP = theApp.serverconnect->GetLocalIP();
					}
					else
						ChallengeIP = theApp.serverconnect->GetClientID();
					break;
				case CRYPT_CIP_NONECLIENT: // maybe not supported in future versions
					ChallengeIP = 0;
					break;
			}
			PokeUInt32(abyBuffer+m_nMyPublicKeyLen+4, ChallengeIP);
			PokeUInt8(abyBuffer+m_nMyPublicKeyLen+4+4, byChaIPKind);
		}
		//v2 end

		bResult = pubkey.VerifyMessage(abyBuffer, m_nMyPublicKeyLen+4+nChIpSize, pachSignature, nInputSize);
	}
	catch(...)
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Error: Unknown exception in %hs"), __FUNCTION__);
		//ASSERT(0);
		bResult = false;
	}
	if (!bResult){
		if (pTarget->IdentState == IS_IDNEEDED)
			pTarget->IdentState = IS_IDFAILED;
	}
	else{
		pTarget->Verified(dwForIP);
	}
	return bResult;
}

bool CClientCreditsList::CryptoAvailable()
{
	return (m_nMyPublicKeyLen > 0 && m_pSignkey != 0 && thePrefs.IsSecureIdentEnabled() );
}


#ifdef _DEBUG
bool CClientCreditsList::Debug_CheckCrypting()
{
	// create random key
	AutoSeededRandomPool rng;

	RSASSA_PKCS1v15_SHA_Signer priv(rng, 384);
	RSASSA_PKCS1v15_SHA_Verifier pub(priv);

	byte abyPublicKey[80];
	ArraySink asink(abyPublicKey, 80);
	pub.DEREncode(asink);
	uint8 PublicKeyLen = (uint8)asink.TotalPutLength();
	asink.MessageEnd();
	uint32 challenge = rand();
	// create fake client which pretends to be this emule
	CreditStruct* newcstruct = new CreditStruct;
	memset(newcstruct, 0, sizeof(CreditStruct));
	CClientCredits* newcredits = new CClientCredits(newcstruct);
	newcredits->SetSecureIdent(m_abyMyPublicKey,m_nMyPublicKeyLen);
	newcredits->m_dwCryptRndChallengeFrom = challenge;
	// create signature with fake priv key
	uchar pachSignature[200];
	memset(pachSignature,0,200);
	uint8 sigsize = CreateSignature(newcredits,pachSignature,200,0,false, &priv);


	// next fake client uses the random created public key
	CreditStruct* newcstruct2 = new CreditStruct;
	memset(newcstruct2, 0, sizeof(CreditStruct));
	CClientCredits* newcredits2 = new CClientCredits(newcstruct2);
	newcredits2->m_dwCryptRndChallengeFor = challenge;

	// if you uncomment one of the following lines the check has to fail
	//abyPublicKey[5] = 34;
	//m_abyMyPublicKey[5] = 22;
	//pachSignature[5] = 232;

	newcredits2->SetSecureIdent(abyPublicKey,PublicKeyLen);

	//now verify this signature - if it's true everything is fine
	bool bResult = VerifyIdent(newcredits2,pachSignature,sigsize,0,0);

	delete newcredits;
	delete newcredits2;

	return bResult;
}
#endif

//EastShare START - Modified by TAHO, modified SUQWT
/*
uint32 CClientCredits::GetSecureWaitStartTime(uint32 dwForIP)
*/
sint64 CClientCredits::GetSecureWaitStartTime(uint32 dwForIP, bool bDontSetNew) //==sFrQlXeRt=> Fix for SUQWT and Extended credit-table-arrangement
//EastShare END - Modified by TAHO, modified SUQWT
{
	if ((m_dwUnSecureWaitTime == 0 || m_dwSecureWaitTime == 0) && !bDontSetNew) //==sFrQlXeRt=> Fix for SUQWT and Extended credit-table-arrangement
		SetSecWaitStartTime(dwForIP);
	//==sFrQlXeRt=> Fix for SUQWT and Extended credit-table-arrangement
	else if (bDontSetNew)
		return 0;
	//<=sFrQlXeRt== Fix for SUQWT and Extended credit-table-arrangement

	if (m_pCredits->nKeySize != 0){	// this client is a SecureHash Client
		if (GetCurrentIdentState(dwForIP) == IS_IDENTIFIED){ // good boy
			return m_dwSecureWaitTime;
		}
		else{	// not so good boy
			if (dwForIP == m_dwWaitTimeIP){
				return m_dwUnSecureWaitTime;
			}
			else{	// bad boy
				// this can also happen if the client has not identified himself yet, but will do later - so maybe he is not a bad boy :) .
				CString buffer2, buffer;
				/*for (uint16 i = 0;i != 16;i++){
					buffer2.Format("%02X",this->m_pCredits->abyKey[i]);
					buffer+=buffer2;
				}
				if (thePrefs.GetLogSecureIdent())
					AddDebugLogLine(false,"Warning: WaitTime resetted due to Invalid Ident for Userhash %s", buffer);*/
				if(theApp.clientcredits->IsSaveUploadQueueWaitTime()){
						//EastShare START - Modified by TAHO, modified SUQWT
						//m_dwUnSecureWaitTime = ::GetTickCount() - m_pCredits->nUnSecuredWaitTime;	// Moonlight: SUQWT//Morph - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
						m_dwUnSecureWaitTime = ::GetTickCount() - ((sint64) m_pCredits->nUnSecuredWaitTime);
						//EastShare END - Modified by TAHO, modified SUQWT
				}
				else{
					m_dwUnSecureWaitTime = ::GetTickCount();//original
				}
				m_dwWaitTimeIP = dwForIP;
				return m_dwUnSecureWaitTime;
			}	
		}
	}
	else{	// not a SecureHash Client - handle it like before for now (no security checks)
		return m_dwUnSecureWaitTime;
	}
}

//Morph Start - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
// Moonlight: SUQWT - Save the wait times.
void CClientCredits::SaveUploadQueueWaitTime(int iKeepPct) {
	if (m_dwUnSecureWaitTime) m_pCredits->nUnSecuredWaitTime = (uint32)((GetTickCount() - m_dwUnSecureWaitTime) / 100) * iKeepPct;
	if (m_dwSecureWaitTime) m_pCredits->nSecuredWaitTime = (uint32)((GetTickCount() - m_dwSecureWaitTime) / 100) * iKeepPct;
	// EastShare START - Marked by TAHO, modified SUQWT
	// SetSecWaitStartTime(m_dwWaitTimeIP);
	// EastShare END - Marked by TAHO, modified SUQWT
}
// Moonlight: SUQWT - Clear the wait times.
void CClientCredits::ClearUploadQueueWaitTime() {
	m_pCredits->nUnSecuredWaitTime = 0;
	m_pCredits->nSecuredWaitTime = 0;
	// Doing SaveUploadQueueWaitTime(0) should be reduced to something equivalent during compile.
}
//Morph End - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)

//EastShare START - Added by TAHO, modified SUQWT
void CClientCredits::SetSecWaitStartTime() {
	SetSecWaitStartTime(m_dwWaitTimeIP);
}
//EastShare END - Added by TAHO, modified SUQWT

// Moonlight: SUQWT: Adjust to take previous wait time into account.//Morph - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
void CClientCredits::SetSecWaitStartTime(uint32 dwForIP)
{
	//Morph Start - modified by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	if(theApp.clientcredits->IsSaveUploadQueueWaitTime()){
		//EastShare START - Added by TAHO, modified SUQWT
		//m_dwUnSecureWaitTime = ::GetTickCount() - m_pCredits->nUnSecuredWaitTime - 1;	// Moonlight: SUQWT
		//m_dwSecureWaitTime = ::GetTickCount() - m_pCredits->nSecuredWaitTime - 1;		// Moonlight: SUQWT
		m_dwUnSecureWaitTime = ::GetTickCount() - ((sint64) m_pCredits->nUnSecuredWaitTime) - 1;
		m_dwSecureWaitTime = ::GetTickCount() - ((sint64) m_pCredits->nSecuredWaitTime) - 1;
		//EastShare END - Added by TAHO, modified SUQWT
	}
	else{
		//original
		m_dwUnSecureWaitTime = ::GetTickCount()-1;
		m_dwSecureWaitTime = ::GetTickCount()-1;
	}
	//Morph End - modified by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	m_dwWaitTimeIP = dwForIP;
}

void CClientCredits::ClearWaitStartTime()
{
	m_dwUnSecureWaitTime = 0;
	m_dwSecureWaitTime = 0;
}

//EastShare Start - added by AndCycle, Pay Back First

//init will be triggered at 
//1. client credit create, 
//2. when reach 10MB Transferred, between first time remove check and second time remove check
//anyway, this just make a check at "check point" :p

void CClientCredits::InitPayBackFirstStatus(){
	//MORPH START - Changed by SiRoB, Pay Back First Tweak
	m_bPayBackFirst = false;
	TestPayBackFirstStatus();
	//MORPH END   - Changed by SiRoB, Pay Back First Tweak
}

//test will be triggered at client have up/down Transferred
void CClientCredits::TestPayBackFirstStatus(){

	if(GetDownloadedTotal() < 9728000){
		m_bPayBackFirst = false;
	}else
	{
		uint64 clientUpload = GetDownloadedTotal();
		uint64 clientDownload = GetUploadedTotal();
		//MORPH START - Changed by SiRoB, Pay Back First Tweak
		if(clientUpload > clientDownload+((uint64)thePrefs.GetPayBackFirstLimit()<<20)){
		//MORPH END   - Changed by SiRoB, Pay Back First Tweak
			m_bPayBackFirst = true;
		}
		else if(clientUpload < clientDownload){
			m_bPayBackFirst = false;
		}
	}
}
//EastShare End - added by AndCycle, Pay Back First Tweak

//MORPH START - Added by SiRoB, reduce a little CPU usage for ratio count
void CClientCreditsList::ResetCheckScoreRatio(){
	CClientCredits* cur_credit;
	CCKey tempkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	while (pos)
	{
		m_mapClients.GetNextAssoc(pos, tempkey, cur_credit);
		cur_credit->m_bCheckScoreRatio = true;
	}
}
//MORPH END   - Added by SiRoB, reduce a little CPU usage for ratio count