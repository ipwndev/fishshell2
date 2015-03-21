
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"
#include "memtool.h"
#include "arm9tcm.h"
#include "fat2.h"
#include "lang.h"
#include "zlibhelp.h"
#include "splash.h"
#include "dllsound.h"
#include "procstate.h"
#include "datetime.h"

#include "shell.h"
#include "disc_io.h"
#include "../../ipc6.h"

#define CurrentProcStateVersion (0x00020007)

DATA_IN_AfterSystem TProcState ProcState;
DATA_IN_AfterSystem bool ProcState_RequestSave;

DATA_IN_IWRAM_MainPass static void DataInit(void)
{
    TProcState *pstate=&ProcState;
  
    pstate->Version=CurrentProcStateVersion;
  
    {
        TProcState_System *psys=&pstate->System;
        psys->LastPageTab=ELPT_Global;
    
        psys->SkipSetup=false;
        psys->BootCheckDisk=true;
        psys->ClickSound=true;
        psys->EnableFadeEffect=true;
        psys->AutoLastState=true;

        psys->LastState=ELS_FileList;
        psys->AudioVolume64=64;
        psys->VideoVolume64=64;
        psys->BacklightLevel=IPC6->DefaultBrightness;
        psys->SkinFilenameUnicode[0]=0;
    
        psys->LRKeyLockType=ELRLT_RelationalPanel;
        psys->EnableResumeFunction=true;
        psys->EnableScreenCapture=false;
        psys->SpeakerPowerOffWhenPanelClosed=true;
    }
  
    {
        TProcState_FileList *pfl=&pstate->FileList;
        pfl->CurrentPathUnicode[0]=(UnicodeChar)'/';
        pfl->CurrentPathUnicode[1]=(UnicodeChar)0;
        pfl->SelectFilenameUnicode[0]=(UnicodeChar)0;
        pfl->SelectWindowTopOffset=0;
        pfl->Mode=EPSFLM_Double;
        pfl->PlayMode=EPSFLPM_AllRep;
        pfl->LRClickLongSeek=true;
        pfl->MoveFolderLocked=false;
        pfl->HiddenFilenameExt=false;
        pfl->HiddenNotSupportFileType=true;
        pfl->HiddenNDSROMFile=false;
        pfl->EnableFileInfo=true;
        pfl->ShowOnlineHelp_LRButton=true;
        pfl->BButtonToFolderUp=false;
        pfl->EasyDeleteKey=false;
        pfl->UseJpegExifThumbnail=true;
        pfl->ShowCoverImage=true;
    
        pfl->HideAttribute_Archive=false;
        pfl->HideAttribute_Hidden=true;
        pfl->HideAttribute_System=true;
        pfl->HideAttribute_ReadOnly=false;
    }
  
    {
        TProcState_ScreenSaver *pss=&pstate->ScreenSaver;
        pss->EnabledScreenSaver=true;
        pss->Use24hFormat=false;
        pss->ShowClock=true;
        pss->ShowID3Tag=true;
        pss->ShowLyric=true;
        pss->BacklightTimeoutSec=10;
        pss->BacklightOffTimeout=false;
        pss->HideScreenSaverCustom=false;
        pss->EnabledTouchPanelCombination=true;
        pss->ScreenSaver=EPSSS_Normal;
        pss->ScreenSaverBG=EPSSSBG_SkinDefault;
    }
  
    {
        TProcState_Music *pm=&pstate->Music;
        pm->PlayListEnd=EPLE_Loop;
    }
  
    {
        TProcState_Audio *pau=&pstate->Audio;
        pau->AlwaysUsedAudioPlayer=false;
        pau->PauseWhenStopped=false;
        pau->DrawPlayList=true;
        pau->DrawSpectrumAnalyzer=true;
    }
  
    {
        TProcState_DPG *pdpg=&pstate->DPG;
        pdpg->EnabledFastStart=true;
        pdpg->PlayMode=EDPM_AllRep;
        pdpg->BacklightFlag=true;
        pdpg->PauseWhenPanelClosed=true;
        pdpg->EnableIndividualResume=true;
        pdpg->ControlPowerLED=true;
        pdpg->InverseVisual_PlayPause=false;
        pdpg->EverytimeStopOfEnd=false;
        pdpg->WhenLateDecodingWaitPause=false;
        pdpg->BacklightTimeOutSec=10;
    }
  
    {
        TProcState_Image *pimg=&pstate->Image;
        pimg->MultipleFix8=1*0x100;
        pimg->ShowInfomation=true;
        pimg->ShowControlIcons=true;
        pimg->DoubleSpeedKey=false;
        pimg->DoubleSpeedTouch=false;
        pimg->MultipleResume=true;
        pimg->AutoFitting=true;
        pimg->StartPos=EISP_LeftTop;
        pimg->EffectHeightPadding=true;
        pimg->EffectPastelForTopBG=false;
        pimg->EffectPastelForBottomBG=true;
    }
  
    {
        TProcState_Text *ptxt=&pstate->Text;
        ptxt->TopScrMode=ETTSM_Clock;
        ptxt->isSwapDisp=false;
        ptxt->BacklightFlag=true;
        ptxt->ScreenSaver=EPSSS_Normal;
        ptxt->FontSize=Text_FontSize_Middle;
        ptxt->ClearTypeFont=ETCTF_None;
        ptxt->LineSpace=ETLS_Middle;
        ptxt->DefaultCodePage=1252;
        ptxt->UseSmoothScroll=true;
        ptxt->BButtonToExitText=true;
        ptxt->LockScrollBar=false;
        ptxt->DetectCharCode_ANSI=true;
        ptxt->DetectCharCode_EUC=true;
        ptxt->DetectCharCode_UTF16BE=true;
        ptxt->DetectCharCode_UTF16LE=true;
        ptxt->DetectCharCode_UTF8=true;
    }
  
    {
        TProcState_MemoEdit *pmemoedit=&pstate->MemoEdit;
        pmemoedit->TickLine=false;
        pmemoedit->PenColor=EMEPC_Black;
    }
}

