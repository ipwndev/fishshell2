
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"
#include "memtool.h"
#include "shell.h"
#include "playlist.h"

#include "arm9tcm.h"

#include "sndeff.h"

#include "procstate.h"

#include "../../ipc6.h"
#include "strpcm.h"

#include "sndeff_dfs.h"

DATA_IN_AfterSystem static TransferSound staticTransferSound;

typedef struct {
  u32 ofs,smpcnt;
  u16 chs,smprate;
} TWaveHeader;
DATA_IN_AfterSystem static FAT_FILE *pWaveFile;
DATA_IN_AfterSystem static TWaveHeader *pWaveHeader;

typedef struct {
  u32 Freq;
  u32 BufCount;
  u32 Channels;
  u8 *lbuf,*rbuf;
} TSound;

DATA_IN_AfterSystem static TSound Sound;
DATA_IN_AfterSystem static u32 WavesCount;

DATA_IN_AfterSystem static EWAVFN lastfn;

static void playSoundBlock(TransferSound *snd)
{
  DCache_CleanRangeOverrun(snd, sizeof(TransferSound) );
  IPC6->soundData = snd;
}

static void Sound_Free(bool WaitForTerminate)
{
  while(IPC6->RequestStopSound==true){
    swiWaitForVBlank();
}

  IPC6->RequestStopSound=true;
  
  if(WaitForTerminate==true){
    while(IPC6->RequestStopSound==true){
      swiWaitForVBlank();
  }
}

  lastfn=(EWAVFN)-1;

  Sound.Freq=0;
  Sound.BufCount=0;
  Sound.Channels=0;

  if(Sound.lbuf!=NULL){
    safefree(&MM_System,Sound.lbuf); Sound.lbuf=NULL;
  }
  if(Sound.rbuf!=NULL){
    safefree(&MM_System,Sound.rbuf); Sound.rbuf=NULL;
  }
}

DATA_IN_IWRAM_MainPass void Sound_Open(void)
{
  lastfn=(EWAVFN)-1;
  
  Sound.Freq=0;
  Sound.BufCount=0;
  Sound.Channels=0;
  Sound.lbuf=NULL;
  Sound.rbuf=NULL;
  
  pWaveFile=NULL;
  pWaveHeader=NULL;
  
  pWaveFile=Shell_FAT_fopen_Internal_WithCheckExists(SNDEFFDATFilename);
  if(pWaveFile==NULL) return;
  
  FAT2_fread_fast(&WavesCount,4,1,pWaveFile);
  pWaveHeader=(TWaveHeader*)safemalloc_chkmem(&MM_System,sizeof(TWaveHeader)*WavesCount);
  FAT2_fread_fast(pWaveHeader,sizeof(TWaveHeader),WavesCount,pWaveFile);

  if(VerboseDebugLog==true) _consolePrintf("Wave header loaded. (%d,%d)\n",sizeof(TWaveHeader),WavesCount);
}

void Sound_Close(void)
{
  Sound_Free(true);
  
  if(pWaveFile!=NULL){
    FAT2_fclose(pWaveFile); pWaveFile=NULL;
  }
  
  if(pWaveHeader!=NULL){
    safefree(&MM_System,pWaveHeader); pWaveHeader=NULL;
  }
}


static void SetupSoundBlock(volatile TransferSound *ptsnd,TSound *pSound,u32 Volume)
{
  volatile TransferSoundData *ptsnddata=ptsnd->data;
  
  u8 *lbuf,*rbuf;
  
  switch(pSound->Channels){
    case 1: {
      lbuf=pSound->lbuf;
      rbuf=pSound->lbuf;
    } break;
    case 2: {
      lbuf=pSound->lbuf;
      rbuf=pSound->rbuf;
      Volume/=2;
    } break;
    default: StopFatalError(13802,"sound effect illigal channels count.\n"); break;
  }
  
  if(127<Volume) Volume=127;
  
  ptsnddata[0].rate=pSound->Freq;
  ptsnddata[0].data=(s8*)lbuf;
  ptsnddata[0].len=pSound->BufCount*1;
  ptsnddata[0].vol=Volume;
  ptsnddata[0].pan=0;
  ptsnddata[0].format=1;
  
  ptsnddata[1].rate=pSound->Freq;
  ptsnddata[1].data=(s8*)rbuf;
  ptsnddata[1].len=pSound->BufCount*1;
  ptsnddata[1].vol=Volume;
  ptsnddata[1].pan=127;
  ptsnddata[1].format=1;
  
  ptsnd->count=2;
}

void Sound_Start(EWAVFN WAVFN)
{
  if((pWaveFile==NULL)||(pWaveHeader==NULL)||(WavesCount<=(u32)WAVFN)) return;
  
  if(ProcState.System.ClickSound==false) return;
  
  if(WAVFN!=lastfn){
    Sound_Free(false);
    
    TWaveHeader *pwh=&pWaveHeader[(u32)WAVFN];
//    if(VerboseDebugLog==true) _consolePrintf("SE: ID:%d, %dHz, %dsmps, %dchs. ofs=%dbyte.\n",WAVFN,pwh->smprate,pwh->smpcnt,pwh->chs,pwh->ofs);
  
    TSound *pSnd=&Sound;
    pSnd->Freq=pwh->smprate;
    pSnd->Channels=pwh->chs;
    pSnd->BufCount=pwh->smpcnt;
  
    FAT2_fseek(pWaveFile,pwh->ofs,SEEK_SET);

    pSnd->lbuf=(u8*)safemalloc_chkmem(&MM_System,pwh->smpcnt);
    FAT2_fread_fast(pSnd->lbuf,1,pwh->smpcnt,pWaveFile);
    
    if(pwh->chs==2){
      pSnd->rbuf=(u8*)safemalloc_chkmem(&MM_System,pwh->smpcnt);
      FAT2_fread_fast(pSnd->rbuf,1,pwh->smpcnt,pWaveFile);
  }
    
    lastfn=WAVFN;
}

  while(IPC6->RequestStopSound==true){
    swiWaitForVBlank();
  }

  IPC6->LoopSound=false;
  
  u32 Volume=strpcmGetAudioVolume64()*2;
  
  if((PlayList_isOpened()==true)&&(PlayList_GetPause()==false)){
      Volume=strpcmGetAudioVolume64()/2;
  }

  SetupSoundBlock(&staticTransferSound,&Sound,Volume);
  playSoundBlock(&staticTransferSound);
}

u32 Sound_GetCurrentPlayTimePerVsync(void)
{
  u32 freq=Sound.Freq;
  u32 samples=Sound.BufCount*1;
  
  return(samples*60/freq);
}

