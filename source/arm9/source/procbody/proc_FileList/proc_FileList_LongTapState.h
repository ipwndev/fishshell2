
typedef struct {
  u32 Enabled;
  u32 VsyncCount;
  s32 StateCount;
  s32 basex,basey;
} TLongTapState;

DATA_IN_IWRAM_FileList static TLongTapState LongTapState;

#define LongTapState_StateMaxCount (8)

static void LongTapState_Init(void)
{
  TLongTapState *plts=&LongTapState;
  
  plts->Enabled=false;
  plts->VsyncCount=0;
  plts->StateCount=0;
  plts->basex=0;
  plts->basey=0;
}

static void LongTapState_Free(void)
{
  LongTapState_Init();
}

static void LongTapState_Start(s32 basex,s32 basey)
{
  TLongTapState *plts=&LongTapState;
  
  basex-=16;
  basey-=16;
  if(basex<0) basex=0;
  if(basey<0) basey=0;
  if((ScreenWidth-32)<basex) basex=ScreenWidth-32;
  if((ScreenHeight-32)<basey) basey=ScreenHeight-32;
  
  plts->Enabled=true;
  plts->VsyncCount=0;
  plts->StateCount=0;
  plts->basex=basex;
  plts->basey=basey;
}

static void LongTapState_ExecStop(void)
{
  TLongTapState *plts=&LongTapState;
  
  if(plts->Enabled==false) return;
  
  plts->Enabled=false;
  
  ScreenRedrawFlag=true;
}

static void LongTapState_AddVsync(u32 VsyncCount)
{
  TLongTapState *plts=&LongTapState;
  
  if(plts->Enabled==false) return;
  
  while(VsyncCount!=0){
    VsyncCount--;
    if(plts->VsyncCount<4){
      plts->VsyncCount++;
      }else{
      if(plts->StateCount<LongTapState_StateMaxCount){
        plts->VsyncCount=0;
        if(plts->StateCount!=0){
          if(plts->StateCount!=7){
            if((plts->StateCount&1)==0) Sound_Start(WAVFN_LongTap);
            }else{
            WaitKeyRelease=true;
            LongPressRequest=true;
            LongTapApplication();
            ScreenRedrawFlag=true;
          }
        }
        ScreenRedrawFlag=true;
        plts->StateCount++;
      }
    }
  }
}

static void LongTapState_DrawIcon(CglCanvas *pCanvas)
{
  TLongTapState *plts=&LongTapState;
  
  if(plts->Enabled==false) return;
  
  ELongTapSkinAlpha ELTSA;
  
  switch(plts->StateCount){
    case 0: ELTSA=ELTSA_f0; break;
    case 1: ELTSA=ELTSA_f1; break;
    case 2: ELTSA=ELTSA_f2; break;
    case 3: ELTSA=ELTSA_f3; break;
    case 4: ELTSA=ELTSA_f4; break;
    case 5: ELTSA=ELTSA_f5; break;
    case 6: ELTSA=ELTSA_f6; break;
    case 7: ELTSA=ELTSA_f7; break;
    default: ELTSA=ELTSA_f7; break;
  }
  
  CglTGF *ptgf=LongTapAlpha_GetSkin(ELTSA);
  ptgf->BitBlt(pCanvas,plts->basex,plts->basey);
}

static bool LongTapState_GetProceed(void)
{
  TLongTapState *plts=&LongTapState;
  
  if(plts->Enabled==false) return(false);
  if(plts->StateCount<LongTapState_StateMaxCount) return(false);
  
  return(true);
}

