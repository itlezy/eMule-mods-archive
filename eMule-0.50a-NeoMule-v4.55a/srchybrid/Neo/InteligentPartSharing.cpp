//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif
#include <io.h>
#include "emule.h"
#include "KnownFile.h"
#include "KnownFileList.h"
#include "SharedFileList.h"
#include "UpDownClient.h"
#include "ClientList.h"
#include "opcodes.h"
#include "ini2.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "NeoPreferences.h"
#include "PartFile.h"
#include "Packets.h"
#include "SafeFile.h"
#include "Log.h"
#include "emuledlg.h"
#include "NeoOpCodes.h"
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "Neo/ClientFileStatus.h" // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// NEO: IPS - [InteligentPartSharing] -- Xanatos -->
uint8 CKnownFile::GetPartState(UINT part) const
{
	//if(GetPartCount() <= 1)
	//	return PR_PART_ON;

	uint8 status = KnownPrefs->GetManagedPart(part); // NEO: MPS - [ManualPartSharing]
	if ((status == PR_PART_NORMAL) && const_cast <CKnownFile*> (this)->KnownPrefs->UseInteligentPartSharing())
		return GetIPSPartStatus(part);

	return (status == PR_PART_NORMAL) ? PR_PART_ON : status;
}

/*uint8 CKnownFile::GetPartState(UINT part, CClientFileStatus* status) const
{
	// Note: We could apply part catch also here, but i think its not nessesery
	if (status && status->IsPartAvailable(part)) // Don't hide Parts that the client olready have
		return PR_PART_ON;

	return GetPartState(part);
}*/

bool CKnownFile::WritePartSelection(CSafeMemFile* file, CUpDownClient* client)
{
	if (KnownPrefs->UseInteligentPartSharing())
		ReCalculateIPS();

	CClientFileStatus* status = client->GetFileStatus(this); // NEO: SCFS - [SmartClientFileStatus]

	CMap<UINT, UINT, BOOL, BOOL> HideMap;
	GetHideMap(status, HideMap);
	if(HideMap.IsEmpty())
		return false;

	UINT parts = GetED2KPartCount();
	file->WriteUInt16((uint16)parts);

	UINT done = 0;
	while (done != parts){
		uint8 towrite = 0;
		for (UINT i = 0; i < 8; i++){
			if (HideMap[done] == FALSE)
				towrite |= (1<<i);
			done++;
			if (done == parts)
				break;
		}
		file->WriteUInt8(towrite);
	}

	return true;
}

void CKnownFile::GetHideMap(CClientFileStatus* status, CMap<UINT, UINT, BOOL, BOOL> &HideMap) const // NEO: SCFS - [SmartClientFileStatus]
{
	if(GetPartCount() <= 1)
		return;

	UINT ShowNeededParts = 0;
	UINT HidenParts = 0;
	int ShowInc = IsIPSforThisEnabled(KnownPrefs->IsShowAlwaysIncompleteParts());
	for (UINT i = 0; i < GetPartCount(); i++){
		if(!status || (!status->IsPartAvailable(i)
		&& !(ShowInc && status->IsIncPartAvailable(i))
		|| GetPartState(i) == PR_PART_OFF)) { // off parts can not be showen
			if(GetPartState(i) != PR_PART_ON){
				HideMap[i] = TRUE;
				HidenParts++;
			}else if(!IsPartFile() || ((CPartFile*)this)->IsComplete(i*PARTSIZE, (i + 1)*PARTSIZE - 1, true))
				ShowNeededParts++;
		}
	}
	
	if(!IsIPSforThisEnabled(KnownPrefs->IsShowAlwaysSomeParts())
	  || ShowNeededParts >= (UINT)KnownPrefs->GetShowAlwaysSomePartsValue() || m_IPSPartsInfo == NULL)
		return;

	// Wee need to show some more parts
	BOOL val;
	UINT part;
	for (UINT i = m_IPSPartsInfo->GetCount(); i > 0; i--){
		if(!m_IPSPartsInfo->GetAt(i-1).OL)
			continue;
		part = m_IPSPartsInfo->GetAt(i-1).Part;

		if(!HidenParts || ShowNeededParts >= (UINT)KnownPrefs->GetShowAlwaysSomePartsValue())
			break;

		if(HideMap.Lookup(part,val) && val == TRUE && (!status || !status->IsPartAvailable(i)) 
		&& (!IsPartFile() || ((CPartFile*)this)->IsComplete(part*PARTSIZE, (part + 1)*PARTSIZE - 1, true))
		&& GetPartState(part) != PR_PART_OFF // We can't show blocked parts
		&& KnownPrefs->GetManagedPart(part) == PR_PART_NORMAL // Manual settings are always kept
		){ 
			HideMap.RemoveKey(part);
			HidenParts--;
			ShowNeededParts++;
		}
	}
}

