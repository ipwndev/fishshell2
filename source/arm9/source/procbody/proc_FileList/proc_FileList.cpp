
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "../../ipc6.h"
#include "datetime.h"
#include "arm9tcm.h"
#include "strpcm.h"
#include "lang.h"

#include "glib.h"

#include "fat2.h"
#include "shell.h"
#include "sndeff.h"
#include "splash.h"
#include "procstate.h"
#include "inifile.h"
#include "strtool.h"
#include "unicode.h"
#include "euc2unicode.h"
#include "rect.h"
#include "skin.h"
#include "cfont.h"
#include "BootROM.h"
#include "ErrorDialog.h"
#include "extlink.h"
#include "msgwin.h"

#include "dll.h"
#include "dllsound.h"
#include "internaldrivers.h"

#include "hiddenpaths.h"

#include "playlist.h"

#include "NDSFiles.h"

#include "ThumbIPK.h"

#include "ThumbDPG.h"

// -----------------------------

DATA_IN_IWRAM_FileList static s32 PlayCursorIndex;

DATA_IN_IWRAM_FileList static bool ScreenRedrawFlag;
DATA_IN_IWRAM_FileList static bool ForceUpdateSubScreenFlag;

DATA_IN_IWRAM_FileList static u32 PanelClosePowerOffTimeOut;
DATA_IN_IWRAM_FileList static u32 PowerOffTimerWhileNoInput;

DATA_IN_IWRAM_FileList static CglCanvas *pSubTempBM;
DATA_IN_IWRAM_FileList static CglCanvas *pDrawItemBM;
// -----------------------------
DATA_IN_IWRAM_FileList static const char *pSlide_Folder_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_Image_Left_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_Image_Right_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_NDS_Left_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_NDS_Right_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_Skin_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_Sound_Left_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_Text_Left_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_Text_Right_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_UpFolder_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_Video_Left_Title;
DATA_IN_IWRAM_FileList static const char *pSlide_Video_Right_Title;
// -----------------------------

static void Backlight_ResetTimer(void);

#include "proc_FileList_SSCustom.h"

DATA_IN_IWRAM_FileList static u32 BacklightTimeout;

static void Backlight_ResetTimer(void)
{
  if(BacklightTimeout==0){
      SSC_Close();
    if(GlobalINI.FileList.SwapTopBottomDisplay==true) REG_POWERCNT|=POWER_SWAP_LCDS;
    IPC6->LCDPowerControl=LCDPC_ON_BOTH;
    ScreenRedrawFlag=true;
    ForceUpdateSubScreenFlag=true;
  }
  BacklightTimeout=ProcState.ScreenSaver.BacklightTimeoutSec*60;
  PowerOffTimerWhileNoInput=GlobalINI.FileList.PowerOffTimerWhileNoInput*60;
}

static void Backlight_SetLast1vsync(void)
{
  BacklightTimeout=(ProcState.ScreenSaver.BacklightTimeoutSec/3)*60;
  if(BacklightTimeout==0) BacklightTimeout=1;
}

static void Backlight_VsyncUpdate(u32 VsyncCount)
{
	if(!ProcState.ScreenSaver.EnabledScreenSaver) {
		BacklightTimeout=ProcState.ScreenSaver.BacklightTimeoutSec*60;
		return;
	}
    if(BacklightTimeout==0){
        SSC_VsyncUpdate(VsyncCount);
        return;
    }
  
  if(BacklightTimeout<=VsyncCount){
    BacklightTimeout=0;
    }else{
    BacklightTimeout-=VsyncCount;
  }
  
  if(BacklightTimeout==0){
    if(GlobalINI.FileList.SwapTopBottomDisplay==true) REG_POWERCNT&=~POWER_SWAP_LCDS;
    if(PanelClosePowerOffTimeOut==0) SSC_Open();
  }
}

static bool Backlight_isStandby(void)
{
  if(BacklightTimeout==0) return(true);
  return(false);
}

// -----------------------------

DATA_IN_IWRAM_FileList static EProcStateFileListMode FileList_Mode;

static void MoveUpFolder(void);
static void MoveFolder(void);
static void StartApplication(void);
static void CustomApplication(void);
static void LongTapApplication(void);

