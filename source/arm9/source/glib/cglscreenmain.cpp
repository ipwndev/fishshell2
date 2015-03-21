
#include <stdlib.h>
#include <nds.h>

#include "glib.h"
#include "glmemtool.h"
#include "cglscreen.h"

#define VRAMBuf ((u16*)(0x06000000))

// 15bit bitmap
#define BG2_CR_BASE_15bitBM (BG_BMP16_256x256 | BG_BMP_BASE(0))
#define BG3_CR_BASE_15bitBM (BG_BMP16_256x256 | BG_BMP_BASE(6))

CglScreenMain::CglScreenMain(void)
{
  REG_BG2CNT = 0;
  REG_BG3CNT = 0;
  
  {
	  REG_BG2PA = 1 << 8;
	  REG_BG2PB = 0 << 8;
	  REG_BG2PC = 0 << 8;
	  REG_BG2PD = 1 << 8;
	  REG_BG2X = 0;
	  REG_BG2Y = 0;
  }
  {
	  REG_BG3PA = 1 << 8;
	  REG_BG3PB = 0 << 8;
	  REG_BG3PC = 0 << 8;
	  REG_BG3PD = 1 << 8;
	  REG_BG3X = 0;
	  REG_BG3Y = 0;
  }
  
  VRAMBufArray[0]=&VRAMBuf[(ScreenHeight*ScreenWidth)*0];
  VRAMBufArray[1]=&VRAMBuf[(ScreenHeight*ScreenWidth)*1];
  
  pViewCanvas=new CglCanvas(&MM_System,VRAMBufArray[0],ScreenWidth,ScreenHeight,pf15bit);
  pBackCanvas=new CglCanvas(&MM_System,VRAMBufArray[1],ScreenWidth,ScreenHeight,pf15bit);
  
  BackVRAMPage=1;
  
  u32 color=RGB15(0,0,0)|BIT15;
  
  pViewCanvas->SetColor(color);
  MemSet32CPU(color,pViewCanvas->GetVRAMBuf(),ScreenWidth*ScreenHeight*2);
  
  pBackCanvas->SetColor(color);
  MemSet32CPU(color,pBackCanvas->GetVRAMBuf(),ScreenWidth*ScreenHeight*2);
  
  mode=ESMM_Normal;

  Flip(true);
}

CglScreenMain::~CglScreenMain(void)
{
  delete pViewCanvas; pViewCanvas=NULL;
  delete pBackCanvas; pBackCanvas=NULL;
}

CODE_IN_ITCM_GLIB void CglScreenMain::Flip(const bool ShowFlag)
{
  BackVRAMPage=1-BackVRAMPage;
  pBackCanvas->SetVRAMBuf(VRAMBufArray[BackVRAMPage],ScreenWidth,ScreenHeight,pf15bit);
  pViewCanvas->SetVRAMBuf(VRAMBufArray[1-BackVRAMPage],ScreenWidth,ScreenHeight,pf15bit);
  
  if(BackVRAMPage==0){
    REG_BG2CNT = BG2_CR_BASE_15bitBM | BG_PRIORITY_2;
    REG_BG3CNT = BG3_CR_BASE_15bitBM | BG_PRIORITY_1;
    }else{
    REG_BG2CNT = BG2_CR_BASE_15bitBM | BG_PRIORITY_1;
    REG_BG3CNT = BG3_CR_BASE_15bitBM | BG_PRIORITY_2;
  }
  
  if(ShowFlag==true){
    SetBlendLevel(16);
    }else{
    SetBlendLevel(0);
  }
}

