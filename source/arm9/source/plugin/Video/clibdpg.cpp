
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "memtool.h"

#include "clibdpg.h"

#include "plug_mp2.h"

#define DPG0ID (0x30475044)
#define DPG1ID (0x31475044)
#define DPG2ID (0x32475044)
#define DPG3ID (0x33475044)
#define DPG4ID (0x34475044)

#define DPGThumbnailImageID (0x304d4854)

#include "_dpgfs.h"

Clibdpg::Clibdpg(CStream *_pCStreamMovie,CStream *_pCStreamAudio)
{
  Initialized=false;
  
  GOPListCount=0;
  pGOPList=NULL;
  if(LoadDPGINFO(_pCStreamMovie)==false) return;
  
  pCStreamMovie=_pCStreamMovie;
  pCStreamAudio=_pCStreamAudio;
  
  pCStreamMovie->SetOffset(DPGINFO.MoviePos);
  pCStreamMovie->OverrideSize(DPGINFO.MoviePos+DPGINFO.MovieSize);
  DPGFS_Movie_SetAttribute(pCStreamMovie->GetOffset(),pCStreamMovie->GetSize());
  pClibmpg=new Clibmpg(pCStreamMovie,DPGINFO.TotalFrame,DPGINFO.FPS,DPGINFO.SndFreq,DPGINFO.PixelFormat);
  
  if(pClibmpg->GetWidth()!=ScreenWidth) StopFatalError(14604,"Horizontal sizes other than 256 pixels became unsupported.\n");
  
  // pClibmpg->SetVideoDelayms(1000*0x100/DPGINFO.FPS*0);
  
  pCStreamAudio->SetOffset(DPGINFO.AudioPos);
  pCStreamAudio->OverrideSize(DPGINFO.AudioPos+DPGINFO.AudioSize);
  DPGFS_Audio_SetAttribute(DPGINFO.AudioPos,DPGINFO.AudioSize);
  
  switch(GetDPGAudioFormat()){
    case DPGAF_MP2: StartMP2(); break;
  }
  
  Initialized=true;
}

Clibdpg::~Clibdpg(void)
{
  if(pClibmpg!=NULL){
    delete pClibmpg; pClibmpg=NULL;
  }
  
  switch(GetDPGAudioFormat()){
    case DPGAF_MP2: FreeMP2(); break;
  }
  
  GOPListCount=0;
  if(pGOPList!=NULL){
    safefree(&MM_DLLDPG,pGOPList); pGOPList=NULL;
  }
  
  pCStreamMovie=NULL;
  pCStreamAudio=NULL;
}

