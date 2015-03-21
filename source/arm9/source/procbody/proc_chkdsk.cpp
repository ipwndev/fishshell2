
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "lang.h"
#include "../../ipc6.h"

#include "glib.h"

#include "fat2.h"
#include "shell.h"
#include "sndeff.h"
#include "splash.h"
#include "procstate.h"
#include "strpcm.h"

#include "inifile.h"

static void CheckRootFiles(void)
{
  if(FAT2_chdir_Alias("/")==false) StopFatalError(16101,"Can not change root path.\n");
  
  u32 filescount=0;
  
  {
    DATA_IN_IWRAM_ChkDsk static const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    while(FAT_FileType!=FT_NONE){
      filescount++;
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }

  if(filescount<=1) StopFatalError(16102,"A necessary file doesn't suffice.\n");
}

// -----------------------------

#define GlobalBGColor15 (RGB15(31,30,30) | BIT15)
#define GlobalTextColor15 (RGB15(3,2,2) | BIT15)

#include "proc_chkdsk_body.h"

static void CB_Start(void)
{
  CheckRootFiles();
  
  {
    CglCanvas *pCanvas=pScreenMain->pBackCanvas;
    
    pCanvas->FillFull(GlobalBGColor15);
    
    pCanvas->SetCglFont(pCglFontDefault);
    pCanvas->SetFontTextColor(GlobalTextColor15);
    
    char msg[64];
    snprintf(msg,64,"%s %s",ROMTITLE,ROMVERSION);
    pCanvas->TextOutA(16,8+(16*0),msg);
    pCanvas->TextOutA(16,8+(16*1),ROMDATE);
    pCanvas->TextOutA(16,8+(16*2),ROMWEB);
    
    pCanvas->TextOutUTF8(16,8+(16*4),Lang_GetUTF8("CD_CheckDisk_Start"));

    ScreenMain_Flip_ProcFadeEffect();
  }
  
//  chkdsk_GetTotalFilesCount();
  chkdsk_DuplicateCluster();
  chkdsk_CheckFATType();
  chkdsk_WriteTest();
  chkdsk_Read16bitTest();
  
  {
    CglCanvas *pCanvas=pScreenMain->pViewCanvas;
    pCanvas->TextOutUTF8(16,8+(16*10),Lang_GetUTF8("CD_CheckDisk_End"));
  }
}

static void CB_VsyncUpdate(u32 VsyncCount)
{
  if(ProcState.System.SkipSetup==false){
    SetNextProc(ENP_Setup,EPFE_CrossFade);
    }else{
    switch(ProcState.System.LastState){
      case ELS_FileList: SetNextProc(ENP_FileList,EPFE_CrossFade); break;
      case ELS_Launch: SetNextProc(ENP_Launch,EPFE_CrossFade); break;
    }
  }
}

static void CB_End(void)
{
}

void ProcChkDsk_SetCallBack(TCallBack *pCallBack)
{
  pCallBack->Start=CB_Start;
  pCallBack->VsyncUpdate=CB_VsyncUpdate;
  pCallBack->End=CB_End;
}

