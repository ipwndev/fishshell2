
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"
#include "maindef.h"

#include "fat2.h"

#include "strtool.h"
#include "memtool.h"
#include "shell.h"
#include "glib.h"
#include "zlibhelp.h"
#include "fastlzss16decpalasm.h"

#include "inifile.h"

#include "splash.h"
#include "extmem.h"

#include "sndeff.h"

#include "procstate.h"

DATA_IN_AfterSystem static FAT_FILE *pfh=NULL;

typedef struct {
  u32 VSyncCount;
  u32 FileOffset;
  u32 ImageSize;
  u32 b15BufCount;
} TScreen;

extern "C" {
void VRAMWriteCache_Enable(void);
void VRAMWriteCache_Disable(void);
}

#define Flags_WaitForTerminate (1 << 0)
#define Flags_AlreadyAllDraw (1 << 1)

DATA_IN_AfterSystem static u32 Flags;
DATA_IN_AfterSystem static u32 ScreenCount;
DATA_IN_AfterSystem static TScreen *pScreen;

DATA_IN_AfterSystem static bool Flags_Betalzss16;

DATA_IN_AfterSystem static bool EndFlag;
DATA_IN_AfterSystem static s32 LastFrameIndex;
DATA_IN_AfterSystem static vu32 VSyncCount;

DATA_IN_AfterSystem static u16 *pBetaBM;

void Splash_IRQVSYNC(void)
{
  VSyncCount++;
}

DATA_IN_IWRAM_MainPass void Splash_Init(void)
{
  //  fastlzss16decpalasm_init();
  pfh=Shell_FAT_fopen_Internal("SPLASH.ANI");
  if(pfh==NULL) return;
  
  pBetaBM=NULL;
  
  EndFlag=false;
  LastFrameIndex=-1;
  VSyncCount=0;
  
  Flags_Betalzss16=false;
  
  FAT2_fread(&Flags,1,4,pfh);
  Flags&=~Flags_AlreadyAllDraw;
  FAT2_fread(&ScreenCount,1,4,pfh);
  
  pScreen=(TScreen*)malloc(ScreenCount*sizeof(TScreen)); // システムロックをまたぐので明示的にsafemallocを使わない
  
  for(u32 idx=0;idx<ScreenCount;idx++){
    u32 buf[4];
    FAT2_fread(buf,4,4,pfh);
    TScreen *ps=&pScreen[idx];
    ps->VSyncCount=buf[0];
    ps->FileOffset=buf[1];
    ps->ImageSize=buf[2];
    ps->b15BufCount=buf[3];
//    _consolePrintf("%d: vs=%d, fo=%d, is=%d, b15=%d.\n",idx, ps->VSyncCount, ps->FileOffset, ps->ImageSize, ps->b15BufCount);
  }
  
  FAT2_fseek(pfh,0,SEEK_SET);

  REG_IME=0;  
  VSyncCount=0;
  Splash_Update();
  REG_IME=1;

  bool laststate=ProcState.System.ClickSound;
  ProcState.System.ClickSound=true;
  Sound_Start(WAVFN_Splash);
  ProcState.System.ClickSound=laststate;
  
  if((Flags&Flags_WaitForTerminate)!=0){
    while(Splash_Update()==true){
    }
  }
}

void Splash_Free(void)
{
  if(pfh==NULL) return;
  
  _consolePrint("Splash_Free: Free.\n");
  
  if((Flags&Flags_AlreadyAllDraw)!=0){
    while(Splash_Update()==true){
    }
  }
  
  FAT2_fclose(pfh); pfh=NULL;

  if(pBetaBM!=NULL){
    free(pBetaBM); pBetaBM=NULL;
  }
  
  EndFlag=false;
  LastFrameIndex=0;
  
  Flags=0;
  ScreenCount=0;
  
  if(pScreen!=NULL){
    free(pScreen); pScreen=NULL;
  }
}

static void Splash_DrawInfo_ins_DrawText(u32 x,u32 y,const char *pstr)
{
  u16 col;
  
  col=RGB15(16,16,16)|BIT15;
  pScreenSub->pCanvas->SetFontTextColor(col);
  pScreenSub->pCanvas->TextOutA(x-1,y-1,pstr);
  
  col=RGB15(31,31,31)|BIT15;
  pScreenSub->pCanvas->SetFontTextColor(col);
  pScreenSub->pCanvas->TextOutA(x+0,y+0,pstr);
}

void Splash_DrawInfo(void)
{
  u32 x=8,y=ScreenHeight-64;
  
  char str[64];
  snprintf(str,64,"%s %s booting...",ROMTITLE,ROMVERSION);
  Splash_DrawInfo_ins_DrawText(x,y,str);
  y+=32;
  
  u32 maxmem=extmem_GetPureMaxSizeByte();
  if(maxmem!=0){
    char str[128];
    snprintf(str,64,"%d KByte %s detected.",maxmem/1024,extmem_GetID());
    Splash_DrawInfo_ins_DrawText(x,y,str);
    }else{
    Splash_DrawInfo_ins_DrawText(x,y,"Extention memory was not found.");
  }
}

static inline void ins_convpal16to32(u16 *ppal16,vu32 *ppal32,u32 palcnt)
{
  for(u32 idx=0;idx<palcnt;idx++){
    ppal32[idx]=ppal16[idx];
  }
}

