
#include <stdio.h>
#include <stdlib.h>

#include <nds.h>

#include "memtoolARM7.h"

#include "a7sleep.h"

#include "_console.h"

bool MappedVRAM;

#define VRAMTopAddress_Start (0x06000000)
#define VRAMTopAddress_End (VRAMTopAddress_Start+(128*1024))
static u32 VRAMTopAddress;

void SetMemoryMode(bool _MappedVRAM)
{
if(_MappedVRAM==true){
  _consolePrintf("Disabled _MappedVRAM==true.\n");
  while(1);
}
return;
/*
//  _consolePrintf("SetMemoryMode(%d);\n",_MappedVRAM);
  MappedVRAM=_MappedVRAM;
  VRAMTopAddress=VRAMTopAddress_Start;
  
  if(MappedVRAM==true){
    u32 *pbuf,*pterm;
    pbuf=(u32*)VRAMTopAddress_Start;
    pterm=(u32*)VRAMTopAddress_End;
    while(pbuf!=pterm){
      *pbuf++=(u32)pbuf;
    }
    pbuf=(u32*)VRAMTopAddress_Start;
    pterm=(u32*)VRAMTopAddress_End;
    bool err=false;
    while(pbuf!=pterm){
      if(*pbuf!=(u32)pbuf){
        if(err==false){
          err=true;
          _consolePrintf("ARM7 VRAM memory check error.\n");
        }
        _consolePrintf("(%x,%x), ",*pbuf,(u32)pbuf);
      }
      pbuf++;
    }
    if(err==true) while(1);
  }*/
}

void SetMemoryMode_End(void)
{
//  _consolePrintf("SetMemoryMode_End();\n");
  MappedVRAM=false;
}

static void* safemalloc_VRAM(int size)
{
//  _consolePrintf("malloc_VRAM(%d);\n",size);
  
  void *ptr=(void*)VRAMTopAddress;
  VRAMTopAddress+=size;
  if(VRAMTopAddress_End<=VRAMTopAddress){
    _consolePrintf("Memory overflow.\n");
    while(1);
  }
  
  static vu32 v=0;
  
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)&v;
  DMA3_DEST = (u32)ptr;
  DMA3_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_FIX | DMA_DST_INC)+(size>>2);
  while((DMA3_CR&DMA_BUSY)!=0);
  
  return(ptr);
}

void* safemalloc(int size)
{
  size=(size+3)&~3;
//  _consolePrintf("safemalloc(%d/0x%x);\n",size,size);
  
  if(MappedVRAM==true) return(safemalloc_VRAM(size));
  
  _consolePrintf("malloc(%d);\n",size);
  
  u32 *res=(u32*)malloc(size);
  if(res==NULL){
    _consolePrintf("Memory overflow.\n");
    while(1);
  }
  
  static vu32 v=0;
  
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)&v;
  DMA3_DEST = (u32)res;
  DMA3_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_FIX | DMA_DST_INC)+(size>>2);
  while((DMA3_CR&DMA_BUSY)!=0);
  
  return(res);
}

static void safefree_VRAM(void *ptr)
{
  return;
}

void safefree(void *ptr)
{
//  _consolePrintf("safefree(0x%x);\n",(u32)ptr);
  
  if(MappedVRAM==true) return(safefree_VRAM(ptr));
  if(ptr!=NULL) free(ptr);
}

/*

void MemSet16CPU(vu16 v,void *dst,u32 len)
{
  if(len<2) return;
  
  u16 *_dst=(u16*)dst;
  
  for(u32 cnt=0;cnt<(len/2);cnt++){
    _dst[cnt]=v;
  }
}

*/

/*

void MemSet32CPU(u32 v,void *dst,u32 len)
{
  if(len<4) return;
  
  u32 *_dst=(u32*)dst;
  
  for(u32 cnt=0;cnt<(len/4);cnt++){
    _dst[cnt]=v;
  }
}

*/

void MemCopy16DMA3(void *src,void *dst,u32 len)
{
//  MemCopy16CPU(src,dst,len);return;
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)src;
  DMA3_DEST = (u32)dst;
  DMA3_CR=(DMA_16_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC)+(len>>1);
  while((DMA3_CR&DMA_BUSY)!=0);
}

/*

void MemCopy32DMA3(void *src,void *dst,u32 len)
{
//  MemCopy32CPU(src,dst,len);return;
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)src;
  DMA3_DEST = (u32)dst;
  DMA3_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW |  DMA_SRC_INC | DMA_DST_INC)+(len/4);
  while((DMA3_CR&DMA_BUSY)!=0);
}

*/

void MemSet16DMA3(u16 v,void *dst,u32 len)
{
//  MemSet16CPU(v,dst,len);return;

  static vu32 dmatmp;
  dmatmp=v | (((u32)v) << 16);
  
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)&dmatmp;
  DMA3_DEST = (u32)dst;
  DMA3_CR=(DMA_16_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_FIX | DMA_DST_INC)+(len>>1);
  while((DMA3_CR&DMA_BUSY)!=0);
}

/*

void MemSet32DMA3(u32 v,void *dst,u32 len)
{
//  MemSet32CPU(v,dst,len);return;
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)&v;
  DMA3_DEST = (u32)dst;
  DMA3_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_FIX | DMA_DST_INC)+(len/4);
  while((DMA3_CR&DMA_BUSY)!=0);
}

*/

static bool testmalloc(int size)
{
  if(size<=0) return(false);
  
  void *ptr;
  
  ptr=malloc(size);
  
  if(ptr==NULL) return(false);
  
  free(ptr);
  
  return(true);
}

#define PrintFreeMem_Seg (1*1024)

u32 PrintFreeMem(void)
{
  u32 FreeMemSize=0;
  s32 i=0;
  
  for(i=96*1024;i!=0;i-=PrintFreeMem_Seg){
    if(testmalloc(i)==true){
      FreeMemSize=i;
      break;
    }
  }
  
  _consolePrintf("FreeMem=%dbyte    \n",FreeMemSize);
  return(FreeMemSize);
}

