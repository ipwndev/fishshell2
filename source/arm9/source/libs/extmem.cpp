
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"

#include "skin.h"

#include "maindef.h"
#include "memtool.h"
#include "shell.h"
#include "splash.h"
#include "strtool.h"
#include "procstate.h"
#include "arm9tcm.h"

#include "inifile.h"

#include "cstream_fs.h"

#include "zlibhelp.h"

#include "extmem.h"

DATA_IN_AfterSystem Textmem extmem;

// -------------------------

static u32 GetMaxSizeByte(u32 TopAddr)
{
  if(TopAddr==0) return(0);
  
  u32 maxsize=32*1024*1024;
  vu32 *ptmp=(vu32*)TopAddr;
  
  for(u32 idx=0;idx<maxsize/4;idx+=256){
    ptmp[idx]=(u32)&ptmp[idx];
    if(TopAddr!=ptmp[0]) break;
  }
  ptmp[0]=TopAddr;
  
  for(u32 idx=0;idx<maxsize/4;idx+=256){
    if(ptmp[idx]!=((u32)&ptmp[idx])){
      if(idx==0){
        return(0);
        }else{
        return(idx*4);
      }
    }
  }
  
  return(maxsize);
}

// -------------------------

extern u32 extmem_DSBM_Start(void);
extern u32 extmem_EZ3in1_Start(void);
extern u32 extmem_M3ExtPack_Start(void);
extern u32 extmem_RawMem_Start(void);
extern u32 extmem_SuperCard_Start(void);

#include "disc_io.h"

DATA_IN_IWRAM_MainPass void extmem_Init(void)
{
  Textmem *pem=&extmem;
  
  pem->pID=NULL;
  pem->MaxSizeByte=0;
  pem->TopAddr=0;
  pem->CurAddr=0;
  pem->TermAddr=0;
  pem->AllocateCount=0;
  
  if(disc_isGBACartridge){
	  _consolePrint("Boot from GBA card.\n");
  }else{
	  _consolePrint("Boot from NDS card.\n");
  }
  
  if(GlobalINI.System.UseGBACartForSwapMemory==false) return;
  
  u32 RawMem_MaxSizeByte=GetMaxSizeByte(extmem_RawMem_Start());
  _consolePrintf("RawMem:%d, ",RawMem_MaxSizeByte);
  u32 DSBM_MaxSizeByte=GetMaxSizeByte(extmem_DSBM_Start());
  _consolePrintf("DSBM:%d, ",DSBM_MaxSizeByte);
  u32 EZ3in1_MaxSizeByte=GetMaxSizeByte(extmem_EZ3in1_Start());
  _consolePrintf("EZ3in1:%d, ",EZ3in1_MaxSizeByte);
  u32 M3ExtPack_MaxSizeByte=GetMaxSizeByte(extmem_M3ExtPack_Start());
  _consolePrintf("M3ExtPack:%d, ",M3ExtPack_MaxSizeByte);
  u32 SuperCard_MaxSizeByte=GetMaxSizeByte(extmem_SuperCard_Start());
  _consolePrintf("SuperCard:%d, ",SuperCard_MaxSizeByte);
  
  u32 maxsize=0;
  
  if(maxsize<DSBM_MaxSizeByte) maxsize=DSBM_MaxSizeByte;
  if(maxsize<EZ3in1_MaxSizeByte) maxsize=EZ3in1_MaxSizeByte;
  if(maxsize<M3ExtPack_MaxSizeByte) maxsize=M3ExtPack_MaxSizeByte;
  if(maxsize<RawMem_MaxSizeByte) maxsize=RawMem_MaxSizeByte;
  if(maxsize<SuperCard_MaxSizeByte) maxsize=SuperCard_MaxSizeByte;
  
  _consolePrintf("maxsize=%d.\n",maxsize);
  
  if(maxsize==0){
    _consolePrint("extmem_Init: Not found extend memory cartridge.\n");
    return;
  }
  
  if(maxsize==DSBM_MaxSizeByte){
    pem->pID="DSBM";
    pem->MaxSizeByte=DSBM_MaxSizeByte;
    pem->TopAddr=extmem_DSBM_Start();
    return;
  }
  
  if(maxsize==EZ3in1_MaxSizeByte){
    pem->pID="EZ3in1";
    pem->MaxSizeByte=EZ3in1_MaxSizeByte;
    pem->TopAddr=extmem_EZ3in1_Start();
    return;
  }
  
  if(maxsize==M3ExtPack_MaxSizeByte){
    pem->pID="M3ExtPack";
    pem->MaxSizeByte=M3ExtPack_MaxSizeByte;
    pem->TopAddr=extmem_M3ExtPack_Start();
    return;
  }
  
  if(maxsize==RawMem_MaxSizeByte){
    pem->pID="RawMem";
    pem->MaxSizeByte=RawMem_MaxSizeByte;
    pem->TopAddr=extmem_RawMem_Start();
    return;
  }
  
  if(maxsize==SuperCard_MaxSizeByte){
    pem->pID="SuperCard";
    pem->MaxSizeByte=SuperCard_MaxSizeByte;
    pem->TopAddr=extmem_SuperCard_Start();
    return;
  }
  
  StopFatalError(18901,"extmem_Init: Internal error.\n");
}

DATA_IN_AfterSystem static const u32 SkinCacheSize=2*1024*1024;

void extmem_Start(void)
{
  Textmem *pem=&extmem;
  
  u32 size=pem->MaxSizeByte;
  if(SkinCacheSize<size) size=SkinCacheSize;
  
  pem->CurAddr=pem->TopAddr;
  pem->TermAddr=pem->TopAddr+size;
  pem->AllocateCount=0;
}

void* extmem_malloc(u32 size)
{
  Textmem *pem=&extmem;
  
  if(pem->MaxSizeByte==0) return(NULL);
  
  size=(size+3)&~3;
  
  u32 StartAddr=pem->CurAddr;
  u32 EndAddr=StartAddr+size;
  
  if(pem->TermAddr<=EndAddr) return(NULL);
  
  pem->CurAddr=EndAddr;
  pem->AllocateCount++;
  return((void*)StartAddr);
}

DATA_IN_IWRAM_MainPass void extmem_ShowMemoryInfo(void)
{
  Textmem *pem=&extmem;
  
  if(pem->MaxSizeByte==0){
    _consolePrint("extmem: Extention memory was not found.\n");
    return;
  }
  
  _consolePrintf("extmem: ID='%s' MaxSizeByte=0x%08x TopAddr=0x%08x.\n",pem->pID,pem->MaxSizeByte,pem->TopAddr);
}

void extmem_ShowMallocInfo(void)
{
  Textmem *pem=&extmem;
  
  if(pem->MaxSizeByte==0) return;
  
  _consolePrintf("extmem: Total=%dbyte, Used=%dbyte, Free=%dbyte, Allocated=%d.\n",pem->TermAddr-pem->TopAddr,pem->CurAddr-pem->TopAddr,pem->TermAddr-pem->CurAddr,pem->AllocateCount);
}

const char* extmem_GetID(void)
{
  return(extmem.pID);
}

u32 extmem_GetPureMaxSizeByte(void)
{
  return(extmem.MaxSizeByte);
}

u32 extmem_GetMaxSizeByte(void)
{
  if(extmem.MaxSizeByte<SkinCacheSize) return(0);
  return(extmem.MaxSizeByte-SkinCacheSize);
}

u32 extmem_GetTopAddr(void)
{
  if(extmem.MaxSizeByte<SkinCacheSize) return(0);
  return(extmem.TopAddr+SkinCacheSize);
}

