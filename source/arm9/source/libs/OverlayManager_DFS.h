
#ifndef _OverlayManager_dfs_h
#define _OverlayManager_dfs_h

static void OVR_DFS_Init(FAT_FILE *FileHandle);
static void OVR_DFS_Free(void);
static void OVR_DFS_SetAttribute(u32 _DataTopOffset,u32 _Size);
static u32 OVR_DFS_GetSize(void);
static void OVR_DFS_SetOffset(u32 ofs);
static u32 OVR_DFS_Read32bit(void *_pbuf,u32 size);

typedef struct {
  u32 Sector;
  u32 Count;
} TBurstList;

static u32 BurstListCount;
static TBurstList *pBurstList;

static void CreateBurstList(FAT_FILE *FileHandle)
{
  _consolePrint("CreateBurstList.\n");
  
  BurstListCount=0;
  
  {
    u32 CurClus=FileHandle->firstCluster;
    BurstListCount++;
    while(1){
      if(CurClus==CLUSTER_FREE) StopFatalError(10201,"OVR cluster link pre-search stopped.\n");
    
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
  
  pBurstList=(TBurstList*)safemalloc_chkmem(&MM_System,BurstListCount*sizeof(TBurstList));
  
  {
    u32 BurstListIndex=0;
    u32 CurClus=FileHandle->firstCluster;
    
    pBurstList[BurstListIndex].Sector=FAT2_ClustToSect(CurClus);
    pBurstList[BurstListIndex].Count=SecPerClus;
    BurstListIndex++;
    
    while(1){
      if(CurClus==CLUSTER_FREE) StopFatalError(10202,"OVR cluster link search stopped.\n");
    
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
  //    _consolePrintf("Index=%d Sector=0x%x Count=%d\n",idx,pBurstList[idx].Sector,pBurstList[idx].Count);
  //}
}

static void FreeBurstList(void)
{
  BurstListCount=0;
  if(pBurstList!=NULL){
      safefree(&MM_System,pBurstList); pBurstList=NULL;
  }
}

static void OVR_DFS_Init(FAT_FILE *FileHandle)
{
    u32 DataSize=FAT2_GetFileSize(FileHandle);
    CreateBurstList(FileHandle);
    OVR_DFS_SetAttribute(0,DataSize);
}

static void OVR_DFS_Free(void)
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

static TFile File;

static void SetAttribute(TFile *pf,u32 _DataTopOffset,u32 _Size)
{
  pf->DataTopOffset=_DataTopOffset;
  pf->Size=_Size;
}

static void OVR_DFS_SetAttribute(u32 _DataTopOffset,u32 _Size)
{
  SetAttribute(&File,_DataTopOffset,_Size);
  OVR_DFS_SetOffset(0);
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
    disc_ReadSectors(pf->BurstListCurSector,1,pf->SectorBuffer);
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
    disc_ReadSectors(pf->BurstListCurSector,reqsector,pbuf);
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
      disc_ReadSectors(pf->BurstListCurSector,1,pf->SectorBuffer);
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

static u32 OVR_DFS_GetSize(void)
{
  TFile *pf=&File;
  return(pf->Size);
}

static void OVR_DFS_SetOffset(u32 ofs)
{
//  _consolePrintf("MovieSetOffset=%d\n",ofs);
  TFile *pf=&File;
  MoveTop(pf);
  ReadSkip(pf,pf->DataTopOffset+ofs);
}

static u32 OVR_DFS_Read32bit(void *_pbuf,u32 size)
{
  TFile *pf=&File;
  size=Read32bit(pf,(u8*)_pbuf,size);
  return(size);
}

#endif
