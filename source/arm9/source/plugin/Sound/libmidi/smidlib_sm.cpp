
#include <stdio.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "memtool.h"
#include "strtool.h"

#include "plugin.h"

#include "smidlib.h"
#include "mtrk.h"
#include "inifile.h"

char *gME_Text=NULL,*gME_Copyright=NULL,*gME_Title=NULL;

TStdMIDI StdMIDI;

typedef struct {
  u32 Len;
} TSysExEvent;

typedef struct {
  u8 ID;
  u32 Len;
  u8 *Data;
} TMetaEvent;

typedef struct {
  u8 Status;
  u32 EventType; // 0=MIDI 1=SysEx 2=Meta
  TMetaEvent MetaEvent;
} TSM_Event;

// -----------------------------------------------------------

void SM_Init(void)
{
  MemSet32CPU(0,&StdMIDI,sizeof(TStdMIDI));
  
  TStdMIDI *_StdMIDI=&StdMIDI;
  
  _StdMIDI->File=NULL;
  _StdMIDI->FilePos=0;
  
  _StdMIDI->FastNoteOn=false;
  
  _StdMIDI->SampleRate=0;
  _StdMIDI->SamplePerClockFix16=0;
  
  _StdMIDI->SM_Chank.ID[0]=0;
  _StdMIDI->SM_Chank.ID[1]=0;
  _StdMIDI->SM_Chank.ID[2]=0;
  _StdMIDI->SM_Chank.ID[3]=0;
  _StdMIDI->SM_Chank.Len=0;
  _StdMIDI->SM_Chank.Format=0;
  _StdMIDI->SM_Chank.Track=0;
  _StdMIDI->SM_Chank.TimeRes=0;
}

void SM_Free(void)
{
  if(gME_Text!=NULL){
    safefree(&MM_DLLSound,gME_Text); gME_Text=NULL;
  }
  if(gME_Copyright!=NULL){
    safefree(&MM_DLLSound,gME_Copyright); gME_Copyright=NULL;
  }
  if(gME_Title!=NULL){
    safefree(&MM_DLLSound,gME_Title); gME_Title=NULL;
  }
}

static inline void SM_ReadSkip(int size)
{
  TStdMIDI *_StdMIDI=&StdMIDI;
  
  _StdMIDI->FilePos+=size;
}

static inline u8 SM_ReadByte(void)
{
  TStdMIDI *_StdMIDI=&StdMIDI;
  
  u8 res=_StdMIDI->File[_StdMIDI->FilePos];
  
  _StdMIDI->FilePos++;
  
  return(res);
}

static inline u16 SM_ReadBigWord(void)
{
  u8 t0=SM_ReadByte();
  u8 t1=SM_ReadByte();
  
  return(((u16)t0) << 8)+((u16)t1);
}

static inline u32 SM_ReadBigDWord(void)
{
  u8 t0=SM_ReadByte();
  u8 t1=SM_ReadByte();
  u8 t2=SM_ReadByte();
  u8 t3=SM_ReadByte();
  
  return( (((u32)t0) << 24)+(((u32)t1) << 16)+(((u32)t2) << 8)+((u32)t3) );
}

static inline u32 SM_ReadVarDWord(void)
{
  u8 tmp;
  u32 res=0;
  
  while(1){
    res=res << 7;
    tmp=SM_ReadByte();
    res=res+(tmp & 0x7f);
    if((tmp & 0x80)==0) return(res);
  }
  
}

static void SM_LoadChank(void)
{
  TSM_Chank *_SM_Chank=&StdMIDI.SM_Chank;
  
  _SM_Chank->ID[0]=SM_ReadByte();
  _SM_Chank->ID[1]=SM_ReadByte();
  _SM_Chank->ID[2]=SM_ReadByte();
  _SM_Chank->ID[3]=SM_ReadByte();
  
  _SM_Chank->Len=SM_ReadBigDWord();
  _SM_Chank->Format=SM_ReadBigWord();
  _SM_Chank->Track=SM_ReadBigWord();
  _SM_Chank->TimeRes=SM_ReadBigWord();
  
  if(_SM_Chank->Format==0) _SM_Chank->Track=1;
}

