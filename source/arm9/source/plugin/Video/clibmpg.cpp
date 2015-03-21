
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "../../ipc6.h"

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "glib.h"
#include "memtool.h"
#include "arm9tcm.h"
#include "inifile.h"

#include "libmpeg2/config.h"

#include "clibdpg.h"
#include "_dpgfs.h"

extern u32 reqflip;

DATA_IN_MTCM_VAR static u32 msGOPSkip;

#define ReadBufSize (32*1024)

#include "clibmpg_diskcache.h"
#include "clibmpg_framecache.h"

Clibmpg::Clibmpg(CStream *_pCStream,const u32 _TotalFrameCount,const u32 _FPS,const u32 _SndFreq,const EMPGPixelFormat _PixelFormat):TotalFrameCount(_TotalFrameCount),FPS(_FPS),SndFreq(_SndFreq),PixelFormat(_PixelFormat)
{
  Initialized=false;
  
  msGOPSkip=400;
  
  {
    u32 col=RGB15(0,0,0)|BIT(15);
    col|=col<<16;
    MemSet32CPU(col,pScreenMain->pBackCanvas->GetVRAMBuf(),ScreenWidth*ScreenHeight*2);
    MemSet32CPU(col,pScreenMain->pViewCanvas->GetVRAMBuf(),ScreenWidth*ScreenHeight*2);
  }
  
  DiskCache_Init();
  
  pCStream=_pCStream;
  DataTopPosition=pCStream->GetOffset();
  
  framenum=0;
  
  decoder=mpeg2_init();
  if(decoder==NULL){
    _consolePrint("Could not allocate a decoder object.\n");
    return;
  }
  info=mpeg2_info(decoder);
  
  if(ProcSequence()==false) return;
  
  if(Width!=ScreenWidth) StopFatalError(14701,"Video width (%dpix) !=  Screen width (%dpix).\n",Width,ScreenWidth);
  
  FrameCache_Init(Width*Height*2);
  
  Initialized=true;
}

Clibmpg::~Clibmpg(void)
{
  if(Initialized==true){
    Initialized=false;
    
    if(decoder!=NULL){
      mpeg2_close(decoder); decoder=NULL;
    }
    info=NULL;
    
    FrameCache_Free();
    DiskCache_Free();
  }
}

void Clibmpg::Reopen(u32 StartFrame,u32 StartOffset)
{
  if(Initialized==true){
    Initialized=false;
    
    if(decoder!=NULL){
      mpeg2_close(decoder); decoder=NULL;
    }
    info=NULL;
  }
  
  framenum=StartFrame;
  
  DPGFS_Movie_SetOffset(StartOffset&~3);
  DiskCache_Clear();
  
  decoder=mpeg2_init();
  if(decoder==NULL){
    _consolePrint("Could not allocate a decoder object.\n");
    return;
  }
  info=mpeg2_info(decoder);
  
  if(ProcSequence()==false) return;
  
  REG_IME=0;
  FrameCache_Clear();
  REG_IME=1;
  
  Initialized=true;
}

bool Clibmpg::ProcReadBuffer(void)
{
  u8 *pbuf=NULL;
  int readsize=DiskCache_ReadOneBlock((void**)&pbuf);

  if((readsize!=0)&&(pbuf!=NULL)){
    mpeg2_buffer(decoder, pbuf,&pbuf[readsize]);
    return(true);
  }
  
  return(false);
}

bool Clibmpg::ProcSequence(void)
{
  while(1){
    mpeg2_state_t state=mpeg2_parse(decoder);

    switch (state) {
      case STATE_BUFFER: {
        if(ProcReadBuffer()==false) return(false);
      } break;
      case STATE_SEQUENCE: {
        Width=info->sequence->width;
        Height=info->sequence->height;
        YUV420toBGR15_Init(Width,Height);
        _consolePrint("Sequence frame detected.\n");
        return(true);
      } break;
      case STATE_GOP: break;
      case STATE_SLICE: {
        _consolePrint("ProcSeq:StateError found first.(Slice/End/InvalidEnd)\n");
        return(false);
      } break;
      default: break;
    }
  }
}

int Clibmpg::GetWidth(void) const
{
  return(Width);
}

int Clibmpg::GetHeight(void) const
{
  return(Height);
}