DATA_IN_IWRAM_FileList static bool HPSwitch_ProcessLong,HPSwitch_ProcessSingleLong;
DATA_IN_IWRAM_FileList static bool MP3Cnt_SeekNext,MP3Cnt_SeekPrev;
DATA_IN_IWRAM_FileList static bool Process_SeekNext,Process_SeekPrev;
DATA_IN_IWRAM_FileList static bool Process_DrawHelpLeft,Process_DrawHelpRight;
DATA_IN_IWRAM_FileList static u32 Process_WaitCount;


#include "proc_FileList_LongTapState.h"

#include "proc_FileList_ScrollBar.h"
#include "proc_FileList_Clock.h"
#include "proc_FileList_FileList.h"
#include "proc_FileList_Popup.h"

// -----------------------------

static void ProcState_RefreshSave(void)
{
  if(ProcState_RequestSave==false) return;
  
  u32 fileidx=ScrollBar.SelectedIndex;
  TNDSFile *pndsf=NDSFiles_GetFileBody(fileidx);
  
  Unicode_Copy(ProcState.FileList.SelectFilenameUnicode,pndsf->pFilenameUnicode);
  ProcState.FileList.SelectWindowTopOffset=(ScrollBar.ItemHeight*fileidx)-ScrollBar.TopPos;
  ProcState.FileList.Mode=FileList_Mode;
  
  ProcState_Save();
  
  ChangedCurrentPath=true;
}

// -----------------------------

static void FileListInit(void)
{
  //PrfStart();
  NDSFiles_RefreshCurrentFolder();
  //PrfEnd(0);
  
  s32 ItemHeight=0,SlideWidth=0;
  
  switch(FileList_Mode){
    case EPSFLM_Single: {
      ItemHeight=NDSROMIcon16Height+2;
      SlideWidth=32;
    } break;
    case EPSFLM_Double: {
      ItemHeight=NDSROMIcon32Height+2;
      SlideWidth=64;
    } break;
  }
  
  ScrollBar_Free(&ScrollBar);
  ScrollBar_Init(&ScrollBar,ItemHeight,SlideWidth);
  
  ScrollBar.TopPos=0;
  ScrollBar.ShowPos=ScrollBar.TopPos;
  ScrollBar.MaxPos=ScrollBar.ItemHeight*NDSFiles_GetFilesCount();
  
  const u32 cnt=NDSFiles_GetFilesCount();
  for(u32 idx=0;idx<cnt;idx++){
      DLLSound_UpdateLoop(true);
    TNDSFile *pndsf=NDSFiles_GetFileBody(idx);
    
    if(Unicode_isEqual(pndsf->pFilenameUnicode,ProcState.FileList.SelectFilenameUnicode)==true){
      ScrollBar_SetSelectedIndex(&ScrollBar,idx);
      ScrollBar_SetDirectTopPos(&ScrollBar,(ScrollBar.ItemHeight*idx)-ProcState.FileList.SelectWindowTopOffset);
      ScrollBar.ShowPos=ScrollBar.TopPos;
    }
  }
}

#include "proc_FileList_MoveFolder.h"
#include "proc_FileList_SelEnc_ui.h"