static void SM_LoadTrackChank(TSM_Track *pSM_Track)
{
  pSM_Track->EndFlag=false;
  
  pSM_Track->ID[0]=SM_ReadByte();
  pSM_Track->ID[1]=SM_ReadByte();
  pSM_Track->ID[2]=SM_ReadByte();
  pSM_Track->ID[3]=SM_ReadByte();
  
  u32 len=SM_ReadBigDWord();
  
  pSM_Track->Data=&StdMIDI.File[StdMIDI.FilePos];
  pSM_Track->DataEnd=&pSM_Track->Data[len];
  
  SM_ReadSkip(len);
  
  pSM_Track->RunningStatus=0;
  pSM_Track->WaitClock=0;
}

static inline void SM_TrackReadSkip(TSM_Track *pSM_Track,int size)
{
  pSM_Track->Data+=size;
}

#define SM_TrackReadByte(pSM_Track) (*pSM_Track->Data++)

static inline u16 SM_TrackReadBigWord(TSM_Track *pSM_Track)
{
  u32 t0=SM_TrackReadByte(pSM_Track);
  u32 t1=SM_TrackReadByte(pSM_Track);
  
  return((u16)((t0<<8)| t1));
}

static inline u32 SM_TrackReadBigDWord(TSM_Track *pSM_Track)
{
  u32 t0=SM_TrackReadByte(pSM_Track);
  u32 t1=SM_TrackReadByte(pSM_Track);
  u32 t2=SM_TrackReadByte(pSM_Track);
  u32 t3=SM_TrackReadByte(pSM_Track);
  
  return((t0<<24) | (t1<<16) | (t2<<8) | t3);
}

static inline u32 SM_TrackReadVarDWord(TSM_Track *pSM_Track)
{
  u32 res=0;
  
  while(1){
    res=res << 7;
    u32 tmp=SM_TrackReadByte(pSM_Track);
    res=res+(tmp & 0x7f);
    if((tmp & 0x80)==0) return(res);
  }
  
}

static void SM_ProcMIDIEvent(bool ShowMessage,bool EnableNote,TSM_Track *pSM_Track,u32 Status)
{
  TStdMIDI *_StdMIDI=&StdMIDI;
  
  u32 cmd=Status >> 4;
  u32 ch=Status & 0x0f;
  
  u32 Data0,Data1;
  
#define ReadData1() { \
  Data0=SM_TrackReadByte(pSM_Track); \
}

#define ReadData2() { \
  Data0=SM_TrackReadByte(pSM_Track); \
  Data1=SM_TrackReadByte(pSM_Track); \
}
  
  switch(cmd){
    case 0x8:
      ReadData2();
      if(ShowMessage==true) _consolePrintf("$%xx/NoteOff(%x):note%d vel%d\n",cmd,ch,Data0,Data1);
      if(EnableNote==true) MTRK_NoteOff(ch,Data0,Data1);
      break;
    case 0x9:
      // if(ch==1) EnableNote=false;
      ReadData2();
      if(ShowMessage==true) _consolePrintf("$%xx/NoteOn(%x):note%d vel%d\n",cmd,ch,Data0,Data1);
      if(Data1!=0){ // Vel!=0
        if(EnableNote==false){
          MTRK_NoteOn_LoadProgram(ch,Data0,Data1);
          }else{
          MTRK_NoteOn(ch,0,Data0,Data1);
        }
        }else{
        if(EnableNote==true) MTRK_NoteOff(ch,Data0,Data1);
      }
      _StdMIDI->FastNoteOn=true;
      break;
    case 0xa:
      ReadData2();
//      if(ShowMessage==true) _consolePrintf("$%xx/PolyKeyPress(%x):note%d vel%d\n",cmd,ch,Data0,Data1);
      break;
    case 0xb:
      ReadData2();
      if(ShowMessage==true) _consolePrintf("$%xx/ControlChange(%x):Num%d Val%d\n",cmd,ch,Data0,Data1);
      MTRKCC_Proc(ch,Data0,Data1);
      break;
    case 0xc:
      ReadData1();
      if(ShowMessage==true) _consolePrintf("$%xx/ProgramChange(%x):prg%d\n",cmd,ch,Data0);
      MTRK_SetProgram(ch,Data0);
      break;
    case 0xd:
      ReadData1();
//      if(ShowMessage==true) _consolePrintf("$%xx/ChannelPress(%x):vel%d\n",cmd,ch,Data0);
      break;
    case 0xe:
      ReadData2();
      s32 PitchBend=(((s32)Data1) << 7)+((s32)Data0);
      PitchBend-=8192;
//      if(ShowMessage==true) _consolePrintf("$%xx/PitchBend(%x):%d\n",cmd,ch,PitchBend);
      MTRK_ChangePitchBend(ch,PitchBend);
      break;
  }
  
