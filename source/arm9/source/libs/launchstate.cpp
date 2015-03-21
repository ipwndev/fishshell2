
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"
#include "memtool.h"
#include "fat2.h"
#include "lang.h"
#include "dll.h"

#include "launchstate.h"

#include "shell.h"
#include "../../ipc6.h"

#define CurrentLaunchStateVersion (0x3756534c)

DATA_IN_AfterSystem TLaunchState *pLaunchState=NULL;

static void DataInit(void)
{
  MemSet32CPU(0,pLaunchState,sizeof(TLaunchState));
 
  TLaunchState *pstate=pLaunchState;
  
  pstate->Version=CurrentLaunchStateVersion;
  
  pstate->LastTab=ELST_NDS;
  
  for(u32 tabsidx=0;tabsidx<LaunchState_TabsCount;tabsidx++){
    TLaunchState_Tab *ptab=&pstate->Tabs[tabsidx];
    ptab->FilesCount=0;
    for(u32 fileidx=0;fileidx<LaunchState_Tab_FilesCountMax;fileidx++){
      ptab->FullPathUnicode[fileidx][0]=(UnicodeChar)0;
    }
  }
  
  pstate->Reserved0=(u32)-1;
  pstate->Reserved1=(u32)-1;
  pstate->Reserved2=(u32)-1;
  pstate->Reserved3=(u32)-1;
}

void LaunchState_Clear(void)
{
  DataInit();
}

bool LaunchState_isOpened(void)
{
  if(pLaunchState==NULL){
    return(false);
    }else{
    return(true);
  }
}

// ----------------------------------------------------------------------

static bool LaunchState_TabFile_isStoredFile(TLaunchState_Tab *pTab,const UnicodeChar *pFullPathUnicode)
{
  for(u32 idx=0;idx<pTab->FilesCount;idx++){
    if(Unicode_isEqual_NoCaseSensitive(pFullPathUnicode,pTab->FullPathUnicode[idx])==true) return(true);
  }
  return(false);
}

static void LaunchState_TabFile_AddLast(TLaunchState_Tab *pTab,const UnicodeChar *pPathUnicode,const UnicodeChar *pFilenameUnicode)
{
  if(LaunchState_Tab_FilesCountMax<=pTab->FilesCount) return;
  
  const UnicodeChar *pFullPathUnicode=FullPathUnicodeFromSplitItem(pPathUnicode,pFilenameUnicode);
  
  if(LaunchState_TabFile_isStoredFile(pTab,pFullPathUnicode)==true) return;
  
  Unicode_Copy(pTab->FullPathUnicode[pTab->FilesCount],pFullPathUnicode);
  pTab->FilesCount++;
}

static void LaunchState_Open_ins_AddLaunchFiles(TLaunchState_Tab *pTab)
{
  if(LaunchState_Tab_FilesCountMax<=pTab->FilesCount) return;
  
  UnicodeChar LaunchPathUnicode[MaxFilenameLength];
  StrConvert_Ank2Unicode(DefaultDataPath "/launch",LaunchPathUnicode);
  
  const char *pLaunchPathAlias=ConvertFull_Unicode2Alias(LaunchPathUnicode,NULL);
  
  if(FAT2_chdir_Alias(pLaunchPathAlias)==false) StopFatalError(13301,"Can not found launch folder.\n");
  
  const char *pafn;
  u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
  
  while(FAT_FileType!=FT_NONE){
    const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
    if(pufn==NULL) StopFatalError(13302,"Can not read unicode filename.\n");
    switch(FAT_FileType){
      case FT_NONE: break;
      case FT_DIR: break;
      case FT_FILE: {
        u32 Ext32=0;
        {
          const char *ptmp=pafn;
          while(*ptmp!=0){
            u32 ch=*ptmp++;
            if(ch==(u32)'.'){
              Ext32=0;
              }else{
              if((0x61<=ch)&&(ch<=0x7a)) ch-=0x20;
              Ext32=(Ext32<<8)|ch;
            }
          }
        }
        
        if(MakeExt32(0,'N','D','S')==Ext32){
          _consolePrintf("Add launch file. [%s]\n",pafn);
          LaunchState_TabFile_AddLast(pTab,LaunchPathUnicode,pufn);
        }
      } break;
    }
    
    FAT_FileType=FAT2_FindNextFile(&pafn);
  }
}

static void LaunchState_DeleteIndex(TLaunchState_Tab *pTab,u32 delidx)
{
  for(s32 idx=delidx+1;idx<pTab->FilesCount;idx++){
    Unicode_Copy(pTab->FullPathUnicode[idx-1],pTab->FullPathUnicode[idx]);
    _consolePrintf("Launch edit: copy %d from %d.\n",idx-1,idx);
  }
  pTab->FilesCount--;
  _consolePrintf("Launch edit: FilesCount set to %d.\n",pTab->FilesCount);
}

static void LaunchState_AddTop(TLaunchState_Tab *pTab,const UnicodeChar *pFullPathUnicode)
{
  if(pTab->FilesCount==LaunchState_Tab_FilesCountMax) StopFatalError(13303,"Launch add error. item full.\n");
  
  for(s32 idx=pTab->FilesCount-1;idx>=0;idx--){
    Unicode_Copy(pTab->FullPathUnicode[idx+1],pTab->FullPathUnicode[idx]);
    _consolePrintf("Launch edit: copy %d from %d.\n",idx+1,idx);
  }
  pTab->FilesCount++;
  _consolePrintf("Launch edit: FilesCount set to %d.\n",pTab->FilesCount);
  
  _consolePrint("Launch edit: New item set to 0.\n");
  Unicode_Copy(pTab->FullPathUnicode[0],pFullPathUnicode);
}

