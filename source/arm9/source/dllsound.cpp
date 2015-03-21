
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "fat2.h"
#include "memtool.h"
#include "shell.h"
#include "lang.h"
#include "strtool.h"
#include "unicode.h"
#include "euc2unicode.h"

#include "dll.h"

#include "dllsound.h"
#include "strpcm.h"
#include "playlist.h"

#include "internaldrivers.h"

#include "OverlayManager.h"

DATA_IN_AfterSystem TID3Tag ID3Tag;

typedef struct {
  u32 SourceRate;
  u32 Pos,Size;
  u32 Mod16,Add16;
  u32 CountSource,Count32768;
  u32 *pReadBufLR;
  u32 LastSampleLR;
} TRateConv;

DATA_IN_AfterSystem static TRateConv RateConv={0,};

DATA_IN_AfterSystem static TPluginBody *pPluginBody=NULL;
DATA_IN_AfterSystem static FAT_FILE *PluginBody_FileHandle=NULL;

DATA_IN_AfterSystem static bool CanUseGaplessMode=false;

DATA_IN_AfterSystem static bool PlugTime_SkipCheck;
DATA_IN_AfterSystem static u32 PlugTime_TotalSamples;
DATA_IN_AfterSystem static u32 PlugTime_SamplesDivCount;
DATA_IN_AfterSystem static float PlugTime_CurrentBitRate;
DATA_IN_AfterSystem static float PlugTime_CurrentCount;

DATA_IN_AfterSystem static const bool FastStart=false;

DATA_IN_AfterSystem static bool DLLSound_FirstDecode;

#define MACRO_LoadGeneralID3Tag(pMM,GetInfoIndexCount,GetInfoStrL,GetInfoStrUTF8,GetInfoStrW) { \
  u32 InfoIndexCount=GetInfoIndexCount(); \
  if(ID3Tag_LinesMaxCount<InfoIndexCount) InfoIndexCount=ID3Tag_LinesMaxCount; \
   \
  TID3Tag *pi3t=&ID3Tag; \
  pi3t->LinesCount=0; \
   \
  for(u32 idx=0;idx<InfoIndexCount;idx++){ \
    const u32 MaxLength=128; \
    UnicodeChar tmpstrw[MaxLength+2]; \
    bool Exists=false; \
    if(Exists==false){ \
      if(GetInfoStrW(idx,tmpstrw,MaxLength)==true) Exists=true; \
    } \
    if(Exists==false){ \
      char tmpstr[MaxLength+2]; \
      if(GetInfoStrUTF8(idx,tmpstr,MaxLength)==true){ \
        Exists=true; \
        tmpstr[MaxLength+0]=0; \
        tmpstr[MaxLength+1]=0; \
        StrConvert_UTF82Unicode(tmpstr,tmpstrw); \
      } \
    } \
    if(Exists==false){ \
      char tmpstr[MaxLength+2]; \
      if(GetInfoStrL(idx,tmpstr,MaxLength)==true){ \
        Exists=true; \
        tmpstr[MaxLength+0]=0; \
        tmpstr[MaxLength+1]=0; \
        if(Shell_isEUCmode()==true){ \
          EUC2Unicode_Convert(tmpstr,tmpstrw,MaxLength); \
          }else{ \
          StrConvert_UTF82Unicode(tmpstr,tmpstrw); \
        } \
      } \
    } \
    if((Exists==true)&&(tmpstrw[0]!=0)) pi3t->ppLines[pi3t->LinesCount++]=Unicode_AllocateCopy(pMM,tmpstrw); \
  } \
}

//#undef MACRO_LoadGeneralID3Tag
//#define MACRO_LoadGeneralID3Tag(GetInfoIndexCount,GetInfoStrL,GetInfoStrUTF8,GetInfoStrW) 

