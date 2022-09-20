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
#include <vcl.h>
#include <KCeasy.h>
#pragma hdrstop

#include <math.h>
#include "PlayerPage.h"
#include "config.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPlayerForm *PlayerForm;
//---------------------------------------------------------------------------

__fastcall TPlayerForm::TPlayerForm(TComponent* Owner)
:   TForm(Owner),
    Dragging(false),
    MPlayer(NULL)
{
    TranslateComponent(this);

    // create MPlayer
    MPlayer = new TMPlayer(DisplayPanel);
    MPlayer->Parent = DisplayPanel;

    // register events
    MPlayer->OnKeyPress = MPlayerKeyPress;
    MPlayer->OnStateChange = MPlayerStateChange;
    MPlayer->OnPositionChange = MPlayerPositionChange;
    MPlayer->OnDisplayPopup = MPlayerDisplayPopup;

    // used to intercept WM_HSCROLL of track bar
    OldCtrlPanelProc = CtrlPanel->WindowProc;
    CtrlPanel->WindowProc = CtrlPanelProc;

    // disable all UI elemets
    ClosePlayback();
    VolumeTrackBar->Enabled = false;
    VolumeTrackBar->Position = 0;
    MuteBtn->Enabled = false;
    VisualizationBtn->Enabled = false;
    ZoomMnu->Enabled = false;
}

__fastcall TPlayerForm::~TPlayerForm()
{
    delete MPlayer;
}

void __fastcall TPlayerForm::Release()
{
    // save volume and mute
    if(MPlayer && MPlayer->IsInitialized())
        Config->SetValueInt("player/volume", MPlayer->GetVolume() * 100);
    if(MPlayer && MPlayer->IsInitialized())
        Config->SetValueInt("player/mute", MPlayer->IsMuted());

    TForm::Release();
}
//---------------------------------------------------------------------------

bool __fastcall TPlayerForm::EngineCallback(TCallbackInfo* CbInfo)
{
    switch(CbInfo->Code) {
    case CbcStateChange:
        if (Engine->IsOffline()) {
            MPlayer->Stop();
        }
    }
    return false;
}
//---------------------------------------------------------------------------

bool TPlayerForm::LoadMPlayer()
{
    if(MPlayer->IsInitialized())
        return true;

    if(!MPlayer->Initialize(Config->GetValueInt("player/vlc_logging"))) {
        // TRANSLATOR: Error shown on player page when VLC cannot be started.
        DisplayPanel->Caption = _("The Media Player (VLC) could not be started.\n"
                                  "This is most likely because you didn't install it with {AppName}.\n"
                                  "To fix this download the full installer and reinstall {AppName}.");
        MPlayer->Visible = false;
        // show error box
        MessageDlg(DisplayPanel->Caption.c_str(),mtError,TMsgDlgButtons() << mbOK,0);
        return false;
    }
    // set saved volume and mute
    MPlayer->SetVolume((double)Config->GetValueInt("player/volume") / 100);
    MPlayer->Mute(Config->GetValueInt("player/mute"));

    // set volume slider to current volume
    VolumeTrackBar->Enabled = true;
    VolumeTrackBar->Position = (double)VolumeTrackBar->Max * MPlayer->GetVolume();
    MuteBtn->Enabled = true;
    MuteBtn->Down = MPlayer->IsMuted();
    VisualizationBtn->Enabled = true;
    VisualizationBtn->Down = Config->GetValueInt("player/visualization");
    ZoomMnu->Enabled = true;

    return true;
}
//---------------------------------------------------------------------------

bool TPlayerForm::PlayFile(const string& File)
{
    if(!LoadMPlayer())
        return false;

    MPlayer->Stop();
    OpenFile = File;

    if(!MPlayer->Play(OpenFile.c_str())) {
        // TRANSLATOR: Message box when file cannot be played in integrated player.
        AnsiString Error = AnsiString::Format(_("Unable to play file \"%s\""),
                           ARRAYOFCONST((OpenFile.c_str())));
        MessageDlg(Error,mtError,TMsgDlgButtons() << mbOK,0);
        ClosePlayback();
        return false;
    }

    return true;
}

