
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "fat2.h"
#include "memtool.h"
#include "shell.h"
#include "splash.h"
#include "procstate.h"
#include "strtool.h"
#include "unicode.h"
#include "euc2unicode.h"
#include "playlist.h"
#include "glib.h"
#include "disc_io.h"
#include "strpcm.h"

#include "arm9tcm.h"

#include "dll.h"
#include "dllsound.h"

#include "playlist_TextPool.h"

extern DISC_INTERFACE* active_interface;

DATA_IN_AfterSystem bool RequestRefreshPlayCursorIndex;

typedef struct {
  char *pPathAlias;
  UnicodeChar *pPathUnicode;
  char *pFilenameAlias;
  UnicodeChar *pFilenameUnicode;
} TPlayList;

DATA_IN_AfterSystem static TPlayList *pPlayList=NULL;
DATA_IN_AfterSystem static u32 PlayListCount=0;
DATA_IN_AfterSystem static u32 PlayListIndex=0;

DATA_IN_AfterSystem static int ShuffleCount=-1;
DATA_IN_AfterSystem static int *pShuffleList=NULL;

DATA_IN_AfterSystem static u32 ResumeSaveTimeVSync;

#define FileHeader_ID (0x31534c50)

typedef struct {
  u32 ID;
  u32 Position;
  bool Pause;
  volatile bool Error;
  UnicodeChar FullPathFilenameW[256-2-2-1];
} TFileHeader;

DATA_IN_AfterSystem static u32 FileHeaderSectorNum;
DATA_IN_AfterSystem static TFileHeader FileHeader;

static void PlayList_ChangeExt2LRC(UnicodeChar *pfnu)
{
  u32 pos=0;
  
  u32 idx=0;
  while(pfnu[idx]!=0){
    if(pfnu[idx]=='.') pos=idx+1;
    idx++;
  }
  
  if(pos==0){
    _consolePrint("PlayList_ChangeExt2LRC: Not found extention.\n");
    return;
  }
  
  pfnu[pos+0]='l';
  pfnu[pos+1]='r';
  pfnu[pos+2]='c';
  pfnu[pos+3]=0;
}

DATA_IN_IWRAM_MainPass void PlayList_Init(void)
{
  if(sizeof(TFileHeader)!=512){
    _consolePrint("TFileHeader size error.\n");
    while(1);
  }
  
  FileHeaderSectorNum=0;
  FileHeader.ID=0;
  FileHeader.Position=0;
  FileHeader.Pause=false;
  FileHeader.Error=false;
  FileHeader.FullPathFilenameW[0]=0;
  
  pTextPoolStart=NULL;
  
  pPlayList=NULL;
  PlayListCount=0;
  PlayListIndex=0;

  ResumeSaveTimeVSync=0;
}

static void PlayList_PlayCurrentIndex(void)
{
  if(PlayListCount==0) return;
  
  FileHeader.Error=true;
  if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
  
  if(VerboseDebugLog==true) _consolePrint("PlayList_PlayCurrentIndex: DLLSound_Close.\n");
  DLLSound_Close(true);
  
  if(VerboseDebugLog==true) _consolePrint("PlayList_PlayCurrentIndex: PlugLRC_Free.\n");
  PlugLRC_Free();
    
  TPlayList *ppl=&pPlayList[PlayListIndex];
  
  if(VerboseDebugLog==true) _consolePrint("PlayList_PlayCurrentIndex: chdir.\n");
  if(FAT2_chdir_Alias(ppl->pPathAlias)==false) StopFatalError(10802,"Can not change current path. [%s]\n",ppl->pPathAlias);
  
  {
     UnicodeChar fnu[MaxFilenameLength];
     Unicode_Copy(fnu,ppl->pFilenameUnicode);
     PlayList_ChangeExt2LRC(fnu);
     
     const char *pLRCAlias=ConvertFull_Unicode2Alias(ppl->pPathUnicode,fnu);
     
     if(VerboseDebugLog==true) _consolePrint("PlayList_PlayCurrentIndex: PlugLRC_Start...\n");
     if(pLRCAlias!=NULL){
         if(PlugLRC_Start(pLRCAlias)==true){
             if(VerboseDebugLog==true) _consolePrintf("LRC file opened:[%s]\n",pLRCAlias);
         }
     }
  }
  
  if(VerboseDebugLog==true) _consolePrint("PlayList_PlayCurrentIndex: DLLSound_Open.\n");
  DLLSound_Open(ppl->pFilenameAlias);
  
  if(FileHeaderSectorNum==0) StopFatalError(0,"Play list resume sector is NULL.\n");
  
  FileHeader.Position=0;
  FileHeader.Error=false;
  Unicode_Copy(FileHeader.FullPathFilenameW,FullPathUnicodeFromSplitItem(ppl->pPathUnicode,ppl->pFilenameUnicode));
  if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
  
//  _consolePrint("PlayList_PlayCurrentIndex: Set resume save timer.\n");
  ResumeSaveTimeVSync=1;
  
  RequestRefreshPlayCursorIndex=true;
}