#undef ReadData1
#undef ReadData2

}

static void SM_ProcSysExEventF0(bool ShowMessage,TSM_Track *pSM_Track)
{
  if(ShowMessage==true) _consolePrint("!SysExEventF0\n");
  
  TSysExEvent SysExEvent;
  
  SysExEvent.Len=SM_TrackReadVarDWord(pSM_Track);
  u8 data[256];
  
  for(u32 cnt=0;cnt<SysExEvent.Len;cnt++){
    if(cnt<256){
      data[cnt]=SM_TrackReadByte(pSM_Track);
      }else{
      SM_TrackReadByte(pSM_Track);
    }
  }
  
  // MakerID($41) , DeviceID($10) , ModelID($42) , CommandID($12) , ... CheckSum($xx) , EndOfEx($f7)
  
  if((data[0]!=0x41)||(data[1]!=0x10)||(data[2]!=0x42)||(data[3]!=0x12)){
    if(ShowMessage==true) _consolePrint("not support exc format.\n");
    return;
  }
  
  if(data[4]!=0x40){
    if(ShowMessage==true) _consolePrint("not support !send exc format.\n");
    return;
  }
  
  if(((data[5]&0xf0)==0x10)&&(data[6]==0x15)){
    const u32 blk2ch[16]={9,0,1,2,3,4,5,6,7,8,10,11,12,13,14,15};
    int ch=blk2ch[data[5]&0x0f];
    int mode=data[7];
    
    switch(mode){
      case 0: {
        _consolePrintf("Ch%d:Use For Rhythm Part to OFF\n",ch);
        MTRK_SetExMap(ch,mode);
      } break;
      case 1: {
        _consolePrintf("Ch%d:Use For Rhythm Part to Map1\n",ch);
        MTRK_SetExMap(ch,mode);
      } break;
      case 2: {
        _consolePrintf("Ch%d:Use For Rhythm Part to Map2\n",ch);
        MTRK_SetExMap(ch,mode);
      } break;
      default: {
        _consolePrintf("Ch%d:Use For Rhythm Part to Unknown(%d)\n",ch,mode);
      } break;
    }
    
    return;
  }
  
  if(ShowMessage==true){
    for(u32 cnt=0;cnt<SysExEvent.Len;cnt++){
      if(cnt<256){
        _consolePrintf("%02x,",data[cnt]);
      }
    }
    _consolePrint("\n");
  }
}

static void SM_ProcSysExEventF7(bool ShowMessage,TSM_Track *pSM_Track)
{
  if(ShowMessage==true) _consolePrint("!SysExEventF7\n");
  
  TSysExEvent SysExEvent;
  
  SysExEvent.Len=SM_TrackReadVarDWord(pSM_Track);
  
  for(u32 cnt=0;cnt<SysExEvent.Len;cnt++){
    SM_TrackReadByte(pSM_Track);
  }
}

static void SM_ProcMetaEvent_InitTempo(void)
{
  // default tempo=120
  
  TStdMIDI *_StdMIDI=&StdMIDI;
  TSM_Chank *_SM_Chank=&_StdMIDI->SM_Chank;
  
  float data;
  
  data=((float)_StdMIDI->SampleRate)/2/((float)_SM_Chank->TimeRes); // TimeBaseClock/0.5sec
  
  _StdMIDI->SamplePerClockFix16=(u32)(data*0x10000);
}

static void SM_ProcMetaEvent_SetTempo(u32 Tempo)
{
  TStdMIDI *_StdMIDI=&StdMIDI;
  TSM_Chank *_SM_Chank=&_StdMIDI->SM_Chank;
  
  float data;
  
  data=(float)Tempo; // microsec/TimeBaseClock
  data=data/((float)_SM_Chank->TimeRes); // microsec/TimeBaseClock -> microsec/Clock
  data=data/1000000; // microsec->sec
  
  _StdMIDI->SamplePerClockFix16=(u32)(data*_StdMIDI->SampleRate*0x10000);
}

