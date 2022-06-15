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

#pragma once
// NEO: FCFG - [FileConfiguration] -- Xanatos -->

///////////////////////////////////////////////////////////////////////
// Neo File Preferences
//

#define FCFG_BASE	2000000000	// lets take somethink easy to remember 2 eight 0's and a number // INT_MAX = 2147483647
#define FCFG_DEF	FCFG_BASE+1	// use default settings of the previuse level
#define FCFG_GLB	FCFG_BASE+2	// use default global setting
#define FCFG_AUT	FCFG_BASE+3	// use automatic value (only one valid for the base)
#define FCFG_STD	FCFG_BASE+4	// use standard value
#define FCFG_UNK	FCFG_BASE+5	// state unknown different files have different falues

#define FCFG_INI_DEF	_T("def")
#define FCFG_INI_GLB	_T("glb")
#define FCFG_INI_AUT	_T("aut")

#include <list>
#include "Neo/PrefFunctions.h"

// NEO: SDT - [SourcesDropTweaks]
#define SDT_LIMIT_MODE_TOTAL		0 // look on the number of sources the file have
#define SDT_LIMIT_MODE_RELATIV		1 // look on the percentage of sources of this type
#define SDT_LIMIT_MODE_SPECIFIC		2 // look on the amount of sources of this type

#define SDT_TIME_MODE_PROGRESSIV	0 // official, drop one source in specyfyed time intervals
#define SDT_TIME_MODE_DISTRIBUTIV	1 // drop a source if he ad this satete for at least the specyfyed time
#define SDT_TIME_MODE_CUMMULATIV	2 // drop all sources of this type in specyfyed intervals

#define SDT_HIGHQ_MODE_NORMAL		0 // normal query rank limit
#define SDT_HIGHQ_MODE_AVERAGE		1 // if rank is this percentage above the average
// NEO: SDT END

#define PR_PART_NORMAL 		0
// NEO: MPS - [ManualPartSharing]
#define PR_PART_ON 			1
#define PR_PART_HIDEN		2
#define PR_PART_OFF			3
// NEO: MPS END
#define PR_PART_WANTED		9 // NEO: MCS - [ManualChunkSelection]

enum EFilePrefsLevel{
	CFP_GLOBAL = 0,
	CFP_CATEGORY = 1,
	CFP_FILE = 2
};


// NEO: IPS - [InteligentPartSharing]
#define IPS_OFF	0 // dissabled
#define IPS_KF	1 // for known files
#define IPS_ON	2 // for known and part files
#define IPS_PF	3 // for part files
#define IsIPSforThisEnabled(x) (x == IPS_ON || (x == IPS_KF && !IsPartFile()) || (x == IPS_PF && IsPartFile())) 

#define IPS_MLT 0 // multiplicativ mode
#define IPS_ADD 1 // additiv mode (like HideOS)

#define IPS_HI	0 // High
#define IPS_LO	1 // Low (acurate)
// NEO: IPS END

// NEO: SRS - [SmartReleaseSharing]
#define REL_MIXED		0
#define REL_BOOST		1
#define REL_POWER		2

#define REL_SIMPLY		0
#define REL_LINEAR		1
#define REL_EXPONENTIAL	2

#define LIM_DISABLE		0
#define LIM_SINGLE		1
#define LIM_BOOTH		2

#define LNK_AND			0
#define LNK_OR			1
// NEO: SRS END

class CTag;
class CKnownFile;
class CPartFile;
class CFileDataIO;
class CIni;

#define	PARTNEO_EXT		_T(".neo")
#define	PARTNEO_BAK_EXT	_T(".bak")
#define	PARTNEO_TMP_EXT	_T(".backup")

///////////////////////////////////////////////////////////////////////
// CKnownPreferences
//

class CKnownPreferences
{
public:
	CKnownPreferences();
	~CKnownPreferences();

	void	Save(CFileDataIO* file);
	bool	Load(CFileDataIO* file);
	void	Save(CIni& ini);
	bool	Load(CIni& ini);

	virtual bool	IsEmpty() const							{return false;}

	virtual bool	IsGlobalPrefs() const					{return true;}
	virtual bool	IsCategoryPrefs() const					{return false;}
	virtual bool	IsFilePrefs() const						{return false;}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	virtual int		IsEnableLanCast() const				{return m_EnableLanCast;}
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	virtual int		IsEnableVoodoo() const				{return m_EnableVoodoo;}
#endif // VOODOO // NEO: VOODOO END

	// NEO: IPS - [InteligentPartSharing]
	virtual	int		UseInteligentPartSharing() const		{return m_UseInteligentPartSharing;}
	virtual	int		GetInteligentPartSharingTimer() const	{return m_InteligentPartSharingTimer;}
	UINT			GetInteligentPartSharingTimerMs()		{return SEC2MS(GetInteligentPartSharingTimer());}

	virtual	int		GetMaxProzentToHide() const				{return m_MaxProzentToHide;}

	// OverAvalibly
	virtual	int		IsHideOverAvaliblyParts() const			{return m_HideOverAvaliblyParts;}
	virtual	int		GetHideOverAvaliblyMode() const			{return m_HideOverAvaliblyMode;}
	virtual	int		GetHideOverAvaliblyValue() const		{return m_HideOverAvaliblyValue;}

	virtual	int		IsBlockHighOverAvaliblyParts() const	{return m_BlockHighOverAvaliblyParts;}
	virtual	int		GetBlockHighOverAvaliblyFactor() const	{return m_BlockHighOverAvaliblyFactor;}

	// OverShared
	virtual	int		IsHideOverSharedParts() const			{return m_HideOverSharedParts;}
	virtual	int		GetHideOverSharedMode() const			{return m_HideOverSharedMode;}
	virtual	int		GetHideOverSharedValue() const			{return m_HideOverSharedValue;}
	virtual	int		GetHideOverSharedCalc() const			{return m_HideOverSharedCalc;}

