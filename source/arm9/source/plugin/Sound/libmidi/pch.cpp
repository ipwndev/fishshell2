
#include <stdio.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "fat2.h"
#include "shell.h"
#include "memtool.h"
#include "sndfont.h"

#include "plugin.h"

#include "smidlib.h"

#include "pch.h"

#include "pch_tbl.h"
//static u16 *ppowertbl;
#define POWRANGE (1024)

//static int *psintbl;
#define SINRANGE (120)

//////////////////////////////////////////////////////////////////////

enum EEnvState {EES_None=0,EES_Attack=1,EES_Decay=2,EES_Sustain=3,EES_Release3=4,EES_Release4=5,EES_Release5=6};

typedef struct {
  u32 SampleRate;
  u32 RootFreq; // fix16.16
  u32 MasterVolume;
  u32 FractionStart;
  u32 FractionEnd;
  u32 Length,LoopStart,LoopEnd;
  u32 LoopFlag,s16Flag;
  u32 MasterPanpot;
  u32 Note;
  u32 VibSweep;
  u32 VibRatio;
  int VibDepth;
  bool EnvelopeFlag;
  int EnvRate[6];
  u32 EnvOfs[6];
  u32 PCMOffset;
  void *pData;
} TProgram;

#define ProgramPatchHeaderSize (sizeof(TProgram)-4)

typedef struct {
  u32 UnusedClockCount;
  u32 PatchCount;
  TProgram *pPrg;
} TProgramPatch;

typedef struct {
  u32 FileOffset;
  
  u32 DataOffset[128];
  TProgramPatch *ppData[128];
} TProgramMap;

typedef struct {
  TProgramMap *pPrgMaps[128];
} TVariationMap;

static TVariationMap *pVarMap_Tone=NULL,*pVarMap_Drum=NULL; // 初期化前に参照される可能性があるためNULLで初期化する

typedef struct {
  u32 RefCount;
  u32 PCMFileOffset;
  void *pData;
} TPCMStack;

#define PCMStackCount (256)
static TPCMStack *pPCMStack;

// ----------------------------------------------------

typedef struct {
  bool DrumMode;
  u32 Vol,Exp,Reverb;
  
  bool Pedal;
} TChannelInfo;

#define ChannelInfoCount PCH_ChannelsCount
static TChannelInfo *pChannelInfo;

typedef struct {
  bool Enabled;
  u32 ChannelNum;
  TChannelInfo *pChannelInfo;
  
  u32 Note,Vel;
  
  u32 Panpot;
  
  u32 OnClock,OffClock;
  
  u32 GT;
  
  bool VibEnable;
  u32 VibSweepAdd;
  u32 VibSweepCur;
  u32 VibAdd;
  u32 VibCur;
  int VibDepth;
  int VibPhase;
  
  bool ModEnable;
  u32 ModAdd;
  u32 ModCur;
  int ModDepth;
  int ModPhase;
  
  EEnvState EnvState;
  int EnvSpeed;
  u32 EnvCurLevel;
  u32 EnvEndLevel;
  u32 EnvRelLevel;
  
  TProgramPatch *pPrgPatch;
  
  TProgram *pPrg;
  u32 PrgPos;
  u32 PrgMstVol;
  u32 PrgVol;
  
  u32 FreqAddFix16;
  u32 FreqCurFix16;
  s32 LastSampleData,CurSampleData;
} TPCH;

static const u32 PCHCount=64;
static TPCH *pPCH;

// ----------------------------------------------------

static u32 Note2FreqTableFix16[128];

static u32 SampleRate;

static u32 GenVolume;

bool MemoryOverflowFlag;

static u32 PCMFileOffset;

// ----------------------------------------------------

static TProgramPatch *pLockedProgramPatch=NULL; // 自分自身をメモリマネージャに開放されないように

static void PCH_ChInit(u32 ch);
static void PCH_UnloadProgram(TProgramPatch *pPrgPatch);

static bool safemalloc_CallBack_RequestFreeMemory_PlugMIDI_PCH(void)
{
  if((pVarMap_Tone==NULL)||(pVarMap_Drum==NULL)) return(false);
  
  PCH_IncrementUnusedClockCount();
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if((_pPCH->pPrg!=NULL)&&(_pPCH->FreqAddFix16!=0)){
      _pPCH->pPrgPatch->UnusedClockCount=0;
    }
  }
  
  u32 MaxClock=0;
  
  TVariationMap *pTagVarMap=NULL;
  u32 tagvar=0,tagprg=0;
  
  for(u32 var=0;var<128;var++){
    TVariationMap *pVarMap;
    pVarMap=pVarMap_Tone;
    if(pVarMap!=NULL){
      TProgramMap *pPrgMap=pVarMap->pPrgMaps[var];
      if(pPrgMap!=NULL){
        for(u32 prg=0;prg<128;prg++){
          TProgramPatch *pPrgPatch=pPrgMap->ppData[prg];
          if((pPrgPatch!=NULL)&&(pPrgPatch!=pLockedProgramPatch)){
            if(MaxClock<pPrgPatch->UnusedClockCount){
              MaxClock=pPrgPatch->UnusedClockCount;
              pTagVarMap=pVarMap;
              tagvar=var;
              tagprg=prg;
            }
          }
        }
      }
    }
    pVarMap=pVarMap_Drum;
    if(pVarMap!=NULL){
      TProgramMap *pPrgMap=pVarMap->pPrgMaps[var];
      if(pPrgMap!=NULL){
        for(u32 prg=0;prg<128;prg++){
          TProgramPatch *pPrgPatch=pPrgMap->ppData[prg];
          if((pPrgPatch!=NULL)&&(pPrgPatch!=pLockedProgramPatch)){
            if(MaxClock<pPrgPatch->UnusedClockCount){
              MaxClock=pPrgPatch->UnusedClockCount;
              pTagVarMap=pVarMap;
              tagvar=var;
              tagprg=prg;
            }
          }
        }
      }
    }
  }
  
  if(MaxClock==0){
//    _consolePrint("Not found unused program patch. retry all program patch.\n");
    for(u32 var=0;var<128;var++){
      TVariationMap *pVarMap;
      pVarMap=pVarMap_Tone;
      if(pVarMap!=NULL){
        TProgramMap *pPrgMap=pVarMap->pPrgMaps[var];
        if(pPrgMap!=NULL){
          for(u32 prg=0;prg<128;prg++){
            TProgramPatch *pPrgPatch=pPrgMap->ppData[prg];
            if((pPrgPatch!=NULL)&&(pPrgPatch!=pLockedProgramPatch)){
              MaxClock=1;
              pTagVarMap=pVarMap;
              tagvar=var;
              tagprg=prg;
            }
          }
        }
      }
      pVarMap=pVarMap_Drum;
      if(pVarMap!=NULL){
        TProgramMap *pPrgMap=pVarMap->pPrgMaps[var];
        if(pPrgMap!=NULL){
          for(u32 prg=0;prg<128;prg++){
            TProgramPatch *pPrgPatch=pPrgMap->ppData[prg];
            if((pPrgPatch!=NULL)&&(pPrgPatch!=pLockedProgramPatch)){
              MaxClock=1;
              pTagVarMap=pVarMap;
              tagvar=var;
              tagprg=prg;
            }
          }
        }
      }
    }
    if(MaxClock==0){
      _consolePrint("Not found program patch.\n");
      return(false);
    }
  }
  
//  _consolePrintf("Unload program patch. Unused clock: %dclks. [%d:%d]\n",MaxClock,tagvar,tagprg);
  
  TProgramPatch *pPrgPatch=pTagVarMap->pPrgMaps[tagvar]->ppData[tagprg];
  
  if(pPrgPatch!=NULL){
    for(u32 ch=0;ch<PCHCount;ch++){
      TPCH *_pPCH=&pPCH[ch];
      if(_pPCH->pPrgPatch==pPrgPatch) PCH_ChInit(ch);
    }
    PCH_UnloadProgram(pPrgPatch);
    safefree(&MM_DLLSound,pPrgPatch); pPrgPatch=NULL;
  }
  
  pTagVarMap->pPrgMaps[tagvar]->ppData[tagprg]=NULL;
  
  return(true);
}

// ----------------------------------------------------

static void PCH_InitProgramMap_Load(TProgramMap *pPrgMap,u32 FileOffset)
{
  MemoryOverflowFlag=false;
  
  pPrgMap->FileOffset=FileOffset;
  
  SndFont_SetOffset(pPrgMap->FileOffset);
  
  SndFont_Read16bit(&pPrgMap->DataOffset[0],128*4);
  
  for(u32 idx=0;idx<128;idx++){
    pPrgMap->ppData[idx]=NULL;
  }
}

