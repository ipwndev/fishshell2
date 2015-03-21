
#include <stdio.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "memtool.h"
#include "maindef.h"
#include "sndfont.h"
#include "unicode.h"
#include "fat2.h"

#include "inifile.h"

#include "plug_midi.h"

#include "libmidi/smidlib.h"
#include "libmidi/rcplib.h"
#include "libmidi/pch.h"

// -----------------

//#define ALIGNED_VAR_IN_DTCM __attribute__ ((aligned(4),section (".dtcm")))
#define ALIGNED_VAR_IN_DTCM

static u8 *DeflateBuf=NULL;
static u32 DeflateSize;

static int TotalClock,CurrentClock;

static u32 ClockCur;

enum EFileFormat {EFF_MID,EFF_RCP};

static EFileFormat FileFormat;

const u32 SampleRate=32768;
const u32 SamplePerFrame=512;

static u32 StackIndex;
static const u32 StackCount=2;
static s32 *pStackBuf[PCH_ChannelsCount][StackCount];
static bool StackBufExists[PCH_ChannelsCount];

// --------------------------------------------------------------------

// ------------------------------------------------------------------------------------

static void selSetParam(u8 *data,u32 GenVolume)
{
  switch(FileFormat){
    case EFF_MID: smidlibSetParam(DeflateBuf,SampleRate,GenVolume); break;
    case EFF_RCP: rcplibSetParam(DeflateBuf,SampleRate,GenVolume); break;
  }
}

static bool selStart(void)
{
  switch(FileFormat){
    case EFF_MID: return(smidlibStart()); 
    case EFF_RCP: return(rcplibStart()); 
  }
  
  return(false);
}

static void selFree(void)
{
  switch(FileFormat){
    case EFF_MID: smidlibFree(); break;
    case EFF_RCP: rcplibFree(); break;
  }
}

static int selGetNearClock(void)
{
  switch(FileFormat){
    case EFF_MID: return(smidlibGetNearClock()); 
    case EFF_RCP: return(rcplibGetNearClock()); 
  }
  
  return(0);
}

static bool selNextClock(bool ShowEventMessage,bool EnableNote,int DecClock)
{
  switch(FileFormat){
    case EFF_MID: return(smidlibNextClock(ShowEventMessage,EnableNote,DecClock)); 
    case EFF_RCP: return(rcplibNextClock(ShowEventMessage,EnableNote,DecClock)); 
  }
  
  return(false);
}

static void selAllSoundOff(void)
{
  switch(FileFormat){
    case EFF_MID: smidlibAllSoundOff(); break;
    case EFF_RCP: rcplibAllSoundOff(); break;
  }
}

static bool sel_isAllTrackEOF(void)
{
  switch(FileFormat){
    case EFF_MID: return(SM_isAllTrackEOF()); 
    case EFF_RCP: return(RCP_isAllTrackEOF()); 
  }
  
  return(false);
}

static u32 sel_GetSamplePerClockFix16(void)
{
  switch(FileFormat){
    case EFF_MID: return(SM_GetSamplePerClockFix16()); 
    case EFF_RCP: return(RCP_GetSamplePerClockFix16()); 
  }
  
  return(0);
}

static void Start_smidlibDetectTotalClock()
{
  TSM_Track *pSM_Track=NULL;
  u32 trklen=0;
  
  {
    for(u32 idx=0;idx<StdMIDI.SM_Chank.Track;idx++){
      TSM_Track *pCurSM_Track=&StdMIDI.SM_Tracks[idx];
      u32 ctrklen=(u32)pCurSM_Track->DataEnd-(u32)pCurSM_Track->Data;
      if(trklen<ctrklen){
        trklen=ctrklen;
        pSM_Track=pCurSM_Track;
      }
    }
  }
  
  if((pSM_Track==NULL)||(trklen==0)) StopFatalError(14901,"Null MID track.\n");
  
  _consolePrint("Detect length.\n");
  
  while(1){
    int DecClock=smidlibGetNearClock();
    if(smidlibNextClock(false,false,DecClock)==false) break;
    TotalClock+=DecClock;
    if(0x7f000000<TotalClock) break;
  }
  
  _consolePrintf("Total length=%dclk.\n",TotalClock);
}

