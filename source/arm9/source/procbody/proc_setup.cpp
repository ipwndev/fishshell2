
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

#include "lang.h"
#include "skin.h"
#include "component.h"
#include "sndeff.h"

#include "particle.h"

enum ECompLabels {ECLS_FileListModeLbl,ECLSCount};
#define CompLabelsCount (ECLSCount)
DATA_IN_IWRAM_Setup static TComponentLabel CompLabels[CompLabelsCount];

enum ECompChecks {ECCS_BootCheckDiskChk,ECCS_ClickSoundChk,
                  ECCS_FileListMode_SingleRadio,ECCS_FileListMode_DoubleRadio,
                  ECCS_SkipSetupChk,ECCSCount};
#define CompChecksCount (ECCSCount)
DATA_IN_IWRAM_Setup static TComponentCheck CompChecks[CompChecksCount];

enum ECompButtons {ECBS_OKBtn,ECBSCount};
#define CompButtonsCount (ECBSCount)
DATA_IN_IWRAM_Setup static TComponentButton CompButtons[CompButtonsCount];

static void Setting_Redraw(u32 VsyncCount)
{
  {
    CglCanvas *pcan=pScreenMain->pBackCanvas;
    
    CglB15 *pbg=Setup_GetSkin(ESS_BG_Bottom);
    pbg->pCanvas->BitBltFullBeta(pcan);
  }
  
  TProcState_System *psys=&ProcState.System;
  
  CompChecks[ECCS_BootCheckDiskChk].Checked=psys->BootCheckDisk;
  CompChecks[ECCS_ClickSoundChk].Checked=psys->ClickSound;
  CompChecks[ECCS_SkipSetupChk].Checked=psys->SkipSetup;
  
  {
    TProcState_FileList *pfl=&ProcState.FileList;
    
    CompChecks[ECCS_FileListMode_SingleRadio].Checked=false;
    CompChecks[ECCS_FileListMode_DoubleRadio].Checked=false;
    
    switch(pfl->Mode){
      case EPSFLM_Single: CompChecks[ECCS_FileListMode_SingleRadio].Checked=true; break;
      case EPSFLM_Double: CompChecks[ECCS_FileListMode_DoubleRadio].Checked=true; break;
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
  
  CglCanvas *pcan=pScreenMain->pBackCanvas;
  Particle_Update(VsyncCount,pcan);
  
  ScreenMain_Flip_ProcFadeEffect();
}

static void CB_BootCheckDiskChk_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  TProcState_System *psys=&ProcState.System;
  psys->BootCheckDisk=!psys->BootCheckDisk;
  SetProcFadeEffect(EPFE_FastCrossFade);
}

static void CB_ClickSoundChk_Click(void *pComponentButton)
{
  TProcState_System *psys=&ProcState.System;
  psys->ClickSound=!psys->ClickSound;
  Sound_Start(WAVFN_Click);
  SetProcFadeEffect(EPFE_FastCrossFade);
}


static void CB_SkipSetupChk_Click(void *pComponentButton)
{
  TProcState_System *psys=&ProcState.System;
  psys->SkipSetup=!psys->SkipSetup;
  Sound_Start(WAVFN_Click);
  SetProcFadeEffect(EPFE_FastCrossFade);
}

static void CB_FileListMode_SingleRadio_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  TProcState_FileList *pfl=&ProcState.FileList;
  pfl->Mode=EPSFLM_Single;
  SetProcFadeEffect(EPFE_FastCrossFade);
}

static void CB_FileListMode_DoubleRadio_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  TProcState_FileList *pfl=&ProcState.FileList;
  pfl->Mode=EPSFLM_Double;
  SetProcFadeEffect(EPFE_FastCrossFade);
}

static void CB_OKBtn_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  switch(ProcState.System.LastState){
    case ELS_FileList: SetNextProc(ENP_FileList,EPFE_CrossFade); break;
    case ELS_Launch: SetNextProc(ENP_Launch,EPFE_CrossFade); break;
  }
}

