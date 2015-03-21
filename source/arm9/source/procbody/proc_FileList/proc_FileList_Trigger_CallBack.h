
#include "PlaySE.h"

//#define EnableTriggerLog

static void CB_Trigger_ins_PlaySE(ETriggerType TriggerType,u32 Count,bool AddLong)
{
//  if(TriggerType!=ETT_PhoneSwitch) return; // HPSwitch以外なら帰る
  
  if(AddLong==true) return; // ロングモードは効果音を再生しないで帰る
  
  PlaySE_Sequence(Count,AddLong);
}

DATA_IN_IWRAM_FileList static bool ReqDisStandby;

static inline bool CB_Trigger_ins_CheckEnabled(ETriggerType TriggerType)
{
  if(ReqDisStandby==true) return(false);
  if(TriggerType==ETT_AButton) return(false);
  
  if((TriggerType==ETT_LButton)||(TriggerType==ETT_RButton)){
      if((IPC6->PanelOpened==false)&&(ProcState.System.LRKeyLockType==ELRLT_RelationalPanel)||(ProcState.System.LRKeyLockType==ELRLT_AlwayOff)) return(false);
  }

  if((TriggerType==ETT_Touch)&&((SSC_Timeout!=0)||(BacklightTimeout!=0)||(!ProcState.ScreenSaver.EnabledTouchPanelCombination))) return(false);
  
  return(true);
}

static void CB_Trigger_ProcStart(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_ProcStart(...); called.\n");
#endif
  
  ReqDisStandby=false;
  if(PanelClosePowerOffTimeOut!=0) ReqDisStandby=true;
  
  Process_SeekNext=false;
  Process_SeekPrev=false;
  Process_DrawHelpLeft=false;
  Process_DrawHelpRight=false;
  Process_WaitCount=0;
}

static void CB_Trigger_ProcEnd(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_ProcEnd(...); called.\n");
#endif
  
  if((Process_DrawHelpLeft==true)||(Process_DrawHelpRight==true)) ScreenRedrawFlag=true;
  
  Process_SeekNext=false;
  Process_SeekPrev=false;
  Process_DrawHelpLeft=false;
  Process_DrawHelpRight=false;
  Process_WaitCount=0;
  
  if(ReqDisStandby==true){
    ReqDisStandby=false;
    CB_ExternalPowerAttach();
    return;
  }
}

static void CB_Trigger_Down(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_Down(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
}

static void CB_Trigger_Up(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_Up(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
}

static void CB_Trigger_LongStart(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_LongStart(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
  CB_Trigger_ins_PlaySE(TriggerType,0,true);
  
  switch(TriggerType){
    case ETT_AButton: break;
    case ETT_LButton: {
      if(ProcState.FileList.LRClickLongSeek==false) Process_SeekPrev=true;
      if(ProcState.FileList.ShowOnlineHelp_LRButton==true) Process_DrawHelpLeft=true;
      ScreenRedrawFlag=true;
    } break;
    case ETT_RButton: {
      if(ProcState.FileList.LRClickLongSeek==false) Process_SeekNext=true;
      if(ProcState.FileList.ShowOnlineHelp_LRButton==true) Process_DrawHelpRight=true;
      ScreenRedrawFlag=true;
    } break;
    case ETT_Touch: {
    	Process_SeekNext=true;
    } break;
    case ETT_PhoneSwitch: {
      Process_SeekNext=true;
    } break;
  }
}

static void CB_Trigger_LongEnd(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_LongEnd(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
}

static void CB_Trigger_SingleClick(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_SingleClick(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
//  CB_Trigger_ins_PlaySE(TriggerType,1,false);
  
  switch(TriggerType){
    case ETT_AButton: break;
    case ETT_LButton: case ETT_RButton: {
    } break;
    case ETT_Touch: {
    	MP3Cnt_Exec_Next();
    	ScreenRedrawFlag=true;
    } break;
    case ETT_PhoneSwitch: {
      if(GlobalINI.FileList.CarSupplyMode==true){
        s32 Volume=ProcState.System.AudioVolume64;
        
        if(strpcmGetAudioVolume64()==Volume){
          Volume-=10;
          if(Volume<0) Volume=0;
        }
        strpcmSetAudioVolume64(Volume);
        ScreenRedrawFlag=true;
        ForceUpdateSubScreenFlag=true;
        }else{
        MP3Cnt_Exec_Next();
        ScreenRedrawFlag=true;
      }
    } break;
  }
}

static void CB_Trigger_SingleLongStart(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_SingleLongStart(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
  CB_Trigger_ins_PlaySE(TriggerType,1,true);
  
  switch(TriggerType){
    case ETT_AButton: break;
    case ETT_LButton: {
      Process_SeekPrev=true;
    } break;
    case ETT_RButton: {
      Process_SeekNext=true;
    } break;
    case ETT_Touch: {
    	Process_SeekPrev=true;
    } break;
    case ETT_PhoneSwitch: {
      Process_SeekPrev=true;
    } break;
  }
}

static void CB_Trigger_SingleLongEnd(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_SingleLongEnd(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
}

static void CB_Trigger_DoubleClick(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_DoubleClick(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
  CB_Trigger_ins_PlaySE(TriggerType,2,false);
  
  switch(TriggerType){
    case ETT_AButton: break;
    case ETT_LButton: {
      MP3Cnt_Exec_Prev();
      ScreenRedrawFlag=true;
    } break;
    case ETT_RButton: {
      MP3Cnt_Exec_Next();
      ScreenRedrawFlag=true;
    } break;
    case ETT_Touch: {
    	MP3Cnt_Exec_Prev();
    	ScreenRedrawFlag=true;
    } break;
    case ETT_PhoneSwitch: {
      if(GlobalINI.FileList.CarSupplyMode==true){
        CB_ExternalPowerDetach();
        }else{
        MP3Cnt_Exec_Prev();
        ScreenRedrawFlag=true;
      }
    } break;
  }
}

static void CB_Trigger_DoubleLongStart(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_DoubleLongStart(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
  CB_Trigger_ins_PlaySE(TriggerType,2,true);
  
  switch(TriggerType){
    case ETT_AButton: break;
    case ETT_LButton: {
    } break;
    case ETT_RButton: {
    } break;
    case ETT_Touch: break;
    case ETT_PhoneSwitch: {
    } break;
  }
}

static void CB_Trigger_DoubleLongEnd(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_DoubleLongEnd(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
}

static void CB_Trigger_TripleClick(ETriggerType TriggerType)
{
#ifdef EnableTriggerLog
  _consolePrint("HPSwitch_TripleClick(...); called.\n");
#endif
  
  if(CB_Trigger_ins_CheckEnabled(TriggerType)==false) return;
  CB_Trigger_ins_PlaySE(TriggerType,3,false);
  
  switch(TriggerType){
    case ETT_AButton: break;
    case ETT_LButton: {
      MP3Cnt_Exec_ChangePause();
      ScreenRedrawFlag=true;
    } break;
    case ETT_RButton: case ETT_Touch: {
      CB_ExternalPowerDetach();
    } break;
    case ETT_PhoneSwitch: {
      if(GlobalINI.FileList.CarSupplyMode==true){
        MP3Cnt_Exec_Next();
        }else{
        CB_ExternalPowerDetach();
      }
    } break;
  }
}

