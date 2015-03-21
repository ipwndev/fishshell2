
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

DATA_IN_IWRAM_TextCustom static TProcState *pCurrentProcState;

#define ClientHeight (192+192+128)

// -----------------------------

DATA_IN_IWRAM_TextCustom static CglCanvas *pScreenBack;
DATA_IN_IWRAM_TextCustom static s32 ScreenPosY,ShowPosY;

static void ScrollBar_SetPosY(s32 PosY)
{
  ScreenPosY=PosY;
  if(ScreenPosY<0) ScreenPosY=0;
  if((ClientHeight-ScreenHeight)<ScreenPosY) ScreenPosY=ClientHeight-ScreenHeight;
}

// -----------------------------

enum ECompLabels {ECLS_TitleLbl,ECLS_TopScrModeTitleLbl,ECLS_ScreenSaverTitleLbl,ECLS_LineSpaceTitleLbl,ECLS_FontSizeTitleLbl,ECLS_ClearTypeFontTitleLbl,ECLS_CPTitleLbl,ECLS_DetectCharCodeTitleLbl,ECLSCount};
#define CompLabelsCount (ECLSCount)
DATA_IN_IWRAM_TextCustom static TComponentLabel CompLabels[CompLabelsCount];

enum ECompChecks {ECCS_TopScrModeLightOffRadio,ECCS_TopScrModeTextRadio,ECCS_TopScrModeClockRadio,ECCS_isSwapDisp,
                  ECCS_ScreenSaverNormalRadio,ECCS_ScreenSaverDigitalRadio,ECCS_ScreenSaverExtendRadio,
                  ECCS_FontSizeSmallRadio,ECCS_FontSizeMiddleRadio,ECCS_FontSizeLargeRadio,
                  ECCS_ClearTypeFontNoneRadio,ECCS_ClearTypeFontLiteRadio,ECCS_ClearTypeFontNormalRadio,ECCS_ClearTypeFontHeavyRadio,
                  ECCS_LineSpaceSmallRadio,ECCS_LineSpaceMiddleRadio,ECCS_LineSpaceLargeRadio,
                  ECCS_CP437Radio,ECCS_CP850Radio,ECCS_CP1252Radio,
                  ECCS_UseSmoothScrollChk,ECCS_BButtonToExitTextChk,ECCS_LockScrollBarChk,
                  ECCS_DetectCharCode_ANSIChk,ECCS_DetectCharCode_EUCChk,ECCS_DetectCharCode_UTF16BEChk,ECCS_DetectCharCode_UTF16LEChk,ECCS_DetectCharCode_UTF8Chk,
                  ECCSCount};
#define CompChecksCount (ECCSCount)
DATA_IN_IWRAM_TextCustom static TComponentCheck CompChecks[CompChecksCount];