static void CompsInit(void)
{
  CglCanvas *pcan=pScreenMain->pBackCanvas;
  
  for(u32 idx=0;idx<CompLabelsCount;idx++){
    ComponentLabel_Init(&CompLabels[idx],pcan);
    CompLabels[idx].TextColor=ColorTable.Setup.Label_Text;
  }
  for(u32 idx=0;idx<CompChecksCount;idx++){
    ComponentCheck_Init(&CompChecks[idx],pcan);
    CompChecks[idx].TextColor=ColorTable.Setup.Check_Text;
  }
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    ComponentButton_Init(&CompButtons[idx],pcan);
    CompButtons[idx].NormalTextColor=ColorTable.Setup.Button_NormalText;
    CompButtons[idx].PressTextColor=ColorTable.Setup.Button_PressText;
  }
  
  s32 chksize=13;
  
  s32 x,y;
  
  x=5;
  y=9;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_BootCheckDiskChk];
    pcc->CallBack_Click=CB_BootCheckDiskChk_Click;
    pcc->pOnIcon=SetupAlpha_GetSkin(ESSA_ChkOverlayOn);
    pcc->pOffIcon=NULL;
    pcc->pMsgUTF8=Lang_GetUTF8("Setup_BootCheckDisk");
    pcc->Rect=CreateRect(x,y,chksize+6,chksize+6);
  }
  
  y+=26;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_ClickSoundChk];
    pcc->CallBack_Click=CB_ClickSoundChk_Click;
    pcc->pOnIcon=SetupAlpha_GetSkin(ESSA_ChkOverlayOn);
    pcc->pOffIcon=NULL;
    pcc->pMsgUTF8=Lang_GetUTF8("Setup_ClickSound");
    pcc->Rect=CreateRect(x,y,chksize+6,chksize+6);
  }
  
  y+=22;
  
  x+=16;
  {
    TComponentLabel *pcl=&CompLabels[ECLS_FileListModeLbl];
    pcl->pMsgUTF8=Lang_GetUTF8("Setup_FileListMode");
    pcl->Rect=CreateRect(x,y,0,0);
  }
  x-=16;
  
  x+=9;
  y+=20;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_FileListMode_SingleRadio];
    pcc->CallBack_Click=CB_FileListMode_SingleRadio_Click;
    pcc->pOnIcon=SetupAlpha_GetSkin(ESSA_RadioOverlayOn);
    pcc->pOffIcon=NULL;
    pcc->pMsgUTF8=Lang_GetUTF8("Setup_FileListMode_Single");
    pcc->Rect=CreateRect(x,y,chksize+6,chksize+6);
  }
  
  y+=22;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_FileListMode_DoubleRadio];
    pcc->CallBack_Click=CB_FileListMode_DoubleRadio_Click;
    pcc->pOnIcon=SetupAlpha_GetSkin(ESSA_RadioOverlayOn);
    pcc->pOffIcon=NULL;
    pcc->pMsgUTF8=Lang_GetUTF8("Setup_FileListMode_Double");
    pcc->Rect=CreateRect(x,y,chksize+6,chksize+6);
  }
  
  x-=9;
  
  y+=40;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_SkipSetupChk];
    pcc->CallBack_Click=CB_SkipSetupChk_Click;
    pcc->pOnIcon=SetupAlpha_GetSkin(ESSA_ChkOverlayOn);
    pcc->pOffIcon=NULL;
    pcc->pMsgUTF8=Lang_GetUTF8("Setup_SkipSetup");
    pcc->Rect=CreateRect(x,y,chksize+6,chksize+6);
  }
  
  x=148;
  y=160;
  
  {
    TComponentButton *pcb=&CompButtons[ECBS_OKBtn];
    pcb->CallBack_Click=CB_OKBtn_Click;
    pcb->pIcon=NULL;
    pcb->pMsgUTF8=Lang_GetUTF8("Setup_OKBtn");
    pcb->Rect=CreateRect(x,y,ScreenWidth-x,ScreenHeight-y);
    pcb->DrawFrame=false;
  }
  
  if(Skin_OwnerDrawText.Setup_Bottom==true){
    for(u32 idx=0;idx<CompLabelsCount;idx++){
      CompLabels[idx].pMsgUTF8="";
    }
    for(u32 idx=0;idx<CompChecksCount;idx++){
      CompChecks[idx].pMsgUTF8="";
    }
    for(u32 idx=0;idx<CompButtonsCount;idx++){
      CompButtons[idx].pMsgUTF8="";
    }
  }
}

// -----------------------------

