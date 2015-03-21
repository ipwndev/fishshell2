
#ifdef ExceptGME

#include "plug_gme.h"
DATA_IN_AfterSystem static bool isGME=false;

static bool DLLSound_ChkExt_internal_gme(const char *pext)
{
  bool res=false;
  if(isStrEqual_NoCaseSensitive(pext,".nsf")==true) res=true;
  if(isStrEqual_NoCaseSensitive(pext,".gbs")==true) res=true;
  if(isStrEqual_NoCaseSensitive(pext,".hes")==true) res=true;
  if(isStrEqual_NoCaseSensitive(pext,".ay")==true) res=true;
  if(isStrEqual_NoCaseSensitive(pext,".sap")==true) res=true;
//  if(isStrEqual_NoCaseSensitive(pext,".spc")==true) res=true;
  if(isStrEqual_NoCaseSensitive(pext,".kss")==true) res=true;
  return(res);
}

bool DLLSound_Open_internal_gme(const char *pext)
{
	OVM_libsnd_gme();
  if(PlugGME_Start(pext,PluginBody_FileHandle,0)==false){
    PlugGME_Free();
    FAT2_fclose(PluginBody_FileHandle); PluginBody_FileHandle=NULL;
    _consolePrintf("Can not start internal GME plugin.\n");
    return(false);
  }
  DTCM_StackCheck(-1);
  
  u32 rate=PlugGME_GetSampleRate();
  u32 spf=PlugGME_GetSamplePerFrame();
  u32 chs=PlugGME_GetChannelCount();
  
  MACRO_LoadGeneralID3Tag(&MM_DLLSound,PlugGME_GetInfoIndexCount,PlugGME_GetInfoStrL,PlugGME_GetInfoStrUTF8,PlugGME_GetInfoStrW);

  DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx1);
  
  MM_CheckOverRange();
  
  PrintFreeMem();
  
  isGME=true;
  
  return(true);
}

#endif

static void DLLSound_Update_internal_gme(u32 BaseSamples,u32 *plrdst)
{
  u32 Samples=0;
  { cwl();
    // PrfStart();
    u32 lastpos=PlugGME_GetPosOffset();
    Samples=PlugGME_Update(plrdst);
    DTCM_StackCheck(-1);
    u32 curpos=PlugGME_GetPosOffset();
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

