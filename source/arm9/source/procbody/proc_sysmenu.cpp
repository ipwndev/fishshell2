
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
#include "BootROM.h"

#include "procstate.h"
#include "datetime.h"

#include "glib.h"

#include "fat2.h"
#include "shell.h"
#include "ErrorDialog.h"

#include "skin.h"
#include "sndeff.h"
#include "lang.h"
#include "mediatype.h"

#include "inifile.h"

#define ItemLeft (48)
#define ItemTop (26+10)
#define ItemWidth (160)
#define ItemHeight (24)
DATA_IN_IWRAM_SysMenu static u32 ItemsCount;

DATA_IN_IWRAM_SysMenu static CglCanvas *pMasterBG;

DATA_IN_IWRAM_SysMenu static s32 ItemIndex;

DATA_IN_IWRAM_SysMenu static bool isExistsSoftResetToFirmware;

#include "particle.h"

// -----------------------------

static bool ExecItem(u32 ItemIndex)
{
  switch(ItemIndex){
    case 0: {
      SetNextProc(ENP_MemoEdit,EPFE_FastCrossFade);
      return(false);
    } 
    case 1: {
      TProcState_FileList *pfl=&ProcState.FileList;
      switch(pfl->Mode){
        case EPSFLM_Single: pfl->Mode=EPSFLM_Double; break;
        case EPSFLM_Double: pfl->Mode=EPSFLM_Single; break;
      }
    } break;
    case 2: {
    	if(GlobalINI.System.ChildrenMode)  {
    		Sound_Start(WAVFN_Notify);
    		return(true);
    	}
      SetNextProc(ENP_Custom,EPFE_RightToLeft);
      return(false);
    } 
    case 3: {
      Skin_ClearCustomBG_FileBody();
    } break;
    case 4: {
      if(isExistsSoftResetToFirmware==true){
        BootROM_SoftResetToFirmware();
        SetNextProc(ENP_BootROM,EPFE_None);
        return(false);
      }
      Sound_Start(WAVFN_Notify);
      return(false);
    } 
  }
  
  return(true);
}

// -----------------------------

static void Setting_Redraw(u32 VsyncCount)
{
  {
    CglCanvas *pcan=pScreenMain->pBackCanvas;
    pMasterBG->BitBltFullBeta(pcan);
    
    if(ItemIndex!=-1){
      CglTGF *pBar=SysMenuAlpha_GetSkin(ESMSA_SelectBar);
      
      u32 x=ItemLeft;
      u32 y=ItemTop+(ItemHeight*ItemIndex);
      
      pBar->BitBlt(pcan,x,y);
    }
    
    for(u32 idx=0;idx<ItemsCount;idx++){
      if(idx!=ItemIndex){
        pcan->SetFontTextColor(ColorTable.SystemMenu.ItemText);
        }else{
        pcan->SetFontTextColor(ColorTable.SystemMenu.ItemSelectText);
      } 
      
      const char *ptext=NULL;
      switch(idx){
        case 0: ptext=Lang_GetUTF8("SysMenu_MemoList"); break;
        case 1: ptext=Lang_GetUTF8("SysMenu_FileListMode"); break;
        case 2: ptext=Lang_GetUTF8("SysMenu_Customize"); break;
        case 3: ptext=Lang_GetUTF8("SysMenu_RestoreBG"); break;
        case 4: {
          ptext=Lang_GetUTF8("SysMenu_Firmware");
          if((idx==ItemIndex)&&(isExistsSoftResetToFirmware==false)){
            DATA_IN_IWRAM_SysMenu static char str[64];
            char fn[8+1];
            snprintf(fn,8+1,"%s.nds",DIMediaID);
            snprintf(str,64,Lang_GetUTF8("SysMenu_Firmware_NotFound"),fn);
            ptext=str;
          }
        } break;
      }
      if(ptext!=NULL){
        s32 x=ItemLeft;
        s32 y=ItemTop+(ItemHeight*idx);
        
        s32 tw=pcan->GetTextWidthUTF8(ptext);
        s32 th=glCanvasTextHeight;
        
        x+=(ItemWidth-tw)/2;
        y+=(ItemHeight-th)/2;
        
        if(x<0) x=0;
        
        pcan->TextOutUTF8(x,y,ptext);
      }
    }
  }
  
  CglCanvas *pcan=pScreenMain->pBackCanvas;
  Particle_Update(VsyncCount,pcan);
  
  ScreenMain_Flip_ProcFadeEffect();
}

// -----------------------------

