#include "stdafx.h"
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif
#include "emule.h"
#include "updownclient.h"
#include "Friend.h"
#include "ClientList.h"
#include "OtherFunctions.h"
#include "PartFile.h"
#include "ListenSocket.h"
#include "Friend.h"
#include "OPCodes.h"
#include "SafeFile.h"
#include "Preferences.h"
#include "ClientCredits.h"
#include "IPFilter.h"
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "SearchList.h"
#include "SharedFileList.h"
#include "Sockets.h"
#include "Statistics.h"
#include "Log.h"
#include "HttpDownloadDlg.h"
#include "emuleDlg.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/prefs.h"
#include "DAMessageBox.h"
#include "ServerList.h"
#include "sockets.h"
#include "PreferencesDlg.h"
#include "HttpComm.h"
#include <math.h>
#include "server.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "UserMsgs.h"
#include "Kademlia/Net/KademliaUDPListener.h"
#include "StatisticsDlg.h"
#include "Scheduler.h"
#include "AduWizard.h"//Anis
#include "AdunanzA.h"
#include "RemoteSettings.h"

//Anis -> revisione totale. Il modulo è stato decisamente riscritto per un buon 70%

CString m_sFilenameUPDATER = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("lastest.adu");
CString m_sFilenameUPDATEREXE = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("adu_update.exe");
extern bool WizardNotOpen;

#ifndef _CONSOLE
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "ChatWnd.h"
#endif

bool updating = false;
bool isnew = false;
uint32 porttcp = NULL;
uint32 portudp = NULL;

//Anis -> è fastweb ? true : false
bool AduIsFastWebLANIP(register uint32 ip)
{ 
	switch (ip & 0xff) 
	{
		case MAN_ROMA:
		case MAN_MILANO:
		case MAN_MILANO_H_NORD:
		case MAN_MILANO_H_SUD:
		case MAN_GENOVA:
		case MAN_TRIVENETO:
		case MAN_BARI:
		case MAN_BOLOGNA:
		case MAN_NAPOLI:
		case MAN_TORINO:
		case MAN_REGGIO_EMILIA:
		case MAN_TOSCANA:
		case MAN_VENETO:
		case MAN_GROSSETO:
		case MAN_SICILIA:
		case MAN_ANCONA:
		case MAN_PIEMONTE_BIS:
		case MAN_SARDEGNA:
		case FASTWEB_ROUTERS:
			return true;
		default:
			return false;
	}
}

bool AduIsFastWebIP(register uint32 ip)
{
	if (AduIsFastWebLANIP(ip)) 
		return true;

	// match rapido: (ip & mask) == netid
	if ((ip & 0x00e0ffff) == 0x00008cd5 || // netid 213.140.0.0/19	- netid (hex) 0x00008cd5 / mask 0x00e0ffff *** da 213.140.0.1 a 213.140.31.254 
		(ip & 0x00e0ffff) == 0x00209cd5 || // netid 213.156.32.0/19	- netid (hex) 0x00209cd5 / mask 0x00e0ffff *** da 213.156.32.1 a 213.156.63.254
		(ip & 0x00c0ffff) == 0x0040653e || // netid 62.101.64.0/18	- netid (hex) 0x0040653e / mask 0x00c0ffff *** da 62.101.64.1 a 62.101.127.254
		(ip & 0x0080ffff) == 0x0000d051 || // netid 81.208.0.0/17	- netid (hex) 0x0000d051 / mask 0x0080ffff *** da 81.208.0.1 a 81.208.127.254
		(ip & 0x0080ffff) == 0x00006753 || // netid 83.103.0.0/17	- netid (hex) 0x00006753 / mask 0x0080ffff *** da 83.103.0.1 a 83.103.127.254
		(ip & 0x0000ffff) == 0x00001255 || // netid 85.18.0.0/16    - netid (hex) 0x00001255 / mask 0x0000ffff *** da 85.18.0.1 a 85.18.255.254
		(ip & 0x0000feff) == 0x00006059 || // netid 89.96.0.0/15    - netid (hex) 0x00006059 / mask 0x0000feff *** da 89.96.0.1 a 89.97.255.254
		(ip & 0x0000e0ff) == 0x0000205D    // netid 93.32.0.0/11    - netid (hex) 0x0000205d / mask 0x0000e0ff *** da 93.32.0.1 a 93.63.255.254
	    )return true; 

	return false;
}

