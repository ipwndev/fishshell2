
static UnicodeChar **ppBGMFilenameList;
static u32 BGMListCount;
static u32 BGMListIndex;

static u32 BGMResumeSaveTimeVSync;

static void BGM_Init(void)
{
  ppBGMFilenameList=NULL;
  BGMListCount=0;
  BGMListIndex=0;
  
  BGMResumeSaveTimeVSync=0;
}

static void BGM_CreateListFromPath(const UnicodeChar *pFilePathUnicode)
{
  const char *pBasePathAlias=ConvertFull_Unicode2Alias(pFilePathUnicode,NULL);
  
  _consolePrintf("pBasePathAlias=%s\n",pBasePathAlias);
  
  BGMListCount=0;
  
  if(FAT2_chdir_Alias(pBasePathAlias)==false) return;
  
  {
    const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
      switch(FAT_FileType){
        case FT_NONE: break;
        case FT_DIR: break;
        case FT_FILE: {
          if(isStrEqual(&pafn[strlen(pafn)-4],".DPG")==true){
            BGMListCount++;
          }
        } break;
      }
      
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
  if(BGMListCount==0) return;
  
  ppBGMFilenameList=(UnicodeChar**)safemalloc_chkmem(&MM_Process,BGMListCount*4);
  for(u32 idx=0;idx<BGMListCount;idx++){
    ppBGMFilenameList[idx]=NULL;
  }
  
  BGMListCount=0;
  
  {
    const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
      switch(FAT_FileType){
        case FT_NONE: break;
        case FT_DIR: break;
        case FT_FILE: {
          if(isStrEqual(&pafn[strlen(pafn)-4],".DPG")==true){
            // _consolePrintf("Find file. %s [%s]\n",pafn,&pafn[strlen(pafn)-4]);
            const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
            if(pufn==NULL) StopFatalError(16701,"Can not found Long filename entry.\n");
            ppBGMFilenameList[BGMListCount]=Unicode_AllocateCopy(&MM_Process,pufn);
            BGMListCount++;
          }
        } break;
      }
      
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
  for(u32 idx1=0;idx1<BGMListCount-1;idx1++){
    for(u32 idx2=idx1+1;idx2<BGMListCount;idx2++){
      if(isSwapFilenameUnicode(ppBGMFilenameList[idx1],ppBGMFilenameList[idx2])==true){
        UnicodeChar *ptmp=ppBGMFilenameList[idx1];
        ppBGMFilenameList[idx1]=ppBGMFilenameList[idx2];
        ppBGMFilenameList[idx2]=ptmp;
      }
    }
  }
}

static u32 BGM_GetIndexFromFilename(const UnicodeChar *pFileNameUnicode)
{
  for(u32 idx=0;idx<BGMListCount;idx++){
    if(Unicode_isEqual(ppBGMFilenameList[idx],pFileNameUnicode)==true) return(idx);
  }
  return(0);
}

static void BGM_PlayCurrentIndex(void)
{
  if(BGMListCount==0) return;
  
  DPGClose();
  DPGOpen(ppBGMFilenameList[BGMListIndex]);
  
  Resume_SetResumeMode(ERM_Video);
  Resume_SetFilename(FullPathUnicodeFromSplitItem(RelationalFilePathUnicode,ppBGMFilenameList[BGMListIndex]));
  Resume_SetPos(0);
  Resume_Save();
  
  BGMResumeSaveTimeVSync=1;
}

static void BGM_Stop(void)
{
  DPGClose();
  
  Resume_SetResumeMode(ERM_None);
  Resume_Save();
  BGMResumeSaveTimeVSync=0;
}

static void BGM_Free(void)
{
  DPGClose();
  
  if(ppBGMFilenameList!=NULL){
    for(u32 idx=0;idx<BGMListCount;idx++){
      if(ppBGMFilenameList[idx]!=NULL){
        safefree(&MM_Process,ppBGMFilenameList[idx]); ppBGMFilenameList[idx]=NULL;
      }
    }
    BGMListCount=0;
    safefree(&MM_Process,ppBGMFilenameList); ppBGMFilenameList=NULL;
  }
  
  BGMListCount=0;
  BGMListIndex=0;
}

static void BGM_Start(const UnicodeChar *pFilePathUnicode,const UnicodeChar *pFileNameUnicode)
{
  BGM_Stop();
  BGM_Free();
  
  BGM_CreateListFromPath(pFilePathUnicode);
  
  if(BGMListCount==0) StopFatalError(16702,"Can not found files.\n");
  
  BGMListIndex=BGM_GetIndexFromFilename(pFileNameUnicode);
  BGM_PlayCurrentIndex();
}

static void BGM_Repeat(void)
{
  BGM_Stop();
  
  if(BGMListCount==0) return;
  
  BGM_PlayCurrentIndex();
}

static void BGM_Prev(void)
{
  BGM_Stop();
  
  if(BGMListCount==0) return;
  
  if(BGMListIndex==0){
    BGMListIndex=BGMListCount-1;
    }else{
    BGMListIndex--;
  }
  
  BGM_PlayCurrentIndex();
}

static void BGM_Next(void)
{
  BGM_Stop();
  
  if(BGMListCount==0) return;
  
  BGMListIndex++;
  if(BGMListIndex==BGMListCount) BGMListIndex=0;
  
  BGM_PlayCurrentIndex();
}

static void BGM_NextRandom(void)
{
  BGM_Stop();
  
  if(BGMListCount==0) return;
  
  if(BGMListCount!=1){
    u32 lastidx=BGMListIndex;
    BGMListIndex=rand()%BGMListCount;
    while(lastidx==BGMListIndex){
      BGMListIndex=rand()%BGMListCount;
    }
  }
  
  BGM_PlayCurrentIndex();
}

static bool BGM_isOpened(void)
{
  return(false);
}

static const UnicodeChar* BGM_GetCurrentFilename(void)
{
  return(ppBGMFilenameList[BGMListIndex]);
}