	virtual	int		IsBlockHighOverSharedParts() const		{return m_BlockHighOverSharedParts;}
	virtual	int		GetBlockHighOverSharedFactor() const	{return m_BlockHighOverSharedFactor;}

	// DontHideUnderAvalibly
	virtual	int		IsDontHideUnderAvaliblyParts() const	{return m_DontHideUnderAvaliblyParts;}
	virtual	int		GetDontHideUnderAvaliblyMode() const	{return m_DontHideUnderAvaliblyMode;}
	virtual	int		GetDontHideUnderAvaliblyValue() const	{return m_DontHideUnderAvaliblyValue;}

	// Other
	virtual	int		IsShowAlwaysSomeParts() const			{return m_ShowAlwaysSomeParts;}
	virtual	int		GetShowAlwaysSomePartsValue() const		{return m_ShowAlwaysSomePartsValue;}

	virtual	int		IsShowAlwaysIncompleteParts() const		{return m_ShowAlwaysIncompleteParts;}
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	virtual	int		GetReleaseMode() const					{return m_ReleaseMode;}
	virtual	int		GetReleaseLevel() const					{return m_ReleaseLevel;}
	virtual	int		GetReleaseTimer() const					{return m_ReleaseTimer;}

	// release limit
	virtual	int		IsReleaseLimit() const					{return m_ReleaseLimit;}
	virtual	int		GetReleaseLimitMode() const				{return m_ReleaseLimitMode;}
	virtual	int		GetReleaseLimitHigh() const				{return m_ReleaseLimitHigh;}
	virtual	int		GetReleaseLimitLow() const				{return m_ReleaseLimitLow;}

	virtual	int		IsReleaseLimitLink() const				{return m_ReleaseLimitLink;}

	virtual	int		IsReleaseLimitComplete() const			{return m_ReleaseLimitComplete;}
	virtual	int		GetReleaseLimitCompleteMode() const		{return m_ReleaseLimitCompleteMode;}
	virtual	int		GetReleaseLimitCompleteHigh() const		{return m_ReleaseLimitCompleteHigh;}
	virtual	int		GetReleaseLimitCompleteLow() const		{return m_ReleaseLimitCompleteLow;}

	// limit
	virtual	int		IsLimitLink() const					{return m_LimitLink;}

	// source limit
	virtual	int		IsSourceLimit() const					{return m_SourceLimit;}
	virtual	int		GetSourceLimitMode() const				{return m_SourceLimitMode;}
	virtual	int		GetSourceLimitHigh() const				{return m_SourceLimitHigh;}
	virtual	int		GetSourceLimitLow() const				{return m_SourceLimitLow;}

	virtual	int		IsSourceLimitLink() const				{return m_SourceLimitLink;}

	virtual	int		IsSourceLimitComplete() const			{return m_SourceLimitComplete;}
	virtual	int		GetSourceLimitCompleteMode() const		{return m_SourceLimitCompleteMode;}
	virtual	int		GetSourceLimitCompleteHigh() const		{return m_SourceLimitCompleteHigh;}
	virtual	int		GetSourceLimitCompleteLow() const		{return m_SourceLimitCompleteLow;}
	// NEO: SRS END

	// NEO: MPS - [ManualPartSharing]
	virtual	void	SetManagedPart(UINT /*part*/, uint8 /*status*/) { ASSERT(0); }
	virtual	uint8	GetManagedPart(UINT /*part*/) const	{return PR_PART_NORMAL;}
	virtual	bool	HasManagedParts() const				{return false;}
	// NEO: MPS END

	virtual void	CheckTweaks();
	virtual void	ResetTweaks() {ASSERT(0);}

protected:
	friend class CPPgNeo;
	friend class CPPgRelease;
	friend class CPPgLancast;
	friend class CSharedFilesCtrl;
	
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	int		m_EnableLanCast;						// Flag
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	int		m_EnableVoodoo;							// Flag
#endif // VOODOO // NEO: VOODOO END

	// NEO: IPS - [InteligentPartSharing]
	int		m_UseInteligentPartSharing;				// Flag
	int		m_InteligentPartSharingTimer;			// Value

	int		m_MaxProzentToHide;						// Value

	// OverAvalibly
	int		m_HideOverAvaliblyParts;				// Flag
	int		m_HideOverAvaliblyMode;					// Flag
	int		m_HideOverAvaliblyValue;				// Value

	int		m_BlockHighOverAvaliblyParts;			// Flag
	int		m_BlockHighOverAvaliblyFactor;			// Value

	// OverShared
	int		m_HideOverSharedParts;					// Flag
	int		m_HideOverSharedMode;					// Flag
	int		m_HideOverSharedValue;					// Value
	int		m_HideOverSharedCalc;					// Flag

	int		m_BlockHighOverSharedParts;				// Flag
	int		m_BlockHighOverSharedFactor;			// Value

	// DontHideUnderAvalibly
	int		m_DontHideUnderAvaliblyParts;			// Flag
	int		m_DontHideUnderAvaliblyMode;			// Flag
	int		m_DontHideUnderAvaliblyValue;			// Value

	// Other
	int		m_ShowAlwaysSomeParts;					// Flag
	int		m_ShowAlwaysSomePartsValue;				// Value

	int		m_ShowAlwaysIncompleteParts;			// Flag
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	int		m_ReleaseMode;							// Flag
	int		m_ReleaseLevel;							// Value
	int		m_ReleaseTimer;							// Value

	// release limit
	int		m_ReleaseLimit;							// Flag
	int		m_ReleaseLimitMode;						// Flag
	int		m_ReleaseLimitHigh;						// Value
	int		m_ReleaseLimitLow;						// Value

	int		m_ReleaseLimitLink;						// Flag