static void StartApplication(void)
{
  ProcState_RequestSave=true;
  ProcState_RefreshSave();
  
  RequestRefreshPlayCursorIndex=true;
  ScreenRedrawFlag=true;
  ForceUpdateSubScreenFlag=true;
  
  TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
  
  switch(pndsf->FileType){
    case ENFFT_UnknownFile: break;
    case ENFFT_UpFolder: MoveUpFolder(); return; 
    case ENFFT_Folder: MoveFolder(); return;
    case ENFFT_Sound: {
      if(DLLList_isSupportFormatExt32(pndsf->Ext32)==EPT_Sound){
        PlayList_Free();
        PlayList_MakeFolder(false,ProcState.FileList.CurrentPathUnicode);
        PlayList_Start(false,ProcState.FileList.CurrentPathUnicode,pndsf->pFilenameUnicode);
        ChangedCurrentPath=true;
        return;
      }
    } break;
    case ENFFT_PlayList: {
      PlayList_Free();
      
      ChangedCurrentPath=true;
      PlayList_ConvertM3U(ProcState.FileList.CurrentPathUnicode,pndsf->pFilenameUnicode);
      
      if(PlayList_Start(true,NULL,NULL)==false){
        Sound_Start(WAVFN_Notify);
        ErrorDialog_Set(EEC_NotFoundMusicFile);
        ErrorDialog_Draw(pScreenMainOverlay->pCanvas);
        return;
      }
      
      ChangedCurrentPath=true;
      return;
    } 
    case ENFFT_Image: {
      Sound_Start(WAVFN_Click);
      ProcState_RequestSave=true;
      Unicode_Copy(RelationalFilePathUnicode,ProcState.FileList.CurrentPathUnicode);
      Unicode_Copy(RelationalFileNameUnicode,pndsf->pFilenameUnicode);
      RelationalFilePos=0;
      SetNextProc(ENP_ImageView,EPFE_None);
      return;
    } 
    case ENFFT_Text: {
        if(pndsf->FileSize==0){
            Sound_Start(WAVFN_Notify);
            ErrorDialog_Set(EEC_Text0byte);
            ErrorDialog_Draw(pScreenMainOverlay->pCanvas);
            return;
        }
        Sound_Start(WAVFN_Click);
        ProcState_RequestSave=true;
        Unicode_Copy(RelationalFilePathUnicode,ProcState.FileList.CurrentPathUnicode);
        Unicode_Copy(RelationalFileNameUnicode,pndsf->pFilenameUnicode);
        RelationalFilePos=0;
        ManualTextEncode=ETE_Auto;
        ManualTextEncode_OverrideFlag=false;
        SetNextProc(ENP_TextView,EPFE_CrossFade);
        return;
    } 
    case ENFFT_Binary: {
    	if(pndsf->FileSize==0){
    		Sound_Start(WAVFN_Notify);
    		ErrorDialog_Set(EEC_Text0byte);
    		ErrorDialog_Draw(pScreenMainOverlay->pCanvas);
    		return;
    	}
    	Sound_Start(WAVFN_Click);
    	ProcState_RequestSave=true;
    	Unicode_Copy(RelationalFilePathUnicode,ProcState.FileList.CurrentPathUnicode);
    	Unicode_Copy(RelationalFileNameUnicode,pndsf->pFilenameUnicode);
    	RelationalFilePos=0;
    	SetNextProc(ENP_BinView,EPFE_CrossFade);
    	return;
    } 
    case ENFFT_Video: {
      Sound_Start(WAVFN_Click);
      ProcState_RequestSave=true;
      Unicode_Copy(RelationalFilePathUnicode,ProcState.FileList.CurrentPathUnicode);
      Unicode_Copy(RelationalFileNameUnicode,pndsf->pFilenameUnicode);
      RelationalFilePos=0;
      SetNextProc(ENP_DPGPlay,EPFE_CrossFade);
      return;
    } 
    case ENFFT_NDSROM: {
      Sound_Start(WAVFN_Click);
      ProcState_RequestSave=true;
      BootROM_SetInfo(ProcState.FileList.CurrentPathUnicode,pndsf->pFilenameUnicode,true);
      return;
    } 
    case ENFFT_GBAROM: {
    	Sound_Start(WAVFN_Click);
    	ProcState_RequestSave=true;
    	BootROM_SetInfo(ProcState.FileList.CurrentPathUnicode,pndsf->pFilenameUnicode,false);
    	return;
    } 
    case ENFFT_Skin: {
      const UnicodeChar *pFullPathUnicode=FullPathUnicodeFromSplitItem(ProcState.FileList.CurrentPathUnicode,pndsf->pFilenameUnicode);
      
      if(Skin_SetFilename(pFullPathUnicode)==true){
        Unicode_Copy(ProcState.System.SkinFilenameUnicode,pFullPathUnicode);
        ProcState_RequestSave=true;
      
        SetNextProc(ENP_FileList,EPFE_CrossFade);
        return;
      }
    } break;
  }
  
  Sound_Start(WAVFN_Notify);
  ChangedCurrentPath=true;
  _consolePrint("StartApplication: Not support function.\n");
}

