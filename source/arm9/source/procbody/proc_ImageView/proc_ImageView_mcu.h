
DATA_IN_IWRAM_ImageView static u32 MCUXCount,MCUYCount,MCUCount;

DATA_IN_IWRAM_ImageView static u32 MCUReadedLineCount;

// TMCUCache_Body.pMCUには明示的なメモリキャッシュシステムが入っているため、safemallocを使わない。
typedef struct {
  u32 Life;
  u8 *pMCU;
} TMCUCache_Body;

typedef struct {
  u32 MCULife;
  TMCUCache_Body *pBody;
} TMCUCache;

DATA_IN_IWRAM_ImageView static TMCUCache MCUCache;

DATA_IN_IWRAM_ImageView static u8 *pWorkMCUBuf;

static u32 GetMCUOffsetPerSectorFromIndex(u32 MCUIndex);

static bool safemalloc_CallBack_RequestFreeMemory_ImageView_MCU(void)
{
  u32 LastLife=0xffffffff;
  u32 LastIndex=0;
  
  for(u32 idx=0;idx<MCUCount;idx++){
    u32 Life=MCUCache.pBody[idx].Life;
    if(Life!=0){
      if(Life<LastLife){
        LastLife=Life;
        LastIndex=idx;
      }
    }
  }
  
//  _consolePrintf("Free memory %d.\n",LastIndex);
  
  if(LastLife==0xffffffff){
    _consolePrint("Can not found last MCU.\n");
    return(false);
  }
  
  TMCUCache_Body *pTagBody=&MCUCache.pBody[LastIndex];
  pTagBody->Life=0;
  if(pTagBody->pMCU!=NULL){
    free(pTagBody->pMCU); pTagBody->pMCU=NULL;
  }
  
  return(true);
}

#include "proc_ImageView_dfs.h"

static bool MCU_Init(void)
{
  MCUXCount=(SrcWidth+(MCUSize-1))/MCUSize;
  MCUYCount=(SrcHeight+(MCUSize-1))/MCUSize;
  MCUCount=MCUXCount*MCUYCount;
  
  DFS_Init(GetMCUOffsetPerSectorFromIndex(MCUCount)*SectorSize+(64*1024)); // 64kbyteは念のため余剰
  
  MCUReadedLineCount=0;
  
  MCUCache.MCULife=1;
  MCUCache.pBody=(TMCUCache_Body*)safemalloc_chkmem(&MM_Process,sizeof(TMCUCache_Body)*MCUCount);
  _consolePrintf("MCUCache.pBody size is %dbyte.\n",sizeof(TMCUCache_Body)*MCUCount);
  MemSet32CPU(0,MCUCache.pBody,sizeof(TMCUCache_Body)*MCUCount);
  
  pWorkMCUBuf=(u8*)safemalloc_chkmem(&MM_Process,MCUXCount*MCUSize*MCUSize*3);
  
  safemalloc_CallBack_RequestFreeMemory_PlugImage=safemalloc_CallBack_RequestFreeMemory_ImageView_MCU;
  
  return(true);
}

static void MCU_Free(void)
{
  DFS_Free();
  Shell_FAT_fclose_SwapFile();
  
  safemalloc_CallBack_RequestFreeMemory_PlugImage=NULL;
  
  if(MCUCache.pBody!=NULL){
    for(u32 idx=0;idx<MCUCount;idx++){
      TMCUCache_Body *pTagBody=&MCUCache.pBody[idx];
      if(pTagBody->pMCU!=NULL){
        free(pTagBody->pMCU); pTagBody->pMCU=NULL;
      }
    }
    safefree(&MM_Process,MCUCache.pBody); MCUCache.pBody=NULL;
  }
  
  if(pWorkMCUBuf!=NULL){
    safefree(&MM_Process,pWorkMCUBuf); pWorkMCUBuf=NULL;
  }
}

static u32 GetMCUOffsetPerSectorFromIndex(u32 MCUIndex)
{
  u32 ofs=(MCUSize*MCUSize*3)*MCUIndex;
  return(ofs/SectorSize);
}

static u32 GetMCUOffsetPerSector(u32 xidx,u32 yidx)
{
  u32 MCUIndex=(yidx*MCUXCount)+xidx;
  return(GetMCUOffsetPerSectorFromIndex(MCUIndex));
}

