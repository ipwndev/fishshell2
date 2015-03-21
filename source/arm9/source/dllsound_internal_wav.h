
#ifdef ExceptWAVE

#include "plug_wav.h"
DATA_IN_AfterSystem static bool isWAVE=false;

static bool DLLSound_ChkExt_internal_wave(const char *pext)
{
  bool res=false;
  if(isStrEqual_NoCaseSensitive(pext,".wav")==true) res=true;
  return(res);
}

static bool DLLSound_Open_internal_wave(const char *pext)
{
    OVM_libsnd_wave();
  if(PlugWAVE_Start(PluginBody_FileHandle)==false){
    PlugWAVE_Free();
    FAT2_fclose(PluginBody_FileHandle); PluginBody_FileHandle=NULL;
    _consolePrint("Can not start internal WAVE plugin.\n");
    return(false);
  }
  DTCM_StackCheck(-1);
  
  u32 rate=PlugWAVE_GetSampleRate();
  u32 spf=PlugWAVE_GetSamplePerFrame();
  u32 chs=PlugWAVE_GetChannelCount();
  
  MACRO_LoadGeneralID3Tag(&MM_DLLSound,PlugWAVE_GetInfoIndexCount,PlugWAVE_GetInfoStrL,PlugWAVE_GetInfoStrUTF8,PlugWAVE_GetInfoStrW);
  
  RateConv.SourceRate=0;
  
  bool f=false;
  
  if((f==false)&&(rate==32768/4)){
      f=true;
      DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx4);
    }
    if((f==false)&&(rate==32768/2)){
      f=true;
      DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx2);
    }
    if((f==false)&&(rate==32768/1)){
      f=true;
      DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx1);
    }
    
    if((f==false)&&(rate<=32768)){
      f=true;
      DLLSound_Open_ins_strpcmStart(rate,spf,chs,SPF_PCMx4);
    }
    
    if(f==false){
        f=true;
        
        TRateConv *prc=&RateConv;
        prc->SourceRate=rate;
        prc->Pos=0;
        prc->Size=0;
        prc->Mod16=0;
        prc->Add16=(rate*0x10000)/32768;
        prc->CountSource=spf;
        prc->Count32768=(spf*32768)/prc->SourceRate;
        if((prc->Count32768&1)!=0) prc->Count32768--;
        prc->pReadBufLR=(u32*)safemalloc_chkmem(&MM_DLLSound,prc->CountSource*3*2*2);
        prc->LastSampleLR=0;
        
        _consolePrintf("Use sampling rate converter.\nsrcrate=%dHz, Add16=0x%04x, Count32768=%dsmps.\n",prc->SourceRate,prc->Add16,prc->Count32768); 
        
        DLLSound_Open_ins_strpcmStart(32768,prc->Count32768,chs,SPF_PCMx1);
      }
  
  MM_CheckOverRange();
  
  PrintFreeMem();
  
  isWAVE=true;
  
  return(true);
}

#endif