bool TPlayerForm::PlayStream(const string& Url)
{
    if(!LoadMPlayer())
        return false;

    MPlayer->Stop();
    OpenFile = Url;

    if(!MPlayer->PlayNetwork(OpenFile.c_str())) {
        // TRANSLATOR: Message box when file cannot be streamed in integrated player.
        AnsiString Error = AnsiString::Format(_("Unable to stream file \"%s\""),
                           ARRAYOFCONST((OpenFile.c_str())));
        MessageDlg(Error,mtError,TMsgDlgButtons() << mbOK,0);
        ClosePlayback();
        return false;
    }

    return true;
}

void TPlayerForm::StopPlayback()
{
    MPlayer->Stop();
}

void TPlayerForm::ClosePlayback()
{
    MPlayer->Stop();

    OpenFile = "";

    Dragging = false;
    MPlayerPositionChange(NULL,0,0);
    
    PlayBtn->Enabled = false;
    PauseBtn->Enabled = false;
    StopBtn->Enabled = false;
    TimeStatic->Caption = "00:00";
    TitleStatic->Caption = "";
    SeekTrackBar->Enabled = false;
    // main form
    MainForm->PlayBtn->Enabled = false;
    MainForm->PauseBtn->Enabled = false;
    MainForm->StopBtn->Enabled = false;
    MainForm->TimeLabel->Caption = "";
    MainForm->TitleLabel->Caption = "";
    MainForm->PlayProgressBar->Enabled = false;
    // display popup
    PlayMnu->Enabled = false;
    PauseMnu->Enabled = false;
    StopMnu->Enabled = false;
}

const string& TPlayerForm::GetOpenFile()
{
    return OpenFile;
}

void TPlayerForm::SeekDrag(double Position)
{
    char buf[64];
    unsigned int LenSec = MPlayer->GetDuration() / 1000;
    double PosSec = Position * LenSec;

    Dragging = true;
    // CtrlPanel
    wsprintf(buf,"%02d:%02d",(unsigned int)PosSec/60,((unsigned int)PosSec)%60);
    TimeStatic->Caption = buf;
    SeekTrackBar->Position = Position * SeekTrackBar->Max;
    // MainForm
    wsprintf(buf,"%02d:%02d / %02d:%02d",(unsigned int)PosSec/60,((unsigned int)PosSec)%60,(unsigned int)LenSec/60,((unsigned int)LenSec)%60);
    MainForm->TimeLabel->Caption = buf;
    MainForm->PlayProgressBar->Position = Position * MainForm->PlayProgressBar->Max;
}

void TPlayerForm::SeekSet(double Position, bool Commit)
{
    if(Commit) {
        // CtrlPanel
        SeekTrackBar->Position = Position * SeekTrackBar->Max;
        SeekTrackBar->SelEnd = SeekTrackBar->Position;
        // MainForm
        MainForm->PlayProgressBar->Position = Position * MainForm->PlayProgressBar->Max;
        MPlayer->SetPosition(Position);
    }

    Dragging = false;
}

bool TPlayerForm::SeekIsDragging()
{
    return Dragging;
}

//---------------------------------------------------------------------------

