#ifndef MAIN_IRQ_TIMER_H_
#define MAIN_IRQ_TIMER_H_

#define MAX( x, y ) ( ( x > y ) ? x : y )
#define MIN( x, y ) ( ( x < y ) ? x : y )

__attribute__((noinline)) static void InterruptHandler_Timer1_SetSwapChannel(void)
{
    s16 *lbuf,*rbuf;
  
    if(strpcmCursorFlag==0){
        lbuf=strpcmL0;
        rbuf=strpcmR0;
    }else{
        lbuf=strpcmL1;
        rbuf=strpcmR1;
    }
  
    u32 channel=strpcmCursorFlag;
  
    u32 chcnt=IPC6->SoundChannels;
  
    if((chcnt==0)||(chcnt==1)){
        // Left channel
        SCHANNEL_CR(channel) = 0;
        SCHANNEL_SOURCE(channel) = (uint32)lbuf;
        SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0) | SOUND_FORMAT_16BIT;
    }
  
    channel+=2;
  
    if((chcnt==0)||(chcnt==2)){
        // Right channel
        SCHANNEL_CR(channel) = 0;
        SCHANNEL_SOURCE(channel) = (uint32)rbuf;
        SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0x7F) | SOUND_FORMAT_16BIT;
    }
  
    strpcmCursorFlag=1-strpcmCursorFlag;
}

// --------------------------------------------------------

void VolNoAGC(s16 *lbuf,s16 *rbuf,u32 count,s32 vol)
{
	asm volatile(
		"VolNoAGC_lbuf .req r0 \n\t"
		"VolNoAGC_rbuf .req r1 \n\t"
		"VolNoAGC_count .req r2 \n\t"
		"VolNoAGC_vol .req r3 \n\t"
		"VolNoAGC_tmpl .req r4 \n\t"
		"VolNoAGC_tmpr .req lr \n\t"
		"VolNoAGC_limmin .req r5 \n\t"
		"VolNoAGC_limmax .req r6 \n\t"
		" \n\t"
		"stmfd sp!, {r4, r5, r6, lr} \n\t"
		"ldr VolNoAGC_limmin, =-32768 \n\t"
		"ldr VolNoAGC_limmax, =32767 \n\t"
		"VolNoAGC_loop: \n\t"
		"ldrsh VolNoAGC_tmpl,[VolNoAGC_lbuf] \n\t"
		"ldrsh VolNoAGC_tmpr,[VolNoAGC_rbuf] \n\t"
		"mul VolNoAGC_tmpl,VolNoAGC_vol,VolNoAGC_tmpl \n\t"
		"mul VolNoAGC_tmpr,VolNoAGC_vol,VolNoAGC_tmpr \n\t"
		"asr VolNoAGC_tmpl,VolNoAGC_tmpl,#11 @ /2048 \n\t"
		"asr VolNoAGC_tmpr,VolNoAGC_tmpr,#11 @ /2048 \n\t"
		"cmp VolNoAGC_tmpl,VolNoAGC_limmin \n\t"
		"movlt VolNoAGC_tmpl,VolNoAGC_limmin \n\t"
		"cmp VolNoAGC_tmpl,VolNoAGC_limmax \n\t"
		"movgt VolNoAGC_tmpl,VolNoAGC_limmax \n\t"
		"cmp VolNoAGC_tmpr,VolNoAGC_limmin \n\t"
		"movlt VolNoAGC_tmpr,VolNoAGC_limmin \n\t"
		"cmp VolNoAGC_tmpr,VolNoAGC_limmax \n\t"
		"movgt VolNoAGC_tmpr,VolNoAGC_limmax \n\t"
		"strh VolNoAGC_tmpl,[VolNoAGC_lbuf],#2 \n\t"
		"subs VolNoAGC_count,#1 \n\t"
		"strh VolNoAGC_tmpr,[VolNoAGC_rbuf],#2 \n\t"
		"bne VolNoAGC_loop \n\t"
		"ldmfd sp!, {r4, r5, r6, pc} \n\t"
		:::"memory"
	);
}