static void CB_KeyPress(u32 VsyncCount,u32 Keys)
{
  if( ((Keys&KEY_A)!=0) || ((Keys&KEY_B)!=0) ) CB_OKBtn_Click(NULL);
}

DATA_IN_IWRAM_Setup static bool deskmf;
DATA_IN_IWRAM_Setup static TComponentButton *pPressingButton;

static void CB_MouseDown(s32 x,s32 y)
{
  Particle_SetMouseDown(x,y);
  
  deskmf=false;
  
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    TComponentButton *pcb=&CompButtons[idx];
    if(ComponentButton_GetIndexFromPos(pcb,x,y)!=-1){
      pPressingButton=pcb;
      pcb->Pressing=true;
      ComponentButton_Draw(pcb);
//      Setting_Redraw();
    }
  }
  
  deskmf=true;
}

static void CB_MouseMove(s32 x,s32 y)
{
  Particle_SetMouseMove(x,y);
  
  if(deskmf==false) return;
  
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    TComponentButton *pcb=&CompButtons[idx];
    if(pcb==pPressingButton){
      if(ComponentButton_GetIndexFromPos(pcb,x,y)==-1){
        if(pcb->Pressing==true){
          pcb->Pressing=false;
          ComponentButton_Draw(pcb);
//          Setting_Redraw();
        }
        }else{
        if(pcb->Pressing==false){
          pcb->Pressing=true;
          ComponentButton_Draw(pcb);
//          Setting_Redraw();
        }
      }
    }
  }
}

static void CB_MouseUp(s32 x,s32 y)
{
  Particle_SetMouseUp();
  
  if(deskmf==false) return;
  
  deskmf=false;
  
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
    
//    Setting_Redraw();
    pPressingButton=NULL;
    return;
  }
  
  for(u32 idx=0;idx<CompLabelsCount;idx++){
    if(ComponentLabel_MouseUp(&CompLabels[idx],x,y)==true){
      // Setting_Redraw();
    }
  }
  for(u32 idx=0;idx<CompChecksCount;idx++){
    if(ComponentCheck_MouseUp(&CompChecks[idx],x,y)==true){
      // Setting_Redraw();
    }
  }
}

static void DrawOnlineHelp(void)
{
  if(Skin_OwnerDrawText.Setup_Top==true) return;
  
  CglB15 *pb15=Setup_GetSkin(ESS_BG_Top);
  
  pb15->pCanvas->SetCglFont(pCglFontDefault);
  
  u32 x=8;
  u32 y=8;
  u32 h=glCanvasTextHeight+3;
  
  for(u32 idx=1;idx<12;idx++){
    const char *pmsg=NULL;
    switch(idx){
#define Prefix "Setup_"
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
        pb15->pCanvas->SetFontTextColor(ColorTable.Setup.HelpTop_Text);
        }else{
        pb15->pCanvas->SetFontTextColor(ColorTable.Setup.HelpBody_Text);
      }
      pb15->pCanvas->TextOutUTF8(x,y,pmsg);
    }
    y+=h;
  }
}

static void CB_Start(void)
{
  DrawOnlineHelp();
  
  {
    CglCanvas *pcan=pScreenSub->pCanvas;
    
    CglB15 *pbg=Setup_GetSkin(ESS_BG_Top);
    pbg->pCanvas->BitBltFullBeta(pcan);
  }
  
  deskmf=false;
  pPressingButton=NULL;
  
  CompsInit();
  
  Particle_Init();
  
  Sound_Start(WAVFN_Opening);
  
//  Setting_Redraw();
}

static void CB_VsyncUpdate(u32 VsyncCount)
{
  Setting_Redraw(VsyncCount);
}

static void CB_End(void)
{
  ProcState_RequestSave=true;
  ProcState_Save();
  Particle_Free();
}

void ProcSetup_SetCallBack(TCallBack *pCallBack)
{
  pCallBack->Start=CB_Start;
  pCallBack->VsyncUpdate=CB_VsyncUpdate;
  pCallBack->End=CB_End;
  pCallBack->KeyPress=CB_KeyPress;
  pCallBack->MouseDown=CB_MouseDown;
  pCallBack->MouseMove=CB_MouseMove;
  pCallBack->MouseUp=CB_MouseUp;
}