void __fastcall TPlayerForm::MPlayerStateChange(TObject* Sender,
      TMPlayerState NewState)
{
    switch(NewState) {
    case MPLoading:
        PlayBtn->Enabled = false;
        PauseBtn->Enabled = false;
        StopBtn->Enabled = true;
        TitleStatic->Caption = FileFromPath(OpenFile).c_str();
        MainForm->TitleLabel->Caption = FileFromPath(OpenFile).c_str();
        // TRANSLATOR: Player status when loading file for playback.
        MainForm->TimeLabel->Caption = _("Loading...");
        break;
    case MPOpen: {
        PlayBtn->Enabled = false;
        PauseBtn->Enabled = false;
        StopBtn->Enabled = true;
        SeekTrackBar->Enabled = true;
        // reset position
        MPlayerPositionChange(NULL,0,0);
        // set visualization if wanted
        int VideoStreams;
        if(MPlayer->GetStreamCount(NULL,&VideoStreams) && VideoStreams == 0) {
            if(Config->GetValueInt("player/visualization")) {
                MPlayer->SetVisualization(true);
                MPlayer->SetDisplaySize(MPFitToWindow);
            }
        }
        break;
    }
    case MPPlaying:
        PlayBtn->Enabled = false;
        PauseBtn->Enabled = true;
        StopBtn->Enabled = true;
        break;
    case MPPaused:
        PlayBtn->Enabled = true;
        PauseBtn->Enabled = false;
        StopBtn->Enabled = true;
        break;
    case MPClosed:
    case MPStopped:
        PlayBtn->Enabled = true;
        PauseBtn->Enabled = false;
        StopBtn->Enabled = false;
        SeekTrackBar->Enabled = false;
        // reset position
        MPlayerPositionChange(NULL,0,MPlayer->GetDuration());
        MPlayer->SetVisualization(false);
        break;
    case MPError:
        // TRANSLATOR: Message box when file cannot be played in integrated player.
        AnsiString Error = AnsiString::Format(_("Unable to play file \"%s\"\n") + 
                           _("Either the file could not be opened or the codecs are not supported."),
                           ARRAYOFCONST((OpenFile.c_str())));
        MessageDlg(Error,mtError,TMsgDlgButtons() << mbOK,0);
        ClosePlayback();
        MPlayer->SetVisualization(false);
        return;
    }
    // main from
    MainForm->PlayBtn->Enabled = PlayBtn->Enabled;
    MainForm->PauseBtn->Enabled = PauseBtn->Enabled;
    MainForm->StopBtn->Enabled = StopBtn->Enabled;
    MainForm->PlayProgressBar->Enabled = SeekTrackBar->Enabled;
    // display popup
    PlayMnu->Enabled = PlayBtn->Enabled;
    PauseMnu->Enabled = PauseBtn->Enabled;
    StopMnu->Enabled = StopBtn->Enabled;
}

void __fastcall TPlayerForm::MPlayerPositionChange(TObject* Sender, float Position, int Duration)
{
    double Length = Duration / 1000;
    double Pos = Position;
    double PosSec = Pos * Length;
    char buf[64];
    // CtrlPanel
    SeekTrackBar->SelEnd = Pos * SeekTrackBar->Max;
    if(!Dragging) {
        // CtrlPanel
        SeekTrackBar->Position = Pos * SeekTrackBar->Max;
        wsprintf(buf,"%02d:%02d",(unsigned int)PosSec/60,((unsigned int)PosSec)%60);
        TimeStatic->Caption = buf;
        // MainForm
        wsprintf(buf,"%02d:%02d / %02d:%02d",(unsigned int)PosSec/60,((unsigned int)PosSec)%60,(unsigned int)Length/60,((unsigned int)Length)%60);
        MainForm->TimeLabel->Caption = buf;
        MainForm->PlayProgressBar->Position = Pos * MainForm->PlayProgressBar->Max;
    }
}
//---------------------------------------------------------------------------

void __fastcall TPlayerForm::MPlayerKeyPress(TObject *Sender, char &Key)
{
    switch(Key) {
    case 27: // escape
        if(MPlayer->GetDisplaySize() == MPFullscreen)
            MPlayer->SetDisplaySize(MPFitToWindow);
        break;
    case ' ':
        MPlayer->TogglePause();
        break;
    }
}

void __fastcall TPlayerForm::MPlayerDisplayPopup(TObject* Sender, bool Show)
{
    if(!Show)
        return;

    switch(MPlayer->GetDisplaySize()) {
    case MPHalfSize:    ZoomHalfMnu->Checked = true;    break;
    case MPDefaultSize: ZoomDefaultMnu->Checked = true; break;
    case MPDoubleSize:  ZoomDoubleMnu->Checked = true;  break;
    case MPFitToWindow: ZoomFitMnu->Checked = true;     break;
    case MPFullscreen:  ZoomFullMnu->Checked = true;    break;
    }

    POINT Point;
    ::GetCursorPos(&Point);
    ::ShowCursor(TRUE);
    DisplayPopup->Popup(Point.x,Point.y);
}
//---------------------------------------------------------------------------