static bool ReadMCUBlock(u32 RequestLineCount)
{
  if(RequestLineCount<=MCUReadedLineCount) return(false);
  
  u8 *pWorkReadBuf=(u8*)safemalloc_chkmem(&MM_Temp,MCUXCount*MCUSize*3);
  
  while(MCUReadedLineCount<RequestLineCount){
    {
        u32 keys=(~REG_KEYINPUT)&0x3ff;
      if((keys&KEY_B)!=0){
        RequestInterruptBreak=true;
        break;
      }
    }
    MemSet32CPU(0x80808080,pWorkReadBuf,MCUXCount*MCUSize*3);
    if(MCUReadedLineCount<SrcHeight){
      if(isPlugJpeg==true){
        PlugJpeg_GetBitmap24(MCUReadedLineCount,pWorkReadBuf);
      }else if(isPlugBmp==true){
          PlugBmp_GetBitmap24(MCUReadedLineCount,pWorkReadBuf);    
      }else if(isPlugGif==true){
          PlugGif_GetBitmap24(MCUReadedLineCount,pWorkReadBuf);    
      }else if(isPlugPsd==true){
          PlugPsd_GetBitmap24(MCUReadedLineCount,pWorkReadBuf);    
      }else if(isPlugPng==true){
          PlugPng_GetBitmap24(MCUReadedLineCount,pWorkReadBuf);    
      }else{
        pPluginBody->pIL->GetBitmap24(MCUReadedLineCount,pWorkReadBuf);
      }
      Preview_Set(MCUReadedLineCount,pWorkReadBuf);
    }
    
    u32 y=MCUReadedLineCount&MCUSizeMask;
    
    u8 *psrc=pWorkReadBuf,*pdst=&pWorkMCUBuf[y*MCUSize*3];
    for(u32 xidx=0;xidx<MCUXCount;xidx++){
      {
        u32 *_psrc=(u32*)psrc;
        u32 *_pdst=(u32*)pdst;
        for(u32 idx=0;idx<(MCUSize*3)/4;idx++){
          u32 col=*_psrc++;
          col=(col&0xfcfcfcfc)>>2;
          if( (col&(0x3f<<(8*0))) == (0x3f<<(8*0)) ) col-=1<<(8*0);
          if( (col&(0x3f<<(8*1))) == (0x3f<<(8*1)) ) col-=1<<(8*1);
          if( (col&(0x3f<<(8*2))) == (0x3f<<(8*2)) ) col-=1<<(8*2);
          if( (col&(0x3f<<(8*3))) == (0x3f<<(8*3)) ) col-=1<<(8*3);
          *_pdst++=col;
        }
      }
      psrc+=MCUSize*3;
      pdst+=MCUSize*MCUSize*3;
    }
    
    MCUReadedLineCount++;
    
    if((MCUReadedLineCount&MCUSizeMask)==0){
      u32 blocknum=(MCUReadedLineCount/MCUSize)-1;
      if(VerboseDebugLog==true) _consolePrintf("Store to swap file. block=%d.\n",blocknum);
      DFS_SeekSectorCount(GetMCUOffsetPerSector(0,blocknum));
      DFS_WriteSectors(pWorkMCUBuf,(MCUXCount*MCUSize*MCUSize*3)/SectorSize);
      Preview_Draw();
    }
  }
  
  if(pWorkReadBuf!=NULL){
    safefree(&MM_Temp,pWorkReadBuf); pWorkReadBuf=NULL;
  }
  
  return(true);
}

// -------------------------------------------------------

static u8* MCU_GetMCU(u32 xidx,u32 yidx)
{
  u32 MCUIndex=(yidx*MCUXCount)+xidx;
  
  TMCUCache_Body *pTargetBody=&MCUCache.pBody[MCUIndex];
  
  if(pTargetBody->pMCU!=NULL){
    pTargetBody->Life=MCUCache.MCULife;
    MCUCache.MCULife++;
    return(pTargetBody->pMCU);
  }
  
  pTargetBody->pMCU=(u8*)malloc(MCUSize*MCUSize*3);
  
  if(pTargetBody->pMCU!=NULL){
    pTargetBody->Life=MCUCache.MCULife;
    MCUCache.MCULife++;
    DFS_SeekSectorCount(GetMCUOffsetPerSectorFromIndex(MCUIndex));
    DFS_ReadSectors(pTargetBody->pMCU,(MCUSize*MCUSize*3)/SectorSize);
    return(pTargetBody->pMCU);
  }
  
  u32 LastLife=0xffffffff;
  u32 LastIndex=0;
  
  for(u32 idx=0;idx<MCUCount;idx++){
    u32 Life=MCUCache.pBody[idx].Life;
    if(Life!=0){
      if(Life<LastLife){
        LastLife=Life;
        LastIndex=idx;
      }
    }
  }
  
//  _consolePrintf("Swap memory %d->%d.\n",LastIndex,MCUIndex);
  
  if(LastLife==0xffffffff) StopFatalError(17704,"Can not found last MCU.\n");
  
  TMCUCache_Body *pLastBody=&MCUCache.pBody[LastIndex];
  
  pTargetBody->Life=MCUCache.MCULife;
  MCUCache.MCULife++;
  pTargetBody->pMCU=pLastBody->pMCU;
  pLastBody->Life=0;
  pLastBody->pMCU=NULL;
  
  DFS_SeekSectorCount(GetMCUOffsetPerSectorFromIndex(MCUIndex));
  DFS_ReadSectors(pTargetBody->pMCU,(MCUSize*MCUSize*3)/SectorSize);
  return(pTargetBody->pMCU);
}

static bool MCU_isLastDecoded(void)
{
  if(MCUReadedLineCount<SrcHeight) return(false);
  return(true);
}
