
DATA_IN_IWRAM_DPGPlay static UnicodeChar **ppBGMFilenameList;
DATA_IN_IWRAM_DPGPlay static u32 BGMListCount;
DATA_IN_IWRAM_DPGPlay static u32 BGMListIndex;

DATA_IN_IWRAM_DPGPlay static int ShuffleCount=-1;
DATA_IN_IWRAM_DPGPlay static int *pShuffleList=NULL;

DATA_IN_IWRAM_DPGPlay static u32 BGMResumeSaveTimeVSync;

static void BGM_Init(void)
{
  ppBGMFilenameList=NULL;
  BGMListCount=0;
  BGMListIndex=0;
  
  BGMResumeSaveTimeVSync=0;
}

#define ATTRIB_ARCH  0x20
#define ATTRIB_HID  0x02
#define ATTRIB_SYS  0x04
#define ATTRIB_RO  0x01

static void BGM_CreateListFromPath(const UnicodeChar *pFilePathUnicode)
{
  const char *pBasePathAlias=ConvertFull_Unicode2Alias(pFilePathUnicode,NULL);
  
  if(VerboseDebugLog==true) _consolePrintf("pBasePathAlias=%s\n",pBasePathAlias);
  
  BGMListCount=0;
  
  if(FAT2_chdir_Alias(pBasePathAlias)==false) return;
  
  u8 AttribChk=0;
      
  if(ProcState.FileList.HideAttribute_Archive) AttribChk|=ATTRIB_ARCH;
  if(ProcState.FileList.HideAttribute_Hidden) AttribChk|=ATTRIB_HID;
  if(ProcState.FileList.HideAttribute_System) AttribChk|=ATTRIB_SYS;
  if(ProcState.FileList.HideAttribute_ReadOnly) AttribChk|=ATTRIB_RO;
    
  {
    const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
        if((FAT2_GetAttrib()&AttribChk)==0){
            switch(FAT_FileType){
                case FT_NONE: break;
                case FT_DIR: break;
                case FT_FILE: {
                    if(isStrEqual(&pafn[strlen(pafn)-4],".DPG")==true){
                        BGMListCount++;
                    }
                } break;
            }
        }
        FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
  if(BGMListCount==0) return;
  
  pShuffleList=(int*)safemalloc_chkmem(&MM_Process,BGMListCount*sizeof(int));
  ppBGMFilenameList=(UnicodeChar**)safemalloc_chkmem(&MM_Process,BGMListCount*4);
  for(u32 idx=0;idx<BGMListCount;idx++){
    ppBGMFilenameList[idx]=NULL;
  }
  
  BGMListCount=0;
  
  {
    const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
        if((FAT2_GetAttrib()&AttribChk)==0){
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
  
  if(pShuffleList!=NULL){
	  safefree(&MM_Process,pShuffleList); pShuffleList=NULL;
  }
  
  BGMListCount=0;
  BGMListIndex=0;
  ShuffleCount=-1;
}

static void Shuffle_Refresh(int TopIndex)
{
	if(ShuffleCount==BGMListCount) return;
  ShuffleCount=BGMListCount;
  
  pShuffleList[0]=TopIndex;
  for(int idx=1;idx<ShuffleCount;idx++){
    pShuffleList[idx]=-1;
  }
  
  for(int idx=0;idx<ShuffleCount;idx++){
    if(idx!=TopIndex){
      int r=(rand()%ShuffleCount)+1;
      int fidx=0;
      while(r!=0){
        fidx=(fidx+1)%ShuffleCount;
        if(pShuffleList[fidx]==-1) r--;
      }
//      _consolePrintf("ref%d,%d\n",fidx,idx);
      pShuffleList[fidx]=idx;
    }
  }
  
}

static void BGM_Start(const UnicodeChar *pFilePathUnicode,const UnicodeChar *pFileNameUnicode)
{
  BGM_Stop();
  BGM_Free();
  
  BGM_CreateListFromPath(pFilePathUnicode);
  
  if(BGMListCount==0) StopFatalError(16702,"Can not found files.\n");
  
  BGMListIndex=BGM_GetIndexFromFilename(pFileNameUnicode);
  
  Shuffle_Refresh(BGMListIndex);
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
  Popup_Show_Prev();
  BGM_PlayCurrentIndex();
}

void BGM_PrevShuffle(void)
{
  BGM_Stop();
  
  u32 LastIndex=BGMListIndex;
  if(ShuffleCount!=BGMListCount) Shuffle_Refresh(LastIndex);
  
  if(BGMListCount!=1){
	  for(int idx=0;idx<ShuffleCount;idx++){
		  if(pShuffleList[idx]==LastIndex){
			  if(idx==0){
				  //ShuffleCount=-1;
				  BGMListIndex=pShuffleList[ShuffleCount-1];
			  }else{
				  BGMListIndex=pShuffleList[idx-1];
			  }
			  break;
		  }
	  }
  }
  
  Popup_Show_Prev();
  BGM_PlayCurrentIndex();
}

static void BGM_Next(void)
{
  BGM_Stop();
  
  if(BGMListCount==0) return;
  
  BGMListIndex++;
  if(BGMListIndex==BGMListCount) BGMListIndex=0;
  
  Popup_Show_Next();
  BGM_PlayCurrentIndex();
}

void BGM_NextShuffle(void)
{
  BGM_Stop();
  
  u32 LastIndex=BGMListIndex;
  if(ShuffleCount!=BGMListCount) Shuffle_Refresh(LastIndex);
  
  if(BGMListCount!=1){
	  for(int idx=0;idx<ShuffleCount;idx++){
		  if(pShuffleList[idx]==LastIndex){
			  if(idx==(ShuffleCount-1)){
				  //ShuffleCount=-1;
				  BGMListIndex=pShuffleList[0];
			  }else{
				  BGMListIndex=pShuffleList[idx+1];
			  }
			  break;
		  }
	  }
  }
  
  Popup_Show_Next();
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