void PCH_InitProgramMap(void)
{
  pVarMap_Tone=(TVariationMap*)safemalloc_chkmem(&MM_DLLSound,sizeof(TVariationMap));
  pVarMap_Drum=(TVariationMap*)safemalloc_chkmem(&MM_DLLSound,sizeof(TVariationMap));
  
  MemSet32CPU(0,pVarMap_Tone,sizeof(TVariationMap));
  MemSet32CPU(0,pVarMap_Drum,sizeof(TVariationMap));
  
  for(int prg=0;prg<128;prg++){
    pVarMap_Tone->pPrgMaps[prg]=NULL;
    pVarMap_Drum->pPrgMaps[prg]=NULL;
  }
  
  SndFont_SetOffset(0x0);
  
  u32 ID=SndFont_Get32bit();
  
  u32 ToneOffset[128];
  u32 DrumOffset[128];
  
  SndFont_Read16bit(&ToneOffset[0],4*128);
  SndFont_Read16bit(&DrumOffset[0],4*128);
  
  PCMFileOffset=SndFont_Get32bit();
  
  for(int prg=0;prg<128;prg++){
    if(ToneOffset[prg]!=0){
      pVarMap_Tone->pPrgMaps[prg]=(TProgramMap*)safemalloc_chkmem(&MM_DLLSound,sizeof(TProgramMap));
      PCH_InitProgramMap_Load(pVarMap_Tone->pPrgMaps[prg],ToneOffset[prg]);
    }
  }
  
  for(int prg=0;prg<128;prg++){
    if(DrumOffset[prg]!=0){
      pVarMap_Drum->pPrgMaps[prg]=(TProgramMap*)safemalloc_chkmem(&MM_DLLSound,sizeof(TProgramMap));
      PCH_InitProgramMap_Load(pVarMap_Drum->pPrgMaps[prg],DrumOffset[prg]);
    }
  }
  
  pPCMStack=(TPCMStack*)safemalloc_chkmem(&MM_DLLSound,PCMStackCount*sizeof(TPCMStack));
  for(u32 idx=0;idx<PCMStackCount;idx++){
    TPCMStack *_pPCMStack=&pPCMStack[idx];
    _pPCMStack->RefCount=0;
    _pPCMStack->PCMFileOffset=0;
    _pPCMStack->pData=NULL;
  }
  
  safemalloc_CallBack_RequestFreeMemory_PlugSound=safemalloc_CallBack_RequestFreeMemory_PlugMIDI_PCH;
}


static void PCMStack_Unregist(void *p);

static void PCH_FreeProgramMap_Free_Patch(TProgramPatch *pPrgPatch)
{
  if(pPrgPatch->pPrg==NULL) return;
  
  for(u32 patidx=0;patidx<pPrgPatch->PatchCount;patidx++){
    TProgram *pPrg=&pPrgPatch->pPrg[patidx];
    if(pPrg->pData!=NULL){
      PCMStack_Unregist(pPrg->pData); pPrg->pData=NULL;
    }
  }
  
  pPrgPatch->UnusedClockCount=1;
  pPrgPatch->PatchCount=0;
  safefree(&MM_DLLSound,pPrgPatch->pPrg); pPrgPatch->pPrg=NULL;
}

static void PCH_FreeProgramMap_Free(TProgramMap *pPrgMap)
{
  for(u32 idx=0;idx<128;idx++){
    if(pPrgMap->ppData[idx]!=NULL){
      PCH_FreeProgramMap_Free_Patch(pPrgMap->ppData[idx]);
      safefree(&MM_DLLSound,pPrgMap->ppData[idx]); pPrgMap->ppData[idx]=NULL;
    }
  }
}

void PCH_FreeProgramMap(void)
{
  safemalloc_CallBack_RequestFreeMemory_PlugSound=NULL;
  
  if(pVarMap_Tone!=NULL){
    for(u32 idx=0;idx<128;idx++){
      if(pVarMap_Tone->pPrgMaps[idx]!=NULL){
        PCH_FreeProgramMap_Free(pVarMap_Tone->pPrgMaps[idx]);
        safefree(&MM_DLLSound,pVarMap_Tone->pPrgMaps[idx]); pVarMap_Tone->pPrgMaps[idx]=NULL;
      }
    }
    safefree(&MM_DLLSound,pVarMap_Tone); pVarMap_Tone=NULL;
  }
  
  if(pVarMap_Drum!=NULL){
    for(u32 idx=0;idx<128;idx++){
      if(pVarMap_Drum->pPrgMaps[idx]!=NULL){
        PCH_FreeProgramMap_Free(pVarMap_Drum->pPrgMaps[idx]);
        safefree(&MM_DLLSound,pVarMap_Drum->pPrgMaps[idx]); pVarMap_Drum->pPrgMaps[idx]=NULL;
      }
    }
    safefree(&MM_DLLSound,pVarMap_Drum); pVarMap_Drum=NULL;
  }
  
  if(pPCMStack!=NULL){
    bool err=false;
    for(u32 idx=0;idx<PCMStackCount;idx++){
      TPCMStack *_pPCMStack=&pPCMStack[idx];
      if((_pPCMStack->pData!=NULL)||(_pPCMStack->RefCount!=0)){
        _consolePrintf("PCMStack.%d: Ref=%d, ofs=%d, pData=0x%08x.\n",_pPCMStack->RefCount,_pPCMStack->PCMFileOffset,_pPCMStack->pData);
        err=true;
      }
    }
    safefree(&MM_DLLSound,pPCMStack); pPCMStack=NULL;
    if(err==true) StopFatalError(0,"PCMStack: Not free pointer or zero referrer count founded.\n");
  }
}

static void* PCMStack_GetEqual(u32 ofs)
{
  for(u32 idx=0;idx<PCMStackCount;idx++){
    TPCMStack *_pPCMStack=&pPCMStack[idx];
    if(_pPCMStack->RefCount!=0){
      if(_pPCMStack->PCMFileOffset==ofs){
        _pPCMStack->RefCount++;
        return(_pPCMStack->pData);
      }
    }
  }
  
  return(NULL);
}

static void PCMStack_Regist(u32 ofs,void *p)
{
  for(u32 idx=0;idx<PCMStackCount;idx++){
    TPCMStack *_pPCMStack=&pPCMStack[idx];
    if(_pPCMStack->RefCount==0){
      _pPCMStack->RefCount=1;
      _pPCMStack->PCMFileOffset=ofs;
      _pPCMStack->pData=p;
      return;
    }
  }
  
  StopFatalError(0,"PCMStack_Regist: Stack buffer overflow.\n");
}

static void PCMStack_Unregist(void *p)
{
  if(p==NULL) StopFatalError(15503,"PCMStack_Unregist: Can not unregist null pointer.\n");
  
  for(u32 idx=0;idx<PCMStackCount;idx++){
    TPCMStack *_pPCMStack=&pPCMStack[idx];
    if(_pPCMStack->RefCount!=0){
      if(_pPCMStack->pData==p){
        _pPCMStack->RefCount--;
        if(_pPCMStack->RefCount==0){
          safefree(&MM_DLLSound,_pPCMStack->pData); _pPCMStack->pData=NULL;
        }
        return;
      }
    }
  }
  
  StopFatalError(15504,"PCMStack_Unregist: Not found target in stack list.\n");
}

static u32 GetVariationNumber(TVariationMap *pVarMap,u32 VarNum,u32 PrgNum,bool DrumMode)
{
  if(128<=VarNum) return((u32)-1);
  if(128<=PrgNum) return((u32)-1);
  
  for(;VarNum!=0;VarNum--){
    if(pVarMap->pPrgMaps[VarNum]!=NULL){
      TProgramMap *pPrgMap=pVarMap->pPrgMaps[VarNum];
      if(pPrgMap!=NULL){
        u32 ofs=pPrgMap->DataOffset[PrgNum];
        if(ofs!=0) return(VarNum);
      }
      break;
    }
    if(DrumMode==true) break;
  }
  
  VarNum=0;
  
  {
    TProgramMap *pPrgMap=pVarMap->pPrgMaps[VarNum];
    if(pPrgMap!=NULL){
      if(DrumMode==false){
        return(VarNum);
        }else{
        if(pPrgMap->DataOffset[PrgNum]!=0) return(VarNum);
      }
    }
  }
  
  return((u32)-1);
}

static TProgramPatch *GetProgramPatchPtr(TVariationMap *pVarMap,u32 VarNum,u32 PrgNum,bool DrumMode)
{
  if(128<=VarNum) return(NULL);
  if(128<=PrgNum) return(NULL);
  
  for(;VarNum!=0;VarNum--){
    if(pVarMap->pPrgMaps[VarNum]!=NULL){
      TProgramMap *pPrgMap=pVarMap->pPrgMaps[VarNum];
      if(pPrgMap!=NULL){
        if(pPrgMap->ppData[PrgNum]!=NULL) return(pPrgMap->ppData[PrgNum]);
      }
      break;
    }
    if(DrumMode==true) break;
  }
  
  VarNum=0;
  
  {
    TProgramMap *pPrgMap=pVarMap->pPrgMaps[VarNum];
    if(pPrgMap!=NULL){
      if(DrumMode==false){
        if(pPrgMap->ppData[PrgNum]!=NULL) return(pPrgMap->ppData[PrgNum]);
        PrgNum&=~7; // find captital tone
        if(pPrgMap->ppData[PrgNum]!=NULL) return(pPrgMap->ppData[PrgNum]);
        }else{
        if(pPrgMap->ppData[PrgNum]!=NULL) return(pPrgMap->ppData[PrgNum]);
      }
    }
  }
  
  return(NULL);
}

static TProgram *GetProgramFromPatchPtr(TProgramPatch *pPrgPatch,s32 Note,bool DrumMode)
{
//  return(pPrgPatch->pPrg[0]);
  
  if(pPrgPatch==NULL) return(NULL);
  if(128<=Note) return(NULL);
  
  if(DrumMode==true) return(&pPrgPatch->pPrg[0]);
  
  if(pPrgPatch->PatchCount==0) return(NULL);
  if(pPrgPatch->PatchCount==1) return(&pPrgPatch->pPrg[0]);
  
  u32 NoteFreq=Note2FreqTableFix16[Note];
  
  u32 NearPatchIndex=0x100;
  u32 NearFreq=0xffffffff;
  
  for(u32 patidx=0;patidx<pPrgPatch->PatchCount;patidx++){
    u32 RootFreq=pPrgPatch->pPrg[patidx].RootFreq;
    
    u32 sub;
    if(NoteFreq<RootFreq){
      sub=RootFreq-NoteFreq;
      }else{
      sub=NoteFreq-RootFreq;
    }
    
    if(sub<=NearFreq){
      NearFreq=sub;
      NearPatchIndex=patidx;
    }
    
//    _consolePrintf("%d,%d,%d,%d,%d\n",patidx,NoteFreq>>16,NearFreq>>16,RootFreq>>16,NearPatchIndex);
  }
  
  if(NearPatchIndex==0x100){
    _consolePrint("find near root freq from patch list??\n");
    return(NULL);
  }
  
//  _consolePrintf("near %4d,%4d,%d\n",NoteFreq>>16,NearFreq>>16,NearPatchIndex);
  
  return(&pPrgPatch->pPrg[NearPatchIndex]);
}

