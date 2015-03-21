
static bool Shell_FAT_FullAlias_ReadFileAlloc(TMM *pMM,const char *pFullAlias,void **pbuf,s32 *psize)
{
  FAT_FILE *pf=FAT2_fopen_AliasForRead(pFullAlias);
  
  if(pf==NULL){
    _consolePrintf("Shell_FAT_ReadAlloc: (%s) File not found.\n",pFullAlias);
    return(false);
  }
  
  *psize=FAT2_GetFileSize(pf);
  
  if(*psize==0){
    _consolePrintf("Shell_FAT_ReadAlloc: (%s) File size == 0\n",pFullAlias);
    FAT2_fclose(pf);
    return(false);
  }
  
  *pbuf=safemalloc_chkmem(pMM,*psize);
  
  if(FAT2_fread(*pbuf,1,*psize,pf)!=*psize){
    safefree(pMM,pbuf);
    _consolePrintf("Shell_FAT_ReadAlloc: (%s) File size error.\n",pFullAlias);
    FAT2_fclose(pf);
    return(false);
  }
  
  FAT2_fclose(pf);
  
  return(true);
}

bool Shell_FAT_ReadAlloc(TMM *pMM,const char *fn,void **pbuf,s32 *psize)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultDataPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(str_isEmpty(pfullalias)==true) return(false);
  return(Shell_FAT_FullAlias_ReadFileAlloc(pMM,pfullalias,pbuf,psize));
}

// ----------------------------------------

DATA_IN_AfterSystem static FAT_FILE *pMSPPackageFileHandle;
DATA_IN_AfterSystem static u8 *pMSPPackageHeader;

CODE_IN_AfterSystem void Shell_FAT_ReadMSP_Open(void)
{
  pMSPPackageFileHandle=Shell_FAT_fopen_Internal(MSPPackageFilename);
  pMSPPackageHeader=(u8*)safemalloc_chkmem(&MM_Temp,512);
  FAT2_fread_fast(pMSPPackageHeader,1,512,pMSPPackageFileHandle);
}

CODE_IN_AfterSystem void Shell_FAT_ReadMSP_Close(void)
{
  if(pMSPPackageHeader!=NULL){
    safefree(&MM_Temp,pMSPPackageHeader); pMSPPackageHeader=NULL;
  }
  
  FAT2_fclose(pMSPPackageFileHandle); pMSPPackageFileHandle=NULL;
}

CODE_IN_AfterSystem static void Shell_FAT_ReadMSP_GetOffset(const char *fn,u32 *pOffset,u32 *pSize)
{
  *pOffset=0;
  *pSize=0;
  
  u8 *phead=pMSPPackageHeader;
  
  u32 MSPsCount=*phead++;
  for(u32 idx=0;idx<MSPsCount;idx++){
    char tagfn[16];
    u32 tagfnlen=*phead++;
    for(u32 idx=0;idx<tagfnlen;idx++){
      tagfn[idx]=*phead++;
    }
    tagfn[tagfnlen]=0;
    if(isStrEqual(fn,tagfn)==true){
      *pOffset=(phead[3]<<24)|(phead[2]<<16)|(phead[1]<<8)|phead[0];
      phead+=4;
      *pSize=(phead[3]<<24)|(phead[2]<<16)|(phead[1]<<8)|phead[0];
      return;
    }
    phead+=8;
  }
  
  StopFatalError(0,"Not found plugin in plugin package. [%s]",fn);
}

CODE_IN_AfterSystem void Shell_FAT_ReadMSP_ReadHeader(const char *fn,void *buf,s32 size)
{
  u32 ofs,size_dummy;
  Shell_FAT_ReadMSP_GetOffset(fn,&ofs,&size_dummy);
  
  FAT2_fseek(pMSPPackageFileHandle,ofs,SEEK_SET);
  FAT2_fread(buf,1,size,pMSPPackageFileHandle);
}
  
CODE_IN_AfterSystem void Shell_FAT_ReadMSP_ReadBody(TMM *pMM,const char *fn,void **pbuf,s32 *psize)
{
  u32 ofs,size;
  Shell_FAT_ReadMSP_GetOffset(fn,&ofs,&size);
  
  FAT2_fseek(pMSPPackageFileHandle,ofs,SEEK_SET);
  *pbuf=safemalloc_chkmem(pMM,size);
  *psize=size;
  if(FAT2_fread(*pbuf,1,size,pMSPPackageFileHandle)!=size) StopFatalError(0,"Plugin load error.");
}

// ----------------------------------------