u32 VolUseAGC(s16 *lbuf,s16 *rbuf,u32 count,s32 vol)
{
	asm volatile(
		"VolUseAGC_lbuf .req r0 \n\t"
		"VolUseAGC_rbuf .req r1 \n\t"
		"VolUseAGC_count .req r2 \n\t"
		"VolUseAGC_vol .req r3 \n\t"
		"VolUseAGC_tmpl .req r4 \n\t"
		"VolUseAGC_tmpr .req lr \n\t"
		"VolUseAGC_limmin .req r5 \n\t"
		"VolUseAGC_limmax .req r6 \n\t"
		"VolUseAGC_overflag .req r7 \n\t"
		" \n\t"
		"stmfd sp!, {r4,r5,r6,r7,lr} \n\t"
		" \n\t"
		"ldr VolUseAGC_limmin,=-32768 \n\t"
		"ldr VolUseAGC_limmax,=32767 \n\t"
		" \n\t"
		"mov VolUseAGC_overflag,#0 \n\t"
		" \n\t"
		"VolUseAGC_loop: \n\t"
		"ldrsh VolUseAGC_tmpl,[VolUseAGC_lbuf] \n\t"
		"ldrsh VolUseAGC_tmpr,[VolUseAGC_rbuf] \n\t"
		"mul VolUseAGC_tmpl,VolUseAGC_vol,VolUseAGC_tmpl \n\t"
		"mul VolUseAGC_tmpr,VolUseAGC_vol,VolUseAGC_tmpr \n\t"
		"asr VolUseAGC_tmpl,VolUseAGC_tmpl,#11 @ /2048 \n\t"
		"asr VolUseAGC_tmpr,VolUseAGC_tmpr,#11 @ /2048 \n\t"
		"cmp VolUseAGC_tmpl,VolUseAGC_limmin \n\t"
		"movlt VolUseAGC_tmpl,VolUseAGC_limmin \n\t"
		"movlt VolUseAGC_overflag,#1 \n\t"
		"cmp VolUseAGC_tmpl,VolUseAGC_limmax \n\t"
		"movgt VolUseAGC_tmpl,VolUseAGC_limmax \n\t"
		"movgt VolUseAGC_overflag,#1 \n\t"
		"cmp VolUseAGC_tmpr,VolUseAGC_limmin \n\t"
		"movlt VolUseAGC_tmpr,VolUseAGC_limmin \n\t"
		"movlt VolUseAGC_overflag,#1 \n\t"
		"cmp VolUseAGC_tmpr,VolUseAGC_limmax \n\t"
		"movgt VolUseAGC_tmpr,VolUseAGC_limmax \n\t"
		"movgt VolUseAGC_overflag,#1 \n\t"
		"strh VolUseAGC_tmpl,[VolUseAGC_lbuf],#2 \n\t"
		"subs VolUseAGC_count,#1 \n\t"
		"strh VolUseAGC_tmpr,[VolUseAGC_rbuf],#2 \n\t"
		"bne VolUseAGC_loop \n\t"
		" \n\t"
		"mov r0,VolUseAGC_overflag \n\t"
		"ldmfd sp!, {r4,r5,r6,r7,pc} \n\t"
		:::"memory"
	);
	return 0; //nop
}

static const s32 LogVolumeCount=128;
static const s32 LogVolume[LogVolumeCount]={
         0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,480,
         512,544,576,608,640,672,704,736,768,800,832,864,896,928,960,992,
         1024,1056,1088,1120,1152,1184,1216,1248,1280,1312,1344,1376,1408,1440,1472,1504,
         1536,1568,1600,1632,1664,1696,1728,1760,1792,1824,1856,1888,1920,1952,1984,2016,
         2048,2123,2202,2283,2368,2455,2546,2641,2738,2840,2945,3054,3167,3284,3405,3531,
         3662,3798,3938,4084,4235,4392,4554,4723,4898,5079,5267,5462,5664,5873,6091,6316,
         6550,6792,7043,7304,7574,7854,8145,8446,8759,9083,9419,9768,10129,10504,10893,11296,
         11714,12147,12597,13063,13546,14047,14567,15106,15665,16244,16846,17469,18115,18785,19481,20201,};

__attribute__((noinline)) static void InterruptHandler_Timer1_ApplyVolume(s16 *lbuf,s16 *rbuf,u32 count)
{
    if((lbuf==NULL)||(rbuf==NULL)) return;
  
    s32 vol=(s32)IPC6->strpcmAudioVolume64;
    if((LogVolumeCount-1)<vol) vol=LogVolumeCount-1;
    
    if(vol==0){
        for(u32 idx=0;idx<count;idx++){
            lbuf[idx]=0;
            rbuf[idx]=0;
        }
        return;
    }
  
    u32 MasterVolume=LogVolume[vol];
  
    VolNoAGC(lbuf,rbuf,count,MasterVolume);
    ARM7_SelfCheck_Check();
}