uint8 CKnownFile::GetIPSPartStatus(UINT part) const
{ 
	uint8 status = PR_PART_ON; 
	if(m_IPSPartStatus.Lookup(part, status)) 
		return status; 
	else 
		return PR_PART_ON; 
}

void CKnownFile::ResetIPSList()
{
	for(UINT part=0; part < GetPartCount(); part++)
		SetIPSPartStatus(part, PR_PART_ON);
}

void CKnownFile::ReCalculateIPS() const
{
	if (::GetTickCount() - m_uLastIPSCalcTime > KnownPrefs->GetInteligentPartSharingTimerMs())
		return;

	CKnownFile* This = const_cast <CKnownFile*> (this);
	This->m_uLastIPSCalcTime = ::GetTickCount();
	This->CalculateIPS();
}

double GetOverLoad1(double Part,double MinimalPart, double OL){
	if (!MinimalPart) MinimalPart = 1;
	double Temp = Part / MinimalPart;
	return (Temp >= OL) ? (Temp / OL) : 0;
}

double GetOverLoad2(double Part,double MinimalPart, double OL){
	double Temp = Part - MinimalPart;
	return (Temp >= OL) ? (Temp /*-*/ / OL) : 0;
}

double GetUnderLoad1(double Part, double NormalPart, double UL){
	if(!Part)
		return 1;
	double Temp = NormalPart / Part;
	return (Temp >= UL) ? (Temp / UL) : 0;
}

double GetUnderLoad2(double Part, double NormalPart, double UL){
	double Temp = NormalPart - Part;
	if (Temp < 0)
		return 0; 
	return (Temp > UL) ? (Temp /*-*/ / UL) : 0;	
}

static void IPSHeapSort(CArray<TPartOL,TPartOL> &OLParts, uint32 first, uint32 last){
	uint32 r;
	for ( r = first; !(r & 0x80000000) && (r<<1) < last; ){
		uint32 r2 = (r<<1)+1;
		if (r2 != last)
			if (OLParts[r2].OL /*<*/ > OLParts[r2+1].OL)
				r2++;
		if (OLParts[r].OL /*<*/ > OLParts[r2].OL){
			TPartOL t = OLParts[r2];
			OLParts[r2] = OLParts[r];
			OLParts[r] = t;
			r = r2;
		}
		else
			break;
	}
}

static void IPSHeapMov(CArray<TPartOL,TPartOL> &OLParts, uint32 r){
	TPartOL t = OLParts[r];
	OLParts[r] = OLParts[0];
	OLParts[0] = t;
}

double CKnownFile::GetPartShared(uint16 part)
{
	if(KnownPrefs->GetHideOverSharedCalc())
		return statistic.GetPartRelease(part); // Lo
	else
		return (float)statistic.GetPartTraffic(part)/GetPartSize((uint16)part); // Hi
}

