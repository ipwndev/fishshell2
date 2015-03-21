
#include <stdio.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"

#include "memtool.h"

#include "rcplib.h"

#include "pch.h"
#include "mtrk.h"

#include "rcplib_rcp.h"

static u8 *bdata;
static u32 bSampleRate;
static u32 bGenVolume;

void rcplibSetParam(u8 *data,u32 SampleRate,u32 GenVolume)
{
  bdata=data;
  bSampleRate=SampleRate;
  bGenVolume=GenVolume;
}

bool rcplibStart(void)
{
  RCP_Init();
  RCP_LoadRCP(bdata,bSampleRate);
  
  PCH_Init(bSampleRate,bGenVolume);
  MTRKCC_Init();
  
  return(true);
}

void rcplibFree(void)
{
  MTRKCC_Free();
  PCH_Free();
  RCP_Free();
}

int rcplibGetNearClock(void)
{
  u32 TrackCount=pRCP_Chank->TrackCount;
  
  int NearClock=0x7fffffff;
  
  for(u32 TrackNum=0;TrackNum<TrackCount;TrackNum++){
    TRCP_Track *pRCP_Track=&pRCP->RCP_Track[TrackNum];
    if(pRCP_Track->EndFlag==false){
      if(pRCP_Track->WaitClock<NearClock) NearClock=pRCP_Track->WaitClock;
    }
  }
  
  int GTNearClock=PCH_GT_GetNearClock();
  if(GTNearClock!=0){
    if(GTNearClock<NearClock) NearClock=GTNearClock;
  }
  
  if(NearClock==0x7fffffff) NearClock=0;
  
  return(NearClock);
}

bool rcplibNextClock(bool ShowEventMessage,bool EnableNote,int DecClock)
{
  u32 TrackCount=pRCP_Chank->TrackCount;
  
  while(1){
    if(RCP_isAllTrackEOF()==true) return(false);
    
    if(EnableNote==true) PCH_GT_DecClock(DecClock);
    
    for(u32 TrackNum=0;TrackNum<TrackCount;TrackNum++){
      TRCP_Track *pRCP_Track=&pRCP->RCP_Track[TrackNum];
      if(pRCP_Track->EndFlag==false){
        pRCP_Track->WaitClock-=DecClock;
        while(pRCP_Track->WaitClock==0){
          RCP_ProcRCP(ShowEventMessage,EnableNote,pRCP_Track);
          
          if(pRCP_Track->EndFlag==true){
//            _consolePrintf("end of Track(%d)\n",TrackNum);
            break;
          }
          
        }
      }
    }
    if(pRCP->FastNoteOn==true) break;
    DecClock=rcplibGetNearClock();
  }
  
  return(true);
}

void rcplibAllSoundOff(void)
{
  PCH_AllSoundOff();
}

