//this file is part of eMule Xtreme-Mod (http://www.xtreme-mod.net)
//Copyright (C)2002-2007 Xtreme-Mod (emulextreme@yahoo.de)

//emule Xtreme is a modification of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

//
//
//	Author: Xman / Maella
//  

#pragma once

#include "Types.h"
#include "Log.h"
#include "opcodes.h"
#include <Iphlpapi.h>
#include <afxtempl.h>


// This class has two purposes:
//
// Collect all statistics relevant to the upload/download bandwidth
// Provide an interface to the LAN adapter (Ethernet, slip, etc...)

class CBandWidthControl 
{
public:
   CBandWidthControl();
   ~CBandWidthControl();

   // Update the history list. Must be called every cycle
   void Process();

   // Actualize current datarate values (Data+Control)
   // Remark: the overhead for the IP/TCP/UDP headers is not included

   //Add width Header:
   /**/ void AddeMuleOutUDPOverall(uint32 octets);
   /**/ void AddeMuleOutTCPOverall(uint32 octets);
   /**/ void AddeMuleInUDPOverall(uint32 octets);
   /**/ void AddeMuleInTCPOverall(uint32 octets);

   //Add without Header (already included)
   /**/ void AddeMuleOutOverallNoHeader(uint32 octets); 
   /**/ void AddeMuleOut(uint32 octets);
   /**/ void AddeMuleOutFriends(uint32 octets); // Multiple friendslots [ZZ] - Mephisto
   /**/ void AddeMuleIn(uint32 octets);
   /**/ void AddeMuleSYNACK();
   //calculating obfuscation
   /**/ void AddeMuleOutObfuscationTCP(uint32 octets);
   /**/ void AddeMuleOutObfuscation(uint32 octets);
   /**/ void AddeMuleOutObfuscationUDP(uint32 octets);
   void AddeMuleInObfuscation(uint32 octets); //only main-thread!!
   /**/ uint64 GeteMuleOutObfuscation() const;
   uint64 GeteMuleInObfuscation() const; //only main-thread!!

   // Accessors, used for the control of the bandwidth (=> slope)
   /**/ uint64 GeteMuleOut() const;
   /**/ uint64 GeteMuleOutFriends() const; // Multiple friendslots [ZZ] - Mephisto
   /**/ uint64 GeteMuleIn() const;
   /**/ uint64 GeteMuleOutOverall() const;
   /**/ uint64 GeteMuleInOverall() const;
   /**/ uint64 GetNetworkOut() const;
   /**/ uint64 GetNetworkIn() const;
   /**/ uint32 GetStartTick() const;

   //Xman show complete internettraffic
   //used for statistic
   uint64	GetSessionNetworkOut() const;
   uint64	GetSessionNetworkIn() const;
   //Xman end

   // Retrieve datarates for barline + upload slot management + Graphic
   void GetDatarates(UINT samples,
                     uint32& eMuleIn, uint32& eMuleInOverall,
                     uint32& eMuleOut, uint32& eMuleOutOverall,
                     uint32& networkIn, uint32& networkOut) const;

   // Retrieve datarates Graphic
   void GetFullHistoryDatarates(uint32& eMuleInHistory, uint32& eMuleOutHistory,
                                uint32& eMuleInSession, uint32& eMuleOutSession) const;

   // Check if the NAFC is available on this computer
   bool IsNAFCAvailable() const {return (m_fGetIfEntry != NULL);}

   // Full NAFC bandwidth control   
   float GetMaxDownload() const {return m_maxDownloadLimit;}
   float GetMaxUpload() const {return m_maxUploadLimit;}

   //Xman 1:3 Ratio (use this instead of GetMaxDownload()
   //Xman GlobalMaxHarlimit for fairness	
	// ==> Enforce Ratio [Stulle] - Stulle
	/*
	float GetMaxDownloadEx(bool force);
	*/
	float GetMaxDownloadEx(uint8 force, uint8 &uReason, uint8 &uRatio);
	// <== Enforce Ratio [Stulle] - Stulle
	float GetForcedDownloadlimit() const {return m_maxforcedDownloadlimit;}

