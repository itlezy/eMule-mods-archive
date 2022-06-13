#include "stdafx.h"
#include "emule.h"
#include "RemoteSettings.h"
#include "Log.h"
#include "OPCodes.h"
#include "HTTPDownloadDlg.h"   // needed for CHTTPDownloader
#include "AdunanzA.h"
#include "kademlia/kademlia/kademlia.h" //Anis -> Aggiornamento nodi automatico
#include "kademlia/routing/RoutingZone.h"

//Anis -> Revisione 17/11/2011 (OK)
//-> Questo modulo è stato risistemato per garantire più efficenza e affidabilità. Riscritto per un buon 20%.

CString SplashDir = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("splash.jpg");
CString RemoteDir = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("adunanza.conf.remote");
CString ConfDir = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("adunanza.conf");

uint32 CRemoteSettings::m_AduValRipBanda_Std;
int CRemoteSettings::kadRepublishTimeK;
int CRemoteSettings::kadRepublishTimeS;
int CRemoteSettings::kadRepublishTimeN;
int CRemoteSettings::kadIndexLifeK;
int CRemoteSettings::kadIndexLifeS;
int CRemoteSettings::kadTotalStoreKey;
int CRemoteSettings::kadTotalStoreSrc;
int CRemoteSettings::kadTotalStoreNotes;
int CRemoteSettings::kadTotalSearchFile;
int CRemoteSettings::kadMaxSrcFile;
int CRemoteSettings::kadMaxNotFile;
int CRemoteSettings::curVer;
float CRemoteSettings::kadFreshGuess_Tol;
float CRemoteSettings::kadFreshGuess_Weight;
int CRemoteSettings::kadFreshGuess_NoNorm;
int CRemoteSettings::kadFreshGuess_LowNorm;
int CRemoteSettings::maxSrc;
int CRemoteSettings::maxSrcUdp;
int CRemoteSettings::kadFindValue;
int CRemoteSettings::kadStore;
int CRemoteSettings::kadFindNode;
int CRemoteSettings::kadReaskTime;
int CRemoteSettings::kadPubTime;
int CRemoteSettings::kadReaskIncs;
int CRemoteSettings::mVer;
int CRemoteSettings::TolleranzaTest;
CStringList CRemoteSettings::bMods;
unsigned char CRemoteSettings::kadOpcode;
unsigned char CRemoteSettings::kadZOpcode;
bool CRemoteSettings::enableAduStats;
CString CRemoteSettings::UpdateURL;
CString CRemoteSettings::SpeedTestUrl;
CString CRemoteSettings::UpdateMessage;
CString CRemoteSettings::HomeAduBrowser;
CString CRemoteSettings::splashUrl;
extern bool WizardNotOpen;

CRemoteSettings::CRemoteSettings() 
{
	kadRepublishTimeK = 18000;
	kadRepublishTimeS = 18000;
	kadRepublishTimeN = 86400;
	kadIndexLifeK = 345600;
	kadIndexLifeS = 345600;
	kadTotalStoreKey = 8;
	kadTotalStoreSrc = 12;
	kadTotalStoreNotes = 1;
	kadTotalSearchFile = 15;
	kadMaxSrcFile = 300;
	kadMaxNotFile = 50;
	kadFreshGuess_Tol = 3999;
	kadFreshGuess_Weight = 7999;
	kadFreshGuess_LowNorm = 16;
	kadFreshGuess_NoNorm = 8;
	maxSrc = 500;
	maxSrcUdp = 400;
	kadFindValue = 8;
	kadStore = 6;
	kadFindNode = 56;
	kadReaskTime = 600000;
	kadPubTime = 2;
	AduSplashTime = ADUSPLASHTIME;
	kadReaskIncs = 3;
	mVer = 0;
	bMods.RemoveAll();
	enableAduStats = true;
	UpdateURL = ADURM_URL;
	kadOpcode = 164;
	kadZOpcode = 165;
	m_AduValRipBanda_Std = 6;
	SpeedTestUrl = ADU_SPEED_TEST_URL;
	UpdateMessage = ADU_MESSAGE_UPDATE;
	HomeAduBrowser = ADU_HOME_PAGE_BROWSER;
	TolleranzaTest = 40;
	CIni cfg(ConfDir, _T("Adunanza"));
	m_AduValRipBanda_Std = cfg.GetInt(_T("AduValRipBandaStd"));
	SpeedTestUrl = cfg.GetString(_T("SpeedTestUrl"));
	HomeAduBrowser = cfg.GetString(_T("HomeAduBrowser"));
	UpdateMessage = cfg.GetString(_T("UpdateMessage")); 
	PrintAllValues();
	ReadSettings(&cfg);
}