void PlayList_Stop(bool WaitForEmpty)
{
    if(VerboseDebugLog==true) _consolePrintf("PlayList_Stop: Stop. WaitForEmpty=%d.\n",WaitForEmpty);
  if(PlayListCount==0){
    DLLSound_Close(false);
    return;
  }
  
  if(VerboseDebugLog==true) _consolePrint("PlayList_Stop: PlugLRC_Free.\n");
  PlugLRC_Free();
      
  PlayList_UpdateResume((u32)-1);
  
  FileHeader.Error=true;
  if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
  
  if(WaitForEmpty==true) DLLSound_WaitForStreamPCM();
  
  if(VerboseDebugLog==true) _consolePrint("PlayList_Stop: DLLSound_Close.\n");
  DLLSound_Close(WaitForEmpty); // 後で確認
  strpcmRequestStop=false;
  
  FileHeader.Error=false;
  if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
  
  ResumeSaveTimeVSync=0;
  
  RequestRefreshPlayCursorIndex=true;
}

void PlayList_Free(void)
{
  FileHeaderSectorNum=0;
  FileHeader.ID=0;
  
  PlayList_Stop(false);
  
  if(VerboseDebugLog==true) _consolePrint("PlayList_Free: Free play list.\n");
  if(pPlayList!=NULL){
    for(u32 idx=0;idx<PlayListCount;idx++){
      TPlayList *ppl=&pPlayList[idx];
      ppl->pPathAlias=NULL;
      ppl->pPathUnicode=NULL;
      ppl->pFilenameAlias=NULL;
      ppl->pFilenameUnicode=NULL;
    }
    PlayListCount=0;
    safefree(&MM_PlayList,pPlayList); pPlayList=NULL;
  }
  
  if(pShuffleList!=NULL){
	  safefree(&MM_PlayList,pShuffleList); pShuffleList=NULL;
  }
  
  PlayListCount=0;
  PlayListIndex=0;
  ShuffleCount=-1;
  
  FreeTextPool();
  
  MM_Compact();
  MM_CheckMemoryLeak(&MM_PlayList);
}

void PlayList_UpdateResume(u32 VsyncCount)
{
  if(ResumeSaveTimeVSync==0) return;
  
  if(VsyncCount!=(u32)-1){
    ResumeSaveTimeVSync+=VsyncCount;
    if(ResumeSaveTimeVSync<(60*1)) return;
  }
  
  ResumeSaveTimeVSync=1;
  
  u32 pos=DLLSound_GetPosOffset();
  if(pos!=FileHeader.Position){
    FileHeader.Position=pos;
    if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
  }
}