static void Start_rcplibDetectTotalClock()
{
  const TRCP *pRCP=RCP_GetStruct_RCP();
  const TRCP_Chank *pRCP_Chank=RCP_GetStruct_RCP_Chank();
  
  const TRCP_Track *pRCP_Track=NULL;
  u32 trklen=0;
  
  {
    for(u32 idx=0;idx<pRCP_Chank->TrackCount;idx++){
      const TRCP_Track *pCurRCP_Track=&pRCP->RCP_Track[idx];
      u32 ctrklen=(u32)pCurRCP_Track->DataEnd-(u32)pCurRCP_Track->Data;
      if(trklen<ctrklen){
        trklen=ctrklen;
        pRCP_Track=pCurRCP_Track;
      }
    }
  }
  
  if((pRCP_Track==NULL)||(trklen==0)) StopFatalError(14902,"Null RCP track.\n");
  
  _consolePrint("Detect length.\n");
  
  while(1){
    int DecClock=rcplibGetNearClock();
    if(rcplibNextClock(false,false,DecClock)==false) break;
    TotalClock+=DecClock;
    if(0x7f000000<TotalClock) break;
  }
  
  _consolePrintf("Total length=%dclk.\n",TotalClock);
}

bool Start_InitStackBuf(void)
{
  for(u32 ch=0;ch<PCH_ChannelsCount;ch++){
    for(u32 sidx=0;sidx<StackCount;sidx++){
      pStackBuf[ch][sidx]=NULL;
    }
  }
  
  for(u32 ch=0;ch<PCH_ChannelsCount;ch++){
    for(u32 sidx=0;sidx<StackCount;sidx++){
      pStackBuf[ch][sidx]=(s32*)safemalloc_chkmem(&MM_DLLSound,(SamplePerFrame+1)*2*4);
      MemSet32CPU(0,pStackBuf[ch][sidx],(SamplePerFrame+1)*2*4);
    }
    StackBufExists[ch]=false;
  }
  
  StackIndex=0;
  
  return(true);
}

bool PlugMIDI_Start(FAT_FILE *pFileHandle)
{
  SndFont_Open();
  
  if(Start_InitStackBuf()==false) return(false);
  
  {
    DeflateSize=FAT2_GetFileSize(pFileHandle);
    DeflateBuf=(u8*)safemalloc_chkmem(&MM_DLLSound,DeflateSize);
    FAT2_fread(DeflateBuf,1,DeflateSize,pFileHandle);
  }
  
  PCH_InitProgramMap();
  
  {
    bool detect=false;
    
    if(strncmp((char*)DeflateBuf,"MThd",4)==0){
      _consolePrint("Start Standard MIDI file.\n");
      detect=true;
      FileFormat=EFF_MID;
    }
    if(strncmp((char*)DeflateBuf,"RCM-PC98V2.0(C)COME ON MUSIC",28)==0){
      _consolePrint("Start RecomposerV2.0 file.\n");
      detect=true;
      FileFormat=EFF_RCP;
    }
    
    if(detect==false){
      _consolePrint("Unknown file format!!\n");
      PCH_FreeProgramMap();
      SndFont_Close();
      return(false);
    }
  }
  
  selSetParam(DeflateBuf,GlobalINI.MIDPlugin.GenVolume);
  if(selStart()==false){
    PlugMIDI_Free();
    return(false);
  }
  
  TotalClock=0;
  CurrentClock=0;
  
  switch(FileFormat){
    case EFF_MID: Start_smidlibDetectTotalClock(); break;
    case EFF_RCP: Start_rcplibDetectTotalClock(); break;
  }
  
  if(TotalClock==0){
    _consolePrint("Detect TotalClock equal Zero.\n");
    PlugMIDI_Free();
    return(false);
  }
  
  selFree();
  
  selStart();
  
  ClockCur=selGetNearClock();
  selNextClock(GlobalINI.MIDPlugin.ShowEventMessage,true,selGetNearClock());
  
  return(true);
}

void PlugMIDI_Free(void)
{
  selFree();
  
  PCH_FreeProgramMap();
  
  for(u32 ch=0;ch<PCH_ChannelsCount;ch++){
    for(u32 sidx=0;sidx<StackCount;sidx++){
      if(pStackBuf[ch][sidx]!=NULL){
        safefree(&MM_DLLSound,pStackBuf[ch][sidx]); pStackBuf[ch][sidx]=NULL;
      }
    }
  }
  
  if(DeflateBuf!=NULL){
    safefree(&MM_DLLSound,DeflateBuf); DeflateBuf=NULL;
  }
  
  SndFont_Close();
}