static void CB_KeyPress(u32 VsyncCount,u32 Keys)
{
  if((Keys&KEY_A)!=0){
    if(ItemIndex!=-1){
      Sound_Start(WAVFN_Click);
      if(ExecItem(ItemIndex)==true){
        SetNextProc(ENP_FileList,EPFE_CrossFade);
        return;
      }
    }
  }
  
  if((Keys&KEY_B)!=0){
    Sound_Start(WAVFN_Click);
    SetNextProc(ENP_FileList,EPFE_CrossFade);
    return;
  }
  
  if((Keys&(KEY_LEFT|KEY_RIGHT|KEY_UP|KEY_DOWN))!=0){
    if((Keys&KEY_LEFT)!=0) ItemIndex=0;
    if((Keys&KEY_RIGHT)!=0) ItemIndex=ItemsCount-1;
    if((Keys&KEY_DOWN)!=0){
      if(ItemIndex==-1){
        ItemIndex=0;
        }else{
        if(ItemIndex<(ItemsCount-1)) ItemIndex++;
      }
    }
    if((Keys&KEY_UP)!=0){
      if(ItemIndex==-1){
        ItemIndex=ItemsCount-1;
        }else{
        if(0<ItemIndex) ItemIndex--;
      }
    }
    return;
  }
}

static s32 Items_GetIndexFromPos(s32 x,s32 y)
{
  for(s32 idx=0;idx<ItemsCount;idx++){
    u32 x1=ItemLeft;
    u32 y1=ItemTop+(ItemHeight*idx);
    u32 x2=x1+ItemWidth;
    u32 y2=y1+ItemHeight;
    
    if((x1<=x)&&(x<x2)&&(y1<=y)&&(y<y2)) return(idx);
  }
  
  return(-1);
}

static void CB_MouseDown(s32 x,s32 y)
{
  Particle_SetMouseDown(x,y);
  
  ItemIndex=Items_GetIndexFromPos(x,y);
}

static void CB_MouseMove(s32 x,s32 y)
{
  Particle_SetMouseMove(x,y);
  
  ItemIndex=Items_GetIndexFromPos(x,y);
}

static void CB_MouseUp(s32 x,s32 y)
{
  Particle_SetMouseUp();
  
  ItemIndex=Items_GetIndexFromPos(x,y);
  if(ItemIndex!=-1){
    Sound_Start(WAVFN_Click);
    if(ExecItem(ItemIndex)==true){
      SetNextProc(ENP_FileList,EPFE_CrossFade);
      return;
    }
  }
  
  if((173<=x)&&(163<=y)){
    Sound_Start(WAVFN_Click);
    SetNextProc(ENP_FileList,EPFE_CrossFade);
    return;
  }
}

