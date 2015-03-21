
// ----------------------------------------

bool FileExistsUnicode(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pFilenameUnicode)
{
  if((pBasePathUnicode==NULL)||(pBasePathUnicode[0]==0)){
    static UnicodeChar su[2]={(UnicodeChar)'/',0};
    pBasePathUnicode=su;
  }
  
  if((pFilenameUnicode==NULL)||(pFilenameUnicode[0]==0)) return(false);
  
  if(FAT2_chdir_Alias("/")==false) StopFatalError(13606,"Can not chdir to root.\n");
  
  if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  
  while(*pBasePathUnicode!=0){
    UnicodeChar pPathUnicode[MaxFilenameLength+1];
    u32 PathUnicodeCount=0;
    
    while((*pBasePathUnicode!=(UnicodeChar)'/')&&(*pBasePathUnicode!=0)){
      pPathUnicode[PathUnicodeCount]=*pBasePathUnicode++;
      PathUnicodeCount++;
    }
    pPathUnicode[PathUnicodeCount]=0;
    
    bool finded=false;
    const char *pafn=NULL;
    
    {
      u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
      
      while((FAT_FileType!=FT_NONE)&&(finded==false)){
        switch(FAT_FileType){
          case FT_NONE: break;
          case FT_DIR: {
            const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
            if(pufn==NULL){
              _consolePrintf("Unicode filename read error.\n Alias='%s'\n",pafn);
              }else{
              if(Unicode_isEqual_NoCaseSensitive(pufn,pPathUnicode)==true) finded=true;
            }
          } break;
          case FT_FILE: break;
        }
        
        if(finded==false) FAT_FileType=FAT2_FindNextFile(&pafn);
      }
    }
    
    if(finded==false){
//      _consolePrintf("FileExistsUnicode: Not find Unicode path item. [%s]\n",StrConvert_Unicode2Ank_Test(pPathUnicode));
      return(false);
      }else{
      if(FAT2_chdir_Alias(pafn)==false) StopFatalError(13607,"Can not chdir to %s.\n",pafn);
    }
    
    if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  }
  
  if(*pFilenameUnicode!=0){
    bool finded=false;
    
    {
      const char *pafn;
      u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
      
      while((FAT_FileType!=FT_NONE)&&(finded==false)){
        switch(FAT_FileType){
          case FT_NONE: break;
          case FT_DIR: break;
          case FT_FILE: {
            const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
            if(pufn==NULL){
              _consolePrintf("Unicode filename read error.\n Alias='%s'\n",pafn);
              }else{
              if(Unicode_isEqual_NoCaseSensitive(pufn,pFilenameUnicode)==true) finded=true;
            }
          } break;
        }
        
        if(finded==false) FAT_FileType=FAT2_FindNextFile(&pafn);
      }
    }
    
    if(finded==false){
//      _consolePrintf("FileExistsUnicode: Not find Unicode file item. [%s]\n",StrConvert_Unicode2Ank_Test(pFilenameUnicode));
      return(false);
    }
  }
  
  return(true);
}

bool PathExistsUnicode(const UnicodeChar *pBasePathUnicode)
{
  if((pBasePathUnicode==NULL)||(pBasePathUnicode[0]==0)){
    static UnicodeChar su[2]={(UnicodeChar)'/',0};
    pBasePathUnicode=su;
  }
  
  if(FAT2_chdir_Alias("/")==false) StopFatalError(13608,"Can not chdir to root.\n");
  
  if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  
  while(*pBasePathUnicode!=0){
    UnicodeChar pPathUnicode[MaxFilenameLength+1];
    u32 PathUnicodeCount=0;
    
    while((*pBasePathUnicode!=(UnicodeChar)'/')&&(*pBasePathUnicode!=0)){
      pPathUnicode[PathUnicodeCount]=*pBasePathUnicode++;
      PathUnicodeCount++;
    }
    pPathUnicode[PathUnicodeCount]=0;
    
    bool finded=false;
    const char *pafn=NULL;
    
    {
      u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
      
      while((FAT_FileType!=FT_NONE)&&(finded==false)){
        switch(FAT_FileType){
          case FT_NONE: break;
          case FT_DIR: {
            const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
            if(pufn==NULL){
              _consolePrintf("Unicode filename read error.\n Alias='%s'\n",pafn);
              }else{
              if(Unicode_isEqual_NoCaseSensitive(pufn,pPathUnicode)==true) finded=true;
            }
          } break;
          case FT_FILE: break;
        }
        
        if(finded==false) FAT_FileType=FAT2_FindNextFile(&pafn);
      }
    }
    
    if(finded==false){
      _consolePrint("Can not find Unicode path item.\n");
      return(false);
      }else{
      if(FAT2_chdir_Alias(pafn)==false) StopFatalError(13609,"Can not chdir to %s.\n",pafn);
    }
    
    if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  }
  
  return(true);
}

bool FullPath_FileExistsUnicode(const UnicodeChar *pFullPathUnicode)
{
  UnicodeChar *pPathUnicode=(UnicodeChar*)safemalloc_chkmem(&MM_Temp,(MaxFilenameLength+1)*2);
  UnicodeChar *pFilenameUnicode=(UnicodeChar*)safemalloc_chkmem(&MM_Temp,(MaxFilenameLength+1)*2);
  
  SplitItemFromFullPathUnicode(pFullPathUnicode,pPathUnicode,pFilenameUnicode);
  
  bool exists=FileExistsUnicode(pPathUnicode,pFilenameUnicode);
  
  if(pPathUnicode!=NULL){
    safefree(&MM_Temp,pPathUnicode); pPathUnicode=NULL;
  }
  if(pFilenameUnicode!=NULL){
    safefree(&MM_Temp,pFilenameUnicode); pFilenameUnicode=NULL;
  }
  
  return(exists);
}

