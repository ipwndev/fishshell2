
#include <nds.h>
#include <stdio.h>

#include "_console.h"
#include "_const.h"

#include "memtool.h"

#include "_dpgfs.h"

#include "fat2.h"
#include "disc_io.h"

#include "libmpeg2/config.h"

extern DISC_INTERFACE* active_interface;

#define CLUSTER_FREE	0x0000
#define	CLUSTER_EOF	0x0FFFFFFF

typedef struct {
  u32 Sector;
  u32 Count;
} TBurstList;

DATA_IN_MTCM_VAR static u32 BurstListCount;
DATA_IN_MTCM_VAR static TBurstList *pBurstList;

static void CreateBurstList(FAT_FILE *pFileHandle)
{
  _consolePrint("CreateBurstList.\n");
  
  BurstListCount=0;
  
  {
    u32 CurClus=pFileHandle->firstCluster;
    BurstListCount++;
    while(1){
      if(CurClus==CLUSTER_FREE) StopFatalError(15101,"cluster link pre-search stopped.\n");
      
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
  
  pBurstList=(TBurstList*)safemalloc_chkmem(&MM_DLLDPG,BurstListCount*sizeof(TBurstList));
  
  {
    u32 BurstListIndex=0;
    u32 CurClus=pFileHandle->firstCluster;
    
    pBurstList[BurstListIndex].Sector=FAT2_ClustToSect(CurClus);
    pBurstList[BurstListIndex].Count=SecPerClus;
    BurstListIndex++;
    
    while(1){
      if(CurClus==CLUSTER_FREE) StopFatalError(15102,"cluster link search stopped.\n");
      
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
  
  //for(u32 idx=0;idx<BurstListCount;idx++){
  //  _consolePrintf("Index=%d Sector=0x%x Count=%d\n",idx,pBurstList[idx].Sector,pBurstList[idx].Count);
  //}
}

static void FreeBurstList(void)
{
  BurstListCount=0;
  if(pBurstList!=NULL){
    safefree(&MM_DLLDPG,pBurstList); pBurstList=NULL;
  }
}

// ------------------------------------------------


void DPGFS_Init(FAT_FILE *pFileHandle)
{
  CreateBurstList(pFileHandle);
}

void DPGFS_Free(void)
{
  FreeBurstList();
}

#define SectorSize (512)

typedef struct {
  u32 DataTopOffset;
  u32 Size;
  u32 Offset;
  u8 SectorBuffer[SectorSize];
  u32 SectorRemainByte;
  u32 BurstListIndex;
  u32 BurstListCurSector;
  u32 BurstListRemainSectorCount;
} TFile;

DATA_IN_MTCM_VAR static TFile FileMovie,FileAudio;

static void SetAttribute(TFile *pf,u32 _DataTopOffset,u32 _Size)
{
  pf->DataTopOffset=_DataTopOffset;
  pf->Size=_Size;
}

void DPGFS_Movie_SetAttribute(u32 _DataTopOffset,u32 _Size)
{
  SetAttribute(&FileMovie,_DataTopOffset,_Size);
  DPGFS_Movie_SetOffset(0);
}

void DPGFS_Audio_SetAttribute(u32 _DataTopOffset,u32 _Size)
{
  SetAttribute(&FileAudio,_DataTopOffset,_Size);
  DPGFS_Audio_SetOffset(0);
}

static void MoveTop(TFile *pf)
{
  pf->Offset=0;
  MemSet32CPU(0,pf->SectorBuffer,SectorSize);
  pf->SectorRemainByte=0;
  pf->BurstListIndex=0;
  pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
  pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
}

static void ReadSkip(TFile *pf,u32 size)
{
  pf->Offset=size;
  
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

static u32 Read32bit(TFile *pf,u8 *pbuf,u32 size)
{
  if((((u32)pbuf)&3)!=0){
    _consolePrintf("pbuf align error. (0x%x)\n",pbuf);
  }
  if((size&3)!=0){
    _consolePrintf("size align error. (0x%x)\n",pbuf);
  }
  
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
      MemCopy32CPU(pf->SectorBuffer,pbuf,size);
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

u32 DPGFS_Movie_GetSize(void)
{
  TFile *pf=&FileMovie;
  return(pf->Size);
}

u32 DPGFS_Audio_GetSize(void)
{
  TFile *pf=&FileAudio;
  return(pf->Size);
}

void DPGFS_Movie_SetOffset(u32 ofs)
{
//  _consolePrintf("MovieSetOffset=%d\n",ofs);
  TFile *pf=&FileMovie;
  MoveTop(pf);
  ReadSkip(pf,pf->DataTopOffset+ofs);
}

void DPGFS_Audio_SetOffset(u32 ofs)
{
//  _consolePrintf("AudioSetOffset=%d\n",ofs);
  TFile *pf=&FileAudio;
  MoveTop(pf);
  ReadSkip(pf,pf->DataTopOffset+ofs);
}

u32 DPGFS_Audio_GetOffset(void)
{
  TFile *pf=&FileAudio;
  return(pf->Offset-pf->DataTopOffset);
}

u32 DPGFS_Movie_Read32bit(void *_pbuf,u32 size)
{
  TFile *pf=&FileMovie;
  size=Read32bit(pf,(u8*)_pbuf,size);
  return(size);
}

u32 DPGFS_Audio_Read32bit(void *_pbuf,u32 size)
{
  TFile *pf=&FileAudio;
  size=Read32bit(pf,(u8*)_pbuf,size);
  return(size);
}
