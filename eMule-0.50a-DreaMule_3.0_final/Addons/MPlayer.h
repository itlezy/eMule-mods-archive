#pragma once

#include "simplevlc.h"

static const UINT WM_SVLC_CALLBACK = WM_USER + 1;

typedef enum
{
	MPClosed,
	MPLoading,
	MPOpen,
	MPPlaying,
	MPPaused,
	MPStopped,
    MPError
} CMPlayerState;

typedef enum
{
    MPHalfSize = 0,
    MPDefaultSize,
    MPDoubleSize,
    MPFitToWindow,
    MPFullscreen
} CMPlayerDisplaySize;

//typedef void (/*__closure*/ *CMPlayerStateChangeEvent)(TObject* Sender, CMPlayerState NewState);
//typedef void (/*__closure*/ *CMPlayerPositionChangeEvent)(TObject* Sender, float Position, int Duration);
//typedef void (/*__closure*/ *CMPlayerDisplayPopupEvent)(TObject* Sender, bool Show);

class CMPlayer /*: public TWinControl*/
{
friend static long FAR PASCAL gDisplayWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
friend static void SVLC_CC gSVlcCallbackFunc (HSVLC svlc, SVlcCallbackEvent event, void *edata, void *udata);
//__published:
//    __property OnKeyPress = {read=FOnKeyPress, write=FOnKeyPress, default=NULL};
//    __property CMPlayerStateChangeEvent OnStateChange = {read=FOnStateChange, write=FOnStateChange, default=NULL};
//    __property CMPlayerPositionChangeEvent OnPositionChange = {read=FOnPositionChange, write=FOnPositionChange, default=NULL};
//    __property CMPlayerDisplayPopupEvent OnDisplayPopup = {read=FOnDisplayPopup, write=FOnDisplayPopup, default=NULL};

public:
//  virtual CMPlayer(TComponent* Owner);
	CMPlayer();
    virtual ~CMPlayer();

//obviously those 3 should be inherited from TWinControl...
	HWND Handle;	//handle to parent
	int Width;		//parent width
	int Height;		//parent height

    // don't call this before the CMPlayer window is fully created
    bool	Initialize(const int LogLevel = -1);
    bool Shutdown();
    bool IsInitialized() const	{ return IsVlcLoaded(); }
    void	Resize();

    // playback
    bool Play(CString Filename);
    bool PlayNetwork(CString Url);
    bool Stop();
    bool	Pause(const bool Pause);
    bool TogglePause();
    bool IsPaused()				{ return (GetState() == MPPaused); }
    CMPlayerState GetState();

    bool IsSeekable();
    bool	SetPosition(const float Position);
    float GetPosition();	// in [0,1]
    int GetDuration();		// in msec

    bool GetStreamCount(int* Audio, int* Video);
    bool	SetVisualization(const bool Show);

    // video
    CMPlayerDisplaySize GetDisplaySize();
    bool	SetDisplaySize(CMPlayerDisplaySize Size);

    // audio
    bool	SetVolume(const float Volume);
    float GetVolume(); // in [0,1]
    bool	Mute(const bool Mute);
    bool IsMuted();
    bool ToggleMute();

private:
	void	CleanVariables(const bool bInit = false);
	SVLC_GetInterface_t pSVLC_GetInterface;
	SVLC_GetVersion_t pSVLC_GetVersion;
	SVLC_Initialize_t pSVLC_Initialize;
	SVLC_Shutdown_t pSVLC_Shutdown;

protected:
//    CMPlayerStateChangeEvent FOnStateChange;
//    CMPlayerPositionChangeEvent FOnPositionChange;
//    CMPlayerDisplayPopupEvent FOnDisplayPopup;
//    TKeyPressEvent FOnKeyPress;

    HINSTANCE hVlcDll;
    SVlcInterface *VlcIntf;
    HSVLC hVlc;
    HWND DisplayWnd;

    bool	LoadVlc(const int LogLevel = -1);
    bool UnloadVlc();
    bool IsVlcLoaded() const;

//    void WMSVlcCallback(TMessage& Msg);

    bool CreateDisplayWnd();
    bool DestroyDisplayWnd();
    long FAR PASCAL DisplayWndProc(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam);

    CMPlayerState ConvertVlcState(SVlcPlaybackState VlcState);

//    virtual void CreateParams(TCreateParams &Params);

//BEGIN_MESSAGE_MAP
//  VCL_MESSAGE_HANDLER(WM_SVLC_CALLBACK, TMessage, WMSVlcCallback)
//END_MESSAGE_MAP(TWinControl)
};