	int		m_ReleaseLimitComplete;					// Flag
	int		m_ReleaseLimitCompleteMode;				// Flag
	int		m_ReleaseLimitCompleteHigh;				// Value
	int		m_ReleaseLimitCompleteLow;				// Value

	// limit
	int		m_LimitLink;							// Flag

	// source limit
	int		m_SourceLimit;							// Flag
	int		m_SourceLimitMode;						// Flag
	int		m_SourceLimitHigh;						// Value
	int		m_SourceLimitLow;						// Value

	int		m_SourceLimitLink;						// Flag

	int		m_SourceLimitComplete;					// Flag
	int		m_SourceLimitCompleteMode;				// Flag
	int		m_SourceLimitCompleteHigh;				// Value
	int		m_SourceLimitCompleteLow;				// Value
	// NEO: SRS END

	CMap<UINT, UINT, uint8, uint8> m_ManagedParts; // NEO: MPS - [ManualPartSharing]

	CArray<CTag*, CTag*> taglist;

private:
	void ClearTags();
};

///////////////////////////////////////////////////////////////////////
// CPartPreferences
//

class CPartPreferences
{
public:
	CPartPreferences();
	~CPartPreferences();

	void	Save(CFileDataIO* file);
	bool	Load(CFileDataIO* file);
	void	Save(CIni& ini);
	bool	Load(CIni& ini);

	virtual bool	IsEmpty() const					{return false;}

	virtual bool	IsGlobalPrefs() const			{return true;}
	virtual bool	IsCategoryPrefs() const			{return false;}
	virtual bool	IsFilePrefs() const				{return false;}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	virtual int		GetLcIntervals() const				{return m_LcIntervals;}
	UINT			GetLcIntervalsMs() const			{return SEC2MS(GetLcIntervals());}

	virtual int		GetLanSourceReaskTime() const		{return m_LanSourceReaskTime;}
	UINT			GetLanSourceReaskTimeMs() const		{return SEC2MS(GetLanSourceReaskTime());}
	virtual int		GetLanNNPSourceReaskTime() const	{return m_LanNNPSourceReaskTime;}
	UINT			GetLanNNPSourceReaskTimeMs() const	{return SEC2MS(GetLanNNPSourceReaskTime());}
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	virtual int		IsVoodooXS() const					{return m_VoodooXS;}
#endif // VOODOO // NEO: VOODOO END

	// NEO: SRT - [SourceRequestTweaks]
	// General
	virtual int		GetSourceLimit() const			{return m_SourceLimit;}
	virtual void	SetSourceLimit(int in)			{m_SourceLimit = in;}
	

	// Management
	virtual int		GetSwapLimit() const			{return m_SwapLimit;}

	//XS
	virtual int		IsXsEnable() const				{return m_XsEnable;}
	virtual int		GetXsLimit() const				{return m_XsLimit;}
	virtual int		GetXsIntervals() const			{return m_XsIntervals;}
	UINT			GetXsIntervalsMs() const		{return SEC2MS(GetXsIntervals());}
	virtual int		GetXsClientIntervals() const	{return m_XsClientIntervals;}
	UINT			GetXsClientIntervalsMs() const	{return SEC2MS(GetXsClientIntervals());}
	virtual int		GetXsCleintDelay() const		{return m_XsCleintDelay;}
	virtual int		GetXsRareLimit() const			{return m_XsRareLimit;}

	// SVR
	virtual int		IsSvrEnable() const				{return m_SvrEnable;}
	virtual int		GetSvrLimit() const				{return m_SvrLimit;}
	virtual int		GetSvrIntervals() const			{return m_SvrIntervals;}
	UINT			GetSvrIntervalsMs() const		{return SEC2MS(GetSvrIntervals());}

	//KAD
	virtual int		IsKadEnable() const				{return m_KadEnable;}
	virtual int		GetKadLimit() const				{return m_KadLimit;}
	virtual int		GetKadIntervals() const			{return m_KadIntervals;}
	UINT			GetKadIntervalsMs() const		{return SEC2MS(GetKadIntervals());}
	virtual int		GetKadMaxFiles() const			{return m_KadMaxFiles;}
	virtual int		GetKadRepeatDelay() const		{return m_KadRepeatDelay;}

	//UDP
	virtual int		IsUdpEnable() const				{return m_UdpEnable;}
	virtual int		GetUdpLimit() const				{return m_UdpLimit;}
	virtual int		GetUdpIntervals() const			{return m_UdpIntervals;}
	UINT			GetUdpIntervalsMs() const		{return SEC2MS(GetUdpIntervals());}
	virtual int		GetUdpGlobalIntervals() const	{return m_UdpGlobalIntervals;}
	UINT			GetUdpGlobalIntervalsMs() const	{return SEC2MS(GetUdpGlobalIntervals());}
	virtual int		GetUdpFilesPerServer() const	{return m_UdpFilesPerServer;}
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	virtual int		UseSourceCache() const			{return m_UseSourceCache;}
	virtual int		GetSourceCacheLimit() const		{return m_SourceCacheLimit;}
	virtual int		GetSourceCacheTime() const		{return m_SourceCacheTime;}
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	virtual int		UseAutoSoftLock() const			{return m_AutoSoftLock;}
	virtual int		GetAutoSoftLockLimit() const	{return m_AutoSoftLockLimit;}
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	virtual int		UseAutoHardLimit() const		{return m_AutoHardLimit;}
	virtual int		GetAutoHardLimitTime() const	{return m_AutoHardLimitTime;}
	UINT			GetAutoHardLimitTimeMs() const	{return SEC2MS(GetAutoHardLimitTime());}
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	virtual int		UseCategorySourceLimit() const			{return m_CategorySourceLimit;}
	virtual int		GetCategorySourceLimitLimit() const		{return m_CategorySourceLimitLimit;}
	virtual int		GetCategorySourceLimitTime() const		{return m_CategorySourceLimitTime;}
	UINT			GetCategorySourceLimitTimeMs() const	{return SEC2MS(GetCategorySourceLimitTime());}
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	virtual int		UseGlobalSourceLimit() const		{return m_GlobalSourceLimit;}
	virtual int		GetGlobalSourceLimitLimit() const	{return m_GlobalSourceLimitLimit;}
	virtual int		GetGlobalSourceLimitTime() const	{return m_GlobalSourceLimitTime;}
	UINT			GetGlobalSourceLimitTimeMs() const	{return SEC2MS(GetGlobalSourceLimitTime());}
	// NEO: GSL END
	