CRemoteSettings::~CRemoteSettings() 
{
	SaveSettings();
}

void CRemoteSettings::PrintAllValues() 
{
#if ADU_BETA_MAJ > 0 && defined BETA
	AddDebugLogLine(DLP_LOW, false, _T("kadRepublishTimeK: %d"),kadRepublishTimeK);
	AddDebugLogLine(DLP_LOW, false, _T("kadRepublishTimeS: %d"),kadRepublishTimeS);
	AddDebugLogLine(DLP_LOW, false, _T("kadRepublishTimeN: %d"),kadRepublishTimeN);
	AddDebugLogLine(DLP_LOW, false, _T("kadIndexLifeK: %d"),kadIndexLifeK);
	AddDebugLogLine(DLP_LOW, false, _T("kadIndexLifeS: %d"),kadIndexLifeS);
	AddDebugLogLine(DLP_LOW, false, _T("kadTotalStoreKey: %d"),kadTotalStoreKey);
	AddDebugLogLine(DLP_LOW, false, _T("kadTotalStoreSrc: %d"),kadTotalStoreSrc);
	AddDebugLogLine(DLP_LOW, false, _T("kadTotalStoreNotes: %d"),kadTotalStoreNotes);
	AddDebugLogLine(DLP_LOW, false, _T("kadTotalSearchFile: %d"),kadTotalSearchFile);
	AddDebugLogLine(DLP_LOW, false, _T("kadMaxSrcFile: %d"),kadMaxSrcFile);
	AddDebugLogLine(DLP_LOW, false, _T("kadMaxNotFile: %d"),kadMaxNotFile);
	AddDebugLogLine(DLP_LOW, false, _T("kadFreshGuess_Tol: %.2f"),kadFreshGuess_Tol);
	AddDebugLogLine(DLP_LOW, false, _T("kadFreshGuess_Weight: %.2f"),kadFreshGuess_Weight);
	AddDebugLogLine(DLP_LOW, false, _T("kadFreshGuess_LowNorm: %d"),kadFreshGuess_LowNorm);
	AddDebugLogLine(DLP_LOW, false, _T("kadFreshGuess_NoNorm: %d"),kadFreshGuess_NoNorm);
	AddDebugLogLine(DLP_LOW, false, _T("maxSrc: %d"),maxSrc);
	AddDebugLogLine(DLP_LOW, false, _T("maxSrcUdp: %d"),maxSrcUdp);
	AddDebugLogLine(DLP_LOW, false, _T("kadFindValue: %d"),kadFindValue);
	AddDebugLogLine(DLP_LOW, false, _T("kadStore: %d"),kadStore);
	AddDebugLogLine(DLP_LOW, false, _T("kadFindNode: %d"),kadFindNode);
	AddDebugLogLine(DLP_LOW, false, _T("kadReaskTime: %d"),kadReaskTime);
	AddDebugLogLine(DLP_LOW, false, _T("kadReaskIncs: %d"),kadReaskIncs);
	AddDebugLogLine(DLP_LOW, false, _T("kadPubTime: %d"),kadPubTime);
	AddDebugLogLine(DLP_LOW, false, _T("AduSplashTime: %d"),AduSplashTime);
    AddDebugLogLine(DLP_LOW, false, _T("mVer: %d"),mVer);
	AddDebugLogLine(DLP_LOW, false, enableAduStats?_T("Statistiche attivate"):_T("Statistiche disattivate"));
	AddDebugLogLine(DLP_LOW, false, _T("opcodes Norm: 0x%02x Zip: 0x%02x"),kadOpcode, kadZOpcode);
	CString toWrite = CString("");
	for (POSITION pos = bMods.GetHeadPosition(); pos != NULL;)
		toWrite += bMods.GetNext(pos) + CString(";");
	AddDebugLogLine(DLP_LOW, false, _T("modstringban: %s"),toWrite);
	AddDebugLogLine(DLP_LOW, false, _T("current value for standard external upload: %u "),m_AduValRipBanda_Std);
	//Anis
	AddDebugLogLine(DLP_LOW, false, _T("TolleranzaTest %d"),TolleranzaTest);
	AddDebugLogLine(DLP_LOW, false, _T("Updating from %s"),UpdateURL);
	AddDebugLogLine(DLP_LOW, false, _T("SpeedTesting URL: %s"),SpeedTestUrl);
	AddDebugLogLine(DLP_LOW, false, _T("UpdateMessage: %s"),UpdateMessage);
	AddDebugLogLine(DLP_LOW, false, _T("current splash version: %i"),curVer);
#endif
}

void CRemoteSettings::SaveToFile() 
{
	SaveSettings();
}