static void DLLSound_Open_ins_strpcmStart(u32 rate,u32 spf,u32 chs,EstrpcmFormat spfrm)
{
//  _consolePrintf("%d,%d %d,%d %d,%d %d,%d\n",IPC6->strpcmFreq,rate,IPC6->strpcmSamples,spf,IPC6->strpcmChannels,chs,IPC6->strpcmFormat,spfrm);
  if((IPC6->strpcmFreq!=rate)||(IPC6->strpcmSamples!=spf)||(IPC6->strpcmChannels!=chs)||(IPC6->strpcmFormat!=spfrm)){
    strpcmStop();
    strpcmStart(FastStart,rate,spf,chs,spfrm);
  }
}

#ifdef ExceptMP3
#include "dllsound_internal_mp3.h"
#endif
#ifdef ExceptMIDI
#include "dllsound_internal_midi.h"
#endif
#ifdef ExceptGME
#include "dllsound_internal_gme.h"
#endif
#ifdef ExceptOGG
#include "dllsound_internal_ogg.h"
#endif
#ifdef ExceptWAVE
#include "dllsound_internal_wav.h"
#endif

static void DLLSound_Open_ins_FirstDecode(void)
{
  if(FastStart==false) return;
  
  _consolePrint("First decode...\n");
  DLLSound_UpdateLoop(false);
}

static void InitID3Tag(void)
{
  TID3Tag *pi3t=&ID3Tag;
  pi3t->LinesCount=0;
}

static void FreeID3Tag(void)
{
  TID3Tag *pi3t=&ID3Tag;
  for(u32 idx=0;idx<pi3t->LinesCount;idx++){
    if(pi3t->ppLines[idx]!=NULL){
      safefree(&MM_DLLSound,pi3t->ppLines[idx]); pi3t->ppLines[idx]=NULL;
    }
  }
  pi3t->LinesCount=0;
}

