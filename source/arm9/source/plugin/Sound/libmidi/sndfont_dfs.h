
#include "fat2.h"
#include "disc_io.h"

extern DISC_INTERFACE* active_interface;

#define CLUSTER_FREE	0x0000
#define	CLUSTER_EOF	0x0FFFFFFF

typedef struct {
  u32 Sector;
  u32 Count;
} TBurstList;

static u32 BurstListCount;
static TBurstList *pBurstList;

static void CreateBurstList(FAT_FILE *pFileHandle)
{
  _consolePrint("CreateBurstList.\n");
  
  BurstListCount=0;
  
  {
    u32 CurClus=pFileHandle->firstCluster;
    BurstListCount++;
    while(1){
      if(CurClus==CLUSTER_FREE) StopFatalError(15801,"cluster link pre-search stopped.\n");
      
      u32 NextClus=FAT2_NextCluster(CurClus);
      if(NextClus==CLUSTER_EOF) break;
      
      if((CurClus+1)!=NextClus){
        BurstListCount++;
        }else{
      }
      
      CurClus=NextClus;
    }
    
    BurstListCount++;
  }
  
  u32 SecPerClus=FAT2_GetSecPerClus();
  
  pBurstList=(TBurstList*)safemalloc_chkmem(&MM_DLLSound,BurstListCount*sizeof(TBurstList));
  
  {
    u32 BurstListIndex=0;
    u32 CurClus=pFileHandle->firstCluster;
    
    pBurstList[BurstListIndex].Sector=FAT2_ClustToSect(CurClus);
    pBurstList[BurstListIndex].Count=SecPerClus;
    BurstListIndex++;
    
    while(1){
      if(CurClus==CLUSTER_FREE) StopFatalError(15802,"cluster link search stopped.\n");
      
      u32 NextClus=FAT2_NextCluster(CurClus);
      if(NextClus==CLUSTER_EOF) break;
      
      if((CurClus+1)!=NextClus){
        pBurstList[BurstListIndex].Sector=FAT2_ClustToSect(NextClus);
        pBurstList[BurstListIndex].Count=SecPerClus;
        BurstListIndex++;
        }else{
        pBurstList[BurstListIndex-1].Count+=SecPerClus;
      }
      
      CurClus=NextClus;
    }
    
    pBurstList[BurstListIndex].Sector=CLUSTER_EOF;
    pBurstList[BurstListIndex].Count=0;
    BurstListIndex++;
  }
  
  return;
  for(u32 idx=0;idx<BurstListCount;idx++){
    _consolePrintf("Index=%d Sector=0x%x Count=%d\n",idx,pBurstList[idx].Sector,pBurstList[idx].Count);
  }
}

static void FreeBurstList(void)
{
  BurstListCount=0;
  if(pBurstList!=NULL){
    safefree(&MM_DLLSound,pBurstList); pBurstList=NULL;
  }
}

// ------------------------------------------------

#define SectorSize (512)

typedef struct {
  u32 Size;
  u32 Offset;
  u8 SectorBuffer[SectorSize];
  u32 SectorRemainByte;
  u32 BurstListIndex;
  u32 BurstListCurSector;
  u32 BurstListRemainSectorCount;
} TSndFontDFS_File;

static TSndFontDFS_File SndFontDFS_File;

static void SndFontDFS_Init(FAT_FILE *pFileHandle)
{
  CreateBurstList(pFileHandle);
}

static void SndFontDFS_Free(void)
{
  FreeBurstList();
}

static void MoveTop(TSndFontDFS_File *pf)
{
  pf->Offset=0;
//  MemSet32CPU(0,pf->SectorBuffer,SectorSize);
  pf->SectorRemainByte=0;
  pf->BurstListIndex=0;
  pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
  pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
}

