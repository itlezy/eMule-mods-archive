#pragma
#include "stdafx.h"
#include "MPlayer.h"
#include "eMule.h"
#include "Log.h"

//boi
#include <windows.h>
#include <lm.h>
#include <stdio.h>
//boi

#define SIMPLEVLC_DLL L"simplevlc.dll"

/*
//BuGFiX: 
//it seems the FIRST call to the IsSeekable function always fails... dunno why up to now... 
//so this is a workaround... added everywhere where errors can occur just to be sure ;)
//try 10 times  = 1 second
#define VLC_ERROR_HANDLER(func, type)	\
	type ret = -1; \
	for(uint8 i = 0; i < 10 && ret == -1; ++i) \
	{ \
		ret = (func); \
		if(ret == -1) \
			Sleep(100); \
	} \
	if(ret < 0) \
	{ \
		ASSERT(0); \
		theApp.QueueDebugLogLineEx(LOG_ERROR, L"VLC: %hs failed!", __FUNCTION__); \
	} \
	if(ret < 0)
*/
//not necessary anymore I guess... due to status change checks
#define VLC_ERROR_HANDLER(func, type)	\
	type ret = (func); \
	if(ret < 0) \
	{ \
		ASSERT(0); \
		theApp.QueueDebugLogLineEx(LOG_ERROR, L"VLC: %hs failed!", __FUNCTION__); \
	} \
	if(ret < 0)

// required simplevlc version
#define REQ_SVLC_VERSION_MAJOR 0
#define REQ_SVLC_VERSION_MINOR 1

// global window proc for our display window
static long FAR PASCAL gDisplayWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    CMPlayer* Player = (CMPlayer*)::GetProp(hWnd, L"MPlayer");

    if(Player)
        return Player->DisplayWndProc(hWnd, Msg, wParam, lParam);

    return ::DefWindowProc(hWnd,Msg,wParam,lParam);
}


// global svlc callback
static void SVLC_CC gSVlcCallbackFunc (HSVLC svlc, SVlcCallbackEvent event, void *edata, void *udata)

{
    CMPlayer* Player = (CMPlayer*)udata;

    if(svlc != Player->hVlc)
        return; // wtf?

    // send a windows message so we are in the right thread
    ::SendMessage(Player->Handle, WM_SVLC_CALLBACK, (WPARAM)event, (LPARAM)edata);
}

// public
//CMPlayer::CMPlayer(TComponent* Owner)
//:   TWinControl(Owner),
CMPlayer::CMPlayer()
{
//obviously those 3 should be inherited from TWinControl...
//    Align = alClient;
//    BevelInner = bvNone;
//    BevelOuter = bvNone;
	DisplayWnd = NULL;
	CleanVariables(true);
}

void CMPlayer::CleanVariables(const bool bInit)
{
	if(!bInit)
	{
		// destroy svlc instance
		if(hVlc && VlcIntf)
		{
			VlcIntf->destroy(hVlc);
			delete VlcIntf;
		}
		if(pSVLC_Shutdown)
			pSVLC_Shutdown();
		if(hVlcDll)
			FreeLibrary(hVlcDll);
	}

	//"default" cleaning...
	hVlcDll = NULL;
	hVlc = NULL;
	VlcIntf = NULL;	
	pSVLC_GetInterface = NULL;
	pSVLC_GetVersion = NULL;
	pSVLC_Initialize = NULL;
	pSVLC_Shutdown = NULL;
}

CMPlayer::~CMPlayer()
{
    Shutdown();
}

bool CMPlayer::Initialize(const int LogLevel)
{
    if(!LoadVlc(LogLevel))
        return false;

    if(!CreateDisplayWnd())
        return false;

    // set display window
    if(VlcIntf->set_window(hVlc, (UINT)DisplayWnd) < 0)
        return false;

    // display video in original size by default
    SetDisplaySize(MPDefaultSize);

    return true;
}