bool PCH_LoadProgram(s32 Note,u32 var,u32 prg,bool DrumMode)
{
  if(MemoryOverflowFlag==true) return(false);
  
  if(Note==0) return(false);
  
  TVariationMap *pVarMap;
  u32 VarNum,PrgNum;
  
  if(DrumMode==true){
    pVarMap=pVarMap_Drum;
    VarNum=prg;
    PrgNum=Note;
    }else{
    pVarMap=pVarMap_Tone;
    VarNum=var;
    PrgNum=prg;
  }
  
  VarNum=GetVariationNumber(pVarMap,VarNum,PrgNum,DrumMode);
  if(VarNum==(u32)-1) return(false);
  
  TProgramMap *pPrgMap=pVarMap->pPrgMaps[VarNum];
  if(pPrgMap==NULL) return(false);
  
  if(pPrgMap->ppData[PrgNum]!=NULL) return(true); // already loaded.
  
  u32 ofs=pPrgMap->DataOffset[PrgNum];
  
  if(ofs==0) return(false);
  
/*
  if(DrumMode==true){
    _consolePrintf("LoadDrum:%d,%d->%d,%d\n",prg,Note,VarNum,PrgNum);
    }else{
    _consolePrintf("LoadTone:%d,%d->%d,%d\n",var,prg,VarNum,PrgNum);
  }
*/
  
  pPrgMap->ppData[PrgNum]=(TProgramPatch*)safemalloc(&MM_DLLSound,sizeof(TProgramPatch));
  if(pPrgMap->ppData[PrgNum]==NULL){
    MemoryOverflowFlag=true;
    _consolePrintf("malloc(%d); out of memory!!\n",sizeof(TProgramPatch));
    return(false);
  }
  
  TProgramPatch *pPrgPatch=pPrgMap->ppData[PrgNum];
  pLockedProgramPatch=pPrgPatch;
  
  pPrgPatch->UnusedClockCount=0;
  
  SndFont_SetOffset(pPrgMap->FileOffset+ofs);
  
  u32 patcount=SndFont_Get32bit();
  pPrgPatch->PatchCount=patcount;
  
  pPrgPatch->pPrg=(TProgram*)safemalloc(&MM_DLLSound,sizeof(TProgram)*patcount);
  if(pPrgPatch->pPrg==NULL){
    MemoryOverflowFlag=true;
    _consolePrintf("PatchLoad malloc(%d); out of memory!!\n",sizeof(TProgram)*patcount);
    pPrgPatch->PatchCount=0;
    safefree(&MM_DLLSound,pPrgMap->ppData[PrgNum]); pPrgMap->ppData[PrgNum]=NULL;
    pLockedProgramPatch=NULL;
    return(false);
  }
  
  for(u32 patidx=0;patidx<patcount;patidx++){
    pPrgPatch->pPrg[patidx].pData=NULL;
  }
  
  bool memoryoverflow=false;
  
  for(u32 patidx=0;patidx<patcount;patidx++){
    TProgram *pPrg=&pPrgPatch->pPrg[patidx];
    SndFont_Read16bit(pPrg,ProgramPatchHeaderSize);
    
    if(pPrg->s16Flag==1){
      pPrg->Length/=2;
      pPrg->LoopStart/=2;
      pPrg->LoopEnd/=2;
    }
    
    pPrg->VibSweep=pPrg->VibSweep/120;
  
    if(true){ // fast decay
      for(int idx=0;idx<6;idx++){
        pPrg->EnvRate[idx]=(pPrg->EnvRate[idx]*3)/2;
      }
    }
    
    if(false){
      _consolePrint("EnvRate=");
      for(int idx=0;idx<6;idx++){
        _consolePrintf("%02x,",pPrg->EnvRate[idx]);
      }
      _consolePrint("\n");
      _consolePrint("EnvOfs =");
      for(int idx=0;idx<6;idx++){
        _consolePrintf("%02x,",pPrg->EnvOfs[idx]);
      }
      _consolePrint("\n");
    }
    
    for(int idx=0;idx<6;idx++){
      pPrg->EnvRate[idx]*=96;
    }
    
    s32 f=((s32)pPrg->FractionStart)-((s32)pPrg->FractionEnd);
    if(f<0){
      if(pPrg->LoopStart!=0) pPrg->LoopStart--;
      f+=0x10;
    }
    pPrg->FractionStart=((u32)f)<<12;
  }
  
  for(u32 patidx=0;patidx<patcount;patidx++){
    TProgram *pPrg=&pPrgPatch->pPrg[patidx];
    
    {
      void *p=PCMStack_GetEqual(pPrg->PCMOffset);
      if(p!=NULL){
        pPrg->pData=p;
        continue;
      }
    }
    
    SndFont_SetOffset(PCMFileOffset+pPrg->PCMOffset);
    
#define WAVEID (0x45564157)
#define TTACID (0x43415454)

    u32 ID=SndFont_Get32bit();
    
    switch(ID){
      case WAVEID: {
        u32 datasize=SndFont_Get32bit();
        
        pPrg->pData=safemalloc(&MM_DLLSound,datasize);
        if(pPrg->pData==NULL){
          memoryoverflow=true;
          break;
        }
        
//        _consolePrintf("Read RAWPCM. %dbytes\n",datasize);
        SndFont_Read16bit(pPrg->pData,1*datasize);
      } break;
      case TTACID: {
        StopFatalError(15505,"Not support TTA compressed sound font.\n");
      } break;
      default: {
        StopFatalError(15506,"Unknown WAVE format.\n");
      } break;
    }
    
    if(memoryoverflow==true) break;
    
    PCMStack_Regist(pPrg->PCMOffset,pPrg->pData);
  }
  
  if(memoryoverflow==true){
    MemoryOverflowFlag=true;
    _consolePrint("PatchLoad out of memory!!\n");
    if(pPrgPatch->pPrg!=NULL){
      for(u32 patidx=0;patidx<pPrgPatch->PatchCount;patidx++){
        TProgram *pPrg=&pPrgPatch->pPrg[patidx];
        if(pPrg->pData!=NULL) PCMStack_Unregist(pPrg->pData);
      }
      safefree(&MM_DLLSound,pPrgPatch->pPrg); pPrgPatch->pPrg=NULL;
    }
    pPrgPatch->PatchCount=0;
    safefree(&MM_DLLSound,pPrgMap->ppData[PrgNum]); pPrgMap->ppData[PrgNum]=NULL;
    
    pLockedProgramPatch=NULL;
    return(false);
  }
  
  pLockedProgramPatch=NULL;
  return(true);
}

static void PCH_UnloadProgram(TProgramPatch *pPrgPatch)
{
  if(pPrgPatch==NULL) StopFatalError(15507,"PCH_UnloadProgram: is NULL.\n");
  
  if(pPrgPatch->pPrg!=NULL){
    for(u32 patidx=0;patidx<pPrgPatch->PatchCount;patidx++){
      TProgram *pPrg=&pPrgPatch->pPrg[patidx];
      PCMStack_Unregist(pPrg->pData); pPrg->pData=NULL;
    }
    safefree(&MM_DLLSound,pPrgPatch->pPrg); pPrgPatch->pPrg=NULL;
  }
  pPrgPatch->PatchCount=0;
}

static void PCH_ChannelInfoInit(u32 ch)
{
  TChannelInfo *_pCI=&pChannelInfo[ch];
  
  _pCI->DrumMode=false;
  _pCI->Vol=0;
  _pCI->Exp=0;
  _pCI->Reverb=0;
  _pCI->Pedal=false;
}

static void PCH_ChInit(u32 ch)
{
  TPCH *_pPCH=&pPCH[ch];
  
  _pPCH->Enabled=false;
  
  _pPCH->ChannelNum=(u32)0;
  _pPCH->pChannelInfo=&pChannelInfo[_pPCH->ChannelNum];
  _pPCH->Note=0;
  _pPCH->Vel=0;
  
  _pPCH->Panpot=0;
  
  _pPCH->OnClock=0;
  _pPCH->OffClock=0;
  
  _pPCH->GT=0;
  
  _pPCH->VibEnable=false;
  _pPCH->VibSweepAdd=0;
  _pPCH->VibSweepCur=0;
  _pPCH->VibAdd=0;
  _pPCH->VibCur=0;
  _pPCH->VibDepth=0;
  _pPCH->VibPhase=0;
  
  _pPCH->ModEnable=false;
  _pPCH->ModAdd=0;
  _pPCH->ModCur=0;
  _pPCH->ModDepth=0;
  _pPCH->ModPhase=0;
  
  _pPCH->EnvState=EES_None;
  _pPCH->EnvSpeed=0;
  _pPCH->EnvCurLevel=0;
  _pPCH->EnvEndLevel=0;
  _pPCH->EnvRelLevel=0;
  
  _pPCH->pPrgPatch=NULL;
  
  _pPCH->pPrg=NULL;
  _pPCH->PrgPos=0;
  _pPCH->PrgMstVol=0;
  _pPCH->PrgVol=0;
  
  _pPCH->FreqAddFix16=0;
  _pPCH->FreqCurFix16=0;
  
  _pPCH->LastSampleData=0;
  _pPCH->CurSampleData=0;
}