void __fastcall TPlayerForm::CtrlPanelProc(TMessage &Msg)
{
    if(Msg.Msg == WM_HSCROLL && (HWND)Msg.LParam == SeekTrackBar->Handle) {
        switch(LOWORD(Msg.WParam)) {
        case TB_PAGEUP:
        case TB_PAGEDOWN:
            Msg.WParam = (Msg.WParam & 0xFFFF) | (::SendMessage(SeekTrackBar->Handle,TBM_GETPOS,0,0) << 16);
        // fall through
        case TB_THUMBTRACK: {
            SeekDrag((double)HIWORD(Msg.WParam) / SeekTrackBar->Max);
            break;
        }
        case TB_ENDTRACK:
            Msg.WParam = (Msg.WParam & 0xFFFF) | (::SendMessage(SeekTrackBar->Handle,TBM_GETPOS,0,0) << 16);
            // fall through
        case TB_THUMBPOSITION:
            SeekSet((double)HIWORD(Msg.WParam) / SeekTrackBar->Max, true);
            break;
        }
    }
    OldCtrlPanelProc(Msg);
}
//---------------------------------------------------------------------------

void __fastcall TPlayerForm::PlayBtnClick(TObject *Sender)
{
    if(MPlayer->GetState() == MPPaused) {
        MPlayer->Pause(false);
    } else {
        if(!MPlayer->Play(OpenFile.c_str()))
            ClosePlayback();
    }
}

void __fastcall TPlayerForm::PauseBtnClick(TObject *Sender)
{
    MPlayer->Pause(true);
}

void __fastcall TPlayerForm::StopBtnClick(TObject *Sender)
{
    MPlayer->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TPlayerForm::VisualizationBtnClick(TObject *Sender)
{
    Config->SetValueInt("player/visualization",VisualizationBtn->Down);
    // set visualization if audio file is playing
    int VideoStreams;
    if(MPlayer->GetStreamCount(NULL,&VideoStreams) && VideoStreams == 0) {
        if(Config->GetValueInt("player/visualization")) {
            MPlayer->SetVisualization(true);
            MPlayer->SetDisplaySize(MPFitToWindow);
        } else {
            MPlayer->SetVisualization(false);
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TPlayerForm::MuteBtnClick(TObject *Sender)
{
    MPlayer->Mute(MuteBtn->Down);
}

void __fastcall TPlayerForm::VolumeTrackBarChange(TObject *Sender)
{
    MPlayer->SetVolume((double)VolumeTrackBar->Position / VolumeTrackBar->Max);
}
//---------------------------------------------------------------------------

void __fastcall TPlayerForm::ZoomHalfMnuClick(TObject *Sender)
{
    MPlayer->SetDisplaySize(MPHalfSize);
}

void __fastcall TPlayerForm::ZoomDefaultMnuClick(TObject *Sender)
{
    MPlayer->SetDisplaySize(MPDefaultSize);
}

void __fastcall TPlayerForm::ZoomDoubleMnuClick(TObject *Sender)
{
    MPlayer->SetDisplaySize(MPDoubleSize);
}

void __fastcall TPlayerForm::ZoomFitMnuClick(TObject *Sender)
{
    MPlayer->SetDisplaySize(MPFitToWindow);
}

void __fastcall TPlayerForm::ZoomFullMnuClick(TObject *Sender)
{
    MPlayer->SetDisplaySize(MPFullscreen);
}
//---------------------------------------------------------------------------

void __fastcall TPlayerForm::HidePlayerBtnClick(TObject *Sender)
{
    if(PlayerPanel->Visible) {
        VSplitter->Visible = false;
        PlayerPanel->Visible = false;
        HidePlayerBtn->Glyph->LoadFromResourceName((unsigned int)HInstance,"SLIDE_LEFT_BMP");
    } else {
        // order panels correctly
        VSplitter->Left = 0;
        PlayerPanel->Left = 1;

        PlayerPanel->Visible = true;
        VSplitter->Visible = true;
        HidePlayerBtn->Glyph->LoadFromResourceName((unsigned int)HInstance,"SLIDE_RIGHT_BMP");
    }
}
//---------------------------------------------------------------------------