//Anis -> mi serve per impostare la porta. differenziare hag/router. Il thread rimane in esecuzione fino a quando la nuova config viene impostata sia per utenti nuovi che con vecchi abbonamenti.
/*Anis -> Per ora è disattivata... 
sarà da implementare nella versione successiva effettuando un parsing del codice html della myfastpage. 
In questo modo è possibile configurare il mulo in modo automatico. Stessa cosa da fare per lo speedtest.
(dalla myfastpage) Per info chiedere ad Hammon che abbiamo discusso sulla questione del prelievo dati dalla myfastpage. 
(ricordo che nome utente e password della myfastpage non servono più accedendo via locale).*/
UINT ConfiguraPerNuoviUtenti(LPVOID lpParameter) 
{
	try 
	{ //Anis's function
		while (true) 
		{
			::Sleep(10);
			if (WizardNotOpen) 
			{
				isnew = false;
				portudp = NULL;
				porttcp = NULL;
				::Sleep(5000);
				CString IP = ipstr(theApp.GetPublicIP());
				static LPWSTR sName;
				DWORD dwLen = sizeof(sName);
				GetComputerName(sName, &dwLen);
				HOSTENT *host = gethostbyname(CT2CA(sName)); //Anis
				DWORD dwIP = 0;
				dwIP |= (BYTE)(host->h_addr_list[0][3]); //La mia terzina :P
				AskPort obj;
				//Anis -> Configurazione porte automatico basato sul riconoscimento di indirizzi IP.
				IP.Delete(4,IP.GetLength()); //Anis -> Mozzo la stringa e prendo il primo pezzettino partendo dall'inizio (primi 4 caratteri)
				if(!IP.Compare(_T("93.5")) || !IP.Compare(_T("2.22")) || !IP.Compare(_T("2.23"))) 
				{
					isnew = true;
					if((thePrefs.udpport == (4672 + (dwIP - 128))) || (thePrefs.port == 4662 + (dwIP - 128))) //Anis -> Alcuni IP anche se iniziano con 93.5 o 2.23 non necessitano del cambio delle porte.
					{
						::AfxEndThread(0);
						return 0;
					}
					portudp = 4672 + (dwIP - 128);
					porttcp = 4662 + (dwIP - 128);
					if(obj.DoModal()) {
						thePrefs.port = porttcp;
						thePrefs.udpport = portudp;
						CDAMessageBox message_box(NULL, _T("Riavviare eMule AdunanzA per rendere effettivo il settaggio delle porte."), false, false);
						message_box.DoModal();
						::AfxEndThread(0);
						return 0;
					}
				}
				else if(!IP.Compare(_T("0.0."))) {continue;}
				else 
				{ 
					//Anis -> Utente standard. Le porte rimangono quelle.
					if(obj.DoModal()) 
					{
						thePrefs.port = 4662;
						thePrefs.udpport = 4672;
						::AfxEndThread(0);
						return 0;
					}
				}
			}
		}
	}
	catch(...) { ::AfxEndThread(0); } //Anis -> meglio stoppare...
} 

