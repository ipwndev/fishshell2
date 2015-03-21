
#define RCP_TrackHeaderSize (44)

#define RCP_Track_LoopMax (32)

static TRCP *pRCP;
static TRCP_Chank *pRCP_Chank;

typedef struct {
  char Memo[24+1];
  u8 Data[24];
} TUserExc;

static TUserExc UserExc[8];

// -----------------------------------------------------------

static void RCP_Init(void)
{
  pRCP=(TRCP*)safemalloc_chkmem(&MM_DLLSound,sizeof(TRCP));
  MemSet32CPU(0,pRCP,sizeof(TRCP));
  
  pRCP->File=NULL;
  pRCP->FilePos=0;
  
  pRCP->FastNoteOn=false;
  
  pRCP->SampleRate=0;
  pRCP->SamplePerClockFix16=0;
  
  for(u32 trk=0;trk<RCPTrackMax;trk++){
    pRCP->RCP_Track[trk].pLoop=NULL;
    pRCP->RCP_Track[trk].pExc=NULL;
  }
  
  pRCP_Chank=(TRCP_Chank*)safemalloc_chkmem(&MM_DLLSound,sizeof(TRCP_Chank));
  MemSet32CPU(0,pRCP_Chank,sizeof(TRCP_Chank));
  
  {
    pRCP_Chank->Title[0]=0;
    pRCP_Chank->Memo[0]=0;
    pRCP_Chank->TimeRes=0;
    pRCP_Chank->Tempo=0;
    pRCP_Chank->PlayBias=0;
    pRCP_Chank->TrackCount=0;
  }
}

static void RCP_Free(void)
{
  if(pRCP!=NULL){
    for(u32 trk=0;trk<RCPTrackMax;trk++){
      if(pRCP->RCP_Track[trk].pLoop!=NULL){
        safefree(&MM_DLLSound,pRCP->RCP_Track[trk].pLoop); pRCP->RCP_Track[trk].pLoop=NULL;
      }
      if(pRCP->RCP_Track[trk].pExc!=NULL){
        safefree(&MM_DLLSound,pRCP->RCP_Track[trk].pExc); pRCP->RCP_Track[trk].pExc=NULL;
      }
    }
    safefree(&MM_DLLSound,pRCP); pRCP=NULL;
  }
  
  if(pRCP_Chank!=NULL){
    safefree(&MM_DLLSound,pRCP_Chank); pRCP_Chank=NULL;
  }
}

static inline void RCP_ReadSkip(int size)
{
  pRCP->FilePos+=size;
}

static inline void RCP_SetPos(int pos)
{
  pRCP->FilePos=pos;
}

static inline u8 RCP_ReadByte(void)
{
  u8 res=pRCP->File[pRCP->FilePos];
  
  pRCP->FilePos++;
  
  return(res);
}

static inline u16 RCP_ReadWord(void)
{
  u16 res;
  
  res=(u16)RCP_ReadByte();
  res|=((u16)RCP_ReadByte())<<8;
  
  return(res);
}

static void RCP_LoadChank(void)
{
  RCP_SetPos(0x0020);
  for(u32 idx=0;idx<64;idx++){
    pRCP_Chank->Title[idx]=(char)RCP_ReadByte();
  }
  pRCP_Chank->Title[64]=0;
  
  RCP_SetPos(0x0060);
  for(u32 idx=0;idx<336;idx++){
    pRCP_Chank->Memo[idx]=(char)RCP_ReadByte();
  }
  pRCP_Chank->Memo[336]=0;
  
  {
    RCP_SetPos(0x01c0);
    u32 TimeBaseLow=(u32)RCP_ReadByte();
    RCP_SetPos(0x01e7);
    u32 TimeBaseHigh=(u32)RCP_ReadByte();
    
    pRCP_Chank->TimeRes=(TimeBaseHigh<<8) | TimeBaseLow;
  }
  
  RCP_SetPos(0x01c1);
  pRCP_Chank->Tempo=(u32)RCP_ReadByte();
  
  RCP_SetPos(0x01c5);
  pRCP_Chank->PlayBias=(s32)((s8)RCP_ReadByte());
  
  RCP_SetPos(0x01e6);
  pRCP_Chank->TrackCount=(u32)RCP_ReadByte();
  if(pRCP_Chank->TrackCount==0) pRCP_Chank->TrackCount=18;
  if(RCPTrackMax<pRCP_Chank->TrackCount) pRCP_Chank->TrackCount=RCPTrackMax;
}

