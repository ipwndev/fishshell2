
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

#include "VideoResume.h"

#define CurrentVideoResumeVersion (0x31444956)

#define VideoResume_FilesCountMax (16)

typedef struct {
    u32 FramePos;
    UnicodeChar FullPathUnicode[MaxFilenameLength];
} TVideoResumeFile;

typedef struct {
  u32 Version;
  u32 StoredID;
  TVideoResumeFile VideoFile[VideoResume_FilesCountMax];
} TVideoResume;

extern TVideoResume *pVideoResume;

DATA_IN_AfterSystem TVideoResume *pVideoResume=NULL;

DATA_IN_AfterSystem static u32 CurrentStoredID=(u32)-1;

static void DataInit(void)
{
    MemSet32CPU(0,pVideoResume,sizeof(TVideoResume));
      
    pVideoResume->Version=CurrentVideoResumeVersion;
    pVideoResume->StoredID=VideoResume_FilesCountMax-1;
}

void VideoResume_Clear(void)
{
  DataInit();
}

bool VideoResume_isOpened(void)
{
  if(pVideoResume==NULL){
    return(false);
    }else{
    return(true);
  }
}

static bool VideoResume_isStoredFile(const UnicodeChar *pPathUnicode,const UnicodeChar *pFilenameUnicode)
{
    const UnicodeChar *pFullPathUnicode=FullPathUnicodeFromSplitItem(pPathUnicode,pFilenameUnicode);
    for(u32 idx=VideoResume_FilesCountMax-1;idx>=0;idx++){
        if(Unicode_isEqual_NoCaseSensitive(pFullPathUnicode,pVideoResume->VideoFile[idx].FullPathUnicode)==true) {
            CurrentStoredID=idx;
            return(true);
        }
    }
  
    Unicode_Copy(pVideoResume->VideoFile[pVideoResume->StoredID].FullPathUnicode,pFullPathUnicode);
    CurrentStoredID=pVideoResume->StoredID;
    
    pVideoResume->StoredID--;
    
    if(pVideoResume->StoredID<0) pVideoResume->StoredID=VideoResume_FilesCountMax-1;
    return(false);
}

void VideoResume_Open(void)
{
    pVideoResume=(TVideoResume*)safemalloc_chkmem(&MM_Process,sizeof(TVideoResume));
      
    DataInit();
      
    _consolePrintf("Resume video file load.\n");
      
    FAT_FILE *pf=Shell_FAT_fopen_Internal(LaunchFilename);
    if(pf==NULL){
        _consolePrintf("not found '%s'. load default.\n",ResumeVideoFilename);
    }else{
        u32 filesize=FAT2_GetFileSize(pf);
        u32 VideoResumeSize=sizeof(TVideoResume);
        
        if(FAT2_fread(pVideoResume,1,VideoResumeSize,pf)!=VideoResumeSize){
            _consolePrint("This file is size too short. load default.\n");
            DataInit();
        }else{
            if(pVideoResume->Version!=CurrentVideoResumeVersion){
                _consolePrint("This file is old version video resume data. load default.\n");
                DataInit();
            }
        }
        
        FAT2_fclose(pf);
    }
}

void VideoResume_Close(void)
{
  _consolePrint("Close video resume data.\n");
  
  if(pVideoResume!=NULL){
    safefree(&MM_Process,pVideoResume); pVideoResume=NULL;
  }
}

static void VideoResume_Save_inc_FillFutter(UnicodeChar *pustr)
{
  u32 idx=0;
  
  for(;idx<MaxFilenameLength;idx++){
    if(pustr[idx]==0) break;
  }
  for(;idx<MaxFilenameLength;idx++){
    pustr[idx]=0;
  }
}

void VideoResume_Save(void)
{
    REG_IME=0;
      
    {
        TVideoResume *presume=pVideoResume;
        for(u32 idx=0;idx<VideoResume_FilesCountMax;idx++){
            TVideoResumeFile *pfile=&presume->VideoFile[idx];
            VideoResume_Save_inc_FillFutter(pfile->FullPathUnicode);
        }
    }
      
    _consolePrintf("Save video resume data '%s'\n",ResumeVideoFilename);

    u32 VideoResumeSize=sizeof(TVideoResume);
    FAT_FILE *pf=Shell_FAT_fopenwrite_Internal(ResumeVideoFilename);
      
    if(pf==NULL){
        StopFatalError(13305,"video resume data file open error. '%s'\n",ResumeVideoFilename);
    }else{
        if(FAT2_fwrite(pVideoResume,1,VideoResumeSize,pf)!=VideoResumeSize){
            StopFatalError(13306,"video resume data file write error. '%s'\n",ResumeVideoFilename);
        }
        FAT2_fclose(pf);
    }
       
    _consolePrintf("Saved video resume data. %dbyte.\n",VideoResumeSize);
      
    REG_IME=1;
}