void CRemoteSettings::ReadSettings(CIni *docfg) 
{
	kadRepublishTimeK = docfg->GetInt(_T("kadRepublishTimeK"), kadRepublishTimeK);
	kadRepublishTimeS = docfg->GetInt(_T("kadRepublishTimeS"), kadRepublishTimeS);
	kadRepublishTimeN = docfg->GetInt(_T("kadRepublishTimeN"), kadRepublishTimeN);
	kadIndexLifeK = docfg->GetInt(_T("kadIndexLifeK"), kadIndexLifeK);
	kadIndexLifeS = docfg->GetInt(_T("kadIndexLifeS"), kadIndexLifeS);
	kadTotalStoreKey = docfg->GetInt(_T("kadTotalStoreKey"), kadTotalStoreKey);
	kadTotalStoreSrc = docfg->GetInt(_T("kadTotalStoreSrc"), kadTotalStoreSrc);
	kadTotalStoreNotes = docfg->GetInt(_T("kadTotalStoreNotes"), kadTotalStoreNotes);
	kadTotalSearchFile = docfg->GetInt(_T("kadTotalSearchFile"), kadTotalSearchFile);
	kadMaxSrcFile = docfg->GetInt(_T("kadMaxSrcFile"), kadMaxSrcFile);
	kadMaxNotFile = docfg->GetInt(_T("kadMaxNotFile"), kadMaxNotFile);
	kadFreshGuess_Tol = docfg->GetInt(_T("kadFreshGuess_Tol"), kadFreshGuess_Tol)/10000.f;
	kadFreshGuess_Weight = docfg->GetInt(_T("kadFreshGuess_Weight"), kadFreshGuess_Weight)/10000.f;
	kadFreshGuess_LowNorm = docfg->GetInt(_T("kadFreshGuess_LowNorm"), kadFreshGuess_LowNorm);
	kadFreshGuess_NoNorm = docfg->GetInt(_T("kadFreshGuess_NoNorm"), kadFreshGuess_NoNorm);
	TolleranzaTest = docfg->GetInt(_T("TolleranzaTest"), TolleranzaTest);
	maxSrc = docfg->GetInt(_T("maxSrc"), maxSrc);
	maxSrcUdp = docfg->GetInt(_T("maxSrcUdp"), maxSrcUdp);
	kadFindValue = docfg->GetInt(_T("kadFindValue"), kadFindValue);
	kadStore = docfg->GetInt(_T("kadStore"), kadStore);
	kadFindNode = docfg->GetInt(_T("kadFindNode"), kadFindNode);
	kadReaskTime = docfg->GetInt(_T("kadReaskTime"), kadReaskTime);
	kadReaskIncs = docfg->GetInt(_T("kadReaskIncs"), kadReaskIncs);
	kadPubTime = docfg->GetInt(_T("kadPubTime"), kadPubTime);
	AduSplashTime = docfg->GetInt(_T("AduSplashTime"), AduSplashTime);
	mVer = docfg->GetInt(_T("mVer"), mVer);
	enableAduStats = docfg->GetBool(_T("AduStats"),false);
	kadOpcode = (unsigned char)docfg->GetInt(_T("kadOpcode"), (int)kadOpcode);
	kadZOpcode = (unsigned char)docfg->GetInt(_T("kadZOpcode"), (int)kadZOpcode);
	CString mods;
	mods = docfg->GetString(_T("modstringban"), _T(""));
	CString strTok;
	int nPos = 0;
	bMods.RemoveAll();
	strTok = mods.Tokenize(_T(";"),nPos);
	while (!strTok.IsEmpty()) 
	{
		bMods.AddTail(strTok);
		strTok = mods.Tokenize(_T(";"),nPos);
	}	
	
	curVer = docfg->GetInt(_T("splashVer"),0);
	splashUrl = docfg->GetString(_T("splashURL"),_T(""));
	//tigerjact ripartirore di banda
	m_AduValRipBanda_Std = docfg->GetInt(_T("AduValRipBandaStd"), 6);
	UpdateURL = docfg->GetString(_T("UpdateURL"), L"ADURM_URL");
	//Anis coding
	SpeedTestUrl = docfg->GetString(_T("SpeedTestUrl"), ADU_SPEED_TEST_URL);
	UpdateMessage = docfg->GetString(_T("UpdateMessage"), ADU_MESSAGE_UPDATE);
	HomeAduBrowser = docfg->GetString(_T("HomeAduBrowser"), ADU_HOME_PAGE_BROWSER);

	if (thePrefs.m_AduRipBanda)
	{
		thePrefs.m_AduValRipBanda = m_AduValRipBanda_Std;
		bool localbool = thePrefs.Save();
		//tigerjact  imposto i ratio standard
		CalcolaRatio(false);
	}
}

