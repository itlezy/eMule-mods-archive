#pragma once

// Maella
// New rules after a restart:
// - If elapsed time < 1 hour, reask all sources
// - If elapsed time > 1 hour, reask 10 sources
// - If elapsed time > 2 days, reask none


class CPartFile;
class CUpDownClient;

class CSourceSaver
{
public:
	CSourceSaver(CPartFile* file);
	~CSourceSaver(void);
	bool Process();
	void DeleteFile();

	// X: [ISS] - [Improved Source Save]
	void LoadSourcesFromFile();
	void SaveSources();
	void AddSourcesToDownload();

#ifdef _DEBUG
	size_t GetSavedSources()	{return m_sourceList.GetCount();}
#endif

protected:
	class CSourceData
	{
	public:
		CSourceData() {} // Necessary for class CAtlList

		CSourceData(uint32 dwID, uint16 wPort, const CString& expiration90mins, const CString& expiration3days) : sourceID(dwID), 
																	                                       sourcePort(wPort),
																		                                   partsavailable(0),
                                                                                                           expiration90mins(expiration90mins),
																		                                   expiration3days(expiration3days) {}

		// X: [SUS] - [Save URL Source]
		CSourceData(const CStringA& url, const CString& expiration90mins, const CString& expiration3days) : URL(url), 
																											partsavailable(0),
																											expiration90mins(expiration90mins),
																											expiration3days(expiration3days) {}

		CSourceData(const CSourceData& ref) : sourceID(ref.sourceID), 
											  sourcePort(ref.sourcePort),
											  partsavailable(ref.partsavailable),
											  URL(ref.URL),
											  expiration90mins(ref.expiration90mins),
                                              expiration3days(ref.expiration3days){}

		CSourceData(CUpDownClient* client, const CString& expiration90mins, const CString& expiration3days);

		bool Compare(const CSourceData& tocompare) const { // X: [SUS] - [Save URL Source]
			if(URL.IsEmpty())
				return (sourceID == tocompare.sourceID) && (sourcePort == tocompare.sourcePort);
			return !URL.CompareNoCase(tocompare.URL);
		}

		uint32	sourceID;
		uint16	sourcePort;
		uint16	partsavailable;
		CString URL; // X: [SUS] - [Save URL Source]
		CString expiration90mins; // 1.5 hours
        CString expiration3days; // 3 days
	};
	typedef CAtlList<CSourceData> SourceList;
	
	//uint32     m_dwLastTimeLoaded; // X: [ISS] - [Improved Source Save]
	uint32     m_dwLastTimeSaved;	
	CPartFile* m_pFile;
	SourceList m_sourceList;
	
	CString CSourceSaver::CalcExpiration(); // 1.5 hours (8 chars)
	CString CSourceSaver::CalcExpirationLong(); // 3 days (6 chars)
	bool IsExpired(const CString& expiration90mins) const;
};
