#ifndef PROCSTATE_DFS_H_
#define PROCSTATE_DFS_H_

#include "fat2.h"
#include "disc_io.h"

typedef struct {
	u32 Sector;
	u32 Count;
} TBurstList;

DATA_IN_AfterSystem static u32 BurstListCount;
DATA_IN_AfterSystem static TBurstList *pBurstList;

DATA_IN_IWRAM_MainPass static void CreateBurstList(FAT_FILE *pFileHandle)
{
	_consolePrint("CreateBurstList.\n");
  
	BurstListCount=0;
  
	{
		u32 CurClus=pFileHandle->firstCluster;
		BurstListCount++;
		while(1){
			if(CurClus==CLUSTER_FREE) StopFatalError(0,"cluster link pre-search stopped.\n");
      
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
		u32 CurClus=pFileHandle->firstCluster;
    
		pBurstList[BurstListIndex].Sector=FAT2_ClustToSect(CurClus);
		pBurstList[BurstListIndex].Count=SecPerClus;
		BurstListIndex++;
    
		while(1){
			if(CurClus==CLUSTER_FREE) StopFatalError(0,"cluster link search stopped.\n");
			
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
	//	_consolePrintf("Index=%d Sector=0x%x Count=%d\n",idx,pBurstList[idx].Sector,pBurstList[idx].Count);
	//}
}

static void FreeBurstList(void)
{
	BurstListCount=0;
	if(pBurstList!=NULL){
		safefree(&MM_System,pBurstList); pBurstList=NULL;
	}
}

// ------------------------------------

#define SectorSize (512)

typedef struct {
	u32 BurstListIndex;
	u32 BurstListCurSector;
	u32 BurstListRemainSectorCount;
} TDFS_File;

// ------------------------------------

static void ProcState_DFS_Free(void)
{
	FreeBurstList();
}

static void ProcState_DFS_Write32bit(u8 *pbuf,u32 SectorsCount)
{
  TDFS_File dfsf;

	dfsf.BurstListIndex=0;
	dfsf.BurstListCurSector=pBurstList[dfsf.BurstListIndex].Sector;
	dfsf.BurstListRemainSectorCount=pBurstList[dfsf.BurstListIndex].Count;

	while(SectorsCount!=0){
		if(dfsf.BurstListRemainSectorCount==0){
			dfsf.BurstListIndex++;
			dfsf.BurstListCurSector=pBurstList[dfsf.BurstListIndex].Sector;
			dfsf.BurstListRemainSectorCount=pBurstList[dfsf.BurstListIndex].Count;
			if(dfsf.BurstListRemainSectorCount==0) StopFatalError(0,"Insufficient swap memory for write.\n");
		}
    
		u32 reqsector=SectorsCount;
		if(dfsf.BurstListRemainSectorCount<reqsector) reqsector=dfsf.BurstListRemainSectorCount;
		if(BlockWriteLimitSectorsCount<reqsector) reqsector=BlockWriteLimitSectorsCount;
		if(dfsf.BurstListCurSector==CLUSTER_EOF) break;
		//    _consolePrintf("[%d,%d,%d,%d]",dfsf.BurstListIndex,dfsf.BurstListCurSector,dfsf.BurstListRemainSectorCount,reqsector);
		disc_WriteSectors(dfsf.BurstListCurSector,reqsector,pbuf);
		pbuf+=reqsector*SectorSize;
		SectorsCount-=reqsector;

		dfsf.BurstListCurSector+=reqsector;
		dfsf.BurstListRemainSectorCount-=reqsector;
	}
}

static void ProcState_DFS_Read32bit(u8 *pbuf,u32 SectorsCount){
	TDFS_File dfsf;

	dfsf.BurstListIndex=0;
	dfsf.BurstListCurSector=pBurstList[dfsf.BurstListIndex].Sector;
	dfsf.BurstListRemainSectorCount=pBurstList[dfsf.BurstListIndex].Count;

	while(SectorsCount!=0){
		if(dfsf.BurstListRemainSectorCount==0){
			dfsf.BurstListIndex++;
			dfsf.BurstListCurSector=pBurstList[dfsf.BurstListIndex].Sector;
			dfsf.BurstListRemainSectorCount=pBurstList[dfsf.BurstListIndex].Count;
			if(dfsf.BurstListRemainSectorCount==0) StopFatalError(0,"Insufficient swap memory for read.\n");
		}

		u32 reqsector=SectorsCount;
		if(dfsf.BurstListRemainSectorCount<reqsector) reqsector=dfsf.BurstListRemainSectorCount;
		if(BlockReadLimitSectorsCount<reqsector) reqsector=BlockReadLimitSectorsCount;
		if(dfsf.BurstListCurSector==CLUSTER_EOF) break;
		//    _consolePrintf("[%d,%d,%d,%d]",dfsf.BurstListIndex,dfsf.BurstListCurSector,dfsf.BurstListRemainSectorCount,reqsector);
		disc_ReadSectors(dfsf.BurstListCurSector,reqsector,pbuf);
		pbuf+=reqsector*SectorSize;
		SectorsCount-=reqsector;

		dfsf.BurstListCurSector+=reqsector;
		dfsf.BurstListRemainSectorCount-=reqsector;
	}
}

#endif /*PROCSTATE_DFS_H_*/
