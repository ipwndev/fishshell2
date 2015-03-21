

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
      if(CurClus==CLUSTER_FREE) StopFatalError(15401,"cluster link pre-search stopped.\n");
      
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
  
  pBurstList=(TBurstList*)safemalloc_chkmem(&MM_DLLImage,BurstListCount*sizeof(TBurstList));
  
  {
    u32 BurstListIndex=0;
    u32 CurClus=pFileHandle->firstCluster;
    
    pBurstList[BurstListIndex].Sector=FAT2_ClustToSect(CurClus);
    pBurstList[BurstListIndex].Count=SecPerClus;
    BurstListIndex++;
    
    while(1){
      if(CurClus==CLUSTER_FREE) StopFatalError(15402,"cluster link search stopped.\n");
      
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
    safefree(&MM_DLLImage,pBurstList); pBurstList=NULL;
  }
}

// ------------------------------------

#define SectorSize (512)

typedef struct {
  u32 BurstListIndex;
  u32 BurstListCurSector;
  u32 BurstListRemainSectorCount;
} TDFS_File;

static TDFS_File DFS_File;

// ------------------------------------

static void DFS_Init(FAT_FILE *pFileHandle)
{
  _consolePrint("Start PrgJpeg DFS swap file mode.\n");
  
  CreateBurstList(pFileHandle);
  
  TDFS_File *pf=&DFS_File;
  
  pf->BurstListIndex=0;
  pf->BurstListCurSector=pBurstList[pf->BurstListIndex].Sector;
  pf->BurstListRemainSectorCount=pBurstList[pf->BurstListIndex].Count;
}

static void DFS_Free(void)
{
  FreeBurstList();
}

static void DFS_SeekSectorCount(u32 SectorsCount)
{
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
      if(pf->BurstListRemainSectorCount==0) StopFatalError(15403,"Insufficient swap memory for seek.\n");
    }
  }
}

static void DFS_WriteSectors(u8 *pbuf,u32 SectorsCount)
{
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
      if(pf->BurstListRemainSectorCount==0) StopFatalError(15404,"Insufficient swap memory for write.\n");
    }
  }
}

static void DFS_ReadSectors(u8 *pbuf,u32 SectorsCount)
{
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
      if(pf->BurstListRemainSectorCount==0) StopFatalError(15405,"Insufficient swap memory for read.\n");
    }
  }
}

// ---------------------------------------------------------------------------------

static u32 DFS_PrgJpeg_SizeByte;
static u32 DFS_PrgJpeg_UsedSectorCount;

typedef struct {
  bool Opened;
  u32 TopSectorIndex;
  u32 AllocatedSectorsCount;
} TDFS_PrgJpeg_File;

#define DFS_PrgJpeg_File_MaxCount (4)
TDFS_PrgJpeg_File DFS_PrgJpeg_File[DFS_PrgJpeg_File_MaxCount];

static void DFS_PrgJpeg_Init(FAT_FILE *pFileHandle)
{
  DFS_Init(pFileHandle);
  
  DFS_PrgJpeg_SizeByte=FAT2_GetFileSize(pFileHandle);
  DFS_PrgJpeg_UsedSectorCount=0;
  
  for(u32 idx=0;idx<DFS_PrgJpeg_File_MaxCount;idx++){
    TDFS_PrgJpeg_File *pf=&DFS_PrgJpeg_File[idx];
    pf->Opened=false;
  }
}

static void DFS_PrgJpeg_Free(void)
{
  for(u32 idx=0;idx<DFS_PrgJpeg_File_MaxCount;idx++){
    TDFS_PrgJpeg_File *pf=&DFS_PrgJpeg_File[idx];
    if(pf->Opened==true) StopFatalError(15406,"DFS_PrgJpeg_Free: internal opened error.\n");
  }
  
  DFS_Free();
}

int DFS_PrgJpeg_fopen(u32 RequestSize)
{
  u32 RemainSectors=(DFS_PrgJpeg_SizeByte/SectorSize)-DFS_PrgJpeg_UsedSectorCount;
  
  u32 RequestSectors=(RequestSize+(SectorSize-1))/SectorSize;
  
  if(RemainSectors<RequestSectors) StopFatalError(15407,"DFS_PrgJpeg_fopen: Allocate area overflow. %dbyte, %d/%d\n",RequestSize,RequestSectors,RemainSectors);
  
  TDFS_PrgJpeg_File *pf=NULL;
  u32 FileHandle=0;
  
  for(u32 idx=0;idx<DFS_PrgJpeg_File_MaxCount;idx++){
    if(DFS_PrgJpeg_File[idx].Opened==false){
      pf=&DFS_PrgJpeg_File[idx];
      FileHandle=idx;
      break;
    }
  }
  
  if(pf==NULL) StopFatalError(15408,"DFS_PrgJpeg_fopen: Not found closed file handle.\n");
  
  pf->Opened=true;
  pf->TopSectorIndex=DFS_PrgJpeg_UsedSectorCount;
  pf->AllocatedSectorsCount=RequestSectors;
  
  DFS_PrgJpeg_UsedSectorCount+=RequestSectors;
  
  return(FileHandle);
}