__attribute__((noinline)) static void InterruptHandler_Timer1_ApplyVolumeWithAGC(s16 *lbuf,s16 *rbuf,u32 count)
{
    if((lbuf==NULL)||(rbuf==NULL)) return;
  
    s32 vol=(s32)IPC6->strpcmVideoVolume64;
    if((LogVolumeCount-1)<vol) vol=LogVolumeCount-1;
  
    if(vol==0){
        for(u32 idx=0;idx<count;idx++){
            lbuf[idx]=0;
            rbuf[idx]=0;
        }
        return;
    }
  
    u32 MasterVolume=LogVolume[vol];
  
    u32 divcnt=8;
    u32 divsize=count/divcnt;
  
    for(u32 idx=0;idx<divcnt;idx++){
        u32 vol=MasterVolume+agc;
        if(VolUseAGC(lbuf,rbuf,divsize,vol)==0){
            agc+=8;
            if(1024<agc) agc=1024;
        }else{
            agc-=16;
            if(agc<-1024) agc=-1024;
        }
        lbuf+=divsize;
        rbuf+=divsize;
    }
  
    ARM7_SelfCheck_Check();
  
}

// --------------------------------------------------------------------

static void InterruptHandler_Timer1_Null(void)
{
}

static void InterruptHandler_Timer1_PCMx1(void)
{
    InterruptHandler_Timer1_SetSwapChannel();
  
    s16 *lbuf,*rbuf;
  
    if(strpcmCursorFlag==0){
        lbuf=strpcmL0;
        rbuf=strpcmR0;
    }else{
        lbuf=strpcmL1;
        rbuf=strpcmR1;
    }
  
    u32 Samples=strpcmSamples;
  
    if(IPC6->strpcmWriteRequest!=0){
        MemSet16DMA3(0,lbuf,Samples*2);
        MemSet16DMA3(0,rbuf,Samples*2);
    }else{
        if(IPC6->strpcmLRBuf==NULL){
            MemSet16DMA3(0,lbuf,Samples*2);
            MemSet16DMA3(0,rbuf,Samples*2);
        }else{
            u32 *plrsrc=IPC6->strpcmLRBuf;
            s16 *plbuf=lbuf,*prbuf=rbuf;
            u32 CurSamples=0;
            s32 slr=0;
            asm volatile (
				" \n\t"
				"InterruptHandler_Timer1_PCMx1_OverSampling_Loop: \n\t"
				"ldr     %[slr], [%[plrsrc]], #4 \n\t"
				"add     %[CurSamples], %[CurSamples], #1 \n\t"
				"strh    %[slr], [%[plbuf]], #2 \n\t"
				"mov     %[slr], %[slr],asr #16 \n\t"
				"cmp     %[CurSamples], %[Samples] \n\t"
				"strh    %[slr], [%[prbuf]],#2 \n\t"
				" \n\t"
				"bcc InterruptHandler_Timer1_PCMx1_OverSampling_Loop \n\t"
				::[plrsrc]"r"(plrsrc), [CurSamples]"r"(CurSamples), [slr]"r"(slr), [plbuf]"r"(plbuf), [Samples]"r"(Samples), [prbuf]"r"(prbuf)
				:"cc", "memory"
			);
        }
    
        IPC6->IR=IR_NextSoundData;
        REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
        IPC6->strpcmWriteRequest=1;
    
        InterruptHandler_Timer1_ApplyVolume(lbuf,rbuf,Samples);
    }
  
    CallBackIRQ_strpcmUpdate();
}

