
#include <nds.h>
//#include <NDS/ARM9/CP15.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"

#include <stdio.h>
#include <stdlib.h>

#include "memtool.h"

#include "../../ipc6.h"

#include "arm9tcm.h"
#include "maindef.h"

#include "strpcm.h"
#include "playlist.h"
#include "splash.h"

#include "plug_mp2.h"

DATA_IN_DTCM vu64 DPGAudioStream_SyncSamples;
DATA_IN_DTCM u32 DPGAudioStream_PregapSamples;

DATA_IN_DTCM volatile bool VBlankPassedFlag;
DATA_IN_DTCM volatile u32 VBlankPassedCount;

DATA_IN_DTCM static int strpcmAudioVolume64;
DATA_IN_DTCM static int strpcmVideoVolume64;

DATA_IN_DTCM volatile bool strpcmRequestStop;

DATA_IN_DTCM volatile bool strpcmRingEmptyFlag;
DATA_IN_DTCM volatile u32 strpcmRingBufReadIndex;
DATA_IN_DTCM volatile u32 strpcmRingBufWriteIndex;

DATA_IN_DTCM u32 *strpcmRingLRBuf=NULL;

DATA_IN_DTCM bool strpcm_ExclusivePause=false;

DATA_IN_DTCM bool strpcm_UseLoopOnOverflow=false;

static __attribute__ ((noinline)) void strpcmUpdate(void);

#include "strpcm_ARM7_SelfCheck.h"

DATA_IN_DTCM static volatile bool VBlank_AutoFlip=false;

static CODE_IN_ITCM_Global void InterruptHandler_VBlank(void)
{
  ARM7_SelfCheck_Check();
  
  {
    u64 *p=(u64*)NULL;
    if(*p!=NULLSTR64bit) IlligalWritedNullPointer_Check((u32)-1);
  }
  
  VBlankPassedFlag=true;
  VBlankPassedCount++;
  
  Splash_IRQVSYNC();
  
  CallBack_ExecuteVBlankHandler();
  
  if(VBlank_AutoFlip==true) pScreenMain->FlipForVSyncAuto();
  
  _consolePrintServer();
}

void VBlank_AutoFlip_Enabled(void)
{
  VBlank_AutoFlip=true;
}

void VBlank_AutoFlip_Disabled(void)
{
  if(VBlank_AutoFlip==false) return;
  VBlank_AutoFlip=false;
  pScreenMain->Flip(true);
}

// ------------------------------------------

extern void IRQSYNC_MP2_flash(void);
extern void IRQSYNC_MP2_fread(void);

static CODE_IN_ITCM_Global void InterruptHandler_IPC_SYNC(void)
{
//  _consolePrintf("CallIPC(%d)\n",IPC6->IR);
  switch(IPC6->IR){
    case IR_NULL: {
    } break;
    case IR_NextSoundData: {
      strpcmUpdate();
      
      const u32 Samples=IPC6->strpcmSamples;
      const u32 Channels=IPC6->strpcmChannels;
      
      DCache_CleanRangeOverrun(IPC6->strpcmLRBuf,Samples*2*2);
      
      IPC6->strpcmWriteRequest=0;
    } break;
    case IR_Flash: {
      IRQSYNC_MP2_flash();
    } break;
    case IR_MP2_fread: {
      IRQSYNC_MP2_fread();
    } break;
    case IR_SyncSamples: {
      u64 curs=IPC6->IR_SyncSamples_SendToARM9;
      u64 bufs=IPC6->strpcmSamples;
      curs+=DPGAudioStream_PregapSamples*4; // gap 4 frames
      if(curs<bufs){
        curs=0;
        }else{
        curs-=bufs;
      }
      DPGAudioStream_SyncSamples=curs;
    } break;
  }
  IPC6->IR=IR_NULL;
}

extern "C" {
extern struct IntTable irqTable[16];
}

#include "snd_click_short_c_bin.h"
#include "snd_click_long_c_bin.h"