void DLLSound_Open(const char *pFilename)
{
  if(DLLSound_isOpened()==true) DLLSound_Close(true);
  
  DLLSound_SetFirstDecode();
  
  pPluginBody=NULL;
  
  InitID3Tag();
  
  const char *pext=NULL;
  {
    u32 idx=0;
    while(pFilename[idx]!=0){
      if(pFilename[idx]=='.') pext=&pFilename[idx];
      idx++;
    }
  }
  
  PluginBody_FileHandle=FAT2_fopen_AliasForRead(pFilename);
  if(VerboseDebugLog==true) _consolePrintf("PluginBody_FileHandle=%x\n",PluginBody_FileHandle);
  if(PluginBody_FileHandle==NULL){
    _consolePrintf("Data file not found. [%s]\n",pFilename);
    return;
  }
  
  PlugTime_SkipCheck=true;
  PlugTime_TotalSamples=0;
  PlugTime_SamplesDivCount=0;
  PlugTime_CurrentBitRate=0;
  PlugTime_CurrentCount=0;
  
  CanUseGaplessMode=true;
  
#ifdef ExceptMP3
  if(DLLSound_ChkExt_internal_mp3(pext)==true){
    if(DLLSound_Open_internal_mp3(pext)==false) return;
    DLLSound_Open_ins_FirstDecode();
    DTCM_StackCheck(0);
    PrintFreeMem();
    return;
  }
#endif
  
#ifdef ExceptMIDI
  if(DLLSound_ChkExt_internal_midi(pext)==true){
    if(DLLSound_Open_internal_midi(pext)==false) return;
    DLLSound_Open_ins_FirstDecode();
    DTCM_StackCheck(0);
    PrintFreeMem();
    return;
  }
#endif
 
#ifdef ExceptGME
  if(DLLSound_ChkExt_internal_gme(pext)==true){
    if(DLLSound_Open_internal_gme(pext)==false) return;
    DLLSound_Open_ins_FirstDecode();
    DTCM_StackCheck(0);
    PrintFreeMem();
    return;
  }
#endif
  
#ifdef ExceptOGG
  if(DLLSound_ChkExt_internal_ogg(pext)==true){
    if(DLLSound_Open_internal_ogg(pext)==false) return;
    DLLSound_Open_ins_FirstDecode();
    DTCM_StackCheck(0);
    PrintFreeMem();
    return;
  }
#endif
  
#ifdef ExceptWAVE
  if(DLLSound_ChkExt_internal_wave(pext)==true){
    if(DLLSound_Open_internal_wave(pext)==false) return;
    DLLSound_Open_ins_FirstDecode();
    DTCM_StackCheck(0);
    PrintFreeMem();
    return;
  }
#endif
  
  char fn[PluginFilenameMax];
  EPluginType PluginType;
  
  PluginType=DLLList_GetPluginFilename(pext,fn);
  if(PluginType!=EPT_Sound){
    FAT2_fclose(PluginBody_FileHandle); PluginBody_FileHandle=NULL;
    _consolePrintf("This plugin is not sound type. (%s)\n",fn);
    return;
  }
  
  _consolePrintf("DLLSound: Load plugin. [%s] [%s]\n",fn,pext);
  pPluginBody=DLLList_LoadPlugin(PluginType,fn);
  if(pPluginBody==NULL){
    FAT2_fclose(PluginBody_FileHandle); PluginBody_FileHandle=NULL;
    _consolePrint("Plugin load error.\n");
    return;
  }
  DTCM_StackCheck(-1);
  
  _consolePrintf("DLLSound: Start plugin. 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",pPluginBody,pPluginBody->pSL,pPluginBody->pSL->Start,(int)PluginBody_FileHandle);
  DTCM_StackCheck(1);
  if(pPluginBody->pSL->Start((int)PluginBody_FileHandle)==false){
    DLLList_FreePlugin(pPluginBody); pPluginBody=NULL;
    FAT2_fclose(PluginBody_FileHandle); PluginBody_FileHandle=NULL;
    _consolePrint("Plugin start error.\n");
    return;
  }
  DTCM_StackCheck(2);
  
  PrintFreeMem();
  
  u32 rate=pPluginBody->pSL->GetSampleRate();
  u32 spf=pPluginBody->pSL->GetSamplePerFrame();
  u32 chs=pPluginBody->pSL->GetChannelCount();
  
  _consolePrintf("DLLSound: rate=%dHz, spf=%dsamples, chs=%dchs.\n",rate,spf,chs);
  
  _consolePrint("DLLSound: Load general ID3Tag.\n");
  MACRO_LoadGeneralID3Tag(pPluginBody->pMM,pPluginBody->pSL->GetInfoIndexCount,pPluginBody->pSL->GetInfoStrL,pPluginBody->pSL->GetInfoStrUTF8,pPluginBody->pSL->GetInfoStrW);
  
  RateConv.SourceRate=0;
  
  bool f=false;
  
  if((f==false)&&(DLLSound_isComplexDecoder()==true)){
    f=true;
    DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx1);
  }
  
  if((f==false)&&(rate==32768/4)){
    f=true;
    DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx4);
  }
  if((f==false)&&(rate==32768/2)){
    f=true;
    DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx2);
  }
  if((f==false)&&(rate==32768/1)){
    f=true;
    DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx1);
  }
  
  if((f==false)&&(rate<=32768)){
    f=true;
    DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx4);
  }
  
  if(f==false){
    f=true;
    
    TRateConv *prc=&RateConv;
    prc->SourceRate=rate;
    prc->Pos=0;
    prc->Size=0;
    prc->Mod16=0;
    prc->Add16=(rate*0x10000)/32768;
    prc->CountSource=spf;
    prc->Count32768=(spf*32768)/prc->SourceRate;
    if((prc->Count32768&1)!=0) prc->Count32768--;
    prc->pReadBufLR=(u32*)safemalloc_chkmem(&MM_DLLSound,prc->CountSource*3*2*2);
    prc->LastSampleLR=0;
    
    _consolePrintf("Use sampling rate converter.\nsrcrate=%dHz, Add16=0x%04x, Count32768=%dsmps.\n",prc->SourceRate,prc->Add16,prc->Count32768); 
    
    DLLSound_Open_ins_strpcmStart(32768,prc->Count32768,chs,SPF_PCMx1);
  }
  
  DLLSound_Open_ins_FirstDecode();
  
  DTCM_StackCheck(0);
  PrintFreeMem();
  
  MM_CheckOverRange();
}

