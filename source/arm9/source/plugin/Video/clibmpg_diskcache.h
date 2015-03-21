
//#define DiskCacheDebugOut

#define DiskCacheBlockSize (ReadBufSize)
#define DiskCacheBlockCount (16)
#define DiskCacheBlockCountMask ((DiskCacheBlockCount)-1)

#define DiskCacheGuardBlockCount (4)

typedef struct {
  u32 ReadIndex,WriteIndex;
  u8 *pbuf; // ring buffer.
  int LastSize;
} TDiskCache;

DATA_IN_MTCM_VAR static TDiskCache DiskCache={0,0,NULL,0};

static void DiskCache_Init(void)
{
  TDiskCache *pdc=&DiskCache;
  
  if(pdc->pbuf!=NULL) StopFatalError(15201,"DiskCache_Init: Already exists disk cache memory.\n");
  
  pdc->ReadIndex=0;
  pdc->WriteIndex=0;
  pdc->pbuf=(u8*)safemalloc_chkmem(&MM_DLLDPG,DiskCacheBlockSize*DiskCacheBlockCount);
  pdc->LastSize=DiskCacheBlockSize;
}

static void DiskCache_Free(void)
{
  TDiskCache *pdc=&DiskCache;
  
  pdc->ReadIndex=0;
  pdc->WriteIndex=0;
  if(pdc->pbuf!=NULL){
    safefree(&MM_DLLDPG,pdc->pbuf); pdc->pbuf=NULL;
  }
  pdc->LastSize=0;
}

static void DiskCache_Clear(void)
{
  TDiskCache *pdc=&DiskCache;
  
  pdc->ReadIndex=0;
  pdc->WriteIndex=0;
  
  pdc->LastSize=DiskCacheBlockSize;
}

void DiskCache_LoadAllBuffer(void)
{
  TDiskCache *pdc=&DiskCache;
  
  if(pdc->ReadIndex!=pdc->WriteIndex) StopFatalError(15203,"DiskCache_LoadAllBuffer();\nAlready exists disk cache blocks. r=%d/w=%d\n",pdc->ReadIndex,pdc->WriteIndex);
  
//  pdc->ReadIndex=0;
  pdc->WriteIndex=pdc->ReadIndex;
  
  s32 readblkcnt=DiskCacheBlockCount-pdc->WriteIndex-DiskCacheGuardBlockCount;
  
  if(0<readblkcnt){
    int readsize=DPGFS_Movie_Read32bit(&pdc->pbuf[DiskCacheBlockSize*pdc->WriteIndex],DiskCacheBlockSize*readblkcnt);
    pdc->WriteIndex=(pdc->WriteIndex+(readsize+(DiskCacheBlockSize-1))/DiskCacheBlockSize)&DiskCacheBlockCountMask;
    
    u32 lastsize=readsize%DiskCacheBlockSize;
    if(lastsize!=0) pdc->LastSize=lastsize;
  
#ifdef DiskCacheDebugOut
    _consolePrintf("DiskCache_LoadAllBuffer readsize=%dbyte %dblocks lastsize=%d\n",readsize,pdc->WriteIndex,pdc->LastSize);
#endif
  }
}

static bool DiskCache_LoadOneBuffer(void)
{
  TDiskCache *pdc=&DiskCache;
  
  if(((pdc->WriteIndex+DiskCacheGuardBlockCount)&DiskCacheBlockCountMask)==pdc->ReadIndex) return(false);
  
#ifdef DiskCacheDebugOut
  _consolePrintf("DiskCache_LoadOneBuffer r=%d/w=%d\n",pdc->ReadIndex,pdc->WriteIndex);
#endif
  
  int readsize=DPGFS_Movie_Read32bit(&pdc->pbuf[DiskCacheBlockSize*pdc->WriteIndex],DiskCacheBlockSize);
  
#ifdef DiskCacheDebugOut
  _consolePrintf("DiskCache_LoadOneBuffer readsize=%dbyte\n",readsize);
#endif

  if(pdc->LastSize==DiskCacheBlockSize){
    if(readsize!=DiskCacheBlockSize) pdc->LastSize=readsize;
  }
  
  if(readsize==0) return(false);

  pdc->WriteIndex=(pdc->WriteIndex+1)&DiskCacheBlockCountMask;
  return(true);
}

static int DiskCache_ReadOneBlock(void **ppdstbuf)
{
  TDiskCache *pdc=&DiskCache;
  
#ifdef DiskCacheDebugOut
  _consolePrintf("DiskCache_ReadOneBlock Read from cache. r=%d/w=%d\n",pdc->ReadIndex,pdc->WriteIndex);
#endif
  
  if(pdc->ReadIndex==pdc->WriteIndex){
#ifdef DiskCacheDebugOut
    _consolePrint("DiskCache_ReadOneBlock Empty. direct read.\n");
#endif
    if(DiskCache_LoadOneBuffer()==false){
      *ppdstbuf=NULL;
      return(0);
    }
/*
    *ppdstbuf=pdc->pcurrentbuf;
    int readsize=DPGFS_Movie_Read32bit(*ppdstbuf,DiskCacheBlockSize);
    if(readsize!=DiskCacheBlockSize) pdc->LastSize=0;
    return(readsize);
*/
  }

  int readsize=DiskCacheBlockSize;
  
  if(((pdc->ReadIndex+1)&DiskCacheBlockCountMask)==pdc->WriteIndex){
    readsize=pdc->LastSize;
    if(readsize!=DiskCacheBlockSize) pdc->LastSize=0;
  }
  
  if(readsize!=0){
    *ppdstbuf=&pdc->pbuf[DiskCacheBlockSize*pdc->ReadIndex];
    pdc->ReadIndex=(pdc->ReadIndex+1)&DiskCacheBlockCountMask;
  }
  
#ifdef DiskCacheDebugOut
    _consolePrintf("DiskCache_ReadOneBlock %dbyte readed.\n",readsize);
#endif

  return(readsize);
}

u32 DiskCache_GetReadySize(void)
{
  TDiskCache *pdc=&DiskCache;
  
  u32 block=(DiskCacheBlockCount+pdc->WriteIndex-pdc->ReadIndex)&DiskCacheBlockCountMask;
  return(block*ReadBufSize);
}
