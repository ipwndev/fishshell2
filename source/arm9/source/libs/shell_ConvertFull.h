
const char* ConvertFull_Unicode2Alias(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pFilenameUnicode)
{
  if((pBasePathUnicode==NULL)||(pBasePathUnicode[0]==0)){
    static UnicodeChar su[2]={(UnicodeChar)'/',0};
    pBasePathUnicode=su;
  }
  
//  _consolePrintf("Start ConvertFull_Unicode2Alias('%s',",StrConvert_Unicode2Ank_Test(pBasePathUnicode));
//  _consolePrintf("'%s');\n",StrConvert_Unicode2Ank_Test(pFilenameUnicode));
  
  static char pfn[MaxFilenameLength+1]={0,};
  
  if(FAT2_chdir_Alias("/")==false) StopFatalError(13604,"Can not chdir to root.\n");
  
  if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  
  pfn[0]='/';
  pfn[1]=0;
  
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
      _consolePrintf("ConvertFull_Unicode2Alias: Not find Unicode path item. [%s]\n",StrConvert_Unicode2Ank_Test(pPathUnicode));
      return(NULL);
      }else{
      strcat(pfn,pafn);
      strcat(pfn,"/");
      
      if(FAT2_chdir_Alias(pafn)==false) StopFatalError(13605,"Can not chdir to %s.\n",pafn);
    }
    
    if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  }
  
  if(Unicode_isEmpty(pFilenameUnicode)==false){
    bool finded=false;
    const char *pafn=NULL;
    
    {
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
      _consolePrintf("ConvertFull_Unicode2Alias: Not find Unicode file item. [%s]\n",StrConvert_Unicode2Ank_Test(pFilenameUnicode));
      return(NULL);
      }else{
      strcat(pfn,pafn);
    }
  }
  
//  _consolePrintf("End ConvertFull_Unicode2Alias=%s\n",pfn);
  
  return(pfn);
}

const char* ConvertPath_Unicode2Alias_CheckExists(const UnicodeChar *pBasePathUnicode)
{
  if((pBasePathUnicode==NULL)||(pBasePathUnicode[0]==0)){
    static UnicodeChar su[2]={(UnicodeChar)'/',0};
    pBasePathUnicode=su;
  }
  
  _consolePrintf("Start ConvertPath_Unicode2Alias_CheckExists=%s\n",StrConvert_Unicode2Ank_Test(pBasePathUnicode));
  
  static char pfn[MaxFilenameLength+1]={0,};
  
  if(FAT2_chdir_Alias("/")==false) return(NULL);
  
  if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  
  pfn[0]='/';
  pfn[1]=0;
  
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
      _consolePrintf("ConvertFull_Unicode2Alias: Not find Unicode path item. [%s]\n",StrConvert_Unicode2Ank_Test(pPathUnicode));
      return(NULL);
      }else{
      strcat(pfn,pafn);
      strcat(pfn,"/");
      
      if(FAT2_chdir_Alias(pafn)==false) return(NULL);
    }
    
    if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  }
  
  _consolePrintf("End ConvertPath_Unicode2Alias_CheckExists=%s\n",pfn);
  
  return(pfn);
}
const char* ConvertFullPath_Unicode2Alias(const UnicodeChar *pFullPathUnicode)
{
  if(Unicode_isEmpty(pFullPathUnicode)==true) return("");

  u32 splitpos=(u32)-1;
  
  u32 findidx=0;
  while(pFullPathUnicode[findidx]!=0){
    if(pFullPathUnicode[findidx]==(UnicodeChar)'/') splitpos=findidx;
    findidx++;
  }
  
  if(splitpos==(u32)-1){
    _consolePrintf("ConvertFullPath_Unicode2Alias: This is not full path.\n");
    return(NULL);
  }
  
  UnicodeChar PathNameUnicde[MaxFilenameLength+1],FileNameUnicde[MaxFilenameLength+1]; // スタック足りるかな…
  
  if(splitpos!=0){
    for(u32 idx=0;idx<splitpos;idx++){
      PathNameUnicde[idx]=pFullPathUnicode[idx];
    }
    PathNameUnicde[splitpos]=0;
    }else{ // store in root
    PathNameUnicde[0]=(UnicodeChar)'/';
    PathNameUnicde[1]=0;
  }
  
  pFullPathUnicode+=splitpos+1;
  
  u32 idx=0;
  while(pFullPathUnicode[idx]!=0){
    FileNameUnicde[idx]=pFullPathUnicode[idx];
    idx++;
  }
  FileNameUnicde[idx]=0;
  
//  _consolePrintf("0 [%s]\n",StrConvert_Unicode2Ank_Test(PathNameUnicde));
//  _consolePrintf("  [%s]\n",StrConvert_Unicode2Ank_Test(FileNameUnicde));

  const char *pres=ConvertFull_Unicode2Alias(PathNameUnicde,FileNameUnicde);
  if(pres==NULL) StopFatalError(13621,"ConvertFullPath_Unicode2Alias: Not found path.\n");
  return(pres);
}