static void InterruptHandler_Timer1_PCMx2(void)
{
    InterruptHandler_Timer1_SetSwapChannel();
  
    s16 *lbuf,*rbuf;
  
    if(strpcmCursorFlag==0){
        lbuf=strpcmL0;
        rbuf=strpcmR0;
    }else{
        lbuf=strpcmL1;
        rbuf=strpcmR1;
    }
  
    u32 Samples=strpcmSamples;
  
    if(IPC6->strpcmWriteRequest!=0){
        MemSet16DMA3(0,lbuf,Samples*2);
        MemSet16DMA3(0,rbuf,Samples*2);
        strpcmLastL=0;
        strpcmLastR=0;
    }else{
        if(IPC6->strpcmLRBuf==NULL){
            MemSet16DMA3(0,lbuf,Samples*2);
            MemSet16DMA3(0,rbuf,Samples*2);
            strpcmLastL=0;
            strpcmLastR=0;
        }else{
            s32 l=strpcmLastL,r=strpcmLastR;
            u32 *plrsrc=IPC6->strpcmLRBuf;
            s16 *plbuf=lbuf,*prbuf=rbuf;
            u32 LastSamples=Samples;
            s32 slr=0,sl=0,sr=0;
            const u32 mask=0xffff;
            asm volatile (
				"InterruptHandler_Timer1_PCMx2_OverSampling_Loop: \n\t"
				"ldr %[slr],[%[plrsrc]],#4 \n\t"
				"mov %[sl],%[slr],lsl #16 \n\t"
				"mov %[sl],%[sl],asr #16 \n\t"
				"mov %[sr],%[slr],asr #16 \n\t"
				"add %[l],%[l],%[sl],asr #1 \n\t"
				" \n\t"
				"and %[l],%[l],%[mask] \n\t"
				"orr %[l],%[l],%[sl],lsl #16 \n\t"
				"str %[l],[%[plbuf]],#4 \n\t"
				"mov %[l],%[sl],asr #1 \n\t"
				" \n\t"
				"add %[r],%[r],%[sr],asr #1 \n\t"
				"and %[r],%[r],%[mask] \n\t"
				"orr %[r],%[r],%[sr],lsl #16 \n\t"
				"str %[r],[%[prbuf]],#4 \n\t"
				"mov %[r],%[sr],asr #1 \n\t"
				" \n\t"
				"subs %[LastSamples],%[LastSamples],#2 \n\t"
				"bne InterruptHandler_Timer1_PCMx2_OverSampling_Loop \n\t"
				::[plrsrc]"r"(plrsrc), [sl]"r"(sl), [slr]"r"(slr), [mask]"r"(mask), [l]"r"(l), [r]"r"(r),
					[LastSamples]"r"(LastSamples), [plbuf]"r"(plbuf), [sr]"r"(sr), [prbuf]"r"(prbuf)
				:"cc", "memory"
			);
            strpcmLastL=l; strpcmLastR=r;
        }
    
        IPC6->IR=IR_NextSoundData;
        REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
        IPC6->strpcmWriteRequest=1;
    
        InterruptHandler_Timer1_ApplyVolume(lbuf,rbuf,Samples);
    }
  
    CallBackIRQ_strpcmUpdate();
}