	virtual int		GetMinSourcePerFile() const			{return m_MinSourcePerFile;}

	// NEO: TCR - [TCPConnectionRetry]
	virtual int		GetTCPConnectionRetry() const		{return m_TCPConnectionRetry;}
	int				GetTCPNewConnectionRetry() const	{return (GetTCPConnectionRetry()+1)/2;}
	// NEO: TCR END

	// NEO: DRT - [DownloadReaskTweaks]
	virtual int		UseSpreadReaskEnable() const		{return m_SpreadReaskEnable;}
	virtual int		GetSpreadReaskTime() const			{return m_SpreadReaskTime;}
	UINT			GetSpreadReaskTimeMs() const		{return SEC2MS(GetSpreadReaskTime());}
	virtual int		GetSourceReaskTime() const			{return m_SourceReaskTime;}
	UINT			GetSourceReaskTimeMs() const		{return SEC2MS(GetSourceReaskTime());}
	UINT			GetSourceReaskTimeZZMs() const		{return (GetSourceReaskTimeMs() * 7) / 10;}
	virtual int		GetFullQSourceReaskTime() const		{return m_FullQSourceReaskTime;}
	UINT			GetFullQSourceReaskTimeMs() const	{return SEC2MS(GetFullQSourceReaskTime());}
	virtual int		GetNNPSourceReaskTime() const		{return m_NNPSourceReaskTime;}
	UINT			GetNNPSourceReaskTimeMs() const		{return SEC2MS(GetNNPSourceReaskTime());}
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	virtual int		GetDropTime() const					{return m_DropTime;}
	UINT			GetDropTimeMs() const				{return SEC2MS(GetDropTime());}

	//Bad
	virtual int		UseBadSourceDrop() const			{return m_BadSourceDrop;}
	virtual int		GetBadSourceLimit() const			{return m_BadSourceLimit;}
	virtual int		GetBadSourceLimitMode() const		{return m_BadSourceLimitMode;}
	virtual int		GetBadSourceDropTime() const		{return m_BadSourceDropTime;}
	UINT			GetBadSourceDropTimeMs() const		{return SEC2MS(GetBadSourceDropTime());}
	virtual int		GetBadSourceDropMode() const		{return m_BadSourceDropMode;}

	//NNP
	virtual int		UseNNPSourceDrop() const			{return m_NNPSourceDrop;}
	virtual int		GetNNPSourceLimit() const			{return m_NNPSourceLimit;}
	virtual int		GetNNPSourceLimitMode() const		{return m_NNPSourceLimitMode;}
	virtual int		GetNNPSourceDropTime() const		{return m_NNPSourceDropTime;}
	UINT			GetNNPSourceDropTimeMs() const		{return SEC2MS(GetNNPSourceDropTime());}
	virtual int		GetNNPSourceDropMode() const		{return m_NNPSourceDropMode;}

	//FullQ
	virtual int		UseFullQSourceDrop() const			{return m_FullQSourceDrop;}
	virtual int		GetFullQSourceLimit() const			{return m_FullQSourceLimit;}
	virtual int		GetFullQSourceLimitMode() const		{return m_FullQSourceLimitMode;}
	virtual int		GetFullQSourceDropTime() const		{return m_FullQSourceDropTime;}
	UINT			GetFullQSourceDropTimeMs() const	{return SEC2MS(GetFullQSourceDropTime());}
	virtual int		GetFullQSourceDropMode() const		{return m_FullQSourceDropMode;}

	//HighQ
	virtual int		UseHighQSourceDrop() const			{return m_HighQSourceDrop;}
	virtual int		GetHighQSourceLimit() const			{return m_HighQSourceLimit;}
	virtual int		GetHighQSourceLimitMode() const		{return m_HighQSourceLimitMode;}
	virtual int		GetHighQSourceDropTime() const		{return m_HighQSourceDropTime;}
	UINT			GetHighQSourceDropTimeMs() const	{return SEC2MS(GetHighQSourceDropTime());}
	virtual int		GetHighQSourceDropMode() const		{return m_HighQSourceDropMode;}
	virtual int		GetHighQSourceMaxRank() const		{return m_HighQSourceMaxRank;}
	virtual int		GetHighQSourceRankMode() const		{return m_HighQSourceRankMode;}

	virtual int		GetDeadTime() const					{return m_DeadTime;}
	UINT			GetDeadTimeMs() const				{return SEC2MS(GetDeadTime());}
	virtual int		GetDeadTimeFWMulti() const			{return m_DeadTimeFWMulti;}
	UINT			GetDeadTimeMsFW() const				{return SEC2MS(GetDeadTime()*GetDeadTimeFWMulti());}
	virtual int		GetGlobalDeadTime() const			{return m_GlobalDeadTime;}
	UINT			GetGlobalDeadTimeMs() const			{return SEC2MS(GetGlobalDeadTime());}
	virtual int		GetGlobalDeadTimeFWMulti() const	{return m_GlobalDeadTimeFWMulti;}
	UINT			GetGlobalDeadTimeMsFW() const		{return SEC2MS(GetGlobalDeadTime()*GetGlobalDeadTimeFWMulti());}
	// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	virtual int		AutoSaveSources() const					{return m_AutoSaveSources;}
	virtual int		GetAutoSaveSourcesIntervals() const		{return m_AutoSaveSourcesIntervals;}
	UINT			GetAutoSaveSourcesIntervalsMs() const	{return SEC2MS(GetAutoSaveSourcesIntervals());}
	virtual int		GetSourceStorageLimit() const			{return m_SourceStorageLimit;}