static void Shuffle_Refresh(int TopIndex)
{
	if(ShuffleCount==PlayListCount) return;
	
  ShuffleCount=PlayListCount;
  
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

void PlayList_Repeat(void)
{
  if(PlayListCount==0) return;
  
  PlayList_Stop(true);
  
  PlayList_PlayCurrentIndex();
}

void PlayList_Prev(void)
{
  if(PlayListCount==0) return;
  
  PlayList_Stop(false);
  
  if(PlayListIndex==0){
    PlayListIndex=PlayListCount-1;
    }else{
    PlayListIndex--;
  }
  
  PlayList_PlayCurrentIndex();
}

void PlayList_PrevShuffle(void)
{
  if(PlayListCount==0) return;
  
  PlayList_Stop(true);
  
  u32 LastIndex=PlayListIndex;
  if(ShuffleCount!=PlayListCount) Shuffle_Refresh(LastIndex);
  
  if(PlayListCount!=1){
	  for(int idx=0;idx<ShuffleCount;idx++){
		  if(pShuffleList[idx]==LastIndex){
			  if(idx==0){
				  //ShuffleCount=-1;
				  PlayListIndex=pShuffleList[ShuffleCount-1];
			  }else{
				  PlayListIndex=pShuffleList[idx-1];
			  }
			  break;
		  }
	  }
  }
  
  PlayList_PlayCurrentIndex();
}

void PlayList_Next(void)
{
  if(PlayListCount==0) return;
  
  PlayList_Stop(true);
  
  PlayListIndex++;
  if(PlayListIndex==PlayListCount) PlayListIndex=0;
  
  PlayList_PlayCurrentIndex();
}

void PlayList_NextShuffle(void)
{
  if(PlayListCount==0) return;
  
  PlayList_Stop(true);
  
  u32 LastIndex=PlayListIndex;
  if(ShuffleCount!=PlayListCount) Shuffle_Refresh(LastIndex);
  
  if(PlayListCount!=1){
	  for(int idx=0;idx<ShuffleCount;idx++){
		  if(pShuffleList[idx]==LastIndex){
			  if(idx==(ShuffleCount-1)){
				  //ShuffleCount=-1;
				  PlayListIndex=pShuffleList[0];
			  }else{
				  PlayListIndex=pShuffleList[idx+1];
			  }
			  break;
		  }
	  }
  }
  
  PlayList_PlayCurrentIndex();
}

bool PlayList_isOpened(void)
{
  if(PlayListCount==0) return(false);
  
  return(DLLSound_isOpened());
}

u32 PlayList_GetFilesCount(void)
{
  return(PlayListCount);
}

u32 PlayList_GetCurrentIndex(void)
{
	return(PlayListIndex);
}

bool PlayList_GetListEndFlag(void)
{
	if(ProcState.FileList.PlayMode==EPSFLPM_Shuffle){
		for(int idx=0;idx<ShuffleCount;idx++){
			if(pShuffleList[idx]==PlayListIndex){
				if(idx==(ShuffleCount-1)) return(true);
			}
		}
		return(false);
	}
	
	if(PlayListIndex==PlayListCount) return(true);
	return(false);
}

const UnicodeChar* PlayList_GetCurrentFilename(void)
{
  return(pPlayList[PlayListIndex].pFilenameUnicode);
}

const UnicodeChar* PlayList_GetCurrentPath(void)
{
  return(pPlayList[PlayListIndex].pPathUnicode);
}

#include "playlist_ConvertM3U.h"
#include "playlist_MakeFolder.h"

void PlayList_MakeBlank(void)
{
  FAT_FILE *pfdst=Shell_FAT_fopenwrite_Internal(ResumePlayListFilename);
  if(pfdst==NULL) StopFatalError(10804,"Resume play list file write error.\n");
  
  FileHeader.ID=0;
  FAT2_fwrite(&FileHeader,1,sizeof(FileHeader),pfdst);
  
  FAT2_fclose(pfdst);
}

static __attribute__ ((noinline)) bool PlayList_Start_ins_LoadPlayList(bool ShowPrg)
{
  u8 *pplsbuf=NULL;
  u32 plsbufsize=0;
  u32 plsbufpos=0;
  
  {
    FAT_FILE *pf=Shell_FAT_fopen_Internal(ResumePlayListFilename);
    if(pf==NULL){
      _consolePrint("PlayList_StartPlayList: Resume play list file not found.\n");
      return(false);
    }
    
    FileHeaderSectorNum=FAT2_ClustToSect(pf->firstCluster);
    FAT2_fread_fast(&FileHeader,1,sizeof(TFileHeader),pf);
    
    if(FileHeader.ID==0){
      FAT2_fclose(pf);
      return(false);
    }
    if(FileHeader.ID!=FileHeader_ID){
      _consolePrintf("Illigal File header ID. 0x%08x!=0x%08x (step.1)\n",FileHeader_ID,FileHeader.ID);
      FAT2_fclose(pf);
      return(false);
    }
    
    plsbufsize=FAT2_GetFileSize(pf)-4-sizeof(TFileHeader);
    if(plsbufsize==0){
      _consolePrint("PlayList_StartPlayList: is blank play list.\n");
      FAT2_fclose(pf);
      return(false);
    }
    pplsbuf=(u8*)safemalloc_chkmem(&MM_Temp,plsbufsize);
    
    FAT2_fread_fast(pplsbuf,1,plsbufsize,pf);
    FAT2_fread_fast(&PlayListCount,4,1,pf);
    _consolePrintf("PlayListCount set to %d.\n",PlayListCount);
    
    FAT2_fclose(pf);
  }
  
  if(PlayListCount==0){
    _consolePrint("PlayList_StartPlayList: is blank play list.\n");
    if(pplsbuf!=NULL){
      safefree(&MM_Temp,pplsbuf); pplsbuf=NULL;
    }
    return(false);
  }

  pShuffleList=(int*)safemalloc_chkmem(&MM_PlayList,PlayListCount*sizeof(int));
  pPlayList=(TPlayList*)safemalloc_chkmem(&MM_PlayList,PlayListCount*sizeof(TPlayList));
  
  for(u32 idx=0;idx<PlayListCount;idx++){
    TPlayList *ppl=&pPlayList[idx];
    ppl->pPathAlias=NULL;
    ppl->pPathUnicode=NULL;
    ppl->pFilenameAlias=NULL;
    ppl->pFilenameUnicode=NULL;
  }
  
  u32 BackupPlayListCount=PlayListCount;
  
  PlayListCount=0;
  InitTextPool();
  
  char LastPathA[256]={0,};
  char *pTextPool_LastPathA=NULL;
  UnicodeChar LastPathW[256]={0,};
  UnicodeChar *pTextPool_LastPathW=NULL;
  
  if(ShowPrg==true) CallBack_MWin_ProgressShow("",0);
  
  while(plsbufpos<plsbufsize){
    u32 pathlen=pplsbuf[plsbufpos++];
    u32 filenamelen=pplsbuf[plsbufpos++];
    if((pathlen==0)||(filenamelen==0)) break;
    UnicodeChar PathW[256],FilenameW[256];
    MemCopy16CPU(&pplsbuf[plsbufpos],PathW,pathlen*2);
    plsbufpos+=pathlen*2;
    MemCopy16CPU(&pplsbuf[plsbufpos],FilenameW,filenamelen*2);
    plsbufpos+=filenamelen*2;
    
    bool ChangePath=false;
    
    if(Unicode_isEqual(LastPathW,PathW)==false){
      ChangePath=true;
      const char *ppathalias=ConvertFull_Unicode2Alias(PathW,NULL);
      if(ppathalias==NULL){
        LastPathA[0]=0;
        LastPathW[0]=0;
        }else{
        StrCopy(ppathalias,LastPathA);
        Unicode_Copy(LastPathW,PathW);
      }
    }
    if(str_isEmpty(LastPathA)==false){
      const char *pFilenameA=NULL;
      {
        const char *pafn;
        u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
        
        while(FAT_FileType!=FT_NONE){
          switch(FAT_FileType){
            case FT_NONE: break;
            case FT_DIR: break;
            case FT_FILE: {
              const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
              if(pufn==NULL){
                _consolePrintf("forPLS_ConvertFull_Unicode2Alias: Unicode filename read error.\n Alias='%s'\n",pafn);
                }else{
                if(Unicode_isEqual_NoCaseSensitive(pufn,FilenameW)==true) pFilenameA=pafn;
              }
            } break;
          }
          
          if(pFilenameA!=NULL) break;
          FAT_FileType=FAT2_FindNextFile(&pafn);
        }
      }
      
      if(pFilenameA!=NULL){
        TPlayList *ppl=&pPlayList[PlayListCount];
        if(ChangePath==true){
          pTextPool_LastPathA=TextPoolChar_AllocateCopy(LastPathA);
          pTextPool_LastPathW=TextPoolUnicode_AllocateCopy(LastPathW);
        }
        ppl->pPathAlias=pTextPool_LastPathA;
        ppl->pPathUnicode=pTextPool_LastPathW;
        ppl->pFilenameAlias=TextPoolChar_AllocateCopy(pFilenameA);
        ppl->pFilenameUnicode=TextPoolUnicode_AllocateCopy(FilenameW);
        PlayListCount++;
        
        if((PlayListCount&255)==0){
          char msg0[64],msg1[64];
          snprintf(msg0,64,"Check play list... %d%%",(PlayListCount*100)/BackupPlayListCount);
          snprintf(msg1,64,"%d / %d files.",PlayListCount,BackupPlayListCount);
          if(ShowPrg==true) CallBack_MWin_ProgressDraw(msg0,msg1,PlayListCount,BackupPlayListCount);
        }
      }
    }
  }
  
  if(2<=PlayListCount){
	  if(VerboseDebugLog==true) _consolePrint("Sort for filenames.\n");
      for(s32 idx0=0;idx0<PlayListCount-1;idx0++){
        DLLSound_UpdateLoop(true);
        for(s32 idx1=idx0+1;idx1<PlayListCount;idx1++){
            DLLSound_UpdateLoop(true);
          TPlayList *pf0=&pPlayList[idx0];
          TPlayList *pf1=&pPlayList[idx1];
          
          if(isSwapFilenameUnicode(&pf0->pFilenameUnicode[0],&pf1->pFilenameUnicode[0])==true){
        	  TPlayList ftemp=*pf0;
        	  *pf0=*pf1;
        	  *pf1=ftemp;
          }
        }
      }
      
      if(VerboseDebugLog==true) _consolePrint("End of sort.\n");
  }
  
  if(ShowPrg==true) CallBack_MWin_ProgressHide();
  
  if(pplsbuf!=NULL){
    safefree(&MM_Temp,pplsbuf); pplsbuf=NULL;
  }
  
  EndTextPool();
  
  return(true);
}

bool PlayList_DeleteListItem(const UnicodeChar *pFileNameUnicode) {
	if(PlayListCount==0) return(false);
	
	s32 DeleteIndex=-1;
	for(s32 idx=0;idx<PlayListCount-1;idx++){
		TPlayList *pf=&pPlayList[idx];
		
		if(Unicode_isEqual(pFileNameUnicode,pf->pFilenameUnicode)) {
			DeleteIndex=idx;
		}
	}
	//_consolePrintf("DeleteIndex=%d  %d\n",DeleteIndex,PlayListCount);
	if(DeleteIndex!=-1){
		/*if(DeleteIndex<PlayListCount)
		for(s32 idx=DeleteIndex;idx<PlayListCount-4;idx++){
			TPlayList *pf0=&pPlayList[idx];
			TPlayList *pf1=&pPlayList[idx+1];
			*pf0=*pf1;
		}
	
		PlayListCount--;
		
		if(DeleteIndex==PlayListIndex) PlayList_PlayCurrentIndex();*/
			
		return(true);
	}
	return(false);
}

bool PlayList_Start(bool ShowPrg,const UnicodeChar *pFilePathUnicode,const UnicodeChar *pFileNameUnicode)
{
  PlayList_Free();
  
  FileHeaderSectorNum=0;
  FileHeader.ID=0;
  
  if(PlayList_Start_ins_LoadPlayList(ShowPrg)==false){
    FileHeaderSectorNum=0;
    return(false);
  }
  
  if(VerboseDebugLog==true) _consolePrintf("FileHeaderSectorNum=%d.\n",FileHeaderSectorNum);
  MemSet32CPU(0,&FileHeader,sizeof(FileHeader));
  if(FileHeaderSectorNum!=0) active_interface->readSectors(FileHeaderSectorNum,1,&FileHeader);
  
  // 念のため再チェック
  if(FileHeader.ID==0){
    PlayList_Free();
    return(false);
  }
  if(FileHeader.ID!=FileHeader_ID){
    _consolePrintf("Illigal File header ID. 0x%08x!=0x%08x (step.2)\n",FileHeader_ID,FileHeader.ID);
    PlayList_Free();
    return(false);
  }
  
  if(PlayListCount==0){
    PlayList_Free();
    return(false);
  }
  
  if(FileHeader.Error==true){
    _consolePrint("Error detected last play.\n");
    PlayList_Free();
    return(false);
  }

  FileHeader.Error=true;
  if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
  
  PlayListIndex=(u32)-1;
  u32 PlayPosition=0;
  
  if((Unicode_isEmpty(pFilePathUnicode)==true)||(Unicode_isEmpty(pFileNameUnicode)==true)){
    if(Unicode_isEmpty(FileHeader.FullPathFilenameW)==false){
    	DATA_IN_AfterSystem static UnicodeChar PathW[MaxFilenameLength],FilenameW[MaxFilenameLength];
      SplitItemFromFullPathUnicode(FileHeader.FullPathFilenameW,PathW,FilenameW);
      pFilePathUnicode=PathW;
      pFileNameUnicode=FilenameW;
      PlayPosition=FileHeader.Position;
    }
  }
  
  if((Unicode_isEmpty(pFilePathUnicode)==false)&&(Unicode_isEmpty(pFileNameUnicode)==false)){
    for(u32 idx=0;idx<PlayListCount;idx++){
      TPlayList *ppl=&pPlayList[idx];
      if(Unicode_isEqual(pFilePathUnicode,ppl->pPathUnicode)==true){
        if(Unicode_isEqual(pFileNameUnicode,ppl->pFilenameUnicode)==true){
          PlayListIndex=idx;
          break;
        }
      }
    }
  }
  
  if(PlayListIndex==(u32)-1){
    PlayListIndex=0;
    PlayPosition=0;
  }
  
  Shuffle_Refresh(PlayListIndex);

  PlayList_PlayCurrentIndex();
  
  if(PlayPosition!=0){
    DLLSound_SetPosOffset(PlayPosition);
    FileHeader.Position=PlayPosition;
  }
  
  FileHeader.Error=false;
  if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
  
  DTCM_StackCheck(-1);
  PrintFreeMem();
  
  return(true);
}

bool PlayList_GetPause(void)
{
  return(FileHeader.Pause);
}

void PlayList_SetPause(bool f)
{
  FileHeader.Pause=f;
  if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
}

void PlayList_TogglePause(void)
{
  if(FileHeader.Pause==false){
    FileHeader.Pause=true;
    }else{
    FileHeader.Pause=false;
  }
  
  if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
}

void PlayList_SetError(bool f)
{
  FileHeader.Error=f;
  if(FileHeaderSectorNum!=0) active_interface->writeSectors(FileHeaderSectorNum,1,&FileHeader);
}

