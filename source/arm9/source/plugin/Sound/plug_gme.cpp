
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "maindef.h"
#include "strtool.h"

#include "memtool.h"
#include "fat2.h"

#include "internaldrivers.h"

#include "plug_gme.h"

#ifdef ExceptGME

#include "inifile.h"

#include "libgme/Music_Emu.h"
#include "libgme/Data_Reader.h"
#include "libgme/Effects_Buffer.h"

enum EFileType {ftNull,ftNSF,ftGBS,ftHES,ftAY,ftSAP,ftKSS};

static EFileType FileType;

static int TrackNum;

static u8 *DeflateBuf;
static u32 DeflateSize;

static Music_Emu *pemu;
static Effects_Buffer *peffbuf;

static s16 *pGenBuf;

static const u32 SampleRate=32768;
static const u32 SamplePerFrame=2048;

static int PosMax;
static int PosOffset;

#define FadeVolumeMax (32)
static int FadeVolume;

#include "libgme/Nsf_Emu.h"
#include "libgme/format_Gb/Gbs_Emu.h"
#include "libgme/format_Hes/Hes_Emu.h"
#include "libgme/format_Ay/Ay_Emu.h"
#include "libgme/format_Sap/Sap_Emu.h"
//#include "libgme/format_Spc/Spc_Emu.h"
#include "libgme/format_Kss/Kss_Emu.h"

#include "plug_gme_nsf.h"
#include "plug_gme_gbs.h"
#include "plug_gme_hes.h"
#include "plug_gme_ay.h"
#include "plug_gme_sap.h"
//#include "plug_gme_spc.h"
#include "plug_gme_kss.h"

// -----------------

CODE_IN_ITCM_GME static void SetEffect(bool flag)
{
  float lev = (float)GlobalINI.GMEPlugin.ReverbLevel/128;
  
  if(lev==0) flag=false;
  
  if(flag==false){
    Effects_Buffer::config_t cfg;
    cfg.effects_enabled=false;
    peffbuf->config(cfg);
    return;
  }
  
  Effects_Buffer::config_t cfg;
  
  cfg.pan_1             = -0.6f * lev;
  cfg.pan_2             =  0.6f * lev;
  cfg.reverb_delay      = 880 * 0.1f;
  cfg.echo_delay        = 610 * 0.1f;
  cfg.reverb_level      = 0.5f * lev;
  cfg.echo_level        = 0.30f * lev;
  cfg.delay_variance    = 180 * 0.1f;
  cfg.effects_enabled   = true;
  
  if(1.0<cfg.pan_1) cfg.pan_1=1.0;
  if(cfg.pan_1<-1.0) cfg.pan_1=-1.0;
  if(1.0<cfg.pan_2) cfg.pan_2=1.0;
  if(cfg.pan_2<-1.0) cfg.pan_2=-1.0;
  
  if(0.8<cfg.reverb_level) cfg.reverb_level=0.8;
  if(0.8<cfg.echo_level) cfg.echo_level=0.8;
  
  peffbuf->config(cfg);
}

CODE_IN_ITCM_GME static bool _start(void)
{
  switch(FileType){
    case ftNull: break;
    case ftNSF: pemu=new Nsf_Emu; break;
    case ftGBS: pemu=new Gbs_Emu; break;
    case ftHES: pemu=new Hes_Emu; break;
    case ftAY: pemu=new Ay_Emu; break;
    case ftSAP: pemu=new Sap_Emu; break;
//    case ftSPC: pemu=new Spc_Emu; break;
    case ftKSS: pemu=new Kss_Emu; break;
  }
  
  if(pemu==NULL) return(false);
  
  peffbuf=new Effects_Buffer;
  if(peffbuf==NULL) return(false);
  
  pemu->set_buffer(peffbuf);
  
  const char* error;
  
  error=pemu->set_sample_rate(SampleRate);
  if(error!=NULL){
    _consolePrintf("gme error:%s\n",error);
    return(false);
  }
  
  {
    Mem_File_Reader *pin;
    
    pin=new Mem_File_Reader(DeflateBuf,DeflateSize);
    error = pemu->load(*pin);
    delete pin; pin=NULL;
    
    if(error!=NULL){
      _consolePrintf("gme error:%s\n",error);
      return(false);
    }
  }
  
  SetEffect(true);
  
  pemu->start_track(TrackNum);
  
  PosOffset=0;
  
  return(true);
}