void StartAdunanzaTest_Settings() 
{ //Anis
		theApp.emuledlg->CloseConnection();
		thePrefs.m_AduNoTips = true;
		extern bool WizardAdunanzA(); //Anis -> Configurazione ADU
		if (WizardAdunanzA()) 
		{
			extern bool X;
			if (X) 
			{
				X = false;
				CHttpDownloadDlg::X = false;
				thePrefs.MaxConperFive = 150;
				
				extern bool escludi0;
				extern bool escludi1;
				extern bool escludi2;
				extern bool escludi3;
				extern bool escludi4;
				extern bool escludi5;
				extern bool escludi6;
				extern bool escludi7;
				extern bool escludi8;
				extern bool escludi9;
				extern bool isAutomatic;
				extern void ConfiguraAdunanza();

				if (isAutomatic) 
				{
					if (escludi0 == true && escludi1 == true && escludi2 == true && escludi3 == true && escludi4 == true && escludi5 == true && escludi6 == true && escludi7 == true && escludi8 == true && escludi9 == true) 
					{
						AfxMessageBox(_T("Il server risulta essere offline. Verrà lanciata la configurazione manuale."));
						AduWizard WizardPersonalizzato;
						WizardPersonalizzato.DoModal();
					}
				}

				ConfiguraAdunanza();
				//AfxMessageBox(_T("La configurazione è stata effettuata correttamente. Per il corretto funzionamento della rete KAdu, si consiglia il riavvio di eMule AdunanzA."),MB_ICONINFORMATION);
				
				escludi0 = true;
				escludi1 = true;
				escludi2 = true;
				escludi3 = true;
				escludi4 = true;
				escludi5 = true;
				escludi6 = true;
				escludi7 = true;
				escludi8 = true;
				escludi9 = true;

				//Anis -> imposto capacità max upload
				switch (thePrefs.maxGraphUploadRate) 
				{
					case UPLOAD_ADSL: thePrefs.m_AduMaxUpSlots = UPLOAD_SLOT_ADSL; thePrefs.adsl_fiber = ADSL; break;
					case UPLOAD_ADSL_LOW: thePrefs.m_AduMaxUpSlots = UPLOAD_SLOT_ADSL_LOW; thePrefs.adsl_fiber = ADSL; break;
					case UPLOAD_FIBRA: thePrefs.m_AduMaxUpSlots = UPLOAD_SLOT_FIBRA; thePrefs.adsl_fiber = FIBRA; break;
				}

				theApp.emuledlg->statisticswnd->SetARange(false, thePrefs.GetMaxGraphUploadRate(true));
				theApp.emuledlg->statisticswnd->SetARange(true, thePrefs.maxGraphDownloadRate);
				theApp.scheduler->SaveOriginals();
				theApp.emuledlg->preferenceswnd->m_wndConnection.LoadSettings();
				theApp.emuledlg->preferenceswnd->m_wndAdunanzA.LoadSettings();
				DeleteFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("speed_test.adu"));
			}
		}

		else 
		{ //Anis
			 AfxMessageBox(_T("Errore critico nel core di AdunanzA. Rivolgersi al supporto tecnico nel forum ufficiale di AdunanzA. codice: 1"),MB_ICONEXCLAMATION);
			 exit(EXIT_FAILURE);
		}

		WizardNotOpen = true;
		thePrefs.m_AduNoTips = false;
		theApp.emuledlg->StartConnection();

		switch(thePrefs.maxupload) 
		{
			case UPLOAD_ADSL_LOW:
			case UPLOAD_ADSL:
				thePrefs.adsl_fiber = ADSL;
			break;
			case UPLOAD_FIBRA:
				thePrefs.adsl_fiber = FIBRA;
		}

		//Anis -> Da implementare nella next version!!
		//::AfxBeginThread(ConfiguraPerNuoviUtenti, NULL, 0, 0, 0, NULL);  //Anis -> Controllo IP nuovi.
}

bool AduIsValidKaduAddress(register uint32 host)
{
	return AduIsFastWebIP(ntohl(host));
}

DWORD AduGetCurrentIP(void)
{
	if (Kademlia::CKademlia::GetIPAddress()) 
		return Kademlia::CKademlia::GetIPAddress();
	else
		return NULL;
}

DWORD CUpDownClient::GetClientAduType(void) const
{
	if (HasFastwebIP())
	{
		if (GetIsAduSoftware()) 
			return ADUNANZA_ICON_ADU;
		else 
			return ADUNANZA_ICON_FW;
	}
	return ADUNANZA_ICON_NONE;
}

bool CUpDownClient::HasFastwebIP(void) const
{
	return AduIsFastWebIP(m_dwUserIP);
}

bool CUpDownClient::IsAduClient(void) const {

	return GetClientAduType() == ADUNANZA_ICON_ADU;

}

DWORD AduGetTypeBand()
{
		if (((theStats.sessionSentBytes - theStats.stat_Adu_sessionSentBytes)/(((GetTickCount() - theStats.transferStarttime) / 1000)*1024)) < thePrefs.m_AduValRipBanda )
		{	
			if ((theApp.uploadqueue->waitinglist.GetCount() - theApp.uploadqueue->GetAdunanzAUserCount()) <= 0)
				return ADUNANZA_FASTWEB;
			else
				return ADUNANZA_EXTERN;
		}
		else 
			return ADUNANZA_FASTWEB;
}