static void RCP_LoadUserExc(void)
{
  RCP_SetPos(0x0406);
  
  for(u32 ExcID=0;ExcID<8;ExcID++){
    TUserExc *pUserExc=&UserExc[ExcID];
    for(u32 idx=0;idx<24;idx++){
      pUserExc->Memo[idx]=(char)RCP_ReadByte();
    }
    pUserExc->Memo[24]=0;
    for(u32 idx=0;idx<24;idx++){
      pUserExc->Data[idx]=RCP_ReadByte();
    }
  }
}

static void RCP_LoadTrackChank(TRCP_Track *pRCP_Track)
{
  pRCP_Track->EndFlag=false;
  
  pRCP_Track->DataTop=&pRCP->File[pRCP->FilePos];
  
  pRCP_Track->unuseTrackLen=(u32)RCP_ReadWord();
  pRCP_Track->unuseTrackNum=(u32)RCP_ReadByte();
  if(RCP_ReadByte()==0x80){
    pRCP_Track->RythmMode=true;
    }else{
    pRCP_Track->RythmMode=false;
  }
  
  pRCP_Track->MIDICh=(u32)RCP_ReadByte();
  
  if(pRCP_Track->MIDICh<0x20){
    pRCP_Track->MIDICh&=0x0f;
    }else{
    pRCP_Track->MIDICh=0xff;
  }
  
  pRCP_Track->KeyBias=(s32)RCP_ReadByte();
  pRCP_Track->unuseStBias=(s32)((s8)RCP_ReadByte());
  pRCP_Track->PlayMode=(ERCPTrackPlayMode)RCP_ReadByte();
  
/*
  for(int idx=0;idx<36;idx++){
    pRCP_Track->Comment[idx]=(char)RCP_ReadByte();
  }
  pRCP_Track->Comment[36]=0;
*/
  for(int idx=0;idx<36;idx++){
    RCP_ReadByte();
  }
  
  pRCP_Track->Data=&pRCP_Track->DataTop[RCP_TrackHeaderSize];
  pRCP_Track->DataEnd=&pRCP->File[pRCP->FilePos+pRCP_Track->unuseTrackLen-RCP_TrackHeaderSize];
  RCP_ReadSkip(pRCP_Track->unuseTrackLen-RCP_TrackHeaderSize);
  
  pRCP_Track->WaitClock=(u32)(0x80+pRCP_Track->unuseStBias);
  pRCP_Track->pReturnSameMeasure=NULL;
  
  pRCP_Track->LoopFreeIndex=0;
  pRCP_Track->pLoop=(TRCP_Track_Loop*)safemalloc_chkmem(&MM_DLLSound,sizeof(TRCP_Track_Loop)*RCP_Track_LoopMax);
  for(u32 idx=0;idx<RCP_Track_LoopMax;idx++){
    TRCP_Track_Loop *pLoop=&pRCP_Track->pLoop[idx];
    pLoop->pReturn=NULL;
    pLoop->LoopCount=0;
  }
  
  pRCP_Track->pExc=(TRCP_Track_Exc*)safemalloc_chkmem(&MM_DLLSound,sizeof(TRCP_Track_Exc));
  
  TRCP_Track_Exc *pExc=pRCP_Track->pExc;
  
  pExc->cmd=EEC_None;
  pExc->GT=0;
  pExc->Vel=0;
  pExc->Count=0;
  for(u32 idx=0;idx<128;idx++){
    pExc->Buf[idx]=0;
  }
}

#define RCP_TrackReadByte(pRCP_Track) (*pRCP_Track->Data++)

static void RCP_Proc_NoteOn(bool ShowMessage,bool EnableNote,TRCP_Track *pRCP_Track,u32 NoteNum,u32 GateTime,u32 Vel)
{
  if((NoteNum==0)||(Vel==0)||(GateTime==0)) return;
  
  pRCP->FastNoteOn=true;
  
  if(pRCP_Track->KeyBias<0x80){
    s32 TransNote=(s32)NoteNum;
    
    if(pRCP_Track->KeyBias<0x40){
      TransNote+=pRCP_Track->KeyBias;
      }else{
      TransNote-=0x80-pRCP_Track->KeyBias;
    }
    
    TransNote+=pRCP_Chank->PlayBias;
    
    if((TransNote<0)||(128<=TransNote)) return;
    
    NoteNum=(u32)TransNote;
  }
  
  if(EnableNote==false){
    MTRK_NoteOn_LoadProgram(pRCP_Track->MIDICh,NoteNum,Vel);
    }else{
    MTRK_NoteOn(pRCP_Track->MIDICh,GateTime,NoteNum,Vel);
  }
}

