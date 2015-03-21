#ifndef MAIN_PROC_H_
#define MAIN_PROC_H_

static bool strpcmPlayFlag;

__attribute__((noinline)) static void main_Proc_strpcmControl(void)
{
    _consolePrintf("main_Proc_strpcmControl();\nIPC6->strpcmControl=%d.\n",IPC6->strpcmControl);
    switch(IPC6->strpcmControl){
        case ESC_NOP: {
        } break;
        case ESC_Play: {
            if((EnableFPGA==true)&&(IPC6->strpcmFormat!=SPF_MP2)){
                FPGA_strpcmPlay();
                irqSet(IRQ_TIMER1,FPGA_InterruptHandler_Timer1);
            }else{
                strpcmPlay();
                switch(strpcmFormat){
                    case SPF_PCMx1: irqSet(IRQ_TIMER1,InterruptHandler_Timer1_PCMx1); break;
                    case SPF_PCMx2: irqSet(IRQ_TIMER1,InterruptHandler_Timer1_PCMx2); break;
                    case SPF_PCMx4: irqSet(IRQ_TIMER1,InterruptHandler_Timer1_PCMx4); break;
                    case SPF_MP2: irqSet(IRQ_TIMER1,InterruptHandler_Timer1_MP2); break;
                    default: irqSet(IRQ_TIMER1,InterruptHandler_Timer1_Null); break;
                }
            }
            strpcmPlayFlag=true;
        } break;
        case ESC_Stop: {
            strpcmPlayFlag=false;
            if((EnableFPGA==true)&&(IPC6->strpcmFormat!=SPF_MP2)){
                FPGA_strpcmStop();
            }else{
                strpcmStop();
            }
            irqSet(IRQ_TIMER1,InterruptHandler_Timer1_Null);
        } break;
        default: {
            REG_IME=0;
            _consolePrintf("Unknown strpcmControl(%d).\n",IPC6->strpcmControl);
            ShowLogHalt();
        } break;
    }
    IPC6->strpcmControl=ESC_NOP;
    ARM7_SelfCheck_Check();
}

__attribute__((noinline)) static void main_Proc_Brightness(u32 Brightness)
{
    u8 data;
  
    data=PM_GetRegister(PM_NDSLITE_ADR);
    data&=~PM_NDSLITE_BRIGHTNESS_MASK;
    data|=PM_NDSLITE_BRIGHTNESS(Brightness);
  
    PM_SetRegister(PM_NDSLITE_ADR,data);
  
    _consolePrintf("Set brightness=%d\n",Brightness);
}

__attribute__((noinline)) static void main_Proc_LCDPowerControl(ELCDPC LCDPC)
{
    switch(LCDPC){
        case LCDPC_OFF_BOTH: {
            a7led(3);
            a7lcd_select(0);
        } break;
        case LCDPC_ON_BOTTOM: {
            a7led(0);
            a7lcd_select(PM_BACKLIGHT_BOTTOM);
        } break;
        case LCDPC_ON_TOP: {
            a7led(0);
            a7lcd_select(PM_BACKLIGHT_TOP);
        } break;
        case LCDPC_ON_BOTH: {
            a7led(0);
            a7lcd_select(PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);
        } break;
        case LCDPC_SOFT_POWEROFF: {
            a7poff();
            while(1);
        }
        default: ShowLogHalt(); break; // this execute is bug.
    }
    ARM7_SelfCheck_Check();
}

static void UpdateIPC6(void)
{
    u32 buttons = (~REG_KEYXY) & 0x7f;
  
    {
        u32 xybtns=0;
#define KEY_TOUCH BIT(12) //!< Touchscreen pendown.
#define KEY_X BIT(10) //!< Keypad X button.
#define KEY_Y BIT(11) //!< Keypad Y button.
        //if((buttons&KEY_TOUCH)!=0) xybtns|=KEY_TOUCH;
        if(touchPenDown()) xybtns|=KEY_TOUCH;
        if((buttons&IPC_X)!=0) xybtns|=KEY_X;
        if((buttons&IPC_Y)!=0) xybtns|=KEY_Y;
        IPC6->XYButtons=xybtns;
    }
  
    if((REG_KEYXY & IPC_LID_CLOSED)!=0){
        IPC6->PanelOpened=false;
    }else{
        IPC6->PanelOpened=true;
    }
  
    IPC6->touchXpx = (u32)-1;
    IPC6->touchYpx = (u32)-1;
        
    if((IPC6->XYButtons & KEY_TOUCH)!=0){
        touchPosition tempPos;
        tempPos.rawx=0;
        tempPos.rawy=0;
        touchReadXY(&tempPos);
        
        if((tempPos.rawx!=0)&&(tempPos.rawy!=0)){
            if((tempPos.px<256)&&(tempPos.py<192)){
                IPC6->touchXpx = tempPos.px;
                IPC6->touchYpx = tempPos.py;
            }else{
                IPC6->XYButtons&=~KEY_TOUCH;
            }
        }else{
            IPC6->XYButtons&=~KEY_TOUCH;
        }
    }
    
    IPC6->Romeo2_HPSwitch_Pressing=FPGA_GetHPSwitch();
  
    if(isNDSLite==false){
        IPC6->ExternalPowerPresent=true; // 旧DSは不明な値を返すので常に接続されていることにする。
    }else{
        IPC6->ExternalPowerPresent=( (PM_GetRegister(PM_NDSLITE_ADR) & PM_NDSLITE_ExternalPowerPresent) != 0) ? true : false;
    }
    
    ARM7_SelfCheck_Check();
}

void CallBackIRQ_strpcmUpdate(void)
{
    if(IPC6->RequestUpdateIPC==true){
        UpdateIPC6();
        IPC6->RequestUpdateIPC=false;
        ARM7_SelfCheck_Check();
    }
    if(IPC6->curtimeFlag==true){
        rtcGetTimeAndDate((uint8 *)&(IPC6->time.rtc.year));
        IPC6->curtimeFlag=false;
        ARM7_SelfCheck_Check();
    }
}

static __attribute__((noinline)) void main_Proc_CheckSoundCode(void)
{
    TransferSound *snd = IPC6->soundData;
    if ((0 != snd)&&(snd->count!=0)) {
        REG_IME=0;
        IPC6->soundData = 0;
        u32 i;
        for (i=0; i<snd->count; i++) {
            s32 chan = getFreeSoundChannel();
            if(chan>=0) startSound(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, snd->data[i].pan, snd->data[i].format);
        }
        REG_IME=1;
    }
 
    if(IPC6->RequestStopSound==true){
        REG_IME=0;
        stopSound();
        IPC6->RequestStopSound=false;
        REG_IME=1;
    }
    
    if(IPC6->ClickSE.Apply==true){
        REG_IME=0;
        StartClickSE(&IPC6->ClickSE);
        REG_IME=1;
    }
}

#endif /*MAIN_PROC_H_*/