static void CustomApplication(void)
{
  TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
  
  switch(pndsf->FileType){
    case ENFFT_UnknownFile: break;
    case ENFFT_UpFolder: StartApplication(); return;
    case ENFFT_Folder: StartApplication(); return;
    case ENFFT_Sound: {
      if(PlayList_isOpened()==false) StartApplication();
      MP3Cnt_WindowState=EMCWS_Lock;
      MP3Cnt_PosX=MP3Cnt_Width;
      MP3Cnt_LockTimeOut=MP3Cnt_LockTimeOutSetup;
      ScreenRedrawFlag=true;
      ProcState_RequestSave=true;
      return;
    } 
    case ENFFT_Image: {
    	Sound_Start(WAVFN_Click);
    	isCustomFromFileList=true;
    	ProcState_RequestSave=true;
    	SetNextProc(ENP_ImageCustom,EPFE_None);
    	return;
    }
    case ENFFT_Text: {
    	Sound_Start(WAVFN_Click);
    	ProcState_RequestSave=true;
    	Unicode_Copy(RelationalFilePathUnicode,ProcState.FileList.CurrentPathUnicode);
    	Unicode_Copy(RelationalFileNameUnicode,pndsf->pFilenameUnicode);
    	RelationalFilePos=0;
    	ManualTextEncode=ETE_Auto;
    	ManualTextEncode_OverrideFlag=false;
    	SetNextProc(ENP_TextMenu,EPFE_CrossFade);
    	return;
    } 
    case ENFFT_Video: {
    	Sound_Start(WAVFN_Click);
    	isCustomFromFileList=true;
    	ProcState_RequestSave=true;
    	SetNextProc(ENP_DPGCustom,EPFE_None);
    	return; 
    }
    case ENFFT_NDSROM: {
    	if(!pndsf->isNDSCommercialROM) {
    		StartApplication(); return; 
    	}
    	Sound_Start(WAVFN_Click);
    	//ProcState_RequestSave=true;
    	//Unicode_Copy(RelationalFilePathUnicode,ProcState.FileList.CurrentPathUnicode);
    	//Unicode_Copy(RelationalFileNameUnicode,pndsf->pFilenameUnicode);
    	//RelationalFilePos=0;
    	//SetNextProc(ENP_ROMCustom,EPFE_CrossFade);
    	return;
    }
    case ENFFT_Skin: StartApplication(); return; 
  }
  
  Sound_Start(WAVFN_MovePage);
  ChangedCurrentPath=true;
  
  _consolePrintf("CustomApplication: Not support function.\n");
  ProcState_RequestSave=true;
}

static void LongTapApplication(void)
{
  TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
  
  switch(pndsf->FileType){
    case ENFFT_UnknownFile: break;
    case ENFFT_UpFolder: break;
    case ENFFT_Folder: break;
    case ENFFT_Sound: break;
    case ENFFT_Image: {
    	Sound_Start(WAVFN_Click);
    	isCustomFromFileList=true;
    	ProcState_RequestSave=true;
    	SetNextProc(ENP_ImageCustom,EPFE_None);
    	return;
    }
    case ENFFT_Text: {
    	Sound_Start(WAVFN_Click);
    	ProcState_RequestSave=true;
    	Unicode_Copy(RelationalFilePathUnicode,ProcState.FileList.CurrentPathUnicode);
    	Unicode_Copy(RelationalFileNameUnicode,pndsf->pFilenameUnicode);
    	RelationalFilePos=0;
    	ManualTextEncode=ETE_Auto;
    	ManualTextEncode_OverrideFlag=false;
    	SetNextProc(ENP_TextMenu,EPFE_CrossFade);
    	return;
    } 
    case ENFFT_Video: {
    	Sound_Start(WAVFN_Click);
    	isCustomFromFileList=true;
    	ProcState_RequestSave=true;
    	SetNextProc(ENP_DPGCustom,EPFE_None);
    	return; 
    }
    case ENFFT_NDSROM: {
    	if(!pndsf->isNDSCommercialROM) {
    		break;
    	}
    	Sound_Start(WAVFN_Click);
    	//ProcState_RequestSave=true;
    	//Unicode_Copy(RelationalFilePathUnicode,ProcState.FileList.CurrentPathUnicode);
    	//Unicode_Copy(RelationalFileNameUnicode,pndsf->pFilenameUnicode);
    	//RelationalFilePos=0;
    	//SetNextProc(ENP_ROMCustom,EPFE_CrossFade);
    	return;
    }
    case ENFFT_Skin: break;
  }
  
  Sound_Start(WAVFN_Notify);
  ChangedCurrentPath=true;
  
  //_consolePrintf("LongTapApplication: Not support function.\n");
  ProcState_RequestSave=true;
}
// -----------------------------