static void _free(void)
{
  if(pemu!=NULL){
    delete pemu; pemu=NULL;
  }
  if(peffbuf!=NULL){
    delete peffbuf; peffbuf=NULL;
  }
}

// -----------------

bool PlugGME_Start(const char *pEXT,FAT_FILE *pFileHandle,int _TrackNum)
{
  FileType=ftNull;
  
  if(isStrEqual_NoCaseSensitive(pEXT,".nsf")==true) FileType=ftNSF;
  if(isStrEqual_NoCaseSensitive(pEXT,".gbs")==true) FileType=ftGBS;
  if(isStrEqual_NoCaseSensitive(pEXT,".hes")==true) FileType=ftHES;
  if(isStrEqual_NoCaseSensitive(pEXT,".ay")==true) FileType=ftAY;
  if(isStrEqual_NoCaseSensitive(pEXT,".sap")==true) FileType=ftSAP;
//  if(isStrEqual_NoCaseSensitive(pEXT,".spc")==true) FileType=ftSPC;
  if(isStrEqual_NoCaseSensitive(pEXT,".kss")==true) FileType=ftKSS;
  
  if(FileType==ftNull) return(false);
  
  TrackNum=_TrackNum;
  if(TrackNum<0) TrackNum=0;
  
  pemu=NULL;
  peffbuf=NULL;
  
  DeflateBuf=NULL;
  DeflateSize=0;
  
  pGenBuf=NULL;
  
  DeflateSize=FAT2_GetFileSize(pFileHandle);
  DeflateBuf=(u8*)safemalloc(&MM_DLLSound,DeflateSize);
  if(DeflateBuf==NULL) return(false);
  FAT2_fread(DeflateBuf,1,DeflateSize,pFileHandle);
  
  pGenBuf=(s16*)safemalloc(&MM_DLLSound,SamplePerFrame*2*2);
  if(pGenBuf==NULL){
    _consolePrint("out of memory.\n");
    PlugGME_Free();
    return(false);
  }
  
  switch(FileType){
    case ftNull: break;
    case ftNSF: {
      if(LoadNSFInfo(DeflateBuf,DeflateSize)==false){
        _consolePrint("NSF Header error.\n");
        PlugGME_Free();
        return(false);
      }
    } break;
    case ftGBS: {
      if(LoadGBSInfo(DeflateBuf,DeflateSize)==false){
        _consolePrint("GBS Header error.\n");
        PlugGME_Free();
        return(false);
      }
    } break;
    case ftHES: {
      if(LoadHESInfo(DeflateBuf,DeflateSize)==false){
        _consolePrint("HES Header error.\n");
        PlugGME_Free();
        return(false);
      }
    } break;
    case ftAY: {
      if(LoadAYInfo(DeflateBuf,DeflateSize,TrackNum)==false){
        _consolePrint("AY Header error.\n");
        PlugGME_Free();
        return(false);
      }
    } break;
    case ftSAP: {
      if(LoadSAPInfo(DeflateBuf,DeflateSize)==false){
        _consolePrint("SAP Header error.\n");
        PlugGME_Free();
        return(false);
      }
    } break;
/*
    case ftSPC: {
      if(LoadSPCInfo(DeflateBuf,DeflateSize)==false){
        _consolePrint("SPC Header error.\n");
        PlugGME_Free();
        return(false);
      }
    } break;
*/
    case ftKSS: {
      if(LoadKSSInfo(DeflateBuf,DeflateSize)==false){
        _consolePrint("KSS Header error.\n");
        PlugGME_Free();
        return(false);
      }
    } break;
  }
  
/*
  switch(FileType){
    case ftNull: break;
    case ftNSF: ShowNSFInfo(); break;
    case ftGBS: ShowGBSInfo(); break;
    case ftHES: ShowHESInfo(); break;
    case ftAY: ShowAYInfo(); break;
    case ftSAP  ShowSAPInfo(); break;
    case ftSPC: ShowSPCInfo(); break;
    case ftKSS: ShowKSSInfo(); break;
  }
*/
  
  if(GlobalINI.GMEPlugin.DefaultLengthSec!=0){
    PosMax=GlobalINI.GMEPlugin.DefaultLengthSec*SampleRate;
    }else{
    PosMax=90*SampleRate;
  }
  
  if(_start()==false){
    PlugGME_Free();
    return(false);
  }
  
  {
    int strcnt=pemu->voice_count();
    const char **strs=pemu->voice_names();
    
    for(int idx=0;idx<strcnt;idx++){
      _consolePrintf("%d:%s\n",idx,strs[idx]);
    }
  }
  
  FadeVolume=FadeVolumeMax;
  
  return(true);
}