#include "procstate_dfs.h"

DATA_IN_IWRAM_MainPass void ProcState_Init(void)
{
    DataInit();
    
    ProcState_RequestSave=false;
  
    pBurstList=NULL;
  
    Splash_Update();
    
    if(VerboseDebugLog==true) _consolePrintf("Open setting file. [%s]\n",DataFilename);
    FAT_FILE *pf=Shell_FAT_fopen_Internal(DataFilename);
    if(pf!=NULL){
		u32 filesize=FAT2_GetFileSize(pf);
		if(filesize==ProcState_ReadWriteSize) CreateBurstList(pf);
		FAT2_fclose(pf);
	}
    
    Splash_Update();
    
    const u32 size=sizeof(ProcState)-ProcState_ReadWriteSize;
    if(VerboseDebugLog==true) _consolePrintf("ProcState: size= %dbyte, R/W size= %dbyte.\n",size,ProcState_ReadWriteSize);
    if((pBurstList==NULL)||(ProcState_ReadWriteSize<size)) StopFatalError(13501,"File not found or size error. [%s] 0x%08x (%d<%d)\n",DataFilename,pBurstList,ProcState_ReadWriteSize,size);
}

void ProcState_Free(void)
{
    ProcState_RequestSave=true;
    ProcState_Save();
    ProcState_DFS_Free();
}

DATA_IN_IWRAM_MainPass void ProcState_Clear(void)
{
    DataInit();
  
    ProcState_RequestSave=true;
}

static void fbuf_LoadFromDisk(void) {
	if(pBurstList==NULL) StopFatalError(13502,"Data sector index is 0 for load.\n");
	ProcState_DFS_Read32bit((u8*)&ProcState,ProcState_ReadWriteSize/SectorSize);
}

static void fbuf_SaveToDisk(void){
	if(pBurstList==NULL) StopFatalError(13503,"Data sector index is 0 for save.\n");
	REG_IME=0;
	DLLSound_UpdateLoop(false);
	ProcState_DFS_Write32bit((u8*)&ProcState,ProcState_ReadWriteSize/SectorSize);
	REG_IME=1;
}

DATA_IN_IWRAM_MainPass void ProcState_Load(void)
{
    if(VerboseDebugLog==true) _consolePrint("Load settings.\n");
  
    fbuf_LoadFromDisk();
    
    Splash_Update();
    
    if(ProcState.Version!=CurrentProcStateVersion){
        _consolePrint("This file is old version setting. load default.\n");
        DataInit();
    }
  
    u32 FontSize=ProcState.Text.FontSize;
    if((FontSize!=Text_FontSize_Small)&&(FontSize!=Text_FontSize_Middle)&&(FontSize!=Text_FontSize_Large)){
        _consolePrintf("Illigal font size detected. reset to middle.\n");
        FontSize=Text_FontSize_Middle;
        ProcState.Text.FontSize=FontSize;
    }
    
    Date_Set24hFormat(ProcState.ScreenSaver.Use24hFormat);
    ProcState_RequestSave=false;
}

static void ProcState_Save_inc_FillFutter(UnicodeChar *pustr)
{
    u32 idx=0;
  
    for(;idx<MaxFilenameLength;idx++){
        if(pustr[idx]==0) break;
    }
    for(;idx<MaxFilenameLength;idx++){
        pustr[idx]=0;
    }
}

void ProcState_Save(void)
{
    if(ProcState_RequestSave==false) return;
    ProcState_RequestSave=false;
    
    if(VerboseDebugLog==true) _consolePrint("Save settings.\n");
  
    ProcState_Save_inc_FillFutter(ProcState.System.SkinFilenameUnicode);
    
    ProcState_Save_inc_FillFutter(ProcState.FileList.CurrentPathUnicode);
    ProcState_Save_inc_FillFutter(ProcState.FileList.SelectFilenameUnicode);
    
    fbuf_SaveToDisk();
  
    if(VerboseDebugLog==true) _consolePrint("Saved setting.\n");
}

void ApplyCurrentBacklightLevel(void)
{
    TProcState_System *psys=&ProcState.System;
    IPC6->Brightness=psys->BacklightLevel;
    if(VerboseDebugLog==true) _consolePrintf("Backlight set to %d.\n",psys->BacklightLevel);
}

void ChangePrevBacklightLevel(void)
{
    TProcState_System *psys=&ProcState.System;
  
    if(psys->BacklightLevel==0){
        psys->BacklightLevel=3;
    }else{
        psys->BacklightLevel--;
    }
  
    ProcState_RequestSave=true;
  
    ApplyCurrentBacklightLevel();
}

void ChangeNextBacklightLevel(void)
{
    TProcState_System *psys=&ProcState.System;
  
    psys->BacklightLevel++;
    if(psys->BacklightLevel==4) psys->BacklightLevel=0;
  
    ProcState_RequestSave=true;
  
    ApplyCurrentBacklightLevel();
}