   //Xman new adapter selection
   void checkAdapterIndex(uint32 highid);
   void SetWasNAFCLastActive(bool in) {wasNAFCLastActive=in;}	
   bool GetwasNAFCLastActive() {return wasNAFCLastActive;} 
   void SetBoundIP(uint32 boundip) {boundIP=boundip;}

#ifdef PRINT_STATISTIC
   void PrintStatistic();
#endif

private:
   // Adapter access
   DWORD getAdapterIndex();
   DWORD m_currentAdapterIndex;
   DWORD m_lastAdapterIndex;

	//Xman new adapter selection
   bool wasNAFCLastActive;
   uint32 boundIP;

   // Type definition
   #pragma pack(1)
   struct Statistic {
      Statistic() {}
      Statistic(const Statistic& ref, uint32 time)
      :   eMuleOutOctets(ref.eMuleOutOctets),
          eMuleOutOctetsFriends(ref.eMuleOutOctetsFriends), // Multiple friendslots [ZZ] - Mephisto
          eMuleInOctets(ref.eMuleInOctets),
          eMuleOutOverallOctets(ref.eMuleOutOverallOctets),
          eMuleInOverallOctets(ref.eMuleInOverallOctets),
          networkOutOctets(ref.networkOutOctets),
          networkInOctets(ref.networkInOctets),
          timeStamp(time) {}

      uint64 eMuleOutOctets; // Data
      uint64 eMuleOutOctetsFriends; // Multiple friendslots [ZZ] - Mephisto
      uint64 eMuleInOctets;
      uint64 eMuleOutOverallOctets; // Data+Control
      uint64 eMuleInOverallOctets;
      uint64 networkOutOctets; // DataFlow of the network Adapter
      uint64 networkInOctets;
      uint32 timeStamp; // Use a time stamp to compensate the inaccuracy of the timer (based on enkeyDEV(Ottavio84))
   };
   #pragma pack()
   typedef CList<Statistic> StatisticHistory; // Use MS container for its memory management

   StatisticHistory m_statisticHistory; // History for the graphic
   /**/ Statistic m_statistic; // Current value

   float m_maxDownloadLimit; // Used for auto U/D limits
   float m_maxUploadLimit;

   //Xman GlobalMaxHarlimit for fairness
	float m_maxforcedDownloadlimit;

   //Xman thread save
   mutable CCriticalSection m_statisticLocker;  

    // Keep last result to detect an overflow
   DWORD m_networkOutOctets;
   DWORD m_networkInOctets;

	//calculating obfuscation
   uint64 m_obfuscation_InOctets;
   uint64 m_obfuscation_OutOctets;

   bool m_errorTraced;

   // Dynamic access to the iphlpapi.dll
   typedef DWORD (WINAPI *GETNUMBEROFINTERFACES)(PDWORD pdwNumIf);
   typedef DWORD (WINAPI *GETIFTABLE)(PMIB_IFTABLE pIfTable, PULONG pdwSize, BOOL bOrder);
   typedef DWORD (WINAPI *GETIPADDRTABLE)(PMIB_IPADDRTABLE pIpAddrTable, PULONG pdwSize, BOOL bOrder);
   typedef DWORD (WINAPI *GETIFENTRY)(PMIB_IFROW pIfRow);

   HINSTANCE m_hIphlpapi;
   GETIFTABLE m_fGetIfTable;
   GETIPADDRTABLE m_fGetIpAddrTable;
   GETIFENTRY m_fGetIfEntry;
   GETNUMBEROFINTERFACES m_fGetNumberOfInterfaces;

private:
   // Don't allow canonical behavior
   CBandWidthControl(const CBandWidthControl&);
   CBandWidthControl& operator=(const CBandWidthControl&);

	// ==> Enforce Ratio [Stulle] - Stulle
	float m_maxforcedDownloadlimitEnforced;
	float m_fMaxDownloadEqualUploadLimit;
	uint8 m_uNAFCRatio;
public:
	float GetForcedDownloadlimitEnforced() const {return m_maxforcedDownloadlimitEnforced;}
	float GetMaxDownloadEqualUploadLimit() const {return m_fMaxDownloadEqualUploadLimit;}
	// <== Enforce Ratio [Stulle] - Stulle

	// ==> Multiple friendslots [ZZ] - Mephisto
	float GetMaxDownloadFriends() const {return m_fMaxDownloadFriends;}
private:
	float m_fMaxDownloadFriends;
	// <== Multiple friendslots [ZZ] - Mephisto
};