DWORD AduNextClient() 
{
	if (theApp.uploadqueue->GetUploadCount() < thePrefs.m_AduMaxUpSlots)
	{
		if ( theApp.uploadqueue->GetAdunanzAUserCount() <= 0 )
		{
			if(thePrefs.GetVerbose() && theApp.uploadqueue->GetActiveUploadsCount() >= thePrefs.m_AduMaxUpSlots)
			{
				static uint32 adunanza = 0;
				adunanza++;
				AddDebugLogLine(DLP_HIGH,false, _T("Non ci sono AdunanzA in coda. %u"),adunanza);
				if (adunanza > 50000)
					adunanza = 0;
			}
			return ADUNANZA_EXTERN;
		}

		if ( theApp.uploadqueue->waitinglist.GetCount() - theApp.uploadqueue->GetAdunanzAUserCount() <= 0 )
		{	
			if(thePrefs.GetVerbose() && theApp.uploadqueue->GetActiveUploadsCount() >= thePrefs.m_AduMaxUpSlots)
			{
				static uint32 esterni = 0;
				esterni++;
				AddDebugLogLine(DLP_HIGH,false, _T("Non ci sono esterni in coda. %u"),esterni);
				if (esterni > 50000)
					esterni = 0;
			}
			return ADUNANZA_FASTWEB;
		}

		if (((theStats.sessionSentBytes - theStats.stat_Adu_sessionSentBytes)/(((GetTickCount() - theStats.transferStarttime) / 1000)*1024)) < thePrefs.m_AduValRipBanda )
			return ADUNANZA_EXTERN;
		else 
			return ADUNANZA_FASTWEB;
	} 
	else
		return ADUNANZA_NONE;
}

bool CemuleApp::IsFirewalledClient(CUpDownClient *client)
{
	// Mod Adu
	// Emanem
	// Privilegio la verifica sulla rete ed2k se un client e' esterno
	if (client && (client->HasFastwebIP() == false))
	{
		if (theApp.serverconnect->IsConnected() && !theApp.serverconnect->IsLowID()) 
			return false; // we have an eD2K HighID -> not firewalled
		return true;
	}
	
	return IsFirewalled();
}

void AduUpdate()
{ 
	//tigerjact, Anis
	try 
	{
		//Anis -> la tabulazione del codice era inesistente.
	    CHttpDownloadDlg dlgDownload;
	    dlgDownload.m_sURLToDownload = ADU_UPD_URL;
	    dlgDownload.m_sFileToDownloadInto = m_sFilenameUPDATER;
	   
		if (dlgDownload.DoModal())
		{   // vedo la prima linea che versione e'
		    FILE *fp = _tfopen(m_sFilenameUPDATER, _T("r") );
		    if (fp)
		    {
			    int	maj_num = ADU_CUR_MAJ_NUM, min_num = ADU_CUR_MIN_NUM, maj_beta_num = ADU_CUR_MAJ_BETA_NUM; // Theking0 - controllo anche beta
				if (3 == _ftscanf(fp, _T("%i.%i.%i\n"), &maj_num, &min_num, &maj_beta_num))
			    {
				    // controllo i major e minor
				    if ((maj_num > ADU_CUR_MAJ_NUM) || ((maj_num == ADU_CUR_MAJ_NUM) && (min_num > ADU_CUR_MIN_NUM)) || ((maj_num == ADU_CUR_MAJ_NUM) && (min_num == ADU_CUR_MIN_NUM) && (maj_beta_num > ADU_CUR_MAJ_BETA_NUM)) || ((maj_num == ADU_CUR_MAJ_NUM) && (min_num == ADU_CUR_MIN_NUM) && (maj_beta_num == 0) &&(ADU_CUR_MAJ_BETA_NUM > 0)))
					{
						//Anis -> messaggio remoto
						if(AfxMessageBox(theApp.rm->UpdateMessage,MB_ICONINFORMATION|MB_YESNO) == IDYES) 
						{
							int read = 0;
							char mirrorlist[10][1000];
							while(fscanf(fp,"%s\n",&mirrorlist[read])>0)
								read++;
							read--;
							int nummirr = 0;

							while(true) 
							{
								CHttpDownloadDlg dlgDownload;
								dlgDownload.m_sURLToDownload = mirrorlist[nummirr];
								dlgDownload.m_sFileToDownloadInto = m_sFilenameUPDATEREXE; 
								updating = true; //Anis
								
								int dialog = dlgDownload.DoModal();
								if (dialog == IDOK)
									break;

								else 
								{
									nummirr++;
									if (nummirr > read)
									{
										CString msg;
										msg.Format(_T("ATTENZIONE!\n\n") _T("Il servizio di autoaggiornamento non è al momento disponibile. Si prega di riprovare al prossimo avvio di eMule AdunanzA."));
										CDAMessageBox mblu(NULL, msg, false);
										mblu.DoModal();
										updating = false;
										DeleteFile(m_sFilenameUPDATER);
										return;
									}
								}
							}
							DeleteFile(m_sFilenameUPDATER);
							//Anis -> c'erano codici inutilizzati e if vuoti. boh :\
							fclose(fp);
							ShellExecute(NULL, NULL, _T("adu_update.exe"), NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), SW_SHOWDEFAULT);
							ExitProcess(0);//Anis -> Chiusura soft perchè cmq non ha ancora caricato nulla.
					}
				}
			    fclose(fp);
			}
	    }
	}
		DeleteFile(m_sFilenameUPDATER);
	}
	catch(...) {ExitProcess(1);} //Anis -> Precauzione
}