void PlugGME_Free(void)
{
  _free();
  
  if(pGenBuf!=NULL){
    safefree(&MM_DLLSound,pGenBuf); pGenBuf=NULL;
  }
  
  if(DeflateBuf!=NULL){
    safefree(&MM_DLLSound,DeflateBuf); DeflateBuf=NULL;
  }
}

u32 PlugGME_Update(u32 *plrdst)
{
  if(pemu==NULL) return(0);
  if(pemu->track_ended()==true) return(0);
  
  if(GlobalINI.GMEPlugin.DefaultLengthSec!=0){
    if(PosMax<PosOffset){
      FadeVolume--;
      if(FadeVolume==0) return(0);
    }
  }
  
  PosOffset+=SamplePerFrame;
  
  if(plrdst==NULL){
    SetEffect(false);
    pemu->skip(SamplePerFrame*2);
    SetEffect(true);
    return(SamplePerFrame);
  }
  
  pemu->play(SamplePerFrame*2,pGenBuf);
  
  u32 *psrclrbuf=(u32*)pGenBuf;
  
  switch(GlobalINI.GMEPlugin.SimpleLPF){
    case EGMESimpleLPF_None: {
      if(FadeVolume==FadeVolumeMax){
        for(u32 idx=0;idx<SamplePerFrame;idx++){
          plrdst[idx]=psrclrbuf[idx];
        }
        }else{
        s32 vol=FadeVolume;
        for(u32 idx=0;idx<SamplePerFrame;idx++){
          u32 smp=psrclrbuf[idx];
          s16 l=smp&0xffff,r=smp>>16;
          l=l*vol/FadeVolumeMax;
          r=r*vol/FadeVolumeMax;
          plrdst[idx]=(l&0xffff)|(r<<16);
        }
      }
    } break;
    case EGMESimpleLPF_Lite: {
      if(FadeVolume==FadeVolumeMax){
        u32 smp=psrclrbuf[0];
        s16 ll=smp&0xffff,lr=smp>>16;
        for(u32 idx=0;idx<SamplePerFrame;idx++){
          u32 smp=psrclrbuf[idx];
          s16 l=smp&0xffff,r=smp>>16;
          ll=(ll+(l*3))/4;
          lr=(lr+(r*3))/4;
          plrdst[idx]=(ll&0xffff)|(lr<<16);
        }
        }else{
        s32 vol=FadeVolume;
        u32 smp=psrclrbuf[0];
        s16 l=smp&0xffff,r=smp>>16;
        s32 ll=l*vol/FadeVolumeMax,lr=r/FadeVolumeMax;
        vol*=3;
        for(u32 idx=0;idx<SamplePerFrame;idx++){
          u32 smp=psrclrbuf[idx];
          s16 l=smp&0xffff,r=smp>>16;
          ll=(ll+(l*vol/FadeVolumeMax))/4;
          lr=(lr+(r*vol/FadeVolumeMax))/4;
          plrdst[idx]=(ll&0xffff)|(lr<<16);
        }
      }
    } break;
    case EGMESimpleLPF_Heavy: {
      if(FadeVolume==FadeVolumeMax){
        u32 smp=psrclrbuf[0];
        s16 ll=smp&0xffff,lr=smp>>16;
        for(u32 idx=0;idx<SamplePerFrame;idx++){
          u32 smp=psrclrbuf[idx];
          s16 l=smp&0xffff,r=smp>>16;
          ll=(ll+l)/2;
          lr=(lr+r)/2;
          plrdst[idx]=(ll&0xffff)|(lr<<16);
        }
        }else{
        s32 vol=FadeVolume;
        u32 smp=psrclrbuf[0];
        s16 l=smp&0xffff,r=smp>>16;
        s32 ll=l*vol/FadeVolumeMax,lr=r*vol/FadeVolumeMax;
        for(u32 idx=0;idx<SamplePerFrame;idx++){
          u32 smp=psrclrbuf[idx];
          s16 l=smp&0xffff,r=smp>>16;
          ll=(ll+(l*vol/FadeVolumeMax))/2;
          lr=(lr+(r*vol/FadeVolumeMax))/2;
          plrdst[idx]=(ll&0xffff)|(lr<<16);
        }
      }
    } break;
  }
  
  return(SamplePerFrame);
}

