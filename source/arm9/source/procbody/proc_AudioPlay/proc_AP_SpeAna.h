
#include "proc_AP_SpeAna_algo_fft.h"

#define BandDiv (4)
#define BandCount (16)
DATA_IN_IWRAM_AudioPlay static const u32 BandScale[BandCount]={
   6/BandDiv,  10/BandDiv,  14/BandDiv,  19/BandDiv,
  26/BandDiv,  36/BandDiv,  49/BandDiv,  66/BandDiv,
  90/BandDiv, 123/BandDiv, 168/BandDiv, 229/BandDiv,
 313/BandDiv, 427/BandDiv, 582/BandDiv, 794/BandDiv,
};
#undef BandDiv

DATA_IN_IWRAM_AudioPlay static s32 SpeAna_CurLev[BandCount]={0,};
DATA_IN_IWRAM_AudioPlay static s32 SpeAna_PeakLev[BandCount]={0,};
DATA_IN_IWRAM_AudioPlay static s32 SpeAna_PeakHoldCount[BandCount]={0,};

static void DrawSpeAna(CglCanvas *pcan,bool isInit)
{
  if(ProcState.Audio.DrawSpectrumAnalyzer==false) return;
  
  if(isInit==true){
    for(u32 idx=0;idx<BandCount;idx++){
      SpeAna_CurLev[idx]=0;
      SpeAna_PeakLev[idx]=0;
      SpeAna_PeakHoldCount[idx]=0;
    }
  }
  
  float v2[BandCount]; // FFT 2乗和
  
  static float x[FFT_Samples],y[FFT_Samples]; // FFT 実数部，虚数部
  
  if(isInit==true){
    for(u32 idx=0;idx<FFT_Samples;idx++){
      x[idx]=0;
      y[idx]=0;
    }
    }else{
    u32 PlayIndex=strpcmRingBufReadIndex;
    u32 BaseSamples=IPC6->strpcmSamples;
    s16 *psrclr=(s16*)strpcmRingLRBuf[PlayIndex];
    u32 pos=0;
    u32 max=BaseSamples*2;
    
    for(u32 idx=0;idx<FFT_Samples;idx++){
      s32 s=(psrclr[pos++]+psrclr[pos++])/2;
      if(pos==max){
        PlayIndex++;
        if(PlayIndex==strpcmRingBufCount) PlayIndex=0;
        psrclr=(s16*)strpcmRingLRBuf[PlayIndex];
        pos=0;
      }
      x[idx]=s*HummingWindow[idx];
      y[idx]=0;
    }
  }
  
//  PrfStart();
  FFT_Exec(x,y);
//  PrfEnd(1);
  
  for(u32 i=0;i<BandCount;i++){
    v2[i]=0;
  }
  
//  PrfStart();
  u32 j=0;
  float tv2=0;
  for(u32 i=1;i<(FFT_Samples/2);i++){
    if(BandScale[j]<=i){
      if(j==0){
        v2[j]=tv2/BandScale[j];
        }else{
        u32 d=BandScale[j]-BandScale[j-1];
        if(d!=0){
          v2[j]=tv2/d;
          }else{
          v2[j]=v2[j-1];
        }
      }
      j++;
      if(BandCount==j) break;
      tv2=0;
    }
    tv2+=(x[i]*x[i])+(y[i]*y[i]);
  }
  
  bool DoubleSpeAna=false;
  if(ProcState.Audio.DrawPlayList==false) DoubleSpeAna=true;
  
  u32 dotw=4;
  u32 h=16;
  if(DoubleSpeAna==true) h*=4;
  if(DoubleSpeAna==true) dotw=5;
  
  {
    s32 tmp[BandCount];
    for(u32 idx=0;idx<BandCount;idx++){
      s32 lev=(s32)(v2[idx]/32768/16*BandScale[idx]*h);
      if(lev<0) lev=0;
      if(h<lev) lev=h;
      tmp[idx]=(SpeAna_CurLev[idx]*1+lev*1)/2;
    }
    SpeAna_CurLev[0]=(tmp[0]+tmp[1])/2;
    SpeAna_CurLev[BandCount-1]=(tmp[BandCount-1-1]+tmp[BandCount-1])/2;
    for(u32 idx=1;idx<BandCount-1;idx++){
      SpeAna_CurLev[idx]=(tmp[idx-1]+tmp[idx]+tmp[idx+1])/3;
    }
  }
  
  for(u32 idx=0;idx<BandCount;idx++){
    if(SpeAna_PeakLev[idx]<SpeAna_CurLev[idx]){
      SpeAna_PeakLev[idx]=SpeAna_CurLev[idx];
      SpeAna_PeakHoldCount[idx]=8;
    }
    if(SpeAna_PeakHoldCount[idx]!=0){
      SpeAna_PeakHoldCount[idx]--;
      }else{
      SpeAna_PeakLev[idx]--;
    }
  }
  
  u32 bandcnt=BandCount;
  if(DoubleSpeAna==true) bandcnt*=3;
  
  u16 *ppbuf[16*4];
  for(u32 y=0;y<h;y++){
    u32 dub=2;
    if(DoubleSpeAna==true) dub=2;
    ppbuf[y]=pcan->GetScanLine(ScreenHeight-4-(h*dub)+(y*dub));
    ppbuf[y]+=ScreenWidth;
    if(DoubleSpeAna==false){
      ppbuf[y]-=4;
      }else{
      ppbuf[y]-=8;
    }
    ppbuf[y]-=dotw*bandcnt;
  }
  
  for(u32 idx=0;idx<bandcnt;idx++){
    s32 lev,peaklev;
    if(DoubleSpeAna==false){
      lev=h-SpeAna_CurLev[idx];
      peaklev=h-SpeAna_PeakLev[idx];
      }else{
      u32 baseidx=idx/3;
      lev=h-SpeAna_CurLev[baseidx+0];
      peaklev=h-SpeAna_PeakLev[baseidx+0];
      if(idx<(bandcnt-3)){
        s32 _lev=h-SpeAna_CurLev[baseidx+1];
        s32 _peaklev=h-SpeAna_PeakLev[baseidx+1];
        switch(idx%3){
          case 0: break;
          case 1: {
            lev=((lev*2)+(_lev*1))/3;
            peaklev=((peaklev*2)+(_peaklev*1))/3;
          } break;
          case 2: {
            lev=((lev*1)+(_lev*2))/3;
            peaklev=((peaklev*1)+(_peaklev*2))/3;
          } break;
        }
      }
    }
    for(u32 y=0;y<h;y++){
      u16 *pbuf=ppbuf[y];
      pbuf+=idx*dotw;
      u32 col;
      if(lev<=y){
        col=RGB15(16,16,31)|BIT15;
        col|=col<<16;
        }else{
        if(peaklev==y){
          col=RGB15(8,8,16)|BIT15;
          col|=col<<16;
          }else{
          col=RGB15(4,4,8)|BIT15;
          col|=col<<16;
        }
      }
      if(DoubleSpeAna==false){
        *(u32*)pbuf=col;
        pbuf+=2;
        *pbuf++=col;
        }else{
        if((idx&1)==0){
          *(u32*)pbuf=col;
          pbuf+=2;
          *(u32*)pbuf=col;
          pbuf+=2;
          }else{
          *pbuf++=col;
          *(u32*)pbuf=col;
          pbuf+=2;
          *pbuf++=col;
        }
      }
    }
  }
}

