#ifndef MAIN_VSYNC_H_
#define MAIN_VSYNC_H_

static bool VsyncPassed;

//#include "resources/snd_click_short_c_bin.h"
//#include "resources/snd_click_long_c_bin.h"

#define snd_click_short_c_bin_Size (386)
#define snd_click_long_c_bin_Size (2756)

static u32 snd_click_short_c_bin=0;
static u32 snd_click_long_c_bin=0;

static void InterruptHandler_VBlank(void)
{
    ARM7_SelfCheck_Check();
  
    IPC6->heartbeat++;
  
    VsyncPassed=true;
    
    if(CSE_Index==CSE_Count) return;
  
    u32 data=CSE_Data[CSE_Index++];
    u32 vol=0,addr=0,size=0;
    switch(data){
        case 0: break; // wait for vblank
        case 1: {
            vol=127;
            addr=(u32)snd_click_short_c_bin;
            size=snd_click_short_c_bin_Size;
        } break;
        case 2: {
            vol=96;
            addr=(u32)snd_click_long_c_bin;
            size=snd_click_long_c_bin_Size;
        } break;
    }
  
    vol=(vol*CSE_Volume)/256;
  
    if((vol!=0)&&(addr!=0)&&(size!=0)){
        u32 ch=14;
        SCHANNEL_CR(ch)=0;
        SCHANNEL_TIMER(ch)  = SOUND_FREQ(32768);
        SCHANNEL_SOURCE(ch) = addr;
        SCHANNEL_LENGTH(ch) = size >> 2;
        SCHANNEL_REPEAT_POINT(ch) = 0;
        SCHANNEL_CR(ch)     = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(vol) | SOUND_PAN(64) | SOUND_FORMAT_8BIT;
    }
}

#endif /*MAIN_VSYNC_H_*/