s32 PlugGME_GetPosMax(void)
{
  return(PosMax);
}

s32 PlugGME_GetPosOffset(void)
{
  return(PosOffset);
}

void PlugGME_SetPosOffset(s32 ofs)
{
  if(pemu==NULL) return;
  
  if(ofs==PosOffset) return;
  
  if(ofs<PosOffset){
    _free();
    _start();
  }
  
  u32 baseofs=PosOffset;
  u32 prgmax=ofs-baseofs;
  bool prgshow=false;
  
  u32 i=0;
  
  while(PosOffset<ofs){
    i++;
    if((i&15)==0){
      const char *pPrgTitle="Seek...";
      if(prgshow==false){
        prgshow=true;
        CallBack_MWin_ProgressShow(pPrgTitle,prgmax);
        CallBack_MWin_ProgressSetPos(pPrgTitle,PosOffset-baseofs,prgmax);
      }
      CallBack_MWin_ProgressSetPos(pPrgTitle,PosOffset-baseofs,prgmax);
    }
    if(PlugGME_Update(NULL)==0) break;
  }
  
  if(prgshow==true) CallBack_MWin_ProgressHide();
  
  FadeVolume=FadeVolumeMax;
}

u32 PlugGME_GetSampleRate(void)
{
  return(SampleRate);
}

u32 PlugGME_GetChannelCount(void)
{
  return(2);
}

u32 PlugGME_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

u32 PlugGME_GetPlayTimeSec(void)
{
    return(0);
}

int PlugGME_GetInfoIndexCount(void)
{
  switch(FileType){
    case ftNull: break;
    case ftNSF: return(GMENSF_GetInfoIndexCount()); break;
    case ftGBS: return(GMEGBS_GetInfoIndexCount()); break;
    case ftHES: return(GMEHES_GetInfoIndexCount()); break;
    case ftAY: return(GMEAY_GetInfoIndexCount()); break;
    case ftSAP: return(GMESAP_GetInfoIndexCount()); break;
//    case ftSPC: return(GMESPC_GetInfoIndexCount()); break;
    case ftKSS: return(GMEKSS_GetInfoIndexCount()); break;
  }
  return(0);
}

bool PlugGME_GetInfoStrL(int idx,char *str,int len)
{
  switch(FileType){
    case ftNull: break;
    case ftNSF: return(GMENSF_GetInfoStrL(idx,str,len)); break;
    case ftGBS: return(GMEGBS_GetInfoStrL(idx,str,len)); break;
    case ftHES: return(GMEHES_GetInfoStrL(idx,str,len)); break;
    case ftAY: return(GMEAY_GetInfoStrL(idx,str,len)); break;
    case ftSAP: return(GMESAP_GetInfoStrL(idx,str,len)); break;
//    case ftSPC: return(GMESPC_GetInfoStrL(idx,str,len)); break;
    case ftKSS: return(GMEKSS_GetInfoStrL(idx,str,len)); break;
  }
  return(false);
}

bool PlugGME_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugGME_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#endif
