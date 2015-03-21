
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"

#include "skin.h"

#include "maindef.h"
#include "memtool.h"
#include "shell.h"
#include "splash.h"
#include "strtool.h"
#include "procstate.h"

#include "cstream_fs.h"

#include "zlibhelp.h"

#include "extmem.h"

// ---------------------------------------------------------------------------------

#include "skin_SkinFile.h"
#include "skin_CustomBG.h"

// ---------------------------------------------------------------------------------

typedef struct {
  CglB15 *pbm;
} TstructB15;

typedef struct {
  CglTGF *pbm;
} TstructTGF;

#define ComponentSkinAlphaCount (ECSACount)
DATA_IN_AfterSystem static TstructTGF ComponentSkinAlpha[ComponentSkinAlphaCount];

#define FileListSkinCount (EFLSCount)
DATA_IN_AfterSystem static TstructB15 FileListSkin[FileListSkinCount];
#define FileListSkinAlphaCount (EFLSACount)
DATA_IN_AfterSystem static TstructTGF FileListSkinAlpha[FileListSkinAlphaCount];
#define FileListClockSkinAlphaCount (EFLCLKSACount)
DATA_IN_AfterSystem static TstructTGF FileListClockSkinAlpha[FileListClockSkinAlphaCount];

#define ScrollBarSkinAlphaCount (ESBSACount)
DATA_IN_AfterSystem static TstructTGF ScrollBarSkinAlpha[ScrollBarSkinAlphaCount];

#define SetupSkinCount (ESSCount)
DATA_IN_AfterSystem static TstructB15 SetupSkin[SetupSkinCount];
#define SetupSkinAlphaCount (ESSACount)
DATA_IN_AfterSystem static TstructTGF SetupSkinAlpha[SetupSkinAlphaCount];

#define SysMenuSkinAlphaCount (ESMSACount)
DATA_IN_AfterSystem static TstructTGF SysMenuSkinAlpha[SysMenuSkinAlphaCount];

#define MoviePlayerSkinCount (EMPSCount)
DATA_IN_AfterSystem static TstructB15 MoviePlayerSkin[MoviePlayerSkinCount];
#define MoviePlayerSkinAlphaCount (EMPSACount)
DATA_IN_AfterSystem static TstructTGF MoviePlayerSkinAlpha[MoviePlayerSkinAlphaCount];

#define ImageViewSkinCount (EIVSCount)
DATA_IN_AfterSystem static TstructB15 ImageViewSkin[ImageViewSkinCount];

#define LaunchSkinCount (ELSCount)
DATA_IN_AfterSystem static TstructB15 LaunchSkin[LaunchSkinCount];
#define LaunchSkinAlphaCount (ELSACount)
DATA_IN_AfterSystem static TstructTGF LaunchSkinAlpha[LaunchSkinAlphaCount];

#define LongTapSkinAlphaCount (ELTSACount)
DATA_IN_AfterSystem static TstructTGF LongTapSkinAlpha[LongTapSkinAlphaCount];

#define MP3CntSkinAlphaCount (EMP3SACount)
DATA_IN_AfterSystem static TstructTGF MP3CntSkinAlpha[MP3CntSkinAlphaCount];

#define CustomSkinCount (ECSCount)
DATA_IN_AfterSystem static TstructB15 CustomSkin[CustomSkinCount];

#define StandbyClockSkinCount (ESCCount)
DATA_IN_AfterSystem static TstructB15 StandbyClockSkin[StandbyClockSkinCount];
#define StandbyClockSkinAlphaCount (ESCACount)
DATA_IN_AfterSystem static TstructTGF StandbyClockSkinAlpha[StandbyClockSkinAlphaCount];

#define TextViewSkinCount (ETVCount)
static TstructB15 TextViewSkin[TextViewSkinCount];
#define TextViewSkinAlphaCount (ETVACount)
DATA_IN_AfterSystem static TstructTGF TextViewSkinAlpha[TextViewSkinAlphaCount];

#define MemoEditSkinCount (EMECount)
DATA_IN_AfterSystem static TstructB15 MemoEditSkin[MemoEditSkinCount];
#define MemoEditSkinAlphaCount (EMEACount)
DATA_IN_AfterSystem static TstructTGF MemoEditSkinAlpha[MemoEditSkinAlphaCount];

#define MemoListSkinCount (EMLCount)
DATA_IN_AfterSystem static TstructB15 MemoListSkin[MemoListSkinCount];

// ---------------------------------------------------------------------------------

