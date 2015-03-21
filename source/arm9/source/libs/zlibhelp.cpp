
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "maindef.h"
#include "_console.h"
#include "memtool.h"

#include "zlibhelp.h"

#include "zlib.h"

extern int ZEXPORT deflateInit2_(
    z_streamp strm,
    int  level,
    int  method,
    int  windowBits,
    int  memLevel,
    int  strategy,
    const char *version,
    int stream_size);


static void* zalloc(void *opaque, u32 items, u32 size)
{
  void *p=safemalloc_chkmem(&MM_Temp,items*size);
//  _consolePrintf("zalloc: 0x%08x, %dx%d\n",p,items,size);
  return(p);
}

static void zfree(void *opaque, void *address)
{
//  _consolePrintf("zfree: 0x%08x\n",address);
  return(safefree(&MM_Temp,address));
}

uLong ZEXPORT compressBound (uLong sourceLen)
{
    return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) + 11;
}

bool zlibcompress(TZLIBData *pZLIBData,u32 LimitSize)
{
  z_stream z;
  
  z.zalloc = zalloc;
  z.zfree = zfree;
  z.opaque = Z_NULL;
  
  if(deflateInit2(&z,Z_BEST_COMPRESSION, Z_DEFLATED, 8, 1, Z_DEFAULT_STRATEGY)!=Z_OK){
//  if(deflateInit(&z,Z_BEST_COMPRESSION)!=Z_OK){
    StopFatalError(19101,"zliberror: deflateInit: %s\n", (z.msg) ? z.msg : "???");
  }
  
  if(LimitSize==0) LimitSize=compressBound(pZLIBData->SrcSize);
  
  pZLIBData->pDstBuf=(u8*)safemalloc_chkmem(&MM_Temp,LimitSize);
  pZLIBData->DstSize=LimitSize;
  
  z.avail_in=pZLIBData->SrcSize;
  z.next_in=pZLIBData->pSrcBuf;
  z.avail_out=pZLIBData->DstSize;
  z.next_out=pZLIBData->pDstBuf;
  
  bool result;
  
  switch(deflate(&z, Z_FINISH)){
    case Z_STREAM_END: {
      pZLIBData->DstSize=z.total_out;
      result=true;
    } break;
    case Z_OK: {
      if(pZLIBData->pDstBuf!=NULL){
        safefree(&MM_Temp,pZLIBData->pDstBuf); pZLIBData->pDstBuf=NULL;
      }
      pZLIBData->DstSize=0;
      result=false;
    } break;
    default: {
      StopFatalError(19102,"zliberror: deflate: %s\n", (z.msg) ? z.msg : "???");
    } break;
  }
  
/*
  _consolePrintf("ZLIBMEM:pZLIBData->pSrcBuf 0x%x %dbyte.\n",pZLIBData->pSrcBuf,pZLIBData->SrcSize);
  _consolePrintf("ZLIBMEM:pZLIBData->pDstBuf 0x%x %dbyte.\n",pZLIBData->pDstBuf,pZLIBData->DstSize);
  PrintFreeMem();
*/
  
  if(deflateEnd(&z)!=Z_OK) StopFatalError(19103,"zliberror: deflateEnd: %s\n", (z.msg) ? z.msg : "???");
  
  return(result);
}

bool zlibdecompress(TZLIBData *pZLIBData)
{
  z_stream z;
  z_streamp pz=(z_streamp)&z;
  
  z.zalloc = zalloc;
  z.zfree = zfree;
  z.opaque = Z_NULL;
  
  if(inflateInit(pz)!=Z_OK) StopFatalError(19104,"zliberror: inflateInit: %s\n", (z.msg) ? z.msg : "???");
  
  if((pZLIBData->pDstBuf==NULL)||(pZLIBData->DstSize==0)) StopFatalError(19105,"zliberror: dist buffer memory overflow.\n");
  
  z.avail_in=pZLIBData->SrcSize;
  z.next_in=pZLIBData->pSrcBuf;
  z.avail_out=pZLIBData->DstSize;
  z.next_out=pZLIBData->pDstBuf;
  
  bool result;
  
  switch(inflate(pz, Z_FINISH)){
    case Z_STREAM_END: {
      if(pZLIBData->DstSize!=z.total_out){
        _consolePrintf("zliberror: pZLIBData->DstSize(%d)!=z.total_out(%d)\n",pZLIBData->DstSize,z.total_out);
        result=false;
        }else{
        result=true;
      }
    } break;
    case Z_OK: {
      _consolePrint("zliberror: inflate result=Z_OK");
      result=false;
    } break;
    default: {
      _consolePrintf("zliberror: inflate: %s\n", (z.msg) ? z.msg : "???");
      result=false;
    } break;
  }
  
  if(inflateEnd(pz)!=Z_OK) StopFatalError(19106,"zliberror: inflateEnd: %s\n", (z.msg) ? z.msg : "???");
  
  return(result);
}