DATA_IN_IWRAM_MainPass void InitInterrupts(void)
{
  REG_IME = 0;
  irqInit();
  
  IPC6->StartFlagARM9 = 1;
  while(IPC6->StartFlagARM7==0);
  fifoInit();

  fifoSendAddress(FIFO_USER_01, (void *)snd_click_short_c_bin);
  fifoSendAddress(FIFO_USER_02, (void *)snd_click_long_c_bin);
    
  fifoSetValue32Handler(FIFO_PM, systemValueHandler, 0);
  fifoSetDatamsgHandler(FIFO_SYSTEM, systemMsgHandler, 0);

  if(REG_DSIMODE) {
	  fifoSendValue32(FIFO_PM,PM_DSI_HACK);
  }
  	
  irqSet(IRQ_VBLANK,InterruptHandler_VBlank);
  irqSet(IRQ_IPC_SYNC,InterruptHandler_IPC_SYNC);
  irqEnable(IRQ_VBLANK | IRQ_IPC_SYNC);
  
  REG_IPC_SYNC=IPC_SYNC_IRQ_ENABLE;
  
  VBlankPassedFlag=false;
  VBlankPassedCount=0;

  if(VerboseDebugLog==true){
    _consolePrint("IRQ jump table.\n");
    u32 *p=(u32*)irqTable;
    while(1){
      if(p[1]==0) break;
      _consolePrintf("adr=0x%x trig=%x\n",p[0],p[1]);
      p+=2;
    }
    _consolePrint("----------\n");
  }
  
  REG_IME = 1;
}

void strpcmStart(bool FastStart,u32 SampleRate,u32 SamplePerBuf,u32 ChannelCount,EstrpcmFormat strpcmFormat)
{
#ifdef notuseSound
  return;
#endif
  
  while(IPC6->strpcmControl!=ESC_NOP){
    ARM7_SelfCheck_Check();
  }
  
  switch(strpcmFormat){
    case SPF_PCMx1: _consolePrint("strpcm: set format SPF_PCMx1.\n"); break;
    case SPF_PCMx2: _consolePrint("strpcm: set format SPF_PCMx2.\n"); break;
    case SPF_PCMx4: _consolePrint("strpcm: set format SPF_PCMx4.\n"); break;
    case SPF_MP2: _consolePrint("strpcm: set format SPF_MP2.\n"); break;
    default: StopFatalError(14001,"strpcm unknown format. (%d)\n",strpcmFormat); break;
  }
  
  if((SampleRate==0)||(SamplePerBuf==0)||(ChannelCount==0)) StopFatalError(14002,"strpcmStart: Driver setting error.\n");
  
  switch(strpcmFormat){
    case SPF_PCMx1: case SPF_PCMx2: case SPF_PCMx4: case SPF_MP2: {
      strpcmRequestStop=false;
      
      u32 Samples=SamplePerBuf;
      u32 RingSamples=Samples*strpcmRingBufCount;
      
      strpcmRingEmptyFlag=false;
      strpcmRingBufReadIndex=0;
      if(FastStart==false){
        strpcmRingBufWriteIndex=strpcmRingBufCount-1;
        }else{
        strpcmRingBufWriteIndex=1;
      }
      
      strpcmRingLRBuf=(u32*)safemalloc_chkmem(&MM_SystemAfter,RingSamples*2*2);
      
      MemSet32CPU(0,strpcmRingLRBuf,RingSamples*2*2);
      
      IPC6->strpcmFreq=SampleRate;
      IPC6->strpcmSamples=Samples;
      IPC6->strpcmChannels=ChannelCount;
      IPC6->strpcmFormat=strpcmFormat;
      
      // ------
      
/*
      IPC6->strpcmLBuf=(s16*)safemalloc_chkmem(Samples*2);
      IPC6->strpcmRBuf=(s16*)safemalloc_chkmem(Samples*2);
      
      MemSet16CPU(0,IPC6->strpcmLBuf,Samples*2);
      MemSet16CPU(0,IPC6->strpcmRBuf,Samples*2);
*/

      IPC6->strpcmLRBuf=NULL;
    } break;
  }
  
  // ------
  
  IPC6->strpcmControl=ESC_Play;
  
  while(IPC6->strpcmControl!=ESC_NOP){
    ARM7_SelfCheck_Check();
  }
  
}