void AduTipBlock(uint32 adutip) 
{
	thePrefs.m_AduTips |= adutip;
}

bool AduTipShow(uint32 adutip) 
{

	if (adutip == ADUTIP_FAKE || thePrefs.m_AduNoTips == true)
		return false;

	return !(thePrefs.m_AduTips & adutip);
}

bool IsFibraAdunanzA(void) 
{
	//Anis -> Questa bellezza della natura è stata scritta da un professionista
	return (thePrefs.adsl_fiber == FIBRA);
}

//Anis -> Per eseguire sta funzione devo aver fatto almeno 1 volta l'AdunanzASpeedTest.
void AduTipLowUp(void) 
{ //Anis -> rewrite da zero.
		
	if ( Kademlia::CKademlia::IsRunning() && Kademlia::CKademlia::IsConnected() && !Kademlia::CKademlia::IsFirewalled() ) {
		uint32 ip = 0;
		if (Kademlia::CKademlia::GetPrefs())
			ip = ntohl(Kademlia::CKademlia::GetPrefs()->GetIPAddress());

		CString connstr = _T("");
		uint8 fiber = IsFibraAdunanzA();
		switch(fiber) {
			case true:
				connstr = _T("in Fibra Ottica");
				break;
			case false:
				connstr = _T("ADSL");
				break;
			default:
				return; //Anis -> non faccio nemmeno il controllo non sapendo che connessione uno abbia. in poche parole sono "cazzi" dell'utente.
		}

		if ( ip && AduIsFastWebLANIP(ip) &&	AduTipShow(ADUTIP_LOWUPLIMITS) ) 
		{
			BOOL signal = false;

			if (fiber && ( thePrefs.maxupload < FIB_LOWUP || thePrefs.m_AduMaxUpSlots < FIB_LOWSLOTS ) )
				signal = true;
			else if (!fiber && ( thePrefs.maxupload < DSL_LOWUP || thePrefs.m_AduMaxUpSlots < DSL_LOWSLOTS ) )
				signal = true;

			if (signal) 
			{
				CString msg;//modifiche AST - Anis
				msg.Format(_T("ATTENZIONE!\n\n")
					_T("I tuoi settaggi di banda correnti non sono adatti alla tua linea %s.\n\n")
					_T("Con questi settaggi, non ottenendo sufficenti crediti per scaricare, non riuscirai mai a sfruttare appieno la tua linea.\n\n")
					_T("Eseguite nuovamente Opzioni->Connessione->AdunanzA First Time Wizard... per correggere il problema."),
					connstr);

				CDAMessageBox mblu(NULL, msg, true, true);

				if (mblu.DoModal() == IDOK)
					AduTipBlock(ADUTIP_LOWUPLIMITS);
			}	
		}
	}
}