enum ECompButtons {ECBS_CancelBtn,ECBS_OkBtn,ECBSCount};
#define CompButtonsCount (ECBSCount)
DATA_IN_IWRAM_TextCustom static TComponentButton CompButtons[CompButtonsCount];

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
    s32 x=ScreenWidth-2-w,y=posy/2.6;
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
    CompChecks[ECCS_LineSpaceSmallRadio].Checked=false;
    CompChecks[ECCS_LineSpaceMiddleRadio].Checked=false;
    CompChecks[ECCS_LineSpaceLargeRadio].Checked=false;
    
    switch(pCurrentProcState->Text.LineSpace){
      case ETLS_Small: CompChecks[ECCS_LineSpaceSmallRadio].Checked=true; break;
      case ETLS_Middle: CompChecks[ECCS_LineSpaceMiddleRadio].Checked=true; break;
      case ETLS_Large: CompChecks[ECCS_LineSpaceLargeRadio].Checked=true; break;
      default: break;
    }
  }
  
  {
    CompChecks[ECCS_TopScrModeTextRadio].Checked=false;
    CompChecks[ECCS_TopScrModeClockRadio].Checked=false;
    CompChecks[ECCS_TopScrModeLightOffRadio].Checked=false;
    
    switch(pCurrentProcState->Text.TopScrMode){
      case ETTSM_Text: {
          CompChecks[ECCS_TopScrModeTextRadio].Checked=true; 
          CompChecks[ECCS_isSwapDisp].Checked=false;
          CompChecks[ECCS_isSwapDisp].Visible=false;
      }break;
      case ETTSM_Clock: CompChecks[ECCS_TopScrModeClockRadio].Checked=true; break;
      case ETTSM_LightOff: CompChecks[ECCS_TopScrModeLightOffRadio].Checked=true; break;
      default: break;
    }
  }
  
  {
    CompChecks[ECCS_ScreenSaverNormalRadio].Checked=false;
    CompChecks[ECCS_ScreenSaverDigitalRadio].Checked=false;
    CompChecks[ECCS_ScreenSaverExtendRadio].Checked=false;
    
    switch(pCurrentProcState->Text.ScreenSaver){
      case EPSSS_Normal: CompChecks[ECCS_ScreenSaverNormalRadio].Checked=true; break;
      case EPSSS_Digital: CompChecks[ECCS_ScreenSaverDigitalRadio].Checked=true; break;
      case EPSSS_Extend: CompChecks[ECCS_ScreenSaverExtendRadio].Checked=true; break;
      default: break;
    }
  }
  
  {
    CompChecks[ECCS_FontSizeSmallRadio].Checked=false;
    CompChecks[ECCS_FontSizeMiddleRadio].Checked=false;
    CompChecks[ECCS_FontSizeLargeRadio].Checked=false;
    
    switch(pCurrentProcState->Text.FontSize){
      case Text_FontSize_Small: CompChecks[ECCS_FontSizeSmallRadio].Checked=true; break;
      case Text_FontSize_Middle: CompChecks[ECCS_FontSizeMiddleRadio].Checked=true; break;
      case Text_FontSize_Large: CompChecks[ECCS_FontSizeLargeRadio].Checked=true; break;
      default: break;
    }
  }
  
  {
    CompChecks[ECCS_ClearTypeFontNoneRadio].Checked=false;
    CompChecks[ECCS_ClearTypeFontLiteRadio].Checked=false;
    CompChecks[ECCS_ClearTypeFontNormalRadio].Checked=false;
    CompChecks[ECCS_ClearTypeFontHeavyRadio].Checked=false;
    
    switch(pCurrentProcState->Text.ClearTypeFont){
      case ETCTF_None: CompChecks[ECCS_ClearTypeFontNoneRadio].Checked=true; break;
      case ETCTF_Lite: CompChecks[ECCS_ClearTypeFontLiteRadio].Checked=true; break;
      case ETCTF_Normal: CompChecks[ECCS_ClearTypeFontNormalRadio].Checked=true; break;
      case ETCTF_Heavy: CompChecks[ECCS_ClearTypeFontHeavyRadio].Checked=true; break;
      default: break;
    }
  }
  
  {
    CompChecks[ECCS_CP437Radio].Checked=false;
    CompChecks[ECCS_CP850Radio].Checked=false;
    CompChecks[ECCS_CP1252Radio].Checked=false;
    
    switch(pCurrentProcState->Text.DefaultCodePage){
      case 437: CompChecks[ECCS_CP437Radio].Checked=true; break;
      case 850: CompChecks[ECCS_CP850Radio].Checked=true; break;
      case 1252: CompChecks[ECCS_CP1252Radio].Checked=true; break;
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

static void CB_TopScrModeLightOffRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.TopScrMode=ETTSM_LightOff;
  TComponentCheck *pcc=&CompChecks[ECCS_isSwapDisp];
  pcc->Visible=true;
}

static void CB_TopScrModeTextRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.TopScrMode=ETTSM_Text;
  pCurrentProcState->Text.isSwapDisp=false;
  TComponentCheck *pcc=&CompChecks[ECCS_isSwapDisp];
  pcc->Checked=false;
  pcc->Visible=false;
}

static void CB_TopScrModeClockRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.TopScrMode=ETTSM_Clock;
  TComponentCheck *pcc=&CompChecks[ECCS_isSwapDisp];
  pcc->Visible=true;
}

