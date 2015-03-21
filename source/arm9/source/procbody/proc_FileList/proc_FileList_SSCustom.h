
#pragma Ospace

#include "component.h"

DATA_IN_IWRAM_FileList static TProcState *pCurrentProcState=&ProcState;

#define ClientHeight (192)

// -----------------------------

DATA_IN_IWRAM_FileList static u32 SSC_Timeout;

// -----------------------------

enum ECompLabels {ECLS_TitleLbl,ECLS_Use24hFormatLbl,ECLS_ScreenSaverTitleLbl,ECLS_ScreenSaverBGTitleLbl,ECLS_TimeoutMsgLbl,ECLSCount};
#define CompLabelsCount (ECLSCount)
DATA_IN_IWRAM_FileList static TComponentLabel CompLabels[CompLabelsCount];

enum ECompChecks {ECCS_ScreenSaverNormalRadio,ECCS_Use24hFormatChk,ECCS_ScreenSaverDigitalRadio,ECCS_ScreenSaverExtendRadio,
                  ECCS_ScreenSaverBGSkinDefaultRadio,ECCS_ScreenSaverBGTopScreenRadio,ECCS_ScreenSaverBGBottomScreenRadio,
                  ECCS_ShowClockChk,
                  ECCS_ShowID3TagChk,
                  ECCS_ShowLyricChk,
                  ECCS_HideScreenSaverCustomChk,
                  ECCSCount};
#define CompChecksCount (ECCSCount)
DATA_IN_IWRAM_FileList static TComponentCheck CompChecks[CompChecksCount];

enum ECompButtons {ECBS_BackBtn,ECBSCount};
#define CompButtonsCount (ECBSCount)
DATA_IN_IWRAM_FileList static TComponentButton CompButtons[CompButtonsCount];