static void MP3Cnt_Exec_Prev(void)
{
	if(PlayList_GetFilesCount()==0) return;
  Sound_Start(WAVFN_Click);
  Popup_Show_Prev();
  
  switch(ProcState.FileList.PlayMode){
  	case EPSFLPM_Repeat: {
  		PlayList_Repeat();
  	} break;
  	case EPSFLPM_AllRep:  {
  		PlayList_Prev();
  	} break;
  	case EPSFLPM_Shuffle: {
  		PlayList_PrevShuffle();
  	} break;
  }
}

static void MP3Cnt_Exec_Next(void)
{
  if(PlayList_GetFilesCount()==0) return;
  Popup_Show_Next();
  
  switch(ProcState.FileList.PlayMode){
    case EPSFLPM_Repeat: {
      PlayList_Repeat();
    } break;
    case EPSFLPM_AllRep: {
      PlayList_Next();
    } break;
    case EPSFLPM_Shuffle: {
      PlayList_NextShuffle();
    } break;
  }
}

static void MP3Cnt_Exec_ChangePause(void)
{
	if(PlayList_GetFilesCount()==0) return;
  if(PlayList_GetPause()==false) Sound_Start(WAVFN_Click);
  PlayList_TogglePause();
  Popup_Show_Pause();
}

static void MP3Cnt_Exec_ChangeAuto(void)
{
  Sound_Start(WAVFN_Click);
  if(MP3Cnt_AutoFlag==true){
    MP3Cnt_AutoFlag=false;
    }else{
    MP3Cnt_AutoFlag=true;
  }
  Popup_Show_MP3CntLock();
}

static void MP3Cnt_Exec_ChangePlayMode(void)
{
  TProcState_FileList *pfl=&ProcState.FileList;
  
  switch(pfl->PlayMode){
    case EPSFLPM_Repeat: pfl->PlayMode=EPSFLPM_AllRep; break;
    case EPSFLPM_AllRep: pfl->PlayMode=EPSFLPM_Shuffle; break;
    case EPSFLPM_Shuffle: pfl->PlayMode=EPSFLPM_Repeat; break;
    default: pfl->PlayMode=EPSFLPM_Repeat; break;
  }
  
  Popup_Show_PlayMode();
  
  ProcState_RequestSave=true;
  ProcState_Save();
}

// -----------------------------

static void CB_ExternalPowerAttach(void)
{
  if(PanelClosePowerOffTimeOut!=0){
    if(BacklightTimeout==0){
    	if(ProcState.ScreenSaver.BacklightOffTimeout){
    		IPC6->LCDPowerControl=LCDPC_OFF_BOTH;
    	}else{
    		IPC6->LCDPowerControl=LCDPC_ON_TOP;
    	}
      }else{
      IPC6->LCDPowerControl=LCDPC_ON_BOTH;
    }
    strpcm_ExclusivePause=false;
    PanelClosePowerOffTimeOut=0;
    REG_POWERCNT |= POWER_LCD;
  }
}

static void CB_ExternalPowerDetach(void)
{
  IPC6->LCDPowerControl=LCDPC_OFF_BOTH;
  strpcm_ExclusivePause=true;
  PanelClosePowerOffTimeOut=10*60*60;
  REG_POWERCNT &= ~POWER_LCD;
}

static void CB_PanelClose(void)
{
  if(GlobalINI.FileList.CarSupplyMode==true){
    CB_ExternalPowerDetach();
  }
}

static void CB_PanelOpen(void)
{
  CB_ExternalPowerAttach();
}

// -----------------------------

#include "proc_FileList_CB_KeyPress.h"

// -----------------------------

static void CB_KeySameLRDown(void)
{
  if((IPC6->PanelOpened==false)&&(ProcState.System.LRKeyLockType==ELRLT_RelationalPanel)||(ProcState.System.LRKeyLockType==ELRLT_AlwayOff)) return;
  MP3Cnt_Exec_ChangePause();
  ScreenRedrawFlag=true;
}