UINT CheckKadCallThread(LPVOID lpParameter)
{
  try 
  {
		while (true) 
		{
			::Sleep(SEC2MS(600));
			if (!Kademlia::CKademlia::IsRunning() || !Kademlia::CKademlia::IsConnected() || Kademlia::CKademlia::IsFirewalled()) 
			{
				if (AduTipShow(ADUTIP_BROKENKAD)) 
				{
				
					 CDAMessageBox mb(NULL,
						_T("E' stato riscontrato un problema nella connessione a KAdu.\n\n")
						_T("Possibili cause:\n")
						_T(" - firewall attivo e non correttamente configurato\n")
						_T(" - accesso alla rete fastweb non diretto (ad. es.: tramite router o altro pc)\n")
						_T(" - incompatibilità tra software di controllo della rete e KAdu (ad. es.: norton)\n\n")
						_T("Senza risolvere questo problema vi è impossibile ottenere buone prestazioni.\n\n"),
						 true,true);
						if (mb.DoModal() == IDOK)
							AduTipBlock(ADUTIP_BROKENKAD);
				}

			} 
			else 
			{
				register uint32 ip = 0;

				if (Kademlia::CKademlia::GetPrefs())
					ip = ntohl(Kademlia::CKademlia::GetPrefs()->GetIPAddress());
				
				if(thePrefs.adsl_fiber == ADSL || thePrefs.adsl_fiber == FIBRA) //Anis
					AduTipLowUp();
	
				if (ip && AduIsFastWebLANIP(ip) && AduTipShow(ADUTIP_LOWDOWNLIMITS)) 
				{
					BOOL signaldown = false;

					if (thePrefs.maxdownload < FIB_LOWDOWN && thePrefs.adsl_fiber == FIBRA) //ANIS /AdunanzASpeedTest
						signaldown = true;
					else if (thePrefs.maxdownload < DSL_LOWDOWN && thePrefs.adsl_fiber == ADSL)
						signaldown = true;

					if (signaldown) 
					{
						CString msg;
						msg.Format(_T("ATTENZIONE!\n\n")
							_T("Hai dei limiti di download bassi per la tua linea.\n\n")
							_T("Esegui nuovamente Opzioni->Connessione->Wizard - AdunanzA First Time Wizard... per correggere il problema."));

						CDAMessageBox mbld(NULL, msg, true, true);

						if (mbld.DoModal() == IDOK)
							AduTipBlock(ADUTIP_LOWDOWNLIMITS);
					}
				}
			}
		}
	} 
	catch(...) {}
	::AfxEndThread(0);
	return 0;
}

//Anis -> nelle seguenti funzioni ho provveduto ad espandere codice in linea manualmente per ottenere benefici di prestazioni. (il compilatore lo fa in automatico, ma spesso non lo fa come lo vuole lo sviluppatore :\)
float CalcolaStima(register float avail, uint32& firstPublish, register uint32 publishInterval, register uint32 pubkRTK, register bool sameIP, register uint32 now) 
{
	// Using this increment style, we can guess also if clients use different publish rates.
	register float inc = (float)pubkRTK / theApp.rm->kadRepublishTimeK;
	
	// Only old clients, with republishtimek == 24hrs don't publish their republishtimek.
	if (inc == 0.f)
  	inc = HR2S(5) / theApp.rm->kadRepublishTimeK;	

	if (publishInterval > theApp.rm->kadRepublishTimeK)
		avail=0.f,firstPublish=now;	

	while ((now-firstPublish) > theApp.rm->kadRepublishTimeK && avail > 2.f)
		avail--,firstPublish += (uint32)(theApp.rm->kadRepublishTimeK/avail);

	// Some client seem to send duplicate publish to already contacted clients.
	// In 90% of the times we avoid this here.
	if (!sameIP)
		avail += inc;

  return avail;
}

//Anis -> Se l'algoritmo fosse stato fatto di qualità maggiore, questa funzione non sarebbe servita ad un tubo di nulla. Purtroppo è necessaria se non si vuole avere un bel MAX int a 32bit per tutti i risultati ottenuti...
float NormalizzaStima(register float avail, register uint32 from, register uint32 to) 
{
 	register uint32 norm_factor = COEFFICENTE_STIMA;
	//Anis -> Ottimizzazione
	avail = min(avail, 100000);

	if (avail > theApp.rm->kadFreshGuess_NoNorm) 
	{
		if (to > from)
			norm_factor = (to - from)/theApp.rm->kadRepublishTimeK;
		else
			norm_factor = COEFFICENTE_STIMA;

		norm_factor = (norm_factor < theApp.rm->kadFreshGuess_Tol ? theApp.rm->kadFreshGuess_Tol : norm_factor);

		// We have a good number of publishes, but not high.
		// Low normalization is still suggested.
		if (avail < theApp.rm->kadFreshGuess_LowNorm && norm_factor > avail / theApp.rm->kadFreshGuess_LowNorm)
			norm_factor /= powf(avail/theApp.rm->kadFreshGuess_LowNorm, 0.5f);
	}

	if (avail > 2.f)
		avail /= powf(norm_factor, theApp.rm->kadFreshGuess_Weight);
	
	return avail;
}