static void RCP_Proc_LoopStart(TRCP_Track *pRCP_Track)
{
  if((pRCP_Track->LoopFreeIndex+1)==RCP_Track_LoopMax){
    _consolePrint("Loop nest buffer overflow.\n");
    return;
  }
  
  TRCP_Track_Loop *pLoop=&pRCP_Track->pLoop[pRCP_Track->LoopFreeIndex];
  pLoop->pReturn=pRCP_Track->Data;
  pLoop->LoopCount=0xffffffff;
  
  pRCP_Track->LoopFreeIndex++;
}

static void RCP_Proc_LoopEnd(TRCP_Track *pRCP_Track,u32 LoopCount)
{
  if(250<=LoopCount) LoopCount=0; // 250回以上は無限ループ…にしてみた。
  
  if(pRCP_Track->LoopFreeIndex==0){
    _consolePrint("Loop nest not found.\n");
    return;
  }
  
  TRCP_Track_Loop *pLoop=&pRCP_Track->pLoop[pRCP_Track->LoopFreeIndex-1];
  
  if(pLoop->LoopCount==0xffffffff){
    if(LoopCount==0){
      _consolePrintf("Infinite loop was not supported.\n");
      LoopCount=2;
    }
    pLoop->LoopCount=LoopCount-1;
  }
  
  if(pLoop->LoopCount!=0){
    pLoop->LoopCount--;
    pRCP_Track->Data=pLoop->pReturn;
    return;
  }
  
  pLoop->pReturn=NULL;
  pLoop->LoopCount=0;
  
  pRCP_Track->LoopFreeIndex--;
}

static void RCP_ProcTempo(u32 _TempoFactor)
{
  pRCP->TempoFactor=_TempoFactor;
  
  u32 CurTempo=pRCP_Chank->Tempo*pRCP->TempoFactor/64;
  
  if(CurTempo==0){
    _consolePrint("Tempo setting error.\n");
    pRCP->SamplePerClockFix16=120*0x10000;
    return;
  }
  
  u32 spc;
  
  spc=(pRCP->SampleRate*0x10000)/pRCP_Chank->TimeRes;
  spc=spc*60/CurTempo;
  
  pRCP->SamplePerClockFix16=spc;
  
//  _consolePrintf("SamplePerClockFix16=%x\n",pRCP->SamplePerClockFix16);
}

bool RCP_isAllTrackEOF(void)
{
  u32 TrackCount=pRCP_Chank->TrackCount;
  
  for(u32 TrackNum=0;TrackNum<TrackCount;TrackNum++){
    TRCP_Track *pRCP_Track=&pRCP->RCP_Track[TrackNum];
    if(pRCP_Track->EndFlag==false) return(false);
  }
  
  return(true);
}

static void RCP_LoadRCP(u8 *FilePtr,u32 SampleRate)
{
  pRCP->File=FilePtr;
  pRCP->FilePos=0;
  
  pRCP->TempoFactor=64;
  
  pRCP->SampleRate=SampleRate;
  pRCP->SamplePerClockFix16=0;
  
  RCP_LoadChank();
  
  {
//    _consolePrintf("Title:%0.24s\n",pRCP_Chank->Title);
//    _consolePrintf("Memo:%0.24s\n",pRCP_Chank->Memo);
    _consolePrintf("TimeRes=%d Tempo=%d PlayBias=%d TrackCount=%d\n",pRCP_Chank->TimeRes,pRCP_Chank->Tempo,pRCP_Chank->PlayBias,pRCP_Chank->TrackCount);
  }
  
  RCP_LoadUserExc();
  
  RCP_SetPos(0x0586);
  
  u32 TrackCount=pRCP_Chank->TrackCount;
  
  MemSet32CPU(0,pRCP->RCP_Track,TrackCount*sizeof(TRCP_Track));
  
  for(u32 TrackNum=0;TrackNum<TrackCount;TrackNum++){
    TRCP_Track *pRCP_Track=&pRCP->RCP_Track[TrackNum];
    RCP_LoadTrackChank(pRCP_Track);
    _consolePrintf("T%2d(%5d):R%d,Ch%2x,KB%d,SB%d,PM%d\n",TrackNum,(u32)pRCP_Track->DataEnd-(u32)pRCP_Track->Data,pRCP_Track->RythmMode,pRCP_Track->MIDICh,pRCP_Track->KeyBias,pRCP_Track->unuseStBias,pRCP_Track->PlayMode);
  }
  
  RCP_ProcTempo(pRCP->TempoFactor);
}