	virtual int		StoreAlsoA4AFSources() const			{return m_StoreAlsoA4AFSources;}

	virtual int		AutoLoadSources() const					{return m_AutoLoadSources;}
	virtual int		GetLoadedSourceCleanUpTime() const		{return m_LoadedSourceCleanUpTime;}
	UINT			GetLoadedSourceCleanUpTimeMs() const	{return SEC2MS(GetLoadedSourceCleanUpTime());}
	virtual int		GetSourceStorageReaskLimit() const		{return m_SourceStorageReaskLimit;}

	virtual int		UseTotalSourceRestore() const			{return m_TotalSourceRestore;}

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	virtual int		GetReaskPropability() const				{return m_ReaskPropability;}
	virtual int		UseUnpredictedPropability() const		{return m_UnpredictedPropability;}
	virtual int		GetUnpredictedReaskPropability() const	{return m_UnpredictedReaskPropability;}
 #endif // NEO_SA // NEO: NSA END

	virtual int		GetAutoReaskStoredSourcesDelay() const	{return m_AutoReaskStoredSourcesDelay;}
	UINT			GetAutoReaskStoredSourcesDelayMs() const{return SEC2MS(GetAutoReaskStoredSourcesDelay());}

	virtual int		GroupStoredSourceReask() const			{return m_GroupStoredSourceReask;}
	virtual int		GetStoredSourceGroupIntervals() const	{return m_StoredSourceGroupIntervals;}
	UINT			GetStoredSourceGroupIntervalsMs() const	{return SEC2MS(GetStoredSourceGroupIntervals());}
	virtual int		GetStoredSourceGroupSize() const		{return m_StoredSourceGroupSize;}
#endif // NEO_SS // NEO: NSS END

	// this flag can only be set on per file base its not available on category or global level
	virtual int		ForceA4AF() const						{return 0;} // NEO: NXC - [NewExtendedCategories] 

	// NEO: MCS - [ManualChunkSelection]
	virtual void	SetWantedPart(UINT /*part*/, uint8 /*status*/) { ASSERT(0); }
	virtual	uint8	GetWantedPart(UINT /*part*/) const	{return PR_PART_NORMAL;}
	virtual bool	HasWantedParts() const				{return false;}
	// NEO: MCS END

	virtual void	CheckTweaks();
	virtual void	ResetTweaks() {ASSERT(0);}

protected:
	friend class CPPgNeo;
	friend class CPPgSources;
	friend class CPPgSourceStorage;
	friend class CPPgLancast;
	friend class CSharedFilesCtrl;

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	int		m_LcIntervals;				// Value

	int		m_LanSourceReaskTime;		// Value
	int		m_LanNNPSourceReaskTime;	// Value
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	int		m_VoodooXS;					// Flag
#endif // VOODOO // NEO: VOODOO END

	// NEO: SRT - [SourceRequestTweaks]
	// General
	int		m_SourceLimit;				// Value

	// Management
	int		m_SwapLimit;				// Value

	//XS
	int		m_XsEnable;					// Flag
	int		m_XsLimit;					// Value
	int		m_XsIntervals;				// Value
	int		m_XsClientIntervals;		// Value
	int		m_XsCleintDelay;			// Value
	int		m_XsRareLimit;				// Value

	// SVR
	int		m_SvrEnable;				// Flag
	int		m_SvrLimit;					// Value
	int		m_SvrIntervals;				// Value

	//KAD
	int		m_KadEnable;				// Flag
	int		m_KadLimit;					// Value
	int		m_KadIntervals;				// Value
	int		m_KadMaxFiles;				// Value
	int		m_KadRepeatDelay;			// Value

	//UDP
	int		m_UdpEnable;				// Flag
	int		m_UdpLimit;					// Value
	int		m_UdpIntervals;				// Value
	int		m_UdpGlobalIntervals;		// Value (Global)
	int		m_UdpFilesPerServer;		// Value (Global)
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	int		m_UseSourceCache;			// Flag
	int		m_SourceCacheLimit;			// Value
	int		m_SourceCacheTime;			// Value
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	int		m_AutoSoftLock;				// Flag
	int		m_AutoSoftLockLimit;		// Value
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	int		m_AutoHardLimit;			// Flag
	int		m_AutoHardLimitTime;		// Value
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	int		m_CategorySourceLimit;		// Flag
	int		m_CategorySourceLimitLimit;	// Value (Category)
	int		m_CategorySourceLimitTime;	// Value (Category)
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	int		m_GlobalSourceLimit;		// Flag
	int		m_GlobalSourceLimitLimit;	// Value (Global)
	int		m_GlobalSourceLimitTime;	// Value (Global)
	// NEO: GSL END

	int		m_MinSourcePerFile;			// Value

	int		m_TCPConnectionRetry;		// Value  // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	int		m_SpreadReaskEnable;		// Flag (Global)
	int		m_SpreadReaskTime;			// Value (Global)
	int		m_SourceReaskTime;			// Value
	int		m_FullQSourceReaskTime;		// Value
	int		m_NNPSourceReaskTime;		// Value
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	int		m_DropTime;					// Value

	//Bad
	int		m_BadSourceDrop;			// Flag
	int		m_BadSourceLimit;			// Value
	int		m_BadSourceLimitMode;		// Flag
	int		m_BadSourceDropTime;		// Value
	int		m_BadSourceDropMode;		// Flag