bool PCH_Init(u32 _SampleRate,u32 _GenVolume)
{
  SampleRate=_SampleRate;
  GenVolume=_GenVolume;
  
  {
    pChannelInfo=(TChannelInfo*)safemalloc_chkmem(&MM_DLLSound,ChannelInfoCount*sizeof(TChannelInfo));
    for(u32 ch=0;ch<ChannelInfoCount;ch++){
      PCH_ChannelInfoInit(ch);
    }
  }
  
  {
    pPCH=(TPCH*)safemalloc_chkmem(&MM_DLLSound,PCHCount*sizeof(TPCH));
    for(u32 ch=0;ch<PCHCount;ch++){
      PCH_ChInit(ch);
    }
  }
  
  for(int idx=0;idx<128;idx++){
    s32 Note=idx-1;
    
    Note-=68; // NoteA4
    
    u32 basefreq=440;
    
    while(Note<0){
      basefreq/=2;
      Note+=12;
    }
    
    while(12<=Note){
      basefreq*=2;
      Note-=12;
    }
    
    Note2FreqTableFix16[idx]=(u32)(basefreq*(0x10000+(u32)powertbl[(Note*POWRANGE)]));
  }
  
  return(true);
}

void PCH_Free(void)
{
  SampleRate=0;
  
  if(pChannelInfo!=NULL){
    safefree(&MM_DLLSound,pChannelInfo); pChannelInfo=NULL;
  }
  
  if(pPCH!=NULL){
    safefree(&MM_DLLSound,pPCH); pPCH=NULL;
  }
}

void PCH_AllSoundOff(void)
{
  for(u32 ch=0;ch<PCHCount;ch++){
    PCH_ChInit(ch);
  }
}

void PCH_AllNoteOff(u32 trk)
{
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if(_pPCH->ChannelNum==trk) _pPCH->Enabled=false;
  }
}

void PCH_NextClock(void)
{
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if(_pPCH->Enabled==true){
    }
  }
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if(_pPCH->Enabled==true){
      _pPCH->OnClock++;
      if(_pPCH->pChannelInfo->DrumMode==true){
        if((120*1.5)==_pPCH->OnClock){
          if(_pPCH->EnvState!=EES_Release5){
            _pPCH->EnvRelLevel=_pPCH->EnvCurLevel;
            _pPCH->EnvState=EES_Release5;
            _pPCH->EnvSpeed=-0x100;//_pPCH->pPrg->EnvRate[5];
            _pPCH->EnvEndLevel=0;
          }
        }
      }
      }else{
      if(_pPCH->pChannelInfo->Pedal==true){
        }else{
        _pPCH->OffClock++;
        if((120*2)==_pPCH->OffClock) PCH_ChInit(ch); // exclusive stop when 2sec over.
      }
    }
  }
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if(_pPCH->VibEnable==true){
      _pPCH->VibCur+=_pPCH->VibAdd;
      while((120*0x10000)<=_pPCH->VibCur){
        _pPCH->VibCur-=120*0x10000;
      }
      _pPCH->VibPhase=sintbl[_pPCH->VibCur>>16]*_pPCH->VibDepth/0x10000;
      if(_pPCH->VibSweepCur!=0x10000){
        _pPCH->VibSweepCur+=_pPCH->VibSweepAdd;
        if(_pPCH->VibSweepCur<0x10000){
          s32 Factor=_pPCH->VibSweepCur>>8;
          _pPCH->VibPhase=(_pPCH->VibPhase*Factor)/0x100;
          }else{
          _pPCH->VibSweepCur=0x10000;
        }
      }
    }
  }
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if(_pPCH->ModEnable==true){
      _pPCH->ModCur+=_pPCH->ModAdd;
      while((120*0x10000)<=_pPCH->ModCur){
        _pPCH->ModCur-=120*0x10000;
      }
      _pPCH->ModPhase=sintbl[_pPCH->ModCur>>16]*_pPCH->ModDepth/0x10000;
    }
  }
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    switch(_pPCH->EnvState){
      case EES_None: {
      } break;
      case EES_Attack: {
        bool next=false;
//        _consolePrintf("a%x\n",_pPCH->EnvCurLevel);
        
        if(_pPCH->EnvSpeed==0){
          next=true;
          }else{
          int nextlevel=(int)_pPCH->EnvCurLevel+_pPCH->EnvSpeed;
          
          if((_pPCH->EnvSpeed<0)&&(nextlevel<(int)_pPCH->EnvEndLevel)) next=true;
          if((0<_pPCH->EnvSpeed)&&((int)_pPCH->EnvEndLevel<nextlevel)) next=true;
          
          _pPCH->EnvCurLevel=(u32)nextlevel;
        }
        if(next==true){
          _pPCH->EnvState=EES_Decay;
          _pPCH->EnvCurLevel=_pPCH->EnvEndLevel;
          _pPCH->EnvSpeed=_pPCH->pPrg->EnvRate[1];
          _pPCH->EnvEndLevel=_pPCH->pPrg->EnvOfs[1];
          if(_pPCH->EnvEndLevel<_pPCH->EnvCurLevel) _pPCH->EnvSpeed=-_pPCH->EnvSpeed;
        }
        _pPCH->PrgVol=_pPCH->PrgMstVol*_pPCH->EnvCurLevel/0x10000;
      } break;
      case EES_Decay: {
        bool next=false;
//        _consolePrintf("d%x\n",_pPCH->EnvCurLevel);
        
        if(_pPCH->EnvSpeed==0){
          next=true;
          }else{
          int nextlevel=(int)_pPCH->EnvCurLevel+_pPCH->EnvSpeed;
          
          if((_pPCH->EnvSpeed<0)&&(nextlevel<(int)_pPCH->EnvEndLevel)) next=true;
          if((0<_pPCH->EnvSpeed)&&((int)_pPCH->EnvEndLevel<nextlevel)) next=true;
          
          _pPCH->EnvCurLevel=(u32)nextlevel;
        }
        if(next==true){
          if(true){ // if(_pPCH->DrumMode==false){
            _pPCH->EnvState=EES_Sustain;
            _pPCH->EnvCurLevel=_pPCH->EnvEndLevel;
            _pPCH->EnvSpeed=_pPCH->pPrg->EnvRate[2];
            _pPCH->EnvEndLevel=_pPCH->pPrg->EnvOfs[2];
            if(_pPCH->EnvEndLevel<_pPCH->EnvCurLevel) _pPCH->EnvSpeed=-_pPCH->EnvSpeed;
            }else{
            _pPCH->EnvRelLevel=_pPCH->EnvCurLevel;
            _pPCH->EnvState=EES_Release3;
            _pPCH->EnvSpeed=-_pPCH->pPrg->EnvRate[3];
            _pPCH->EnvEndLevel=(_pPCH->pPrg->EnvOfs[3]*_pPCH->EnvRelLevel)>>16;
          }
        }
        _pPCH->PrgVol=_pPCH->PrgMstVol*_pPCH->EnvCurLevel/0x10000;
      } break;
      case EES_Sustain: {
//        _consolePrintf("s%x\n",_pPCH->EnvCurLevel);
        bool next=false;
        
        if(_pPCH->EnvSpeed==0){
          next=true;
          }else{
          int nextlevel=(int)_pPCH->EnvCurLevel+_pPCH->EnvSpeed;
          
          if((_pPCH->EnvSpeed<0)&&(nextlevel<(int)_pPCH->EnvEndLevel)) next=true;
          if((0<_pPCH->EnvSpeed)&&((int)_pPCH->EnvEndLevel<nextlevel)) next=true;
          
          _pPCH->EnvCurLevel=(u32)nextlevel;
        }
        if(next==true){
          _pPCH->EnvState=EES_None;
          _pPCH->EnvCurLevel=_pPCH->EnvEndLevel;
          _pPCH->EnvSpeed=0;
          _pPCH->EnvEndLevel=0;
        }
        _pPCH->PrgVol=_pPCH->PrgMstVol*_pPCH->EnvCurLevel/0x10000;
      } break;
      case EES_Release3: {
        bool next=false;
//        _consolePrintf("d%x\n",_pPCH->EnvCurLevel);
        
        if(_pPCH->EnvSpeed==0){
          next=true;
          }else{
          int nextlevel=(int)_pPCH->EnvCurLevel+_pPCH->EnvSpeed;
          
          if(nextlevel<(int)_pPCH->EnvEndLevel) next=true;
          
          _pPCH->EnvCurLevel=(u32)nextlevel;
        }
        if(next==true){
          _pPCH->EnvState=EES_Release4;
          _pPCH->EnvCurLevel=_pPCH->EnvEndLevel;
          _pPCH->EnvSpeed=-_pPCH->pPrg->EnvRate[4];
          _pPCH->EnvEndLevel=(_pPCH->pPrg->EnvOfs[4]*_pPCH->EnvRelLevel)>>16;
        }
        _pPCH->PrgVol=_pPCH->PrgMstVol*_pPCH->EnvCurLevel/0x10000;
      } break;
      case EES_Release4: {
        bool next=false;
//        _consolePrintf("d%x\n",_pPCH->EnvCurLevel);
        
        if(_pPCH->EnvSpeed==0){
          next=true;
          }else{
          int nextlevel=(int)_pPCH->EnvCurLevel+_pPCH->EnvSpeed;
          
          if(nextlevel<(int)_pPCH->EnvEndLevel) next=true;
          
          _pPCH->EnvCurLevel=(u32)nextlevel;
        }
        if(next==true){
          _pPCH->EnvState=EES_Release5;
          _pPCH->EnvCurLevel=_pPCH->EnvEndLevel;
          _pPCH->EnvSpeed=-_pPCH->pPrg->EnvRate[5];
//          _pPCH->EnvEndLevel=(_pPCH->pPrg->EnvOfs[5]*_pPCH->EnvRelLevel)>>16;
          _pPCH->EnvEndLevel=0;
        }
        _pPCH->PrgVol=_pPCH->PrgMstVol*_pPCH->EnvCurLevel/0x10000;
      } break;
      case EES_Release5: {
        bool next=false;
//        _consolePrintf("r%x\n",_pPCH->EnvCurLevel);
        
        if(_pPCH->EnvSpeed==0){
          next=true;
          }else{
          int nextlevel=(int)_pPCH->EnvCurLevel+_pPCH->EnvSpeed;
          
          if(nextlevel<(int)_pPCH->EnvEndLevel) next=true;
          
          _pPCH->EnvCurLevel=(u32)nextlevel;
        }
        if(next==true){
          PCH_ChInit(ch);
          }else{
          _pPCH->PrgVol=_pPCH->PrgMstVol*_pPCH->EnvCurLevel/0x10000;
        }
      } break;
    }
    if(EES_Release3<=_pPCH->EnvState){
      if(_pPCH->EnvCurLevel<0x100) PCH_ChInit(ch);
    }
  }
  
}