static void ExcBufClear(TRCP_Track_Exc *pExc)
{
  pExc->Count=0;
  for(u32 idx=0;idx<128;idx++){
    pExc->Buf[idx]=0;
  }
}

static void ExcStart(TRCP_Track_Exc *pExc,ERCPExcCmd _ExcCmd,u32 GateTime,u32 Vel)
{
  pExc->cmd=_ExcCmd;
  
  if(pExc->cmd!=EEC_CHExclusive){
    pExc->cmd=EEC_None;
    return;
  }
  
  ExcBufClear(pExc);
  
  pExc->GT=(u8)GateTime;
  pExc->Vel=(u8)Vel;
}

static void ExcSend(bool ShowMessage,u32 ch,TRCP_Track_Exc *pExc)
{
  if(pExc->cmd==EEC_None) return;
  
  u8 data[128];
  u32 size=0;
  
  for(u32 idx=0;idx<pExc->Count;idx++){
    u8 src=pExc->Buf[idx];
    switch(src){
      case 0x80: data[size++]=pExc->GT; break;
      case 0x81: data[size++]=pExc->Vel; break;
      case 0x82: data[size++]=ch; break; // MIDI CH
      case 0x83: break; // ClearCS
      case 0x84: data[size++]=0; break; // CS
      default: data[size++]=src;
    }
  }
  
  ExcBufClear(pExc);
  
  if((data[0]!=0x41)||(data[1]!=0x10)||(data[2]!=0x42)||(data[3]!=0x12)){
    if(ShowMessage==true) _consolePrint("not support pExc-> format.\n");
    return;
  }
  
  if(data[4]!=0x40){
    if(ShowMessage==true) _consolePrint("not support !send pExc-> format.\n");
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
    for(u32 cnt=0;cnt<size;cnt++){
      if(cnt<256){
        _consolePrintf("%02x,",data[cnt]);
      }
    }
    _consolePrint("\n");
  }
}

static void ExcAdd(bool ShowMessage,u32 ch,TRCP_Track_Exc *pExc,u32 data0,u32 data1)
{
  if(pExc->cmd==EEC_None) return;
  
  if((pExc->Count+2)<128){
    pExc->Buf[pExc->Count+0]=(u8)data0;
    pExc->Buf[pExc->Count+1]=(u8)data1;
    pExc->Count+=2;
  }
  
  if((data0==0xf7)||(data1==0xf7)) ExcSend(ShowMessage,ch,pExc);
}

static void UserExcSend(bool ShowMessage,u32 ch,TRCP_Track_Exc *pExc,u32 ExcID,u32 GateTime,u32 Vel)
{
  TUserExc *pUserExc=&UserExc[ExcID];
  
  ExcStart(pExc,EEC_CHExclusive,GateTime,Vel);
  
  for(u32 idx=0;idx<24;idx+=2){
    ExcAdd(ShowMessage,ch,pExc,pUserExc->Data[idx+0],pUserExc->Data[idx+1]);
  }
  
  ExcSend(ShowMessage,ch,pExc);
}

