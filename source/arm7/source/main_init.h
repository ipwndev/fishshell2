#ifndef MAIN_INIT_H_
#define MAIN_INIT_H_

static void SNDShortAddressHandler( void * address, void * userdata ) {
    snd_click_short_c_bin=(u32)address;
    _consolePrintf("snd_click_short_c_bin=0x%x\n",snd_click_short_c_bin);
    fifoSetAddressHandler(FIFO_USER_01, NULL, 0);
}

static void SNDLongAddressHandler( void * address, void * userdata ) {
    snd_click_long_c_bin=(u32)address;
    _consolePrintf("snd_click_long_c_bin=0x%x\n",snd_click_long_c_bin);
    fifoSetAddressHandler(FIFO_USER_02, NULL, 0);
}

//---------------------------------------------------------------------------------
static void i2cIRQHandler() {
//---------------------------------------------------------------------------------
	//int cause = (i2cReadRegister(I2C_PM, 0x10) & 0x3) | (i2cReadRegister(I2C_UNK4, 0x02)<<2);
	
	//if (cause & 1) exitflag = true;
}

//////////////////////////////////////////////////////////////////////

static inline void main_InitIRQ(void)
{
    REG_IME = 0;
    irqInit();
    
    IPC6->StartFlagARM7 = 1;
    while(IPC6->StartFlagARM9==0);
    fifoInit();
    
    installSystemFIFO();
    
    irqSet(IRQ_TIMER1,InterruptHandler_Timer1_Null);
    irqSet(IRQ_VBLANK,InterruptHandler_VBlank);
    irqSetAUX(IRQ_I2C, i2cIRQHandler);
    irqEnableAUX(IRQ_I2C);
    	
    irqEnable( IRQ_VBLANK | IRQ_TIMER1 | IRQ_NETWORK);
      
    fifoSetAddressHandler(FIFO_USER_01, SNDShortAddressHandler, 0);
    fifoSetAddressHandler(FIFO_USER_02, SNDLongAddressHandler, 0);
    
    REG_IME = 1;
}

static inline void main_InitVsync(void)
{
    VsyncPassed=false;
}

#define PM_NDSLITE_ADR (4)
#define PM_NDSLITE_ISLITE BIT(6)
#define PM_NDSLITE_ExternalPowerPresent BIT(3)
#define PM_NDSLITE_BRIGHTNESS(x) ((x & 0x03)<<0)
#define PM_NDSLITE_BRIGHTNESS_MASK (PM_NDSLITE_BRIGHTNESS(3))

static bool isNDSLite;

static inline void main_InitNDSL(void)
{
    isNDSLite=false;
    IPC6->DefaultBrightness=0;
    IPC6->Brightness=0xff;
  
    u32 td1=touchRead(TSC_MEASURE_TEMP1);
    u32 td2=touchRead(TSC_MEASURE_TEMP2);
    if((td1==0x0fff)&&(td2==0x0fff)){
        IPC6->DSType=DST_DSi;
        return;
    }
  
    if( (PM_GetRegister(PM_NDSLITE_ADR) & PM_NDSLITE_ISLITE) == 0){
        IPC6->DSType=DST_DS;
        return;
    }
  
    IPC6->DSType=DST_DSLite;
    isNDSLite=true;
  
    u8 data;
    data=PM_GetRegister(PM_NDSLITE_ADR);
    data&=PM_NDSLITE_BRIGHTNESS_MASK;
    IPC6->DefaultBrightness=data;
}

static inline void main_InitSoundDevice(void)
{
	powerOn(POWER_SOUND);
	REG_SOUNDCNT = SOUND_ENABLE | SOUND_VOL(0x7F);
  
    swiChangeSoundBias(1,0x400);
    a7SetSoundAmplifier(true);
}

__attribute__((noinline)) static void main_InitAll(void)
{
    IPC6->heartbeat=0;
  
    REG_IME=0;
    REG_IE=0;
  
    // Clear DMA
    for(int i=0;i<0x30/4;i++){
        *((vu32*)(0x40000B0+i))=0;
    }
  
    // Reset the clock if needed
    //rtcReset();
    
    // Start the RTC tracking IRQ
    initClockIRQ();
    
    IPC6->StartFlagARM7=0;
    IPC6->StartFlagARM9=0;
    
    IPC6->RequestUpdateIPC=true;
    IPC6->curtimeFlag=true;
  
    IPC6->RESET=0;
  
    IPC6->XYButtons=0;
    IPC6->PanelOpened=true;
        
    IPC6->DPG_ControlPowerLED=false;
    IPC6->LCDPowerControl=LCDPC_NOP;
  
    IPC6->RequestStopSound=false;
  
    IPC6->strpcmControl=ESC_NOP;
  
    IPC6->IR=IR_NULL;
  
    IPC6->ClickSE.Apply=false;
    
    IPC6->SoundChannels=0;
    
    REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
    readUserSettings();
    REG_SPICNT = 0;
  
    IPC6->UserLanguage=PersonalData->language;
    IPC6->BirthMonth=PersonalData->birthMonth;
    IPC6->BirthDay=PersonalData->birthDay;
  
    main_InitNDSL();
  
    main_InitVsync();
  
    main_InitIRQ();
  
    main_InitSoundDevice();
}

#endif /*MAIN_INIT_H_*/