	//NNP
	int		m_NNPSourceDrop;			// Flag
	int		m_NNPSourceLimit;			// Value
	int		m_NNPSourceLimitMode;		// Flag
	int		m_NNPSourceDropTime;		// Value
	int		m_NNPSourceDropMode;		// Flag

	//FullQ
	int		m_FullQSourceDrop;			// Flag
	int		m_FullQSourceLimit;			// Value
	int		m_FullQSourceLimitMode;		// Flag
	int		m_FullQSourceDropTime;		// Value
	int		m_FullQSourceDropMode;		// Flag

	//HighQ
	int		m_HighQSourceDrop;			// Flag
	int		m_HighQSourceLimit;			// Value
	int		m_HighQSourceLimitMode;		// Flag
	int		m_HighQSourceDropTime;		// Value
	int		m_HighQSourceDropMode;		// Flag
	int		m_HighQSourceMaxRank;		// Value
	int		m_HighQSourceRankMode;		// Flag

	int		m_DeadTime;					// Value
	int		m_DeadTimeFWMulti;			// Value
	int		m_GlobalDeadTime;			// Value
	int		m_GlobalDeadTimeFWMulti;	// Value
	// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	int		m_AutoSaveSources;				// Flag
	int		m_AutoSaveSourcesIntervals;		// Value
	int		m_SourceStorageLimit;			// Value

	int		m_StoreAlsoA4AFSources;			// Flag

	int		m_AutoLoadSources;				// Flag
	int		m_LoadedSourceCleanUpTime;		// Value
	int		m_SourceStorageReaskLimit;		// Value

	int		m_TotalSourceRestore;			// Flag

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	int		m_ReaskPropability;				// Value
	int		m_UnpredictedPropability;		// Flag
	int		m_UnpredictedReaskPropability;	// Value
 #endif // NEO_SA // NEO: NSA END

	int		m_AutoReaskStoredSourcesDelay;	// Value

	int		m_GroupStoredSourceReask;		// Flag
	int		m_StoredSourceGroupIntervals;	// Value
	int		m_StoredSourceGroupSize;		// Value
#endif // NEO_SS // NEO: NSS END

	int		m_ForceA4AF;					// Flag // NEO: NXC - [NewExtendedCategories] 

	CMap<UINT, UINT, uint8, uint8> m_WantedParts; // NEO: MCS - [ManualChunkSelection]

	CArray<CTag*, CTag*> taglist;

private:
	void ClearTags();
};



///////////////////////////////////////////////////////////////////////
// CKnownPreferencesEx
//

class CKnownPreferencesEx : public CKnownPreferences
{
public:
	CKnownPreferencesEx(EFilePrefsLevel Level);

	friend class CPreferences;
	friend class CKnownFileList;
	friend class CKnownFile;
	friend class CPartFile;

	virtual	void	SetParent(CKnownFile* File)	{KnownFile = File;}
	virtual	void	SetPrefs(CKnownPreferences* Prefs)	{KnownPrefs = Prefs;}

	virtual bool	IsEmpty() const;

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	virtual int		IsEnableLanCast() const;
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	virtual int		IsEnableVoodoo() const;
#endif // VOODOO // NEO: VOODOO END

	// NEO: IPS - [InteligentPartSharing]
	virtual	int		UseInteligentPartSharing() const;
	virtual	int		GetInteligentPartSharingTimer() const;

	virtual	int		GetMaxProzentToHide() const;

	// OverAvalibly
	virtual	int		IsHideOverAvaliblyParts() const;
	virtual	int		GetHideOverAvaliblyMode() const;
	virtual	int		GetHideOverAvaliblyValue() const;

	virtual	int		IsBlockHighOverAvaliblyParts() const;
	virtual	int		GetBlockHighOverAvaliblyFactor() const;

	// OverShared
	virtual	int		IsHideOverSharedParts() const;
	virtual	int		GetHideOverSharedMode() const;
	virtual	int		GetHideOverSharedValue() const;
	virtual	int		GetHideOverSharedCalc() const;

	virtual	int		IsBlockHighOverSharedParts() const;
	virtual	int		GetBlockHighOverSharedFactor() const;

	// DontHideUnderAvalibly
	virtual	int		IsDontHideUnderAvaliblyParts() const;
	virtual	int		GetDontHideUnderAvaliblyMode() const;
	virtual	int		GetDontHideUnderAvaliblyValue() const;

	// Other
	virtual	int		IsShowAlwaysSomeParts() const;
	virtual	int		GetShowAlwaysSomePartsValue() const;

	virtual	int		IsShowAlwaysIncompleteParts() const;
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	virtual	int		GetReleaseMode() const;
	virtual	int		GetReleaseLevel() const;
	virtual	int		GetReleaseTimer() const;

	// release limit
	virtual	int		IsReleaseLimit() const;
	virtual	int		GetReleaseLimitMode() const;
	virtual	int		GetReleaseLimitHigh() const;
	virtual	int		GetReleaseLimitLow() const;

	virtual	int		IsReleaseLimitLink() const;

	virtual	int		IsReleaseLimitComplete() const;
	virtual	int		GetReleaseLimitCompleteMode() const;
	virtual	int		GetReleaseLimitCompleteHigh() const;
	virtual	int		GetReleaseLimitCompleteLow() const;

	// limit
	virtual	int		IsLimitLink() const;

	// source limit
	virtual	int		IsSourceLimit() const;
	virtual	int		GetSourceLimitMode() const;
	virtual	int		GetSourceLimitHigh() const;
	virtual	int		GetSourceLimitLow() const;

	virtual	int		IsSourceLimitLink() const;

	virtual	int		IsSourceLimitComplete() const;
	virtual	int		GetSourceLimitCompleteMode() const;
	virtual	int		GetSourceLimitCompleteHigh() const;
	virtual	int		GetSourceLimitCompleteLow() const;
	// NEO: SRS END