void DFS_PrgJpeg_fclose(u32 FileHandle)
{
  TDFS_PrgJpeg_File *pf=&DFS_PrgJpeg_File[FileHandle];
  if(pf->Opened==false) StopFatalError(15409,"DFS_PrgJpeg_fclose: is closed file. (%d)\n",FileHandle);
  
  pf->Opened=false;
}

void DFS_PrgJpeg_fread(u32 FileHandle,u32 pos,u8 *pbuf,u32 size)
{
  TDFS_PrgJpeg_File *pf=&DFS_PrgJpeg_File[FileHandle];
  if(pf->Opened==false) StopFatalError(15410,"DFS_PrgJpeg_fread: is closed file. (%d)\n",FileHandle);
  
  {
    u32 sec=pos/SectorSize;
    DFS_SeekSectorCount(pf->TopSectorIndex+sec);
    pos-=sec*SectorSize;
  }
  
  if(pos!=0){
    u8 tmpbuf[SectorSize];
    DFS_ReadSectors(tmpbuf,1);
    u32 tmpsize=size;
    if((SectorSize-pos)<tmpsize) tmpsize=SectorSize-pos;
    for(u32 idx=0;idx<tmpsize;idx++){
      *pbuf++=tmpbuf[pos+idx];
    }
    size-=tmpsize;
  }
  
  if(((u32)pbuf&1)!=0){
    _consolePrintf("DFS_PrgJpeg_fread: Memory allign error. (%d, 0x%08x)\n",FileHandle,pbuf);
    while(SectorSize<=size){
      u8 tmpbuf[SectorSize];
      DFS_ReadSectors(tmpbuf,1);
      for(u32 idx=0;idx<SectorSize;idx++){
        *pbuf++=tmpbuf[idx];
      }
      size-=SectorSize;
    }
    }else{
    u32 reqsec=size/SectorSize;
    DFS_ReadSectors(pbuf,reqsec);
    pbuf+=reqsec*SectorSize;
    size-=reqsec*SectorSize;
  }
  
  if(size!=0){
    u8 tmpbuf[SectorSize];
    DFS_ReadSectors(tmpbuf,1);
    for(u32 idx=0;idx<size;idx++){
      *pbuf++=tmpbuf[idx];
    }
  }
}

void DFS_PrgJpeg_fwrite(u32 FileHandle,u32 pos,u8 *pbuf,u32 size)
{
  TDFS_PrgJpeg_File *pf=&DFS_PrgJpeg_File[FileHandle];
  if(pf->Opened==false) StopFatalError(15411,"DFS_PrgJpeg_fwrite: is closed file. (%d)\n",FileHandle);
  
  u32 cursec;
  
  {
    u32 sec=pos/SectorSize;
    cursec=pf->TopSectorIndex+sec;
    DFS_SeekSectorCount(cursec);
    pos-=sec*SectorSize;
  }
  
  if(pos!=0){
    u8 tmpbuf[SectorSize];
    DFS_ReadSectors(tmpbuf,1);
    u32 tmpsize=size;
    if((SectorSize-pos)<tmpsize) tmpsize=SectorSize-pos;
    for(u32 idx=0;idx<tmpsize;idx++){
      tmpbuf[pos+idx]=*pbuf++;
    }
    DFS_SeekSectorCount(cursec);
    DFS_WriteSectors(tmpbuf,1);
    cursec++;
    size-=tmpsize;
  }
  
  if(((u32)pbuf&1)!=0){
    _consolePrintf("DFS_PrgJpeg_fread: Memory allign error. (%d, 0x%08x)\n",FileHandle,pbuf);
    while(SectorSize<=size){
      u8 tmpbuf[SectorSize];
      for(u32 idx=0;idx<SectorSize;idx++){
        tmpbuf[idx]=*pbuf++;
      }
      DFS_WriteSectors(tmpbuf,1);
      cursec++;
      size-=SectorSize;
    }
    }else{
    u32 reqsec=size/SectorSize;
    DFS_WriteSectors(pbuf,reqsec);
    cursec+=reqsec;
    pbuf+=reqsec*SectorSize;
    size-=reqsec*SectorSize;
  }
  
  if(size!=0){
    u8 tmpbuf[SectorSize];
    DFS_ReadSectors(tmpbuf,1);
    for(u32 idx=0;idx<size;idx++){
      tmpbuf[idx]=*pbuf++;
    }
    DFS_SeekSectorCount(cursec);
    DFS_WriteSectors(tmpbuf,1);
    cursec++;
  }
}





