#pragma once
//Anis -> Riscritto per un buon 75%
#define AduV(M, m) _T(#M) _T(".") _T(#m)
#define _AduV(M, m) AduV(M,m)
#define AduB(n, l) _T(#n) _T(l)
#define _AduB(n, l) AduB(n, l)
#define AduR(M, m) #M "-" #m
#define _AduR(M, m) AduR(M,m)
#define AduRU(M, m) _T(#M) _T("-") _T(#m)
#define _AduRU(M, m) AduRU(M,m)
#define ADU_VER_MAJ   3
#define ADU_VER_MIN   18
#define ADU_VER_RL _AduV(ADU_VER_MAJ, ADU_VER_MIN)
#define ADU_BETA_MAJ 0
#define ADU_BETA_MIN ""

#undef BETA //Anis -> definire questo se si desidera attivare tutti gli avvisi nella verbose.
#if ADU_BETA_MAJ > 0
	#define ADU_VER ADU_VER_RL _T("b") _AduB(ADU_BETA_MAJ, ADU_BETA_MIN)
#else
	#define ADU_VER ADU_VER_RL _T("")
#endif

//Anis's define da implementare nella prossima versione!
#define ADU_SPEED_TEST_URL _T("http://debian.fastweb.it/debian-cd/3.1_r2/i386/jigdo-dvd/debian-31r2-i386-binary-1.template")
#define ADU_MESSAGE_UPDATE _T("È disponibile una nuova versione di eMule AdunanzA. Aggiornare ora?")
#define ADU_HOME_PAGE_BROWSER _T("http://www.adunanza.net/")

#define ADU_MOD_NAME _T("AdunanzA")
#define ADU_MOD ADU_MOD_NAME _T(" ") ADU_VER
//Anis -> quando eseguo update di nodes.dat?
#define CONTATTI_KADU_MINIMI 50
#define REG_VLC_DIR _T("Software\\VideoLAN\\VLC")
#define REG_VLC_VAR _T("InstallDir")
#define VLC_EXE_NAME _T("\\vlc.exe")
#define MIRRORS_URL	_T("http://langmirror%i.emule-project.org/lang/%i%i%i%i/")
//Anis -> //velocità indicative in kbyte di download minimo del tipo di connessione.
//devono essere il più basse possibile poichè tali velocità devono essere superate in download -> SpeedTest Upload
//Logicamente devono essere direttamente proporzionali tra di loro
#define MIN_ADSL_1 1 
#define MIN_ADSL_2 180
#define MIN_ADSL_4 200
#define MIN_ADSL_6 550
#define MIN_ADSL_8 800
#define MIN_ADSL_12 1300
#define MIN_ADSL_16 1400
#define MIN_ADSL_20 1800
#define MIN_FIBRA_10 1100
#define MIN_FIBRA_100 3000
//Anis -> Frequenza CPU Minima per configurazione di connessioni (hz)
// PER ORA NON OPERATIVO #define CPU 1500
//Anis -> Capacità download massima
#define ADSL1 128
#define ADSL2 256
#define ADSL4 512
#define ADSL6 768
#define ADSL8 1024
#define ADSL12 1536
#define ADSL16 2048
#define ADSL20 2560
#define FIBRA10 1280
#define FIBRA100 12800
//Anis -> Capacità upload massima
#define UPLOAD_ADSL_LOW 64
#define UPLOAD_ADSL 128
#define UPLOAD_FIBRA 1280

//Anis -> Upload Slot
#define UPLOAD_SLOT_ADSL_LOW 6
#define UPLOAD_SLOT_ADSL 10
#define UPLOAD_SLOT_FIBRA 20

//Anis -> Nuova tipologia di riconoscimento connessione non più basata su IP ma su velocità di connessione.
enum tipo_fastweb {ADSL, FIBRA, SCONOSCIUTO};

// Parametri e definizioni esportate dal file AdunanzA.cpp
#define ADUNANZA_ANY        0
#define ADUNANZA_EXTERN     1
#define ADUNANZA_FASTWEB    2
#define ADUNANZA_NONE    0xff

// Anis -> refresh automatico file condivisi
#define ASFU_OFF		0
#define ASFU_DEFAULT	1
#define ASFU_SIMPLE		2

// Definizioni per icone e per riconoscimento clients
#define ADUNANZA_ICON_NONE  0
#define ADUNANZA_ICON_FW    1
#define ADUNANZA_ICON_ADU   2

// Definizione default del numero di slots
// Definizione default banda Adu
#define ADUNANZA_DEF_UPSL   8

#define ADUNANZA_MIN_BW_TROLLER 2
#define ADU_MAX_RATIO_KADU_DOWN 3145728 // 3 mega * 1024 KB in mega * 1024 byte in KB
#define ADU_EXT_FULL_WAIT_TIME 480 //8 min
#define ADU_VER_CODE(M,m,o) (((M) << 12) + ((m) << 7) + (o))

//ICS  - Dynamic Block Request [NetFinity] Adaptation by Anis
#define	CM_RELEASE_MODE			1
#define	CM_SPREAD_MODE			2
#define	CM_SHARE_MODE			3