	virtual bool	IsGlobalPrefs() const	{return false;}
	virtual bool	IsCategoryPrefs() const	{return m_Level == CFP_CATEGORY;}
	virtual bool	IsFilePrefs() const		{return m_Level == CFP_FILE;}

	// NEO: MPS - [ManualPartSharing]
	virtual	void	SetManagedPart(UINT part, uint8 status);
	virtual	uint8	GetManagedPart(UINT part) const;
	virtual	bool	HasManagedParts() const					{ return m_ManagedParts.IsEmpty() == FALSE; }
	// NEO: MPS END

	virtual void	ResetTweaks();

protected:
	CKnownPreferences* KnownPrefs;
	CKnownFile* KnownFile;
	EFilePrefsLevel m_Level;
};


///////////////////////////////////////////////////////////////////////
// CPartPreferencesEx
//

class CPartPreferencesEx : public CPartPreferences
{
public:
	CPartPreferencesEx(EFilePrefsLevel Level);

	friend class CPreferences;
	friend class CDownloadQueue;
	friend class CKnownFile;
	friend class CPartFile;

	virtual	void	SetParent(CPartFile* File)	{PartFile = File;}
	virtual	void	SetPrefs(CPartPreferences* Prefs)	{PartPrefs = Prefs;}

	virtual bool	IsEmpty() const;

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	virtual int		GetLcIntervals() const;

	virtual int		GetLanSourceReaskTime() const;
	virtual int		GetLanNNPSourceReaskTime() const;
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	virtual int		IsVoodooXS() const;
#endif // VOODOO // NEO: VOODOO END

	// NEO: SRT - [SourceRequestTweaks]
	// General
	virtual int		GetSourceLimit() const;

	// Management
	virtual int		GetSwapLimit() const;

	//XS
	virtual int		IsXsEnable() const;
	virtual int		GetXsLimit() const;
	virtual int		GetXsIntervals() const;
	virtual int		GetXsClientIntervals() const;
	virtual int		GetXsCleintDelay() const;
	virtual int		GetXsRareLimit() const;

	// SVR
	virtual int		IsSvrEnable() const;
	virtual int		GetSvrLimit() const;
	virtual int		GetSvrIntervals() const;

	//KAD
	virtual int		IsKadEnable() const;
	virtual int		GetKadLimit() const;
	virtual int		GetKadIntervals() const;
	virtual int		GetKadMaxFiles() const;
	virtual int		GetKadRepeatDelay() const;

	//UDP
	virtual int		IsUdpEnable() const;
	virtual int		GetUdpLimit() const;
	virtual int		GetUdpIntervals() const;
	virtual int		GetUdpGlobalIntervals() const		{ASSERT(0); return 0;}
	virtual int		GetUdpFilesPerServer() const		{ASSERT(0); return 0;}
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	virtual int		UseSourceCache() const;
	virtual int		GetSourceCacheLimit() const;
	virtual int		GetSourceCacheTime() const;
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	virtual int		UseAutoSoftLock() const;
	virtual int		GetAutoSoftLockLimit() const;
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	virtual int		UseAutoHardLimit() const;
	virtual int		GetAutoHardLimitTime() const;
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	virtual int		UseCategorySourceLimit() const;
	virtual int		GetCategorySourceLimitLimit() const;
	virtual int		GetCategorySourceLimitTime() const;
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	virtual int		UseGlobalSourceLimit() const;
	// NEO: GSL END

	virtual int		GetMinSourcePerFile() const;

	virtual int		GetTCPConnectionRetry() const; // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	virtual int		UseSpreadReaskEnable() const		{ASSERT(0); return 0;}
	virtual int		GetSpreadReaskTime() const		{ASSERT(0); return 0;}
	virtual int		GetSourceReaskTime() const;
	virtual int		GetFullQSourceReaskTime() const;
	virtual int		GetNNPSourceReaskTime() const;
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	virtual int		GetDropTime() const;

	//Bad
	virtual int		UseBadSourceDrop() const;
	virtual int		GetBadSourceLimit() const;
	virtual int		GetBadSourceLimitMode() const;
	virtual int		GetBadSourceDropTime() const;
	virtual int		GetBadSourceDropMode() const;

	//NNP
	virtual int		UseNNPSourceDrop() const;
	virtual int		GetNNPSourceLimit() const;
	virtual int		GetNNPSourceLimitMode() const;
	virtual int		GetNNPSourceDropTime() const;
	virtual int		GetNNPSourceDropMode() const;

	//FullQ
	virtual int		UseFullQSourceDrop() const;
	virtual int		GetFullQSourceLimit() const;
	virtual int		GetFullQSourceLimitMode() const;
	virtual int		GetFullQSourceDropTime() const;
	virtual int		GetFullQSourceDropMode() const;

	//HighQ
	virtual int		UseHighQSourceDrop() const;
	virtual int		GetHighQSourceLimit() const;
	virtual int		GetHighQSourceLimitMode() const;
	virtual int		GetHighQSourceDropTime() const;
	virtual int		GetHighQSourceDropMode() const;
	virtual int		GetHighQSourceMaxRank() const;
	virtual int		GetHighQSourceRankMode() const;

	virtual int		GetDeadTime() const;
	virtual int		GetDeadTimeFWMulti() const;
	virtual int		GetGlobalDeadTime() const			{ASSERT(0); return 0;}
	virtual int		GetGlobalDeadTimeFWMulti() const	{ASSERT(0); return 0;}
	// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	virtual int		AutoSaveSources() const;
	virtual int		GetAutoSaveSourcesIntervals() const;
	virtual int		GetSourceStorageLimit() const;

	virtual int		StoreAlsoA4AFSources() const;

	virtual int		AutoLoadSources() const;
	virtual int		GetLoadedSourceCleanUpTime() const;
	virtual int		GetSourceStorageReaskLimit() const;