static void SSC_Setting_Redraw(void)
{
  pScreenMainOverlay->pCanvas->FillFull(0);
  
  CglCanvas *pcan=pScreenMain->pBackCanvas;
  
  pcan->FillFull(ColorTable.ScreenSaverCustom.BG);
  
  for(u32 idx=0;idx<CompLabelsCount;idx++){
    TComponentLabel *pcl=&CompLabels[idx];
    pcl->pCanvas=pcan;
  }
  for(u32 idx=0;idx<CompChecksCount;idx++){
    TComponentCheck *pcc=&CompChecks[idx];
    pcc->pCanvas=pcan;
  }
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    TComponentButton *pcb=&CompButtons[idx];
    pcb->pCanvas=pcan;
  }
  
  for(u32 idx=0;idx<CompChecksCount;idx++){
    TComponentCheck *pcc=&CompChecks[idx];
    bool *pchk=(bool*)pcc->UserData;
    if(pchk!=NULL) pcc->Checked=*pchk;
  }
  
  {
    CompChecks[ECCS_ScreenSaverNormalRadio].Checked=false;
    CompChecks[ECCS_ScreenSaverDigitalRadio].Checked=false;
    CompChecks[ECCS_ScreenSaverExtendRadio].Checked=false;
    
    switch(pCurrentProcState->ScreenSaver.ScreenSaver){
      case ETCTF_None: CompChecks[ECCS_ScreenSaverNormalRadio].Checked=true; break;
      case ETCTF_Lite: CompChecks[ECCS_ScreenSaverDigitalRadio].Checked=true; break;
      case ETCTF_Normal: CompChecks[ECCS_ScreenSaverExtendRadio].Checked=true; break;
      default: break;
    }
  }
  
  {
    CompChecks[ECCS_ScreenSaverBGSkinDefaultRadio].Checked=false;
    CompChecks[ECCS_ScreenSaverBGTopScreenRadio].Checked=false;
    CompChecks[ECCS_ScreenSaverBGBottomScreenRadio].Checked=false;
    
    switch(pCurrentProcState->ScreenSaver.ScreenSaverBG){
      case ETCTF_None: CompChecks[ECCS_ScreenSaverBGSkinDefaultRadio].Checked=true; break;
      case ETCTF_Lite: CompChecks[ECCS_ScreenSaverBGTopScreenRadio].Checked=true; break;
      case ETCTF_Normal: CompChecks[ECCS_ScreenSaverBGBottomScreenRadio].Checked=true; break;
      default: break;
    }
  }
  
  {
    TComponentLabel *pcl=&CompLabels[ECLS_TimeoutMsgLbl];
    if(SSC_Timeout==(u32)-1){
      pcl->pMsgUTF8="";
      }else{
    	  DATA_IN_IWRAM_FileList static char msg[64+1];
      snprintf(msg,64,Lang_GetUTF8("SSC_TimeoutMsg"),SSC_Timeout/60);
      pcl->pMsgUTF8=msg;
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
  
  ScreenMain_Flip_ProcFadeEffect();
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

static void CB_BackBtn_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  Backlight_ResetTimer();
}

static void CB_CheckBox_Click(void *pComponentButton)
{
  Sound_Start(WAVFN_Click);
  TComponentCheck *pcc=(TComponentCheck*)pComponentButton;
  bool *pchk=(bool*)pcc->UserData;
  if(pchk!=NULL) *pchk=!*pchk;
}

static void SSC_CompsInit(void)
{
  CglCanvas *pcan=pScreenMain->pBackCanvas;
  
  for(u32 idx=0;idx<CompLabelsCount;idx++){
    ComponentLabel_Init(&CompLabels[idx],pcan);
    TComponentLabel *pcl=&CompLabels[idx];
    pcl->TextColor=ColorTable.ScreenSaverCustom.Label_Text;
  }
  for(u32 idx=0;idx<CompChecksCount;idx++){
    ComponentCheck_Init(&CompChecks[idx],pcan);
    TComponentCheck *pcc=&CompChecks[idx];
    pcc->CallBack_Click=CB_CheckBox_Click;
    pcc->UserData=NULL;
    pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_CheckOn);
    pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_CheckOff);
    pcc->TextColor=ColorTable.ScreenSaverCustom.Check_Text;
  }
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    ComponentButton_Init(&CompButtons[idx],pcan);
    TComponentButton *pcb=&CompButtons[idx];
  }
  
  // -----------------
  
  u32 TextHeight=glCanvasTextHeight;
  
  u32 x=0,y=0,w=0,h=0;
  
  TProcState *pcps=pCurrentProcState;
  
  h=TextHeight;
  
  // -----------------
  
  x=8;
  y+=8;
  
  {
    TComponentLabel *pcl=&CompLabels[ECLS_TitleLbl];
    pcl->pMsgUTF8=Lang_GetUTF8("SSC_Title");
    pcl->Rect=CreateRect(x+24,y,ScreenWidth-x,h);
    pcl->isTitle=true;
    pcl->TextColor=ColorTable.ScreenSaverCustom.TitleLabel_Text;
  }
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_Use24hFormatChk];
    pcc->UserData=(u32)&pcps->ScreenSaver.Use24hFormat;
    pcc->pMsgUTF8=Lang_GetUTF8("SSC_Use24hFormat");
    pcc->Rect=CreateRect(ScreenWidth-ComponentCheck_GetWidth(pcc)-20,y,24,14);
  }
  
  y+=TextHeight;
  
  // -----------------
  
  u32 chkw=24,chkh=14;
  
  u32 PaddingY=6;
  
  y+=PaddingY;

  // -----------------
    
  {
    u32 rx=x;

    {
      TComponentLabel *pcl=&CompLabels[ECLS_ScreenSaverTitleLbl];
      pcl->pMsgUTF8=Lang_GetUTF8("SSC_ScreenSaverTitle");
      pcl->Rect=CreateRect(rx,y,ScreenWidth-x,h);
      rx+=ComponentLabel_GetWidth(pcl);
    }
    
  y+=TextHeight+4;
  rx=x;
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ScreenSaverNormalRadio];
      pcc->CallBack_Click=CB_ScreenSaverNormalRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("SSC_ScreenSaverNormal");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ScreenSaverDigitalRadio];
      pcc->CallBack_Click=CB_ScreenSaverDigitalRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("SSC_ScreenSaverDigital");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ScreenSaverExtendRadio];
      pcc->CallBack_Click=CB_ScreenSaverExtendRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("SSC_ScreenSaverExtend");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    if(rx>ScreenWidth) {
    	u32 tx=ComponentLabel_GetWidth(&CompLabels[ECLS_ScreenSaverTitleLbl])+10;
    	u32 ty=y-TextHeight-4;
    	
    	CompChecks[ECCS_ScreenSaverNormalRadio].Rect=CreateRect(tx,ty,chkw,chkh);
    	tx=x;
    	ty+=TextHeight+4;
    	CompChecks[ECCS_ScreenSaverDigitalRadio].Rect=CreateRect(tx,ty,chkw,chkh);
    	tx+=ComponentCheck_GetWidth(&CompChecks[ECCS_ScreenSaverDigitalRadio]);
    	CompChecks[ECCS_ScreenSaverExtendRadio].Rect=CreateRect(tx,ty,chkw,chkh);
    	tx+=ComponentCheck_GetWidth(&CompChecks[ECCS_ScreenSaverExtendRadio]);
    }
    
  }
  
  y+=TextHeight+PaddingY;
  
  // -----------------
  
  {
    u32 rx=x;
    
    {
      TComponentLabel *pcl=&CompLabels[ECLS_ScreenSaverBGTitleLbl];
      pcl->pMsgUTF8=Lang_GetUTF8("SSC_ScreenSaverBGTitle");
      pcl->Rect=CreateRect(rx,y,ScreenWidth-x,h);
      rx+=ComponentLabel_GetWidth(pcl);
    }
    
  y+=TextHeight+4;
  rx=x;
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ScreenSaverBGSkinDefaultRadio];
      pcc->CallBack_Click=CB_ScreenSaverBGSkinDefaultRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("SSC_ScreenSaverBGSkinDefault");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ScreenSaverBGTopScreenRadio];
      pcc->CallBack_Click=CB_ScreenSaverBGTopScreenRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("SSC_ScreenSaverBGTopScreen");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    
    {
      TComponentCheck *pcc=&CompChecks[ECCS_ScreenSaverBGBottomScreenRadio];
      pcc->CallBack_Click=CB_ScreenSaverBGBottomScreenRadio_Click;
      pcc->pOnIcon=ComponentAlpha_GetSkin(ECSA_RadioOn);
      pcc->pOffIcon=ComponentAlpha_GetSkin(ECSA_RadioOff);
      pcc->pMsgUTF8=Lang_GetUTF8("SSC_ScreenSaverBGBottomScreen");
      pcc->Rect=CreateRect(rx,y,chkw,chkh);
      rx+=ComponentCheck_GetWidth(pcc);
    }
    if(rx>ScreenWidth) {
    	u32 tx=ComponentLabel_GetWidth(&CompLabels[ECLS_ScreenSaverBGTitleLbl])+10;
    	u32 ty=y-TextHeight-4;
        	
    	CompChecks[ECCS_ScreenSaverBGSkinDefaultRadio].Rect=CreateRect(tx,ty,chkw,chkh);
    	tx=x;
    	ty+=TextHeight+4;
    	CompChecks[ECCS_ScreenSaverBGTopScreenRadio].Rect=CreateRect(tx,ty,chkw,chkh);
    	tx+=ComponentCheck_GetWidth(&CompChecks[ECCS_ScreenSaverBGTopScreenRadio]);
    	CompChecks[ECCS_ScreenSaverBGBottomScreenRadio].Rect=CreateRect(tx,ty,chkw,chkh);
    	tx+=ComponentCheck_GetWidth(&CompChecks[ECCS_ScreenSaverBGBottomScreenRadio]);
    }
  }
  
  y+=TextHeight+PaddingY+2;
  
  // -----------------
  
  {
	  TComponentCheck *pcc=&CompChecks[ECCS_ShowClockChk];
      pcc->UserData=(u32)&pcps->ScreenSaver.ShowClock;
      pcc->pMsgUTF8=Lang_GetUTF8("SSC_ShowClock");
      pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+4;
    
  {
    TComponentCheck *pcc=&CompChecks[ECCS_ShowID3TagChk];
    pcc->UserData=(u32)&pcps->ScreenSaver.ShowID3Tag;
    pcc->pMsgUTF8=Lang_GetUTF8("SSC_ShowID3Tag");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+4;
  
  // -----------------
    
    {
        TComponentCheck *pcc=&CompChecks[ECCS_ShowLyricChk];
        pcc->UserData=(u32)&pcps->ScreenSaver.ShowLyric;
        pcc->pMsgUTF8=Lang_GetUTF8("SSC_ShowLyric");
        pcc->Rect=CreateRect(x,y,chkw,chkh);
    }
    y+=TextHeight+4;
    
  // -----------------
  
  {
    TComponentCheck *pcc=&CompChecks[ECCS_HideScreenSaverCustomChk];
    pcc->UserData=(u32)&pcps->ScreenSaver.HideScreenSaverCustom;
    pcc->pMsgUTF8=Lang_GetUTF8("SSC_HideScreenSaverCustom");
    pcc->Rect=CreateRect(x,y,chkw,chkh);
  }
  y+=TextHeight+4;
  
  // -----------------
  
  {
    TComponentLabel *pcl=&CompLabels[ECLS_TimeoutMsgLbl];
    pcl->pMsgUTF8=Lang_GetUTF8("SSC_TimeoutMsg");
    pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
    pcl->isTitle=false;
  }
  y+=TextHeight*2;
  
  // -----------------
  
  u32 padx;
  
  padx=12;
  
  h=12;
  x=padx;
  y=ClientHeight-h-PaddingY;
  
  // -----------------
  
  {
    TComponentLabel *pcl=&CompLabels[ECLS_TimeoutMsgLbl];
    pcl->pMsgUTF8=Lang_GetUTF8("SSC_TimeoutMsg");
    pcl->Rect=CreateRect(x,y,ScreenWidth-x,h);
    pcl->isTitle=false;
  }
  
  // -----------------
  
  padx=15;
  w=72;
  h=24;
  x=ScreenWidth-padx-w;
  y=ClientHeight-h-PaddingY;
  
  {
    TComponentButton *pcb=&CompButtons[ECBS_BackBtn];
    pcb->CallBack_Click=CB_BackBtn_Click;
    pcb->pIcon=ComponentAlpha_GetSkin(ECSA_Ok);
    pcb->pMsgUTF8=Lang_GetUTF8("SSC_Back");
    pcb->Rect=CreateRect(x,y,w,h);
  }
}

// -----------------------------

DATA_IN_IWRAM_FileList static bool SSC_deskmf;
DATA_IN_IWRAM_FileList static TComponentButton *SSC_pPressingButton;
DATA_IN_IWRAM_FileList static bool SSC_First;

static void SSC_CB_MouseDown(s32 x,s32 y)
{
  if(SSC_Timeout==0) return;
  
  if(SSC_Timeout!=(u32)-1){
    SSC_Timeout=(u32)-1; // never
    SSC_Setting_Redraw();
  }
  
  SSC_deskmf=false;
  
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    TComponentButton *pcb=&CompButtons[idx];
    if(ComponentButton_GetIndexFromPos(pcb,x,y)!=-1){
      SSC_pPressingButton=pcb;
      pcb->Pressing=true;
      ComponentButton_Draw(pcb);
      SSC_Setting_Redraw();
      SSC_deskmf=true;
      return;
    }
  }
  
  SSC_deskmf=true;
}