void DLLSound_Close(bool UseGapless)
{
  if((CanUseGaplessMode==false)||(UseGapless==false)) strpcmStop();
  
  FreeID3Tag();
  
  if(pPluginBody!=NULL){
    DLLList_FreePlugin(pPluginBody); pPluginBody=NULL;
  }
  DTCM_StackCheck(-1);
  
#ifdef ExceptMP3
  if(isMP3==true){
    isMP3=false;
    PlugMP3_Free();
    DTCM_StackCheck(-1);
  }
#endif
#ifdef ExceptMIDI
  if(isMIDI==true){
    isMIDI=false;
    PlugMIDI_Free();
    DTCM_StackCheck(-1);
  }
#endif
#ifdef ExceptGME
  if(isGME==true){
    isGME=false;
    PlugGME_Free();
    DTCM_StackCheck(-1);
  }
#endif
#ifdef ExceptOGG
  if(isOGG==true){
    isOGG=false;
    PlugOGG_Free();
    DTCM_StackCheck(-1);
  }
#endif
#ifdef ExceptWAVE
  if(isWAVE==true){
    isWAVE=false;
    PlugWAVE_Free();
    DTCM_StackCheck(-1);
  }
#endif
  if(PluginBody_FileHandle!=NULL){
    FAT2_fclose(PluginBody_FileHandle); PluginBody_FileHandle=NULL;
  }
  
  TRateConv *prc=&RateConv;
  if(prc->SourceRate!=0){
    if(prc->pReadBufLR!=NULL){
      safefree(&MM_DLLSound,prc->pReadBufLR); prc->pReadBufLR=NULL;
    }
  }
  
  MM_Compact();
  MM_CheckOverRange();
  MM_CheckMemoryLeak(&MM_DLLSound);
}

bool DLLSound_isOpened(void)
{
#ifdef ExceptMP3
  if(isMP3==true) return(true);
#endif
#ifdef ExceptMIDI
  if(isMIDI==true) return(true);
#endif
#ifdef ExceptGME
  if(isGME==true) return(true);
#endif
#ifdef ExceptOGG
  if(isOGG==true) return(true);
#endif
#ifdef ExceptWAVE
  if(isWAVE==true) return(true);
#endif

  if(pPluginBody!=NULL){
    return(true);
    }else{
    return(false);
  }
}

bool DLLSound_isComplexDecoder(void)
{
#ifdef ExceptMP3
  if(isMP3==true) return(false);
#endif
#ifdef ExceptMIDI
  if(isMIDI==true) return(true);
#endif
#ifdef ExceptGME
  if(isGME==true) return(false);
#endif
#ifdef ExceptOGG
  if(isOGG==true) return(false);
#endif
#ifdef ExceptWAVE
  if(isWAVE==true) return(false);
#endif
  if(pPluginBody==NULL) return(false);
  
  return(pPluginBody->pSL->isComplexDecoder());
}