void PCH_ChangeVolume(u32 trk,u32 v)
{
  if(ChannelInfoCount<=trk){
    _consolePrintf("Channel overflow. ch=%d",trk);
    return;
  }
  
  TChannelInfo *_pCI=&pChannelInfo[trk];
  _pCI->Vol=v;
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if((_pPCH->Enabled==true)&&(_pPCH->ChannelNum==trk)){
      u32 vol;
      
      vol=(_pCI->Vol*_pCI->Exp*_pPCH->Vel*_pPCH->pPrg->MasterVolume)/(128*128*128*128/128);
      vol=vol*GenVolume/128;
      _pPCH->PrgMstVol=vol;
      if(_pPCH->pPrg->EnvelopeFlag==false){
        _pPCH->PrgVol=vol;
        }else{
        _pPCH->PrgVol=vol*_pPCH->EnvCurLevel/0x10000;
      }
    }
  }
}

void PCH_ChangeExpression(u32 trk,u32 e)
{
  if(ChannelInfoCount<=trk){
    _consolePrintf("Channel overflow. ch=%d",trk);
    return;
  }
  
  TChannelInfo *_pCI=&pChannelInfo[trk];
  _pCI->Exp=e;
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if((_pPCH->Enabled==true)&&(_pPCH->ChannelNum==trk)){
      u32 vol;
      
      vol=(_pCI->Vol*_pCI->Exp*_pPCH->Vel*_pPCH->pPrg->MasterVolume)/(128*128*128*128/128);
      vol=vol*GenVolume/128;
      _pPCH->PrgMstVol=vol;
      if(_pPCH->pPrg->EnvelopeFlag==false){
        _pPCH->PrgVol=vol;
        }else{
        _pPCH->PrgVol=vol*_pPCH->EnvCurLevel/0x10000;
      }
    }
  }
}

static inline u32 FreqTransAddFix16(u32 basefreq,u32 powval,u32 SrcSampleRate,u32 RootFreq)
{
  // basefreq is unsigned 15bit
  // (0x10000+powval) is unsigned 17bit
  // SrcSampleRate is unsigned 16bit
  // RootFreq is unsigned fix16.16bit
  // SampleRate is unsigned 16bit
  
  float f=basefreq*(0x10000+powval);
  f=(f*(SrcSampleRate*0x10000))/RootFreq;
  f/=SampleRate;
  
  return((u32)f);
}

void PCH_ChangePitchBend(u32 trk,s32 Pitch)
{
  if(ChannelInfoCount<=trk){
    _consolePrintf("Channel overflow. ch=%d",trk);
    return;
  }
  
//  TChannelInfo *_pCI=&pChannelInfo[trk];
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if((_pPCH->Enabled==true)&&(_pPCH->ChannelNum==trk)){
      TProgram *pPrg=_pPCH->pPrg;
      s32 Note=_pPCH->Note;
      
      s32 TunePitch=Pitch;
      
      Note-=1;
      
      Note-=68; // NoteA4
      
      while(TunePitch<0){
        Note--;
        TunePitch+=POWRANGE;
      }
      
      while(POWRANGE<=TunePitch){
        Note++;
        TunePitch-=POWRANGE;
      }
      
      u32 basefreq=440;
      
      while(Note<0){
        basefreq/=2;
        Note+=12;
      }
      
      while(12<=Note){
        basefreq*=2;
        Note-=12;
      }
      
      _pPCH->FreqAddFix16=FreqTransAddFix16(basefreq,(u32)powertbl[(Note*POWRANGE)+TunePitch],pPrg->SampleRate,pPrg->RootFreq);
    }
  }
}

void PCH_ChangePanpot(u32 trk,u32 p)
{
  if(ChannelInfoCount<=trk){
    _consolePrintf("Channel overflow. ch=%d",trk);
    return;
  }
  
//  TChannelInfo *_pCI=&pChannelInfo[trk];
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if((_pPCH->Enabled==true)&&(_pPCH->ChannelNum==trk)){
      _pPCH->Panpot=p;
    }
  }
}

void PCH_ChangeModLevel(u32 trk,u32 ModLevel)
{
  if(ModLevel==0){
    for(u32 ch=0;ch<PCHCount;ch++){
      TPCH *_pPCH=&pPCH[ch];
      if((_pPCH->Enabled==true)&&(_pPCH->ChannelNum==trk)){
        _pPCH->ModEnable=false;
      }
    }
    }else{
    for(u32 ch=0;ch<PCHCount;ch++){
      TPCH *_pPCH=&pPCH[ch];
      if((_pPCH->Enabled==true)&&(_pPCH->ChannelNum==trk)){
        _pPCH->ModEnable=true;
        _pPCH->ModDepth=ModLevel*10; // (128*10)/0x10000 per
        _pPCH->ModDepth=(int)(_pPCH->FreqAddFix16*_pPCH->ModDepth/0x10000);
      }
    }
  }
}

static int PCH_FindEqualChannel(u32 trk,u32 Note)
{
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if(_pPCH->Enabled==true){
      if((_pPCH->ChannelNum==trk)&&(_pPCH->Note==Note)){
        return(ch);
      }
    }
  }
  
  return(-1);
}

static int PCH_FindDisableChannel(u32 PlayChOnly)
{
  bool FindCh[PCHCount];
  if(PlayChOnly==false){
    for(u32 ch=0;ch<PCHCount;ch++){
      FindCh[ch]=true;
    }
    }else{
    for(u32 ch=0;ch<PCHCount;ch++){
      TPCH *_pPCH=&pPCH[ch];
      if(_pPCH->pPrg!=NULL){
        FindCh[ch]=true;
        }else{
        FindCh[ch]=false;
      }
    }
  }
  
  for(u32 ch=0;ch<PCHCount;ch++){
    if(FindCh[ch]==true){
      TPCH *_pPCH=&pPCH[ch];
      if(_pPCH->FreqAddFix16==0) return(ch);
    }
  }
  
  {
    bool FindedDisabledCh=false;
    for(u32 ch=0;ch<PCHCount;ch++){
      if(FindCh[ch]==true){
        TPCH *_pPCH=&pPCH[ch];
        if((_pPCH->Enabled==false)&&(_pPCH->pChannelInfo->DrumMode==true)){
          FindedDisabledCh=true;
        }
      }
    }
    
    if(FindedDisabledCh==true){
      u32 MaxClock=0;
      for(u32 ch=0;ch<PCHCount;ch++){
        if(FindCh[ch]==true){
          TPCH *_pPCH=&pPCH[ch];
          if((_pPCH->Enabled==false)&&(_pPCH->pChannelInfo->DrumMode==true)){
            if(MaxClock<_pPCH->OffClock) MaxClock=_pPCH->OffClock;
          }
        }
      }
      for(u32 ch=0;ch<PCHCount;ch++){
        if(FindCh[ch]==true){
          TPCH *_pPCH=&pPCH[ch];
          if((_pPCH->Enabled==false)&&(_pPCH->pChannelInfo->DrumMode==true)){
            if(MaxClock==_pPCH->OffClock) return(ch);
          }
        }
      }
      MaxClock=0;
      for(u32 ch=0;ch<PCHCount;ch++){
        if(FindCh[ch]==true){
          TPCH *_pPCH=&pPCH[ch];
          if((_pPCH->Enabled==true)&&(_pPCH->pChannelInfo->DrumMode==true)){
            if(MaxClock<_pPCH->OnClock) MaxClock=_pPCH->OnClock;
          }
        }
      }
      for(u32 ch=0;ch<PCHCount;ch++){
        if(FindCh[ch]==true){
          TPCH *_pPCH=&pPCH[ch];
          if((_pPCH->Enabled==true)&&(_pPCH->pChannelInfo->DrumMode==true)){
            if(MaxClock==_pPCH->OnClock) return(ch);
          }
        }
      }
    }
  }
  
  {
    bool FindedDisabledCh=false;
    for(u32 ch=0;ch<PCHCount;ch++){
      if(FindCh[ch]==true){
        TPCH *_pPCH=&pPCH[ch];
        if(_pPCH->Enabled==false){
          FindedDisabledCh=true;
        }
      }
    }
    
    if(FindedDisabledCh==true){
      u32 MaxClock=0;
      for(u32 ch=0;ch<PCHCount;ch++){
        if(FindCh[ch]==true){
          TPCH *_pPCH=&pPCH[ch];
          if(_pPCH->Enabled==false){
            if(MaxClock<_pPCH->OffClock) MaxClock=_pPCH->OffClock;
          }
        }
      }
      for(u32 ch=0;ch<PCHCount;ch++){
        if(FindCh[ch]==true){
          TPCH *_pPCH=&pPCH[ch];
          if(_pPCH->Enabled==false){
            if(MaxClock==_pPCH->OffClock) return(ch);
          }
        }
      }
    }
  }
  
  {
    u32 MaxClock=0;
    for(u32 ch=0;ch<PCHCount;ch++){
      if(FindCh[ch]==true){
        TPCH *_pPCH=&pPCH[ch];
        if(_pPCH->Enabled==true){
          if(MaxClock<_pPCH->OnClock) MaxClock=_pPCH->OnClock;
        }
      }
    }
    for(u32 ch=0;ch<PCHCount;ch++){
      if(FindCh[ch]==true){
        TPCH *_pPCH=&pPCH[ch];
        if(_pPCH->Enabled==true){
          if(MaxClock==_pPCH->OnClock) return(ch);
        }
      }
    }
  }
  
  _consolePrint("PCH_FindDisableChannel: Can not found diabled channel.");
  return(-1);
}