bool Clibmpg::ProcMoveFrame(u32 TargetFrame,u64 TargetSamplesCount)
{
  mpeg2_skip(decoder,false);
  if(framenum==TargetFrame) return(true);
  
  if(TargetFrame<framenum){
    Reopen(0,0);
  }
  
  if(TargetFrame==0) return(true);
  
  mpeg2_skip(decoder,true);
  
  while(1){
    mpeg2_state_t state=mpeg2_parse(decoder);
    
    switch (state) {
      case STATE_BUFFER: {
        if(ProcReadBuffer()==false) return(false);
      } break;
      case STATE_SEQUENCE: break;
      case STATE_GOP: break;
      case STATE_SLICE: {
        framenum++;
        if(TargetFrame<=framenum){
          mpeg2_skip(decoder,false);
          return(true);
        }
      } break;
      default: break;
    }
  }
}

bool Clibmpg::ProcMoveFrameGOP(u32 TargetFrame,u64 TargetSamplesCount,u32 TargetGOPFrame,u32 TargetGOPOffset)
{
  _consolePrintf("framenum=%d TargetFrame=%d\n",framenum,TargetFrame);
  
  if(framenum==TargetFrame) return(true);
  
  Reopen(TargetGOPFrame,TargetGOPOffset&~3);
  return(true);
  
  if((TargetGOPFrame<=framenum)&&(framenum<=TargetFrame)){
    }else{
    Reopen(TargetGOPFrame,TargetGOPOffset&~3);
  }
  
  if(framenum==TargetFrame) return(true);
  
  while(1){
    mpeg2_state_t state=mpeg2_parse(decoder);
    
    switch (state) {
      case STATE_BUFFER: {
        if(ProcReadBuffer()==false) return(false);
      } break;
      case STATE_SEQUENCE: break;
      case STATE_GOP: break;
      case STATE_SLICE: {
        framenum++;
        if(TargetFrame<=framenum){
          return(true);
        }
      } break;
      default: break;
    }
  }
}

extern vu64 DPGAudioStream_SyncSamples;

extern void UpdateDPG_Audio(void);

extern "C" {
void VRAMWriteCache_Enable(void);
void VRAMWriteCache_Disable(void);
}

bool Clibmpg::ProcDecode(void)
{
  UpdateDPG_Audio();
  
  {
    u64 framesamples=(u64)framenum*SndFreq*0x100/FPS;
    
    FrameCache_WriteSetShowSamples(framesamples);
    
    if(DPGAudioStream_SyncSamples!=0){
      while(FrameCache_isWriteFull()==true){
        if(DiskCache_LoadOneBuffer()==false) break;
      }
    }
    
    if(DPGAudioStream_SyncSamples<framesamples){
      if(FrameCache_isWriteFull()==true) return(true);
    }
  }
  
  framenum++;
  if(TotalFrameCount<=framenum){
    _consolePrint("End of video stream.\n");
    return(false);
  }
  
  DATA_IN_MTCM_VAR static bool skipflag=false;
  
  while(1){
    mpeg2_state_t state=mpeg2_parse(decoder);
    
    UpdateDPG_Audio();
    
    switch (state) {
      case STATE_BUFFER: {
        if(ProcReadBuffer()==false) return(false);
      } break;
      case STATE_SEQUENCE: break;
      case STATE_GOP: {
        u64 framesamples=FrameCache_WriteGetShowSamples();
        if(DPGAudioStream_SyncSamples<framesamples){
          if(skipflag==true){
            skipflag=false;
            mpeg2_skip(decoder,skipflag);
          }
          }else{
          s32 LastDelaySamples=(s32)(DPGAudioStream_SyncSamples-framesamples);
          s32 delayms=LastDelaySamples*1000/SndFreq;
          if(msGOPSkip<delayms){
            if(skipflag==false){
              skipflag=true;
              mpeg2_skip(decoder,skipflag);
              _consolePrintf("DropGOP. delayms=%d\n",delayms);
            }
          }
        }
      } break;
      case STATE_SLICE: {
        if((skipflag==false)&&(info->display_fbuf)){
          if(DPGAudioStream_SyncSamples<FrameCache_WriteGetShowSamples()){
            // frame cache mode
            u16 *pVRAMBuf=FrameCache_WriteStart();
            if((framenum&1)==0){
              YUV420toBGR15Fix_CopyFull1_asm(info->display_fbuf->buf,pVRAMBuf);
              }else{
              YUV420toBGR15Fix_CopyFull2_asm(info->display_fbuf->buf,pVRAMBuf);
            }
            FrameCache_WriteEnd();
            }else{
            // direct VRAM mode
            REG_IME=0;
            if(reqflip!=0){
              if(reqflip==3) pScreenMain->Flip(false);
              pScreenMain->SetBlendLevel(16);
              reqflip=0;
            }
            REG_IME=1;
            
            // ARM9 processing converter
            bool reqtrans=true;
            
            u16 *pVRAMBuf=pScreenMain->pBackCanvas->GetVRAMBuf();
            
            if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
            
            s32 LastDelaySamples=(s32)((s64)DPGAudioStream_SyncSamples-FrameCache_WriteGetShowSamples());
            s32 delayms=LastDelaySamples*1000/(s32)SndFreq;
            if(delayms<(s32)msGOPSkip){
              pVRAMBuf=&pVRAMBuf[((ScreenHeight-Height)/2)*ScreenWidth];
              if((framenum&1)==0){
                YUV420toBGR15Fix_CopyFull1_asm(info->display_fbuf->buf,pVRAMBuf);
                }else{
                YUV420toBGR15Fix_CopyFull2_asm(info->display_fbuf->buf,pVRAMBuf);
              }
              // _consolePrintf("direct. delayms=%d\n",delayms);
              }else{
              reqtrans=false;
              // _consolePrintf("skip. delayms=%d\n",delayms);
            }
            
            if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
            
            if(reqtrans==true) reqflip=3;
          }
        }
        
        UpdateDPG_Audio();
        return(true);
      } break;
      default: break;
    }
  }
}