#define	CM_SPREAD_MINSRC		10
#define	CM_SHARE_MINSRC			25
#define CM_MAX_SRC_CHUNK		3
//ICS  - Dynamic Block Request

//Anis -> Chunk minimo per client esterni
#define MIN_BLOCK_SIZE	9728

// ADU URLS
#define ADU_REMOTE_FILTER    "http://update.adunanza.net/adu_remoteipfilter.dat" //ADD by Tigerjact
#define PORTTESTURL       _T("http://adutest.adunanza.net/emule_testport/adutest.php?tcp=%i&udp=%i&lang=%i")
#define ADURM_URL            "http://update.adunanza.net/"  _AduR(ADU_VER_MAJ,ADU_VER_MIN) "/adu_remote.conf"
#define ADU_LANG_URL      _T("http://update.adunanza.net/") _AduRU(ADU_VER_MAJ,ADU_VER_MIN) _T("/")
#define ADU_NODES_DAT        "http://update.adunanza.net/"  _AduR(ADU_VER_MAJ,ADU_VER_MIN) "/adu_nodes.dat"

#if ADU_BETA_MAJ > 0
	#define ADU_UPD_URL _T("http://update.adunanza.net/") _AduRU(ADU_VER_MAJ,ADU_VER_MIN) _T("/adupdater.beta.adu")	
#else
	#define ADU_UPD_URL        _T("http://update.adunanza.net/adupdater.adu")	
#endif

// Definizione zone italia FW
#define MAN_MILANO 1
#define MAN_MILANO_H_NORD 2
#define MAN_GENOVA 5
#define FASTWEB_ROUTERS 10
#define MAN_VENETO 11
#define MAN_MILANO_H_SUD 14
#define MAN_ANCONA 21
#define MAN_GROSSETO 22
#define MAN_ROMA 23
#define MAN_TOSCANA 27
#define MAN_PIEMONTE_BIS 28
#define MAN_TRIVENETO 29
#define MAN_BARI 31
#define MAN_SICILIA 36
#define MAN_BOLOGNA 37
#define MAN_NAPOLI 39
#define MAN_TORINO 41
#define MAN_REGGIO_EMILIA 42
#define MAN_SARDEGNA 51

//Anis
UINT		ConfiguraPerNuoviUtenti(LPVOID lpParameter);
void		AduUpdate();
UINT		CheckKadCallThread(LPVOID lpParameter);
void		StartAdunanzaTest_Settings();
bool		AduIsFastWebIP(register uint32 ip);
bool		AduIsFastWebLANIP(register uint32 ip);
DWORD		AduGetTypeBand();
bool		AduIsValidKaduAddress(register uint32 host);
DWORD		AduGetCurrentIP(void);
bool		IsFibraAdunanzA(void);
void		AduTipLowUp(void);
void		AduTipBlock(register uint32 adutip);
bool		AduTipShow(register uint32 adutip);
DWORD		AduNextClient();
void		CalcolaRatio (register bool updatepage);
float		CalcolaStima(register float avail, uint32& firstPublish, register uint32 publishInterval, register uint32 pubkRTK, register bool sameIP, register uint32 now);
float		NormalizzaStima(register float avail, register uint32 from, register uint32 to);

// Define utili per la versione del mulo
#define ADU_CUR_MAJ_NUM ADU_VER_MAJ
#define ADU_CUR_MIN_NUM ADU_VER_MIN
#define ADU_CUR_MAJ_BETA_NUM ADU_BETA_MAJ // Theking0 - controllo anche beta

//Anis -> quando eseguo l'update di nodes.dat
#define CONTATTI_MINIMI			50

#define ADUTIP_FAKE              0x0
#define ADUTIP_BROKENKAD        0x10
#define ADUTIP_LOWUPLIMITS      0x20
#define ADUTIP_LOWDOWNLIMITS    0x40

#define FIB_LOWDOWN 500
#define FIB_LOWUP 500
#define FIB_LOWSLOTS 8

#define DSL_LOWDOWN 100
#define DSL_LOWUP 30
#define DSL_LOWSLOTS 4
#define COEFFICENTE_STIMA 2 //Anis -> più il valore è alto, più bassa è la stima.

#include "resource.h"

class AskPort : public CDialog
{
	DECLARE_DYNAMIC(AskPort)

public:
	AskPort(CWnd* pParent = NULL);   // costruttore standard
	virtual ~AskPort();
	enum { IDD = IDD_ASKPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
	virtual BOOL OnInitDialog(); //Anis
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedYes();
	afx_msg void OnBnClickedButton1();
};


// finestra di dialogo AduWait

class AduWait : public CDialog
{
	DECLARE_DYNAMIC(AduWait)

public:
	AduWait(CWnd* pParent);   // costruttore standard
	virtual ~AduWait();
	void SetText(LPCTSTR testo = NULL);
// Dati della finestra di dialogo
	enum { IDD = IDD_ADU_WAITING };
protected:
	virtual BOOL OnInitDialog(); //Anis
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
	BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
};
