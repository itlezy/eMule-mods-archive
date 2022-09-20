/*
This file is part of KCeasy (http://www.kceasy.com)
Copyright (C) 2002-2004 Markus Kern <mkern@kceasy.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
//---------------------------------------------------------------------------
#ifndef PlayerPageH
#define PlayerPageH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <OleCtrls.hpp>
#include <Menus.hpp>

#include "Engine.h"
#include "MPlayer.h"
//---------------------------------------------------------------------------

class TPlayerForm : public TForm
{
__published:	// IDE-managed Components
    TPanel *PlayerPanel;
    TSplitter *VSplitter;
    TPanel *DisplayPanel;
    TPanel *CtrlPanel;
    TSpeedButton *PlayBtn;
    TSpeedButton *PauseBtn;
    TSpeedButton *StopBtn;
    TStaticText *TimeStatic;
    TStaticText *TitleStatic;
    TTrackBar *SeekTrackBar;
    TPopupMenu *DisplayPopup;
    TMenuItem *PlayMnu;
    TMenuItem *PauseMnu;
    TMenuItem *StopMnu;
    TMenuItem *N1;
    TMenuItem *ZoomHalfMnu;
    TMenuItem *ZoomDefaultMnu;
    TMenuItem *ZoomDoubleMnu;
    TMenuItem *ZoomFitMnu;
    TMenuItem *ZoomMnu;
    TMenuItem *ZoomFullMnu;
    TMenuItem *N2;
    TTrackBar *VolumeTrackBar;
    TSpeedButton *MuteBtn;
    TPanel *HidePlayerPanel;
    TSpeedButton *HidePlayerBtn;
    TSpeedButton *VisualizationBtn;
    void __fastcall PlayBtnClick(TObject *Sender);
    void __fastcall PauseBtnClick(TObject *Sender);
    void __fastcall StopBtnClick(TObject *Sender);
    void __fastcall ZoomHalfMnuClick(TObject *Sender);
    void __fastcall ZoomDefaultMnuClick(TObject *Sender);
    void __fastcall ZoomDoubleMnuClick(TObject *Sender);
    void __fastcall ZoomFitMnuClick(TObject *Sender);
    void __fastcall ZoomFullMnuClick(TObject *Sender);
    void __fastcall VolumeTrackBarChange(TObject *Sender);
    void __fastcall MuteBtnClick(TObject *Sender);
    void __fastcall MPlayerStateChange(TObject* Sender, TMPlayerState NewState);
    void __fastcall MPlayerPositionChange(TObject* Sender, float Position, int Duration);
    void __fastcall MPlayerKeyPress(TObject *Sender, char &Key);
    void __fastcall MPlayerDisplayPopup(TObject* Sender, bool Show);
    void __fastcall HidePlayerBtnClick(TObject *Sender);
    void __fastcall VisualizationBtnClick(TObject *Sender);

private:	// User declarations
    bool Dragging;
    string OpenFile;

    TMPlayer *MPlayer;
    bool LoadMPlayer();

    void __fastcall (__closure *OldCtrlPanelProc)(TMessage &Msg);
    void __fastcall CtrlPanelProc(TMessage &Msg);

public:		// User declarations
    __fastcall TPlayerForm(TComponent* Owner);
    __fastcall ~TPlayerForm();
    void __fastcall Release();

    bool __fastcall EngineCallback(TCallbackInfo* CbInfo);
    bool PlayFile(const string& File);
    bool PlayStream(const string& Url);
    void StopPlayback();
    void ClosePlayback();
    const string& GetOpenFile();
    void SeekDrag(double Position);
    void SeekSet(double Position, bool Commit);
    bool SeekIsDragging();
};
//---------------------------------------------------------------------------
extern PACKAGE TPlayerForm *PlayerForm;
//---------------------------------------------------------------------------
#endif