static void CB_isSwapDisp_Click(void *pComponentButton)
{
    Sound_Start(WAVFN_Click);
    TComponentCheck *pcc=(TComponentCheck*)pComponentButton;
    bool *pchk=(bool*)pcc->UserData;
    if(pchk!=NULL) *pchk=!*pchk;
}

static void CB_ScreenSaverNormalRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.ScreenSaver=EPSSS_Normal;
}

static void CB_ScreenSaverDigitalRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.ScreenSaver=EPSSS_Digital;
}

static void CB_ScreenSaverExtendRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.ScreenSaver=EPSSS_Extend;
}

static void CB_LineSpaceSmallRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.LineSpace=ETLS_Small;
}

static void CB_LineSpaceMiddleRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.LineSpace=ETLS_Middle;
}

static void CB_LineSpaceLargeRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.LineSpace=ETLS_Large;
}

static void CB_FontSizeSmallRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.FontSize=Text_FontSize_Small;
}

static void CB_FontSizeMiddleRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.FontSize=Text_FontSize_Middle;
}

static void CB_FontSizeLargeRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.FontSize=Text_FontSize_Large;
}

static void CB_ClearTypeFontNoneRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.ClearTypeFont=ETCTF_None;
}

static void CB_ClearTypeFontLiteRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.ClearTypeFont=ETCTF_Lite;
}

static void CB_ClearTypeFontNormalRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.ClearTypeFont=ETCTF_Normal;
}

static void CB_ClearTypeFontHeavyRadio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.ClearTypeFont=ETCTF_Heavy;
}

static void CB_CP437Radio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.DefaultCodePage=437;
}

static void CB_CP850Radio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.DefaultCodePage=850;
}

static void CB_CP1252Radio_Click(void *pComponentCheck)
{
  Sound_Start(WAVFN_Click);
  pCurrentProcState->Text.DefaultCodePage=1252;
}

static void CB_CancelBtn_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  SetNextProc(ENP_TextView,EPFE_LeftToRight);
}

