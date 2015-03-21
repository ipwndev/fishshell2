
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

DATA_IN_IWRAM_ImageCustom static TProcState *pCurrentProcState;

#define ClientHeight (192+192)

// -----------------------------

DATA_IN_IWRAM_ImageCustom static CglCanvas *pScreenBack;
DATA_IN_IWRAM_ImageCustom static s32 ScreenPosY,ShowPosY;

static void ScrollBar_SetPosY(s32 PosY)
{
  ScreenPosY=PosY;
  if(ScreenPosY<0) ScreenPosY=0;
  if((ClientHeight-ScreenHeight)<ScreenPosY) ScreenPosY=ClientHeight-ScreenHeight;
}

// -----------------------------

enum ECompLabels {ECLS_TitleLbl,ECLS_StartPosTitleLbl,ECLS_EffectTitleLbl,ECLSCount};
#define CompLabelsCount (ECLSCount)
DATA_IN_IWRAM_ImageCustom static TComponentLabel CompLabels[CompLabelsCount];

enum ECompChecks {ECCS_ShowInfomationChk,ECCS_ShowControlIconsChk,ECCS_DoubleSpeedKeyChk,ECCS_DoubleSpeedTouchChk,ECCS_MultipleResumeChk,ECCS_AutoFittingChk,
                  ECCS_StartPosLeftTopRadio,ECCS_StartPosRightTopRadio,ECCS_StartPosLeftBottomRadio,ECCS_StartPosRightBottomRadio,
                  ECCS_EffectHeightPaddingChk,ECCS_EffectPastelForTopBGChk,ECCS_EffectPastelForBottomBGChk,
                  ECCSCount};
#define CompChecksCount (ECCSCount)
DATA_IN_IWRAM_ImageCustom static TComponentCheck CompChecks[CompChecksCount];

enum ECompButtons {ECBS_CancelBtn,ECBS_OkBtn,ECBSCount};
#define CompButtonsCount (ECBSCount)
DATA_IN_IWRAM_ImageCustom static TComponentButton CompButtons[CompButtonsCount];

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
  }
  
  for(u32 idx=0;idx<CompChecksCount;idx++){
    TComponentCheck *pcc=&CompChecks[idx];
    bool *pchk=(bool*)pcc->UserData;
    if(pchk!=NULL) pcc->Checked=*pchk;
  }
  
  {
    CompChecks[ECCS_StartPosLeftTopRadio].Checked=false;
    CompChecks[ECCS_StartPosRightTopRadio].Checked=false;
    CompChecks[ECCS_StartPosLeftBottomRadio].Checked=false;
    CompChecks[ECCS_StartPosRightBottomRadio].Checked=false;
    
    switch(pCurrentProcState->Image.StartPos){
      case EISP_LeftTop: CompChecks[ECCS_StartPosLeftTopRadio].Checked=true; break;
      case EISP_RightTop: CompChecks[ECCS_StartPosRightTopRadio].Checked=true; break;
      case EISP_LeftBottom: CompChecks[ECCS_StartPosLeftBottomRadio].Checked=true; break;
      case EISP_RightBottom: CompChecks[ECCS_StartPosRightBottomRadio].Checked=true; break;
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

static void CB_StartPosLeftTopRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Image.StartPos=EISP_LeftTop;
}

static void CB_StartPosRightTopRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Image.StartPos=EISP_RightTop;
}

static void CB_StartPosLeftBottomRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Image.StartPos=EISP_LeftBottom;
}

static void CB_StartPosRightBottomRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Image.StartPos=EISP_RightBottom;
}

static void CB_CancelBtn_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  if(isCustomFromFileList) {
	  isCustomFromFileList=false;
	  SetNextProc(ENP_FileList,EPFE_CrossFade);
  }else{
	  SetNextProc(ENP_ImageView,EPFE_LeftToRight);
  }
}

static void CB_OkBtn_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  
  ProcState=*pCurrentProcState;
  ProcState_RequestSave=true;
  ProcState_Save();
  
  if(isCustomFromFileList) {
	  isCustomFromFileList=false;
	  SetNextProc(ENP_FileList,EPFE_CrossFade);
  }else{
	  SetNextProc(ENP_ImageView,EPFE_LeftToRight);
  }
}