#include "proc_FileList_CB_Mouse.h"

static void DrawOnlineHelp(void)
{
  if(Skin_OwnerDrawText.FileList_Top==true) return;
  
  CglB15 *pb15=FileList_GetSkin(EFLS_BG_TopMsg);
  
  pb15->pCanvas->SetCglFont(pCglFontDefault);
  
  u32 x=8;
  u32 y=8;
  u32 h=glCanvasTextHeight+3;
  
  for(u32 idx=0;idx<12;idx++){
    const char *pmsg=NULL;
    switch(idx){
#define Prefix "FL_"
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
        pb15->pCanvas->SetFontTextColor(ColorTable.FileList.HelpTop_Text);
        }else{
        pb15->pCanvas->SetFontTextColor(ColorTable.FileList.HelpBody_Text);
      }
      pb15->pCanvas->TextOutUTF8(x,y,pmsg);
    }
    y+=h;
  }
}

static void CB_Start(void)
{
  if(GlobalINI.FileList.SwapTopBottomDisplay==true) REG_POWERCNT|=POWER_SWAP_LCDS;
  
  if(ErrorDialog_isExists()==false) Sound_Start(WAVFN_Open);
  ChangedCurrentPath=true;
  
  DrawOnlineHelp();
  
  HiddenPaths_Init();
  
  RequestRefreshPlayCursorIndex=false;
  PlayCursorIndex=-1;

  MP3Cnt_SeekNext=false;
  MP3Cnt_SeekPrev=false;
  Process_SeekNext=false;
  Process_SeekPrev=false;
  Process_DrawHelpLeft=false;
  Process_DrawHelpRight=false;
  Process_WaitCount=0;

  PanelClosePowerOffTimeOut=0;
  
  PowerOffTimerWhileNoInput=GlobalINI.FileList.PowerOffTimerWhileNoInput*60;
  
  if(Skin_OwnerDrawText.SlideTitle==false) {
	  pSlide_Folder_Title=Lang_GetUTF8("FL_Slide_Folder");
	  pSlide_Image_Left_Title=Lang_GetUTF8("FL_Slide_Image_Left");
	  pSlide_Image_Right_Title=Lang_GetUTF8("FL_Slide_Image_Right");
	  pSlide_NDS_Left_Title=Lang_GetUTF8("FL_Slide_NDS_Left");
	  pSlide_NDS_Right_Title=Lang_GetUTF8("FL_Slide_NDS_Right");
	  pSlide_Skin_Title=Lang_GetUTF8("FL_Slide_Skin");
	  pSlide_Sound_Left_Title=Lang_GetUTF8("FL_Slide_Sound_Left");
	  pSlide_Text_Left_Title=Lang_GetUTF8("FL_Slide_Text_Left");
	  pSlide_Text_Right_Title=Lang_GetUTF8("FL_Slide_Text_Right");
	  pSlide_UpFolder_Title=Lang_GetUTF8("FL_Slide_UpFolder");
	  pSlide_Video_Left_Title=Lang_GetUTF8("FL_Slide_Video_Left");
	  pSlide_Video_Right_Title=Lang_GetUTF8("FL_Slide_Video_Right");
  }
  
  InitIPK();
  InitDPGThumb();
  
  MP3Cnt_WindowState=EMCWS_Hide;
  MP3Cnt_PosX=0;
  MP3Cnt_AutoFlag=true;
  MP3Cnt_LockTimeOut=0;
    
  MP3Cnt_PosForVSync=1;

  pScreenMainOverlay->pCanvas->FillFull(0);
  pScreenMainOverlay->SetPosition_for_Right64x192(0,0);
  pScreenMainOverlay->SetVisible_for_LeftTop128x64(false);
  
  if(ErrorDialog_isExists()==true){
    Sound_Start(WAVFN_Notify);
    ChangedCurrentPath=true;
    ErrorDialog_Draw(pScreenMainOverlay->pCanvas);
  }
  
  Popup_Init();
  Clock_Init();
  Clock_Refresh();
  Backlight_ResetTimer();
  
  FileList_Mode=ProcState.FileList.Mode;
  FileListInit();
  isPressMouseButton=false;
  isPressMouseButtonAndLRKey=false;
  
  isCustomFromFileList=false;
  
  ChangedCurrentPath=true;
  
  ScreenRedrawFlag=true;
  ForceUpdateSubScreenFlag=true;
 
  pSubTempBM=new CglCanvas(&MM_Process,NULL,ScreenWidth,ScreenHeight,pf15bit);
  pDrawItemBM=new CglCanvas(&MM_Process,NULL,ScreenWidth+(ScrollBar.SlideWidth*2),ScrollBar.ItemHeight,pf15bit);

  if(ScreenRedrawFlag==true){
    ScreenRedrawFlag=false;
    ForceUpdateSubScreenFlag=false;
    FileList_SubDrawBG(&ScrollBar);
    FileList_MainDrawBG(&ScrollBar);
    FileList_MainDrawBG(&ScrollBar);
  }
  
  Skin_Load_FileList_ScreenSaver();
  
  if((Unicode_isEmpty(RelationalFilePathUnicode)==false)||(Unicode_isEmpty(RelationalFileNameUnicode)==false)) StopFatalError(0,"Relational file error.\n");
  
  if(PlayList_isOpened()==false){
    if(PlayList_Start(true,NULL,NULL)==true){
      Backlight_SetLast1vsync();
      
      RequestRefreshPlayCursorIndex=true;
    }
  }
  
  ChangedCurrentPath=true;
  
  if(ScreenRedrawFlag==true){
    ScreenRedrawFlag=false;
    ForceUpdateSubScreenFlag=false;
    FileList_SubDrawBG(&ScrollBar);
    FileList_MainDrawBG(&ScrollBar);
    FileList_MainDrawBG(&ScrollBar);
  }
  
  SSC_deskmf=false;
  SSC_pPressingButton=NULL;
    
  SSC_CompsInit();
  
  LongTapState_Init();
}

