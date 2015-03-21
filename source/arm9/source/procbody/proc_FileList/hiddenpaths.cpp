
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "arm9tcm.h"

#include "fat2.h"
#include "memtool.h"
#include "shell.h"
#include "strtool.h"
#include "unicode.h"

#include "hiddenpaths.h"

#define PathsMaxCount (64)
DATA_IN_IWRAM_FileList static UnicodeChar *pPaths[PathsMaxCount];
DATA_IN_IWRAM_FileList static u32 PathsCount=0;

void HiddenPaths_Init(void)
{
  char *pinibuf=NULL;
  u32 inibufsize=0;
  u32 inibufpos=0;
  
  {
    FAT_FILE *pf=Shell_FAT_fopen_Root(HiddenPathsFilename);
    if(pf==NULL) StopFatalError(10501,"HiddenPaths_Init: File not found. [%s]\n",HiddenPathsFilename);
    inibufsize=FAT2_GetFileSize(pf);
    pinibuf=(char*)safemalloc_chkmem(&MM_Temp,inibufsize+32);
    FAT2_fread(pinibuf,1,inibufsize,pf);
    FAT2_fclose(pf);
    for(u32 idx=0;idx<32;idx++){
      pinibuf[inibufsize+idx]=0;
    }
  }
  
  PathsCount=0;
  for(u32 idx=0;idx<PathsMaxCount;idx++){
    pPaths[idx]=NULL;
  }
  
  char readstr[512];
  u32 readstrpos=0;
  
  while(inibufpos<inibufsize){
    char ch=pinibuf[inibufpos++];
    if(0x20<=ch){
      readstr[readstrpos++]=ch;
      }else{
      if(readstrpos!=0){
        readstr[readstrpos]=0;
        if((readstr[0]=='\\')||(readstr[0]=='/')){
          for(u32 idx=0;idx<readstrpos;idx++){
            if(readstr[idx]=='\\') readstr[idx]='/';
          }
          if(PathsCount<PathsMaxCount){
            UnicodeChar pathw[256];
            StrConvert_UTF82Unicode(readstr,pathw);
            if(pathw[Unicode_GetLength(pathw)-1]=='/') pathw[Unicode_GetLength(pathw)-1]=0; // ÅŒã‚ª'/'‚¾‚Á‚½‚çœ‹Ž‚·‚éB
//            _consolePrintf("Hidden path(%d/%d): %s.\n",PathsCount,PathsMaxCount,StrConvert_Unicode2Ank_Test(pathw));
            pPaths[PathsCount++]=Unicode_AllocateCopy(&MM_Process,pathw);
          }
        }
        readstrpos=0;
      }
    }
  }
  
  if(pinibuf!=NULL){
    safefree(&MM_Temp,pinibuf); pinibuf=NULL;
  }
}

void HiddenPaths_Free(void)
{
  for(u32 idx=0;idx<PathsCount;idx++){
    if(pPaths[idx]!=NULL){
      safefree(&MM_Process,pPaths[idx]); pPaths[idx]=NULL;
    }
  }
  
  PathsCount=0;
}

bool HiddenPaths_isHidden(const UnicodeChar *pTargetPath)
{
  for(u32 idx=0;idx<PathsCount;idx++){
    if(Unicode_isEqual_NoCaseSensitive(pTargetPath,pPaths[idx])==true) return(true);
  }
  return(false);
}