static void SSC_CB_MouseMove(s32 x,s32 y)
{
  if(SSC_Timeout==0) return;
  
  if(SSC_deskmf==false) return;
  
  for(u32 idx=0;idx<CompButtonsCount;idx++){
    TComponentButton *pcb=&CompButtons[idx];
    if(pcb==SSC_pPressingButton){
      if(ComponentButton_GetIndexFromPos(pcb,x,y)==-1){
        if(pcb->Pressing==true){
          pcb->Pressing=false;
          ComponentButton_Draw(pcb);
          SSC_Setting_Redraw();
        }
        }else{
        if(pcb->Pressing==false){
          pcb->Pressing=true;
          ComponentButton_Draw(pcb);
          SSC_Setting_Redraw();
        }
      }
    }
  }
}

static void SSC_CB_MouseUp(s32 x,s32 y)
{
  if(SSC_Timeout==0){
	if(PlayList_isOpened()==false) Backlight_ResetTimer();
	if(!ProcState.ScreenSaver.EnabledTouchPanelCombination) Backlight_ResetTimer();
    return;
  }
  
  if(SSC_deskmf==false) return;
  
  SSC_deskmf=false;
  
  if(SSC_pPressingButton!=NULL){
    SSC_pPressingButton->Pressing=false;
    ComponentButton_Draw(SSC_pPressingButton);
    
    for(u32 idx=0;idx<CompButtonsCount;idx++){
      TComponentButton *pcb=&CompButtons[idx];
      if(pcb==SSC_pPressingButton){
        if(ComponentButton_GetIndexFromPos(pcb,x,y)!=-1){
          ComponentButton_MouseUp(&CompButtons[idx],x,y);
        }
      }
    }
    
    SSC_Setting_Redraw();
    SSC_pPressingButton=NULL;
    return;
  }
  
  for(u32 idx=0;idx<CompLabelsCount;idx++){
    if(ComponentLabel_MouseUp(&CompLabels[idx],x,y)==true) SSC_Setting_Redraw();
  }
  for(u32 idx=0;idx<CompChecksCount;idx++){
    if(ComponentCheck_MouseUp(&CompChecks[idx],x,y)==true){
      SSC_Timeout=(u32)-1; // never
      ProcState_RequestSave=true;
      ProcState_Save();
      Date_Set24hFormat(pCurrentProcState->ScreenSaver.Use24hFormat);
      Skin_Load_FileList_ScreenSaver();
      if(DLLSound_isComplexDecoder()==false) SetProcFadeEffect(EPFE_CrossFade);
      SSC_Setting_Redraw();
    }
  }
}