void strpcmStop(void)
{
#ifdef notuseSound
  return;
#endif
  
  strpcmRequestStop=false;
  
//  _consolePrint("Wait for terminate. (0)\n");
  while(IPC6->strpcmControl!=ESC_NOP){
    ARM7_SelfCheck_Check();
  }
  
  IPC6->strpcmControl=ESC_Stop;
  
//  _consolePrint("Wait for terminate. (1)\n");
  while(IPC6->strpcmControl!=ESC_NOP){
    ARM7_SelfCheck_Check();
  }
  
  if(VerboseDebugLog==true) _consolePrint("ARM7 strpcm terminated.\n");
  
  switch(IPC6->strpcmFormat){
    case SPF_PCMx1: case SPF_PCMx2: case SPF_PCMx4: case SPF_MP2: {
      strpcmRingEmptyFlag=false;
      strpcmRingBufReadIndex=0;
      strpcmRingBufWriteIndex=0;
      
      if(strpcmRingLRBuf!=NULL){
        safefree(&MM_SystemAfter,strpcmRingLRBuf); strpcmRingLRBuf=NULL;
      }
      
      IPC6->strpcmFreq=0;
      IPC6->strpcmSamples=0;
      IPC6->strpcmChannels=0;
      IPC6->strpcmFormat=SPF_DWORD;
      
/*
      if(IPC6->strpcmLBuf!=NULL){
        safefree(IPC6->strpcmLBuf); IPC6->strpcmLBuf=NULL;
      }
      if(IPC6->strpcmRBuf!=NULL){
        safefree(IPC6->strpcmRBuf); IPC6->strpcmRBuf=NULL;
      }
*/
      IPC6->strpcmLRBuf=NULL;
    } break;
  }
  
  _consolePrint("strpcm stopped.\n");
}

// ----------------------------------------------

static __attribute__ ((noinline)) void strpcmUpdate(void)
{
#ifdef notuseSound
  strpcmRingBufReadIndex=(strpcmRingBufReadIndex+1) & strpcmRingBufBitMask;
  return;
#endif
  
  if((strpcm_ExclusivePause==true)||(PlayList_GetPause()==true)){
    IPC6->strpcmLRBuf=NULL;
    return;
  }
  
  u32 Samples=IPC6->strpcmSamples;
  const u32 Channels=IPC6->strpcmChannels;
  
  if(strpcmRingLRBuf==NULL) StopFatalError(14003,"strpcmUpdate: strpcmRingLRBuf is NULL.\n");
  
  bool IgnoreFlag=false;
  
  u32 CurIndex=(strpcmRingBufReadIndex+1) & strpcmRingBufBitMask;
  
  if(CurIndex==strpcmRingBufWriteIndex){
    strpcmRingEmptyFlag=true;
    IgnoreFlag=true;
  }
  
  if(strpcm_UseLoopOnOverflow==true){
    if(IgnoreFlag==true) CurIndex=(strpcmRingBufWriteIndex+1) & strpcmRingBufBitMask;
    IPC6->strpcmLRBuf=&strpcmRingLRBuf[Samples*CurIndex];
    strpcmRingBufReadIndex=CurIndex;
    }else{
    if(IgnoreFlag==true){
      IPC6->strpcmLRBuf=NULL;
      }else{
      IPC6->strpcmLRBuf=&strpcmRingLRBuf[Samples*CurIndex];
      strpcmRingBufReadIndex=CurIndex;
    }
  }
}

void strpcmClearRingBuffer(void)
{
  u32 ime=REG_IME;
  REG_IME=0;
  u32 Samples=IPC6->strpcmSamples;
  u32 RingSamples=Samples*strpcmRingBufCount;
  MemSet32CPU(0,strpcmRingLRBuf,RingSamples*2*2);
  REG_IME=ime;
}


void strpcmSetAudioVolume64(int v)
{
  if(v<0) v=0;
  if(128<v) v=128;
  
  strpcmAudioVolume64=v;
  
  IPC6->strpcmAudioVolume64=strpcmAudioVolume64;
  //_consolePrintf("%s(%d);\n",__FUNCTION__,strpcmAudioVolume64);
}

int strpcmGetAudioVolume64(void)
{
  return(strpcmAudioVolume64);
}

void strpcmSetVideoVolume64(int v)
{
  if(v<0) v=0;
  if(128<v) v=128;
  
  strpcmVideoVolume64=v;
  
  IPC6->strpcmVideoVolume64=strpcmVideoVolume64;
  //_consolePrintf("%s(%d);\n",__FUNCTION__,strpcmVideoVolume64);
}

int strpcmGetVideoVolume64(void)
{
  return(strpcmVideoVolume64);
}


