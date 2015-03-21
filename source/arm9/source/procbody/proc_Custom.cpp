
#pragma Ospace

#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"

#include "procstate.h"
#include "datetime.h"

#include "glib.h"

#include "fat2.h"
#include "shell.h"

#include "skin.h"
#include "component.h"
#include "sndeff.h"
#include "lang.h"
#include "strpcm.h"
#include "rect.h"

#include "BootROM.h"

DATA_IN_IWRAM_Custom static TProcState *pCurrentProcState;

#define ClientHeight (192*2+32)

// -----------------------------

DATA_IN_IWRAM_Custom static CglCanvas *pScreenBack;
DATA_IN_IWRAM_Custom static s32 ScreenPosY,ShowPosY;

static void ScrollBar_SetPosY(s32 PosY)
{
  ScreenPosY=PosY;
  if(ScreenPosY<0) ScreenPosY=0;
  if((ClientHeight-ScreenHeight)<ScreenPosY) ScreenPosY=ClientHeight-ScreenHeight;
}

// -----------------------------

enum ECompLabels {ECLS_TitleLbl,ECLS_PageTab_TitleLbl,ECLS_LRKeyLockTitleLbl,ECLS_HideAttribute_TitleLbl,ECLS_ScreenSaverTitleLbl,ECLS_ScreenSaverBGTitleLbl,ECLS_ScreenSaverTimeOutSecTitleLbl,ECLS_PlayListEndTitleLbl,ECLSCount};
#define CompLabelsCount (ECLSCount)
DATA_IN_IWRAM_Custom static TComponentLabel CompLabels[CompLabelsCount];

enum ECompChecks {ECCS_PageTab_GlobalRadio,ECCS_PageTab_FileListRadio,ECCS_PageTab_ScreenSaverRadio,ECCS_PageTab_MusicRadio,
                  
                  ECCSG_SkipSetupChk,ECCSG_BootCheckDiskChk,
                  ECCSG_ClickSoundChk,ECCSG_EnableFadeEffectChk,ECCSG_AutoLastStateChk,
                  ECCSG_LRKeyLockAlwayOffRadio,ECCSG_LRKeyLockRelationalPanelRadio,ECCSG_LRKeyLockAlwayOnRadio,
                  ECCSG_EnableResumeFunctionChk,ECCSG_EnableScreenCaptureChk,ECCSG_SpeakerPowerOffWhenPanelClosedChk,
                  
                  ECCSF_MoveFolderLockedChk,ECCSF_HiddenFilenameExtChk,ECCSF_HiddenNotSupportFileTypeChk,ECCSF_HiddenNDSROMFileChk,
                  ECCSF_EnableFileInfoChk,ECCSF_ShowOnlineHelp_LRButtonChk,ECCSF_BButtonToFolderUpChk,ECCSF_EasyDeleteKeyChk,
                  ECCSF_UseJpegExifThumbnailChk,ECCSF_ShowCoverImageChk,
                  ECCSF_HideAttribute_ArchiveChk,ECCSF_HideAttribute_HiddenChk,ECCSF_HideAttribute_SystemChk,ECCSF_HideAttribute_ReadOnlyChk,
                  
                  ECCSS_EnabledScreenSaverChk,ECCSS_Use24hFormatChk,
                  ECCSS_ScreenSaverNormalRadio,ECCSS_ScreenSaverDigitalRadio,ECCSS_ScreenSaverExtendRadio,
                  ECCSS_ScreenSaverBGSkinDefaultRadio,ECCSS_ScreenSaverBGTopScreenRadio,ECCSS_ScreenSaverBGBottomScreenRadio,
                  ECCSS_ScreenSaverTimeOutSec5secRadio,ECCSS_ScreenSaverTimeOutSec10secRadio,ECCSS_ScreenSaverTimeOutSec30secRadio,ECCSS_ScreenSaverTimeOutSec60secRadio,
                  ECCSS_ScreenSaverTimeOutSecOffChk,ECCSS_ShowClockChk,ECCSS_ShowID3TagChk,ECCSS_ShowLyricChk,ECCSS_EnabledTouchPanelCombinationChk,ECCSS_HideScreenSaverCustomChk,
                  
                  ECCSM_PlayListEndLoopRadio,ECCSM_PlayListEndStopRadio,ECCSM_PlayListEndPowerOffRadio,
                  
                  ECCSCount};
#define CompChecksCount (ECCSCount)
DATA_IN_IWRAM_Custom static TComponentCheck CompChecks[CompChecksCount];

enum ECompButtons {ECBS_CancelBtn,ECBS_OkBtn,ECBSCount};
#define CompButtonsCount (ECBSCount)
DATA_IN_IWRAM_Custom static TComponentButton CompButtons[CompButtonsCount];

static void Setting_Redraw_TransOnly(void)
{
  s32 posy=ShowPosY;
  
  if(posy<ScreenHeight){
    CglB15 *pb15=Custom_GetSkin(ECS_TopMsg);
    pb15->BitBlt(pScreenSub->pCanvas,0,0,ScreenWidth,ScreenHeight-posy,0,0);
  }
  
  pScreenBack->BitBlt(pScreenSub->pCanvas,0,ScreenHeight-posy,ScreenWidth,posy,0,0,false);
  
  pScreenBack->BitBlt(pScreenMain->pBackCanvas,0,0,ScreenWidth,ScreenHeight,0,posy,false);
  
  {
    CglCanvas *pcan=pScreenMain->pBackCanvas;
    const u16 FrameColor=ColorTable.Component.SoftwareScrollBar_Frame;
    const u16 BGColor=ColorTable.Component.SoftwareScrollBar_BG;
    
    s32 w=10,h=(ScreenHeight*ScreenHeight)/ClientHeight;
    s32 x=ScreenWidth-2-w,y=posy/2;
    if(y>ScreenHeight-h) y=ScreenHeight-h;
    pcan->SetColor(FrameColor);
    pcan->DrawBox(x,y,w,h);
    
    x+=1; y+=1; w-=2; h-=2;
    u16 *pbuf=pcan->GetScanLine(y);
    pbuf+=x;
    u32 bufsize=pcan->GetWidth()-w;
    for(s32 py=0;py<h;py++){
      for(s32 px=0;px<w;px++){
        u16 col=*pbuf;
        u16 mask=RGB15(30,30,30);
        col=(col&mask)>>1;
        *pbuf++=(BGColor+col)|BIT15;
      }
      pbuf+=bufsize;
    }
  }
  
  ScreenMain_Flip_ProcFadeEffect();
}