static void CB_OkBtn_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  
  ProcState=*pCurrentProcState;
  ProcState_RequestSave=true;
  ProcState_Save();
  
  SetNextProc(ENP_TextView,EPFE_LeftToRight);
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
    pcl->pMsgUTF8=Lang_GetUTF8("TCS_Title");
    pcl->Rect=CreateRect(x+24,y,ScreenWidth-x,h);
    pcl->isTitle=true;
  }
  y+=12*2;

  // -----------------
  
  u32 chkw=24,chkh=14;
  
  u32 PaddingY=7;
  
  y+=TextHeight;
  
  TProcState *pcps=pCurrentProcState;
  
  // -----------------
  
  {
    u32 rx=x;
    {
      TComponentLabel *pcl=&CompLabels[ECLS_TopScrModeTitleLbl];
      pcl->pMsgUTF8=Lang_GetUTF8("TCS_TopScrModeTitle");
      pcl->Rect=CreateRect(rx,y,ScreenWidth-rx,h);
      rx+=ComponentLabel_GetWidth(pcl);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_isSwapDisp];
      pcc->CallBack_Click=CB_isSwapDisp_Click;
      pcc->UserData=(u32)&pcps->Text.isSwapDisp;
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_isSwapDisp");
      pcc->Rect=CreateRect(rx+2,y-2,chkw,chkh);
    }
  }
  y+=TextHeight+PaddingY;
    
  {
    u32 rx=x;
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_TopScrModeLightOffRadio];
      pcc->CallBack_Click=CB_TopScrModeLightOffRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_TopScrModeLightOff");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_TopScrModeTextRadio];
      pcc->CallBack_Click=CB_TopScrModeTextRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_TopScrModeText");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_TopScrModeClockRadio];
      pcc->CallBack_Click=CB_TopScrModeClockRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_TopScrModeClock");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
  }
  
  y+=TextHeight*2+1;
  
  
  // -----------------
    {
      TComponentLabel *pcl=&CompLabels[ECLS_ScreenSaverTitleLbl];
      pcl->pMsgUTF8=Lang_GetUTF8("TCS_ScreenSaverTitle");
      pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
    }
  y+=TextHeight+PaddingY;
          
  {
    u32 rx=x;
    
    
  
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ScreenSaverNormalRadio];
      pcc->CallBack_Click=CB_ScreenSaverNormalRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_ScreenSaverNormal");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ScreenSaverDigitalRadio];
      pcc->CallBack_Click=CB_ScreenSaverDigitalRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_ScreenSaverDigital");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ScreenSaverExtendRadio];
      pcc->CallBack_Click=CB_ScreenSaverExtendRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_ScreenSaverExtend");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
  }
  
  y+=TextHeight*2+1;
  
  // -----------------
    {
      TComponentLabel *pcl=&CompLabels[ECLS_LineSpaceTitleLbl];
      pcl->pMsgUTF8=Lang_GetUTF8("TCS_LineSpaceTitle");
      pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
    }
    
  y+=TextHeight+PaddingY;
  
  {
    u32 rx=x;
    
    
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_LineSpaceSmallRadio];
      pcc->CallBack_Click=CB_LineSpaceSmallRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_LineSpaceSmall");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_LineSpaceMiddleRadio];
      pcc->CallBack_Click=CB_LineSpaceMiddleRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_LineSpaceMiddle");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_LineSpaceLargeRadio];
      pcc->CallBack_Click=CB_LineSpaceLargeRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_LineSpaceLarge");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
  }
  
  y+=TextHeight*2+1;
  
  // -----------------
  
  {
      TComponentLabel *pcl=&CompLabels[ECLS_FontSizeTitleLbl];
      pcl->pMsgUTF8=Lang_GetUTF8("TCS_FontSizeTitle");
      pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
    }
    
    y+=TextHeight+PaddingY;
    
  {
    u32 rx=x;
    
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_FontSizeSmallRadio];
      pcc->CallBack_Click=CB_FontSizeSmallRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_FontSizeSmall");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_FontSizeMiddleRadio];
      pcc->CallBack_Click=CB_FontSizeMiddleRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_FontSizeMiddle");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_FontSizeLargeRadio];
      pcc->CallBack_Click=CB_FontSizeLargeRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_FontSizeLarge");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    if(rx>ScreenWidth) {
        u32 tx=ComponentLabel_GetWidth(&CompLabels[ECLS_FontSizeTitleLbl])+10;
        u32 ty=y-TextHeight-PaddingY;
                                
        CompChecks[ECCS_FontSizeSmallRadio].Rect=CreateRect(tx,ty,chkw,chkh);
        tx=x;
        ty+=TextHeight+PaddingY;
        CompChecks[ECCS_FontSizeMiddleRadio].Rect=CreateRect(tx,ty,chkw,chkh);
        tx+=ComponentCheck_GetWidth(&CompChecks[ECCS_FontSizeMiddleRadio]);
        CompChecks[ECCS_FontSizeLargeRadio].Rect=CreateRect(tx,ty,chkw,chkh);
        tx+=ComponentCheck_GetWidth(&CompChecks[ECCS_FontSizeLargeRadio]);
    }
  }
  
  y+=TextHeight*2+1;
  
  // -----------------
  
  {
      TComponentLabel *pcl=&CompLabels[ECLS_ClearTypeFontTitleLbl];
      pcl->pMsgUTF8=Lang_GetUTF8("TCS_ClearTypeFontTitle");
      pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
    }
    
    y+=TextHeight+PaddingY;
  {
    u32 rx=x;
    
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ClearTypeFontNoneRadio];
      pcc->CallBack_Click=CB_ClearTypeFontNoneRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_ClearTypeFontNone");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ClearTypeFontLiteRadio];
      pcc->CallBack_Click=CB_ClearTypeFontLiteRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_ClearTypeFontLite");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ClearTypeFontNormalRadio];
      pcc->CallBack_Click=CB_ClearTypeFontNormalRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_ClearTypeFontNormal");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ClearTypeFontHeavyRadio];
      pcc->CallBack_Click=CB_ClearTypeFontHeavyRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_ClearTypeFontHeavy");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
  }
  
  y+=TextHeight*2+1;
  
  // -----------------
  
  {
    u32 rx=x;
    
    {
      TComponentLabel *pcl=&CompLabels[ECLS_CPTitleLbl];
      pcl->pMsgUTF8=Lang_GetUTF8("TCS_CPTitle");
      pcl->Rect=CreateRect(rx,y,ScreenWidth-x,h);
      rx+=ComponentLabel_GetWidth(pcl);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_CP437Radio];
      pcc->CallBack_Click=CB_CP437Radio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_CP437");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    y+=TextHeight+PaddingY;
    
    rx=x;
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_CP850Radio];
      pcc->CallBack_Click=CB_CP850Radio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_CP850");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_CP1252Radio];
      pcc->CallBack_Click=CB_CP1252Radio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_CP1252");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
  }
    
  y+=TextHeight*2;
  
  {
      TComponentCheck *pcc=&CompChecks[ECCS_UseSmoothScrollChk];
      pcc->UserData=(u32)&pcps->Text.UseSmoothScroll;
      pcc->pMsgUTF8=Lang_GetUTF8("TCS_UseSmoothScroll");
      pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2-2;
  
  {
        TComponentCheck *pcc=&CompChecks[ECCS_BButtonToExitTextChk];
        pcc->UserData=(u32)&pcps->Text.BButtonToExitText;
        pcc->pMsgUTF8=Lang_GetUTF8("TCS_BButtonToExitText");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2-2;
  
  {
        TComponentCheck *pcc=&CompChecks[ECCS_LockScrollBarChk];
        pcc->UserData=(u32)&pcps->Text.LockScrollBar;
        pcc->pMsgUTF8=Lang_GetUTF8("TCS_LockScrollBar");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2-2;
    
  // -----------------
  
  {
    TComponentLabel *pcl=&CompLabels[ECLS_DetectCharCodeTitleLbl];
    pcl->pMsgUTF8=Lang_GetUTF8("TCS_DetectCharCodeTitle");
    pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
  }
  y+=TextHeight*2-2;
  
  {
    u32 rx=x;
    {
      TComponentCheck *pcc=&CompChecks[ECCS_DetectCharCode_ANSIChk];
      pcc->UserData=(u32)&pcps->Text.DetectCharCode_ANSI;
      pcc->pMsgUTF8="ANSI (8bit ASCII)";
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_DetectCharCode_UTF8Chk];
      pcc->UserData=(u32)&pcps->Text.DetectCharCode_UTF8;
      pcc->pMsgUTF8="UTF-8";
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
    }
  }
  y+=TextHeight+PaddingY+1;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_DetectCharCode_EUCChk];
    pcc->UserData=(u32)&pcps->Text.DetectCharCode_EUC;
    pcc->pMsgUTF8="EUC/S-JIS";
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+PaddingY+1;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_DetectCharCode_UTF16BEChk];
    pcc->UserData=(u32)&pcps->Text.DetectCharCode_UTF16BE;
    pcc->pMsgUTF8="UTF-16 Big Endian";
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+PaddingY+1;
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_DetectCharCode_UTF16LEChk];
    pcc->UserData=(u32)&pcps->Text.DetectCharCode_UTF16LE;
    pcc->pMsgUTF8="UTF-16 Little Endian";
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight*2;
  
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
        FAT_FILE *pf=FAT2_fopen_AliasForWrite("/SS_TEXT.BMP");
        if(pf!=NULL){
            FAT2_fwrite(pbmp,1,size,pf);
            FAT2_fclose(pf);
        }
        safefree(&MM_Temp,pbmp); pbmp=NULL;
    }
    Sound_Start(WAVFN_Click);
}

DATA_IN_IWRAM_TextCustom static bool scrmf;
DATA_IN_IWRAM_TextCustom static s32 scrmy;

DATA_IN_IWRAM_TextCustom static bool deskmf;
DATA_IN_IWRAM_TextCustom static TComponentButton *pPressingButton;

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
#define Prefix "TCS_"
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

void ProcTextCustom_SetCallBack(TCallBack *pCallBack)
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