void Clibmpg::SliceOneFrame(u16 *pVRAMBuf1,u16 *pVRAMBuf2)
{
  u32 align=((ScreenHeight-Height)/2)*ScreenWidth;
  pVRAMBuf1=&pVRAMBuf1[align];
  pVRAMBuf2=&pVRAMBuf2[align];
  
  framenum++;
  if(TotalFrameCount<=framenum){
    MemSet32CPU(0,pVRAMBuf1,Width*Height*2);
    MemSet32CPU(0,pVRAMBuf2,Width*Height*2);
    _consolePrint("End of video stream.\n");
    return;
  }
  
  while(1){
    mpeg2_state_t state=mpeg2_parse(decoder);
    
    switch (state) {
      case STATE_BUFFER: {
        if(ProcReadBuffer()==false) return;
      } break;
      case STATE_SEQUENCE: break;
      case STATE_GOP: {
      } break;
      case STATE_SLICE: {
        if(info->display_fbuf){
          swiWaitForVBlank();
          YUV420toBGR15Fix_CopyFull1_asm(info->display_fbuf->buf,pVRAMBuf2);
          YUV420toBGR15Fix_CopyFull2_asm(info->display_fbuf->buf,pVRAMBuf1);
          return;
        }
      } break;
      default: break;
    }
  }
}

int Clibmpg::GetFrameNum(void)
{
  return(framenum);
}


// -----------------------------------------------------------------

extern u8 LimitTable[128+256+128];
DATA_IN_MTCM_VAR u8 LimitTable[128+256+128];

extern u32 ConvertSize;
DATA_IN_MTCM_VAR u32 ConvertSize;

#define FIX16(x) ((s32)(x*0x10000))
extern const s32 fixtable[4];
DATA_IN_MTCM_SET const s32 fixtable[4]={FIX16(-0.3441),FIX16( 1.4020),FIX16(-0.7139),FIX16( 1.7718)};

void Clibmpg::YUV420toBGR15_Init(int Width,int Height)
{
  TYUV420toBGR15_DATA *id=&YUV420toBGR15_DATA;
  
  id->width=Width;
  id->height=Height;
  id->Ydiv2=Height/2;
  
  switch(PixelFormat){
    case PF_RGB24: _consolePrint("Setup limitter for RGB24.\n"); break;
    case PF_RGB15: case PF_RGB18: case PF_RGB21: StopFatalError(14702,"Not support pixel format.\n"); break;
    default: StopFatalError(14703,"Unknown pixel format.\n"); break;
  }
  
  for(s32 idx=-128;idx<256+128;idx++){
    s32 c=idx-16;
    if(c<0) c=0;
    if(255<c) c=255;
    LimitTable[128+idx]=c>>3;
  }
  
  ConvertSize=id->Ydiv2*(ScreenWidth/2);
}