u32 PlugMIDI_Update(u32 *plrbuf)
{
  if(sel_isAllTrackEOF()==true) return(0);
  
  int ProcClock=0;
  
  {
    u32 SamplePerFrameFix16=SamplePerFrame*0x10000;
    u32 SamplePerClockFix16=sel_GetSamplePerClockFix16();
    while(ClockCur<SamplePerFrameFix16){
      ProcClock++;
      ClockCur+=SamplePerClockFix16;
    }
    ClockCur-=SamplePerFrame*0x10000;
  }
  
  while(ProcClock!=0){
    int DecClock=selGetNearClock();
//    if(ProcClock>=DecClock) _consolePrintf("[%d]",CurrentClock+1);
    if(ProcClock<DecClock) DecClock=ProcClock;
    ProcClock-=DecClock;
    CurrentClock+=DecClock;
    if(selNextClock(GlobalINI.MIDPlugin.ShowEventMessage,true,DecClock)==false) break;
  }
  
  PCH_NextClock();
  
  {
    // d‚¢i–ñ1.3ƒ~ƒŠ•bj‚Ì‚Å–ñ1•b‚Éˆê‰ñ‚¾‚¯ŒvŽZ‚·‚é
    static u32 a=0;
    if(a<32){
      a++;
      }else{
      a=0;
      PCH_IncrementUnusedClockCount();
    }
  }
  
  PCH_RenderStart(SamplePerFrame);
  
  if(plrbuf!=NULL){
    // curpos=5017,17284us static.dstbuf DTCM.tmpbuf spf=1024.
    // curpos=5017,25171us static.dstbuf static.tmpbuf spf=1024.
    // curpos=5017,7058us DTCM.dstbuf DTCM.tmpbuf spf=512.
    s32 dstbuf[SamplePerFrame*2];
    MemSet32CPU(0,dstbuf,SamplePerFrame*2*4);
    
    StackIndex++;
    if(StackIndex==StackCount) StackIndex=0;
    
    for(u32 ch=0;ch<PCH_ChannelsCount;ch++){
      bool reqproc=false;
      bool reqrender=false;
      if(StackBufExists[ch]==true) reqproc=true;
      if(PCH_RequestRender(ch)==true){
        reqproc=true;
        reqrender=true;
      }
      
      if(reqproc==true){
        s32 tmpbuf[SamplePerFrame*2];
        MemSet32CPU(0,tmpbuf,SamplePerFrame*2*4);
        
        if(reqrender==true) PCH_Render(ch,tmpbuf,SamplePerFrame);
        
        s32 *pstackbuf=pStackBuf[ch][StackIndex];
        
        {
          s32 *p=&pstackbuf[(SamplePerFrame-1)*2];
          s32 l=*p++;
          s32 r=*p++;
          *p++=l;
          *p++=r;
        }
        
        u32 ReverbFactor;
        
        if(PCH_isDrumMap(ch)==true){
          ReverbFactor=GlobalINI.MIDPlugin.ReverbFactor_DrumMap;
          }else{
          ReverbFactor=GlobalINI.MIDPlugin.ReverbFactor_ToneMap;
        }
        
        vu32 Reverb=16+PCH_GetReverb(ch);
        if(127<Reverb) Reverb=127;
        Reverb=Reverb*ReverbFactor*2; // 127*127*2=15bit
        
        s32 *pdstbuf=dstbuf;
        s32 *ptmpbuf=tmpbuf;
        s32 sl0,sr0,sl1,sr1;
        s32 tmpl,tmpr;
        u32 SampleCount=SamplePerFrame;
        bool exists=false;
        
        asm volatile (
			"reverb_loop: \n\t"
			" \n\t"
			"ldmia %[pstackbuf],{%[sl0],%[sr0],%[sl1],%[sr1]} \n\t"
			"ldmia %[ptmpbuf]!,{%[tmpl],%[tmpr]} \n\t"
			" \n\t"
			// LPF
			"add %[sl0],%[sl0],%[sl1]; smulwb %[sl1],%[sl0],%[Reverb] \n\t"
			"add %[sr0],%[sr0],%[sr1]; smulwb %[sr1],%[sr0],%[Reverb] \n\t"
			" \n\t"
			"add %[sr0],%[sl1],%[tmpr] \n\t"
			"add %[sl0],%[sr1],%[tmpl] \n\t"
			" \n\t"
			"ldmia %[pdstbuf],{%[tmpl],%[tmpr]} \n\t"
			" \n\t"
			"stmia %[pstackbuf]!,{%[sl0],%[sr0]} \n\t"
			" \n\t"
			"add %[tmpl],%[tmpl],%[sl0] \n\t"
			"add %[tmpr],%[tmpr],%[sr0] \n\t"
			" \n\t"
			"stmia %[pdstbuf]!,{%[tmpl],%[tmpr]} \n\t"
			" \n\t"
			"add %[tmpl],%[tmpl],%[tmpr] \n\t"
			"cmp %[tmpl],#0x100 \n\t"
			"movgt %[exists],#1 \n\t"
			" \n\t"
			"subs %[SampleCount],%[SampleCount],#1 \n\t"
			"bne reverb_loop \n\t"
			:[pstackbuf]"+r"(pstackbuf), [sl0]"+r"(sl0), [sl1]"+r"(sl1), [ptmpbuf]"+r"(ptmpbuf), [tmpl]"+r"(tmpl), 
				[tmpr]"+r"(tmpr), [Reverb]"+r"(Reverb), [sr0]"+r"(sr0), [sr1]"+r"(sr1), [pdstbuf]"+r"(pdstbuf), 
				[exists]"+r"(exists), [SampleCount]"+r"(SampleCount)
			::"memory"
        );
        
        StackBufExists[ch]=exists;
      }
    }
    
    u32 SampleCount=SamplePerFrame;
    s32 *pdstbuf=dstbuf;
    //const s32 LimitMin=-0x8000,LimitMax=0xffff;
	s32 LimitMin=-0x8000,LimitMax=0xffff;
    s32 sl0=0,sr0=0,sl1=0,sr1=0,sl2=0,sr2=0,sl3=0,sr3=0;
    
    asm volatile (
		"copybuffer_stereo: \n\t"
		" \n\t"
		"ldmia %[pdstbuf]!,{%[sl0],%[sr0],%[sl1],%[sr1],%[sl2],%[sr2],%[sl3],%[sr3]} \n\t"
		" \n\t"
		"cmp %[sl0],%[LimitMin]; movlt %[sl0],%[LimitMin]; cmp %[sl0],%[LimitMax],lsr #1; movgt %[sl0],%[LimitMax],lsr #1 \n\t"
		"cmp %[sr0],%[LimitMin]; movlt %[sr0],%[LimitMin]; cmp %[sr0],%[LimitMax],lsr #1; movgt %[sr0],%[LimitMax],lsr #1 \n\t"
		"cmp %[sl1],%[LimitMin]; movlt %[sl1],%[LimitMin]; cmp %[sl1],%[LimitMax],lsr #1; movgt %[sl1],%[LimitMax],lsr #1 \n\t"
		"cmp %[sr1],%[LimitMin]; movlt %[sr1],%[LimitMin]; cmp %[sr1],%[LimitMax],lsr #1; movgt %[sr1],%[LimitMax],lsr #1 \n\t"
		"cmp %[sl2],%[LimitMin]; movlt %[sl2],%[LimitMin]; cmp %[sl2],%[LimitMax],lsr #1; movgt %[sl2],%[LimitMax],lsr #1 \n\t"
		"cmp %[sr2],%[LimitMin]; movlt %[sr2],%[LimitMin]; cmp %[sr2],%[LimitMax],lsr #1; movgt %[sr2],%[LimitMax],lsr #1 \n\t"
		"cmp %[sl3],%[LimitMin]; movlt %[sl3],%[LimitMin]; cmp %[sl3],%[LimitMax],lsr #1; movgt %[sl3],%[LimitMax],lsr #1 \n\t"
		"cmp %[sr3],%[LimitMin]; movlt %[sr3],%[LimitMin]; cmp %[sr3],%[LimitMax],lsr #1; movgt %[sr3],%[LimitMax],lsr #1 \n\t"
		" \n\t"
		"and %[sl0],%[sl0],%[LimitMax]; orr %[sl0],%[sl0],%[sr0],lsl #16 \n\t"
		"and %[sl1],%[sl1],%[LimitMax]; orr %[sl1],%[sl1],%[sr1],lsl #16 \n\t"
		"and %[sl2],%[sl2],%[LimitMax]; orr %[sl2],%[sl2],%[sr2],lsl #16 \n\t"
		"and %[sl3],%[sl3],%[LimitMax]; orr %[sl3],%[sl3],%[sr3],lsl #16 \n\t"
		"stmia %[pdstbuf]!,{%[sl0],%[sl1],%[sl2],%[sl3]} \n\t"
		" \n\t"
		"subs %[SampleCount],%[SampleCount],#4 \n\t"
		"bne copybuffer_stereo \n\t"
		:[sl0]"+r"(sl0), [sr0]"+r"(sr0), [sl1]"+r"(sl1), [sr1]"+r"(sr1), [sl2]"+r"(sl2), [sr2]"+r"(sr2), [sl3]"+r"(sl3), [sr3]"+r"(sr3), 
			[SampleCount]"+r"(SampleCount), [pdstbuf]"+r"(pdstbuf), [LimitMin]"+r"(LimitMin), [LimitMax]"+r"(LimitMax)
		::"memory"
    );
  }
  
  PCH_RenderEnd();
  
  return(SamplePerFrame);
}