static void SSC_VsyncUpdate(u32 VsyncCount)
{
  if(SSC_Timeout==0) return;
  
  if(SSC_Timeout==(u32)-1) return; // never
  
  for(u32 idx=0;idx<VsyncCount;idx++){
    if(SSC_Timeout!=0){
      SSC_Timeout--;
      if(SSC_First==false){
        if((SSC_Timeout%60)==0) SSC_Setting_Redraw();
      }
    }
  }
  
  SSC_First=false;
  
  if(SSC_Timeout!=0) return;
  
  pScreenMain->pBackCanvas->FillFull(RGB15(0,0,0)|BIT15);
  if(DLLSound_isComplexDecoder()==false) SetProcFadeEffect(EPFE_CrossFade);
  ScreenMain_Flip_ProcFadeEffect();
  
  IPC6->LCDPowerControl=LCDPC_ON_TOP;
}

static void SSC_Open(void)
{
  if(ProcState.ScreenSaver.HideScreenSaverCustom || ProcState.ScreenSaver.BacklightOffTimeout){
    SSC_Timeout=0;
    pScreenMain->pBackCanvas->FillFull(RGB15(0,0,0)|BIT15);
    if(DLLSound_isComplexDecoder()==false) SetProcFadeEffect(EPFE_CrossFade);
    ScreenMain_Flip_ProcFadeEffect();
    if(ProcState.ScreenSaver.BacklightOffTimeout){
    	IPC6->LCDPowerControl=LCDPC_OFF_BOTH;
    }else{
    	IPC6->LCDPowerControl=LCDPC_ON_TOP;
    }
    return;
  }
  
  IPC6->LCDPowerControl=LCDPC_ON_BOTH;
  SSC_Timeout=60*(5+1);
  
  SSC_deskmf=false;
  SSC_pPressingButton=NULL;
  SSC_First=true;
  
  if(DLLSound_isComplexDecoder()==false) SetProcFadeEffect(EPFE_CrossFade);
  SSC_Setting_Redraw();
}

static void SSC_Close(void)
{
}

#pragma Otime