bool Clibdpg::LoadDPGINFO(CStream *pCStream)
{
  pCStream->SetOffset(0);
  
  u32 id=pCStream->Readu32();
  if((id!=DPG0ID)&&(id!=DPG1ID)&&(id!=DPG2ID)&&(id!=DPG3ID)&&(id!=DPG4ID)){
    pCStream->SetOffset(0);
    _consolePrintf("Unknown DPG Format.ID=%08x\n",pCStream->Readu32());
    return(false);
  }
  
  DPGINFO.TotalFrame=pCStream->Readu32();
  DPGINFO.FPS=pCStream->Readu32();
  DPGINFO.SndFreq=pCStream->Readu32();
  DPGINFO.SndCh=pCStream->Readu32();
  DPGINFO.AudioPos=pCStream->Readu32();
  DPGINFO.AudioSize=pCStream->Readu32();
  DPGINFO.MoviePos=pCStream->Readu32();
  DPGINFO.MovieSize=pCStream->Readu32();
  
  DPGINFO.GOPListPos=0;
  DPGINFO.GOPListSize=0;
  GOPListCount=0;
  pGOPList=NULL;
  
  if((id==DPG2ID)||(id==DPG3ID)||(id==DPG4ID)){
    DPGINFO.GOPListPos=pCStream->Readu32();
    DPGINFO.GOPListSize=pCStream->Readu32();
  }
  
  if(id==DPG0ID) DPGINFO.PixelFormat=PF_RGB24;
  if((id==DPG1ID)||(id==DPG2ID)||(id==DPG3ID)||(id==DPG4ID)) DPGINFO.PixelFormat=(EMPGPixelFormat)(pCStream->Readu32());
  
  if(pCStream->Readu32()==DPGThumbnailImageID) {
        DPGINFO.isExistsThumbnailImage=true;
        DPGINFO.ThumbnailImageOffset=pCStream->GetOffset();
  }else{
        DPGINFO.isExistsThumbnailImage=false;
        DPGINFO.ThumbnailImageOffset=0;
  }
  
  if(DPGINFO.isExistsThumbnailImage){
      _consolePrintf("Included thumbnail image.Offset=0x%x\n",DPGINFO.ThumbnailImageOffset);
  }else{
      _consolePrint("Not include thumbnail image.\n");
  }
  
  if((DPGINFO.GOPListPos!=0)&&(DPGINFO.GOPListSize!=0)){
    GOPListCount=DPGINFO.GOPListSize/8;
    pGOPList=(TGOPList*)safemalloc_chkmem(&MM_DLLDPG,GOPListCount*8);
    _consolePrintf("GOPListCount=%d,pGOPList=0x%x\n",GOPListCount,pGOPList);
    pCStream->SetOffset(DPGINFO.GOPListPos);
    for(u32 idx=0;idx<GOPListCount;idx++){
      pGOPList[idx].FrameIndex=pCStream->Readu32();
      pGOPList[idx].Offset=pCStream->Readu32();
    }
  }
  
  return(true);
}

bool Clibdpg::MovieProcDecode(void)
{
  return(pClibmpg->ProcDecode());
}

void Clibdpg::SliceOneFrame(u16 *pVRAMBuf1,u16 *pVRAMBuf2)
{
  pClibmpg->SliceOneFrame(pVRAMBuf1,pVRAMBuf2);
}

void Clibdpg::SetFrame(u32 Frame)
{
  u64 smp=(u64)Frame*DPGINFO.SndFreq*0x100/DPGINFO.FPS;
  
  if((GOPListCount==0)||(pGOPList==NULL)){
    if(pClibmpg->ProcMoveFrame(Frame,smp)==false) StopFatalError(14601,"pClibmpg->ProcMoveFrame failed.\n");
    }else{
    u32 GOPFrame=0,GOPOffset=0;
    for(u32 idx=0;idx<GOPListCount;idx++){
      if((int)pGOPList[idx].FrameIndex<=Frame){
        GOPFrame=pGOPList[idx].FrameIndex;
        GOPOffset=pGOPList[idx].Offset;
        }else{
        break;
      }
    }
    _consolePrintf("GOPFrame=%d, GOPOffset=0x%x\n",GOPFrame,GOPOffset);
    if(pClibmpg->ProcMoveFrameGOP(Frame,smp,GOPFrame,GOPOffset)==false) StopFatalError(14602,"pClibmpg->ProcMoveFrameGOP failed.\n");
    Frame=pClibmpg->GetFrameNum();
  }
  
  double per=(double)Frame/((double)DPGINFO.TotalFrame);
  u64 ssmp=(u64)Frame*DPGINFO.SndFreq*0x100/DPGINFO.FPS;
  
  switch(GetDPGAudioFormat()){
    case DPGAF_MP2: {
      MP2_SetPosition(per,ssmp);
    } break;
  }
  
}

int Clibdpg::GetFrameNum(void)
{
  return(pClibmpg->GetFrameNum());
}

int Clibdpg::GetWidth(void)
{
  return(pClibmpg->GetWidth());
}

int Clibdpg::GetHeight(void)
{
  return(pClibmpg->GetHeight());
}

EDPGAudioFormat Clibdpg::GetDPGAudioFormat(void)
{
  switch(DPGINFO.SndCh){
    case 0: return(DPGAF_MP2);
  }
  
  StopFatalError(14603,"unknown audio format. (%d)\n",DPGINFO.SndCh);
  return((EDPGAudioFormat)-1);
}