static bool DLLSound_Update(void)
{
  if(DLLSound_isOpened()==false) return(false);
  if(strpcmRequestStop==true) return(false);
  
//  _consolePrint("DLLSound_Update()\n");
  
  u32 BaseSamples=IPC6->strpcmSamples;
  u32 Samples=0;
  
  REG_IME=0;
  
  u32 CurIndex=(strpcmRingBufWriteIndex+1) & strpcmRingBufBitMask;
  u32 PlayIndex=strpcmRingBufReadIndex;
  bool EmptyFlag;
  
  EmptyFlag=strpcmRingEmptyFlag;
  strpcmRingEmptyFlag=false;
  
  REG_IME=1;
  
  if(CurIndex==PlayIndex) return(false);
  
  if(EmptyFlag==true){ cwl();
    _consolePrint("strpcm:CPU overflow.\n");
  }
  
  if(strpcmRingLRBuf==NULL) return(false);
  
  u32 *plrdst=&strpcmRingLRBuf[BaseSamples*CurIndex];
  
#ifdef ExceptMP3
  if(isMP3==true){
    DLLSound_Update_internal_mp3(BaseSamples,plrdst);
    REG_IME=0;
    strpcmRingBufWriteIndex=CurIndex;
    REG_IME=1;
    return(true);
  }
#endif
#ifdef ExceptMIDI
  if(isMIDI==true){
    DLLSound_Update_internal_midi(BaseSamples,plrdst);
    REG_IME=0;
    strpcmRingBufWriteIndex=CurIndex;
    REG_IME=1;
    return(true);
  }
#endif
#ifdef ExceptGME
  if(isGME==true){
    DLLSound_Update_internal_gme(BaseSamples,plrdst);
    REG_IME=0;
    strpcmRingBufWriteIndex=CurIndex;
    REG_IME=1;
    return(true);
  }
#endif
#ifdef ExceptOGG
  if(isOGG==true){
    DLLSound_Update_internal_ogg(BaseSamples,plrdst);
    REG_IME=0;
    strpcmRingBufWriteIndex=CurIndex;
    REG_IME=1;
    return(true);
  }
#endif 
#ifdef ExceptWAVE
  if(isWAVE==true){
    DLLSound_Update_internal_wave(BaseSamples,plrdst);
    REG_IME=0;
    strpcmRingBufWriteIndex=CurIndex;
    REG_IME=1;
    return(true);
  }
#endif 
  if(RateConv.SourceRate==0){
    u32 Samples=0;
    {
      // PrfStart();
      u32 lastpos=pPluginBody->pSL->GetPosOffset();
      Samples=pPluginBody->pSL->Update(plrdst);
      DTCM_StackCheck(-1);
      u32 curpos=pPluginBody->pSL->GetPosOffset();
      if((PlugTime_SkipCheck==false)&&(lastpos<=curpos)){
        PlugTime_TotalSamples+=Samples;
        PlugTime_SamplesDivCount+=curpos-lastpos;
      }
      PlugTime_SkipCheck=false;
      // PrfEnd(curpos);
      if(Samples!=BaseSamples) strpcmRequestStop=true;
    }
    if(Samples<BaseSamples){ cwl();
      for(u32 idx=Samples;idx<BaseSamples;idx++){
        plrdst[idx]=0;
      }
    }
    
    REG_IME=0;
    strpcmRingBufWriteIndex=CurIndex;
    REG_IME=1;
    
    return(true);
  }
  
  {
    TRateConv *prc=&RateConv;
    if(prc->Pos!=0){
      if(prc->Pos<prc->Size){
        MemCopy32CPU(&prc->pReadBufLR[prc->Pos],&prc->pReadBufLR[0],(prc->Size-prc->Pos)*4);
        prc->Size-=prc->Pos;
        prc->Pos=0;
      }
    }
    
    while((((s32)prc->Size-1)-(s32)prc->Pos)<(s32)prc->CountSource){
      { cwl();
        // PrfStart();
        u32 lastpos=pPluginBody->pSL->GetPosOffset();
        u32 Samples=pPluginBody->pSL->Update(&prc->pReadBufLR[prc->Size]);
        DTCM_StackCheck(-1);
        u32 curpos=pPluginBody->pSL->GetPosOffset();
        if((PlugTime_SkipCheck==false)&&(lastpos<=curpos)){
          PlugTime_TotalSamples+=Samples;
          PlugTime_SamplesDivCount+=curpos-lastpos;
        }
        PlugTime_SkipCheck=false;
        prc->Size+=Samples;
        // PrfEnd(curpos);
        if(Samples!=prc->CountSource){
          strpcmRequestStop=true;
          break;
        }
      }
    }
    
    s32 LastSrcSamples=((s32)prc->Size-1)-(s32)prc->Pos;
    
    if(LastSrcSamples<=0){
      Samples=0;
      }else{
      Samples=prc->Count32768;
      
      u32 ConvSamples=(LastSrcSamples*32768)/prc->SourceRate;
      if(ConvSamples<Samples) Samples=ConvSamples;
      
      // PrfStart(); // 270 268 271 266us
      
      u32 *psrcbuflr=&prc->pReadBufLR[prc->Pos];
      u32 *pdstbuflr=plrdst;
      u32 cnt=Samples+1;
      u32 Pos=prc->Pos*4;
      u32 Mod16=prc->Mod16;
      u32 Add16=prc->Add16;
      s32 LastSample=prc->LastSampleLR;
      
      u32 REG_Mask0xffff=0xffff;
      u32 REG_iMod16,REG_tmpsmp,REG_tmpl,REG_tmpr;
	  asm volatile (
			"Linner_Loop2: \n\t"
			"subs %[cnt],%[cnt],#1 \n\t"
			"beq Linner_End2 \n\t"
			" \n\t"
			"mov %[REG_iMod16],#0x10000 \n\t"
			"sub %[REG_iMod16],%[REG_iMod16],%[Mod16] \n\t"
			" \n\t"
			"ldr %[REG_tmpsmp],[%[psrcbuflr],%[Pos]] \n\t"
			"smulwb %[REG_tmpl],%[REG_iMod16],%[LastSample] \n\t"
			"smlawb %[REG_tmpl],%[Mod16],%[REG_tmpsmp],%[REG_tmpl] @ REG_tmpsmp‚Ì‰ºˆÊ16bit‚¾‚¯‰‰ŽZ‚³‚ê‚é‚Í‚¸ \n\t"
			"and %[REG_tmpl],%[REG_tmpl],%[REG_Mask0xffff] \n\t"
			" \n\t"
			"smulwt %[REG_tmpr],%[REG_iMod16],%[LastSample] \n\t"
			"smlawt %[REG_tmpr],%[Mod16],%[REG_tmpsmp],%[REG_tmpr] @ REG_tmpsmp‚ÌãˆÊ16bit‚¾‚¯‰‰ŽZ‚³‚ê‚é‚Í‚¸ \n\t"
			" \n\t"
			"orr %[REG_tmpsmp],%[REG_tmpl],%[REG_tmpr],lsl #16 \n\t"
			"str %[REG_tmpsmp],[%[pdstbuflr]],#4 \n\t"
			" \n\t"
			"add %[Mod16],%[Mod16],%[Add16] \n\t"
			"cmp %[Mod16],#0x10000 \n\t"
			"blo Linner_Loop2 \n\t"
			" \n\t"
			"Linner_Modulation2: \n\t"
			"sub %[Mod16],%[Mod16],#0x10000 \n\t"
			"ldr %[LastSample],[%[psrcbuflr],%[Pos]] \n\t"
			"add %[Pos],%[Pos],#4 \n\t"
			"cmp %[Mod16],#0x10000 \n\t"
			"blo Linner_Loop2 \n\t"
			"b Linner_Modulation2 \n\t"
			" \n\t"
			"Linner_End2: \n\t"
		:[cnt]"+r"(cnt), [REG_iMod16]"+r"(REG_iMod16), [Mod16]"+r"(Mod16), [REG_tmpsmp]"+r"(REG_tmpsmp), [psrcbuflr]"+r"(psrcbuflr), [Pos]"+r"(Pos),
			[REG_tmpl]"+r"(REG_tmpl), [LastSample]"+r"(LastSample), [REG_Mask0xffff]"+r"(REG_Mask0xffff), [REG_tmpr]"+r"(REG_tmpr), [Add16]"+r"(Add16),
			[pdstbuflr]"+r"(pdstbuflr)
		::"memory"
		);

      prc->Pos=Pos/4;
      prc->Mod16=Mod16;
      prc->Add16=Add16;
      prc->LastSampleLR=LastSample;
      
      // PrfEnd(0);
    }
  }
  
  if(Samples<BaseSamples){ cwl();
    for(u32 idx=Samples;idx<BaseSamples;idx++){ cwl();
      plrdst[idx]=0;
    }
  }
  
  REG_IME=0;
  strpcmRingBufWriteIndex=CurIndex;
  REG_IME=1;
  
  return(true);
}