volatile bool InterruptVsync_RequestFlickerFree=false;
static u16 InterruptVsync_RequestFlickerFree_BG2_CR;
static u32 InterruptVsync_RequestFlickerFree_BG3_CR;
CODE_IN_ITCM_GLIB void CglScreenMain::Flip_FlickerFree(void)
{
  BackVRAMPage=1-BackVRAMPage;
  pBackCanvas->SetVRAMBuf(VRAMBufArray[BackVRAMPage],ScreenWidth,ScreenHeight,pf15bit);
  pViewCanvas->SetVRAMBuf(VRAMBufArray[1-BackVRAMPage],ScreenWidth,ScreenHeight,pf15bit);
  
  while(InterruptVsync_RequestFlickerFree==true);
  
  REG_IME=0;
  
  InterruptVsync_RequestFlickerFree=true;
  
  if(BackVRAMPage==0){
    InterruptVsync_RequestFlickerFree_BG2_CR = BG2_CR_BASE_15bitBM | BG_PRIORITY_2;
    InterruptVsync_RequestFlickerFree_BG3_CR = BG3_CR_BASE_15bitBM | BG_PRIORITY_1;
    }else{
    InterruptVsync_RequestFlickerFree_BG2_CR = BG2_CR_BASE_15bitBM | BG_PRIORITY_1;
    InterruptVsync_RequestFlickerFree_BG3_CR = BG3_CR_BASE_15bitBM | BG_PRIORITY_2;
  }
  
  REG_IME=1;
  
//  SetBlendLevel(16); // カラーキーは使用不可
}
CODE_IN_ITCM_GLIB void CglScreenMain::Flip_FlickerFree_FromInterrupt(void)
{
  if(InterruptVsync_RequestFlickerFree==false) return;
  
  InterruptVsync_RequestFlickerFree=false;
  
  REG_BG2CNT=InterruptVsync_RequestFlickerFree_BG2_CR;
  REG_BG3CNT=InterruptVsync_RequestFlickerFree_BG3_CR;
  
  SetBlendLevel(16); // カラーキーは使用不可
}
CODE_IN_ITCM_GLIB void CglScreenMain::FlipForVSyncAuto(void)
{
  BackVRAMPage=1-BackVRAMPage;
  
  if(BackVRAMPage==0){
    REG_BG2CNT = BG2_CR_BASE_15bitBM | BG_PRIORITY_2;
    REG_BG3CNT = BG3_CR_BASE_15bitBM | BG_PRIORITY_1;
    }else{
    REG_BG2CNT = BG2_CR_BASE_15bitBM | BG_PRIORITY_1;
    REG_BG3CNT = BG3_CR_BASE_15bitBM | BG_PRIORITY_2;
  }
}

CODE_IN_ITCM_GLIB void CglScreenMain::SetBlendLevel(const int BlendLevel)
{
  SetBlendLevelManual(16-BlendLevel,BlendLevel);
}

CODE_IN_ITCM_GLIB void CglScreenMain::SetBlendLevelManual(const int BlendLevelBack,const int BlendLevelView)
{
  if((BlendLevelBack==0)&&(BlendLevelView==16)){
    if(BackVRAMPage==0){
      REG_BLDCNT=BLEND_ALPHA | BLEND_SRC_SPRITE | BLEND_DST_BG3;
      }else{
      REG_BLDCNT=BLEND_ALPHA | BLEND_SRC_SPRITE | BLEND_DST_BG2;
    }
    REG_BLDALPHA=(16 << 0) | (16 << 8);
    return;
  }
  
  if(BackVRAMPage==0){
    REG_BLDCNT=BLEND_ALPHA | BLEND_SRC_BG3 | BLEND_DST_BG2;
    }else{
    REG_BLDCNT=BLEND_ALPHA | BLEND_SRC_BG2 | BLEND_DST_BG3;
  }
  
  int blb=BlendLevelBack;
  int blv=BlendLevelView;
  
  if(blb<0) blb=0;
  if(16<blb) blb=16; // Max16
  if(blv<0) blv=0;
  if(16<blv) blv=16; // Max16
  
  REG_BLDALPHA=(blv << 0) | (blb << 8);
}

void CglScreenMain::SetMode(EScrMainMode _mode)
{return;
  mode=_mode;
  
  switch(mode){
    case ESMM_Normal: {
      vramSetMainBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_SPRITE_0x06400000, VRAM_C_MAIN_BG_0x06020000,VRAM_D_SUB_SPRITE);
      REG_BG2CNT = BG2_CR_BASE_15bitBM | BG_PRIORITY_2;
      REG_BG3CNT = BG3_CR_BASE_15bitBM | BG_PRIORITY_1;
      BackVRAMPage=1;
    } break;
    case ESMM_ForARM7: {
      vramSetBankD(VRAM_D_MAIN_BG_0x06060000);
      MemSet32CPU(0,(u32*)0x6060000,128*1024);
      vramSetMainBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000, VRAM_C_ARM7_0x06000000,VRAM_D_SUB_SPRITE);
      REG_BG2CNT = BG2_CR_BASE_15bitBM | BG_PRIORITY_2;
      REG_BG3CNT = BG3_CR_BASE_15bitBM | BG_PRIORITY_1;
      BackVRAMPage=1;
    } break;
  }
  
  Flip(true);
}

EScrMainMode CglScreenMain::GetMode(void)
{
  return(mode);
}