s32 PlugMIDI_GetPosMax(void)
{
  return(TotalClock);
}

s32 PlugMIDI_GetPosOffset(void)
{
  return(CurrentClock);
}

void PlugMIDI_SetPosOffset(s32 ofs)
{
  if(ofs<CurrentClock){
    selFree();
    selStart();
    CurrentClock=0;
    }else{
    selAllSoundOff();
  }
  
  while(CurrentClock<=ofs){
    int DecClock=selGetNearClock();
    if(selNextClock(false,false,DecClock)==false) break;
    CurrentClock+=DecClock;
  }
  
  ClockCur=0;
}

u32 PlugMIDI_GetChannelCount(void)
{
  return(2);
}

u32 PlugMIDI_GetSampleRate(void)
{
  return(SampleRate);
}

u32 PlugMIDI_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

u32 PlugMIDI_GetPlayTimeSec(void)
{
    return(0);
}

int PlugMIDI_GetInfoIndexCount(void)
{
  if(GlobalINI.MIDPlugin.ShowInfomationMessages==true){
    if(MemoryOverflowFlag==true) return(1);
  }
  
  switch(FileFormat){
    case EFF_MID: return(5); 
    case EFF_RCP: return(15); 
  }
  
  return(0);
}

bool PlugMIDI_GetInfoStrL(int idx,char *str,int len)
{
  if(GlobalINI.MIDPlugin.ShowInfomationMessages==true){
    if(MemoryOverflowFlag==true){
      switch(idx){
        case 0: snprintf(str,len,"Sound font memory overflow."); return(true); 
      }
      return(false);
    }
  }
  
  switch(FileFormat){
    case EFF_MID: {
      TSM_Chank *_SM_Chank=&StdMIDI.SM_Chank;
      switch(idx){
        case 0: snprintf(str,len,"Format=%d Tracks=%d TimeRes=%d",_SM_Chank->Format,_SM_Chank->Track,_SM_Chank->TimeRes); return(true); 
        case 1: snprintf(str,len,"Title=%s",(gME_Title!=NULL) ? gME_Title : ""); return(true); 
        case 2: snprintf(str,len,"Copyright=%s",(gME_Copyright!=NULL) ? gME_Copyright : ""); return(true); 
        case 3: snprintf(str,len,"Text=%s",(gME_Text!=NULL) ? gME_Text : ""); return(true); 
        case 4: snprintf(str,len,"TotalClock=%d",TotalClock); return(true); 
      }
    } break;
    case EFF_RCP: {
      const TRCP_Chank *pRCP_Chank=RCP_GetStruct_RCP_Chank();
      switch(idx){
        case 0: snprintf(str,len,"TimeRes=%d Tempo=%d PlayBias=%d",pRCP_Chank->TimeRes,pRCP_Chank->Tempo,pRCP_Chank->PlayBias); return(true); 
        case 1: snprintf(str,len,"TrackCount=%d TotalClock=%d",pRCP_Chank->TrackCount,TotalClock); return(true); 
        case 2: snprintf(str,len,"%0.64s",pRCP_Chank->Title); return(true); 
        case 3: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[0*28]); return(true); 
        case 4: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[1*28]); return(true); 
        case 5: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[2*28]); return(true); 
        case 6: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[3*28]); return(true); 
        case 7: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[4*28]); return(true); 
        case 8: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[5*28]); return(true); 
        case 9: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[6*28]); return(true); 
        case 10: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[7*28]); return(true); 
        case 11: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[8*28]); return(true); 
        case 12: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[9*28]); return(true); 
        case 13: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[10*28]); return(true); 
        case 14: snprintf(str,len,"%0.28s",&pRCP_Chank->Memo[11*28]); return(true); 
      }
    } break;
  }
  
  return(false);
}

bool PlugMIDI_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugMIDI_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

// -----------------------------------------------------------

