
#define MMEnumCount (9)
TMM *MMEnum[MMEnumCount];

TMM MM_Temp,MM_System,MM_SystemAfter,MM_Skin,MM_DLLImage,MM_DLLSound,MM_DLLDPG,MM_PlayList,MM_Process;

#define MMListExpandUnitCount (32)

void MM_Init(void)
{
  safemalloc_CallBack_RequestFreeMemory_PlugSound=NULL;
  safemalloc_CallBack_RequestFreeMemory_PlugImage=NULL;
  
  for(u32 mmidx=0;mmidx<MMEnumCount;mmidx++){
    TMM *pMM=NULL;
    switch(mmidx){
      case 0: pMM=&MM_Temp; pMM->pName="Temp"; break;
      case 1: pMM=&MM_System; pMM->pName="System"; break;
      case 2: pMM=&MM_SystemAfter; pMM->pName="SystemAfter"; break;
      case 3: pMM=&MM_Skin; pMM->pName="Skin"; break;
      case 4: pMM=&MM_DLLImage; pMM->pName="DLLImage"; break;
      case 5: pMM=&MM_DLLSound; pMM->pName="DLLSound"; break;
      case 6: pMM=&MM_DLLDPG; pMM->pName="DLLDPG"; break;
      case 7: pMM=&MM_PlayList; pMM->pName="PlayList"; break;
      case 8: pMM=&MM_Process; pMM->pName="Process"; break;
    }
    if(pMM==NULL) StopFatalError(0,"Unknown memory manager ID.\n");
    MMEnum[mmidx]=pMM;
    pMM->ListCount=0;
    pMM->pLists=NULL;
  }
}

void MM_ShowAllocated(TMM *pMM)
{
  _consoleLogPause();
  _consolePrintf("Allocated memory information. [%s]\n",pMM->pName);
  
  for(u32 idx=0;idx<pMM->ListCount;idx++){
    TMM_List *pList=&pMM->pLists[idx];
    if(pList->adr!=0){
      _consolePrintf("idx=%d adr=0x%08x size=%d %s:%d %s",idx,pList->adr,pList->size,pList->filename,pList->linenum,pList->funcname);
      if(pList->locked==false){
        _consolePrint("\n");
        }else{
        _consolePrint(" locked.\n");
      }
    }
  }
  
  _consolePrint("------------------\n");
  _consoleLogResume();
}

static void MM_Compact_ins(TMM *pMM)
{
  u32 maxlistcount=0;
  for(u32 idx=0;idx<pMM->ListCount;idx++){
    TMM_List *pList=&pMM->pLists[idx];
    if(pList->adr!=0) maxlistcount=idx+1;
  }
  
  if(pMM->ListCount==maxlistcount) return;
  
  if(maxlistcount==0){
    pMM->ListCount=0;
    if(pMM->pLists!=NULL){
      free(pMM->pLists); pMM->pLists=NULL;
    }
    return;
  }
  
  maxlistcount=(maxlistcount+(MMListExpandUnitCount-1))&~(MMListExpandUnitCount-1);
  if(pMM->ListCount==maxlistcount) return;
  
  if(VerboseDebugLog==true) _consolePrintf("Compact MemMgr array. [%s] %d->%d\n",pMM->pName,pMM->ListCount,maxlistcount);
  
  pMM->ListCount=maxlistcount;
  
  // 明示的にsafereallocを使わない。（Lockされるから）
  pMM->pLists=(TMM_List*)realloc(pMM->pLists,pMM->ListCount*sizeof(TMM_List));
  if(pMM->pLists==NULL) StopFatalError(13401,"MM_Compact: Memory overflow. [%s]\n",pMM->pName);
}

void MM_Compact(void)
{
  for(u32 mmidx=0;mmidx<MMEnumCount;mmidx++){
    TMM *pMM=MMEnum[mmidx];
    MM_Compact_ins(pMM);
  }
}

static void MM_Set(TMM *pMM,const char *filename,int linenum,const char *funcname,u32 adr,u32 size)
{
  for(u32 idx=0;idx<pMM->ListCount;idx++){
    TMM_List *pList=&pMM->pLists[idx];
    if(pList->adr==0){
      pList->adr=adr;
      pList->size=size;
      pList->filename=filename;
      pList->linenum=linenum;
      pList->funcname=funcname;
      return;
    }
  }
  
  { // 拡張するための空きメモリを先にチェック
    u32 reqsize=(pMM->ListCount+256)*sizeof(TMM_List);
    void *p=malloc(reqsize);
    while(p==NULL){
      if(CallRequestFree()==false) StopFatalError(13402,"Expand memory overflow. [%s]\n",pMM->pName);
      p=malloc(reqsize);
    }
    free(p); p=NULL;
  }
  
  u32 lastmaxlistcount=pMM->ListCount;
  pMM->ListCount+=MMListExpandUnitCount;
  
  if(VerboseDebugLog==true) _consolePrintf("Expand MemMgr array. [%s] %d->%d\n",pMM->pName,lastmaxlistcount,pMM->ListCount);
  
  // 明示的にsafereallocを使わない。（Lockされるから）
  pMM->pLists=(TMM_List*)realloc(pMM->pLists,pMM->ListCount*sizeof(TMM_List));
  
  for(u32 idx=lastmaxlistcount;idx<pMM->ListCount;idx++){
    TMM_List *pList=&pMM->pLists[idx];
    pList->adr=0;
    pList->size=0;
    pList->filename="";
    pList->linenum=0;
    pList->funcname="";
    pList->locked=false;
  }
  
  TMM_List *pList=&pMM->pLists[lastmaxlistcount];
  pList->adr=adr;
  pList->size=size;
  pList->filename=filename;
  pList->linenum=linenum;
  pList->funcname=funcname;
}