static void InterruptHandler_Timer1_PCMx4(void)
{
    InterruptHandler_Timer1_SetSwapChannel();
  
    s16 *lbuf,*rbuf;
  
    if(strpcmCursorFlag==0){
        lbuf=strpcmL0;
        rbuf=strpcmR0;
    }else{
        lbuf=strpcmL1;
        rbuf=strpcmR1;
    }
  
    u32 Samples=strpcmSamples;
  
    if(IPC6->strpcmWriteRequest!=0){
        MemSet16DMA3(0,lbuf,Samples*2);
        MemSet16DMA3(0,rbuf,Samples*2);
        strpcmLastL=0;
        strpcmLastR=0;
    }else{
        if(IPC6->strpcmLRBuf==NULL){
            MemSet16DMA3(0,lbuf,Samples*2);
            MemSet16DMA3(0,rbuf,Samples*2);
            strpcmLastL=0;
            strpcmLastR=0;    
        }else{
            s32 l=strpcmLastL,r=strpcmLastR;
            u32 *plrsrc=IPC6->strpcmLRBuf;
            s16 *plbuf=lbuf,*prbuf=rbuf;
            u32 LastSamples=Samples;
            s32 slr=0,sl=0,sr=0,tmp=0;
            asm volatile (
				"InterruptHandler_Timer1_PCMx4_OverSampling_Loop: \n\t"
				"ldr %[slr],[%[plrsrc]],#4 \n\t"
				"mov %[sl],%[slr],lsl #16 \n\t"
				"mov %[sl],%[sl],asr #16 \n\t"
				"mov %[sr],%[slr],asr #16 \n\t"
				" \n\t"
				"strh %[l],[%[plbuf]],#2 \n\t"
				"add %[tmp],%[l],%[l] \n\t"
				"add %[tmp],%[tmp],%[l] \n\t"
				"add %[tmp],%[tmp],%[sl] \n\t"
				"mov %[tmp],%[tmp],asr #2 \n\t"
				"strh %[tmp],[%[plbuf]],#2 \n\t"
				"add %[tmp],%[l],%[sl] \n\t"
				"mov %[tmp],%[tmp],asr #1 \n\t"
				"strh %[tmp],[%[plbuf]],#2 \n\t"
				"add %[tmp],%[sl],%[sl] \n\t"
				"add %[tmp],%[tmp],%[sl] \n\t"
				"add %[tmp],%[tmp],%[l] \n\t"
				"mov %[tmp],%[tmp],asr #2 \n\t"
				"strh %[tmp],[%[plbuf]],#2 \n\t"
				"mov %[l],%[sl] \n\t"
				" \n\t"
				"strh %[r],[%[prbuf]],#2 \n\t"
				"add %[tmp],%[r],%[r] \n\t"
				"add %[tmp],%[tmp],%[r] \n\t"
				"add %[tmp],%[tmp],%[sr] \n\t"
				"mov %[tmp],%[tmp],asr #2 \n\t"
				"strh %[tmp],[%[prbuf]],#2 \n\t"
				"add %[tmp],%[r],%[sr] \n\t"
				"mov %[tmp],%[tmp],asr #1 \n\t"
				"strh %[tmp],[%[prbuf]],#2 \n\t"
				"add %[tmp],%[sr],%[sr] \n\t"
				"add %[tmp],%[tmp],%[sr] \n\t"
				"add %[tmp],%[tmp],%[r] \n\t"
				"mov %[tmp],%[tmp],asr #2 \n\t"
				"strh %[tmp],[%[prbuf]],#2 \n\t"
				"mov %[r],%[sr] \n\t"
				" \n\t"
				"subs %[LastSamples],%[LastSamples],#4 \n\t"
				"bne InterruptHandler_Timer1_PCMx4_OverSampling_Loop \n\t"
				::[plrsrc]"r"(plrsrc), [sl]"r"(sl), [slr]"r"(slr), [l]"r"(l), [r]"r"(r),
					[LastSamples]"r"(LastSamples), [plbuf]"r"(plbuf), [sr]"r"(sr), [prbuf]"r"(prbuf), [tmp]"r"(tmp)
				:"cc", "memory"
			);
            strpcmLastL=l; strpcmLastR=r;
        }
    
        IPC6->IR=IR_NextSoundData;
        REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
        IPC6->strpcmWriteRequest=1;
    
        InterruptHandler_Timer1_ApplyVolume(lbuf,rbuf,Samples);
    }
  
    CallBackIRQ_strpcmUpdate();
}

__attribute__ ((__section__ ("IWRAM_libglobal_mp2"))) static void InterruptHandler_Timer1_MP2(void)
{
    InterruptHandler_Timer1_SetSwapChannel();
      
    s16 *lbuf,*rbuf;
      
    if(strpcmCursorFlag==0){
        lbuf=strpcmL0;
        rbuf=strpcmR0;
    }else{
        lbuf=strpcmL1;
        rbuf=strpcmR1;
    }
  
    u32 Samples=strpcmSamples;
      
    if((IPC6->strpcmWriteRequest!=0)||(IPC6->MP2PauseFlag==true)){
        MemSet16DMA3(0,lbuf,Samples*2);
        MemSet16DMA3(0,rbuf,Samples*2);
    }else{
        if(IPC6->DPG_ControlPowerLED){
            if((REG_KEYXY & IPC_LID_CLOSED)==0){
                a7led(0);a7led(3);
            }
        }
        IPC6->strpcmWriteRequest=1;
        
        if(IPC6->IR_flash==true){
            IPC6->IR=IR_Flash;
            REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
//          FlashFileBufferMP2();
            while(IPC6->IR!=IR_NULL) swiDelay(1);
            IPC6->IR_flash=false;
        }else{
            IPC6->IR_SyncSamples_SendToARM9+=(u64)Samples;
        }
    
        while(IPC6->IR!=IR_NULL) swiDelay(1);
        IPC6->IR=IR_SyncSamples;
        REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
        
        s16 *ldst=lbuf,*rdst=rbuf;
    
        int OutputSamples=UpdateMP2(ldst,rdst);
        ARM7_SelfCheck_Check();
    
        if(OutputSamples==0){
            for(u32 cnt=0;cnt<Samples;cnt++){
                *lbuf++=0;
                *rbuf++=0;
            }
        }else{
            InterruptHandler_Timer1_ApplyVolumeWithAGC(lbuf,rbuf,Samples);
        }
        
        IPC6->strpcmWriteRequest=0;
    }
  
    CallBackIRQ_strpcmUpdate();
}

#endif /*MAIN_IRQ_TIMER_H_*/