void DLLSound_UpdateLoop(bool OneShot)
{
  bool FirstDecode=DLLSound_GetFirstDecode();
  
  if(FirstDecode==true) PlayList_SetError(true);
  
  if(OneShot==true){
    DLLSound_Update();
    }else{
    u32 vsynccount=VBlankPassedCount;
    while(1){
      if(DLLSound_Update()==false) break;
      if(8<=(VBlankPassedCount-vsynccount)) break; // break over 0.133sec(8x16ms) for UI.
    }
  }
  
  if(FirstDecode==true) PlayList_SetError(false);
}
void DLLSound_WaitForStreamPCM(void)
{
  if(DLLSound_isOpened()==false) return;
  if(strpcmRequestStop==false) return;
  if(PlayList_GetPause()==true) return;
  
  if(CanUseGaplessMode==true){
    _consolePrint("DLLSound_WaitForStreamPCM: Gapless mode.\n");
    return;
  }
  
  _consolePrint("DLLSound_WaitForStreamPCM: Wait for terminate.\n");
  
  while(1){
    vu32 CurIndex=(strpcmRingBufWriteIndex-1) & strpcmRingBufBitMask; // -1‚É‚È‚Á‚Ä‚àƒ}ƒXƒNŠ|‚¯‚é‚©‚ç‘åä•v
    vu32 PlayIndex=strpcmRingBufReadIndex;
    if(CurIndex==PlayIndex) return;
  }
}