static void RCP_ProcRCP(bool ShowMessage,bool EnableNote,TRCP_Track *pRCP_Track)
{
  u32 ch=pRCP_Track->MIDICh;
  
  if(ch==0xff){
    pRCP_Track->EndFlag=true;
    return;
  }
  
  if(pRCP_Track->PlayMode==ERCPTPM_Mute){
    pRCP_Track->EndFlag=true;
    return;
  }
  
  u32 cmd,step,data0,data1;
  
  cmd=(u32)RCP_TrackReadByte(pRCP_Track);
  step=(u32)RCP_TrackReadByte(pRCP_Track);
  data0=(u32)RCP_TrackReadByte(pRCP_Track);
  data1=(u32)RCP_TrackReadByte(pRCP_Track);
  
  // Command referrence from CVS.EXE Ver.5.06 95/08/29
  
  if(cmd<128){ // NoteOn
    RCP_Proc_NoteOn(ShowMessage,EnableNote,pRCP_Track,cmd,data0,data1);
    }else{
//    _consolePrintf("%d,%02x:%02x:%02x:%02x\n",ch,cmd,step,data0,data1);
    if((0x90<=cmd)&&(cmd<=0x97)){
      // User Exclusive 1 | 90 (F0) Cf.1|     Step     | Ex Data (gt) | Ex Data (vel)
      UserExcSend(ShowMessage,ch,pRCP_Track->pExc,cmd-0x90,data0,data1);
      }else{
      // CH Exclusive     | 98          |     Step     | Ex Data (gt) | Ex Data (vel)
      // 外部CMDの実行    | 99  <Cf.3>  |      -       | Command Type |      -       
      // Comment Start    | F6          |      -       | Ascii Code 1 | Ascii Code 2 
      ERCPExcCmd ExcCmd=(ERCPExcCmd)cmd;
      if((ExcCmd==EEC_CHExclusive)||(ExcCmd==EEC_Exec)||(ExcCmd==EEC_Commment)){
        ExcStart(pRCP_Track->pExc,ExcCmd,data0,data1);
        if(ExcCmd!=EEC_CHExclusive) step=0;
        }else{
        // 2nd Event        | F7  <Cf.2>  |      -       | Ex Data 1    | Ex Data 2    
        if(cmd==0xf7){
          ExcAdd(ShowMessage,ch,pRCP_Track->pExc,data0,data1);
          step=0;
          }else{
          switch(cmd){
            case 0xe2: { // Bank & Program   | E2 (Cn & Bn)|     Step     | Prog. Change | Bank Select  
              _consolePrintf("Bank & Program(%x):p%d,b%d\n",ch,data0,data1);
              MTRKCC_Proc(ch,0,data1); // SetVariation
              MTRK_SetProgram(ch,data0);
            } break;
            case 0xe5: { // Key Scan         | E5          |     Step     | Scan Trk No. |      -       
              if(ShowMessage==true) _consolePrintf("Key Scan(%x):%d\n",ch,data0);
            } break;
            case 0xe6: { // Midi CH Change   | E6  <Cf.5>  |     Step     | CH No.       |      -       
              if(ShowMessage==true) _consolePrintf("Midi CH Change(%x):%d\n",ch,data0);
              u32 ch=data0;
              if(ch==0){
                ch=0xff;
                }else{
                ch--;
                if(ch<0x20){
                  ch&=0x0f;
                  }else{
                  ch=0xff;
                }
              }
              pRCP_Track->MIDICh=ch;
            } break;
            case 0xe7: { // Tempo   Change   | E7          |     Step     | Prameter     | Graduation   
              if(ShowMessage==true) _consolePrintf("Tempo Change(%x):%d,%d\n",ch,data0,data1);
              RCP_ProcTempo(data0);
            } break;
            case 0xea: { // After Touch (CH) | EA  (Dn)    |     Step     | Parameter    |      -       
//              if(ShowMessage==true) _consolePrintf("After Touch(%x):%d\n",ch,data0);
            } break;
            case 0xeb: { // Control Change   | EB  (Bn)    |     Step     | Control No.  | Parameter    
              if(ShowMessage==true) _consolePrintf("Control Change(%x):%d,%d\n",ch,data0,data1);
              MTRKCC_Proc(ch,data0,data1);
            } break;
            case 0xec: { // Program Change   | EC  (Cn)    |     Step     | Program No.  |      -       
              _consolePrintf("Program Change(%x):p%d\n",ch,data0);
              MTRK_SetProgram(ch,data0);
            } break;
            case 0xed: { // After Touch Pori.| ED  (An)    |     Step     | Key No.      | Parameter    
//              if(ShowMessage==true) _consolePrintf("After Touch Pori.:note%d vel%d\n",ch,Data0,Data1);
            } break;
            case 0xee: { // Pitch Bend       | EE  (En)    |     Step     | Parameter 1  | Parameter 2  
              s32 PitchBend=(s32)(((data1&0x7f)<<7)|(data0&0x7f));
              PitchBend-=8192;
//              if(ShowMessage==true) _consolePrintf("Pitch Bend(%x):%d\n",ch,PitchBend);
              MTRK_ChangePitchBend(ch,PitchBend);
            } break;
            case 0xd0: { // YAMAHA Base      | D0          |     Step     | Base Addr1   | Base Addr2   
            } break;
            case 0xd1: { // YAMAHA Dev       | D1          |     Step     | Device ID    | Model ID     
            } break;
            case 0xd2: { // YAMAHA Addr/Para | D2          |     Step     | Base Addr3   | Parameter    
            } break;
            case 0xd3: { // YAMAHA XG Ad/Para| D3          |     Step     | Base Addr3   | Parameter    
            } break;
            case 0xdd: { // Roland Base      | DD          |     Step     | Base Address | Base Address 
            } break;
            case 0xde: { // Roland Para      | DE          |     Step     | Offset Addr. | Parameter    
            } break;
            case 0xdf: { // Roland Device    | DF          |     Step     | Device ID    | Model ID     
            } break;
            case 0xf5: { // Key Change       | F5          | Key Data     |      -       |      -       
              if(ShowMessage==true) _consolePrintf("Key Change(%x):%d,%d,%d\n",ch,step,data0,data1);
              step=0;
            } break;
            case 0xf8: { // Loop End         | F8          | Loop Count   |      -       |      -       
              RCP_Proc_LoopEnd(pRCP_Track,step);
              step=0;
            } break;
            case 0xf9: { // Loop Start       | F9  <Cf.7>  |      -       |      -       |      -       
              RCP_Proc_LoopStart(pRCP_Track);
              step=0;
            } break;
            case 0xfc: { // Same Measure     | FC  <Cf.8>  | Same Meas No.| Offset Low   | Offset High  
              if(pRCP_Track->pReturnSameMeasure!=NULL){
                pRCP_Track->Data=pRCP_Track->pReturnSameMeasure;
                pRCP_Track->pReturnSameMeasure=NULL;
                }else{
                pRCP_Track->pReturnSameMeasure=pRCP_Track->Data;
                pRCP_Track->Data=&pRCP_Track->DataTop[(data0 & 0xFC) | (data1 << 8)];
              }
              step=0;
            } break;
            case 0xfd: { // Mesure End       | FD          |      -       |      -       |      -       
              if(pRCP_Track->pReturnSameMeasure!=NULL){
                pRCP_Track->Data=pRCP_Track->pReturnSameMeasure;
                pRCP_Track->pReturnSameMeasure=NULL;
              }
              step=0;
            } break;
            case 0xfe: { // End of Track     | FE          |      -       |      -       |      -       
              pRCP_Track->EndFlag=true;
              step=0;
            } break;
            case 0xc0: { // DX7 Function     | C0          |     Step     |  Parameter   |     Data     
            } break;
            case 0xc1: { // DX  Parameter    | C1          |     Step     |  Parameter   |     Data     
            } break;
            case 0xc2: { // DX  RERF         | C2          |     Step     |  Parameter   |     Data     
            } break;
            case 0xc3: { // TX  Function     | C3          |     Step     |  Parameter   |     Data     
            } break;
            case 0xc5: { // FB-01 P Parameter| C5          |     Step     |  Parameter   |     Data     
            } break;
            case 0xc6: { // FB-01 S System   | C6          |     Step     |  Parameter   |     Data     
            } break;
            case 0xc7: { // TX81Z V VCED     | C7          |     Step     |  Parameter   |     Data     
            } break;
            case 0xc8: { // TX81Z A ACED     | C8          |     Step     |  Parameter   |     Data     
            } break;
            case 0xc9: { // TX81Z P PCED     | C9          |     Step     |  Parameter   |     Data     
            } break;
            case 0xca: { // TX81Z S Sysytem  | CA          |     Step     |  Parameter   |     Data     
            } break;
            case 0xcb: { // TX81Z E EFFECT   | CB          |     Step     |  Parameter   |     Data     
            } break;
            case 0xcc: { // DX7-2 R REMOTE SW| CC          |     Step     |  Parameter   |     Data     
            } break;
            case 0xcd: { // DX7-2 A ACED     | CD          |     Step     |  Parameter   |     Data     
            } break;
            case 0xce: { // DX7-2 P PCED     | CE          |     Step     |  Parameter   |     Data     
            } break;
            case 0xcf: { // TX802 P PCED     | CF          |     Step     |  Parameter   |     Data     
            } break;
            case 0xdc: { // MKS-7            | DC          |     Step     |  Parameter   |     Data     
            } break;
            default: {
            } break;
          }
        }
      }
    }
  }
  
  pRCP_Track->WaitClock+=step;
  
  if((u32)pRCP_Track->DataEnd<=(u32)pRCP_Track->Data){
    pRCP_Track->EndFlag=true;
  }
}

u32 RCP_GetSamplePerClockFix16(void)
{
  return(pRCP->SamplePerClockFix16);
}

const TRCP* RCP_GetStruct_RCP(void)
{
  return(pRCP);
}

const TRCP_Chank* RCP_GetStruct_RCP_Chank(void)
{
  return(pRCP_Chank);
}

