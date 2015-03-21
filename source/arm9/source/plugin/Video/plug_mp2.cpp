
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds.h>

#include "memtool.h"

#include "_console.h"
#include "_consoleWriteLog.h"

#include "../../../ipc6.h"

#include "plug_mp2.h"
#include "_dpgfs.h"
#include "libmpeg2/config.h"

#define ReadBufSize (8*1024)

DATA_IN_MTCM_VAR static bool Initialized=false;

void FreeMP2(void);

DATA_IN_MTCM_VAR static u64 MoveSample;

#include "plug_mp2_synth_d.h"

static inline void IPCSYNC_Enabled(void)
{
  REG_IE|=IRQ_IPC_SYNC;
}

static inline void IPCSYNC_Disabled(void)
{
  REG_IE&=~IRQ_IPC_SYNC;
}

bool StartMP2(void)
{
  if(Initialized==true) FreeMP2();
  Initialized=true;
  
  _consolePrint("Request ARM7 mp2 decode.\n");
  
//  IPC6->psynth_d=(void*)D;
  
  IPC6->IR=IR_NULL;
  IPC6->IR_filesize=DPGFS_Audio_GetSize();
  IPC6->IR_readsize=0;
  IPC6->IR_readbuf=(u8*)safemalloc_chkmem(&MM_DLLDPG,ReadBufSize);
  IPC6->IR_readbufsize=0;
  IPC6->IR_flash=false;
  IPC6->IR_EOF=false;
  
  IPC6->IR_SyncSamples_SendToARM9=0;
  
  MoveSample=0;
  
  return(true);
}

void FreeMP2(void)
{
  if(Initialized==false) return;
  Initialized=false;
  
  IPC6->IR_filesize=0;
  IPC6->IR_readsize=0;
  if(IPC6->IR_readbuf!=NULL){
    safefree(&MM_DLLDPG,IPC6->IR_readbuf); IPC6->IR_readbuf=NULL;
  }
}

void MP2_SetPosition(double per,u64 smp)
{
  if(Initialized==false) return;
  
  if(per<0) per=0;
  MoveSample=smp;
  
  REG_IE&=~IRQ_IPC_SYNC;
  
  u32 MovePosition=(u32)(DPGFS_Audio_GetSize()*per);
  MovePosition&=~3;
  
  u8 *buf=(u8*)IPC6->IR_readbuf;
  
  DPGFS_Audio_SetOffset(MovePosition);
  MovePosition=0;
  
  IPCSYNC_Disabled();
  u32 ReadSize=DPGFS_Audio_Read32bit(&buf[0],ReadBufSize);
  IPCSYNC_Enabled();
  
  if(DPGFS_Audio_GetSize()<=DPGFS_Audio_GetOffset()) IPC6->IR_EOF=true;
  
  IPC6->IR_readbufsize=ReadSize;
  
  IPC6->IR_flash=true;
  
  REG_IE|=IRQ_IPC_SYNC;
}

void MP2_LoadReadBuffer(void)
{
  if(Initialized==false) return;
  
  if(IPC6->IR_flash==true) return;

  IPCSYNC_Disabled();
  
//  _consolePrintf("fr%d,%d\n",IPC6->IR_readsize,IPC6->IR_readbufsize);
  
  u8 *buf=(u8*)IPC6->IR_readbuf;
  int ReadSize=ReadBufSize-IPC6->IR_readbufsize;
  
  ReadSize&=~1;
  if(ReadSize==0){
    IPCSYNC_Enabled();
    return;
  }
  
  if(DPGFS_Audio_GetSize()<=DPGFS_Audio_GetOffset()) IPC6->IR_EOF=true;
  
  int ReadedSize=DPGFS_Audio_Read32bit(&buf[IPC6->IR_readbufsize],ReadSize);
  DCache_CleanRangeOverrun(&buf[IPC6->IR_readbufsize],ReadSize);
  
  IPC6->IR_readbufsize+=ReadedSize;
  
  IPCSYNC_Enabled();
}

CODE_IN_ITCM_DPG void IRQSYNC_MP2_flash(void)
{
  if(Initialized==false) return;
  IPC6->IR_SyncSamples_SendToARM9=MoveSample;
  MoveSample=0;
}

CODE_IN_ITCM_DPG void IRQSYNC_MP2_fread(void)
{
  u8 *buf=(u8*)IPC6->IR_readbuf;
  int size=IPC6->IR_readsize;
  int remain=IPC6->IR_readbufsize-size;

//  _consolePrintf("MP2_fread: readsize=%d, readbufsize=%d remain=%d.\n",IPC6->IR_readsize,IPC6->IR_readbufsize,remain);

  if(remain<0){
    _consolePrintf("MP2_fread: Readed buffer is short! readsize=%d, readbufsize=%d\n",IPC6->IR_readsize,IPC6->IR_readbufsize);
    while(1);
  }
  
  if(remain!=0){
    //PrfStart();
    DCache_CleanRangeOverrun((void*)&buf[size],remain*2);
    DMA0_SRC = (uint32)&buf[size];
    DMA0_DEST = (uint32)&buf[0];
/*
    if((DMA0_SRC&3)!=0){
      _consolePrintf("align error. DMA0_SRC=0x%x\n",DMA0_SRC);
      while(1);
    }
    if((DMA0_DEST&3)!=0){
      _consolePrintf("align error. DMA0_DEST=0x%x\n",DMA0_DEST);
      while(1);
    }
*/
    DMA0_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_32_BIT | (remain/2);
    DCache_FlushRangeOverrun((void*)&buf[0],remain*2);
    //PrfEnd(remain);
//    while(DMA0_CR & DMA_BUSY);
  }

  IPC6->IR_readbufsize=remain;
}