void DLLSound_SetVolume64(u32 v)
{
  strpcmSetAudioVolume64(v);
}

u32 DLLSound_GetSampleRate(void)
{
#ifdef ExceptMP3
  if(isMP3==true) return(PlugMP3_GetSampleRate());
#endif
#ifdef ExceptMIDI
  if(isMIDI==true) return(PlugMIDI_GetSampleRate());
#endif
#ifdef ExceptGME
  if(isGME==true) return(PlugGME_GetSampleRate());
#endif
#ifdef ExceptOGG
  if(isOGG==true) return(PlugOGG_GetSampleRate());
#endif
#ifdef ExceptWAVE
  if(isWAVE==true) return(PlugWAVE_GetSampleRate());
#endif
  if(pPluginBody==NULL) return(0);
  return(pPluginBody->pSL->GetSampleRate());
}

void DLLSound_SetFirstDecode(void)
{
  DLLSound_FirstDecode=true;
}

bool DLLSound_GetFirstDecode(void)
{
  bool f=DLLSound_FirstDecode;
  DLLSound_FirstDecode=false;
  return(f);
}

s32 DLLSound_GetPosMax(void)
{
#ifdef ExceptMP3
  if(isMP3==true) return(PlugMP3_GetPosMax());
#endif
#ifdef ExceptMIDI
  if(isMIDI==true) return(PlugMIDI_GetPosMax());
#endif
#ifdef ExceptGME
  if(isGME==true) return(PlugGME_GetPosMax());
#endif
#ifdef ExceptOGG
  if(isOGG==true) return(PlugOGG_GetPosMax());
#endif
#ifdef ExceptWAVE
  if(isWAVE==true) return(PlugWAVE_GetPosMax());
#endif
  if(pPluginBody==NULL) return(0);
  return(pPluginBody->pSL->GetPosMax());
}

s32 DLLSound_GetPosOffset(void)
{
#ifdef ExceptMP3
  if(isMP3==true) return(PlugMP3_GetPosOffset());
#endif
#ifdef ExceptMIDI
  if(isMIDI==true) return(PlugMIDI_GetPosOffset());
#endif
#ifdef ExceptGME
  if(isGME==true) return(PlugGME_GetPosOffset());
#endif
#ifdef ExceptOGG
  if(isOGG==true) return(PlugOGG_GetPosOffset());
#endif
#ifdef ExceptWAVE
  if(isWAVE==true) return(PlugWAVE_GetPosOffset());
#endif
  if(pPluginBody==NULL) return(0);
  return(pPluginBody->pSL->GetPosOffset());
}