void CRemoteSettings::SaveSettings() 
{ //tigerjact, Anis
	CIni cfg(ConfDir, _T("Adunanza") );
	cfg.WriteInt(_T("kadRepublishTimeK"), kadRepublishTimeK);
	cfg.WriteInt(_T("kadRepublishTimeS"), kadRepublishTimeS);
	cfg.WriteInt(_T("kadRepublishTimeN"), kadRepublishTimeN);
	cfg.WriteInt(_T("kadIndexLifeK"), kadIndexLifeK);
	cfg.WriteInt(_T("kadIndexLifeS"), kadIndexLifeS);
	cfg.WriteInt(_T("kadTotalStoreKey"), kadTotalStoreKey);
	cfg.WriteInt(_T("kadTotalStoreSrc"), kadTotalStoreSrc);
	cfg.WriteInt(_T("kadTotalStoreNotes"), kadTotalStoreNotes);
	cfg.WriteInt(_T("kadTotalSearchFile"), kadTotalSearchFile);
	cfg.WriteInt(_T("kadMaxSrcFile"), kadMaxSrcFile);
	cfg.WriteInt(_T("kadMaxNotFile"), kadMaxNotFile);
	cfg.WriteInt(_T("kadFreshGuess_Tol"), (int)(kadFreshGuess_Tol * 10000));
	cfg.WriteInt(_T("kadFreshGuess_Weight"), (int)(kadFreshGuess_Weight * 10000));
	cfg.WriteInt(_T("kadFreshGuess_LowNorm"), kadFreshGuess_LowNorm);
	cfg.WriteInt(_T("kadFreshGuess_NoNorm"), kadFreshGuess_NoNorm);
	cfg.WriteInt(_T("maxSrc"), maxSrc);
	cfg.WriteInt(_T("maxSrcUdp"), maxSrcUdp);
	cfg.WriteInt(_T("kadFindValue"), kadFindValue);
	cfg.WriteInt(_T("kadStore"), kadStore);
	cfg.WriteInt(_T("kadFindNode"), kadFindNode);
	cfg.WriteInt(_T("kadReaskTime"), kadReaskTime);
	cfg.WriteInt(_T("kadReaskIncs"), kadReaskIncs);
	cfg.WriteInt(_T("kadPubTime"), kadPubTime);
	cfg.WriteInt(_T("AduSplashTime"), AduSplashTime);
	cfg.WriteInt(_T("mVer"), mVer);
	cfg.WriteInt(_T("TolleranzaTest"), TolleranzaTest);
	cfg.WriteBool(_T("AduStats"),enableAduStats);
	cfg.WriteInt(_T("kadOpcode"), (int)kadOpcode);
	cfg.WriteInt(_T("kadZOpcode"), (int)kadZOpcode);

	CString toWrite = CString("");
	for (POSITION pos = bMods.GetHeadPosition(); pos != NULL; )
		toWrite += bMods.GetNext(pos)+CString(";");
	
	//Anis -> Rifatto
	cfg.WriteInt(_T("splashVer"), curVer);
	cfg.WriteString(_T("modstringban"), toWrite);
	cfg.WriteInt(_T("AduValRipBandaStd"), m_AduValRipBanda_Std);
	cfg.WriteString(_T("SpeedTestUrl"), SpeedTestUrl);
	cfg.WriteString(_T("UpdateMessage"), UpdateMessage);
	cfg.WriteString(_T("HomeAduBrowser"), HomeAduBrowser);
	cfg.WriteString(_T("splashURL"), splashUrl);
	cfg.WriteString(_T("UpdateURL"), UpdateURL);
}

void CRemoteSettings::CheckUpdate() 
{ //Funzione di Anis
	try
	{
		extern bool FileExist(LPCTSTR filename);
		extern uint32 oldVer;

		CHttpDownloadDlg dlgDownload;
		dlgDownload.m_sURLToDownload = UpdateURL;
		dlgDownload.m_sFileToDownloadInto = RemoteDir;
		dlgDownload.DoModal();
		CIni cfg(RemoteDir, _T("Adunanza"));
		ReadSettings(&cfg);
		DeleteFile(RemoteDir);
		if ((curVer > oldVer) || (!FileExist(SplashDir))) {
			if (!splashUrl.IsEmpty()) {
				CHttpDownloadDlg splDownload;
				splDownload.m_sURLToDownload = splashUrl;
				splDownload.m_sFileToDownloadInto = SplashDir;
				splDownload.DoModal();
			}
		}

		SaveToFile();
		PrintAllValues();
	}
	catch(...) {}
}
