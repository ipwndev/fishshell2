
#ifdef ExceptMP3

#include "plug_mp3.h"
DATA_IN_AfterSystem static bool isMP3=false;

static bool DLLSound_ChkExt_internal_mp3(const char *pext)
{
  bool res=false;
  if(isStrEqual_NoCaseSensitive(pext,".mp1")==true) res=true;
  if(isStrEqual_NoCaseSensitive(pext,".mp2")==true) res=true;
  if(isStrEqual_NoCaseSensitive(pext,".mp3")==true) res=true;
  return(res);
}

static bool DLLSound_Open_internal_mp3(const char *pext)
{
    OVM_libsnd_mp3();
  if(PlugMP3_Start(PluginBody_FileHandle)==false){
    PlugMP3_Free();
    FAT2_fclose(PluginBody_FileHandle); PluginBody_FileHandle=NULL;
    _consolePrint("Can not start internal MP3 plugin.\n");
    return(false);
  }
  DTCM_StackCheck(-1);
  
  u32 rate=PlugMP3_GetSampleRate();
  u32 spf=PlugMP3_GetSamplePerFrame();
  u32 chs=PlugMP3_GetChannelCount();
  EstrpcmFormat spfrm;
  
  if(rate<=48000){
    spfrm=SPF_PCMx4;
    }else{
    spfrm=SPF_PCMx2;
  }
  
  MACRO_LoadGeneralID3Tag(&MM_DLLSound,PlugMP3_GetInfoIndexCount,PlugMP3_GetInfoStrL,PlugMP3_GetInfoStrUTF8,PlugMP3_GetInfoStrW);
  
  DLLSound_Open_ins_strpcmStart(rate,spf,chs,spfrm);
  
  MM_CheckOverRange();
  
  isMP3=true;
  
  return(true);
}

#endif

static void DLLSound_Update_internal_mp3(u32 BaseSamples,u32 *plrdst)
{
  u32 Samples=0;
  { cwl();
    // PrfStart();
    u32 lastpos=PlugMP3_GetPosOffset();
    Samples=PlugMP3_Update(plrdst);
    DTCM_StackCheck(-1);
    u32 curpos=PlugMP3_GetPosOffset();
    // PrfEnd(curpos);
    if(Samples!=BaseSamples) strpcmRequestStop=true;
  }
  
  if(Samples<BaseSamples){ cwl();
    for(u32 idx=Samples;idx<BaseSamples;idx++){ cwl();
      plrdst[idx]=0;
    }
  }
}

static u32 DLLSound_GetPlayTimeSec_internal_mp3(void)
{
  return(PlugMP3_GetPlayTimeSec());
}