void Clibmpg::YUV420toBGR15Fix_CopyFull0_asm(const void *fbuf,u16 *_FrameBuf)
{
  // r13=sp r14=lr s15=pc
  asm volatile (
	"stmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,lr} \n\t"
	" \n\t"
	"ldmia r1,{r0,r1,r3} \n\t"
	" \n\t"
	"py .req r0 \n\t"
	"pu .req r1 \n\t"
	"pv .req r3 \n\t"
	"pdst .req r2 \n\t"
	" \n\t"
	"pLimitTable .req r12 \n\t"
	" \n\t"
	".global LimitTable \n\t"
	"ldr pLimitTable,=LimitTable \n\t"
	"add pLimitTable,pLimitTable,#128*1 \n\t"
	" \n\t"
	"colormask .req r11 \n\t"
	"mov colormask,#0x8000 \n\t"
	"orr colormask,colormask,colormask,lsl #16 \n\t"
	" \n\t"
	"loopcnt .req r14 \n\t"
	".global  ConvertSize \n\t"
	"ldr loopcnt,=ConvertSize \n\t"
	"ldr loopcnt,[loopcnt] \n\t"
	" \n\t"
	"YUV420toBGR15Fix_CopyFull0_asm_body_loop: \n\t"
	" \n\t"
	"cb .req r4 \n\t"
	"cr .req r5 \n\t"
	" \n\t"
	"ldrb cb,[pu],#1 \n\t"
	"ldrb cr,[pv],#1 \n\t"
	" \n\t"
	"pfixtable .req r6 \n\t"
	"fixm03441 .req r6 \n\t"
	"fixp14020 .req r7 \n\t"
	"fixm07139 .req r8 \n\t"
	"fixp17718 .req r9 \n\t"
	" \n\t"
	".global  fixtable \n\t"
	"ldr pfixtable,=fixtable \n\t"
	"sub cb,cb,#0x80 @ for interlock \n\t"
	"sub cr,cr,#0x80 @ for interlock \n\t"
	"ldmia pfixtable,{fixm03441,fixp14020,fixm07139,fixp17718} \n\t"
	" \n\t"
	"tr .req cr \n\t"
	"tg .req r6 \n\t"
	"tb .req cb \n\t"
	" \n\t"
	"smulwb tr,fixp14020,cr \n\t"
	"smulwb tg,fixm03441,cb \n\t"
	" \n\t"
	"y0 .req r10 \n\t"
	"y1 .req r10 \n\t"
	"tmp1 .req r7 \n\t"
	"tmp2 .req r8 \n\t"
	"rgb .req r9 \n\t"
	" \n\t"
	"ldrb y0,[py,#256+0] \n\t"
	"smlawb tg,fixm07139,cr,tg \n\t"
	"smulwb tb,fixp17718,cb \n\t"
	"add y0,pLimitTable,y0 \n\t"
	"@  add y0,#0 @ for Dither \n\t"
	"ldrb rgb,[y0,tr] \n\t"
	"ldrb tmp1,[y0,tg] \n\t"
	"ldrb tmp2,[y0,tb] \n\t"
	"ldrb y1,[py,#256+1] \n\t"
	"orr rgb,rgb,colormask \n\t"
	"orr rgb,rgb,tmp1,lsl #5 \n\t"
	"add y1,pLimitTable,y1 \n\t"
	"@  add y1,#4 @ for Dither \n\t"
	"ldrb tmp1,[y1,tr] \n\t"
	"orr rgb,rgb,tmp2,lsl #10 \n\t"
	"ldrb tmp2,[y1,tg] \n\t"
	"orr rgb,rgb,tmp1,lsl #16+0 \n\t"
	"ldrb tmp1,[y1,tb] \n\t"
	"orr rgb,rgb,tmp2,lsl #16+5 \n\t"
	" \n\t"
	"@ ----------------- \n\t"
	" \n\t"
	"ldrb y0,[py,#0+0] \n\t"
	"orr rgb,rgb,tmp1,lsl #16+10 @ for stage.1 \n\t"
	"str rgb,[pdst,#256*1*2] @ for stage.1 \n\t"
	"add y0,pLimitTable,y0 \n\t"
	"@  add y0,#2 @ for Dither \n\t"
	"ldrb rgb,[y0,tr] \n\t"
	"ldrb tmp1,[y0,tg] \n\t"
	"ldrb tmp2,[y0,tb] \n\t"
	"ldrb y1,[py,#0+1] \n\t"
	"orr rgb,rgb,colormask \n\t"
	"orr rgb,rgb,tmp1,lsl #5 \n\t"
	"add y1,pLimitTable,y1 \n\t"
	"@  add y1,#6 @ for Dither \n\t"
	"ldrb tmp1,[y1,tr] \n\t"
	"orr rgb,rgb,tmp2,lsl #10 \n\t"
	"ldrb tmp2,[y1,tg] \n\t"
	"orr rgb,rgb,tmp1,lsl #16+0 \n\t"
	"ldrb tmp1,[y1,tb] \n\t"
	"orr rgb,rgb,tmp2,lsl #16+5 \n\t"
	"add py,py,#2*1 \n\t"
	"orr rgb,rgb,tmp1,lsl #16+10 \n\t"
	" \n\t"
	"str rgb,[pdst],#2*2 \n\t"
	" \n\t"
	"sub loopcnt,#1 \n\t"
	"tst loopcnt,#(256/2)-1 \n\t"
	"bne YUV420toBGR15Fix_CopyFull0_asm_body_loop \n\t"
	" \n\t"
	"add py,#256*1 \n\t"
	"add pdst,#256*2 \n\t"
	" \n\t"
	"cmp loopcnt,#0 \n\t"
	"bne YUV420toBGR15Fix_CopyFull0_asm_body_loop \n\t"
	" \n\t"
	"ldmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,pc}"
	:::"memory"
	);
}