static void gausscolor50bk(CglCanvas *pcan)
{
  CglCanvas *psrccan=new CglCanvas(&MM_Temp,NULL,ScreenWidth,ScreenHeight,pf15bit);
  pcan->BitBltFullBeta(psrccan);
  
  CglCanvas *pdstcan=new CglCanvas(&MM_Temp,NULL,ScreenWidth,ScreenHeight,pf15bit);
  pdstcan->FillFull(RGB15(0,0,0)|BIT15);
  
  for(u32 y=1;y<(ScreenHeight-1);y++){
    u16 *psrcbufm1=psrccan->GetScanLine(y-1);
    u16 *psrcbuf=psrccan->GetScanLine(y);
    u16 *psrcbufp1=psrccan->GetScanLine(y+1);
    u16 *ptagbuf=pdstcan->GetScanLine(y);
    for(u32 x=1;x<(ScreenWidth-1);x++){
      u32 c0=psrcbuf[x];
      u32 c1=psrcbufm1[x];
      u32 c2=psrcbuf[x-1];
      u32 c3=psrcbuf[x+1];
      u32 c4=psrcbufp1[x];
      u32 c5=psrcbufm1[x-1];
      u32 c6=psrcbufm1[x+1];
      u32 c7=psrcbufp1[x-1];
      u32 c8=psrcbufp1[x+1];
      u32 r0=(c0&(0x1f<<0))>>0;
      u32 r1=(c1&(0x1f<<0))>>0;
      u32 r2=(c2&(0x1f<<0))>>0;
      u32 r3=(c3&(0x1f<<0))>>0;
      u32 r4=(c4&(0x1f<<0))>>0;
      u32 r5=(c5&(0x1f<<0))>>0;
      u32 r6=(c6&(0x1f<<0))>>0;
      u32 r7=(c7&(0x1f<<0))>>0;
      u32 r8=(c8&(0x1f<<0))>>0;
      u32 g0=(c0&(0x1f<<5))>>5;
      u32 g1=(c1&(0x1f<<5))>>5;
      u32 g2=(c2&(0x1f<<5))>>5;
      u32 g3=(c3&(0x1f<<5))>>5;
      u32 g4=(c4&(0x1f<<5))>>5;
      u32 g5=(c5&(0x1f<<5))>>5;
      u32 g6=(c6&(0x1f<<5))>>5;
      u32 g7=(c7&(0x1f<<5))>>5;
      u32 g8=(c8&(0x1f<<5))>>5;
      u32 b0=(c0&(0x1f<<10))>>10;
      u32 b1=(c1&(0x1f<<10))>>10;
      u32 b2=(c2&(0x1f<<10))>>10;
      u32 b3=(c3&(0x1f<<10))>>10;
      u32 b4=(c4&(0x1f<<10))>>10;
      u32 b5=(c5&(0x1f<<10))>>10;
      u32 b6=(c6&(0x1f<<10))>>10;
      u32 b7=(c7&(0x1f<<10))>>10;
      u32 b8=(c8&(0x1f<<10))>>10;
      u32 r=((r0*4)+((r1+r2+r3+r4)*2)+((r5+r6+r7+r8)*1))/(4+8+4);
      u32 g=((g0*4)+((g1+g2+g3+g4)*2)+((g5+g6+g7+g8)*1))/(4+8+4);
      u32 b=((b0*4)+((b1+b2+b3+b4)*2)+((b5+b6+b7+b8)*1))/(4+8+4);
      r=r/2;
      g=g/2;
      b=b/2;
      ptagbuf[x]=RGB15(r,g,b)|BIT15;
    }
  }
  
  pdstcan->BitBltFullBeta(pcan);
  
  delete psrccan; psrccan=NULL;
  delete pdstcan; pdstcan=NULL;
}

static void DrawFishShell2Version(CglCanvas *pcan)
{
  pcan->SetCglFont(pCglFontDefault);
  
  u32 x=8,y=8;
  char str[64];
  snprintf(str,64,"%s %s",ROMTITLE,ROMVERSION);
  
  pcan->SetFontTextColor(RGB15(4,4,4)|BIT15);
  for(s32 px=-1;px<=1;px++){
    for(s32 py=-1;py<=1;py++){
      if((px!=0)||(py!=0)){
        pcan->TextOutA(x+px,y+(0*14)+py,str);
      }
    }
  }
  
  for(s32 px=-1;px<=1;px++){
      for(s32 py=-1;py<=1;py++){
          if((px!=0)||(py!=0)){
              pcan->TextOutA(x+px,192-4-(1*14)+py,ROMWEB);
          }
      }
  }
      
  pcan->SetFontTextColor(RGB15(16,16,16)|BIT15);
  pcan->TextOutA(x,y+(0*14),str);
  pcan->TextOutA(x,192-4-(1*14),ROMWEB);
}

static void CB_Start(void)
{
  Sound_Start(WAVFN_Open);
  
  isExistsSoftResetToFirmware=BootROM_isExistsSoftResetToFirmware();
  
  ItemsCount=4;
  ItemsCount=5; // for Error dialog test.
  
  pMasterBG=new CglCanvas(&MM_Process,NULL,ScreenWidth,ScreenHeight,pf15bit);
  CglCanvas *psrccan=pScreenMain->pViewCanvas;
  psrccan->BitBltFullBeta(pMasterBG);
  
  gausscolor50bk(pMasterBG);
  gausscolor50bk(pScreenSub->pCanvas);
  
  DrawFishShell2Version(pMasterBG);
  
  CglTGF *pBG=SysMenuAlpha_GetSkin(ESMSA_BG);
  pBG->BitBlt(pMasterBG,0,0);
  
  ItemIndex=-1;
  
  Particle_Init();
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
  
  if(pMasterBG!=NULL){
    delete pMasterBG; pMasterBG=NULL;
  }
}

void ProcSysMenu_SetCallBack(TCallBack *pCallBack)
{
  pCallBack->Start=CB_Start;
  pCallBack->VsyncUpdate=CB_VsyncUpdate;
  pCallBack->End=CB_End;
  pCallBack->KeyPress=CB_KeyPress;
  pCallBack->MouseDown=CB_MouseDown;
  pCallBack->MouseMove=CB_MouseMove;
  pCallBack->MouseUp=CB_MouseUp;
}