//tigerjact
void CalcolaRatio(register bool updatepage)
{
	if ((thePrefs.m_AduRipBanda))
		thePrefs.m_AduValRipBanda = theApp.rm->m_AduValRipBanda_Std;

	if (thePrefs.m_AduValRipBanda > thePrefs.GetMaxUpload())
		thePrefs.m_AduValRipBanda = thePrefs.GetMaxUpload()- ADUNANZA_MIN_BW_TROLLER;

	if (thePrefs.m_AduValRipBanda < 10)
		thePrefs.m_AduExtMaxDown = (thePrefs.m_AduValRipBanda)*4*1024;

	if (thePrefs.m_AduValRipBanda < 4)
		thePrefs.m_AduExtMaxDown = (thePrefs.m_AduValRipBanda)*3*1024;

	if (thePrefs.m_AduValRipBanda >= 10)
		thePrefs.m_AduExtMaxDown = UNLIMITED*1024;
	
	uint32 getupspeed = 0;
	if (thePrefs.GetMaxUpload()!= UNLIMITED)
		//ho un limite di banda	
		getupspeed = thePrefs.GetMaxUpload();
	
	else 
		getupspeed = thePrefs.GetMaxGraphUploadRate(true);

	if((getupspeed - thePrefs.m_AduValRipBanda)< 20)
	{
		if (thePrefs.GetMaxDownload()!= UNLIMITED)
			thePrefs.m_AduKaduMaxDown = ((thePrefs.GetMaxDownload())*1024) /2;
		else
			thePrefs.m_AduKaduMaxDown = ((thePrefs.GetMaxGraphDownloadRate())*1024) /2;
		
		if (thePrefs.m_AduKaduMaxDown > ADU_MAX_RATIO_KADU_DOWN)
			thePrefs.m_AduKaduMaxDown = ADU_MAX_RATIO_KADU_DOWN;
	}

	else 
		thePrefs.m_AduKaduMaxDown = UNLIMITED*1024;
	
	if ((!thePrefs.m_AduRipBanda) && updatepage)
		theApp.emuledlg->preferenceswnd->m_wndAdunanzA.LoadSettings();
}

IMPLEMENT_DYNAMIC(AskPort, CDialog)

AskPort::AskPort(CWnd* pParent /*=NULL*/) : CDialog(AskPort::IDD, pParent)
{
}

AskPort::~AskPort()
{
}

BOOL AskPort::OnInitDialog()
{
	if(isnew) 
	{
		CString tcpport;
		tcpport.Format(_T("TCP:%d"),::porttcp);
		CString udpport;
		udpport.Format(_T("UDP:%d"),::portudp);
		GetDlgItem(IDC_TCP_ADU)->SetWindowTextW(tcpport);
		GetDlgItem(IDC_UDP_ADU)->SetWindowTextW(udpport);
	}
	else 
	{
		GetDlgItem(IDC_TCP_ADU)->SetWindowTextW(_T("TCP: 4662"));
		GetDlgItem(IDC_UDP_ADU)->SetWindowTextW(_T("UDP: 4672"));
	}
	return true;
}


void AskPort::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(AskPort, CDialog)
	ON_BN_CLICKED(IDYES, OnBnClickedYes)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()

void AskPort::OnBnClickedYes()
{
	EndDialog(IDOK);
}

void AskPort::OnBnClickedButton1()
{
	EndDialog(IDCANCEL);
}

IMPLEMENT_DYNAMIC(AduWait, CDialog)

AduWait::AduWait(CWnd* pParent) : CDialog(AduWait::IDD, pParent)
{

}

AduWait::~AduWait()
{
	ClipCursor(NULL);
	SetCapture();
	ReleaseCapture();
	EndWaitCursor();
}

void AduWait::SetText(LPCTSTR testo)
{
	SetDlgItemText(IDC_ADU_WAITING, testo);
}

void AduWait::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL AduWait::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_F4 || pMsg->wParam == VK_LBUTTON || pMsg->wParam == VK_RBUTTON)
		return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL AduWait::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetDlgItemText(IDC_ADU_WAITING, _T("AdunanzA P2P Streaming v.1.0"));
	CenterWindow();
	BeginWaitCursor();
	CRect r;
	GetWindowRect(r);
	ClipCursor(&r);
	SetCapture();
	return TRUE;
}

BEGIN_MESSAGE_MAP(AduWait, CDialog)
END_MESSAGE_MAP()