static void Setting_Redraw(void)
{
  {
    CglB15 *pb15=Custom_GetSkin(ECS_BG);
    pb15->pCanvas->BitBltFullBeta(pScreenBack);
    pScreenBack->SetColor(RGB15(14,14,15) | BIT(15));
    pScreenBack->DrawLine(1,81,ScreenWidth,81);
  }
  
  for(u32 idx=0;idx<CompChecksCount;idx++){
    TComponentCheck *pcc=&CompChecks[idx];
    bool *pchk=(bool*)pcc->UserData;
    if(pchk!=NULL) pcc->Checked=*pchk;
  }
  
  {
      CompChecks[ECCS_PageTab_GlobalRadio].Checked=false;
      CompChecks[ECCS_PageTab_FileListRadio].Checked=false;
      CompChecks[ECCS_PageTab_ScreenSaverRadio].Checked=false;
      CompChecks[ECCS_PageTab_MusicRadio].Checked=false;
      
      switch(pCurrentProcState->System.LastPageTab){
        case ELPT_Global: CompChecks[ECCS_PageTab_GlobalRadio].Checked=true; break;
        case ELPT_FileList: CompChecks[ECCS_PageTab_FileListRadio].Checked=true; break;
        case ELPT_ScreenSaver: CompChecks[ECCS_PageTab_ScreenSaverRadio].Checked=true; break;
        case ELPT_Music: CompChecks[ECCS_PageTab_MusicRadio].Checked=true; break;
        default: break;
      }
  }
  
  {
      CompChecks[ECCSG_LRKeyLockAlwayOffRadio].Checked=false;
      CompChecks[ECCSG_LRKeyLockRelationalPanelRadio].Checked=false;
      CompChecks[ECCSG_LRKeyLockAlwayOnRadio].Checked=false;
        
      switch(pCurrentProcState->System.LRKeyLockType){
          case ELRLT_AlwayOff: CompChecks[ECCSG_LRKeyLockAlwayOffRadio].Checked=true; break;
          case ELRLT_RelationalPanel: CompChecks[ECCSG_LRKeyLockRelationalPanelRadio].Checked=true; break;
          case ELRLT_AlwayOn: CompChecks[ECCSG_LRKeyLockAlwayOnRadio].Checked=true; break;
          default: break;
      }
  }
  
  {
      CompChecks[ECCSS_ScreenSaverNormalRadio].Checked=false;
      CompChecks[ECCSS_ScreenSaverDigitalRadio].Checked=false;
      CompChecks[ECCSS_ScreenSaverExtendRadio].Checked=false;
      
      switch(pCurrentProcState->ScreenSaver.ScreenSaver){
        case EPSSS_Normal: CompChecks[ECCSS_ScreenSaverNormalRadio].Checked=true; break;
        case EPSSS_Digital: CompChecks[ECCSS_ScreenSaverDigitalRadio].Checked=true; break;
        case EPSSS_Extend: CompChecks[ECCSS_ScreenSaverExtendRadio].Checked=true; break;
        default: break;
      }
  }
  
  {
      CompChecks[ECCSS_ScreenSaverBGSkinDefaultRadio].Checked=false;
      CompChecks[ECCSS_ScreenSaverBGTopScreenRadio].Checked=false;
      CompChecks[ECCSS_ScreenSaverBGBottomScreenRadio].Checked=false;
        
      switch(pCurrentProcState->ScreenSaver.ScreenSaverBG){
          case EPSSSBG_SkinDefault: CompChecks[ECCSS_ScreenSaverBGSkinDefaultRadio].Checked=true; break;
          case EPSSSBG_TopScreen: CompChecks[ECCSS_ScreenSaverBGTopScreenRadio].Checked=true; break;
          case EPSSSBG_BottomScreen: CompChecks[ECCSS_ScreenSaverBGBottomScreenRadio].Checked=true; break;
          default: break;
      }
  }
  
  {
      CompChecks[ECCSS_ScreenSaverTimeOutSec5secRadio].Checked=false;
      CompChecks[ECCSS_ScreenSaverTimeOutSec10secRadio].Checked=false;
      CompChecks[ECCSS_ScreenSaverTimeOutSec30secRadio].Checked=false;
      CompChecks[ECCSS_ScreenSaverTimeOutSec60secRadio].Checked=false;
            
      switch(pCurrentProcState->ScreenSaver.BacklightTimeoutSec){
          case 5: CompChecks[ECCSS_ScreenSaverTimeOutSec5secRadio].Checked=true; break;
          case 10: CompChecks[ECCSS_ScreenSaverTimeOutSec10secRadio].Checked=true; break;
          case 30: CompChecks[ECCSS_ScreenSaverTimeOutSec30secRadio].Checked=true; break;
          case 60: CompChecks[ECCSS_ScreenSaverTimeOutSec60secRadio].Checked=true; break;
          default: break;
      }
  }
  
  {
      CompChecks[ECCSM_PlayListEndLoopRadio].Checked=false;
      CompChecks[ECCSM_PlayListEndStopRadio].Checked=false;
      CompChecks[ECCSM_PlayListEndPowerOffRadio].Checked=false;
          
      switch(pCurrentProcState->Music.PlayListEnd){
          case EPLE_Loop: CompChecks[ECCSM_PlayListEndLoopRadio].Checked=true; break;
          case EPLE_Stop: CompChecks[ECCSM_PlayListEndStopRadio].Checked=true; break;
          case EPLE_Off: CompChecks[ECCSM_PlayListEndPowerOffRadio].Checked=true; break;
          default: break;
      }
  }
  
  for(u32 idx=0;idx<CompLabelsCount;idx++){
    ComponentLabel_Draw(&CompLabels[idx]);
  }
  for(u32 idx=0;idx<CompChecksCount;idx++){
    ComponentCheck_Draw(&CompChecks[idx]);
  }
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    ComponentButton_Draw(&CompButtons[idx]);
  }
  
  Setting_Redraw_TransOnly();
}

