
#include "fat2.h"
#include "disc_io.h"
#include "extmem.h"

extern DISC_INTERFACE* active_interface;

#define CLUSTER_FREE	0x0000
#define	CLUSTER_EOF	0x0FFFFFFF

typedef struct {
  u32 Sector;
  u32 Count;
} TBurstList;

DATA_IN_IWRAM_ImageView static u32 BurstListCount;
DATA_IN_IWRAM_ImageView static TBurstList *pBurstList;

static void CreateBurstList(FAT_FILE *pFileHandle)
{
  _consolePrint("CreateBurstList.\n");
  
  BurstListCount=0;
  
  {
    u32 CurClus=pFileHandle->firstCluster;
    BurstListCount++;
    while(1){
      if(CurClus==CLUSTER_FREE) StopFatalError(17601,"cluster link pre-search stopped.\n");
      
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
  
  pBurstList=(TBurstList*)safemalloc_chkmem(&MM_Process,BurstListCount*sizeof(TBurstList));
  
  {
    u32 BurstListIndex=0;
    u32 CurClus=pFileHandle->firstCluster;
    
    pBurstList[BurstListIndex].Sector=FAT2_ClustToSect(CurClus);
    pBurstList[BurstListIndex].Count=SecPerClus;
    BurstListIndex++;
    
    while(1){
      if(CurClus==CLUSTER_FREE) StopFatalError(17602,"cluster link search stopped.\n");
      
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
  
#if 0
  for(u32 idx=0;idx<BurstListCount;idx++){
      _consolePrintf("Index=%d Sector=0x%x Count=%d\n",idx,pBurstList[idx].Sector,pBurstList[idx].Count);
  }
#endif
}

static void FreeBurstList(void)
{
  BurstListCount=0;
  if(pBurstList!=NULL){
    safefree(&MM_Process,pBurstList); pBurstList=NULL;
  }
}

// ------------------------------------

DATA_IN_IWRAM_ImageView static bool DFS_UseExtMem;
DATA_IN_IWRAM_ImageView static u32 DFS_ExtMem_TopAddr;
DATA_IN_IWRAM_ImageView static u32 DFS_ExtMem_Position;

#define SectorSize (512)

typedef struct {
  u32 BurstListIndex;
  u32 BurstListCurSector;
  u32 BurstListRemainSectorCount;
} TDFS_File;

DATA_IN_IWRAM_ImageView static TDFS_File DFS_File;

// ------------------------------------

static void DFS_Init(u32 MaxSizeByte)
{
  if(MaxSizeByte<=extmem_GetMaxSizeByte()){
      if(VerboseDebugLog==true) _consolePrint("Start DFS extmem mode.\n");
    DFS_UseExtMem=true;
    DFS_ExtMem_TopAddr=extmem_GetTopAddr();
    DFS_ExtMem_Position=0;
    return;
  }
  
  DFS_UseExtMem=false;
  
  if(VerboseDebugLog==true) _consolePrint("Start DFS swap file mode.\n");
  
  {
    FAT_FILE *pFileHandle=Shell_FAT_fopen_SwapFile(MaxSizeByte);
    CreateBurstList(pFileHandle);
    FAT2_fclose(pFileHandle);
  }
  
  TDFS_File *pf=&DFS_File;
  
  pf->BurstListIndex=0;
  pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
  pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
}

static void DFS_Free(void)
{
  if(DFS_UseExtMem==true){
    DFS_UseExtMem=false;
    return;
  }
  
  FreeBurstList();
}

static void DFS_SeekSectorCount(u32 SectorsCount)
{
  if(DFS_UseExtMem==true){
    DFS_ExtMem_Position=SectorsCount*SectorSize;
    return;
  }
  
  TDFS_File *pf=&DFS_File;
  
  pf->BurstListIndex=0;
  pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
  pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
  
  while(SectorsCount!=0){
    u32 Remain;
    if(SectorsCount<=pf->BurstListRemainSectorCount){
      Remain=SectorsCount;
      }else{
      Remain=pf->BurstListRemainSectorCount;
    }
      
    pf->BurstListCurSector+=Remain;
    pf->BurstListRemainSectorCount-=Remain;
    SectorsCount-=Remain;
    
    if(pf->BurstListRemainSectorCount==0){
      pf->BurstListIndex++;
      pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
      pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
      if(pf->BurstListRemainSectorCount==0) StopFatalError(17603,"Insufficient swap memory for seek.\n");
    }
  }
}

static void DFS_WriteSectors(u8 *pbuf,u32 SectorsCount)
{
  if(DFS_UseExtMem==true){
    u8 *pextbuf=(u8*)(DFS_ExtMem_TopAddr+DFS_ExtMem_Position);
    u32 size=SectorsCount*SectorSize;
    MemCopy32CPU(pbuf,pextbuf,size);
    DFS_ExtMem_Position+=size;
    return;
  }
  
  TDFS_File *pf=&DFS_File;
  
  if((((u32)pbuf)&3)!=0){
    _consolePrintf("pbuf align error. (0x%x)\n",pbuf);
  }
  
  while(SectorsCount!=0){
    u32 reqsector=SectorsCount;
    if(pf->BurstListRemainSectorCount<reqsector) reqsector=pf->BurstListRemainSectorCount;
    if(255<reqsector) reqsector=255;
    if(pf->BurstListCurSector==CLUSTER_EOF) break;
//    _consolePrintf("[%d,%d,%d,%d]",pf->BurstListIndex,pf->BurstListCurSector,pf->BurstListRemainSectorCount,reqsector);
    active_interface->writeSectors(pf->BurstListCurSector,reqsector,pbuf);
    pbuf+=reqsector*SectorSize;
    SectorsCount-=reqsector;
    
    pf->BurstListCurSector+=reqsector;
    pf->BurstListRemainSectorCount-=reqsector;
    
    if(pf->BurstListRemainSectorCount==0){
      pf->BurstListIndex++;
      pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
      pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
      if(pf->BurstListRemainSectorCount==0) StopFatalError(17604,"Insufficient swap memory for write.\n");
    }
  }
}

static void DFS_ReadSectors(u8 *pbuf,u32 SectorsCount)
{
  if(DFS_UseExtMem==true){
    u8 *pextbuf=(u8*)(DFS_ExtMem_TopAddr+DFS_ExtMem_Position);
    u32 size=SectorsCount*SectorSize;
    MemCopy32CPU(pextbuf,pbuf,size);
    DFS_ExtMem_Position+=size;
    return;
  }
  
  TDFS_File *pf=&DFS_File;
  
  if((((u32)pbuf)&3)!=0){
    _consolePrintf("pbuf align error. (0x%x)\n",pbuf);
  }
  
  while(SectorsCount!=0){
    u32 reqsector=SectorsCount;
    if(pf->BurstListRemainSectorCount<reqsector) reqsector=pf->BurstListRemainSectorCount;
    if(255<reqsector) reqsector=255;
    if(pf->BurstListCurSector==CLUSTER_EOF) break;
//    _consolePrintf("[%d,%d,%d,%d]",pf->BurstListIndex,pf->BurstListCurSector,pf->BurstListRemainSectorCount,reqsector);
    active_interface->readSectors(pf->BurstListCurSector,reqsector,pbuf);
    pbuf+=reqsector*SectorSize;
    SectorsCount-=reqsector;
    
    pf->BurstListCurSector+=reqsector;
    pf->BurstListRemainSectorCount-=reqsector;
    
    if(pf->BurstListRemainSectorCount==0){
      pf->BurstListIndex++;
      pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
      pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
      if(pf->BurstListRemainSectorCount==0) StopFatalError(17605,"Insufficient swap memory for read.\n");
    }
  }
}

