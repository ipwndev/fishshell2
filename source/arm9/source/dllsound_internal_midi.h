
#ifdef ExceptMIDI

#include "plug_midi.h"
DATA_IN_AfterSystem static bool isMIDI=false;

static bool DLLSound_ChkExt_internal_midi(const char *pext)
{
  bool res=false;
  if(isStrEqual_NoCaseSensitive(pext,".mid")==true) res=true;
  if(isStrEqual_NoCaseSensitive(pext,".rcp")==true) res=true;
  if(isStrEqual_NoCaseSensitive(pext,".r36")==true) res=true;
  return(res);
}

static bool DLLSound_Open_internal_midi(const char *pext)
{
    OVM_libsnd_midi();
  if(PlugMIDI_Start(PluginBody_FileHandle)==false){
    PlugMIDI_Free();
    FAT2_fclose(PluginBody_FileHandle); PluginBody_FileHandle=NULL;
    _consolePrint("Can not start internal MIDI plugin.\n");
    return(false);
  }
  DTCM_StackCheck(-1);
  
  u32 rate=PlugMIDI_GetSampleRate();
  u32 spf=PlugMIDI_GetSamplePerFrame();
  u32 chs=PlugMIDI_GetChannelCount();
  
  MACRO_LoadGeneralID3Tag(&MM_DLLSound,PlugMIDI_GetInfoIndexCount,PlugMIDI_GetInfoStrL,PlugMIDI_GetInfoStrUTF8,PlugMIDI_GetInfoStrW);

  DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx1);
  
  MM_CheckOverRange();
  
  PrintFreeMem();
  
  isMIDI=true;
  
  return(true);
}

#endif

static void DLLSound_Update_internal_midi(u32 BaseSamples,u32 *plrdst)
{
  u32 Samples=0;
  { cwl();
    // PrfStart();
    u32 lastpos=PlugMIDI_GetPosOffset();
    Samples=PlugMIDI_Update(plrdst);
    DTCM_StackCheck(-1);
    u32 curpos=PlugMIDI_GetPosOffset();
    if((PlugTime_SkipCheck==false)&&(lastpos<=curpos)){
      PlugTime_TotalSamples+=Samples;
      PlugTime_SamplesDivCount+=curpos-lastpos;
    }
    PlugTime_SkipCheck=false;
    // PrfEnd(curpos);
    if(Samples!=BaseSamples) strpcmRequestStop=true;
  }
  
  if(Samples<BaseSamples){ cwl();
    for(u32 idx=Samples;idx<BaseSamples;idx++){ cwl();
      plrdst[idx]=0;
    }
  }
}

static u32 DLLSound_GetPlayTimeSec_internal_midi(void)
{
  return(PlugMIDI_GetPlayTimeSec());
}