void Clibmpg::YUV420toBGR15Fix_CopyFull1_asm(const void *fbuf,u16 *_FrameBuf)
{
  // r13=sp r14=lr s15=pc
  asm volatile (
	"stmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,lr} \n\t"
	" \n\t"
	"ldmia r1,{r0,r1,r3} \n\t"
	" \n\t"
	"py .req r0 \n\t"
	"pu .req r1 \n\t"
	"pv .req r3 \n\t"
	"pdst .req r2 \n\t"
	" \n\t"
	"pLimitTable .req r12 \n\t"
	" \n\t"
	".global  LimitTable \n\t"
	"ldr pLimitTable,=LimitTable \n\t"
	"add pLimitTable,pLimitTable,#128*1 \n\t"
	" \n\t"
	"colormask .req r11 \n\t"
	"mov colormask,#0x8000 \n\t"
	"orr colormask,colormask,colormask,lsl #16 \n\t"
	" \n\t"
	"loopcnt .req r14 \n\t"
	".global  ConvertSize \n\t"
	"ldr loopcnt,=ConvertSize \n\t"
	"ldr loopcnt,[loopcnt] \n\t"
	" \n\t"
	"YUV420toBGR15Fix_CopyFull1_asm_body_loop: \n\t"
	" \n\t"
	"cb .req r4 \n\t"
	"cr .req r5 \n\t"
	" \n\t"
	"ldrb cb,[pu],#1 \n\t"
	"ldrb cr,[pv],#1 \n\t"
	" \n\t"
	"pfixtable .req r6 \n\t"
	"fixm03441 .req r6 \n\t"
	"fixp14020 .req r7 \n\t"
	"fixm07139 .req r8 \n\t"
	"fixp17718 .req r9 \n\t"
	" \n\t"
	".global  fixtable \n\t"
	"ldr pfixtable,=fixtable \n\t"
	"sub cb,cb,#0x80 @ for interlock \n\t"
	"sub cr,cr,#0x80 @ for interlock \n\t"
	"ldmia pfixtable,{fixm03441,fixp14020,fixm07139,fixp17718} \n\t"
	" \n\t"
	"tr .req cr \n\t"
	"tg .req r6 \n\t"
	"tb .req cb \n\t"
	" \n\t"
	"smulwb tr,fixp14020,cr \n\t"
	"smulwb tg,fixm03441,cb \n\t"
	" \n\t"
	"y0 .req r10 \n\t"
	"y1 .req r10 \n\t"
	"tmp1 .req r7 \n\t"
	"tmp2 .req r8 \n\t"
	"rgb .req r9 \n\t"
	" \n\t"
	"ldrb y0,[py,#256+0] \n\t"
	"smlawb tg,fixm07139,cr,tg \n\t"
	"smulwb tb,fixp17718,cb \n\t"
	"add y0,pLimitTable,y0 \n\t"
	"@  add y0,#0 @ for Dither \n\t"
	"ldrb rgb,[y0,tr] \n\t"
	"ldrb tmp1,[y0,tg] \n\t"
	"ldrb tmp2,[y0,tb] \n\t"
	"ldrb y1,[py,#256+1] \n\t"
	"orr rgb,rgb,colormask \n\t"
	"orr rgb,rgb,tmp1,lsl #5 \n\t"
	"add y1,pLimitTable,y1 \n\t"
	"add y1,#4 @ for Dither \n\t"
	"ldrb tmp1,[y1,tr] \n\t"
	"orr rgb,rgb,tmp2,lsl #10 \n\t"
	"ldrb tmp2,[y1,tg] \n\t"
	"orr rgb,rgb,tmp1,lsl #16+0 \n\t"
	"ldrb tmp1,[y1,tb] \n\t"
	"orr rgb,rgb,tmp2,lsl #16+5 \n\t"
	" \n\t"
	"@ ----------------- \n\t"
	" \n\t"
	"ldrb y0,[py,#0+0] \n\t"
	"orr rgb,rgb,tmp1,lsl #16+10 @ for stage.1 \n\t"
	"str rgb,[pdst,#256*1*2] @ for stage.1 \n\t"
	"add y0,pLimitTable,y0 \n\t"
	"add y0,#2 @ for Dither \n\t"
	"ldrb rgb,[y0,tr] \n\t"
	"ldrb tmp1,[y0,tg] \n\t"
	"ldrb tmp2,[y0,tb] \n\t"
	"ldrb y1,[py,#0+1] \n\t"
	"orr rgb,rgb,colormask \n\t"
	"orr rgb,rgb,tmp1,lsl #5 \n\t"
	"add y1,pLimitTable,y1 \n\t"
	"add y1,#6 @ for Dither \n\t"
	"ldrb tmp1,[y1,tr] \n\t"
	"orr rgb,rgb,tmp2,lsl #10 \n\t"
	"ldrb tmp2,[y1,tg] \n\t"
	"orr rgb,rgb,tmp1,lsl #16+0 \n\t"
	"ldrb tmp1,[y1,tb] \n\t"
	"orr rgb,rgb,tmp2,lsl #16+5 \n\t"
	"add py,py,#2*1 \n\t"
	"orr rgb,rgb,tmp1,lsl #16+10 \n\t"
	" \n\t"
	"str rgb,[pdst],#2*2 \n\t"
	" \n\t"
	"sub loopcnt,#1 \n\t"
	"tst loopcnt,#(256/2)-1 \n\t"
	"bne YUV420toBGR15Fix_CopyFull1_asm_body_loop \n\t"
	" \n\t"
	"add py,#256*1 \n\t"
	"add pdst,#256*2 \n\t"
	" \n\t"
	"cmp loopcnt,#0 \n\t"
	"bne YUV420toBGR15Fix_CopyFull1_asm_body_loop \n\t"
	" \n\t"
	"ldmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,pc} \n\t"
	:::"memory"
	);
}

