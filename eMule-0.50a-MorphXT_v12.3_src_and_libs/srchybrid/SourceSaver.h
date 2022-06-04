#pragma once

class CPartFile;
class CUpDownClient;

class CSourceSaver
{
public:
	CSourceSaver(void);
	~CSourceSaver(void);
	//MORPH - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
	//bool Process(CPartFile* file, int maxSourcesToSave=2);
	bool Process(CPartFile* file, UINT maxSourcesToSave=10);
	void DeleteFile(CPartFile* file);

protected:
	// khaos::kmod+ Source Exchange Version
	class CSourceData
	{
	public:
		//MORPH - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
		//CSourceData(uint32 dwID, uint16 wPort, const char* exp, uint8 srcexver) {	sourceID = dwID; 
		//																			sourcePort = wPort; 
		//																			memcpy(expiration, exp, 7);
		//																			expiration[6] = 0;
		//																			nSrcExchangeVer = srcexver;}
		CSourceData(uint32 dwID, uint16 wPort,uint32 dwserverip, uint16 wserverport, const TCHAR* exp, uint8 srcexver) {	sourceID = dwID; 
																					sourcePort = wPort; 
																					serverip = dwserverip;
																					serverport = wserverport;
																					memcpy(expiration, exp, 11*sizeof(TCHAR));
																					expiration[10] = 0;
																					nSrcExchangeVer = srcexver;}

		CSourceData(CUpDownClient* client, const TCHAR* exp);
		//MORPH - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
		//CSourceData(CSourceData* pOld) {							sourceID = pOld->sourceID; 
		//															sourcePort = pOld->sourcePort; 
		//															memcpy(expiration, pOld->expiration, 7); 
		//															partsavailable = pOld->partsavailable;
		//															expiration[6] = 0;
		//															nSrcExchangeVer = pOld->nSrcExchangeVer;}
		CSourceData(CSourceData* pOld) {							sourceID = pOld->sourceID; 
																	sourcePort = pOld->sourcePort; 
																	serverip = pOld->serverip;
																	serverport = pOld->serverport;
																	memcpy(expiration, pOld->expiration, 11*sizeof(TCHAR)); 
																	partsavailable = pOld->partsavailable;
																	expiration[10] = 0;
																	nSrcExchangeVer = pOld->nSrcExchangeVer;}

		bool Compare(CSourceData* tocompare) {						return ((sourceID == tocompare->sourceID) 
																	 && (sourcePort == tocompare->sourcePort)); }

		uint32	sourceID;
		uint16	sourcePort;
		uint32	serverip;
		uint16	serverport;
		uint32	partsavailable;
		//MORPH - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
		//char	expiration[7];
		TCHAR	expiration[11];
		uint8	nSrcExchangeVer;
	};
	// khaos::kmod-
	typedef CTypedPtrList<CPtrList, CSourceData*> SourceList;

	void LoadSourcesFromFile(CPartFile* file, SourceList* sources, LPCTSTR slsfile);
	void SaveSources(CPartFile* file, SourceList* prevsources, LPCTSTR slsfile, UINT maxSourcesToSave);
	void AddSourcesToDownload(CPartFile* file, SourceList* sources);
	
	uint32	m_dwLastTimeLoaded;
	uint32  m_dwLastTimeSaved;
	
	//MORPH - Changed by SiRoB, SLS keep only for rar files, reduce Saved Source and life time
	//CString CalcExpiration(int nDays);
	CString CalcExpiration(int nMinutes);
	bool IsExpired(CString expirationdate);
};