void PCH_NoteOn(u32 trk,u32 GT,s32 Note,s32 Pitch,u32 Vol,u32 Exp,u32 Vel,u32 var,u32 prg,u32 panpot,u32 reverb,bool DrumMode,u32 ModLevel)
{
  if(ChannelInfoCount<=trk){
    _consolePrintf("Channel overflow. ch=%d",trk);
    return;
  }
  
  {
    int EqualCh=PCH_FindEqualChannel(trk,Note);
    if(EqualCh!=-1){
      TPCH *_pPCH=&pPCH[EqualCh];
      _pPCH->GT=GT;
      return;
    }
  }
  
  TVariationMap *pVarMap;
  u32 VarNum,PrgNum;
  
  if(DrumMode==true){
    pVarMap=pVarMap_Drum;
    VarNum=prg;
    PrgNum=Note;
    }else{
    pVarMap=pVarMap_Tone;
    VarNum=var;
    PrgNum=prg;
  }
  
  TProgramPatch *pPrgPatch=GetProgramPatchPtr(pVarMap,VarNum,PrgNum,DrumMode);
  if(pPrgPatch==NULL) return;
  TProgram *pPrg=GetProgramFromPatchPtr(pPrgPatch,Note,DrumMode);
  if(pPrg==NULL) return;
  
  int ch=PCH_FindDisableChannel(false);
  
  if(ch==-1){
    _consolePrintf("PCHON %d,%d Error ChannelEmpty.\n",trk,Note);
    return;
  }
  
  PCH_ChInit(ch);
  
  TPCH *_pPCH=&pPCH[ch];
  
  TChannelInfo *_pCI=&pChannelInfo[trk];
  
  _pPCH->ChannelNum=trk;
  _pPCH->pChannelInfo=&pChannelInfo[_pPCH->ChannelNum];
  
  _pPCH->Enabled=true;
  _pPCH->GT=GT;
  _pCI->DrumMode=DrumMode;
  _pPCH->Note=Note;
  _pCI->Vol=Vol;
  _pCI->Exp=Exp;
  _pPCH->Vel=Vel;
  _pCI->Reverb=reverb;
  _pPCH->pPrgPatch=pPrgPatch;
  _pPCH->pPrg=pPrg;
  _pPCH->PrgPos=1;
  
  if(_pPCH->pPrg->s16Flag==false){
    s8 *pData=(s8*)pPrg->pData;
    _pPCH->LastSampleData=pData[0]<<8;
    _pPCH->CurSampleData=pData[1]<<8;
    }else{
    s16 *pData=(s16*)pPrg->pData;
    _pPCH->LastSampleData=pData[0];
    _pPCH->CurSampleData=pData[1];
  }
  
  pPrg->VibRatio=0;
  
  if((pPrg->VibRatio==0)||(pPrg->VibDepth==0)){
    _pPCH->VibEnable=false;
    }else{
    _pPCH->VibEnable=true;
    if(pPrg->VibSweep==0){
      _pPCH->VibSweepAdd=0;
      _pPCH->VibSweepCur=0x10000;
      }else{
      _pPCH->VibSweepAdd=pPrg->VibSweep;
      _pPCH->VibSweepCur=0;
    }
    _pPCH->VibAdd=pPrg->VibRatio;
    _pPCH->VibCur=0;
    _pPCH->VibDepth=0;
    _pPCH->VibPhase=0;
  }
  
  if(ModLevel==0){
    _pPCH->ModEnable=false;
    }else{
    _pPCH->ModEnable=true;
  }
  _pPCH->ModAdd=0x10000*8; // 1/8sec
  _pPCH->ModCur=0;
  _pPCH->ModDepth=ModLevel*8; // (128*8)/0x10000 per
  _pPCH->ModPhase=0;
  
  if(pPrg->EnvelopeFlag==false){
    _pPCH->EnvState=EES_None;
    }else{
    _pPCH->EnvState=EES_Attack;
    _pPCH->EnvSpeed=pPrg->EnvRate[0];
    _pPCH->EnvCurLevel=0;
    _pPCH->EnvEndLevel=pPrg->EnvOfs[0];
  }
  
  {
    u32 vol;
    
    vol=(_pCI->Vol*_pCI->Exp*_pPCH->Vel*_pPCH->pPrg->MasterVolume)/(128*128*128*128/128);
    vol=vol*GenVolume/128;
    _pPCH->PrgMstVol=vol;
    if(pPrg->EnvelopeFlag==false){
      _pPCH->PrgVol=vol;
      }else{
      _pPCH->PrgVol=vol*_pPCH->EnvCurLevel/0x10000;
    }
  }
  
  if(DrumMode==false){
    _pPCH->Panpot=panpot;
    }else{
    _pPCH->Panpot=pPrg->MasterPanpot;
    _pPCH->Note=pPrg->Note;
    Note=_pPCH->Note;
  }
  
  if(_pPCH->Note==0){
    _pPCH->FreqAddFix16=(u32)((float)pPrg->SampleRate/SampleRate*0x10000);
    _pPCH->FreqCurFix16=0;
    }else{
    Note-=1;
    
    Note-=68; // NoteA4
    
    while(Pitch<0){
      Note--;
      Pitch+=POWRANGE;
    }
    
    while(POWRANGE<=Pitch){
      Note++;
      Pitch-=POWRANGE;
    }
    
    u32 basefreq=440;
    
    while(Note<0){
      basefreq/=2;
      Note+=12;
    }
    
    while(12<=Note){
      basefreq*=2;
      Note-=12;
    }
    
    _pPCH->FreqAddFix16=FreqTransAddFix16(basefreq,(u32)powertbl[(Note*POWRANGE)+Pitch],pPrg->SampleRate,pPrg->RootFreq);
    _pPCH->FreqCurFix16=0;
    
    _pPCH->Panpot=panpot;
  }
  
  if(_pPCH->VibEnable==true){
    _pPCH->VibDepth=(int)(_pPCH->FreqAddFix16*pPrg->VibDepth/0x10000);
  }
  
  if(_pPCH->ModEnable==true){
    _pPCH->ModDepth=(int)(_pPCH->FreqAddFix16*_pPCH->ModDepth/0x10000);
  }
}

void PCH_NoteOff(u32 trk,u32 Note,bool DrumMode)
{
  if(DrumMode==true) return;
  
  if(ChannelInfoCount<=trk){
    _consolePrintf("Channel overflow. ch=%d",trk);
    return;
  }
  
  TChannelInfo *_pCI=&pChannelInfo[trk];
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if(_pPCH->Enabled==true){
      if((_pPCH->ChannelNum==trk)&&(_pPCH->Note==Note)){
        _pPCH->Enabled=false;
        if(_pCI->Pedal==false){
          if((_pPCH->pPrg->EnvelopeFlag==false)||(_pPCH->EnvCurLevel==0)){
            PCH_ChInit(ch);
            }else{
            if(_pPCH->EnvState<EES_Release3){
              _pPCH->EnvRelLevel=_pPCH->EnvCurLevel;
              _pPCH->EnvState=EES_Release3;
              _pPCH->EnvSpeed=-_pPCH->pPrg->EnvRate[3];
              _pPCH->EnvEndLevel=(_pPCH->pPrg->EnvOfs[3]*_pPCH->EnvRelLevel)>>16;
            }
          }
        }
      }
    }
  }
}

void PCH_PedalOn(u32 trk)
{
  if(ChannelInfoCount<=trk){
    _consolePrintf("Channel overflow. ch=%d",trk);
    return;
  }
  
  TChannelInfo *_pCI=&pChannelInfo[trk];
  
  if(_pCI->Pedal==true) return;
  
  _pCI->Pedal=true;
}

void PCH_PedalOff(u32 trk)
{
  if(ChannelInfoCount<=trk){
    _consolePrintf("Channel overflow. ch=%d",trk);
    return;
  }
  
  TChannelInfo *_pCI=&pChannelInfo[trk];
  
  if(_pCI->Pedal==false) return;
  
  _pCI->Pedal=false;
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if((_pPCH->Enabled==false)&&(_pPCH->ChannelNum==trk)){
      if((_pPCH->pPrg->EnvelopeFlag==false)||(_pPCH->EnvCurLevel==0)){
        PCH_ChInit(ch);
        }else{
        if(_pPCH->EnvState<EES_Release3){
          _pPCH->EnvRelLevel=_pPCH->EnvCurLevel;
          _pPCH->EnvState=EES_Release3;
          _pPCH->EnvSpeed=-_pPCH->pPrg->EnvRate[3];
          _pPCH->EnvEndLevel=(_pPCH->pPrg->EnvOfs[3]*_pPCH->EnvRelLevel)>>16;
        }
      }
    }
  }
}