void DLLSound_SetPosOffset(s32 pos)
{
  DLLSound_SetFirstDecode();
  
  PlugTime_SkipCheck=true;
  
#ifdef ExceptMP3
  if(isMP3==true){
    PlugMP3_SetPosOffset(pos);
    return;
  }
#endif
#ifdef ExceptMIDI
  if(isMIDI==true){
    PlugMIDI_SetPosOffset(pos);
    return;
  }
#endif
#ifdef ExceptGME
  if(isGME==true){
    PlugGME_SetPosOffset(pos);
    return;
  }
#endif
#ifdef ExceptOGG
  if(isOGG==true){
    PlugOGG_SetPosOffset(pos);
    return;
  }
#endif
#ifdef ExceptWAVE
  if(isWAVE==true){
    PlugWAVE_SetPosOffset(pos);
    return;
  }
#endif
  if(pPluginBody==NULL) return;
  pPluginBody->pSL->SetPosOffset(pos);
}

bool DLLSound_GetTimeSec(u32 *pCurTime,u32 *pPlayTime)
{
  *pCurTime=0;
  *pPlayTime=0;
  
  u32 CurTime=0,PlayTime=0;
  
#ifdef ExceptMP3
  if(isMP3==true){
    PlayTime=PlugMP3_GetPlayTimeSec();
  }
#endif
#ifdef ExceptMIDI
  if(isMIDI==true){
      PlayTime=PlugMIDI_GetPlayTimeSec();
  }
#endif
#ifdef ExceptGME
  if(isGME==true){
      PlayTime=PlugGME_GetPlayTimeSec();
  }
#endif
#ifdef ExceptOGG
  if(isOGG==true){
      PlayTime=PlugOGG_GetPlayTimeSec();
  }
#endif
#ifdef ExceptWAVE
  if(isWAVE==true){
      PlayTime=PlugWAVE_GetPlayTimeSec();
  }
#endif

  if(PlayTime==0){
//    _consolePrintf("%d/%d %d,%d\n",DLLSound_GetPosOffset(),DLLSound_GetPosMax(),PlugTime_TotalSamples,PlugTime_SamplesDivCount);
    if(PlugTime_SamplesDivCount==0) return(0);
    float f=(float)PlugTime_TotalSamples/PlugTime_SamplesDivCount;
    if(PlugTime_CurrentCount<16){
      PlugTime_CurrentCount++;
      PlugTime_CurrentBitRate=f;
      }else{
      PlugTime_CurrentCount=16;
      PlugTime_CurrentBitRate=((PlugTime_CurrentBitRate*(PlugTime_CurrentCount-1))+f)/PlugTime_CurrentCount;
    }
    
    u32 pos=DLLSound_GetPosMax();
    u32 rate=DLLSound_GetSampleRate();
    if(rate==0) return(false);
    if(0x10000<=pos){
      pos/=0x100;
      rate/=0x100;
    }
    PlayTime=(pos*PlugTime_CurrentBitRate)/rate;
  }
  
  if(CurTime==0){
    u32 pos=DLLSound_GetPosOffset();
    u32 max=DLLSound_GetPosMax();
    if(max==0) return(false);
    if(0x10000<=pos){
      pos/=0x100;
      max/=0x100;
    }
    CurTime=(PlayTime*pos)/max;
  }
  
  *pCurTime=CurTime;
  *pPlayTime=PlayTime;
  
  return(true);
}

bool DLLSound_SeekPer(s32 per)
{
  if(DLLSound_isOpened()==false) return(false);
  
  DLLSound_SetFirstDecode();
  
  if(per==0) return(false);
  
  s32 ofs=DLLSound_GetPosOffset(),max=DLLSound_GetPosMax();
  s32 from=ofs;
  
  s32 diff=max/100;
  if(diff==0) diff=1;
  
  diff*=per;
  if(diff==0) diff=per;
  
  ofs+=diff;
  if(ofs<0) ofs=0;
  if(max<ofs) ofs=max;
  if(diff<=0){
    ofs&=~3;
    }else{
    ofs=(ofs+3)&~3;
    if(max<=ofs) ofs=max-1;
    if(ofs<0) ofs=0;
  }
  
//  _consolePrintf("SeekInfo: %d,%d,%d %d,%d\n",from,ofs,max,diff,per);
  
  DLLSound_SetPosOffset(ofs);
  
  return(true);
}