void CKnownFile::CalculateIPS()
{
	if(GetPartCount()<=1) 
		return;

	// Initialising
	m_nCompleteSourcesTime = 0; // force update
	UpdatePartsInfo();

	ResetIPSList();

	// Preparing
	const bool isPartFile = IsPartFile();
	UINT PartCount = GetPartCount();

	UINT DonePartCount = 0;
	CArray<bool,bool> Donemap;	Donemap.SetSize(PartCount);
	if (isPartFile)
		for (UINT i = 0; i < PartCount; i++){
			if (((CPartFile*)this)->IsComplete(i*PARTSIZE, ((i+1)*PARTSIZE)-1, true)){
				DonePartCount++;
				Donemap[i] = true;
			}else{
				Donemap[i] = false;
			}
		}

	UINT tmpParts = isPartFile ? DonePartCount : PartCount;
	UINT MaxPartsToHide = tmpParts * KnownPrefs->GetMaxProzentToHide() / 100;
	if (MaxPartsToHide >= tmpParts) // Never hide All !!!
		MaxPartsToHide = tmpParts - 1;

	UINT MinimalAvalibility = GetPartAvailibility(0);
	UINT NormalAvalibility = GetPartAvailibility(0);
	double MinimalShared = GetPartShared(0);
	//if (!MinimalShared) MinimalShared = 1;
	for (UINT i = 1/*0*/; i < PartCount; i++){
		// OA
		UINT PartAvailibility = GetPartAvailibility((uint16)i);
		if(MinimalAvalibility > PartAvailibility)
			MinimalAvalibility = PartAvailibility;
		NormalAvalibility += PartAvailibility;
		// Only Parts we have for OS
		if (isPartFile && !Donemap[i])
			continue;
		// OS
		double PartShared = GetPartShared((uint16)i);
		//if(!PartShared)PartShared = 1;
		if(MinimalShared > PartShared)
			MinimalShared = PartShared;
	}
	NormalAvalibility /= PartCount;
	//if (!NormalAvalibility)		NormalAvalibility = 1;
	//if (!MinimalAvalibility)	MinimalAvalibility = 1;
	
	// Make things faster ;)
	double(*GetOverAvalibly)(double,double,double);		GetOverAvalibly = KnownPrefs->GetHideOverAvaliblyMode() ? &GetOverLoad2 : &GetOverLoad1;
	double(*GetOverShare)(double,double,double);		GetOverShare = KnownPrefs->GetHideOverSharedMode() ? &GetOverLoad2 : &GetOverLoad1;
	double(*GetUnderAvalibly)(double,double,double);	GetUnderAvalibly = KnownPrefs->GetDontHideUnderAvaliblyMode() ? &GetUnderLoad2 : &GetUnderLoad1;
	const float HideOAval = (float)KnownPrefs->GetHideOverAvaliblyValue()/100.0F;
	const float HideOSval = (float)KnownPrefs->GetHideOverSharedValue()/100.0F;
	const float nHideUAval = (float)KnownPrefs->GetDontHideUnderAvaliblyValue()/100.0F;
	if (!HideOAval || !HideOSval || !nHideUAval){
		ASSERT(FALSE);
		return;
	}

	// Hide Part Arrays
	CArray<TPartOL,TPartOL> HideOAmap;	HideOAmap.SetSize(PartCount);
	CArray<TPartOL,TPartOL> HideOSmap;	HideOSmap.SetSize(PartCount);

	// Hide OA
	const bool HideOA  = IsIPSforThisEnabled(KnownPrefs->IsHideOverAvaliblyParts());
	if (HideOA){
		double OA = 0;
		for (UINT i = 0; i < PartCount; i++){
			OA = GetOverAvalibly(GetPartAvailibility((uint16)i), MinimalAvalibility, HideOAval);
			HideOAmap[i].Part = i;
			if(OA){
				HideOAmap[i].OL = OA;
			}else{
				HideOAmap[i].OL = 0;
			}
		}
	}

	// Hide OS
	const bool HideOS  = IsIPSforThisEnabled(KnownPrefs->IsHideOverSharedParts()) && NeoPrefs.UsePartTraffic();
	const bool DontHideUA = IsIPSforThisEnabled(KnownPrefs->IsDontHideUnderAvaliblyParts());
	if (HideOS){
		double OS = 0;
		for (UINT i = 0; i < PartCount; i++){
			OS = GetOverShare(GetPartShared((uint16)i), MinimalShared, HideOSval);
			HideOSmap[i].Part = i;
			if(OS && (!DontHideUA || !GetUnderAvalibly(GetPartAvailibility((uint16)i), NormalAvalibility /*MinimalAvalibility*/, nHideUAval))){
				HideOSmap[i].OL = OS;
			}else{
				HideOSmap[i].OL = 0;
			}
		}
	}

	// Final Array
	CArray<TPartOL,TPartOL> *HideParts;

	// Prepare Final Array, combinate OA and OS to OL
	CArray<TPartOL,TPartOL> HideOLmap;
	if (HideOS && HideOA) {
		HideOLmap.SetSize(PartCount);
		double OA; 
		double OS;
		for (UINT i = 0; i < PartCount; i++) {
			OA = HideOAmap[i].OL;
			OS = HideOSmap[i].OL;
			HideOLmap[i].Part = i;
			HideOLmap[i].OL = OS + OA;
		}
		HideParts = &HideOLmap;
	} else if (HideOA) {
		HideParts = &HideOAmap;
	} else if (HideOS) {
		HideParts = &HideOSmap;
	} else {
		//ASSERT(FALSE); // can happen when manualy triggerd
		return;
	}

	// Sort Array
	int n = HideParts->GetSize();
	if (n > 0)
	{
		int r;
		for (r = n/2; r--; )
			IPSHeapSort(*HideParts, r, n-1);
		for (r = n; --r; ){
			IPSHeapMov(*HideParts,r);
			IPSHeapSort(*HideParts, 0, r-1);
		}
	}

	// Prepare Blocking
	const int BlockOA = IsIPSforThisEnabled(KnownPrefs->IsBlockHighOverAvaliblyParts());
	const int BlockOS = IsIPSforThisEnabled(KnownPrefs->IsBlockHighOverSharedParts());
	const float BlockOAval = HideOAval * (float)KnownPrefs->GetBlockHighOverAvaliblyFactor()/100;
	const float BlockOSval = HideOSval * (float)KnownPrefs->GetBlockHighOverSharedFactor()/100;
	if (!BlockOSval || !BlockOAval){
		ASSERT(FALSE);
		return;
	}

	if(IsIPSforThisEnabled(KnownPrefs->IsShowAlwaysSomeParts())){
		if(!m_IPSPartsInfo)
			m_IPSPartsInfo = new CArray<TPartOL,TPartOL>;
			
		if((UINT)m_IPSPartsInfo->GetSize() < PartCount)
			m_IPSPartsInfo->SetSize(PartCount);

		for (UINT i = 0; i < PartCount; i++)
			m_IPSPartsInfo->SetAt(i,HideParts->GetAt(i));
	}

	// Hide Parts
	bool Block;
	for (UINT i = 0; i < MaxPartsToHide; i++) {

		// Note: We can hide a Over Available part that we dont have, this is OK, becouse we will share more need parts.
		if (!HideParts->GetAt(i).OL)
			break;

		UINT	part = HideParts->GetAt(i).Part;

		// Block Parts
		Block = 
			((
			 BlockOA && 
			 (GetOverAvalibly(GetPartAvailibility((uint16)part), MinimalAvalibility, BlockOAval))
			)||(
			 BlockOS && 
			 (GetOverShare(GetPartShared((uint16)part), MinimalShared, BlockOSval))
			));

		// Set Part Status
		SetIPSPartStatus(part, Block ? PR_PART_OFF : PR_PART_HIDEN);
	}


}
// NEO: IPS END <-- Xanatos --