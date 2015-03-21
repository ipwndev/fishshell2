
#define _REG_WAIT_CR (*(vuint16*)0x04000204)
#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)

static void FPGA_BusOwnerARM7_LocalSetting(void)
{
  u16 bw=_REG_WAIT_CR;
  
  bw&=~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6);
  
  // ROM cycle=4,6 , RAM cycle = 6 , PHI = 16.76MHz output , CAR = ARM7
  
  bw|=2 << 0; // 0-1  RAM-region access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=2 << 2; // 2-3  ROM 1st access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=1 << 4; // 4    ROM 2nd access cycle control   0..1=6,4 cycles def.0
  bw|=3 << 5; // 5-6  PHI-terminal output control   0..3=Lowlevel, 4.19MHz, 8.38MHZ, 16.76MHz clock output def.0
  
  _REG_WAIT_CR=bw;
}

#undef _REG_WAIT_CR
#undef BIT0
#undef BIT1
#undef BIT2
#undef BIT3
#undef BIT4
#undef BIT5
#undef BIT6

// --------------------------------------------------------------------------

static bool EnableFPGA=false;

static void FPGA_main_ins(void)
{
  EnableFPGA=false;
  
  FPGA_BusOwnerARM7_LocalSetting();
  
  u32 last=FBPCM_POS;
  while(last==FBPCM_POS);
  
  pcm1770_setup();
  
  EnableFPGA=true;
}

// --------------------------------------------------------------------------

static u32 FPGA_LastWritePos;

static u32 *pFPGA_ReadBuf;
static u32 FPGA_ReadBufPos,FPGA_ReadBufSize;

static s16 FPGA_LastL,FPGA_LastR;
static s16 FPGA_CurL,FPGA_CurR;
static u32 FPGA_CurFix16,FPGA_AddFix16;

static u32 *pFPGA_SendBuf;

static u32 FPGA_GetRequestSamplesCount(void)
{
  u32 pos=RAWPCM_POS&(RAWPCM_BUF_SIZE-1);
  
  u32 lastcnt; // 生成済み未再生サンプル数
  if(pos<FPGA_LastWritePos){
    lastcnt=FPGA_LastWritePos-pos;
    }else{
    lastcnt=RAWPCM_BUF_SIZE-pos+FPGA_LastWritePos;
  }
  
  u32 cnt=RAWPCM_BUF_SIZE-lastcnt;
  
  cnt=cnt&~1; // 念のため2samples単位でアライメントする。
  
  return(cnt);
}

static void PCM8M_FPGADAC_SendTo(u32 *psendbuf,u32 SamplesCount)
{
  u32 *pdst32=(u32*)RAWPCM_RAM;
  
  u32 wpos=FPGA_LastWritePos;
  
  for(u32 idx=0;idx<SamplesCount;idx++){
    pdst32[wpos] = *psendbuf++;
    wpos=(wpos+1)&(RAWPCM_BUF_SIZE-1);
  }
  
  FPGA_LastWritePos=wpos;
}

// --------------------------------------------------------------------------

static inline void FPGA_strpcmPlay()
{
  _consolePrintf("FPGA_strpcmPlay();\n");
  
  IPC6->IR=IR_NULL;
  
  SetMemoryMode(false);
  
  u32 strpcmFreq=IPC6->strpcmFreq;
  u32 strpcmSamples=IPC6->strpcmSamples;
  u32 strpcmChannels=IPC6->strpcmChannels;
  
  _consolePrintf("strpcm freq=%d samples=%d chs=%d\n",strpcmFreq,strpcmSamples,strpcmChannels);
  
  FPGA_LastWritePos=0;
  
  pFPGA_ReadBuf=(u32*)safemalloc(strpcmSamples*4);
  FPGA_ReadBufPos=0;
  FPGA_ReadBufSize=0;
  
  FPGA_LastL=0;
  FPGA_LastR=0;
  FPGA_CurL=0;
  FPGA_CurR=0;
  
  FPGA_CurFix16=0;
  FPGA_AddFix16=strpcmFreq*0x10000/48000;
  
  pFPGA_SendBuf=(u32*)safemalloc(RAWPCM_BUF_SIZE/2*4);

  if((pFPGA_ReadBuf==NULL)||(pFPGA_SendBuf==NULL)){
    a7led(3); while(1);
  }
  
  pcm1770_set_volume(63-0,63-0);
  pcm1770_set_volumemute(false);
  RAWPCM_Start_48kHzBypass();
//  RAWPCM_Start_VarFreq(strpcmFreq);
  
//  powerON(POWER_SOUND);
//  SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
  
  TIMER0_DATA = SOUND_FREQ(48000); // DAC直接接続RAWPCMは48kHz固定
  TIMER0_CR = TIMER_DIV_1 | TIMER_ENABLE;
  
  TIMER1_DATA = 0x10000 - (RAWPCM_BUF_SIZE/4*2); // バッファサイズの4倍の周波数でポーリングしちゃう。
  TIMER1_CR = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
  
  IPC6->strpcmWriteRequest=0;
  
  _consolePrintf("FPGA strpcm initialized.\n");
}