const char* ConvertFullPath_Ansi2Alias(const char *pFullPathAnsi)
{
  UnicodeChar FullPathUnicode[MaxFilenameLength+1]; // スタック足りるかな…
  StrConvert_Ank2Unicode(pFullPathAnsi,FullPathUnicode);
  return(ConvertFullPath_Unicode2Alias(FullPathUnicode));
}

// -----------------------------------------------------------------------

bool forPLS_ConvertFull_Unicode2Alias(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pFilenameUnicode,char *pPathAlias,char *pFilenameAlias)
{
  if((pBasePathUnicode==NULL)||(pBasePathUnicode[0]==0)){
    static UnicodeChar su[2]={(UnicodeChar)'/',0};
    pBasePathUnicode=su;
  }
  
  pPathAlias[0]=0;
  pFilenameAlias[0]=0;
  
  if(FAT2_chdir_Alias("/")==false) StopFatalError(13614,"Can not chdir to root.\n");
  
  if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  
  pPathAlias[0]='/';
  pPathAlias[1]=0;
  
  while(*pBasePathUnicode!=0){
    UnicodeChar pPathUnicode[MaxFilenameLength];
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
              _consolePrintf("forPLS_ConvertFull_Unicode2Alias: Unicode filename read error.\n Alias='%s'\n",pafn);
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
      _consolePrintf("forPLS_ConvertFull_Unicode2Alias: Not find Unicode path item. [%s]\n",StrConvert_Unicode2Ank_Test(pPathUnicode));
      return(false);
      }else{
      strcat(pPathAlias,pafn);
      strcat(pPathAlias,"/");
      
      if(FAT2_chdir_Alias(pafn)==false) StopFatalError(13615,"Can not chdir to %s.\n",pafn);
    }
    
    if(*pBasePathUnicode==(UnicodeChar)'/') pBasePathUnicode++;
  }
  
  if(Unicode_isEmpty(pFilenameUnicode)==false){
    bool finded=false;
    const char *pafn=NULL;
    
    {
      u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
      
      while((FAT_FileType!=FT_NONE)&&(finded==false)){
        switch(FAT_FileType){
          case FT_NONE: break;
          case FT_DIR: break;
          case FT_FILE: {
            const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
            if(pufn==NULL){
              _consolePrintf("forPLS_ConvertFull_Unicode2Alias: Unicode filename read error.\n Alias='%s'\n",pafn);
              }else{
              if(Unicode_isEqual_NoCaseSensitive(pufn,pFilenameUnicode)==true) finded=true;
            }
          } break;
        }
        
        if(finded==false) FAT_FileType=FAT2_FindNextFile(&pafn);
      }
    }
    
    if(finded==false){
      _consolePrintf("forPLS_ConvertFull_Unicode2Alias: Not find Unicode file item. [%s]\n",StrConvert_Unicode2Ank_Test(pFilenameUnicode));
      return(false);
      }else{
      strcpy(pFilenameAlias,pafn);
    }
  }
  
  return(true);
}

// -----------------------------------------------------------------------