static void CB_CheckBox_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  TComponentCheck *pcc=(TComponentCheck*)pComponentButton;
  bool *pchk=(bool*)pcc->UserData;
  if(pchk!=NULL) *pchk=!*pchk;
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
    pcl->pMsgUTF8=Lang_GetUTF8("ICS_Title");
    pcl->Rect=CreateRect(x+24,y,ScreenWidth-x,h);
    pcl->isTitle=true;
  }
  y+=12*2;
  
  // -----------------
  
  u32 chkw=24,chkh=14;
  
  u32 PaddingY=8;
  
  y+=TextHeight;
  
  TProcState *pcps=pCurrentProcState;
  
  // -----------------
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_ShowInfomationChk];
    pcc->UserData=(u32)&pcps->Image.ShowInfomation;
    pcc->pMsgUTF8=Lang_GetUTF8("ICS_ShowInfomation");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_ShowControlIconsChk];
    pcc->UserData=(u32)&pcps->Image.ShowControlIcons;
    pcc->pMsgUTF8=Lang_GetUTF8("ICS_ShowControlIcons");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_DoubleSpeedKeyChk];
    pcc->UserData=(u32)&pcps->Image.DoubleSpeedKey;
    pcc->pMsgUTF8=Lang_GetUTF8("ICS_DoubleSpeedKey");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_DoubleSpeedTouchChk];
    pcc->UserData=(u32)&pcps->Image.DoubleSpeedTouch;
    pcc->pMsgUTF8=Lang_GetUTF8("ICS_DoubleSpeedTouch");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_MultipleResumeChk];
    pcc->UserData=(u32)&pcps->Image.MultipleResume;
    pcc->pMsgUTF8=Lang_GetUTF8("ICS_MultipleResume");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_AutoFittingChk];
    pcc->UserData=(u32)&pcps->Image.AutoFitting;
    pcc->pMsgUTF8=Lang_GetUTF8("ICS_AutoFitting");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  y+=PaddingY;
  
  // -----------------
  
  {
    TComponentLabel *pcl=&CompLabels[ECLS_StartPosTitleLbl];
    pcl->pMsgUTF8=Lang_GetUTF8("ICS_StartPosTitle");
    pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
  }
  y+=TextHeight*2;
    
  {
    u32 rx=x;
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_StartPosLeftTopRadio];
      pcc->CallBack_Click=CB_StartPosLeftTopRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("ICS_StartPosLeftTop");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_StartPosRightTopRadio];
      pcc->CallBack_Click=CB_StartPosRightTopRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("ICS_StartPosRightTop");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_StartPosLeftBottomRadio];
      pcc->CallBack_Click=CB_StartPosLeftBottomRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("ICS_StartPosLeftBottom");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_StartPosRightBottomRadio];
      pcc->CallBack_Click=CB_StartPosRightBottomRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("ICS_StartPosRightBottom");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    if(rx>ScreenWidth) {
        u32 tx=ComponentLabel_GetWidth(&CompLabels[ECLS_StartPosTitleLbl])+10;
        u32 ty=y-TextHeight*2;
                
        CompChecks[ECCS_StartPosLeftTopRadio].Rect=CreateRect(tx,ty,chkw,chkh);
        tx=x;
        ty+=TextHeight*2;
        CompChecks[ECCS_StartPosRightTopRadio].Rect=CreateRect(tx,ty,chkw,chkh);
        tx+=ComponentCheck_GetWidth(&CompChecks[ECCS_StartPosRightTopRadio]);
        CompChecks[ECCS_StartPosLeftBottomRadio].Rect=CreateRect(tx,ty,chkw,chkh);
        tx+=ComponentCheck_GetWidth(&CompChecks[ECCS_StartPosLeftBottomRadio]);
        CompChecks[ECCS_StartPosRightBottomRadio].Rect=CreateRect(tx,ty,chkw,chkh);
        tx+=ComponentCheck_GetWidth(&CompChecks[ECCS_StartPosRightBottomRadio]);
    }
  }
    
  y+=TextHeight*2;
  
  y+=PaddingY;
  
  // -----------------
  
  {
    TComponentLabel *pcl=&CompLabels[ECLS_EffectTitleLbl];
    pcl->pMsgUTF8=Lang_GetUTF8("ICS_EffectTitle");
    pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
  }
  y+=TextHeight*2;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_EffectHeightPaddingChk];
    pcc->UserData=(u32)&pcps->Image.EffectHeightPadding;
    pcc->pMsgUTF8=Lang_GetUTF8("ICS_EffectHeightPadding");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_EffectPastelForTopBGChk];
    pcc->UserData=(u32)&pcps->Image.EffectPastelForTopBG;
    pcc->pMsgUTF8=Lang_GetUTF8("ICS_EffectPastelForTopBG");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_EffectPastelForBottomBGChk];
    pcc->UserData=(u32)&pcps->Image.EffectPastelForBottomBG;
    pcc->pMsgUTF8=Lang_GetUTF8("ICS_EffectPastelForBottomBG");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
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
    
    u32 size;
    u8 *pbmp=pScreenBack->CreateBMPImage(&size);
    if(pbmp!=NULL){
        FAT_FILE *pf=FAT2_fopen_AliasForWrite("/SS_IMAGE.BMP");
        if(pf!=NULL){
            FAT2_fwrite(pbmp,1,size,pf);
            FAT2_fclose(pf);
        }
        safefree(&MM_Temp,pbmp); pbmp=NULL;
    }
    Sound_Start(WAVFN_Click);
}

DATA_IN_IWRAM_ImageCustom static bool scrmf;
DATA_IN_IWRAM_ImageCustom static s32 scrmy;

DATA_IN_IWRAM_ImageCustom static bool deskmf;
DATA_IN_IWRAM_ImageCustom static TComponentButton *pPressingButton;

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
#define Prefix "ICS_"
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
  
  _consolePrintf("Wait for key releases.\n");
  while(1){
	  if(IPC6->RequestUpdateIPC==false){
		  u32 keys=((~REG_KEYINPUT)&0x3ff) | IPC6->XYButtons;
		  if(keys==0) break;
  		  IPC6->RequestUpdateIPC=true;
	  }
  }
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

void ProcImageCustom_SetCallBack(TCallBack *pCallBack)
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