static void MM_Clear(TMM *pMM,u32 adr)
{
  for(u32 idx=0;idx<pMM->ListCount;idx++){
    TMM_List *pList=&pMM->pLists[idx];
    if(pList->adr==adr){
      if(pList->locked==true) StopFatalError(13403,"This addres is locked. [%s] (0x%08x)\n",pMM->pName,adr);
      pList->adr=0;
      return;
    }
  }
  
  StopFatalError(13404,"Can not clear. Not found adr. [%s] (0x%08x)\n",pMM->pName,adr);
}

static u32 MM_GetSize(TMM *pMM,u32 adr)
{
  for(u32 idx=0;idx<pMM->ListCount;idx++){
    TMM_List *pList=&pMM->pLists[idx];
    if(pList->adr==adr){
      return(pList->size);
    }
  }
  
  StopFatalError(13405,"Can not get size. Not found adr. [%s] (0x%08x)\n",pMM->pName,adr);
  return(0);
}

void MM_CheckMemoryLeak(TMM *pMM)
{
  bool haltflag=false;
  
  for(u32 idx=0;idx<pMM->ListCount;idx++){
    TMM_List *pList=&pMM->pLists[idx];
    if((pList->locked==false)&&(pList->adr!=0)){
      if(haltflag==false){
        haltflag=true;
        _consolePrint("Memory leak detected.\n");
      }
      u32 size=pList->size;
      u8 *pbuf=(u8*)pList->adr;
      _consolePrintf("adr=0x%08x size=%d %s:%d %s\n",pbuf,size,pList->filename,pList->linenum,pList->funcname);
      for(u32 idx=0;idx<8;idx++){
        _consolePrintf("%02x,",pbuf[idx]);
      }
      for(u32 idx=0;idx<8;idx++){
        if((0x20<=(u8)pbuf[idx])&&((u8)pbuf[idx]<0xff)){
          _consolePrintf("%c",pbuf[idx]);
          }else{
          _consolePrint("_");
        }
      }
      _consolePrint("\n");
    }
  }
  
  if(haltflag==true) StopFatalError(13408,"Memory leak detected. [%s]\n",pMM->pName);
}

void MM_CheckOverRange(void)
{
  bool haltflag=false;
  
  for(u32 mmidx=0;mmidx<MMEnumCount;mmidx++){
    TMM *pMM=MMEnum[mmidx];
    for(u32 idx=0;idx<pMM->ListCount;idx++){
      TMM_List *pList=&pMM->pLists[idx];
      if(pList->adr!=0){
        u32 size=pList->size;
        u8 *pbuf=(u8*)pList->adr;
        
        if((pbuf[-8]!=0xa8)||(pbuf[-7]!=0xa7)||(pbuf[-6]!=0xa6)||(pbuf[-5]!=0xa5)||(pbuf[-4]!=0xa4)||(pbuf[-3]!=0xa3)||(pbuf[-2]!=0xa2)||(pbuf[-1]!=0xa1)||
           (pbuf[size+0]!=0xb0)||(pbuf[size+1]!=0xb1)||(pbuf[size+2]!=0xb2)||(pbuf[size+3]!=0xb3)||(pbuf[size+4]!=0xb4)||(pbuf[size+5]!=0xb5)||(pbuf[size+6]!=0xb6)||(pbuf[size+7]!=0xb7)){
          if(haltflag==false){
            haltflag=true;
            _consolePrintf("Memory check error. Illigal writing code? [%s]\n",pMM->pName);
          }
          _consolePrintf("adr=0x%08x size=%d %s:%d %s\n",pbuf,size,pList->filename,pList->linenum,pList->funcname);
          for(u32 idx=8;idx>0;idx--){
            _consolePrintf("%02x,",pbuf[-idx]);
          }
          _consolePrint("\n");
          for(u32 idx=0;idx<8;idx++){
            _consolePrintf("%02x,",pbuf[idx]);
          }
          _consolePrint("\n");
          for(u32 idx=0;idx<8;idx++){
            _consolePrintf("%02x,",pbuf[size+idx]);
          }
          _consolePrint("\n");
        }
      }
    }
  }
  
  if(haltflag==true) StopFatalError(13409,"Memory check error. Illigal writing code?\n");
}

void MM_MemoryLock(TMM *pMM,void *ptr)
{
  for(u32 idx=0;idx<pMM->ListCount;idx++){
      TMM_List *pList=&pMM->pLists[idx];
      if(pList->adr==(u32)ptr) {
          pList->locked=true;
          //_consolePrintf("Locked. [%s] (0x%08x)\n",pMM->pName,adr);
          return;
      }
  }
  
  StopFatalError(13410,"Can not Lock Memory. Not found adr. [%s] (0x%08x)\n",pMM->pName,(u32)ptr);
}

void MM_MemoryUnlock(TMM *pMM,void *ptr)
{
  for(u32 idx=0;idx<pMM->ListCount;idx++){
      TMM_List *pList=&pMM->pLists[idx];
    if(pList->adr==(u32)ptr) {
        pList->locked=false;
        //_consolePrintf("Unlocked. [%s] (0x%08x)\n",pMM->pName,adr);
        return;
    }
  }
  
  StopFatalError(13411,"Can not Unlock Memory. Not found adr. [%s] (0x%08x)\n",pMM->pName,(u32)ptr);
}

void MM_ExecuteForceAllFree(TMM *pMM)
{
  for(u32 idx=0;idx<pMM->ListCount;idx++){
    TMM_List *pList=&pMM->pLists[idx];
    if(pList->adr!=0){
      safefree(pMM,(void*)(pList->adr));
    }
  }
}