#include "proc_FileList_CB_VsyncUpdate.h"

static void CB_End(void)
{
  TCallBack *pCallBack=CallBack_GetPointer();
  pCallBack->VBlankHandler=NULL;
  
  Popup_Free();
  
  MP3Cnt_WindowState=EMCWS_Hide;
  MP3Cnt_PosX=0;
  MP3Cnt_PosForVSync=0;
  
  pScreenMainOverlay->pCanvas->FillFull(0);
  pScreenMainOverlay->SetPosition_for_Right64x192(0,0);
  pScreenMainOverlay->SetVisible_for_LeftTop128x64(true);
  
  ProcState_RefreshSave();
  
  HiddenPaths_Free();
  
  Clock_Free();
  NDSFiles_Free();
  ScrollBar_Free(&ScrollBar);
  
  LongTapState_Free();
  
  if(pSubTempBM!=NULL){
	  delete pSubTempBM; pSubTempBM=NULL;
  }
  
  if(pDrawItemBM!=NULL){
	  delete pDrawItemBM; pDrawItemBM=NULL;
  }
  
  FreeIPK();
  FreeDPGThumb();
  
  if(GlobalINI.FileList.SwapTopBottomDisplay==true) REG_POWERCNT&=~POWER_SWAP_LCDS;
}

static void CB_VBlankHandler(void)
{
  s32 posx=MP3Cnt_PosX;
  
  if(MP3Cnt_WindowState==EMCWS_Hide){
    if(0<posx){
      posx=(posx*15)/16;
    }
  }
  
  if(MP3Cnt_WindowState==EMCWS_Lock){
      if(posx<MP3Cnt_Width){
        s32 x=MP3Cnt_Width-posx;
        x=(x*15)/16;
        posx=MP3Cnt_Width-x;
      }
    }
    
    if(MP3Cnt_WindowState==EMCWS_Link){
      s32 x=-ScrollBar.SelectXOfs;
      if(x<0) x=0;
      if(MP3Cnt_Width<x) x=MP3Cnt_Width;
      s32 v=posx-x;
      if(v!=0){
        v/=2;
        if(v==0){
          posx=x;
          }else{
          posx-=v;
        }
      }
    }
    
    MP3Cnt_PosX=posx;
  
  if(posx!=MP3Cnt_PosForVSync){
    MP3Cnt_PosForVSync=posx;
    if(posx<0) posx=0;
    if(MP3Cnt_Width<posx) posx=MP3Cnt_Width;
    pScreenMainOverlay->SetPosition_for_Right64x192(MP3Cnt_Width-posx,0);
  }
}