__attribute__((noinline)) static void FPGA_strpcmStop()
{
  pcm1770_set_volumemute(true);
  RAWPCM_Stop();
  
//  powerOFF(POWER_SOUND);
//  SOUND_CR = 0;

  TIMER0_CR = 0;
  TIMER1_CR = 0;
  
  if(pFPGA_ReadBuf!=NULL){
    safefree(pFPGA_ReadBuf); pFPGA_ReadBuf=NULL;
  }
  
  if(pFPGA_SendBuf!=NULL){
    safefree(pFPGA_SendBuf); pFPGA_SendBuf=NULL;
  }
  
  ARM7_SelfCheck_Check();
  
  SetMemoryMode_End();
  
  IPC6->IR=IR_NULL;
  
  _consolePrintf("strpcm Stopped.\n");
}

static void FPGA_InterruptHandler_Timer1(void)
{
  u32 vol=IPC6->strpcmAudioVolume64/2;
  vol=32+vol-1;
  if(63<vol) vol=63;
  pcm1770_set_volume(vol,vol);
  
  u32 reqsize=FPGA_GetRequestSamplesCount();
  
  if(reqsize==0){
    CallBackIRQ_strpcmUpdate();
    return;
  }
  
  //u32 strpcmFreq=IPC6->strpcmFreq;
  u32 strpcmSamples=IPC6->strpcmSamples;
  //u32 strpcmChannels=IPC6->strpcmChannels;
  
  u32 *preadbuf=pFPGA_ReadBuf;
  u32 readpos=FPGA_ReadBufPos;
  u32 readsize=FPGA_ReadBufSize;

  s16 lastl=FPGA_LastL,lastr=FPGA_LastR;
  s16 curl=FPGA_CurL,curr=FPGA_CurR;
    
  u32 curfix=FPGA_CurFix16,addfix=FPGA_AddFix16;
  
  u32 *psendbuf=pFPGA_SendBuf;
  
  for(u32 idx=0;idx<reqsize;idx++){
    curfix+=addfix;
    if(0x10000<=curfix){
      curfix-=0x10000;
      
      lastl=curl; lastr=curr;
      u32 tmp=preadbuf[readpos];
      curl=(tmp>> 0)&0xffff;
      curr=(tmp>>16)&0xffff;
      
      readpos++;
      if(readpos==strpcmSamples){
        IPC6->strpcmWriteRequest=1;
        IPC6->IR=IR_NextSoundData;
        REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
        while(IPC6->strpcmWriteRequest!=0);
        
        readpos=0;
        readsize=strpcmSamples;
        
        u32 *psrcl=IPC6->strpcmLRBuf;
        for(u32 idx=0;idx<readsize;idx++){
          preadbuf[idx]=psrcl[idx];
        }
      }
    }
    u32 cf=curfix>>8;
    s16 l=(lastl*(0x100-cf)/0x100)+(curl*cf/0x100);
    s16 r=(lastr*(0x100-cf)/0x100)+(curr*cf/0x100);
    psendbuf[idx]=(l&0xffff) | ((r&0xffff)<<16);
  }
  
  FPGA_ReadBufPos=readpos;
  FPGA_ReadBufSize=readsize;
  
  FPGA_LastL=lastl;
  FPGA_LastR=lastr;
  
  FPGA_CurL=curl;
  FPGA_CurR=curr;
  
  FPGA_CurFix16=curfix;
  
  PCM8M_FPGADAC_SendTo(psendbuf,reqsize);
  
  CallBackIRQ_strpcmUpdate();
}

static inline bool FPGA_GetHPSwitch(void)
{
  if(EnableFPGA==true){ 
    if(hp_sense_switch()==HP_SWITCH_OFF){
      return(false);
      }else{
      return(true);
    }
  }
  
  return(false);
}