void SM_ProcMetaEvent(TSM_Track *pSM_Track)
{
  TStdMIDI *_StdMIDI=&StdMIDI;
  TSM_Event SM_Event;
  
  SM_Event.MetaEvent.ID=SM_TrackReadByte(pSM_Track);
  SM_Event.MetaEvent.Len=SM_TrackReadVarDWord(pSM_Track);
  
  u32 len=SM_Event.MetaEvent.Len;
  
  if(len==0) return;
  
  SM_Event.MetaEvent.Data=(u8*)safemalloc_chkmem(&MM_Temp,len+1);
  
  for(u32 cnt=0;cnt<len;cnt++){
    SM_Event.MetaEvent.Data[cnt]=SM_TrackReadByte(pSM_Track);
  }
  SM_Event.MetaEvent.Data[len]=0;
  
  const bool ShowMessage=GlobalINI.MIDPlugin.ShowEventMessage;
  
  switch(SM_Event.MetaEvent.ID){
    case 0x00:
      if(ShowMessage==true) _consolePrint("Meta:SeqNo\n");
      break;
    case 0x01:
      if(ShowMessage==true) _consolePrintf("Meta:Text:%s\n",SM_Event.MetaEvent.Data);
      if(gME_Text==NULL) gME_Text=str_AllocateCopy(&MM_DLLSound,(char*)SM_Event.MetaEvent.Data);
      break;
    case 0x02:
      if(ShowMessage==true) _consolePrintf("Meta:Copyright:%s\n",SM_Event.MetaEvent.Data);
      if(gME_Copyright==NULL) gME_Copyright=str_AllocateCopy(&MM_DLLSound,(char*)SM_Event.MetaEvent.Data);
      break;
    case 0x03:
      if(ShowMessage==true) _consolePrintf("Meta:Title:%s\n",SM_Event.MetaEvent.Data);
      if(gME_Title==NULL) gME_Title=str_AllocateCopy(&MM_DLLSound,(char*)SM_Event.MetaEvent.Data);
      break;
    case 0x04:
      if(ShowMessage==true) _consolePrintf("Meta:InstName:%s\n",SM_Event.MetaEvent.Data);
      break;
    case 0x05:
      if(ShowMessage==true) _consolePrintf("Meta:Liq:%s\n",SM_Event.MetaEvent.Data);
      break;
    case 0x06:
      if(ShowMessage==true) _consolePrintf("Meta:Marker:%s\n",SM_Event.MetaEvent.Data);
      break;
    case 0x07:
      if(ShowMessage==true) _consolePrintf("Meta:QPoint:%s\n",SM_Event.MetaEvent.Data);
      break;
    case 0x08:
      if(ShowMessage==true) _consolePrintf("Meta:ProgramName:%s\n",SM_Event.MetaEvent.Data);
      break;
    case 0x09:
      if(ShowMessage==true) _consolePrintf("Meta:DeviceName:%s\n",SM_Event.MetaEvent.Data);
      break;
    case 0x20:
      if(ShowMessage==true) _consolePrintf("Meta:ChannelPrefix:%x\n",SM_Event.MetaEvent.Data[0]);
      break;
    case 0x21:
      if(ShowMessage==true) _consolePrintf("Meta:SetPort:%x\n",SM_Event.MetaEvent.Data[0]);
      break;
    case 0x2f:
      _consolePrint("Meta:EndTrack\n");
      pSM_Track->EndFlag=true;
      break;
    case 0x51:
      {
        u32 Tempo;
        Tempo=(((u32)SM_Event.MetaEvent.Data[0])<<16)+(((u32)SM_Event.MetaEvent.Data[1])<<8)+(((u32)SM_Event.MetaEvent.Data[2])<<0);
        if(ShowMessage==true) _consolePrintf("Meta:Tempo %dusec/%dclk\n",Tempo,_StdMIDI->SM_Chank.TimeRes);
        SM_ProcMetaEvent_SetTempo(Tempo);
      }
      break;
    case 0x54:
      if(ShowMessage==true) _consolePrint("Meta:SMPTE\n");
      break;
    case 0x58:
      if(ShowMessage==true) _consolePrint("Meta:beatup\n");
      break;
    case 0x59:
      if(ShowMessage==true) _consolePrint("Meta:beatdown\n");
      break;
    case 0x7f:
      if(ShowMessage==true) _consolePrint("Meta:SeqMetaEvent\n");
      break;
    default:
      if(ShowMessage==true) _consolePrint("Meta:UnknownMetaData\n");
      break;
  }
  
  safefree(&MM_Temp,SM_Event.MetaEvent.Data); SM_Event.MetaEvent.Data=NULL;
}