bool CMPlayer::Shutdown()
{
    if(!IsVlcLoaded())
        return true;

    UnloadVlc();
    DestroyDisplayWnd();

    return true;
}
CString Acentuar(CString Filename)
{
	// definir Começar
	bool cedilhaMin = false;
	bool cedilhaMai = false;
	//-------//
	bool amin = false;
	bool amai = false;
	//-------//
	bool tilmin = false;
	bool tilmais = false;
	//-------//
	bool acirmin = false;
	bool acirmai = false;
	//-------//
	bool e1min = false;
	bool e1mai = false;
	//-------//
	bool e2min = false;
	bool e2mai = false;
	//-------//
	bool ecirmin = false;
	bool ecirmai = false;
	//-------//
	bool o1min = false;
	bool o1mai = false;
	//-------//
	bool o2min = false;
	bool o2mai = false;
	//-------//
	bool o3min = false;
	bool o3mai = false;
	//-------//
	bool o4min = false;
	bool o4mai = false;

	//-------//
	bool u1min = false;
	bool u1mai = false;
	//-------//
	bool i1min = false;
	bool i1mai = false;
	///////////////////////
	int numero;
	// definir Terminar
	
	//Comparar começar
	///////////////////////////////////////////////////
	 numero = Filename.Find(_T("ç"));
	if (numero>1)
	{
		cedilhaMin = true;
	}

	numero = 0;
	numero = Filename.Find(_T("Ç"));
	if (numero>1)
	{
		cedilhaMai = true;
	}
	////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("á"));
	if (numero>1)
	{
		amin = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Á"));
	if (numero>1)
	{
		amai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("ã"));
	if (numero>1)
	{
		tilmin = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Ã"));
	if (numero>1)
	{
		tilmais = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("â"));
	if (numero>1)
	{
		acirmin = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Â"));
	if (numero>1)
	{
		acirmai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("è"));
	if (numero>1)
	{
		e1min = true;
	}
	numero = 0;
	numero = Filename.Find(_T("È"));
	if (numero>1)
	{
		e1mai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("é"));
	if (numero>1)
	{
		e2min = true;
	}
	numero = 0;
	numero = Filename.Find(_T("É"));
	if (numero>1)
	{
		e2mai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("ê"));
	if (numero>1)
	{
		ecirmin = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Ê"));
	if (numero>1)
	{
		ecirmai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("ô"));
	if (numero>1)
	{
		o1min = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Ô"));
	if (numero>1)
	{
		o1mai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("ó"));
	if (numero>1)
	{
		o2min = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Ó"));
	if (numero>1)
	{
		o2mai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("õ"));
	if (numero>1)
	{
		o3min = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Õ"));
	if (numero>1)
	{
		o3mai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("ó"));
	if (numero>1)
	{
		o4min = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Ó"));
	if (numero>1)
	{
		o4mai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("ú"));
	if (numero>1)
	{
		u1min = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Ú"));
	if (numero>1)
	{
		u1mai = true;
	}
///////////////////////////////////////////////////////
	numero = 0;
	numero = Filename.Find(_T("í"));
	if (numero>1)
	{
		i1min = true;
	}
	numero = 0;
	numero = Filename.Find(_T("Í"));
	if (numero>1)
	{
		i1mai = true;
	}
///////////////////////////////////////////////////////
	//Comparar Terminar

	//Mudar caracts
	if (cedilhaMin)
	{
		Filename.Replace(_T("ç"),_T("\303\247"));// ç Minuscula
	}
	if (cedilhaMai)
	{
		Filename.Replace(_T("Ç"),_T("\303\207"));// ç Minuscula
	}
	////////////////////////////////////////////////////////////////
	if (amin)
	{
		Filename.Replace(_T("á"),_T("\303\241"));// á minuscula
	}
	if (amai)
	{
		Filename.Replace(_T("Á"),_T("\303\201"));// Á Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (tilmin)
	{
		Filename.Replace(_T("ã"),_T("\303\243"));// ã minuscula
	}
	if (tilmais)
	{
		Filename.Replace(_T("Ã"),_T("\303\203"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (acirmin)
	{
		Filename.Replace(_T("â"),_T("\303\242"));// ã minuscula
	}
	if (acirmai)
	{
		Filename.Replace(_T("Â"),_T("\303\202"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (e1min)
	{
		Filename.Replace(_T("è"),_T("\303\250"));// ã minuscula
	}
	if (e1mai)
	{
		Filename.Replace(_T("È"),_T("\303\210"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (e2min)
	{
		Filename.Replace(_T("é"),_T("\303\251"));// ã minuscula
	}
	if (e2mai)
	{
		Filename.Replace(_T("É"),_T("\303\211"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (ecirmin)
	{
		Filename.Replace(_T("ê"),_T("\303\252"));// ã minuscula
	}
	if (ecirmai)
	{
		Filename.Replace(_T("Ê"),_T("\303\212"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (o1min)
	{
		Filename.Replace(_T("ô"),_T("\303\264"));// ã minuscula
	}
	if (o1mai)
	{
		Filename.Replace(_T("Ô"),_T("\303\224"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (o2min)
	{
		Filename.Replace(_T("ô"),_T("\303\263"));// ã minuscula
	}
	if (o2mai)
	{
		Filename.Replace(_T("Ô"),_T("\303\223"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (o3min)
	{
		Filename.Replace(_T("õ"),_T("\303\265"));// ã minuscula
	}
	if (o3mai)
	{
		Filename.Replace(_T("Õ"),_T("\303\225"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (o4min)
	{
		Filename.Replace(_T("ó"),_T("\303\263"));// ã minuscula
	}
	if (o4mai)
	{
		Filename.Replace(_T("Ó"),_T("\303\223"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (u1min)
	{
		Filename.Replace(_T("ú"),_T("\303\272"));// ã minuscula
	}
	if (u1mai)
	{
		Filename.Replace(_T("Ú"),_T("\303\232"));// Ã Maiuscula
	}
	////////////////////////////////////////////////////////////////
	if (i1min)
	{
		Filename.Replace(_T("í"),_T("\303\255"));// ã minuscula
	}
	if (i1mai)
	{
		Filename.Replace(_T("Í"),_T("\303\215"));// Ã Maiuscula
	}
	/*	
	

	Filename.Replace(_T("â"),_T("\303\242"));// â cincunflexo Minusculo
	Filename.Replace(_T("Â"),_T("\303\202"));// Â Cincunflexo Maiusculo
	Filename.Replace(_T("È"),_T("\303\210"));// É Maiusculo
    Filename.Replace(_T("è"),_T("\303\250"));// é Minusculo
	Filename.Replace(_T("É"),_T("\303\211"));// É Maiusculo
	Filename.Replace(_T("é"),_T("\303\251"));// é Minisculo
	Filename.Replace(_T("Ê"),_T("\303\212"));// Ê Maiusculo
	Filename.Replace(_T("ê"),_T("\303\252"));// ê Minusculo
    */
	return Filename;
}
// playback
LPSTR UnicodeToAnsi(LPCWSTR s)
{
if (s==NULL) return NULL;
int cw=lstrlenW(s);
if (cw==0) {CHAR *psz=new CHAR[1];*psz='\0';return psz;}
int cc=WideCharToMultiByte(CP_ACP,0,s,cw,NULL,0,NULL,NULL);
if (cc==0) return NULL;
CHAR *psz=new CHAR[cc+1];
cc=WideCharToMultiByte(CP_ACP,0,s,cw,psz,cc,NULL,NULL);
if (cc==0) {delete[] psz;return NULL;}
psz[cc]='\0';
return psz;
}
bool CMPlayer::Play(CString Filename)
{
	LPSTR bstr;
	WCHAR wszUserName[UNLEN+1];          // Unicode user name
	CString acentudada;
	if(!IsVlcLoaded())
        return false;

    // make at least sure the file exists
    if(::GetFileAttributes(Filename) == 0xFFFFFFFF)
        return false;
	
	USES_CONVERSION;
	acentudada = Acentuar(Filename);; //acentuar da Forma maluca do boi
	//MultiByteToWideChar( CP_ACP, 0,acentudada,strlen(acentudada)+1, wszUserName,sizeof(wszUserName)/sizeof(wszUserName[0]) );
	
    //if(VlcIntf->play(hVlc, T2A(Filename)) < 0)
	//if(VlcIntf->play(hVlc, wszUserName) < 0)
	if(VlcIntf->play(hVlc, T2A(acentudada)) < 0)
		return false;

    return true;
}

bool CMPlayer::PlayNetwork(CString Url)
{
    if(!IsVlcLoaded())
        return false;

	USES_CONVERSION;
    if(VlcIntf->play(hVlc, T2A(Url)) < 0)
        return false;

    return true;
}

bool CMPlayer::Stop()
{
    if(!IsVlcLoaded())
        return false;

	VLC_ERROR_HANDLER(VlcIntf->stop(hVlc), int)
	    return false;

    return true;
}

bool CMPlayer::Pause(const bool Pause)
{
    if(!IsVlcLoaded())
        return false;

	VLC_ERROR_HANDLER(VlcIntf->pause(hVlc, Pause), int)
        return false;

    return true;
}

bool CMPlayer::TogglePause()
{
    if(!IsVlcLoaded())
        return false;

	VLC_ERROR_HANDLER(VlcIntf->pause(hVlc, -1), int)
        return false;

    return true;
}

CMPlayerState CMPlayer::GetState()
{
    if(!IsVlcLoaded())
        return MPStopped;

    return ConvertVlcState(VlcIntf->get_playback_state(hVlc));
}

bool CMPlayer::IsSeekable()
{
    if(!IsVlcLoaded())
        return 0;

	VLC_ERROR_HANDLER(VlcIntf->is_seekable(hVlc), int)
		return false;
	return true;
}

bool CMPlayer::SetPosition(const float Position)
{
	ASSERT(Position >= 0);
    if(!IsVlcLoaded() || Position < 0)
        return false;

	VLC_ERROR_HANDLER(VlcIntf->set_position(hVlc, Position), int)
        return false;

    return true;
}

float CMPlayer::GetPosition()
{
    if(!IsVlcLoaded())
        return 0;

	VLC_ERROR_HANDLER(VlcIntf->get_position(hVlc), float)
        return 0;

    return ret;
}

int CMPlayer::GetDuration()
{    
    if(!IsVlcLoaded())
        return 0;

	VLC_ERROR_HANDLER(VlcIntf->get_duration(hVlc), int)
        return 0;

    return ret;
}

bool CMPlayer::GetStreamCount(int* Audio, int* Video)
{
    if(!IsVlcLoaded())
        return false;

	SVlcStreamInfo info;
	memset(&info, 0, sizeof(SVlcStreamInfo));
	VLC_ERROR_HANDLER(VlcIntf->get_stream_info(hVlc, &info), int)
        return false;

    if(Audio) 
		*Audio = info.audio_streams;
    if(Video) 
		*Video = info.video_streams;

    return true;
}

bool CMPlayer::SetVisualization(const bool Show)
{
    if(!IsVlcLoaded())
        return false;

    char* Name = Show ? "goom" : NULL;

	VLC_ERROR_HANDLER(VlcIntf->set_visualization(hVlc, Name), int)
        return false;

    return true;
}

// video output

CMPlayerDisplaySize CMPlayer::GetDisplaySize()
{
    int Fullscreen, FitToScreen;
    float Zoom;

    if((Fullscreen = VlcIntf->get_fullscreen(hVlc)) < 0)
        return MPDefaultSize;

    if(Fullscreen == 1)
        return MPFullscreen;

    if((FitToScreen = VlcIntf->get_fitwindow(hVlc)) < 0)
        return MPDefaultSize;

    if(FitToScreen == 1)
        return MPFitToWindow;

    if((Zoom = VlcIntf->get_zoom(hVlc)) < 0)
        return MPDefaultSize;

    if(Zoom == 0.5) 
		return MPHalfSize;
    if(Zoom == 1)  
		return MPDefaultSize;
    if(Zoom == 2)   
		return MPDoubleSize;

    return MPDefaultSize;
}

bool CMPlayer::SetDisplaySize(CMPlayerDisplaySize Size)
{
    if(!IsVlcLoaded())
        return false;

    switch(Size)
    {
		case MPHalfSize:
			// switch to normal size
			if(VlcIntf->set_fullscreen(hVlc,0) < 0)
				return false;
			// unfit fit to window
			if(VlcIntf->set_fitwindow(hVlc,0) < 0)
				return false;
			// set zoom factor
			if(VlcIntf->set_zoom(hVlc,0.5) < 0)
				return false;
			break;

		case MPDefaultSize:
			// switch to normal size
			if(VlcIntf->set_fullscreen(hVlc,0) < 0)
				return false;
			// unfit fit to window
			if(VlcIntf->set_fitwindow(hVlc,0) < 0)
				return false;
			// set zoom factor
			if(VlcIntf->set_zoom(hVlc,1.0) < 0)
				return false;
			break;

		case MPDoubleSize:
			// switch to normal size
			if(VlcIntf->set_fullscreen(hVlc,0) < 0)
				return false;
			// unfit fit to window
			if(VlcIntf->set_fitwindow(hVlc,0) < 0)
				return false;
			// set zoom factor
			if(VlcIntf->set_zoom(hVlc,2.0) < 0)
				return false;
			break;

		case MPFitToWindow:
			// switch to normal size
			if(VlcIntf->set_fullscreen(hVlc,0) < 0)
				return false;
			// fit to window
			if(VlcIntf->set_fitwindow(hVlc,1) < 0)
				return false;
			break;

		case MPFullscreen:
			// switch to fullscreen
			if(VlcIntf->set_fullscreen(hVlc,1) < 0)
				return false;
			// fit to window
			if(VlcIntf->set_fitwindow(hVlc,1) < 0)
				return false;
			break;

		default:
			return false;
    }

    return true;
}

// audio
bool CMPlayer::SetVolume(float Volume)
{
    if(!IsVlcLoaded())
	{
	AddLogLine(false, _T("Loaded ?"));     
		return false;
	}
   // if(VlcIntf->set_volume(hVlc, Volume) < 0)
	//{
	Volume = Volume * 1,01;
		VlcIntf->set_volume(hVlc, Volume);
        return false;
	//}
	//AddLogLine(false, _T("fora ?"));
    //return true;
}

float CMPlayer::GetVolume()
{
    if(!IsVlcLoaded())
        return 0;

	const float Volume = VlcIntf->get_volume(hVlc);
	if(Volume < 0)
        return 0;

    return Volume;
}

bool CMPlayer::Mute(const bool Mute)
{
    if(!IsVlcLoaded())
        return false;

    if(VlcIntf->set_mute(hVlc, Mute) < 0)
        return false;

    return true;
}

bool CMPlayer::IsMuted()
{
    if(!IsVlcLoaded())
        return false;

	const int Mute = VlcIntf->get_mute(hVlc);
    if(Mute < 0)
        return false;

    return Mute != 0;
}

bool CMPlayer::ToggleMute()
{
    if(!IsVlcLoaded())
        return false;

    if(VlcIntf->set_mute(hVlc, -1) < 0)
        return false;

    return true;
}

// protected

bool CMPlayer::LoadVlc(const int LogLevel)
{
    if(IsVlcLoaded())
        return true;

    // load dll
	hVlcDll = LoadLibrary(SIMPLEVLC_DLL);
    if(hVlcDll != NULL)
	{
		// get entry points
	    pSVLC_GetInterface = (SVLC_GetInterface_t)GetProcAddress(hVlcDll, "SVLC_GetInterface");
	    pSVLC_GetVersion = (SVLC_GetVersion_t)GetProcAddress(hVlcDll, "SVLC_GetVersion");
	    pSVLC_Initialize = (SVLC_Initialize_t)GetProcAddress(hVlcDll, "SVLC_Initialize");
		pSVLC_Shutdown = (SVLC_Shutdown_t)GetProcAddress(hVlcDll, "SVLC_Shutdown");
	    if(pSVLC_GetInterface && pSVLC_GetVersion && pSVLC_Initialize && pSVLC_Shutdown)
		{
			// init dll
			if(pSVLC_Initialize() != -1) 
			{
				// check version
				int Major, Minor, Micro;
				pSVLC_GetVersion(&Major, &Minor, &Micro);
				if(Major > REQ_SVLC_VERSION_MAJOR || (Major == REQ_SVLC_VERSION_MAJOR && Minor >= REQ_SVLC_VERSION_MINOR))
				{
					// get interface
					VlcIntf = new SVlcInterface;
					if(pSVLC_GetInterface(VlcIntf) != -1) 
					{
						// create svlc instance
						hVlc = VlcIntf->create(LogLevel);
						if(hVlc != NULL) 
						{
							// set udata to us
							VlcIntf->set_udata(hVlc, this);

							// register callbacks
							if(VlcIntf->set_callback (hVlc, gSVlcCallbackFunc,
								/*SVLC_CB_DISPLAY_POPUP |*/
								SVLC_CB_STATE_CHANGE |
								SVLC_CB_POSITION_CHANGE /*|
								SVLC_CB_KEY_PRESSED*/) != -1)
								return true;
						}
					}
				}
				else
					theApp.QueueLogLineEx(LOG_ERROR, L"VLC: Falha ao carregar %s devido a versão errada (%u.%u.%u - preciso: %u.%u.%u)", SIMPLEVLC_DLL, Major, Minor, Micro, REQ_SVLC_VERSION_MAJOR, REQ_SVLC_VERSION_MINOR, 0);
			}
		}
	}

	CleanVariables();
	return false;
}

bool CMPlayer::UnloadVlc()
{
    if(!IsVlcLoaded())
        return true;

	CleanVariables();

    return true;
}

bool CMPlayer::IsVlcLoaded() const
{
    return (hVlcDll && hVlc && VlcIntf);
}

//void CMPlayer::WMSVlcCallback(TMessage& Msg)
//{
//    SVlcCallbackEvent event = (SVlcCallbackEvent)Msg.WParam;
//
//    switch(event) {
//    case SVLC_CB_STATE_CHANGE: {
//        // call user
//        if(FOnStateChange)
//            FOnStateChange(this, ConvertVlcState(((SVlcCbStateData*)Msg.LParam)->new_state));
//        break;
//    }
//    case SVLC_CB_POSITION_CHANGE: {
//        // call user
//        if(FOnPositionChange)
//            FOnPositionChange(this,
//                              ((SVlcCbPositionChangeData*)Msg.LParam)->position,
//                              ((SVlcCbPositionChangeData*)Msg.LParam)->duration);
//        break;
//    }
//    case SVLC_CB_DISPLAY_POPUP: {
//        // call user
//        if(FOnDisplayPopup)
//            FOnDisplayPopup(this, ((SVlcCbDisplayPopupData*)Msg.LParam)->show);
//        break;
//    }
//    case SVLC_CB_KEY_PRESSED: {
//        if(!FOnKeyPress)
//            break;
//
//        int VlcKey = ((SVlcCbKeyPressedData*)Msg.LParam)->key;
//        char Key = 0;
//        // do some converting
//        if(VlcKey & SVLC_KEY_ASCII) {
//            Key = (char)(VlcKey & SVLC_KEY_ASCII);
//        } else {
//            switch(VlcKey) {
//            case SVLC_KEY_SPACE:     Key = ' ';
//            case SVLC_KEY_ENTER:     Key = '\n';
//            case SVLC_KEY_ESC:       Key = 27;
//            case SVLC_KEY_TAB:       Key = '\t';
//            case SVLC_KEY_BACKSPACE: Key = '\r';
//            }
//        }
//        // call user
//        if(Key > 0)
//            FOnKeyPress(this, Key);
//        break;
//    }
//    };
//}

bool CMPlayer::CreateDisplayWnd()
{
    if(DisplayWnd)
        return true;

    // create display window class
    WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_NOCLOSE;// | CS_DBLCLKS;
    wc.lpfnWndProc   = (WNDPROC)gDisplayWndProc;
//    wc.cbClsExtra    = 0;
//    wc.cbWndExtra    = 0;
    wc.hInstance     = ::GetModuleHandle(NULL);
//    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
//    wc.lpszMenuName  = NULL;
    wc.lpszClassName = L"MPlayer VLC Window";
//    wc.hIconSm       = NULL;

    if(!::RegisterClassEx(&wc)) 
	{
        WNDCLASS wndclass;

		// check why it failed, if the class already exists that's fine
        if(!::GetClassInfo(::GetModuleHandle(NULL), L"MPlayer VLC Window", &wndclass))
            return false;
    }

    // create the window
    DisplayWnd = ::CreateWindow(L"MPlayer VLC Window", L"MPlayer VLC Window",
                                WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN,
                                0, 0, Width, Height,
                                Handle, // parent
                                NULL,
                                ::GetModuleHandle(NULL),
                                NULL);

    if(!DisplayWnd)
        return false;

    // set this pointer as window property. used in window proc.
    ::SetProp(DisplayWnd, L"MPlayer", (HANDLE)this);

    return true;
}

bool CMPlayer::DestroyDisplayWnd()
{
    if(!DisplayWnd)
        return true;

    // remove window property
    ::RemoveProp(DisplayWnd, L"MPlayer");

    ::DestroyWindow(DisplayWnd);
    DisplayWnd = NULL;
    return true;
}

long FAR PASCAL CMPlayer::DisplayWndProc(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam)
{
    if(hWnd != DisplayWnd)
        return ::DefWindowProc(hWnd,Msg,wParam,lParam); // shouldn't happen

    // this happens of there is no vlc window, e.g. for audio only files
//    if(Msg == WM_RBUTTONUP && FOnDisplayPopup)
//        FOnDisplayPopup(this, true);

    return ::DefWindowProc(hWnd,Msg,wParam,lParam);
}

CMPlayerState CMPlayer::ConvertVlcState(SVlcPlaybackState VlcState)
{
    switch(VlcState)
    {
		case SVLC_PLAYBACK_CLOSED:  return MPClosed;
		case SVLC_PLAYBACK_LOADING: return MPLoading;
		case SVLC_PLAYBACK_OPEN:    return MPOpen;
		case SVLC_PLAYBACK_PLAYING: return MPPlaying;
		case SVLC_PLAYBACK_PAUSED:  return MPPaused;
		case SVLC_PLAYBACK_STOPPED: return MPStopped;
		case SVLC_PLAYBACK_ERROR:   return MPError;
    }

    return MPError;
}

void CMPlayer::Resize()
{
    if(DisplayWnd)
        ::SetWindowPos(DisplayWnd, HWND_TOP, 0, 0, Width, Height, SWP_NOACTIVATE);

//    TWinControl::Resize();
}

//void CMPlayer::CreateParams(TCreateParams &Params)
//{
//    // clip children of this control
//    TWinControl::CreateParams(Params);
//    Params.Style |= WS_CLIPCHILDREN;
//}