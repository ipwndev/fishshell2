#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "maindef.h"

#include "../../ipc6.h"
#include "memtoolARM7.h"
#include "a7sleep.h"

#include "plug_mp2.h"

static inline void ShowLogHalt(void)
{
    while(1);
}

#include "main_ARM7_SelfCheck.h"

//---------------------------------------------------------------------------------
static __attribute__((noinline)) void startSound(int sampleRate, const void* data, u32 bytes, u8 channel, u8 vol,  u8 pan, u8 format) {
//---------------------------------------------------------------------------------
    if((data==NULL)||(bytes==0)) return;
	
	SCHANNEL_TIMER(channel)  = SOUND_FREQ(sampleRate);
	SCHANNEL_SOURCE(channel) = (u32)data;
	SCHANNEL_LENGTH(channel) = bytes >> 2 ;
	SCHANNEL_REPEAT_POINT(channel) = 0;
	if(IPC6->LoopSound==true){
		SCHANNEL_CR(channel)     = SCHANNEL_ENABLE | SOUND_REPEAT | SOUND_VOL(vol) | SOUND_PAN(pan) | (format==1?SOUND_FORMAT_8BIT:SOUND_FORMAT_16BIT);
	}else{
		SCHANNEL_CR(channel)     = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(vol) | SOUND_PAN(pan) | (format==1?SOUND_FORMAT_8BIT:SOUND_FORMAT_16BIT);
	}
}


//---------------------------------------------------------------------------------
static __attribute__((noinline)) s32 getFreeSoundChannel() {
//---------------------------------------------------------------------------------
	int i;
	for (i=4; i<16; i++) { // 4 to 15
		if ( (SCHANNEL_CR(i) & SCHANNEL_ENABLE) == 0 ) return i;
	}
	return -1;
}

//---------------------------------------------------------------------------------
static __attribute__((noinline)) void stopSound() {
//---------------------------------------------------------------------------------
	int i;
	for (i=4; i<16; i++) { // 4 to 15
		SCHANNEL_CR(i)     = 0;
	}
}

//////////////////////////////////////////////////////////////////////

extern void CallBackIRQ_strpcmUpdate(void);

// ---------------------------

#include "fpga/romeo2.h"
#include "fpga/pcm1770.h"
#include "fpga/rawpcm.h"
#include "fpga/romeo2_hp.h"

#include "main_fpga.h"

// ---------------------------

#include "main_strpcm.h"

//////////////////////////////////////////////////////////////////////

#include "main_irq_timer.h"

//////////////////////////////////////////////////////////////////////

static u32 CSE_Volume;
static u32 CSE_Index,CSE_Count;
static u8 CSE_Data[5*60]; // max 5 secs.

static void StartClickSE(volatile TClickSE *pClickSE)
{
    CSE_Volume=pClickSE->Volume;
  
    CSE_Index=0;
    CSE_Count=0;
  
    for(u32 idx=0;idx<pClickSE->Count;idx++){
        CSE_Data[CSE_Count++]=1;
        for(u32 w=0;w<4;w++){
            CSE_Data[CSE_Count++]=0;
        }
        CSE_Count++;
    }
  
    if(pClickSE->AddLong==true) CSE_Data[CSE_Count++]=2;
  
    pClickSE->Apply=false;
}

//////////////////////////////////////////////////////////////////////

static int NDSType;
static void sys_exit(){
	//if(!bootstub_arm7){ //sadly won't be supported.
		if(NDSType>=2)writePowerManagement(0x10, 1);
		else writePowerManagement(0, PM_SYSTEM_PWR);
	//}
	//bootstub_arm7(); //won't return
}

#include "main_vsync.h"
#include "main_init.h"
#include "main_proc.h"
#include "main_boot_gbarom.h"

static ELCDPC LCDPC_LastState;

