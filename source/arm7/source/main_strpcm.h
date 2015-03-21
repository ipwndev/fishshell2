#ifndef MAIN_STRPCM_H_
#define MAIN_STRPCM_H_

static u32 strpcmCursorFlag=0;

static u32 strpcmFreq,strpcmSamples,strpcmChannels;
static EstrpcmFormat strpcmFormat=(EstrpcmFormat)0;

static s16 *strpcmL0=NULL,*strpcmL1=NULL,*strpcmR0=NULL,*strpcmR1=NULL;
static s32 strpcmLastL,strpcmLastR;

static s32 agc;

#undef SOUND_FREQ
#define SOUND_FREQ(n)	((vu16)(0x10000 - (0x1000000 / (n))))

static inline void strpcmPlay()
{
    _consolePrintf("strpcmPlay();\n");
  
    IPC6->IR=IR_NULL;
  
    strpcmCursorFlag=0;
  
    strpcmFormat=IPC6->strpcmFormat;
  
    switch(strpcmFormat){
        case SPF_PCMx1: case SPF_PCMx2: case SPF_PCMx4: SetMemoryMode(false); break;
        case SPF_MP2: SetMemoryMode(false); break;
        default: {
            _consolePrintf("Unknown strpcm format.\n");
            ShowLogHalt();
        } break;
    }
  
    switch(strpcmFormat){
        case SPF_PCMx1: case SPF_PCMx2: case SPF_PCMx4: {
        } break;
        case SPF_MP2: {
            ARM7_SelfCheck_Check();
            if(StartMP2()==false){ a7led(3); while(1); }
            ARM7_SelfCheck_Check();
            IPC6->strpcmSamples=MP2_GetSamplePerFrame();
            IPC6->strpcmChannels=MP2_GetChannelCount();
            // Ignore MP2 stream sample rate. set from ARM9 DPGINFO.SndFreq
            // IPC6->strpcmFreq=MP2_GetSampleRate();
            ARM7_SelfCheck_Check();
        } break;
        default: {
            _consolePrintf("Unknown strpcm format.\n");
            ShowLogHalt();
        } break;
    }
  
    strpcmFreq=IPC6->strpcmFreq;
    strpcmSamples=IPC6->strpcmSamples;
    strpcmChannels=IPC6->strpcmChannels;
  
    switch(strpcmFormat){
        case SPF_PCMx1: {
        } break;
        case SPF_PCMx2: {
            strpcmFreq*=2;
            strpcmSamples*=2;
        } break;
        case SPF_PCMx4: {
            strpcmFreq*=4;
            strpcmSamples*=4;
        } break;
        case SPF_MP2: {
        } break;
		default: break;
    }
  
    _consolePrintf("strpcm freq=%d samples=%d chs=%d\n",strpcmFreq,strpcmSamples,strpcmChannels);
  
    strpcmL0=(s16*)safemalloc(strpcmSamples*2);
    strpcmL1=(s16*)safemalloc(strpcmSamples*2);
    strpcmR0=(s16*)safemalloc(strpcmSamples*2);
    strpcmR1=(s16*)safemalloc(strpcmSamples*2);
  
    _consolePrintf("strpcm buf 0x%x,0x%x 0x%x,0x%x\n",strpcmL0,strpcmL1,strpcmR0,strpcmR1);
  
    if((strpcmL0==NULL)||(strpcmL1==NULL)||(strpcmR0==NULL)||(strpcmR1==NULL)){
        a7led(3); while(1);
    }
  
    strpcmLastL=0;
    strpcmLastR=0;
  
//  powerON(POWER_SOUND);
//  SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
    SCHANNEL_CR(0) = 0;
    SCHANNEL_CR(1) = 0;
    SCHANNEL_CR(2) = 0;
    SCHANNEL_CR(3) = 0;
  
    TIMER0_DATA = SOUND_FREQ(strpcmFreq);
    TIMER0_CR = TIMER_DIV_1 | TIMER_ENABLE;
  
    TIMER1_DATA = 0x10000 - (strpcmSamples*2);
    TIMER1_CR = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
  
    for(u32 ch=0;ch<4;ch++){
        SCHANNEL_CR(ch) = 0;
        SCHANNEL_TIMER(ch) = SOUND_FREQ(strpcmFreq);
        SCHANNEL_LENGTH(ch) = (strpcmSamples*2) >> 2;
        SCHANNEL_REPEAT_POINT(ch) = 0;
    }

    agc=0;
    
    IPC6->strpcmWriteRequest=0;
  
    _consolePrintf("strpcm initialized.\n");
}

__attribute__((noinline)) static void strpcmStop()
{
    _consolePrintf("strpcm Stop: strpcm buf 0x%x,0x%x 0x%x,0x%x\n",strpcmL0,strpcmL1,strpcmR0,strpcmR1);
  
//  powerOFF(POWER_SOUND);
//  SOUND_CR = 0;
    TIMER0_CR = 0;
    TIMER1_CR = 0;
  
    for(u32 ch=0;ch<4;ch++){
        SCHANNEL_CR(ch) = 0;
    }
  
    if(strpcmL0!=NULL){
        safefree(strpcmL0); strpcmL0=NULL;
    }
    if(strpcmL1!=NULL){
        safefree(strpcmL1); strpcmL1=NULL;
    }
    if(strpcmR0!=NULL){
        safefree(strpcmR0); strpcmR0=NULL;
    }
    if(strpcmR1!=NULL){
        safefree(strpcmR1); strpcmR1=NULL;
    }
  
    switch(strpcmFormat){
        case SPF_PCMx1: case SPF_PCMx2: case SPF_PCMx4: break;
        case SPF_MP2: FreeMP2(); break;
        default: break;
    }
    strpcmFormat=(EstrpcmFormat)0;
    ARM7_SelfCheck_Check();
    
    a7led(0);
    SetMemoryMode_End();
  
    IPC6->IR=IR_NULL;
  
    _consolePrintf("strpcm Stopped.\n");
}

#endif /*MAIN_STRPCM_H_*/