#include "proc_FileList_DeleteFileDialog.h"

static void CB_MWin_ProgressShow(const char *pTitleStr,s32 Max)
{
  msgwin_Clear();
  
  if(MP3Cnt_WindowState==EMCWS_Hide){
    CglCanvas *pcan=pScreenMainOverlay->pCanvas;
    pcan->SetColor(0);
    pcan->FillFast(0,0,ScreenWidth,ScreenHeight);
    pScreenMainOverlay->SetPosition_for_Right64x192(0,0);
  }
}

static void CB_MWin_ProgressSetPos(const char *pTitleStr,s32 pos, s32 max)
{
  char str[64];
  snprintf(str,64,"%d/%d %d%%",pos,max,pos*100/max);
  msgwin_Draw(pTitleStr,str,pos,max);
}

static void CB_MWin_ProgressDraw(const char *pstr0,const char *pstr1,s32 pos,s32 max)
{
  msgwin_Draw(pstr0,pstr1,pos,max);
}

static void CB_MWin_ProgressHide(void)
{
  msgwin_Clear();
  
  if(MP3Cnt_WindowState==EMCWS_Hide){
    pScreenMainOverlay->SetPosition_for_Right64x192(64,0);
  }
}

#include "proc_FileList_Trigger_CallBack.h"

void ProcFileList_SetCallBack(TCallBack *pCallBack)
{
  pCallBack->Start=CB_Start;
  pCallBack->VsyncUpdate=CB_VsyncUpdate;
  pCallBack->End=CB_End;
  pCallBack->KeyPress=CB_KeyPress;
  pCallBack->KeyLongPress=CB_KeyLongPress;
  pCallBack->KeyReleases=CB_KeyReleases;
  pCallBack->KeySameLRDown=CB_KeySameLRDown;
  pCallBack->MouseDown=CB_MouseDown;
  pCallBack->MouseMove=CB_MouseMove;
  pCallBack->MouseUp=CB_MouseUp;
  pCallBack->VBlankHandler=CB_VBlankHandler;
  pCallBack->PanelClose=CB_PanelClose;
  pCallBack->PanelOpen=CB_PanelOpen;
  
  pCallBack->Trigger_ProcStart=CB_Trigger_ProcStart;
  pCallBack->Trigger_ProcEnd=CB_Trigger_ProcEnd;
  pCallBack->Trigger_Down=CB_Trigger_Down;
  pCallBack->Trigger_Up=CB_Trigger_Up;
  pCallBack->Trigger_LongStart=CB_Trigger_LongStart;
  pCallBack->Trigger_LongEnd=CB_Trigger_LongEnd;
  pCallBack->Trigger_SingleClick=CB_Trigger_SingleClick;
  pCallBack->Trigger_SingleLongStart=CB_Trigger_SingleLongStart;
  pCallBack->Trigger_SingleLongEnd=CB_Trigger_SingleLongEnd;
  pCallBack->Trigger_DoubleClick=CB_Trigger_DoubleClick;
  pCallBack->Trigger_DoubleLongStart=CB_Trigger_DoubleLongStart;
  pCallBack->Trigger_DoubleLongEnd=CB_Trigger_DoubleLongEnd;
  pCallBack->Trigger_TripleClick=CB_Trigger_TripleClick;
  
  pCallBack->ExternalPowerAttach=CB_ExternalPowerAttach;
  pCallBack->ExternalPowerDetach=CB_ExternalPowerDetach;
  
  pCallBack->MWin_ProgressShow=CB_MWin_ProgressShow;
  pCallBack->MWin_ProgressSetPos=CB_MWin_ProgressSetPos;
  pCallBack->MWin_ProgressDraw=CB_MWin_ProgressDraw;
  pCallBack->MWin_ProgressHide=CB_MWin_ProgressHide;
}

/*
    case ENFFT_Text: {
      // あとでロングタップしたときのサブメニューで選択できるようにする。
      ManualTextEncode=ETE_Auto;
      ManualTextEncode_OverrideFlag=false;
      return;
    } break;
*/