int main(int argc, char ** argv)
{
	//if(OverlayHeader_ID_CanNotAccess[0]=='\0') IPC6->UserLanguage=(u32)-1;
	
    IPC6->UserLanguage=(u32)-1;
    
    IPC6->RequestFPGAInit=false;
    IPC6->Romeo2_HPSwitch_Pressing=false;
  
    ARM7_SelfCheck_Init();
   
    main_InitAll();
    
    VsyncPassed=false;
  
    _consolePrintf("ARM7 debug log port opened.\n");
    PrintFreeMem();
  
    strpcmPlayFlag=false;
  
    LCDPC_LastState=LCDPC_ON_BOTH;

	{
		NDSType=0;
		u32 myself = readPowerManagement(4); //(PM_BACKLIGHT_LEVEL);
		if(myself & (1<<6))
			NDSType=(myself==readPowerManagement(5))?1:2;
/*
		u32 td1=touchRead(TSC_MEASURE_TEMP1);
		u32 td2=touchRead(TSC_MEASURE_TEMP2);
		if((td1==0x0fff)&&(td2==0x0fff))NDSType=2;
*/
	}
	setPowerButtonCB(sys_exit);

    //delay. 0x2f might not be initializied.
    IPC6->DSCT_SDHCFlag=*(u32*)0x2fFFC24;

    // Keep the ARM7 out of main RAM
    while (1){
        ARM7_SelfCheck_Check();
        while(VsyncPassed==false){
            swiWaitForVBlank();
            ARM7_SelfCheck_Check();
        }
        VsyncPassed=false;

		if(0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R | KEY_RIGHT | KEY_DOWN))){
			sys_exit();
		}

        main_Proc_CheckSoundCode();
        
        if(strpcmPlayFlag==false){
            REG_IME=0;
            CallBackIRQ_strpcmUpdate();
            REG_IME=1;
        }
        
        if(IPC6->strpcmControl!=ESC_NOP){
            REG_IME=0;
            main_Proc_strpcmControl();
            REG_IME=1;
        }
    
        if(IPC6->Brightness!=0xff){
            REG_IME=0;
            if(isNDSLite==true) main_Proc_Brightness(IPC6->Brightness);
            IPC6->Brightness=0xff;
            REG_IME=1;
        }
        
        if(IPC6->RequestFPGAInit==true){
            FPGA_main_ins();
            IPC6->RequestFPGAInit=false;
        }
        
        if(IPC6->RESET!=0){ // this clause is basically based on Rudolph's idea.
            //REG_IME=0;
            //reboot();
				#define ARM7_PROG (0x03810000 - 0xA00)
				void (*_menu7_Gen)();
				u32	*adr;
				u32	*buf;
				u32	i;

				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);

				REG_IME = 0;
				REG_IE = 0;
				REG_IF = REG_IF;

				REG_IPC_SYNC = 0;
				DMA0_CR = 0;
				DMA1_CR = 0;
				DMA2_CR = 0;
				DMA3_CR = 0;

				for(i=0x04000400; i<0x04000500; i+=4)
					*((u32*)i)=0;

				REG_SOUNDCNT = 0;

				//clear out ARM7 DMA channels and timers
				for(i=0x040000B0;i<(0x040000B0+0x30);i+=4)
					*((vu32*)i)=0;
				for(i=0x04000100;i<0x04000110;i+=2)
					*((u16*)i)=0;

				//copy final loader to private RAM
				adr = (u32*)ARM7_PROG;
				buf = (u32*)reboot;
				for(i = 0; i < 0x200/4; i++) {
					*adr = *buf;
					adr++;
					buf++;
				}

				//while((*(vu32*)0x02fFFDFC) != 0x02fFFDF8 && (*(vu32*)0x02fFFDFC) != 0x0cfFFDF8);	// Timing adjustment with ARM9
				_menu7_Gen = (void(*)())ARM7_PROG;
				_menu7_Gen();
            while(1);
        }
    
        bool pcapply=false;
    
        if(IPC6->LCDPowerControl!=LCDPC_NOP){
            REG_IME=0;
            LCDPC_LastState=IPC6->LCDPowerControl;
            IPC6->LCDPowerControl=LCDPC_NOP;
            pcapply=true;
            REG_IME=1;
        }
    
        static u32 LastLID=IPC_LID_CLOSED;
    
        {
            u32 LID=REG_KEYXY & IPC_LID_CLOSED;
            if(LastLID!=LID){
                LastLID=LID;
                pcapply=true;
            }
        }
     
        if(pcapply==true){
            REG_IME=0;
            //_consolePrintf("LCD apply: %d,%d.\n",LastLID,LCDPC_LastState);
            if(LastLID==IPC_LID_CLOSED){
                main_Proc_LCDPowerControl(LCDPC_OFF_BOTH);
                if(LCDPC_LastState==LCDPC_SOFT_POWEROFF){
                    main_Proc_LCDPowerControl(LCDPC_LastState);
                    while(1);
                }
            }else{ 
                main_Proc_LCDPowerControl(LCDPC_LastState);
            }
            REG_IME=1;
        }
    } 
  
    //return 0;
}