static void CB_PageTab_GlobalRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->System.LastPageTab=ELPT_Global;
  CompLabels[ECLS_LRKeyLockTitleLbl].Visible=true;
  CompLabels[ECLS_HideAttribute_TitleLbl].Visible=false;
  CompLabels[ECLS_ScreenSaverTitleLbl].Visible=false;
  CompLabels[ECLS_ScreenSaverBGTitleLbl].Visible=false;
  CompLabels[ECLS_ScreenSaverTimeOutSecTitleLbl].Visible=false;
  CompLabels[ECLS_PlayListEndTitleLbl].Visible=false;
  
  for(u32 idx=4;idx<CompChecksCount;idx++){
      TComponentCheck *pcc=&CompChecks[idx];
      if((3<idx)&&(idx<15)) pcc->Visible=true;
      else pcc->Visible=false;
  }
  Setting_Redraw();
}
static void CB_PageTab_FileListRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->System.LastPageTab=ELPT_FileList;
  CompLabels[ECLS_LRKeyLockTitleLbl].Visible=false;
  CompLabels[ECLS_HideAttribute_TitleLbl].Visible=true;
  CompLabels[ECLS_ScreenSaverTitleLbl].Visible=false;
  CompLabels[ECLS_ScreenSaverBGTitleLbl].Visible=false;
  CompLabels[ECLS_ScreenSaverTimeOutSecTitleLbl].Visible=false;
  CompLabels[ECLS_PlayListEndTitleLbl].Visible=false;
  
  for(u32 idx=4;idx<CompChecksCount;idx++){
      TComponentCheck *pcc=&CompChecks[idx];
      if((14<idx)&&(idx<29)) pcc->Visible=true;
      else pcc->Visible=false;
  }
  Setting_Redraw();
}
static void CB_PageTab_ScreenSaverRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->System.LastPageTab=ELPT_ScreenSaver;
  CompLabels[ECLS_LRKeyLockTitleLbl].Visible=false;
  CompLabels[ECLS_HideAttribute_TitleLbl].Visible=false;
  CompLabels[ECLS_ScreenSaverTitleLbl].Visible=true;
  CompLabels[ECLS_ScreenSaverBGTitleLbl].Visible=true;
  CompLabels[ECLS_ScreenSaverTimeOutSecTitleLbl].Visible=true;
  CompLabels[ECLS_PlayListEndTitleLbl].Visible=false;
    
  for(u32 idx=4;idx<CompChecksCount;idx++){
      TComponentCheck *pcc=&CompChecks[idx];
      if((28<idx)&&(idx<47)) pcc->Visible=true;
      else pcc->Visible=false;
  }
  Setting_Redraw();
}
static void CB_PageTab_MusicRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->System.LastPageTab=ELPT_Music;
  CompLabels[ECLS_LRKeyLockTitleLbl].Visible=false;
  CompLabels[ECLS_HideAttribute_TitleLbl].Visible=false;
  CompLabels[ECLS_ScreenSaverTitleLbl].Visible=false;
  CompLabels[ECLS_ScreenSaverBGTitleLbl].Visible=false;
  CompLabels[ECLS_ScreenSaverTimeOutSecTitleLbl].Visible=false;
  CompLabels[ECLS_PlayListEndTitleLbl].Visible=true;
  
  for(u32 idx=4;idx<CompChecksCount;idx++){
      TComponentCheck *pcc=&CompChecks[idx];
      if((46<idx)&&(idx<50)) pcc->Visible=true;
      else pcc->Visible=false;
  }
  Setting_Redraw();
}

static void CB_LRKeyLockAlwayOffRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->System.LRKeyLockType=ELRLT_AlwayOff;
}
static void CB_LRKeyLockRelationalPanelRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->System.LRKeyLockType=ELRLT_RelationalPanel;
}
static void CB_LRKeyLockAlwayOnRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->System.LRKeyLockType=ELRLT_AlwayOn;
}
static void CB_ScreenSaverNormalRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.ScreenSaver=EPSSS_Normal;
}
static void CB_ScreenSaverDigitalRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.ScreenSaver=EPSSS_Digital;
}
static void CB_ScreenSaverExtendRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.ScreenSaver=EPSSS_Extend;
}
static void CB_ScreenSaverBGSkinDefaultRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.ScreenSaverBG=EPSSSBG_SkinDefault;
}
static void CB_ScreenSaverBGTopScreenRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.ScreenSaverBG=EPSSSBG_TopScreen;
}
static void CB_ScreenSaverBGBottomScreenRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.ScreenSaverBG=EPSSSBG_BottomScreen;
}
static void CB_ScreenSaverTimeOutSec5secRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.BacklightTimeoutSec=5;
}
static void CB_ScreenSaverTimeOutSec10secRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.BacklightTimeoutSec=10;
}
static void CB_ScreenSaverTimeOutSec30secRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.BacklightTimeoutSec=30;
}
static void CB_ScreenSaverTimeOutSec60secRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->ScreenSaver.BacklightTimeoutSec=60;
}
static void CB_PlayListEndLoopRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Music.PlayListEnd=EPLE_Loop;
}
static void CB_PlayListEndStopRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Music.PlayListEnd=EPLE_Stop;
}
static void CB_PlayListEndPowerOffRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Music.PlayListEnd=EPLE_Off;
}

static void CB_CancelBtn_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  SetNextProc(ENP_FileList,EPFE_LeftToRight);
}

static void CB_OkBtn_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  
  ProcState=*pCurrentProcState;
  ProcState_RequestSave=true;
  ProcState_Save();
  
  Date_Set24hFormat(pCurrentProcState->ScreenSaver.Use24hFormat);
  
  SetNextProc(ENP_FileList,EPFE_LeftToRight);
}

static void CB_CheckBox_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  TComponentCheck *pcc=(TComponentCheck*)pComponentButton;
  bool *pchk=(bool*)pcc->UserData;
  if(pchk!=NULL) *pchk=!*pchk;
}