static __attribute__ ((noinline)) void PCH_Render_Loop(int ch,u32 FreqAddFix16,s32 *pbuf,int SampleCount)
{
  TPCH *_pPCH=&pPCH[ch];
  TProgram *pPrg=_pPCH->pPrg;
  
  u32 PrgPos=_pPCH->PrgPos;
  u32 FreqCurFix16=_pPCH->FreqCurFix16;
  
  s32 LastSampleData=_pPCH->LastSampleData;
  s32 CurSampleData=_pPCH->CurSampleData;
  
  s32 lvol=(_pPCH->PrgVol*_pPCH->Panpot)<<2; // 7+7+2=16bit
  s32 rvol=(_pPCH->PrgVol*(0x80-_pPCH->Panpot))<<2; // 7+7+2=16bit
  
  u32 EndPoint;
  u32 LoopStart;
  s32 Fraction=pPrg->FractionStart;
  
  if(pPrg->LoopFlag==false){
    EndPoint=pPrg->Length;
    LoopStart=0x10000000;
    }else{
    EndPoint=pPrg->LoopEnd;
    LoopStart=pPrg->LoopStart;
  }
  
  if(pPrg->s16Flag==false){
    s8 *pData=(s8*)pPrg->pData;
    s32 REG_tmp0,REG_tmp1,REG_sl,REG_sr;
    u32 REG_0xffff=0xffff;
    
    if(FreqAddFix16<=0x10000){
		SampleCount++;
		do{
			do {
				if(--SampleCount == 0)
					goto lp_end;
				REG_sl = pbuf[0];
				REG_sr = pbuf[1];
				asm volatile (
					"smulwb %[REG_tmp0],%[FreqCurFix16],%[CurSampleData] \n\t"
					:[REG_tmp0]"=r"(REG_tmp0)
					:[FreqCurFix16]"r"(FreqCurFix16), [CurSampleData]"r"(CurSampleData)
					:
				);
				REG_tmp1 = REG_0xffff - FreqCurFix16;
				asm volatile (
					"smlawb %0,%1,%2,%0 \n\t"
					:"=r"(REG_tmp0), "=r"(REG_tmp1)
					:[LastSampleData]"r"(LastSampleData)
					:
				);
				FreqCurFix16 += FreqAddFix16;
				asm (
					"smlawb %0, %1, %2, %0 \n\t"
					:"=r"(REG_sl), "=r"(lvol)
					:"r"(REG_tmp0)
					:
				);
				asm (
					"smlawb %0, %1, %2, %0 \n\t"
					:"=r"(REG_sr), "=r"(rvol)
					:"r"(REG_tmp0)
					:
				);
				*pbuf++ = REG_sl;
				*pbuf++ = REG_sr;
			} while(FreqCurFix16 < 0x10000);
			FreqCurFix16 -= 0x10000;
			++PrgPos;
			if(PrgPos != EndPoint) {
				LastSampleData = CurSampleData;
				CurSampleData = (int)(pData[PrgPos]);
				CurSampleData <<= 8;
				continue;
			}
			if(LoopStart == 0x10000000)
				break;
			PrgPos = LoopStart;
			FreqCurFix16 += Fraction;
			LastSampleData = CurSampleData;
			CurSampleData = (int)(pData[PrgPos]);
			CurSampleData <<= 8;
			if(FreqCurFix16 < 0x10000)
				continue;
			++PrgPos;
			FreqCurFix16 -= 0x10000;
			LastSampleData = CurSampleData;
			CurSampleData = (int)(pData[PrgPos]);
			CurSampleData <<= 8;
		} while(1);
		pbuf = NULL;
lp_end:
		;
	}else{
		SampleCount++;
		do {
			if(--SampleCount == 0)
				break;
			REG_sl = pbuf[0];
			REG_sr = pbuf[1];
			asm volatile (
				"smulwb %[REG_tmp0],%[FreqCurFix16],%[CurSampleData] \n\t"
				:[REG_tmp0]"=r"(REG_tmp0)
				:[FreqCurFix16]"r"(FreqCurFix16), [CurSampleData]"r"(CurSampleData)
				:
			);
			REG_tmp1 = REG_0xffff - FreqCurFix16;
			asm volatile (
				"smlawb %0,%1,%2,%0 \n\t"
				:"=r"(REG_tmp0), "=r"(REG_tmp1)
				:[LastSampleData]"r"(LastSampleData)
				:
			);
			FreqCurFix16 += FreqAddFix16;
			asm (
				"smlawb %0, %1, %2, %0 \n\t"
				:"=r"(REG_sl), "=r"(lvol)
				:"r"(REG_tmp0)
				:
			);
			asm (
				"smlawb %0, %1, %2, %0 \n\t"
				:"=r"(REG_sr), "=r"(rvol)
				:"r"(REG_tmp0)
				:
			);
			*pbuf++ = REG_sl;
			*pbuf++ = REG_sr;
			PrgPos += FreqCurFix16 >> 16;
			FreqCurFix16 &= REG_0xffff;

			if(PrgPos < EndPoint) {
				CurSampleData = (int)(pData[PrgPos]);
				LastSampleData = PrgPos - 1;
				LastSampleData = (int)(pData[LastSampleData]);
				CurSampleData <<= 8;
				LastSampleData <<= 8;
				continue;
			}
			if(LoopStart != 0x10000000)
				break;

			PrgPos -= EndPoint;
			PrgPos += LoopStart;
			FreqCurFix16 += Fraction;

			LastSampleData = CurSampleData;
			CurSampleData = (int)(pData[PrgPos]);
			CurSampleData <<= 8;
			if(FreqCurFix16 < 0x10000)
				continue;
			++PrgPos;
			FreqCurFix16 -= 0x10000;
			LastSampleData = CurSampleData;
			CurSampleData = (int)(pData[PrgPos]);
			CurSampleData <<= 8;
		} while(1);
		if(SampleCount != 0)
			pbuf = NULL;
	}
    
    }else{
    s16 *pData=(s16*)pPrg->pData;
    s32 REG_tmp0,REG_tmp1,REG_sl,REG_sr;
    u32 REG_0xffff=0xffff;
    
    if(FreqAddFix16<=0x10000){
		SampleCount++;
		do{
			do {
				if(--SampleCount == 0)
					goto lp_end2;
				REG_sl = pbuf[0];
				REG_sr = pbuf[1];
				asm volatile (
					"smulwb %[REG_tmp0],%[FreqCurFix16],%[CurSampleData] \n\t"
					:[REG_tmp0]"=r"(REG_tmp0)
					:[FreqCurFix16]"r"(FreqCurFix16), [CurSampleData]"r"(CurSampleData)
					:
				);
				REG_tmp1 = REG_0xffff - FreqCurFix16;
				asm volatile (
					"smlawb %0,%1,%2,%0 \n\t"
					:"=r"(REG_tmp0), "=r"(REG_tmp1)
					:[LastSampleData]"r"(LastSampleData)
					:
				);
				FreqCurFix16 += FreqAddFix16;
				asm (
					"smlawb %0, %1, %2, %0 \n\t"
					:"=r"(REG_sl), "=r"(lvol)
					:"r"(REG_tmp0)
					:
				);
				asm (
					"smlawb %0, %1, %2, %0 \n\t"
					:"=r"(REG_sr), "=r"(rvol)
					:"r"(REG_tmp0)
					:
				);
				*pbuf++ = REG_sl;
				*pbuf++ = REG_sr;
			} while(FreqCurFix16 < 0x10000);
			FreqCurFix16 -= 0x10000;
			++PrgPos;
			if(PrgPos != EndPoint) {
				LastSampleData = CurSampleData;
				//this is special
				CurSampleData = (int)(pData[PrgPos]);
				continue;
			}
			if(LoopStart == 0x10000000)
				break;
			PrgPos = LoopStart;
			FreqCurFix16 += Fraction;
			CurSampleData = (int)(pData[PrgPos]);
			if(FreqCurFix16 < 0x10000)
				continue;
			++PrgPos;
			FreqCurFix16 -= 0x10000;
			LastSampleData = CurSampleData;
			CurSampleData = (int)(pData[PrgPos]);
		} while(1);
		pbuf = NULL;
lp_end2:
		;
	}else{
		SampleCount++;
		do {
			if(--SampleCount == 0)
				break;
			REG_sl = pbuf[0];
			REG_sr = pbuf[1];
			asm volatile (
				"smulwb %[REG_tmp0],%[FreqCurFix16],%[CurSampleData] \n\t"
				:[REG_tmp0]"=r"(REG_tmp0)
				:[FreqCurFix16]"r"(FreqCurFix16), [CurSampleData]"r"(CurSampleData)
				:
			);
			REG_tmp1 = REG_0xffff - FreqCurFix16;
			asm volatile (
				"smlawb %0,%1,%2,%0 \n\t"
				:"=r"(REG_tmp0), "=r"(REG_tmp1)
				:[LastSampleData]"r"(LastSampleData)
				:
			);
			FreqCurFix16 += FreqAddFix16;
			asm (
				"smlawb %0, %1, %2, %0 \n\t"
				:"=r"(REG_sl), "=r"(lvol)
				:"r"(REG_tmp0)
				:
			);
			asm (
				"smlawb %0, %1, %2, %0 \n\t"
				:"=r"(REG_sr), "=r"(rvol)
				:"r"(REG_tmp0)
				:
			);
			*pbuf++ = REG_sl;
			*pbuf++ = REG_sr;
			PrgPos += FreqCurFix16 >> 16;
			FreqCurFix16 &= REG_0xffff;

			if(PrgPos < EndPoint) {
				CurSampleData = (int)(pData[PrgPos]);
				LastSampleData = (int)(pData[PrgPos - 1]);
				continue;
			}
			if(LoopStart != 0x10000000)
				break;

			PrgPos -= EndPoint;
			PrgPos += LoopStart;
			FreqCurFix16 += Fraction;

			LastSampleData = CurSampleData;
			CurSampleData = (int)(pData[PrgPos]);
			if(FreqCurFix16 < 0x10000)
				continue;
			++PrgPos;
			FreqCurFix16 -= 0x10000;
			LastSampleData = CurSampleData;
			CurSampleData = (int)(pData[PrgPos]);
			CurSampleData <<= 8;
		} while(1);
		if(SampleCount != 0)
			pbuf = NULL;
	}
  }

  if(pbuf==NULL) PCH_ChInit(ch);

  _pPCH->LastSampleData=LastSampleData;
  _pPCH->CurSampleData=CurSampleData;
  
  _pPCH->FreqCurFix16=FreqCurFix16;
  _pPCH->PrgPos=PrgPos;
}