void LaunchState_Add(ELaunchState_Tabs Tabs,const UnicodeChar *pFullPathUnicode)
{
  _consoleLogPause();
  
  TLaunchState_Tab *pTab=&pLaunchState->Tabs[Tabs];
  
  if(pTab->FilesCount!=0){
    for(s32 idx=pTab->FilesCount-1;idx>=0;idx--){
      if(Unicode_isEqual_NoCaseSensitive(pFullPathUnicode,pTab->FullPathUnicode[idx])==true){
        _consolePrintf("Launch edit: Delete equal item %d.\n",idx);
        LaunchState_DeleteIndex(pTab,idx);
      }
    }
  }
  
  if(pTab->FilesCount==LaunchState_Tab_FilesCountMax){
    _consolePrint("Launch edit: Delete last item.\n");
    LaunchState_DeleteIndex(pTab,pTab->FilesCount-1);
  }
  
  _consolePrint("Launch edit: Add to top.\n");
  LaunchState_AddTop(pTab,pFullPathUnicode);
  
  _consolePrint("Launch edit: end.\n");
  _consoleLogResume();
}

static void LaunchState_DeleteNotExistsFile(TLaunchState_Tab *pTab)
{
  for(s32 idx=pTab->FilesCount-1;idx>=0;idx--){
    if(pTab->FullPathUnicode[idx][0]==0){
      LaunchState_DeleteIndex(pTab,idx);
      }else{
      if(FullPath_FileExistsUnicode(pTab->FullPathUnicode[idx])==false){
        _consolePrintf("Launch edit: Delete not exists item %d.\n",idx);
        LaunchState_DeleteIndex(pTab,idx);
      }
    }
  }
}

// ----------------------------------------------------------------------

void LaunchState_Open(void)
{
  pLaunchState=(TLaunchState*)safemalloc_chkmem(&MM_Process,sizeof(TLaunchState));
  
  DataInit();
  
  _consolePrintf("Load launch data '%s'\n",LaunchFilename);
  
  FAT_FILE *pf=Shell_FAT_fopen_Internal(LaunchFilename);
  if(pf==NULL){
    _consolePrintf("not found '%s'. load default.\n",LaunchFilename);
    }else{
    u32 filesize=FAT2_GetFileSize(pf);
    u32 LaunchStateSize=sizeof(TLaunchState);
    
    _consolePrint("launch data load from file.\n");
    if(FAT2_fread(pLaunchState,1,LaunchStateSize,pf)!=LaunchStateSize){
      _consolePrint("This file is size too short. load default.\n");
      DataInit();
      }else{
      if(pLaunchState->Version!=CurrentLaunchStateVersion){
        _consolePrint("This file is old version launch data. load default.\n");
        DataInit();
      }
    }
    
    FAT2_fclose(pf);
  }
  
  for(u32 idx=0;idx<LaunchState_TabsCount;idx++){
    TLaunchState_Tab *pTab=&pLaunchState->Tabs[idx];
    LaunchState_DeleteNotExistsFile(pTab);
  }
  
  LaunchState_Open_ins_AddLaunchFiles(&pLaunchState->Tabs[ELST_Launch]);
}

void LaunchState_Close(void)
{
  _consolePrint("Close launch data.\n");
  
  if(pLaunchState!=NULL){
    safefree(&MM_Process,pLaunchState); pLaunchState=NULL;
  }
}

static void LaunchState_Save_inc_FillFutter(UnicodeChar *pustr)
{
  u32 idx=0;
  
  for(;idx<MaxFilenameLength;idx++){
    if(pustr[idx]==0) break;
  }
  for(;idx<MaxFilenameLength;idx++){
    pustr[idx]=0;
  }
}

void LaunchState_Save(void)
{
  REG_IME=0;
  
  {
    TLaunchState *pstate=pLaunchState;
    for(u32 tabsidx=0;tabsidx<LaunchState_TabsCount;tabsidx++){
      TLaunchState_Tab *ptab=&pstate->Tabs[tabsidx];
      for(u32 fileidx=0;fileidx<LaunchState_Tab_FilesCountMax;fileidx++){
        LaunchState_Save_inc_FillFutter(ptab->FullPathUnicode[fileidx]);
      }
    }
  }
  
  _consolePrintf("Save launch data '%s'\n",LaunchFilename);

  u32 LaunchStateSize=sizeof(TLaunchState);
  FAT_FILE *pf=Shell_FAT_fopenwrite_Internal(LaunchFilename);
  
  if(pf==NULL){
    StopFatalError(13305,"launch data file open error. '%s'\n",LaunchFilename);
    }else{
    if(FAT2_fwrite(pLaunchState,1,LaunchStateSize,pf)!=LaunchStateSize){
        StopFatalError(13306,"launch data file write error. '%s'\n",LaunchFilename);
    }
    FAT2_fclose(pf);
  }
  
  _consolePrintf("Saved launch data. %dbyte.\n",LaunchStateSize);
  
  REG_IME=1;
}