static void CompsInit_Global(u32 x,u32 y)
{
    u32 TextHeight=glCanvasTextHeight;
    u32 chkw=24,chkh=14;
  
    u32 h=TextHeight;
      
    u32 PaddingY=8;
      
    TProcState *pcps=pCurrentProcState;
      
    TextHeight-=1;
    
    // -----------------
    
    {
        TComponentCheck *pcc=&CompChecks[ECCSG_SkipSetupChk];
        pcc->UserData=(u32)&pcps->System.SkipSetup;
        pcc->pMsgUTF8=Lang_GetUTF8("CSG_SkipSetup");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
    y+=TextHeight*2;
      
    {
        TComponentCheck *pcc=&CompChecks[ECCSG_BootCheckDiskChk];
        pcc->UserData=(u32)&pcps->System.BootCheckDisk;
        pcc->pMsgUTF8=Lang_GetUTF8("CSG_BootCheckDisk");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
    y+=TextHeight*2;
      
    y+=PaddingY;
  
  // -----------------
  
    {
        TComponentCheck *pcc=&CompChecks[ECCSG_ClickSoundChk];
        pcc->UserData=(u32)&pcps->System.ClickSound;
        pcc->pMsgUTF8=Lang_GetUTF8("CSG_ClickSound");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
  
    {
        TComponentCheck *pcc=&CompChecks[ECCSG_EnableFadeEffectChk];
        pcc->UserData=(u32)&pcps->System.EnableFadeEffect;
        pcc->pMsgUTF8=Lang_GetUTF8("CSG_EnableFadeEffect");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
  
    {
        TComponentCheck *pcc=&CompChecks[ECCSG_AutoLastStateChk];
        pcc->UserData=(u32)&pcps->System.AutoLastState;
        pcc->pMsgUTF8=Lang_GetUTF8("CSG_AutoLastState");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
  
    y+=PaddingY;
  
    {
        u32 rx=x;
        {
            TComponentLabel *pcl=&CompLabels[ECLS_LRKeyLockTitleLbl];
            pcl->pMsgUTF8=Lang_GetUTF8("CSG_LRKeyLockTitle");
            pcl->Rect=CreateRect(rx,y,ScreenWidth-x,h);
            rx+=ComponentLabel_GetWidth(pcl);
        }
  
  {
            TComponentCheck *pcc=&CompChecks[ECCSG_LRKeyLockAlwayOffRadio];
            pcc->CallBack_Click=CB_LRKeyLockAlwayOffRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSG_LRKeyLockAlwayOff");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
              
        y+=TextHeight+PaddingY;
              
        rx=x;
          
        {
            TComponentCheck *pcc=&CompChecks[ECCSG_LRKeyLockRelationalPanelRadio];
            pcc->CallBack_Click=CB_LRKeyLockRelationalPanelRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSG_LRKeyLockRelationalPanel");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
          
        {
            TComponentCheck *pcc=&CompChecks[ECCSG_LRKeyLockAlwayOnRadio];
            pcc->CallBack_Click=CB_LRKeyLockAlwayOnRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSG_LRKeyLockAlwayOn");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
  }
  
    y+=TextHeight*2;
          
    y+=PaddingY;
    
    {
        TComponentCheck *pcc=&CompChecks[ECCSG_EnableResumeFunctionChk];
        pcc->UserData=(u32)&pcps->System.EnableResumeFunction;
        pcc->pMsgUTF8=Lang_GetUTF8("CSG_EnableResumeFunction");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
          
    {
        TComponentCheck *pcc=&CompChecks[ECCSG_EnableScreenCaptureChk];
        pcc->UserData=(u32)&pcps->System.EnableScreenCapture;
        pcc->pMsgUTF8=Lang_GetUTF8("CSG_EnableScreenCapture");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
    
    {
        TComponentCheck *pcc=&CompChecks[ECCSG_SpeakerPowerOffWhenPanelClosedChk];
        pcc->UserData=(u32)&pcps->System.SpeakerPowerOffWhenPanelClosed;
        pcc->pMsgUTF8=Lang_GetUTF8("CSG_SpeakerPowerOffWhenPanelClosed");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
  // -----------------
  
}
static void CompsInit_FileList(u32 x,u32 y)
{
    u32 TextHeight=glCanvasTextHeight;
  u32 chkw=24,chkh=14;
  
    u32 h=TextHeight;
  
    u32 PaddingY=8;
  
  TProcState *pcps=pCurrentProcState;
  
  TextHeight-=1;
  
  // -----------------
  
  {
        TComponentCheck *pcc=&CompChecks[ECCSF_MoveFolderLockedChk];
        pcc->UserData=(u32)&pcps->FileList.MoveFolderLocked;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_MoveFolderLocked");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
        TComponentCheck *pcc=&CompChecks[ECCSF_HiddenFilenameExtChk];
        pcc->UserData=(u32)&pcps->FileList.HiddenFilenameExt;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_HiddenFilenameExt");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
    {
        TComponentCheck *pcc=&CompChecks[ECCSF_HiddenNotSupportFileTypeChk];
        pcc->UserData=(u32)&pcps->FileList.HiddenNotSupportFileType;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_HiddenNotSupportFileType");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
  
    {
        TComponentCheck *pcc=&CompChecks[ECCSF_HiddenNDSROMFileChk];
        pcc->UserData=(u32)&pcps->FileList.HiddenNDSROMFile;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_HiddenNDSROMFile");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
  
  {
        TComponentCheck *pcc=&CompChecks[ECCSF_EnableFileInfoChk];
        pcc->UserData=(u32)&pcps->FileList.EnableFileInfo;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_EnableFileInfo");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
    {
        TComponentCheck *pcc=&CompChecks[ECCSF_ShowOnlineHelp_LRButtonChk];
        pcc->UserData=(u32)&pcps->FileList.ShowOnlineHelp_LRButton;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_ShowOnlineHelp_LRButton");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
        TComponentCheck *pcc=&CompChecks[ECCSF_BButtonToFolderUpChk];
        pcc->UserData=(u32)&pcps->FileList.BButtonToFolderUp;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_BButtonToFolderUp");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
        TComponentCheck *pcc=&CompChecks[ECCSF_EasyDeleteKeyChk];
        pcc->UserData=(u32)&pcps->FileList.EasyDeleteKey;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_EasyDeleteKey");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
                                
    {
        TComponentCheck *pcc=&CompChecks[ECCSF_UseJpegExifThumbnailChk];
        pcc->UserData=(u32)&pcps->FileList.UseJpegExifThumbnail;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_UseJpegExifThumbnail");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight*2;
    
    {
        TComponentCheck *pcc=&CompChecks[ECCSF_ShowCoverImageChk];
        pcc->UserData=(u32)&pcps->FileList.ShowCoverImage;
        pcc->pMsgUTF8=Lang_GetUTF8("CSF_ShowCoverImage");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  y+=PaddingY;
  
    {
        TComponentLabel *pcl=&CompLabels[ECLS_HideAttribute_TitleLbl];
        pcl->pMsgUTF8=Lang_GetUTF8("CSF_HideAttribute_Title");
        pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
    }
    
    y+=TextHeight*2;
    
    {
        u32 rx=x;
        {
            TComponentCheck *pcc=&CompChecks[ECCSF_HideAttribute_ArchiveChk];
            pcc->UserData=(u32)&pcps->FileList.HideAttribute_Archive;
            pcc->pMsgUTF8=Lang_GetUTF8("CSF_HideAttribute_Archive");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
    
        {
            TComponentCheck *pcc=&CompChecks[ECCSF_HideAttribute_HiddenChk];
            pcc->UserData=(u32)&pcps->FileList.HideAttribute_Hidden;
            pcc->pMsgUTF8=Lang_GetUTF8("CSF_HideAttribute_Hidden");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
        
        {
            TComponentCheck *pcc=&CompChecks[ECCSF_HideAttribute_SystemChk];
            pcc->UserData=(u32)&pcps->FileList.HideAttribute_System;
            pcc->pMsgUTF8=Lang_GetUTF8("CSF_HideAttribute_System");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
        
        {
            TComponentCheck *pcc=&CompChecks[ECCSF_HideAttribute_ReadOnlyChk];
            pcc->UserData=(u32)&pcps->FileList.HideAttribute_ReadOnly;
            pcc->pMsgUTF8=Lang_GetUTF8("CSF_HideAttribute_ReadOnly");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
        
        if(rx>ScreenWidth) {
            u32 tx=ComponentLabel_GetWidth(&CompLabels[ECLS_HideAttribute_TitleLbl])+10;
            u32 ty=y-TextHeight*2;
            
            CompChecks[ECCSF_HideAttribute_ArchiveChk].Rect=CreateRect(tx,ty,chkw,chkh);
            tx=x;
            ty+=TextHeight*2;
            CompChecks[ECCSF_HideAttribute_HiddenChk].Rect=CreateRect(tx,ty,chkw,chkh);
            tx+=ComponentCheck_GetWidth(&CompChecks[ECCSF_HideAttribute_HiddenChk]);
            CompChecks[ECCSF_HideAttribute_SystemChk].Rect=CreateRect(tx,ty,chkw,chkh);
            tx+=ComponentCheck_GetWidth(&CompChecks[ECCSF_HideAttribute_SystemChk]);
            CompChecks[ECCSF_HideAttribute_ReadOnlyChk].Rect=CreateRect(tx,ty,chkw,chkh);
            tx+=ComponentCheck_GetWidth(&CompChecks[ECCSF_HideAttribute_ReadOnlyChk]);
        }
    }
}
static void CompsInit_ScreenSaver(u32 x,u32 y)
{
    u32 TextHeight=glCanvasTextHeight;
    u32 chkw=24,chkh=14;
        
    u32 h=TextHeight;
        
    u32 PaddingY=8;
              
    TProcState *pcps=pCurrentProcState;
              
    TextHeight-=1;
    
  // -----------------
  
  {
	  TComponentCheck *pcc=&CompChecks[ECCSS_EnabledScreenSaverChk];
	  pcc->UserData=(u32)&pcps->ScreenSaver.EnabledScreenSaver;
	  pcc->pMsgUTF8=Lang_GetUTF8("CSS_EnabledScreenSaver");
	  pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+6;

  {
	  TComponentCheck *pcc=&CompChecks[ECCSS_ShowClockChk];
	  pcc->UserData=(u32)&pcps->ScreenSaver.ShowClock;
	  pcc->pMsgUTF8=Lang_GetUTF8("CSS_ShowClock");
      pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+6;
  
  {
	  TComponentCheck *pcc=&CompChecks[ECCSS_Use24hFormatChk];
	  pcc->UserData=(u32)&pcps->ScreenSaver.Use24hFormat;
	  pcc->pMsgUTF8=Lang_GetUTF8("CSS_Use24hFormat");
	  pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+6;
    
    y+=PaddingY;
    
  {
        TComponentLabel *pcl=&CompLabels[ECLS_ScreenSaverTitleLbl];
        pcl->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverTitle");
        pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
  }
        
  y+=TextHeight*2;
  
  {
        u32 rx=x;
        
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverNormalRadio];
            pcc->CallBack_Click=CB_ScreenSaverNormalRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverNormal");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
            
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverDigitalRadio];
            pcc->CallBack_Click=CB_ScreenSaverDigitalRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverDigital");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
            
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverExtendRadio];
            pcc->CallBack_Click=CB_ScreenSaverExtendRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverExtend");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
        
        if(rx>ScreenWidth) {
            u32 tx=ComponentLabel_GetWidth(&CompLabels[ECLS_ScreenSaverTitleLbl])+10;
            u32 ty=y-TextHeight*2;
                    
            CompChecks[ECCSS_ScreenSaverNormalRadio].Rect=CreateRect(tx,ty,chkw,chkh);
            tx=x;
            ty+=TextHeight*2;
            CompChecks[ECCSS_ScreenSaverDigitalRadio].Rect=CreateRect(tx,ty,chkw,chkh);
            tx+=ComponentCheck_GetWidth(&CompChecks[ECCSS_ScreenSaverDigitalRadio]);
            CompChecks[ECCSS_ScreenSaverExtendRadio].Rect=CreateRect(tx,ty,chkw,chkh);
            tx+=ComponentCheck_GetWidth(&CompChecks[ECCSS_ScreenSaverExtendRadio]);
        }
  }
    
  y+=TextHeight*2;
  
    y+=PaddingY;
        
  {
        TComponentLabel *pcl=&CompLabels[ECLS_ScreenSaverBGTitleLbl];
        pcl->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverBGTitle");
        pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
    }
            
    y+=TextHeight*2;
        
    {
        u32 rx=x;
            
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverBGSkinDefaultRadio];
            pcc->CallBack_Click=CB_ScreenSaverBGSkinDefaultRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverBGSkinDefault");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
                
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverBGTopScreenRadio];
            pcc->CallBack_Click=CB_ScreenSaverBGTopScreenRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverBGTopScreen");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
                
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverBGBottomScreenRadio];
            pcc->CallBack_Click=CB_ScreenSaverBGBottomScreenRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverBGBottomScreen");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
        if(rx>ScreenWidth) {
            u32 tx=ComponentLabel_GetWidth(&CompLabels[ECLS_ScreenSaverBGTitleLbl])+10;
            u32 ty=y-TextHeight*2;
                            
            CompChecks[ECCSS_ScreenSaverBGSkinDefaultRadio].Rect=CreateRect(tx,ty,chkw,chkh);
            tx=x;
            ty+=TextHeight*2;
            CompChecks[ECCSS_ScreenSaverBGTopScreenRadio].Rect=CreateRect(tx,ty,chkw,chkh);
            tx+=ComponentCheck_GetWidth(&CompChecks[ECCSS_ScreenSaverBGTopScreenRadio]);
            CompChecks[ECCSS_ScreenSaverBGBottomScreenRadio].Rect=CreateRect(tx,ty,chkw,chkh);
            tx+=ComponentCheck_GetWidth(&CompChecks[ECCSS_ScreenSaverBGBottomScreenRadio]);
        }
  }
  y+=TextHeight*2;
  
  y+=PaddingY;
  
    {
        u32 rx=x;
        
        {
            TComponentLabel *pcl=&CompLabels[ECLS_ScreenSaverTimeOutSecTitleLbl];
            pcl->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverTimeOutSecTitle");
            pcl->Rect=CreateRect(rx,y,ScreenWidth-x,h);
            rx+=ComponentLabel_GetWidth(pcl);
        }
        
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverTimeOutSec5secRadio];
            pcc->CallBack_Click=CB_ScreenSaverTimeOutSec5secRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverTimeOutSec5sec");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
        
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverTimeOutSec10secRadio];
            pcc->CallBack_Click=CB_ScreenSaverTimeOutSec10secRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverTimeOutSec10sec");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
        
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverTimeOutSec30secRadio];
            pcc->CallBack_Click=CB_ScreenSaverTimeOutSec30secRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverTimeOutSec30sec");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
        
        {
            TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverTimeOutSec60secRadio];
            pcc->CallBack_Click=CB_ScreenSaverTimeOutSec60secRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverTimeOutSec60sec");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
    }
    
    y+=TextHeight*2;
                
    y+=PaddingY;
  
  {
        TComponentCheck *pcc=&CompChecks[ECCSS_ScreenSaverTimeOutSecOffChk];
        pcc->UserData=(u32)&pcps->ScreenSaver.BacklightOffTimeout;
        pcc->pMsgUTF8=Lang_GetUTF8("CSS_ScreenSaverTimeOutSecOff");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+6;
  
  {
      TComponentCheck *pcc=&CompChecks[ECCSS_ShowID3TagChk];
      pcc->UserData=(u32)&pcps->ScreenSaver.ShowID3Tag;
      pcc->pMsgUTF8=Lang_GetUTF8("CSS_ShowID3Tag");
      pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+6;
  
  {
      TComponentCheck *pcc=&CompChecks[ECCSS_ShowLyricChk];
      pcc->UserData=(u32)&pcps->ScreenSaver.ShowLyric;
      pcc->pMsgUTF8=Lang_GetUTF8("CSS_ShowLyric");
      pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+6;
    
  {
	  TComponentCheck *pcc=&CompChecks[ECCSS_EnabledTouchPanelCombinationChk];
	  pcc->UserData=(u32)&pcps->ScreenSaver.EnabledTouchPanelCombination;
	  pcc->pMsgUTF8=Lang_GetUTF8("CSS_EnabledTouchPanelCombination");
	  pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+6;
        
  {
      TComponentCheck *pcc=&CompChecks[ECCSS_HideScreenSaverCustomChk];
      pcc->UserData=(u32)&pcps->ScreenSaver.HideScreenSaverCustom;
      pcc->pMsgUTF8=Lang_GetUTF8("CSS_HideScreenSaverCustom");
      pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
}

static void CompsInit_Music(u32 x,u32 y)
{
    u32 TextHeight=glCanvasTextHeight;
    u32 chkw=24,chkh=14;
            
    u32 h=TextHeight;
                  
    TextHeight-=1;
        
    // -----------------
    {
        TComponentLabel *pcl=&CompLabels[ECLS_PlayListEndTitleLbl];
        pcl->pMsgUTF8=Lang_GetUTF8("CSM_PlayListEndTitle");
        pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
    }   
                
  y+=TextHeight*2;
  
    {
        u32 rx=x;
                
        {
            TComponentCheck *pcc=&CompChecks[ECCSM_PlayListEndLoopRadio];
            pcc->CallBack_Click=CB_PlayListEndLoopRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSM_PlayListEndLoop");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
                    
        {
            TComponentCheck *pcc=&CompChecks[ECCSM_PlayListEndStopRadio];
            pcc->CallBack_Click=CB_PlayListEndStopRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSM_PlayListEndStop");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
                    
        {
            TComponentCheck *pcc=&CompChecks[ECCSM_PlayListEndPowerOffRadio];
            pcc->CallBack_Click=CB_PlayListEndPowerOffRadio_Click;
            pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
            pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
            pcc->pMsgUTF8=Lang_GetUTF8("CSM_PlayListEndPowerOff");
            pcc->Rect=CreateRect(rx,y,chkw,chkh);
            rx+=ComponentCheck_GetWidth(pcc);
        }
    }
}

static void CompsInit(void)
{
  CglCanvas *pcan=pScreenBack;
  
  for(u32 idx=0;idx<CompLabelsCount;idx++){
    ComponentLabel_Init(&CompLabels[idx],pcan);
    //TComponentLabel *pcl=&CompLabels[idx];
  }
  for(u32 idx=0;idx<CompChecksCount;idx++){
    ComponentCheck_Init(&CompChecks[idx],pcan);
    TComponentCheck *pcc=&CompChecks[idx];
    pcc->CallBack_Click=CB_CheckBox_Click;
    pcc->UserData=NULL;
    pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_CheckOn);
    pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_CheckOff);
  }
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    ComponentButton_Init(&CompButtons[idx],pcan);
    //TComponentButton *pcb=&CompButtons[idx];
  }
  
  // -----------------
  
  u32 TextHeight=glCanvasTextHeight;
  
  u32 x=0,y=0,w=0,h=0;
  
  h=TextHeight;
  
  // -----------------
  
  x=8;
  y+=8;
  
  {
    TComponentLabel *pcl=&CompLabels[ECLS_TitleLbl];
    pcl->pMsgUTF8=Lang_GetUTF8("CS_Title");
    pcl->Rect=CreateRect(x+24,y,ScreenWidth-x,h);
    pcl->isTitle=true;
  }
  y+=12*2;
  
  // -----------------
  
  u32 chkw=24,chkh=14;
  
  u32 PaddingY=8;
  
  y+=PaddingY;
  
  TextHeight-=1;
  
  // -----------------
  {
      u32 rx=x;
      {
          TComponentLabel *pcl=&CompLabels[ECLS_PageTab_TitleLbl];
          pcl->pMsgUTF8=Lang_GetUTF8("CS_PageTab_Title");
          pcl->Rect=CreateRect(rx,y,ScreenWidth-x,h);
          rx+=ComponentLabel_GetWidth(pcl);
      }
          
      {
          TComponentCheck *pcc=&CompChecks[ECCS_PageTab_GlobalRadio];
          pcc->CallBack_Click=CB_PageTab_GlobalRadio_Click;
          pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
          pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
          pcc->pMsgUTF8=Lang_GetUTF8("CS_PageTab_Global");
          pcc->Rect=CreateRect(rx,y,chkw,chkh);
          rx+=ComponentCheck_GetWidth(pcc);
      }
      
      {
          TComponentCheck *pcc=&CompChecks[ECCS_PageTab_FileListRadio];
          pcc->CallBack_Click=CB_PageTab_FileListRadio_Click;
          pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
          pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
          pcc->pMsgUTF8=Lang_GetUTF8("CS_PageTab_FileList");
          pcc->Rect=CreateRect(rx,y,chkw,chkh);
          rx+=ComponentCheck_GetWidth(pcc);
      }
          
      y+=TextHeight+PaddingY;
          
      rx=x;
      
      {
          TComponentCheck *pcc=&CompChecks[ECCS_PageTab_ScreenSaverRadio];
          pcc->CallBack_Click=CB_PageTab_ScreenSaverRadio_Click;
          pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
          pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
          pcc->pMsgUTF8=Lang_GetUTF8("CS_PageTab_ScreenSaver");
          pcc->Rect=CreateRect(rx,y,chkw,chkh);
          rx+=ComponentCheck_GetWidth(pcc);
      }
      
      {
          TComponentCheck *pcc=&CompChecks[ECCS_PageTab_MusicRadio];
          pcc->CallBack_Click=CB_PageTab_MusicRadio_Click;
          pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
          pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
          pcc->pMsgUTF8=Lang_GetUTF8("CS_PageTab_Music");
          pcc->Rect=CreateRect(rx,y,chkw,chkh);
          rx+=ComponentCheck_GetWidth(pcc);
      }
  }
  
  y+=TextHeight*2+PaddingY+5;
  
  CompsInit_Global(x,y);
  CompsInit_FileList(x,y);
  CompsInit_ScreenSaver(x,y);
  CompsInit_Music(x,y);
  
  y+=PaddingY;
  
  // -----------------
  
  u32 padx=24;
  
  w=96;
  h=24;
  x=padx;
  y=ClientHeight-h-PaddingY;
  
  {
    TComponentButton *pcb=&CompButtons[ECBS_CancelBtn];
    pcb->CallBack_Click=CB_CancelBtn_Click;
    pcb->pIcon=ComponentAlpha_GetSkin(ECSA_Cancel);
    pcb->pMsgUTF8=Lang_GetUTF8("CS_Cancel");
    pcb->Rect=CreateRect(x,y,w,h);
  }
  
  w=72;
  x=ScreenWidth-padx-w;
  
  {
    TComponentButton *pcb=&CompButtons[ECBS_OkBtn];
    pcb->CallBack_Click=CB_OkBtn_Click;
    pcb->pIcon=ComponentAlpha_GetSkin(ECSA_Ok);
    pcb->pMsgUTF8=Lang_GetUTF8("CS_Ok");
    pcb->Rect=CreateRect(x,y,w,h);
  }
  
  switch(pCurrentProcState->System.LastPageTab){
      case ELPT_Global: {
          CompLabels[ECLS_LRKeyLockTitleLbl].Visible=true;
          CompLabels[ECLS_HideAttribute_TitleLbl].Visible=false;
          CompLabels[ECLS_ScreenSaverTitleLbl].Visible=false;
          CompLabels[ECLS_ScreenSaverBGTitleLbl].Visible=false;
          CompLabels[ECLS_ScreenSaverTimeOutSecTitleLbl].Visible=false;
          CompLabels[ECLS_PlayListEndTitleLbl].Visible=false;
          
          for(u32 idx=4;idx<CompChecksCount;idx++){
              TComponentCheck *pcc=&CompChecks[idx];
              if((3<idx)&&(idx<15)) pcc->Visible=true;
              else pcc->Visible=false;
          }
      } break;
      case ELPT_FileList: {
          CompLabels[ECLS_LRKeyLockTitleLbl].Visible=false;
          CompLabels[ECLS_HideAttribute_TitleLbl].Visible=true;
          CompLabels[ECLS_ScreenSaverTitleLbl].Visible=false;
          CompLabels[ECLS_ScreenSaverBGTitleLbl].Visible=false;
          CompLabels[ECLS_ScreenSaverTimeOutSecTitleLbl].Visible=false;
          CompLabels[ECLS_PlayListEndTitleLbl].Visible=false;
            
          for(u32 idx=4;idx<CompChecksCount;idx++){
              TComponentCheck *pcc=&CompChecks[idx];
              if((14<idx)&&(idx<29)) pcc->Visible=true;
              else pcc->Visible=false;
          }
      } break;
      case ELPT_ScreenSaver: {
          CompLabels[ECLS_LRKeyLockTitleLbl].Visible=false;
          CompLabels[ECLS_HideAttribute_TitleLbl].Visible=false;
          CompLabels[ECLS_ScreenSaverTitleLbl].Visible=true;
          CompLabels[ECLS_ScreenSaverBGTitleLbl].Visible=true;
          CompLabels[ECLS_ScreenSaverTimeOutSecTitleLbl].Visible=true;
          CompLabels[ECLS_PlayListEndTitleLbl].Visible=false;
              
          for(u32 idx=5;idx<CompChecksCount;idx++){
              TComponentCheck *pcc=&CompChecks[idx];
              if((28<idx)&&(idx<47)) pcc->Visible=true;
              else pcc->Visible=false;
          }
      } break;
      case ELPT_Music: {
          CompLabels[ECLS_LRKeyLockTitleLbl].Visible=false;
          CompLabels[ECLS_HideAttribute_TitleLbl].Visible=false;
          CompLabels[ECLS_ScreenSaverTitleLbl].Visible=false;
          CompLabels[ECLS_ScreenSaverBGTitleLbl].Visible=false;
          CompLabels[ECLS_ScreenSaverTimeOutSecTitleLbl].Visible=false;
          CompLabels[ECLS_PlayListEndTitleLbl].Visible=true;
          
          for(u32 idx=5;idx<CompChecksCount;idx++){
              TComponentCheck *pcc=&CompChecks[idx];
              if((46<idx)&&(idx<50)) pcc->Visible=true;
              else pcc->Visible=false;
          }
      } break;
      default: break;
  }
}

// -----------------------------

static void CB_KeyPress(u32 VsyncCount,u32 Keys)
{
  if((Keys&KEY_B)!=0){
    CB_CancelBtn_Click(NULL);
  }
  
  if((Keys&KEY_A)!=0){
    CB_OkBtn_Click(NULL);
  }
  
  if((Keys&(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT))!=0){
    s32 v=0;
    
    if((Keys&(KEY_UP|KEY_LEFT))!=0) v=-1;
    if((Keys&(KEY_DOWN|KEY_RIGHT))!=0) v=1;
    
    ScrollBar_SetPosY(ScreenPosY+(32*v));
  }
}

static void CB_KeyLongPress(u32 Keys)
{
    if(Keys!=KEY_SELECT) return;
    
    if(CompChecks[ECCS_PageTab_GlobalRadio].Checked){
        CB_PageTab_GlobalRadio_Click(NULL);
    
        u32 size;
        u8 *pbmp=pScreenBack->CreateBMPImage(&size);
        if(pbmp!=NULL){
            FAT_FILE *pf=FAT2_fopen_AliasForWrite("/SS_CSG.BMP");
            if(pf!=NULL){
                FAT2_fwrite(pbmp,1,size,pf);
                FAT2_fclose(pf);
            }
            safefree(&MM_Temp,pbmp); pbmp=NULL;
        }
    }
    if(CompChecks[ECCS_PageTab_FileListRadio].Checked){
        CB_PageTab_FileListRadio_Click(NULL);
        
        u32 size;
        u8 *pbmp=pScreenBack->CreateBMPImage(&size);
        if(pbmp!=NULL){
            FAT_FILE *pf=FAT2_fopen_AliasForWrite("/SS_CSF.BMP");
            if(pf!=NULL){
                FAT2_fwrite(pbmp,1,size,pf);
                FAT2_fclose(pf);
            }
            safefree(&MM_Temp,pbmp); pbmp=NULL;
        }
    }
    if(CompChecks[ECCS_PageTab_ScreenSaverRadio].Checked){
        CB_PageTab_ScreenSaverRadio_Click(NULL);
        
        u32 size;
        u8 *pbmp=pScreenBack->CreateBMPImage(&size);
        if(pbmp!=NULL){
            FAT_FILE *pf=FAT2_fopen_AliasForWrite("/SS_CSS.BMP");
            if(pf!=NULL){
                FAT2_fwrite(pbmp,1,size,pf);
                FAT2_fclose(pf);
            }
            safefree(&MM_Temp,pbmp); pbmp=NULL;
        }
    }
    if(CompChecks[ECCS_PageTab_MusicRadio].Checked){
        CB_PageTab_MusicRadio_Click(NULL);
        
        u32 size;
        u8 *pbmp=pScreenBack->CreateBMPImage(&size);
        if(pbmp!=NULL){
            FAT_FILE *pf=FAT2_fopen_AliasForWrite("/SS_CSM.BMP");
            if(pf!=NULL){
                FAT2_fwrite(pbmp,1,size,pf);
                FAT2_fclose(pf);
            }
            safefree(&MM_Temp,pbmp); pbmp=NULL;
        }
    }
    
    Sound_Start(WAVFN_Click);
}

DATA_IN_IWRAM_Custom static bool scrmf;
DATA_IN_IWRAM_Custom static s32 scrmy;

DATA_IN_IWRAM_Custom static bool deskmf;
DATA_IN_IWRAM_Custom static TComponentButton *pPressingButton;

static void CB_MouseDown(s32 x,s32 y)
{
  if((ScreenWidth-24)<=x){
    scrmf=true;
    scrmy=y;
    s32 h=(ScreenHeight*ScreenHeight)/ClientHeight;
    ScrollBar_SetPosY(((scrmy*ClientHeight)/ScreenHeight)-(h/2));
    ShowPosY=ScreenPosY;
    Setting_Redraw_TransOnly();
    return;
  }
  
  deskmf=false;
  
  y+=ScreenPosY;
  
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    TComponentButton *pcb=&CompButtons[idx];
    if(ComponentButton_GetIndexFromPos(pcb,x,y)!=-1){
      pPressingButton=pcb;
      pcb->Pressing=true;
      ComponentButton_Draw(pcb);
      Setting_Redraw();
      deskmf=true;
      return;
    }
  }
  
  deskmf=true;
}

static void CB_MouseMove(s32 x,s32 y)
{
  if(scrmf==true){
    scrmy=y;
    s32 h=(ScreenHeight*ScreenHeight)/ClientHeight;
    ScrollBar_SetPosY(((scrmy*ClientHeight)/ScreenHeight)-(h/2));
    ShowPosY=ScreenPosY;
    Setting_Redraw_TransOnly();
    return;
  }
  
  if(deskmf==false) return;
  
  y+=ScreenPosY;
  
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    TComponentButton *pcb=&CompButtons[idx];
    if(pcb==pPressingButton){
      if(ComponentButton_GetIndexFromPos(pcb,x,y)==-1){
        if(pcb->Pressing==true){
          pcb->Pressing=false;
          ComponentButton_Draw(pcb);
          Setting_Redraw();
        }
        }else{
        if(pcb->Pressing==false){
          pcb->Pressing=true;
          ComponentButton_Draw(pcb);
          Setting_Redraw();
        }
      }
    }
  }
}

static void CB_MouseUp(s32 x,s32 y)
{
  if(scrmf==true){
    scrmf=false;
    scrmy=y;
    s32 h=(ScreenHeight*ScreenHeight)/ClientHeight;
    ScrollBar_SetPosY(((scrmy*ClientHeight)/ScreenHeight)-(h/2));
    ShowPosY=ScreenPosY;
    Setting_Redraw_TransOnly();
    return;
  }
  
  if(deskmf==false) return;
  
  deskmf=false;
  
  y+=ScreenPosY;
  
  if(pPressingButton!=NULL){
    pPressingButton->Pressing=false;
    ComponentButton_Draw(pPressingButton);
    
    for(u32 idx=0;idx<CompButtonsCount;idx++){
      TComponentButton *pcb=&CompButtons[idx];
      if(pcb==pPressingButton){
        if(ComponentButton_GetIndexFromPos(pcb,x,y)!=-1){
          ComponentButton_MouseUp(&CompButtons[idx],x,y);
        }
      }
    }
    
    Setting_Redraw();
    pPressingButton=NULL;
    return;
  }
  
  for(u32 idx=0;idx<CompLabelsCount;idx++){
    if(ComponentLabel_MouseUp(&CompLabels[idx],x,y)==true) Setting_Redraw();
  }
  for(u32 idx=0;idx<CompChecksCount;idx++){
    if(ComponentCheck_MouseUp(&CompChecks[idx],x,y)==true) Setting_Redraw();
  }
  
}

static void DrawOnlineHelp(void)
{
  if(Skin_OwnerDrawText.Custom_Top==true) return;
  
  CglB15 *pb15=Custom_GetSkin(ECS_TopMsg);
  
  pb15->pCanvas->SetCglFont(pCglFontDefault);
  
  u32 x=8;
  u32 y=8;
  u32 h=glCanvasTextHeight+3;
  
  for(u32 idx=0;idx<12;idx++){
    const char *pmsg=NULL;
    switch(idx){
#define Prefix "CS_"
      case 0: pmsg=Lang_GetUTF8(Prefix "Help0"); break;
      case 1: pmsg=Lang_GetUTF8(Prefix "Help1"); break;
      case 2: pmsg=Lang_GetUTF8(Prefix "Help2"); break;
      case 3: pmsg=Lang_GetUTF8(Prefix "Help3"); break;
      case 4: pmsg=Lang_GetUTF8(Prefix "Help4"); break;
      case 5: pmsg=Lang_GetUTF8(Prefix "Help5"); break;
      case 6: pmsg=Lang_GetUTF8(Prefix "Help6"); break;
      case 7: pmsg=Lang_GetUTF8(Prefix "Help7"); break;
      case 8: pmsg=Lang_GetUTF8(Prefix "Help8"); break;
      case 9: pmsg=Lang_GetUTF8(Prefix "Help9"); break;
      case 10: pmsg=Lang_GetUTF8(Prefix "Help10"); break;
      case 11: pmsg=Lang_GetUTF8(Prefix "Help11"); break;
#undef Prefix
    }
    if(pmsg!=NULL){
      if(idx==0){
        pb15->pCanvas->SetFontTextColor(ColorTable.Component.HelpTop_Text);
        }else{
        pb15->pCanvas->SetFontTextColor(ColorTable.Component.HelpBody_Text);
      }
      pb15->pCanvas->TextOutUTF8(x,y,pmsg);
    }
    y+=h;
  }
}

static void CB_Start(void)
{
  Sound_Start(WAVFN_Open);
  
  DrawOnlineHelp();
  
  scrmf=false;
  scrmy=0;
  
  deskmf=false;
  pPressingButton=NULL;
  
  pScreenBack=new CglCanvas(&MM_Process,NULL,ScreenWidth,ClientHeight,pf15bit);
  ScrollBar_SetPosY(0);
  ShowPosY=ScreenPosY;
  
  pCurrentProcState=(TProcState*)safemalloc_chkmem(&MM_Process,sizeof(TProcState));
  *pCurrentProcState=ProcState;
  
  CompsInit();
  
  Setting_Redraw();
}

static void CB_VsyncUpdate(u32 VsyncCount)
{
  if(ShowPosY!=ScreenPosY){
    VsyncCount*=4;
    while(VsyncCount!=0){
      VsyncCount--;
      if(ScreenPosY!=ShowPosY){
        if(ScreenPosY<ShowPosY){
          ShowPosY--;
          }else{
          ShowPosY++;
        }
      }
    }
    Setting_Redraw_TransOnly();
  }
}

static void CB_End(void)
{
  if(pCurrentProcState!=NULL){
    safefree(&MM_Process,pCurrentProcState); pCurrentProcState=NULL;
  }
  
  if(pScreenBack!=NULL){
    delete pScreenBack; pScreenBack=NULL;
  }
}

void ProcCustom_SetCallBack(TCallBack *pCallBack)
{
  pCallBack->Start=CB_Start;
  pCallBack->VsyncUpdate=CB_VsyncUpdate;
  pCallBack->End=CB_End;
  pCallBack->KeyPress=CB_KeyPress;
  pCallBack->KeyLongPress=CB_KeyLongPress;
  pCallBack->MouseDown=CB_MouseDown;
  pCallBack->MouseMove=CB_MouseMove;
  pCallBack->MouseUp=CB_MouseUp;
}

