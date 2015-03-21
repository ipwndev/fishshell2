
DATA_IN_AfterSystem static u32 MF_FilesCount;

#define ATTRIB_ARCH  0x20
#define ATTRIB_HID  0x02
#define ATTRIB_SYS  0x04
#define ATTRIB_RO  0x01

static __attribute__ ((noinline)) void PlayList_MakeFolder_Body(bool FindDeep,const UnicodeChar *pBasePathW,FAT_FILE *pfdst)
{
  u32 PathsCount=0;
  char **ppPathsA;
  UnicodeChar **ppPathsW;
  
  const u32 dstbufsize=4*1024;
  u32 dstbufpos=0;
  u8 *pdstbuf=(u8*)safemalloc_chkmem(&MM_Temp,dstbufsize);
  
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
        const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
        if(pufn==NULL) StopFatalError(11502,"Can not pre-read unicode filename.\n");
        switch(FAT_FileType){
          case FT_NONE: break;
          case FT_DIR: {
            if(strcmp(pafn,".")==0){
              }else{
              if(strcmp(pafn,"..")==0){
                }else{
                PathsCount++;
              }
            }
          } break;
          case FT_FILE: {
            if(DLLList_isSupportFormatFromFilenameAlias(pafn)==EPT_Sound){
              MF_FilesCount++;
              if((MF_FilesCount&63)==0){
                char msg0[64];
                snprintf(msg0,64,"Registed %dfiles.",MF_FilesCount);
                if(FindDeep==true) CallBack_MWin_ProgressDraw(msg0,"",0,0);
              }
              
              if((dstbufsize-2048)<=dstbufpos){
                FAT2_fwrite(pdstbuf,1,dstbufpos,pfdst);
                dstbufpos=0;
              }
              u32 pathlen=Unicode_GetLength(pBasePathW)+1;
              u32 filenamelen=Unicode_GetLength(pufn)+1;
              pdstbuf[dstbufpos++]=pathlen;
              pdstbuf[dstbufpos++]=filenamelen;
              MemCopy16CPU(pBasePathW,&pdstbuf[dstbufpos],pathlen*2);
              dstbufpos+=pathlen*2;
              MemCopy16CPU(pufn,&pdstbuf[dstbufpos],filenamelen*2);
              dstbufpos+=filenamelen*2;
            }
          } break;
        }
      }
      
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
  if(dstbufpos!=0) FAT2_fwrite(pdstbuf,1,dstbufpos,pfdst);
  
  if(pdstbuf!=NULL){
    safefree(&MM_Temp,pdstbuf); pdstbuf=NULL;
  }
  
  if(FindDeep==false) return;
  
  if(PathsCount==0) return;
  
  ppPathsA=(char**)safemalloc_chkmem(&MM_Temp,PathsCount*4);
  ppPathsW=(UnicodeChar**)safemalloc_chkmem(&MM_Temp,PathsCount*4);
  PathsCount=0;
  
  {
    const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
      if((FAT2_GetAttrib()&AttribChk)==0){
        const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
        if(pufn==NULL) StopFatalError(11504,"Can not read unicode filename.\n");
        switch(FAT_FileType){
          case FT_NONE: break;
          case FT_DIR: {
            if(strcmp(pafn,".")==0){
              }else{
              if(strcmp(pafn,"..")==0){
                }else{
                ppPathsA[PathsCount]=str_AllocateCopy(&MM_Temp,pafn);
                ppPathsW[PathsCount]=Unicode_AllocateCopy(&MM_Temp,pufn);
                PathsCount++;
              }
            }
          } break;
          case FT_FILE: break;
        }
      }
      
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
  for(u32 idx=0;idx<PathsCount;idx++){
    if(FAT2_chdir_Alias(ppPathsA[idx])==false) StopFatalError(11505,"Can not change to deep path. [%s]\n",ppPathsA[idx]);
    UnicodeChar *pDeepPath=Unicode_AllocateCopy(&MM_Temp,FullPathUnicodeFromSplitItem(pBasePathW,ppPathsW[idx]));
    PlayList_MakeFolder_Body(true,pDeepPath,pfdst);
    DTCM_StackCheck(-1);
    if(pDeepPath!=NULL){
      safefree(&MM_Temp,pDeepPath); pDeepPath=NULL;
    }
    if(FAT2_chdir_Alias("..")==false) StopFatalError(11506,"Can not recovery path. [%s]\n",ppPathsA[idx]);
    
    if(ppPathsA[idx]!=NULL){
      safefree(&MM_Temp,ppPathsA[idx]); ppPathsA[idx]=NULL;
    }
    if(ppPathsW[idx]!=NULL){
      safefree(&MM_Temp,ppPathsW[idx]); ppPathsW[idx]=NULL;
    }
  }
  
  if(ppPathsA!=NULL){
    safefree(&MM_Temp,ppPathsA); ppPathsA=NULL;
  }
  if(ppPathsW!=NULL){
    safefree(&MM_Temp,ppPathsW); ppPathsW=NULL;
  }
}

void PlayList_MakeFolder(bool FindDeep,const UnicodeChar *pBasePathUnicode)
{
  FileHeaderSectorNum=0;
  
  if(FindDeep==true) CallBack_MWin_ProgressShow("Searching...",0);
  
  FAT_FILE *pfdst=Shell_FAT_fopenwrite_Internal(ResumePlayListFilename);
  if(pfdst==NULL) StopFatalError(11507,"Resume play list file write error.\n");
  
  FileHeader.ID=FileHeader_ID;
  FileHeader.Position=0;
  FileHeader.Pause=false;
  FileHeader.Error=false;
  FileHeader.FullPathFilenameW[0]=0;
  FAT2_fwrite(&FileHeader,1,sizeof(FileHeader),pfdst);
  
  {
    const char *pBasePathAlias=ConvertFull_Unicode2Alias(pBasePathUnicode,NULL);
    _consolePrintf("PlayList_MakeFolder: Start alias path is [%s].\n",pBasePathAlias);
    if(FAT2_chdir_Alias(pBasePathAlias)==false) StopFatalError(11508,"Can not change current path.\n");
  }
  
  MF_FilesCount=0;
  PlayList_MakeFolder_Body(FindDeep,pBasePathUnicode,pfdst);
  _consolePrintf("Found files count= %d.\n",MF_FilesCount);
  
  u16 Terminater=0;
  FAT2_fwrite(&Terminater,1,2,pfdst);
  
  while((FAT2_GetFileSize(pfdst)&3)!=0){
    u32 dummy=0;
    FAT2_fwrite(&dummy,1,1,pfdst);
  }
  
  FAT2_fwrite(&MF_FilesCount,4,1,pfdst);
  
  FAT2_fclose(pfdst);
  
  if(FindDeep==true) CallBack_MWin_ProgressHide();
  
  MM_Compact();
  
  DTCM_StackCheck(-1);
}