static void Splash_Update_Draw_Betalzss16(u32 FrameIndex)
{
  TScreen *ps=&pScreen[FrameIndex];
  
  TZLIBData zd;
  zd.SrcSize=ps->ImageSize;
  zd.pSrcBuf=(u8*)safemalloc_chkmem(&MM_Temp,zd.SrcSize);
  zd.DstSize=0;
  zd.pDstBuf=(u8*)pScreenSub->GetVRAMBuf();
  
  FAT2_fseek(pfh,ps->FileOffset,SEEK_SET);
  FAT2_fread(zd.pSrcBuf,1,zd.SrcSize,pfh);
  
  u32 palcnt=*(u32*)zd.pSrcBuf;
  u16 *ppal16=(u16*)&zd.pSrcBuf[4];
  u32 pal32[256];
  
  ins_convpal16to32(ppal16,pal32,palcnt);
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
  
  fastlzss16decpalasm_decode(&zd.pSrcBuf[4+(palcnt*2)],zd.pDstBuf,pal32);
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
  
  safefree(&MM_Temp,zd.pSrcBuf); zd.pSrcBuf=NULL;
}

static void Splash_Update_Draw_BetaZLIB(u32 FrameIndex)
{
  TScreen *ps=&pScreen[FrameIndex];
  
  u32 ImageSize=ps->ImageSize;
  
//  _consolePrintf("Splash(%d): fofs=%d, b15BufCount=%d, ImageSize=%d.\n",FrameIndex,ps->FileOffset,b15BufCount,ImageSize);
  
  if(pBetaBM!=NULL) StopFatalError(13903,"Splash: Exists multiple key frame?\n");
  
  pBetaBM=(u16*)malloc(ImageSize*2);
  
  TZLIBData z;
  
  z.SrcSize=ps->b15BufCount*2;
  z.pSrcBuf=(u8*)safemalloc_chkmem(&MM_Temp,z.SrcSize);
  z.DstSize=ImageSize*2;
  z.pDstBuf=(u8*)pBetaBM;

  FAT2_fseek(pfh,ps->FileOffset,SEEK_SET);
  FAT2_fread(z.pSrcBuf,1,z.SrcSize,pfh);
  
  if(zlibdecompress(&z)==false) {
	  if(z.pSrcBuf!=NULL){
		  safefree(&MM_Temp,z.pSrcBuf); z.pSrcBuf=NULL;
	  }
	  if(pBetaBM!=NULL){
		  free(pBetaBM); pBetaBM=NULL;
	  }
	  Splash_Update_Draw_Betalzss16(FrameIndex);
	  Flags_Betalzss16=true;
	  return;
	  //StopFatalError(13904,"Splash: Key frame decompress error.\n");
  }
  
  if(z.pSrcBuf!=NULL){
    safefree(&MM_Temp,z.pSrcBuf); z.pSrcBuf=NULL;
  }

  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
  
  MemCopy32CPU(pBetaBM,pScreenSub->GetVRAMBuf(),ImageSize*2);
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
}

static void Splash_Update_Draw_Diff(u32 FrameIndex)
{
  TScreen *ps=&pScreen[FrameIndex];
  
  u32 ImageSize=ps->ImageSize;
  u32 b15BufCount=ps->b15BufCount;
  u16 *pb15Buf=(u16*)safemalloc_chkmem(&MM_Temp,b15BufCount*2);
  u16 *pbm=(u16*)safemalloc_chkmem(&MM_Temp,ImageSize*2);
  
//  _consolePrintf("Splash(%d): fofs=%d, b15BufCount=%d, ImageSize=%d.\n",FrameIndex,ps->FileOffset,b15BufCount,ImageSize);
  
  FAT2_fseek(pfh,ps->FileOffset,SEEK_SET);
  FAT2_fread(pb15Buf,1,b15BufCount*2,pfh);
  
  if(pBetaBM==NULL) StopFatalError(13905,"Splash: Not found key frame.\n");
  
  MemCopy32CPU(pBetaBM,pbm,ImageSize*2);
  
  {
    u16 *psrc=pb15Buf;
    u16 *pdst=pbm;
    for(u32 idx=0;idx<b15BufCount;idx++){
      u16 data=*psrc++;
      if((data&BIT15)==0){
        pdst+=data;
        }else{
        *pdst++=data;
      }
    }
  }
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
  
  MemCopy32CPU(pbm,pScreenSub->GetVRAMBuf(),ImageSize*2);
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
  
  if(pb15Buf!=NULL){
    safefree(&MM_Temp,pb15Buf); pb15Buf=NULL;
  }
  if(pbm!=NULL){
    safefree(&MM_Temp,pbm); pbm=NULL;
  }
}

bool Splash_Update(void)
{
  if(pfh==NULL) return(false);
  
  if(EndFlag==true) return(false);
  
  s32 curidx=0;
  
  for(u32 idx=0;idx<ScreenCount;idx++){
    if(pScreen[idx].VSyncCount<=VSyncCount) curidx=idx;
  }
  
  //_consolePrintf("Splash_Update[%d:%d]\n",curidx,VSyncCount);
  
  if(curidx==LastFrameIndex) return(true);
  
  if(Flags_Betalzss16){
	  Splash_Update_Draw_Betalzss16(curidx);
  }else{
	  if(curidx==0){
		  Splash_Update_Draw_BetaZLIB(curidx);
	  }else{
		  Splash_Update_Draw_Diff(curidx);
	  }
  }
  LastFrameIndex=curidx;
  
  if((curidx+1)==ScreenCount){
    EndFlag=true;
    return(false);
  }
 
  return(true);
}