static void ComponentAlpha_Init(void)
{
  for(u32 idx=0;idx<ComponentSkinAlphaCount;idx++){
    TstructTGF *ptag=&ComponentSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((EComponentSkinAlpha)idx){
      case ECSA_Info: pfn="cmps_Info.tgf"; break;
      case ECSA_BoxMin: pfn="cmps_BoxMin.tgf"; break;
      case ECSA_BoxPlus: pfn="cmps_BoxPlus.tgf"; break;
      case ECSA_CheckOff: pfn="cmps_ChkOff.tgf"; break;
      case ECSA_CheckOn: pfn="cmps_ChkOn.tgf"; break;
      case ECSA_CheckOnOff: pfn="cmps_ChkOnOff.tgf"; break;
      case ECSA_RadioOff: pfn="cmps_RadioOff.tgf"; break;
      case ECSA_RadioOn: pfn="cmps_RadioOn.tgf"; break;
      case ECSA_Ok: pfn="cmps_Ok.tgf"; break;
      case ECSA_Cancel: pfn="cmps_Cancel.tgf"; break;
      case ECSACount: default: StopFatalError(13701,"ComponentAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void ComponentAlpha_Free(void)
{
  for(u32 idx=0;idx<ComponentSkinAlphaCount;idx++){
    TstructTGF *ptag=&ComponentSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* ComponentAlpha_GetSkin(EComponentSkinAlpha idx)
{
  TstructTGF *ptag=&ComponentSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void FileList_Init(void)
{
  CglCanvas *pCustomBG=LoadCustomBG();
  
  if(pCustomBG==NULL){
    for(u32 idx=0;idx<FileListSkinCount;idx++){
      TstructB15 *ptag=&FileListSkin[(u32)idx];
      const char *pfn=NULL;
      switch((EFileListSkin)idx){
        case EFLS_BG_TopMsg: pfn="FL_BG_TopMsg.b15"; break;
        case EFLS_BG_Top: pfn="FL_BG_Top.b15"; break;
        case EFLS_BG_Bottom: pfn="FL_BG_Bottom.b15"; break;
        case EFLS_DeleteFileDialog: pfn="FL_DeleteFileDialog.b15"; break;
        case EFLSCount: default: StopFatalError(13702,"FileList: Unknown type. %d\n",idx); break;
      }
      SkinFile_LoadB15(pfn,&ptag->pbm);
    }
    }else{
    Splash_Update();
    {
      TstructB15 *ptag=&FileListSkin[(u32)EFLS_BG_TopMsg];
      ptag->pbm=new CglB15(&MM_Skin,NULL,ScreenWidth | (ScreenHeight<<16));
      CglCanvas *pcan=ptag->pbm->pCanvas;
      pCustomBG->BitBlt(pcan,0,0,ScreenWidth,ScreenHeight,0,0,false);
    }
    Splash_Update();
    {
      TstructB15 *ptag=&FileListSkin[(u32)EFLS_BG_Top];
      ptag->pbm=new CglB15(&MM_Skin,NULL,ScreenWidth | (ScreenHeight<<16));
      CglCanvas *pcan=ptag->pbm->pCanvas;
      pCustomBG->BitBlt(pcan,0,0,ScreenWidth,ScreenHeight,0,0,false);
    }
    Splash_Update();
    {
      TstructB15 *ptag=&FileListSkin[(u32)EFLS_BG_Bottom];
      ptag->pbm=new CglB15(&MM_Skin,NULL,ScreenWidth | (ScreenHeight<<16));
      CglCanvas *pcan=ptag->pbm->pCanvas;
      pCustomBG->BitBlt(pcan,0,0,ScreenWidth,ScreenHeight,0,ScreenHeight,false);
    }
    Splash_Update();
    {
      TstructB15 *ptag=&FileListSkin[(u32)EFLS_DeleteFileDialog];
      const char *pfn="FL_DeleteFileDialog.b15";
      SkinFile_LoadB15(pfn,&ptag->pbm);
    }
  }
  
  if(pCustomBG!=NULL){
    delete pCustomBG; pCustomBG=NULL;
  }
}

static void FileList_Free(void)
{
  for(u32 idx=0;idx<FileListSkinCount;idx++){
    TstructB15 *ptag=&FileListSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* FileList_GetSkin(EFileListSkin idx)
{
  TstructB15 *ptag=&FileListSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void FileListAlpha_Init(EProcStateFileListMode Mode)
{
  const char *pPrefix;
  switch(Mode){
    case EPSFLM_Single: pPrefix="FL_Single_%s"; break;
    case EPSFLM_Double: pPrefix="FL_Double_%s"; break;
    default: pPrefix="%s"; break;
  }
  
  for(u32 idx=0;idx<FileListSkinAlphaCount;idx++){
    TstructTGF *ptag=&FileListSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((EFileListSkinAlpha)idx){
      case EFLSA_ItemBG_Clear: pfn="ItemBG_Clear.tgf"; break;
      case EFLSA_ItemBG_Select: pfn="ItemBG_Select.tgf"; break;
      case EFLSA_ItemBG_PlayIcon: pfn="ItemBG_PlayIcon.tgf"; break;
      case EFLSA_Icon_UnknownFile: pfn="Icon_UnknownFile.tgf"; break;
      case EFLSA_Icon_UpFolder: pfn="Icon_UpFolder.tgf"; break;
      case EFLSA_Icon_Folder: pfn="Icon_Folder.tgf"; break;
      case EFLSA_Icon_Sound: pfn="Icon_Sound.tgf"; break;
      case EFLSA_Icon_Image: pfn="Icon_Image.tgf"; break;
      case EFLSA_Icon_Video: pfn="Icon_Video.tgf"; break;
      case EFLSA_Icon_NDSROM: pfn="Icon_NDSROM.tgf"; break;
      case EFLSA_Icon_GBAROM: pfn="Icon_GBAROM.tgf"; break;
      case EFLSA_Icon_Skin: pfn="Icon_Skin.tgf"; break;
      case EFLSA_Icon_Text: pfn="Icon_Text.tgf"; break;
      case EFLSA_Slide_Folder: pfn="Slide_Folder.tgf"; break;
      case EFLSA_Slide_Image_Left: pfn="Slide_Image_Left.tgf"; break;
      case EFLSA_Slide_Image_Right: pfn="Slide_Image_Right.tgf"; break;
      case EFLSA_Slide_NDS_Left: pfn="Slide_NDS_Left.tgf"; break;
      case EFLSA_Slide_NDS_Right: pfn="Slide_NDS_Right.tgf"; break;
      case EFLSA_Slide_Skin: pfn="Slide_Skin.tgf"; break;
      case EFLSA_Slide_Sound_Left: pfn="Slide_Sound_Left.tgf"; break;
      case EFLSA_Slide_Text_Left: pfn="Slide_Text_Left.tgf"; break;
      case EFLSA_Slide_Text_Right: pfn="Slide_Text_Right.tgf"; break;
      case EFLSA_Slide_UpFolder: pfn="Slide_UpFolder.tgf"; break;
      case EFLSA_Slide_Video_Left: pfn="Slide_Video_Left.tgf"; break;
      case EFLSA_Slide_Video_Right: pfn="Slide_Video_Right.tgf"; break;
      case EFLSACount: default: StopFatalError(13703,"FileListAlpha: Unknown type. %d\n",idx); break;
    }
    char fn[32];
    snprintf(fn,32,pPrefix,pfn);
    SkinFile_LoadTGF(fn,&ptag->pbm);
  }
}

static void FileListAlpha_Free(void)
{
  for(u32 idx=0;idx<FileListSkinAlphaCount;idx++){
    TstructTGF *ptag=&FileListSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* FileListAlpha_GetSkin(EFileListSkinAlpha idx)
{
  TstructTGF *ptag=&FileListSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void FileListClockAlpha_Init(void)
{
  for(u32 idx=0;idx<FileListClockSkinAlphaCount;idx++){
    TstructTGF *ptag=&FileListClockSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((EFileListClockSkinAlpha)idx){
      case EFLCLKSA_BG: pfn="FLCLK_BG.tgf"; break;
      case EFLCLKSA_Digits: pfn="FLCLK_digits.tgf"; break;
      case EFLCLKSACount: default: StopFatalError(13704,"FileListClockAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void FileListClockAlpha_Free(void)
{
  for(u32 idx=0;idx<FileListClockSkinAlphaCount;idx++){
    TstructTGF *ptag=&FileListClockSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* FileListClockAlpha_GetSkin(EFileListClockSkinAlpha idx)
{
  TstructTGF *ptag=&FileListClockSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void ScrollBarAlpha_Init(void)
{
  for(u32 idx=0;idx<ScrollBarSkinAlphaCount;idx++){
    TstructTGF *ptag=&ScrollBarSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((EScrollBarSkinAlpha)idx){
      case ESBSA_BG: pfn="SB_BG.tgf"; break;
      case ESBSA_DownBtn_Normal: pfn="SB_DownBtn_Normal.tgf"; break;
      case ESBSA_DownBtn_Press: pfn="SB_DownBtn_Press.tgf"; break;
      case ESBSA_UpBtn_Normal: pfn="SB_UpBtn_Normal.tgf"; break;
      case ESBSA_UpBtn_Press: pfn="SB_UpBtn_Press.tgf"; break;
      case ESBSA_GripBG_Normal: pfn="SB_GripBG_Normal.tgf"; break;
      case ESBSA_GripBG_Press: pfn="SB_GripBG_Press.tgf"; break;
      case ESBSA_GripBottom_Normal: pfn="SB_GripBottom_Normal.tgf"; break;
      case ESBSA_GripBottom_Press: pfn="SB_GripBottom_Press.tgf"; break;
      case ESBSA_GripTop_Normal: pfn="SB_GripTop_Normal.tgf"; break;
      case ESBSA_GripTop_Press: pfn="SB_GripTop_Press.tgf"; break;
      case ESBSACount: default: StopFatalError(13705,"ScrollBarAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void ScrollBarAlpha_Free(void)
{
  for(u32 idx=0;idx<ScrollBarSkinAlphaCount;idx++){
    TstructTGF *ptag=&ScrollBarSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* ScrollBarAlpha_GetSkin(EScrollBarSkinAlpha idx)
{
  TstructTGF *ptag=&ScrollBarSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void Setup_Init(void)
{
  for(u32 idx=0;idx<SetupSkinCount;idx++){
    TstructB15 *ptag=&SetupSkin[(u32)idx];
    const char *pfn=NULL;
    switch((ESetupSkin)idx){
      case ESS_BG_Top: pfn="Setup_BG_Top.b15"; break;
      case ESS_BG_Bottom: pfn="Setup_BG_Bottom.b15"; break;
      case ESSCount: default: StopFatalError(13706,"Setup: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadB15(pfn,&ptag->pbm);
  }
}

static void Setup_Free(void)
{
  for(u32 idx=0;idx<SetupSkinCount;idx++){
    TstructB15 *ptag=&SetupSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* Setup_GetSkin(ESetupSkin idx)
{
  TstructB15 *ptag=&SetupSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void SetupAlpha_Init(void)
{
  for(u32 idx=0;idx<SetupSkinAlphaCount;idx++){
    TstructTGF *ptag=&SetupSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((ESetupSkinAlpha)idx){
      case ESSA_ChkOverlayOn: pfn="Setup_ChkOverlayOn.tgf"; break;
      case ESSA_RadioOverlayOn: pfn="Setup_RadioOverlayOn.tgf"; break;
      case ESSACount: default: StopFatalError(13707,"SetupAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void SetupAlpha_Free(void)
{
  for(u32 idx=0;idx<SetupSkinAlphaCount;idx++){
    TstructTGF *ptag=&SetupSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* SetupAlpha_GetSkin(ESetupSkinAlpha idx)
{
  TstructTGF *ptag=&SetupSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void SysMenuAlpha_Init(void)
{
  for(u32 idx=0;idx<SysMenuSkinAlphaCount;idx++){
    TstructTGF *ptag=&SysMenuSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((ESysMenuSkinAlpha)idx){
      case ESMSA_BG: pfn="SysMenu_BG.tgf"; break;
      case ESMSA_SelectBar: pfn="SysMenu_SelectBar.tgf"; break;
      case ESMSACount: default: StopFatalError(13708,"SysMenuAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void SysMenuAlpha_Free(void)
{
  for(u32 idx=0;idx<SysMenuSkinAlphaCount;idx++){
    TstructTGF *ptag=&SysMenuSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* SysMenuAlpha_GetSkin(ESysMenuSkinAlpha idx)
{
  TstructTGF *ptag=&SysMenuSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void MoviePlayer_Init(void)
{
  for(u32 idx=0;idx<MoviePlayerSkinCount;idx++){
    TstructB15 *ptag=&MoviePlayerSkin[(u32)idx];
    const char *pfn=NULL;
    switch((EMoviePlayerSkin)idx){
      case EMPS_bg: pfn="mp_bg.b15"; break;
      case EMPS_seekbar_off: pfn="mp_seekbar_off.b15"; break;
      case EMPS_seekbar_on: pfn="mp_seekbar_on.b15"; break;
      case EMPSCount: default: StopFatalError(13709,"MoviePlayer: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadB15(pfn,&ptag->pbm);
  }
}

static void MoviePlayer_Free(void)
{
  for(u32 idx=0;idx<MoviePlayerSkinCount;idx++){
    TstructB15 *ptag=&MoviePlayerSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* MoviePlayer_GetSkin(EMoviePlayerSkin idx)
{
  TstructB15 *ptag=&MoviePlayerSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void MoviePlayerAlpha_Init(void)
{
  for(u32 idx=0;idx<MoviePlayerSkinAlphaCount;idx++){
    TstructTGF *ptag=&MoviePlayerSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((EMoviePlayerSkinAlpha)idx){
      case EMPSA_mode_repeat: pfn="mp_mode_repeat.tgf"; break;
      case EMPSA_mode_allrep: pfn="mp_mode_allrep.tgf"; break;
      case EMPSA_mode_random: pfn="mp_mode_random.tgf"; break;
      case EMPSA_play: pfn="mp_play.tgf"; break;
      case EMPSA_pause: pfn="mp_pause.tgf"; break;
      case EMPSA_stop: pfn="mp_stop.tgf"; break;
      case EMPSA_prev: pfn="mp_prev.tgf"; break;
      case EMPSA_next: pfn="mp_next.tgf"; break;
      case EMPSA_volbar_off: pfn="mp_volbar_off.tgf"; break;
      case EMPSA_volbar_on: pfn="mp_volbar_on.tgf"; break;
      case EMPSA_backlight: pfn="mp_backlight.tgf"; break;
      case EMPSA_seekbargrip: pfn="mp_seekbargrip.tgf"; break;
      case EMPSA_digits: pfn="mp_digits.tgf"; break;
      case EMPSACount: default: StopFatalError(13710,"MoviePlayerAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void MoviePlayerAlpha_Free(void)
{
  for(u32 idx=0;idx<MoviePlayerSkinAlphaCount;idx++){
    TstructTGF *ptag=&MoviePlayerSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* MoviePlayerAlpha_GetSkin(EMoviePlayerSkinAlpha idx)
{
  TstructTGF *ptag=&MoviePlayerSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void ImageView_Init(void)
{
  for(u32 idx=0;idx<ImageViewSkinCount;idx++){
    TstructB15 *ptag=&ImageViewSkin[(u32)idx];
    const char *pfn=NULL;
    switch((EImageViewSkin)idx){
      case EIVS_OverlayBG: pfn="iv_OverlayBG.b15"; break;
      case EIVS_prgbar_off: pfn="iv_prgbar_off.b15"; break;
      case EIVS_prgbar_on: pfn="iv_prgbar_on.b15"; break;
      case EIVSCount: default: StopFatalError(13711,"ImageView: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadB15(pfn,&ptag->pbm);
  }
}

static void ImageView_Free(void)
{
  for(u32 idx=0;idx<ImageViewSkinCount;idx++){
    TstructB15 *ptag=&ImageViewSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* ImageView_GetSkin(EImageViewSkin idx)
{
  TstructB15 *ptag=&ImageViewSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void Launch_Init(void)
{
  CglCanvas *pCustomBG=LoadCustomBG();
  
  for(u32 idx=0;idx<LaunchSkinCount;idx++){
    TstructB15 *ptag=&LaunchSkin[(u32)idx];
    const char *pfn=NULL;
    switch((ELaunchSkin)idx){
      case ELS_BGTop: pfn="Launch_BGTop.b15"; break;
      case ELS_BGBottom: pfn="Launch_BGBottom.b15"; break;
      case ELSCount: default: StopFatalError(13712,"Launch: Unknown type. %d\n",idx); break;
    }
    if((pCustomBG!=NULL)&&((ELaunchSkin)idx==ELS_BGTop)){
      ptag->pbm=new CglB15(&MM_Skin,NULL,ScreenWidth | (ScreenHeight<<16));
      CglCanvas *pcan=ptag->pbm->pCanvas;
      pCustomBG->BitBlt(pcan,0,0,ScreenWidth,ScreenHeight,ScreenWidth,0,false);
      }else{
      if((pCustomBG!=NULL)&&((ELaunchSkin)idx==ELS_BGBottom)){
        ptag->pbm=new CglB15(&MM_Skin,NULL,ScreenWidth | (ScreenHeight<<16));
        CglCanvas *pcan=ptag->pbm->pCanvas;
        pCustomBG->BitBlt(pcan,0,0,ScreenWidth,ScreenHeight,ScreenWidth,ScreenHeight,false);
        }else{
        SkinFile_LoadB15(pfn,&ptag->pbm);
      }
    }
  }
  
  if(pCustomBG!=NULL){
    delete pCustomBG; pCustomBG=NULL;
  }
}

static void Launch_Free(void)
{
  for(u32 idx=0;idx<LaunchSkinCount;idx++){
    TstructB15 *ptag=&LaunchSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* Launch_GetSkin(ELaunchSkin idx)
{
  TstructB15 *ptag=&LaunchSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void LaunchAlpha_Init(void)
{
  for(u32 idx=0;idx<LaunchSkinAlphaCount;idx++){
    TstructTGF *ptag=&LaunchSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((ELaunchSkinAlpha)idx){
      case ELSA_Item_ClearBG: pfn="Launch_Item_ClearBG.tgf"; break;
      case ELSA_Item_SelectBG: pfn="Launch_Item_SelectBG.tgf"; break;
      case ELSA_Tab0_Launch: pfn="Launch_Tab0_Launch.tgf"; break;
      case ELSA_Tab1_NDS: pfn="Launch_Tab1_NDS.tgf"; break;
      case ELSA_FileInfoFrame: pfn="Launch_FileInfoFrame.tgf"; break;
      case ELSACount: default: StopFatalError(13713,"LaunchAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void LaunchAlpha_Free(void)
{
  for(u32 idx=0;idx<LaunchSkinAlphaCount;idx++){
    TstructTGF *ptag=&LaunchSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* LaunchAlpha_GetSkin(ELaunchSkinAlpha idx)
{
  TstructTGF *ptag=&LaunchSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void LongTapAlpha_Init(void)
{
  for(u32 idx=0;idx<LongTapSkinAlphaCount;idx++){
    TstructTGF *ptag=&LongTapSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((ELongTapSkinAlpha)idx){
      case ELTSA_f0: pfn="LongTap_f0.tgf"; break;
      case ELTSA_f1: pfn="LongTap_f1.tgf"; break;
      case ELTSA_f2: pfn="LongTap_f2.tgf"; break;
      case ELTSA_f3: pfn="LongTap_f3.tgf"; break;
      case ELTSA_f4: pfn="LongTap_f4.tgf"; break;
      case ELTSA_f5: pfn="LongTap_f5.tgf"; break;
      case ELTSA_f6: pfn="LongTap_f6.tgf"; break;
      case ELTSA_f7: pfn="LongTap_f7.tgf"; break;
      case ELTSACount: default: StopFatalError(13714,"LongTapAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void LongTapAlpha_Free(void)
{
  for(u32 idx=0;idx<LongTapSkinAlphaCount;idx++){
    TstructTGF *ptag=&LongTapSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* LongTapAlpha_GetSkin(ELongTapSkinAlpha idx)
{
  TstructTGF *ptag=&LongTapSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// -----------------------------

static void MP3CntAlpha_Init(void)
{
  for(u32 idx=0;idx<MP3CntSkinAlphaCount;idx++){
    TstructTGF *ptag=&MP3CntSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((EMP3CntSkinAlpha)idx){
      case EMP3SA_p0_Auto: pfn="MP3Cnt_p0_Auto.tgf"; break;
      case EMP3SA_p0_Lock: pfn="MP3Cnt_p0_Lock.tgf"; break;
      case EMP3SA_p1_prev: pfn="MP3Cnt_p1_prev.tgf"; break;
      case EMP3SA_p2_stop: pfn="MP3Cnt_p2_stop.tgf"; break;
      case EMP3SA_p2_play: pfn="MP3Cnt_p2_play.tgf"; break;
      case EMP3SA_p2_pause: pfn="MP3Cnt_p2_pause.tgf"; break;
      case EMP3SA_p3_next: pfn="MP3Cnt_p3_next.tgf"; break;
      case EMP3SA_p4_repeat: pfn="MP3Cnt_p4_repeat.tgf"; break;
      case EMP3SA_p4_allrep: pfn="MP3Cnt_p4_allrep.tgf"; break;
      case EMP3SA_p4_shuffle: pfn="MP3Cnt_p4_shuffle.tgf"; break;
      case EMP3SACount: default: StopFatalError(13715,"MP3CntAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void MP3CntAlpha_Free(void)
{
  for(u32 idx=0;idx<MP3CntSkinAlphaCount;idx++){
    TstructTGF *ptag=&MP3CntSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* MP3CntAlpha_GetSkin(EMP3CntSkinAlpha idx)
{
  TstructTGF *ptag=&MP3CntSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void Custom_Init(void)
{
  for(u32 idx=0;idx<CustomSkinCount;idx++){
    TstructB15 *ptag=&CustomSkin[(u32)idx];
    const char *pfn=NULL;
    switch((ECustomSkin)idx){
      case ECS_TopMsg: pfn="Custom_TopMsg.b15"; break;
      case ECS_BG: pfn="Custom_BG.b15"; break;
      case ECSCount: default: StopFatalError(13716,"Custom: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadB15(pfn,&ptag->pbm);
  }
}

static void Custom_Free(void)
{
  for(u32 idx=0;idx<CustomSkinCount;idx++){
    TstructB15 *ptag=&CustomSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* Custom_GetSkin(ECustomSkin idx)
{
  TstructB15 *ptag=&CustomSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void StandbyClock_Init(EProcStateScreenSaver ScreenSaver)
{
  const char *pPrefixID=NULL;
  switch(ScreenSaver){
    case EPSSS_Normal: pPrefixID="N"; break;
    case EPSSS_Digital: pPrefixID="D"; break;
    case EPSSS_Extend: pPrefixID="E"; break;
  }
  
  for(u32 idx=0;idx<StandbyClockSkinCount;idx++){
    TstructB15 *ptag=&StandbyClockSkin[(u32)idx];
    const char *pfn=NULL;
    switch((EStandbyClockSkin)idx){
      case ESC_BG: pfn="SC%s_BG.b15"; break;
      case ESCCount: default: StopFatalError(13717,"StandbyClock: Unknown type. %d\n",idx); break;
    }
    char fn[32];
    snprintf(fn,32,pfn,pPrefixID);
    SkinFile_LoadB15(fn,&ptag->pbm);
  }
}

static void StandbyClock_Free(void)
{
  for(u32 idx=0;idx<StandbyClockSkinCount;idx++){
    TstructB15 *ptag=&StandbyClockSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* StandbyClock_GetSkin(EStandbyClockSkin idx)
{
  TstructB15 *ptag=&StandbyClockSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void StandbyClockAlpha_Init(EProcStateScreenSaver ScreenSaver)
{
  const char *pPrefixID=NULL;
  switch(ScreenSaver){
    case EPSSS_Normal: pPrefixID="N"; break;
    case EPSSS_Digital: pPrefixID="D"; break;
    case EPSSS_Extend: pPrefixID="E"; break;
  }
  
  for(u32 idx=0;idx<StandbyClockSkinAlphaCount;idx++){
    TstructTGF *ptag=&StandbyClockSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((EStandbyClockSkinAlpha)idx){
      case ESCA_Font16: pfn="SC%s_Font16.tgf"; break;
      case ESCA_Font24: pfn="SC%s_Font24.tgf"; break;
      case ESCA_Font56: pfn="SC%s_Font56.tgf"; break;
      case ESCA_CalenderFont: pfn="SC%s_CalenderFont.tgf"; break;
      case ESCA_CalenderTodayFont: pfn="SC%s_CalenderTodayFont.tgf"; break;
      case ESCA_WeekStr: pfn="SC%s_WeekStr.tgf"; break;
      case ESCACount: default: StopFatalError(13718,"StandbyClockAlpha: Unknown type. %d\n",idx); break;
    }
    if(pfn!=NULL){
      char fn[32];
      snprintf(fn,32,pfn,pPrefixID);
      SkinFile_LoadTGF(fn,&ptag->pbm);
    }
  }
}

static void StandbyClockAlpha_Free(void)
{
  for(u32 idx=0;idx<StandbyClockSkinAlphaCount;idx++){
    TstructTGF *ptag=&StandbyClockSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* StandbyClockAlpha_GetSkin(EStandbyClockSkinAlpha idx)
{
  TstructTGF *ptag=&StandbyClockSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void TextView_Init(void)
{
  for(u32 idx=0;idx<TextViewSkinCount;idx++){
    TstructB15 *ptag=&TextViewSkin[(u32)idx];
    const char *pfn=NULL;
    switch((ETextViewSkin)idx){
      case ETV_PageBG: pfn="TV_PageBG.b15"; break;
      case ETV_Bookmark_LoadBG: pfn="TV_Bookmark_LoadBG.b15"; break;
      case ETV_Bookmark_SaveBG: pfn="TV_Bookmark_SaveBG.b15"; break;
      case ETV_Bookmark_PreviewBG: pfn="TV_Bookmark_PreviewBG.b15"; break;
      case ETVCount: default: StopFatalError(13719,"TextView: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadB15(pfn,&ptag->pbm);
  }
}

static void TextView_Free(void)
{
  for(u32 idx=0;idx<TextViewSkinCount;idx++){
    TstructB15 *ptag=&TextViewSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* TextView_GetSkin(ETextViewSkin idx)
{
  TstructB15 *ptag=&TextViewSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void TextViewAlpha_Init(void)
{
  for(u32 idx=0;idx<TextViewSkinAlphaCount;idx++){
    TstructTGF *ptag=&TextViewSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((ETextViewSkinAlpha)idx){
      case ETVA_Bookmark_Cursor: pfn="TV_Bookmark_Cursor.tgf"; break;
      case ETVA_Bookmark_Clear: pfn="TV_Bookmark_Clear.tgf"; break;
      case ETVA_Bookmark_Slot0Icon: pfn="TV_Bookmark_Slot0Icon.tgf"; break;
      case ETVA_Bookmark_Slot1Icon: pfn="TV_Bookmark_Slot1Icon.tgf"; break;
      case ETVA_Bookmark_Slot2Icon: pfn="TV_Bookmark_Slot2Icon.tgf"; break;
      case ETVA_Bookmark_Slot3Icon: pfn="TV_Bookmark_Slot3Icon.tgf"; break;
      case ETVACount: default: StopFatalError(13720,"TextViewAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void TextViewAlpha_Free(void)
{
  for(u32 idx=0;idx<TextViewSkinAlphaCount;idx++){
    TstructTGF *ptag=&TextViewSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* TextViewAlpha_GetSkin(ETextViewSkinAlpha idx)
{
  TstructTGF *ptag=&TextViewSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void MemoEdit_Init(void)
{
  for(u32 idx=0;idx<MemoEditSkinCount;idx++){
    TstructB15 *ptag=&MemoEditSkin[(u32)idx];
    const char *pfn=NULL;
    switch((EMemoEditSkin)idx){
      case EME_BGTop: pfn="MemoEdit_BGTop.b15"; break;
      case EME_BGBottom: pfn="MemoEdit_BGBottom.b15"; break;
      case EMECount: default: StopFatalError(0,"MemoEdit: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadB15(pfn,&ptag->pbm);
  }
}

static void MemoEdit_Free(void)
{
  for(u32 idx=0;idx<MemoEditSkinCount;idx++){
    TstructB15 *ptag=&MemoEditSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* MemoEdit_GetSkin(EMemoEditSkin idx)
{
  TstructB15 *ptag=&MemoEditSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void MemoEditAlpha_Init(void)
{
  for(u32 idx=0;idx<MemoEditSkinAlphaCount;idx++){
    TstructTGF *ptag=&MemoEditSkinAlpha[(u32)idx];
    const char *pfn=NULL;
    switch((EMemoEditSkinAlpha)idx){
      case EMEA_FileInfoBG: pfn="MemoEdit_FileInfoBG.tgf"; break;
      case EMEACount: default: StopFatalError(0,"MemoEditAlpha: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadTGF(pfn,&ptag->pbm);
  }
}

static void MemoEditAlpha_Free(void)
{
  for(u32 idx=0;idx<MemoEditSkinAlphaCount;idx++){
    TstructTGF *ptag=&MemoEditSkinAlpha[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglTGF* MemoEditAlpha_GetSkin(EMemoEditSkinAlpha idx)
{
  TstructTGF *ptag=&MemoEditSkinAlpha[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

static void MemoList_Init(void)
{
  for(u32 idx=0;idx<MemoListSkinCount;idx++){
    TstructB15 *ptag=&MemoListSkin[(u32)idx];
    const char *pfn=NULL;
    switch((EMemoListSkin)idx){
      case EML_BG: pfn="MemoList_BG.b15"; break;
      case EMLCount: default: StopFatalError(0,"MemoList: Unknown type. %d\n",idx); break;
    }
    SkinFile_LoadB15(pfn,&ptag->pbm);
  }
}

static void MemoList_Free(void)
{
  for(u32 idx=0;idx<MemoListSkinCount;idx++){
    TstructB15 *ptag=&MemoListSkin[(u32)idx];
    if(ptag->pbm!=NULL){
      delete ptag->pbm; ptag->pbm=NULL;
    }
  }
}

CglB15* MemoList_GetSkin(EMemoListSkin idx)
{
  TstructB15 *ptag=&MemoListSkin[(u32)idx];
  return(ptag->pbm);
}

// ---------------------------------------------------------------------------------

DATA_IN_AfterSystem TSkin_OwnerDrawText Skin_OwnerDrawText;
DATA_IN_AfterSystem TSkin_Calender Skin_Calender;
DATA_IN_AfterSystem TColorTable ColorTable;

#include "skin_colortbl.h"

// ---------------------------------------------------------------------------------

bool Skin_SetFilename(const UnicodeChar *pFullPath)
{
	DATA_IN_AfterSystem static UnicodeChar DefaultFilenameW[64];
  StrConvert_Ank2Unicode(DefaultSkinPath "/default.skn",DefaultFilenameW);
  if(Unicode_isEqual_NoCaseSensitive(pFullPath,DefaultFilenameW)==true){
    {
      char pfn[MaxFilenameLength];
      snprintf(pfn,MaxFilenameLength,DefaultSkinPath "/def_%s.skn",Shell_GetCodePageStr());
      if(FullPath_FileExistsAnsi(pfn)) {
          StrConvert_Ank2Unicode(pfn,DefaultFilenameW);
      }else{
          StrConvert_Ank2Unicode(DefaultSkinPath "/default.skn",DefaultFilenameW);
      }
      pFullPath=DefaultFilenameW;
      _consolePrintf("Default skin alias to '%s'.\n",pfn);
    }
  }

  if(SkinFile_Check(pFullPath)==false){
    _consolePrint("Skin file check failed.\n");
    return(false);
  }
  
  SkinFile_Close();
  
  if(SkinFile_Open(pFullPath)==false){
    _consolePrint("Skin file open failed.\n");
    return(false);
  }
  
  LoadColorTable_colortbl_ini();
  
  return(true);
}

void Skin_CloseFile(void)
{
  SkinFile_Close();
}

void Skin_Load_ChkDsk(void)
{
  extmem_ShowMallocInfo();
}

void Skin_Load_Setup(void)
{
  ComponentAlpha_Init();
  Setup_Init();
  SetupAlpha_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_FileList(void)
{
  ComponentAlpha_Init();
  ScrollBarAlpha_Init();
  FileList_Init();
  LongTapAlpha_Init();
  FileListAlpha_Init(ProcState.FileList.Mode);
  FileListClockAlpha_Init();
  MP3CntAlpha_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_FileList_ScreenSaver(void)
{
  StandbyClock_Free();
  StandbyClockAlpha_Free();
  StandbyClock_Init(ProcState.ScreenSaver.ScreenSaver);
  StandbyClockAlpha_Init(ProcState.ScreenSaver.ScreenSaver);
  extmem_ShowMallocInfo();
}

void Skin_Load_SysMenu(void)
{
  SysMenuAlpha_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_DPGCustom(void)
{
  ComponentAlpha_Init();
  Custom_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_DPGPlay(void)
{
  MoviePlayer_Init();
  MoviePlayerAlpha_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_ImageCustom(void)
{
  ComponentAlpha_Init();
  Custom_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_ImageView(void)
{
  ImageView_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_ImageView_AfterFree(void)
{
  if(ImageViewSkin[EIVS_OverlayBG].pbm!=NULL){
    delete ImageViewSkin[EIVS_OverlayBG].pbm; ImageViewSkin[EIVS_OverlayBG].pbm=NULL;
  }
}

void Skin_Load_TextMenu(void)
{
  SysMenuAlpha_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_TextView(void)
{
  TextView_Init();
  TextViewAlpha_Init();
  if(ProcState.Text.TopScrMode==ETTSM_Clock){
    StandbyClock_Init(ProcState.Text.ScreenSaver);
    StandbyClockAlpha_Init(ProcState.Text.ScreenSaver);
  }
  extmem_ShowMallocInfo();
}

void Skin_Load_BinView(void)
{
  TextView_Init();
  TextViewAlpha_Init();
  if(ProcState.Text.TopScrMode==ETTSM_Clock){
    StandbyClock_Init(ProcState.Text.ScreenSaver);
    StandbyClockAlpha_Init(ProcState.Text.ScreenSaver);
  }
  extmem_ShowMallocInfo();
}

void Skin_Load_TextCustom(void)
{
  ComponentAlpha_Init();
  Custom_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_Launch(void)
{
  Launch_Init();
  LaunchAlpha_Init();
  LongTapAlpha_Init();
  FileListAlpha_Init(EPSFLM_Double);
  extmem_ShowMallocInfo();
}

void Skin_Load_Custom(void)
{
  ComponentAlpha_Init();
  Custom_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_BootROM(void)
{
  extmem_ShowMallocInfo();
}

void Skin_Load_MemoEdit(void)
{
  MemoEdit_Init();
  MemoEditAlpha_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_MemoList(void)
{
  MemoList_Init();
  extmem_ShowMallocInfo();
}

void Skin_Load_AudioCustom(void)
{
  ComponentAlpha_Init();
  Custom_Init();
  extmem_ShowMallocInfo();
}

void Skin_Free(void)
{
  ComponentAlpha_Free();
  FileList_Free();
  FileListAlpha_Free();
  FileListClockAlpha_Free();
  ScrollBarAlpha_Free();
  Setup_Free();
  SetupAlpha_Free();
  SysMenuAlpha_Free();
  MoviePlayer_Free();
  MoviePlayerAlpha_Free();
  ImageView_Free();
  Launch_Free();
  LaunchAlpha_Free();
  LongTapAlpha_Free();
  MP3CntAlpha_Free();
  Custom_Free();
  StandbyClock_Free();
  StandbyClockAlpha_Free();
  TextView_Free();
  TextViewAlpha_Free();
  MemoEdit_Free();
  MemoEditAlpha_Free();
  MemoList_Free();
}

void DrawSkin(CglB15 *psrcbm,CglCanvas *pdstbm,s32 x,s32 y)
{
  psrcbm->pCanvas->BitBlt(pdstbm,x,y,psrcbm->GetWidth(),psrcbm->GetHeight(),0,0,false);
}

void DrawSkinAlpha(CglTGF *psrcbm,CglCanvas *pdstbm,s32 x,s32 y)
{
  psrcbm->BitBlt(pdstbm,x,y);
}

void Skin_ClearCustomBG_FileBody(void)
{
  FAT_FILE *pwfh=Shell_FAT_fopenwrite_Internal(BGBMPFilename);
  if(pwfh==NULL){
    _consolePrint("Open for write failed.\n");
    return;
  }
  
  u32 BGBMPType=EBGBT_None;
  FAT2_fwrite(&BGBMPType,1,4,pwfh);
  
  FAT2_fclose(pwfh);
}

CglTGF* Skin_GetErrorDialogBG(void)
{
  CglTGF *pbm=NULL;
  SkinFile_LoadTGF("ErrorDialogBG.tgf",&pbm);
  return(pbm);
}
