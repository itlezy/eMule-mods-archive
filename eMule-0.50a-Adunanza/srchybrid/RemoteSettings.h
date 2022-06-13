#include "Ini2.h"
#include "Types.h"
#include "Preferences.h"
#pragma once

class CRemoteSettings {
public:
	CRemoteSettings();
	~CRemoteSettings();

	void ReadSettings(CIni *docfg);
	void SaveSettings();

	void PrintAllValues();

	void SaveToFile();
	void CheckUpdate();
	
	static int kadRepublishTimeK;
	static int kadRepublishTimeS;
	static int kadRepublishTimeN;

	static int kadIndexLifeK;
	static int kadIndexLifeS;

	static int kadTotalStoreKey;
	static int kadTotalStoreSrc;
	static int kadTotalStoreNotes;

	static int kadTotalSearchFile;

	static int kadMaxSrcFile;
	static int kadMaxNotFile;

	static float kadFreshGuess_Tol;
	static float kadFreshGuess_Weight;
	static int  kadFreshGuess_NoNorm;
	static int  kadFreshGuess_LowNorm;

	static int maxSrc;
	static int maxSrcUdp;

	static int kadFindValue;
	static int kadStore;
	static int kadFindNode;

	static int kadReaskTime;
	static int kadPubTime;

	static int kadReaskIncs;

	static int mVer;
	static int TolleranzaTest;
	static CStringList bMods;
	int AduSplashTime;   // the timeout value used for advertisment in splash window.


	static unsigned char kadOpcode;
	static unsigned char kadZOpcode;
	static bool enableAduStats;
	static CString UpdateURL;
	static uint32 m_AduValRipBanda_Std;
	static CString SpeedTestUrl;
	static CString UpdateMessage;
	static CString HomeAduBrowser;
	static CString splashUrl;
private:
	static int curVer;
};
extern CRemoteSettings rm;