static __attribute__ ((noinline)) void PCH_Render_Bulk(int ch,u32 FreqAddFix16,s32 *pbuf,u32 SampleCount)
{
  TPCH *_pPCH=&pPCH[ch];
  TProgram *pPrg=_pPCH->pPrg;
  
  u32 PrgPos=_pPCH->PrgPos;
  u32 FreqCurFix16=_pPCH->FreqCurFix16;
  
  s32 LastSampleData=_pPCH->LastSampleData;
  s32 CurSampleData=_pPCH->CurSampleData;
  
  s32 lvol=(_pPCH->PrgVol*_pPCH->Panpot)<<2; // 7+7+2=16bit
  s32 rvol=(_pPCH->PrgVol*(0x80-_pPCH->Panpot))<<2; // 7+7+2=16bit
  
  if(pPrg->s16Flag==false){
    s8 *pData=(s8*)pPrg->pData;
    s32 REG_tmp0,REG_tmp1,REG_sl,REG_sr;
    u32 REG_0xffff=0xffff;
    
 	SampleCount++;
	do{
		if(--SampleCount == 0)
			break;
		REG_sl = pbuf[0];
		REG_sr = pbuf[1];
		asm volatile (
			"smulwb %[REG_tmp0],%[FreqCurFix16],%[CurSampleData] \n\t"
			:[REG_tmp0]"=r"(REG_tmp0)
			:[FreqCurFix16]"r"(FreqCurFix16), [CurSampleData]"r"(CurSampleData)
			:
		);
		REG_tmp1 = REG_0xffff - FreqCurFix16;
		asm volatile (
			"smlawb %0,%1,%2,%0 \n\t"
			:"=r"(REG_tmp0), "=r"(REG_tmp1)
			:[LastSampleData]"r"(LastSampleData)
			:
		);
		FreqCurFix16 += FreqAddFix16;
		asm (
			"smlawb %0, %1, %2, %0 \n\t"
			:"=r"(REG_sl), "=r"(lvol)
			:"r"(REG_tmp0)
			:
		);
		asm (
			"smlawb %0, %1, %2, %0 \n\t"
			:"=r"(REG_sr), "=r"(rvol)
			:"r"(REG_tmp0)
			:
		);
		*pbuf++ = REG_sl;
		*pbuf++ = REG_sr;
		if(FreqCurFix16 < 0x10000)
			continue;
		PrgPos += FreqCurFix16 >> 16;
		CurSampleData = (int)(pData[PrgPos]);
		FreqCurFix16 &= REG_0xffff;
		REG_tmp0 = PrgPos - 1;
		LastSampleData = (int)(pData[REG_tmp0]);
		CurSampleData <<= 8;
		LastSampleData <<= 8;
	} while(1);
    
    }else{
    s16 *pData=(s16*)pPrg->pData;
    s32 REG_tmp0,REG_tmp1,REG_sl,REG_sr;
    u32 REG_0xffff=0xffff;
    
	SampleCount++;
	do{
		if(--SampleCount == 0)
			break;
		REG_sl = pbuf[0];
		REG_sr = pbuf[1];
		asm volatile (
			"smulwb %[REG_tmp0],%[FreqCurFix16],%[CurSampleData] \n\t"
			:[REG_tmp0]"=r"(REG_tmp0)
			:[FreqCurFix16]"r"(FreqCurFix16), [CurSampleData]"r"(CurSampleData)
			:
		);
		REG_tmp1 = REG_0xffff - FreqCurFix16;
		asm volatile (
			"smlawb %0,%1,%2,%0 \n\t"
			:"=r"(REG_tmp0), "=r"(REG_tmp1)
			:[LastSampleData]"r"(LastSampleData)
			:
		);
		FreqCurFix16 += FreqAddFix16;
		asm (
			"smlawb %0, %1, %2, %0 \n\t"
			:"=r"(REG_sl), "=r"(lvol)
			:"r"(REG_tmp0)
			:
		);
		asm (
			"smlawb %0, %1, %2, %0 \n\t"
			:"=r"(REG_sr), "=r"(rvol)
			:"r"(REG_tmp0)
			:
		);
		*pbuf++ = REG_sl;
		*pbuf++ = REG_sr;
		if(FreqCurFix16 < 0x10000)
			continue;
		PrgPos += FreqCurFix16 >> 16;
		CurSampleData = (int)(pData[PrgPos]);
		FreqCurFix16 &= REG_0xffff;
		REG_tmp0 = PrgPos - 1;
		LastSampleData = (int)(pData[REG_tmp0]);
	} while(1);
  }

  _pPCH->LastSampleData=LastSampleData;
  _pPCH->CurSampleData=CurSampleData;
  
  _pPCH->FreqCurFix16=FreqCurFix16;
  _pPCH->PrgPos=PrgPos;
}

bool PCH_RequestRender(u32 TagChannel)
{
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if(_pPCH->ChannelNum==TagChannel){
      if((_pPCH->pPrg!=NULL)&&(_pPCH->FreqAddFix16!=0)){
        return(true);
      }
    }
  }
  
  return(false);
}

void PCH_RenderStart(u32 SampleCount)
{
  u32 totalch=0,heavych=0;
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if((_pPCH->pPrg!=NULL)&&(_pPCH->FreqAddFix16!=0)){
      totalch++;
      if(0x10000<_pPCH->FreqAddFix16) heavych++;
    }
  }
  
  static u32 a=0;
  if(a<totalch){
    a=totalch;
    _consolePrintf("------ Total channels: %dchs.\n",totalch);
  }
  
  u32 limitch=PCHCount-heavych;
  if(totalch<=limitch) return;
  
  while(limitch<totalch){
    u32 fch=PCH_FindDisableChannel(true);
    if(fch==(u32)-1) break;
    PCH_ChInit(fch);
//    _consolePrintf("%d,",fch);
    
    totalch=0;
    for(u32 ch=0;ch<PCHCount;ch++){
      TPCH *_pPCH=&pPCH[ch];
      if((_pPCH->pPrg!=NULL)&&(_pPCH->FreqAddFix16!=0)) totalch++;
    }
  }
//  _consolePrint("\n");
}

void PCH_Render(u32 TagChannel,s32 *buf,u32 SampleCount)
{
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    if(_pPCH->ChannelNum==TagChannel){
      if((_pPCH->pPrg!=NULL)&&(_pPCH->FreqAddFix16!=0)){
        TProgram *pPrg=_pPCH->pPrg;
        
        u32 FreqAddFix16;
        
        {
          int tmp=(int)_pPCH->FreqAddFix16;
          if(_pPCH->VibEnable==true) tmp+=_pPCH->VibPhase;
          if(_pPCH->ModEnable==true) tmp+=_pPCH->ModPhase;
          FreqAddFix16=(u32)tmp;
        }
        
        u32 EndPos=_pPCH->PrgPos+((FreqAddFix16*SampleCount)>>16)+1;
        
        bool BulkFlag=true;
        
        if(pPrg->LoopFlag==false){
          if(pPrg->Length<=EndPos) BulkFlag=false;
          }else{
          if(pPrg->LoopEnd<=EndPos) BulkFlag=false;
        }
        
        if(BulkFlag==true){
          PCH_Render_Bulk(ch,FreqAddFix16,buf,SampleCount);
          }else{
          PCH_Render_Loop(ch,FreqAddFix16,buf,SampleCount);
        }
      }
    }
  }
  
}

void PCH_RenderEnd(void)
{
}

void PCH_IncrementUnusedClockCount(void)
{
  for(u32 var=0;var<128;var++){
    TProgramMap *pPrgMap_Tone=pVarMap_Tone->pPrgMaps[var];
    if(pPrgMap_Tone!=NULL){
      for(u32 prg=0;prg<128;prg++){
        TProgramPatch *pPrgPatch=pPrgMap_Tone->ppData[prg];
        if(pPrgPatch!=NULL) pPrgPatch->UnusedClockCount++;
      }
    }
    TProgramMap *pPrgMap_Drum=pVarMap_Drum->pPrgMaps[var];
    if(pPrgMap_Drum!=NULL){
      for(u32 prg=0;prg<128;prg++){
        TProgramPatch *pPrgPatch=pPrgMap_Drum->ppData[prg];
        if(pPrgPatch!=NULL) pPrgPatch->UnusedClockCount++;
      }
    }
  }
}

u32 PCH_GetReverb(u32 TagChannel)
{
  return(pChannelInfo[TagChannel].Reverb);
}

int PCH_GT_GetNearClock(void)
{
  int NearClock=0x7fffffff;
  
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    
    if(_pPCH->Enabled==true){
      if(_pPCH->GT!=0){
        if((s32)_pPCH->GT<NearClock) NearClock=(s32)_pPCH->GT;
      }
    }
  }
  
  if(NearClock==0x7fffffff) NearClock=0;
  
  return(NearClock);
}

void PCH_GT_DecClock(u32 clk)
{
  for(u32 ch=0;ch<PCHCount;ch++){
    TPCH *_pPCH=&pPCH[ch];
    
    if(_pPCH->Enabled==true){
      if(_pPCH->GT!=0){
        _pPCH->GT-=clk;
        if(_pPCH->GT==0){
          PCH_NoteOff(_pPCH->ChannelNum,_pPCH->Note,_pPCH->pChannelInfo->DrumMode);
        }
      }
    }
  }
}

bool PCH_isDrumMap(u32 TagChannel)
{
  return(pChannelInfo[TagChannel].DrumMode);
}