bool FullPath_FileExistsAnsi(const char *pFullPathAnsi)
{
  UnicodeChar *pPathUnicode=(UnicodeChar*)safemalloc_chkmem(&MM_Temp,(MaxFilenameLength+1)*2);
  UnicodeChar *pFilenameUnicode=(UnicodeChar*)safemalloc_chkmem(&MM_Temp,(MaxFilenameLength+1)*2);
  
  UnicodeChar *pFullPathUnicode=(UnicodeChar*)safemalloc_chkmem(&MM_Temp,(MaxFilenameLength+1)*2);
  
  StrConvert_Ank2Unicode(pFullPathAnsi,pFullPathUnicode);
  SplitItemFromFullPathUnicode(pFullPathUnicode,pPathUnicode,pFilenameUnicode);
  
  bool exists=FileExistsUnicode(pPathUnicode,pFilenameUnicode);
  
  if(pPathUnicode!=NULL){
    safefree(&MM_Temp,pPathUnicode); pPathUnicode=NULL;
  }
  if(pFilenameUnicode!=NULL){
    safefree(&MM_Temp,pFilenameUnicode); pFilenameUnicode=NULL;
  }
  if(pFullPathUnicode!=NULL){
    safefree(&MM_Temp,pFullPathUnicode); pFullPathUnicode=NULL;
  }
  
  return(exists);
}

void SplitItemFromFullPathAlias(const char *pFullPathAlias,char *pPathAlias,char *pFilenameAlias)
{
  if((pPathAlias==NULL)||(pFilenameAlias==NULL)) StopFatalError(13610,"SplitItemFromFullPathAlias: output buffer is NULL.\n");
  
  if((pFullPathAlias==NULL)||(pFullPathAlias[0]==0)) StopFatalError(13611,"SplitItemFromFullPathAlias: pFullPathAlias is NULL or blank.\n");
  
  u32 SplitPos=0;
  {
    u32 idx=0;
    while(1){
      char uc=pFullPathAlias[idx];
      if(uc==0) break;
      if(uc=='/') SplitPos=idx+1;
      idx++;
    }
  }
  
  if(SplitPos<=1){
    pPathAlias[0]='/';
    pPathAlias[1]=0;
    }else{
    for(u32 idx=0;idx<SplitPos-1;idx++){
      pPathAlias[idx]=pFullPathAlias[idx];
    }
    pPathAlias[SplitPos-1]=0;
  }
  
  strcpy(pFilenameAlias,&pFullPathAlias[SplitPos]);
}

void SplitItemFromFullPathUnicode(const UnicodeChar *pFullPathUnicode,UnicodeChar *pPathUnicode,UnicodeChar *pFilenameUnicode)
{
  if((pPathUnicode==NULL)||(pFilenameUnicode==NULL)) StopFatalError(13612,"SplitItemFromFullPathUnicode: output buffer is NULL.\n");
  
  if((pFullPathUnicode==NULL)||(pFullPathUnicode[0]==0)) StopFatalError(13613,"SplitItemFromFullPathUnicode: pFullPathUnicode is NULL or blank.\n");
  
  u32 SplitPos=0;
  {
    u32 idx=0;
    while(1){
      UnicodeChar uc=pFullPathUnicode[idx];
      if(uc==0) break;
      if(uc==(UnicodeChar)'/') SplitPos=idx+1;
      idx++;
    }
  }
  
  if(SplitPos<=1){
    pPathUnicode[0]=(UnicodeChar)'/';
    pPathUnicode[1]=0;
    }else{
    for(u32 idx=0;idx<SplitPos-1;idx++){
      pPathUnicode[idx]=pFullPathUnicode[idx];
    }
    pPathUnicode[SplitPos-1]=0;
  }
  
  Unicode_Copy(pFilenameUnicode,&pFullPathUnicode[SplitPos]);
}

const UnicodeChar* FullPathUnicodeFromSplitItem(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pFilenameUnicode)
{
  if((pBasePathUnicode==NULL)||(pBasePathUnicode[0]==0)||(pBasePathUnicode[1]==0)){
    pBasePathUnicode=NULL;
  }
  
  if((pFilenameUnicode==NULL)||(pFilenameUnicode[0]==0)) StopFatalError(13622,"FullPathUnicodeFromSplitItem: pFilenameUnicode is NULL or blank.\n");
  
  static UnicodeChar FullPathUnicode[MaxFilenameLength];
  
  const UnicodeChar us[2]={(UnicodeChar)'/',0};
  if(pBasePathUnicode==NULL){
    Unicode_Copy(FullPathUnicode,us);
    }else{
    Unicode_Copy(FullPathUnicode,pBasePathUnicode);
    Unicode_Add(FullPathUnicode,us);
  }
  
  Unicode_Add(FullPathUnicode,pFilenameUnicode);
  
  return(FullPathUnicode);
}

// -----------------------------------------------------------------------