	virtual int		UseTotalSourceRestore() const;

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	virtual int		GetReaskPropability() const;
	virtual int		UseUnpredictedPropability() const;
	virtual int		GetUnpredictedReaskPropability() const;
 #endif // NEO_SA // NEO: NSA END

	virtual int		GetAutoReaskStoredSourcesDelay() const;

	virtual int		GroupStoredSourceReask() const;
	virtual int		GetStoredSourceGroupIntervals() const;
	virtual int		GetStoredSourceGroupSize() const;
#endif // NEO_SS // NEO: NSS END

	// this flag can only be set on per file base its not available on category or global level
	virtual int		ForceA4AF() const						{ if(!IsFilePrefs()) return 0; return m_ForceA4AF;} // NEO: NXC - [NewExtendedCategories] 

	// NEO: MCS - [ManualChunkSelection]
	virtual	void	SetWantedPart(UINT part, uint8 status);
	virtual	uint8	GetWantedPart(UINT part) const;
	virtual	bool	HasWantedParts() const					{ return m_WantedParts.IsEmpty() == FALSE; }
	// NEO: MCS END

	virtual bool	IsGlobalPrefs() const	{return false;}
	virtual bool	IsCategoryPrefs() const	{return m_Level == CFP_CATEGORY;}
	virtual bool	IsFilePrefs() const		{return m_Level == CFP_FILE;}

	virtual void	ResetTweaks();

protected:
	CPartPreferences* PartPrefs;
	CPartFile* PartFile;
	EFilePrefsLevel m_Level;
};

///////////////////////////////////////////////////////////////////////////////////////
// Helpers
//

// Helper Functions
int GetRightVal (int &mode, int def, int max, int val1, int val2, int val3 = 0);
CString EncodeFPValue(int Value);
int DecodeFPValue(CString Text, int Default);
void CheckFPValue(int val, int def, bool glb, bool cat);
bool CheckModes(int &val, int &mod);

// tag implements
#define IMPLEMENT_FP_B_SET(val,opcode) \
	if (val != FCFG_DEF){ \
		CTag tag(opcode, (uint32)val); \
		tag.WriteNewEd2kTag(file); \
		uTagCount++; \
	}

#define IMPLEMENT_FP_B_GET(val,opcode) \
			case opcode:{ \
                ASSERT( newtag->IsInt() ); \
                val = newtag->GetInt(); \
                delete newtag; \
                break; \
            }

// Ini implements
#define IMPLEMENT_FP_I_SET(val,entry) \
	if(val == FCFG_DEF) \
		ini.DeleteKey(entry); \
	else \
		ini.WriteString(entry, EncodeFPValue(val)); 

#define IMPLEMENT_FP_I_GET(val,entry,def) \
	val = DecodeFPValue(ini.GetString(entry, _T("")), IsGlobalPrefs() ? def : FCFG_DEF); 
#define IMPLEMENT_FP_I_GET2(val,entry,def,mod,def_mod) \
	val = DecodeFPValue(ini.GetString(entry, _T("")), IsGlobalPrefs() ? GetRightVal(mod, def_mod, 2, DEF_##def##_1, DEF_##def##_2) : FCFG_DEF); 
#define IMPLEMENT_FP_I_GET3(val,entry,def,mod,def_mod) \
	val = DecodeFPValue(ini.GetString(entry, _T("")), IsGlobalPrefs() ? GetRightVal(mod, def_mod, 3, DEF_##def##_1, DEF_##def##_2, DEF_##def##_3) : FCFG_DEF); 

// check implements
#define IMPLEMENT_FP_CHK_VAL(val,def) \
	if(val > FCFG_BASE) \
		CheckFPValue(val,DEF_##def,IsGlobalPrefs(),IsCategoryPrefs()); \
	else \
		MinMax(&val,MIN_##def,MAX_##def);

#define IMPLEMENT_FP_CHK_VAL2(val,def,mod) \
	if(CheckModes(val,mod)) \
	{ \
		if(val > FCFG_BASE) \
			CheckFPValue(val,GetRightVal(mod, 0, 2, DEF_##def##_1, DEF_##def##_2),IsGlobalPrefs(),IsCategoryPrefs()); \
		else \
			MinMax(&val,GetRightVal(mod, 0, 2, MIN_##def##_1, MIN_##def##_2),GetRightVal(mod, 0, 3, MAX_##def##_1, MAX_##def##_2)); \
	}

#define IMPLEMENT_FP_CHK_VAL3(val,def,mod) \
	if(CheckModes(val,mod)) \
	{ \
		if(val > FCFG_BASE) \
			CheckFPValue(val,GetRightVal(mod, 0, 3, DEF_##def##_1, DEF_##def##_2, DEF_##def##_3),IsGlobalPrefs(),IsCategoryPrefs()); \
		else \
			MinMax(&val,GetRightVal(mod, 0, 3, MIN_##def##_1, MIN_##def##_2, MIN_##def##_3),GetRightVal(mod, 0, 3, MAX_##def##_1, MAX_##def##_2, MIN_##def##_3)); \
	}

#define IMPLEMENT_FP_CHK_FLAG(val,def,mod) \
	if(val > mod && IsGlobalPrefs()) \
		val = def; \
	if(val > mod+2 && IsFilePrefs()) \
		val = FCFG_DEF; \
	if(val > mod+1 && IsCategoryPrefs()) \
		val = FCFG_DEF;


// Function implements
#define IMPLEMENT_CFPEX(cls, var, fx, pref) \
int cls::fx() const { \
	if(var == FCFG_GLB) \
		return NeoPrefs.pref.fx(); \
	if(var == FCFG_DEF) \
		return pref->fx(); \
	return var; \
}

#define ASSERT_VAL(val) ASSERT(val < FCFG_BASE || val == FCFG_AUT); // no other values are valid

// NEO: FCFG END <-- Xanatos --