bool SM_isAllTrackEOF(void)
{
  TStdMIDI *_StdMIDI=&StdMIDI;
  
  u32 TrackCount=_StdMIDI->SM_Chank.Track;
  
  for(u32 TrackNum=0;TrackNum<TrackCount;TrackNum++){
    TSM_Track *pSM_Track=&_StdMIDI->SM_Tracks[TrackNum];
    if(pSM_Track->EndFlag==false) return(false);
  }
  
  return(true);
}

void SM_LoadStdMIDI(u8 *FilePtr,u32 SampleRate)
{
  TStdMIDI *_StdMIDI=&StdMIDI;
  
  _StdMIDI->File=FilePtr;
  _StdMIDI->FilePos=0;
  
  _StdMIDI->SampleRate=SampleRate;
  _StdMIDI->SamplePerClockFix16=0;
  
  SM_LoadChank();
  SM_ProcMetaEvent_InitTempo();
  
  if(GlobalINI.MIDPlugin.ShowInfomationMessages==true){
    TSM_Chank *_SM_Chank=&StdMIDI.SM_Chank;
    _consolePrintf("Chank:Len=%d frm=%d trk=%d res=%d\n",_SM_Chank->Len,_SM_Chank->Format,_SM_Chank->Track,_SM_Chank->TimeRes);
  }
  
  if(SM_TracksCountMax<_StdMIDI->SM_Chank.Track) _StdMIDI->SM_Chank.Track=SM_TracksCountMax;
  
  MemSet32CPU(0,&_StdMIDI->SM_Tracks,SM_TracksCountMax*sizeof(TSM_Track));
  
  u32 TrackCount=_StdMIDI->SM_Chank.Track;
  
  for(u32 TrackNum=0;TrackNum<TrackCount;TrackNum++){
    TSM_Track *pSM_Track=&_StdMIDI->SM_Tracks[TrackNum];
    SM_LoadTrackChank(pSM_Track);
//    _consolePrintf("Track.%d:length=%d\n",TrackNum,(u32)pSM_Track->DataEnd-(u32)pSM_Track->Data);
  }
}

int SM_GetDeltaTime(TSM_Track *pSM_Track)
{
  return(SM_TrackReadVarDWord(pSM_Track));
}

void SM_ProcStdMIDI(bool ShowMessage,bool EnableNote,TSM_Track *pSM_Track)
{
  u32 Status=SM_TrackReadByte(pSM_Track);
  
  if(Status<0x80){
    pSM_Track->Data--;
    Status=pSM_Track->RunningStatus;
//    _consolePrintf("RE%2x,",Status);
    SM_ProcMIDIEvent(ShowMessage,EnableNote,pSM_Track,Status);
    }else{
    if(Status<0xf0){
      pSM_Track->RunningStatus=Status;
//      _consolePrintf("Ev%2x,",Status);
      SM_ProcMIDIEvent(ShowMessage,EnableNote,pSM_Track,Status);
      }else{
//      _consolePrintf("Ex%2x,",Status);
      switch(Status){
        case 0xf0:
          SM_ProcSysExEventF0(ShowMessage,pSM_Track);
          break;
        case 0xf7:
          SM_ProcSysExEventF7(ShowMessage,pSM_Track);
          break;
        case 0xff:
          SM_ProcMetaEvent(pSM_Track);
          break;
        default:
          _consolePrint("Status Error.\n");
//          while(1);
          break;
      }
    }
  }
  
  if((u32)pSM_Track->DataEnd<=(u32)pSM_Track->Data){
    pSM_Track->EndFlag=true;
  }
}

u32 SM_GetSamplePerClockFix16(void)
{
  return(StdMIDI.SamplePerClockFix16);
}