void Clibmpg::YUV420toBGR15Fix_CopyFull2_asm(const void *fbuf,u16 *_FrameBuf)
{
  // r13=sp r14=lr s15=pc
  asm volatile (
	"stmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,lr} \n\t"
	" \n\t"
	"ldmia r1,{r0,r1,r3} \n\t"
	" \n\t"
	"py .req r0 \n\t"
	"pu .req r1 \n\t"
	"pv .req r3 \n\t"
	"pdst .req r2 \n\t"
	" \n\t"
	"pLimitTable .req r12 \n\t"
	" \n\t"
	".global  LimitTable \n\t"
	"ldr pLimitTable,=LimitTable \n\t"
	"add pLimitTable,pLimitTable,#128*1 \n\t"
	" \n\t"
	"colormask .req r11 \n\t"
	"mov colormask,#0x8000 \n\t"
	"orr colormask,colormask,colormask,lsl #16 \n\t"
	" \n\t"
	"loopcnt .req r14 \n\t"
	".global  ConvertSize \n\t"
	"ldr loopcnt,=ConvertSize \n\t"
	"ldr loopcnt,[loopcnt] \n\t"
	" \n\t"
	"YUV420toBGR15Fix_CopyFull2_asm_body_loop: \n\t"
	" \n\t"
	"cb .req r4 \n\t"
	"cr .req r5 \n\t"
	" \n\t"
	"ldrb cb,[pu],#1 \n\t"
	"ldrb cr,[pv],#1 \n\t"
	" \n\t"
	"pfixtable .req r6 \n\t"
	"fixm03441 .req r6 \n\t"
	"fixp14020 .req r7 \n\t"
	"fixm07139 .req r8 \n\t"
	"fixp17718 .req r9 \n\t"
	" \n\t"
	".global  fixtable \n\t"
	"ldr pfixtable,=fixtable \n\t"
	"sub cb,cb,#0x80 @ for interlock \n\t"
	"sub cr,cr,#0x80 @ for interlock \n\t"
	"ldmia pfixtable,{fixm03441,fixp14020,fixm07139,fixp17718} \n\t"
	" \n\t"
	"tr .req cr \n\t"
	"tg .req r6 \n\t"
	"tb .req cb \n\t"
	" \n\t"
	"smulwb tr,fixp14020,cr \n\t"
	"smulwb tg,fixm03441,cb \n\t"
	" \n\t"
	"y0 .req r10 \n\t"
	"y1 .req r10 \n\t"
	"tmp1 .req r7 \n\t"
	"tmp2 .req r8 \n\t"
	"rgb .req r9 \n\t"
	" \n\t"
	"ldrb y0,[py,#256+0] \n\t"
	"smlawb tg,fixm07139,cr,tg \n\t"
	"smulwb tb,fixp17718,cb \n\t"
	"add y0,pLimitTable,y0 \n\t"
	"add y1,#6 @ for Dither \n\t"
	"ldrb rgb,[y0,tr] \n\t"
	"ldrb tmp1,[y0,tg] \n\t"
	"ldrb tmp2,[y0,tb] \n\t"
	"ldrb y1,[py,#256+1] \n\t"
	"orr rgb,rgb,colormask \n\t"
	"orr rgb,rgb,tmp1,lsl #5 \n\t"
	"add y1,pLimitTable,y1 \n\t"
	"add y1,#2 @ for Dither \n\t"
	"ldrb tmp1,[y1,tr] \n\t"
	"orr rgb,rgb,tmp2,lsl #10 \n\t"
	"ldrb tmp2,[y1,tg] \n\t"
	"orr rgb,rgb,tmp1,lsl #16+0 \n\t"
	"ldrb tmp1,[y1,tb] \n\t"
	"orr rgb,rgb,tmp2,lsl #16+5 \n\t"
	" \n\t"
	"@ ----------------- \n\t"
	" \n\t"
	"ldrb y0,[py,#0+0] \n\t"
	"orr rgb,rgb,tmp1,lsl #16+10 @ for stage.1 \n\t"
	"str rgb,[pdst,#256*1*2] @ for stage.1 \n\t"
	"add y0,pLimitTable,y0 \n\t"
	"add y0,#4 @ for Dither \n\t"
	"ldrb rgb,[y0,tr] \n\t"
	"ldrb tmp1,[y0,tg] \n\t"
	"ldrb tmp2,[y0,tb] \n\t"
	"ldrb y1,[py,#0+1] \n\t"
	"orr rgb,rgb,colormask \n\t"
	"orr rgb,rgb,tmp1,lsl #5 \n\t"
	"add y1,pLimitTable,y1 \n\t"
	"@ add y0,#0 @ for Dither \n\t"
	"ldrb tmp1,[y1,tr] \n\t"
	"orr rgb,rgb,tmp2,lsl #10 \n\t"
	"ldrb tmp2,[y1,tg] \n\t"
	"orr rgb,rgb,tmp1,lsl #16+0 \n\t"
	"ldrb tmp1,[y1,tb] \n\t"
	"orr rgb,rgb,tmp2,lsl #16+5 \n\t"
	"add py,py,#2*1 \n\t"
	"orr rgb,rgb,tmp1,lsl #16+10 \n\t"
	" \n\t"
	"str rgb,[pdst],#2*2 \n\t"
	" \n\t"
	"sub loopcnt,#1 \n\t"
	"tst loopcnt,#(256/2)-1 \n\t"
	"bne YUV420toBGR15Fix_CopyFull2_asm_body_loop \n\t"
	" \n\t"
	"add py,#256*1 \n\t"
	"add pdst,#256*2 \n\t"
	" \n\t"
	"cmp loopcnt,#0 \n\t"
	"bne YUV420toBGR15Fix_CopyFull2_asm_body_loop \n\t"
	" \n\t"
	"ldmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,pc} \n\t"
	:::"memory"
	);
}

#if 0

0,4 6,2
2,6 4,0

#endif