static void DLLSound_Update_internal_wave(u32 BaseSamples,u32 *plrdst)
{
    u32 Samples=0;
    if(RateConv.SourceRate==0){
        u32 Samples=0;
        {
          // PrfStart();
          u32 lastpos=PlugWAVE_GetPosOffset();
          Samples=PlugWAVE_Update(plrdst);
          DTCM_StackCheck(-1);
          u32 curpos=PlugWAVE_GetPosOffset();
          if((PlugTime_SkipCheck==false)&&(lastpos<=curpos)){
            PlugTime_TotalSamples+=Samples;
            PlugTime_SamplesDivCount+=curpos-lastpos;
          }
          PlugTime_SkipCheck=false;
          // PrfEnd(curpos);
          if(Samples!=BaseSamples) strpcmRequestStop=true;
        }
        if(Samples<BaseSamples){ cwl();
          for(u32 idx=Samples;idx<BaseSamples;idx++){
            plrdst[idx]=0;
          }
        }
        
        return;
      }
      
      {
        TRateConv *prc=&RateConv;
        if(prc->Pos!=0){
          if(prc->Pos<prc->Size){
            MemCopy32CPU(&prc->pReadBufLR[prc->Pos],&prc->pReadBufLR[0],(prc->Size-prc->Pos)*4);
            prc->Size-=prc->Pos;
            prc->Pos=0;
          }
        }
        
        while((((s32)prc->Size-1)-(s32)prc->Pos)<(s32)prc->CountSource){
          { cwl();
            // PrfStart();
            u32 lastpos=PlugWAVE_GetPosOffset();
            u32 Samples=PlugWAVE_Update(&prc->pReadBufLR[prc->Size]);
            DTCM_StackCheck(-1);
            u32 curpos=PlugWAVE_GetPosOffset();
            if((PlugTime_SkipCheck==false)&&(lastpos<=curpos)){
              PlugTime_TotalSamples+=Samples;
              PlugTime_SamplesDivCount+=curpos-lastpos;
            }
            PlugTime_SkipCheck=false;
            prc->Size+=Samples;
            // PrfEnd(curpos);
            if(Samples!=prc->CountSource){
              strpcmRequestStop=true;
              break;
            }
          }
        }
        
        s32 LastSrcSamples=((s32)prc->Size-1)-(s32)prc->Pos;
        
        if(LastSrcSamples<=0){
          Samples=0;
          }else{
          Samples=prc->Count32768;
          
          u32 ConvSamples=(LastSrcSamples*32768)/prc->SourceRate;
          if(ConvSamples<Samples) Samples=ConvSamples;
          
          // PrfStart(); // 270 268 271 266us
          
          u32 *psrcbuflr=&prc->pReadBufLR[prc->Pos];
          u32 *pdstbuflr=plrdst;
          u32 cnt=Samples+1;
          u32 Pos=prc->Pos*4;
          u32 Mod16=prc->Mod16;
          u32 Add16=prc->Add16;
          s32 LastSample=prc->LastSampleLR;
          
          u32 REG_Mask0xffff=0xffff;
          u32 REG_iMod16,REG_tmpsmp,REG_tmpl,REG_tmpr;
         
          asm volatile (
				"Linner_Loop1: \n\t"
				"subs %[cnt],%[cnt],#1 \n\t"
				"beq Linner_End1 \n\t"
				" \n\t"
				"mov %[REG_iMod16],#0x10000 \n\t"
				"sub %[REG_iMod16],%[REG_iMod16],%[Mod16] \n\t"
				" \n\t"
				"ldr %[REG_tmpsmp],[%[psrcbuflr],%[Pos]] \n\t"
				"smulwb %[REG_tmpl],%[REG_iMod16],%[LastSample] \n\t"
				"smlawb %[REG_tmpl],%[Mod16],%[REG_tmpsmp],%[REG_tmpl] @ REG_tmpsmp‚Ì‰ºˆÊ16bit‚¾‚¯‰‰ŽZ‚³‚ê‚é‚Í‚¸ \n\t"
				"and %[REG_tmpl],%[REG_tmpl],%[REG_Mask0xffff] \n\t"
				" \n\t"
				"smulwt %[REG_tmpr],%[REG_iMod16],%[LastSample] \n\t"
				"smlawt %[REG_tmpr],%[Mod16],%[REG_tmpsmp],%[REG_tmpr] @ REG_tmpsmp‚ÌãˆÊ16bit‚¾‚¯‰‰ŽZ‚³‚ê‚é‚Í‚¸ \n\t"
				" \n\t"
				"orr %[REG_tmpsmp],%[REG_tmpl],%[REG_tmpr],lsl #16 \n\t"
				"str %[REG_tmpsmp],[%[pdstbuflr]],#4 \n\t"
				" \n\t"
				"add %[Mod16],%[Mod16],%[Add16] \n\t"
				"cmp %[Mod16],#0x10000 \n\t"
				"blo Linner_Loop1 \n\t"
				" \n\t"
				"Linner_Modulation1: \n\t"
				"sub %[Mod16],%[Mod16],#0x10000 \n\t"
				"ldr %[LastSample],[%[psrcbuflr],%[Pos]] \n\t"
				"add %[Pos],%[Pos],#4 \n\t"
				"cmp %[Mod16],#0x10000 \n\t"
				"blo Linner_Loop1 \n\t"
				"b Linner_Modulation1 \n\t"
				" \n\t"
				"Linner_End1: \n\t"
			:[cnt]"+r"(cnt), [REG_iMod16]"+r"(REG_iMod16), [Mod16]"+r"(Mod16), [REG_tmpsmp]"+r"(REG_tmpsmp), [psrcbuflr]"+r"(psrcbuflr), [Pos]"+r"(Pos),
				[REG_tmpl]"+r"(REG_tmpl), [LastSample]"+r"(LastSample), [REG_Mask0xffff]"+r"(REG_Mask0xffff), [REG_tmpr]"+r"(REG_tmpr), [Add16]"+r"(Add16),
				[pdstbuflr]"+r"(pdstbuflr)
			::"memory"
			);

          prc->Pos=Pos/4;
          prc->Mod16=Mod16;
          prc->Add16=Add16;
          prc->LastSampleLR=LastSample;
          
          // PrfEnd(0);
        }
      }
      
      if(Samples<BaseSamples){ cwl();
        for(u32 idx=Samples;idx<BaseSamples;idx++){ cwl();
          plrdst[idx]=0;
        }
      }
}

static u32 DLLSound_GetPlayTimeSec_internal_wave(void)
{
  return(PlugWAVE_GetPlayTimeSec());
}