static void ReadSkip(TSndFontDFS_File *pf,u32 size)
{
  if((size&1)!=0) StopFatalError(15803,"ReadSkip: size align error. (0x%x)\n",size);
  
  pf->Offset+=size;
  
  if(size!=0){
    u32 reqsize=size;
    if(pf->SectorRemainByte<reqsize) reqsize=pf->SectorRemainByte;
    size-=reqsize;
    pf->SectorRemainByte-=reqsize;
  }
  
  if(pf->BurstListRemainSectorCount==0) return;
  
  while(SectorSize<=size){
    pf->BurstListCurSector++;
    pf->BurstListRemainSectorCount--;
    size-=SectorSize;
    
    if(pf->BurstListRemainSectorCount==0){
      pf->BurstListIndex++;
      pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
      pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
    }
  }
  
  if(pf->BurstListRemainSectorCount==0) return;
  
  if(size!=0){
    active_interface->readSectors(pf->BurstListCurSector,1,pf->SectorBuffer);
    pf->SectorRemainByte=SectorSize-size;
    
    pf->BurstListCurSector++;
    pf->BurstListRemainSectorCount--;
    
    if(pf->BurstListRemainSectorCount==0){
      pf->BurstListIndex++;
      pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
      pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
    }
  }
}

static u32 Read16bit(TSndFontDFS_File *pf,u8 *pbuf,u32 size)
{
  if((((u32)pbuf)&1)!=0) StopFatalError(15804,"Read16bit: pbuf align error. (0x%x)\n",pbuf);
  if((size&1)!=0) StopFatalError(15805,"Read16bit: size align error. (0x%x)\n",size);
  
  u32 readedsize=0;
  
  if(size!=0){
    u32 reqsize=size;
    if(pf->SectorRemainByte<reqsize) reqsize=pf->SectorRemainByte;
    MemCopy32CPU(&pf->SectorBuffer[SectorSize-pf->SectorRemainByte],pbuf,reqsize);
    readedsize+=reqsize;
    pbuf+=reqsize;
    size-=reqsize;
    pf->SectorRemainByte-=reqsize;
  }
  
  if(pf->BurstListRemainSectorCount==0) return(readedsize);
  
  while(SectorSize<=size){
    u32 reqsector=size/SectorSize;
    if(pf->BurstListRemainSectorCount<reqsector) reqsector=pf->BurstListRemainSectorCount;
    if(255<reqsector) reqsector=255;
    if(pf->BurstListCurSector==CLUSTER_EOF) break;
//    _consolePrintf("[%d,%d,%d,%d]",pf->BurstListIndex,pf->BurstListCurSector,pf->BurstListRemainSectorCount,reqsector);
    active_interface->readSectors(pf->BurstListCurSector,reqsector,pbuf);
    u32 reqsize=reqsector*SectorSize;
    readedsize+=reqsize;
    pbuf+=reqsize;
    size-=reqsize;
    
    pf->BurstListCurSector+=reqsector;
    pf->BurstListRemainSectorCount-=reqsector;
    
    if(pf->BurstListRemainSectorCount==0){
      pf->BurstListIndex++;
      pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
      pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
    }
  }
  
  if(pf->BurstListRemainSectorCount==0) return(readedsize);
  
  if(size!=0){
    if(pf->BurstListCurSector==CLUSTER_EOF){
      }else{
      active_interface->readSectors(pf->BurstListCurSector,1,pf->SectorBuffer);
      MemCopy16CPU(pf->SectorBuffer,pbuf,size);
      readedsize+=size;
      pf->SectorRemainByte=SectorSize-size;
    
      pf->BurstListCurSector++;
      pf->BurstListRemainSectorCount--;
    
      if(pf->BurstListRemainSectorCount==0){
        pf->BurstListIndex++;
        pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
        pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
      }
    }
  }
  
  pf->Offset+=readedsize;
  return(readedsize);
}

static void SndFontDFS_SetOffset(u32 ofs)
{
  TSndFontDFS_File *pf=&SndFontDFS_File;
  MoveTop(pf);
  ReadSkip(pf,ofs);
}

static u32 SndFontDFS_Read16bit(void *_pbuf,u32 size)
{
  TSndFontDFS_File *pf=&SndFontDFS_File;
  size=Read16bit(pf,(u8*)_pbuf,size);
  return(size);
}
