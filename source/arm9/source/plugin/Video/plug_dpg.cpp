
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds.h>

#include "cstream_fs.h"
#include "_console.h"

#include "plug_dpg.h"
#include "plug_mp2.h"

#include "clibdpg.h"

#include "libmpeg2/config.h"

DATA_IN_MTCM_VAR bool DPG_RequestSyncStart;

DATA_IN_MTCM_VAR static bool Initialized=false;

DATA_IN_MTCM_VAR static Clibdpg *pClibdpg=NULL;

DATA_IN_MTCM_VAR static CStreamFS *pCStreamFS_DPGM=NULL;
DATA_IN_MTCM_VAR static CStreamFS *pCStreamFS_DPGA=NULL;

#include "_dpgfs.h"

bool StartDPG(FAT_FILE *_FileHandleVideo,FAT_FILE *_FileHandleAudio)
{
  if(Initialized==true) FreeDPG();
  Initialized=true;
  
  DPGFS_Init(_FileHandleVideo);
  
  pCStreamFS_DPGM=new CStreamFS(_FileHandleVideo);
  pCStreamFS_DPGA=new CStreamFS(_FileHandleAudio);
  
  pClibdpg=new Clibdpg(pCStreamFS_DPGM,pCStreamFS_DPGA);
  
  if(pClibdpg->Initialized==false) return(false);
  
  pClibdpg->SetFrame(0);
  
  return(true);
}

void FreeDPG(void)
{
  if(Initialized==false) return;
  Initialized=false;
  
  DPGFS_Free();
  
  DPG_RequestSyncStart=false;
  
  if(pCStreamFS_DPGM!=NULL){
    delete pCStreamFS_DPGM; pCStreamFS_DPGM=NULL;
  }
  if(pCStreamFS_DPGA!=NULL){
    delete pCStreamFS_DPGA; pCStreamFS_DPGA=NULL;
  }
  if(pClibdpg!=NULL){
    delete pClibdpg; pClibdpg=NULL;
  }
}

u32 DPG_GetCurrentFrameCount(void)
{
  return(pClibdpg->GetFrameNum());
}

u32 DPG_GetTotalFrameCount(void)
{
  return(pClibdpg->DPGINFO.TotalFrame);
}

u32 DPG_GetFPS(void)
{
  return(pClibdpg->DPGINFO.FPS);
}

u32 DPG_GetSampleRate(void)
{
  return(pClibdpg->DPGINFO.SndFreq);
}

u32 DPG_GetChannelCount(void)
{
  return(pClibdpg->DPGINFO.SndCh);
}

void UpdateDPG_Audio(void)
{
  if(Initialized==false) return;
  
  switch(pClibdpg->GetDPGAudioFormat()){
    case DPGAF_MP2: MP2_LoadReadBuffer(); break;
  }
}

bool UpdateDPG_Video(void)
{
  if(Initialized==false) return(false);
  
  return(pClibdpg->MovieProcDecode());
}

extern void DPG_SliceOneFrame(u16 *pVRAMBuf1,u16 *pVRAMBuf2)
{
  if(Initialized==false) return;
  pClibdpg->SliceOneFrame(pVRAMBuf1,pVRAMBuf2);
}

void DPG_SetFrameCount(u32 Frame)
{
  pClibdpg->SetFrame(Frame);
}

u32 DPG_GetWidth(void)
{
  return(pClibdpg->GetWidth());
}

u32 DPG_GetHeight(void)
{
  return(pClibdpg->GetHeight());
}

EDPGAudioFormat DPG_GetDPGAudioFormat(void)
{
  return(pClibdpg->GetDPGAudioFormat());
}

void DPG_DrawInfo(CglCanvas *pCanvas)
{
  TDPGINFO *pDPGINFO=&pClibdpg->DPGINFO;
  
  char str[64];
  
  u32 x=8,y=DPG_DrawInfoDiffHeight()+8,h=14;
  u32 line=0;
  
  snprintf(str,64,"DPG file information.");
  pCanvas->TextOutA(x,y,str);
  line++;
  
  snprintf(str,64,"Video TotalFrame=%d fps=%f",pDPGINFO->TotalFrame,(float)pDPGINFO->FPS/0x100);
  pCanvas->TextOutA(x,y+(h*line),str);
  line++;
  const char pfstr[][6]={"RGB15","RGB18","RGB21","RGB24"};
  snprintf(str,64,"ScreenSize %dx%dpixels (%s)",pClibdpg->GetWidth(),pClibdpg->GetHeight(),pfstr[(int)pDPGINFO->PixelFormat]);
  pCanvas->TextOutA(x,y+(h*line),str);
  line++;
  switch(pDPGINFO->SndCh){
    case 0: snprintf(str,64,"%dHz mp2 build-in mpeg1audio-layer2",pDPGINFO->SndFreq); break;
    default: snprintf(str,64,"Unknown format"); break;
  }
  pCanvas->TextOutA(x,y+(h*line),str);
  line++;
  snprintf(str,64,"MovieData %d->%dbytes.",pDPGINFO->MoviePos,pDPGINFO->MovieSize);
  pCanvas->TextOutA(x,y+(h*line),str);
  line++;
  snprintf(str,64,"AudioData %d->%dbytes.",pDPGINFO->AudioPos,pDPGINFO->AudioSize);
  pCanvas->TextOutA(x,y+(h*line),str);
  line++;
  snprintf(str,64,"GOPListData %d->%dbytes.",pDPGINFO->GOPListPos,pDPGINFO->GOPListSize);
  pCanvas->TextOutA(x,y+(h*line),str);
  line++;
}

extern u32 DiskCache_GetReadySize(void);

u32 DPG_DrawInfoDiffHeight(void)
{
  return(32);
}

void DPG_DrawInfoDiff(CglCanvas *pCanvas)
{
  char str[64];
  
  u32 x=8,y=4;
  
  snprintf(str,64,"Frame cache %d/%dframes",FrameCache_GetReadFrameLastCount(),FrameCache_GetReadFramesCount());
  pCanvas->TextOutA(x,y+(14*0),str);
  snprintf(str,64,"Disk cache %dbytes",DiskCache_GetReadySize());
  pCanvas->TextOutA(x,y+(14*1),str);
  
  x+=176;
  pCanvas->TextOutA(x,y+(14*0),"current.");
  snprintf(str,64,"%dframe",DPG_GetCurrentFrameCount());
  pCanvas->TextOutA(x,y+(14*1),str);
}


