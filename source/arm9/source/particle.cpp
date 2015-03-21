
#include <nds.h>

#include "_console.h"
#include "_consoleWriteLog.h"

#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "skin.h"

#include "particle.h"

DATA_IN_AfterSystem static bool Particle_MouseDown;
DATA_IN_AfterSystem static u32 Particle_MouseX,Particle_MouseY;

typedef struct {
  s32 x,y,sx,sy;
} TParticle_Part;

#define PartsCount (64)

typedef struct {
  u8 *pdstbuf;
  float ax,ay;
  TParticle_Part Parts[PartsCount];
} TParticle;

DATA_IN_AfterSystem static TParticle *pParticle;

void Particle_Init(void)
{
  Particle_MouseDown=false;
  Particle_MouseX=0;
  Particle_MouseY=0;
  
  pParticle=(TParticle*)safemalloc_chkmem(&MM_Process,sizeof(TParticle));
  
  pParticle->pdstbuf=NULL;
  
  pParticle->ax=3.14159*-0.2;
  pParticle->ay=3.14159*0.2;
  
  for(u32 idx=0;idx<PartsCount;idx++){
    TParticle_Part *ppart=&pParticle->Parts[idx];
    ppart->x=0*256;
    ppart->y=0*256;
    ppart->sx=0*256;
    ppart->sy=0*256;
  }
}

void Particle_Free(void)
{
  if(pParticle->pdstbuf!=NULL){
    safefree(&MM_Process,pParticle->pdstbuf); pParticle->pdstbuf=NULL;
  }
  
  if(pParticle!=NULL){
    safefree(&MM_Process,pParticle); pParticle=NULL;
  }
}

#include <math.h>

#define r(x) (x/12)
static const u8 ParticleColorMap[8][8]={
  {r(0x02),r(0x1E),r(0x37),r(0x4B),r(0x4F),r(0x43),r(0x2B),r(0x0F)},
  {r(0x1A),r(0x3F),r(0x67),r(0x85),r(0x8D),r(0x79),r(0x53),r(0x2B)},
  {r(0x2B),r(0x5D),r(0x94),r(0xBF),r(0xCA),r(0xAD),r(0x78),r(0x43)},
  {r(0x35),r(0x6C),r(0xAB),r(0xE4),r(0xF5),r(0xCA),r(0x8B),r(0x4F)},
  {r(0x32),r(0x67),r(0xA4),r(0xD6),r(0xE4),r(0xBF),r(0x85),r(0x4B)},
  {r(0x23),r(0x4F),r(0x7F),r(0xA4),r(0xAD),r(0x94),r(0x67),r(0x38)},
  {r(0x0B),r(0x2D),r(0x4F),r(0x67),r(0x6D),r(0x5E),r(0x3F),r(0x1E)},
  {r(0x00),r(0x0B),r(0x23),r(0x32),r(0x35),r(0x2B),r(0x19),r(0x03)},
};
#undef r

static inline void Particle_Update_ins_Draw(CglCanvas *pcan,s32 x,s32 y)
{
  if((x<4)||((ScreenWidth-4)<=x)) return;
  if((y<4)||((ScreenHeight-4)<=y)) return;
  
  x-=4;
  y-=4;
  
  u8 *pbuf=pParticle->pdstbuf;
  pbuf+=y*ScreenWidth;
  pbuf+=x;
  const u8 *pcolmap=(const u8*)ParticleColorMap;
  for(u32 py=0;py<8;py++){
    for(u32 px=0;px<8;px++){
      u32 c=pbuf[px];
      c+=*pcolmap++;
      if(255<c) c=255;
      pbuf[px]=c;
    }
    pbuf+=ScreenWidth;
  }
}

void Particle_Update(u32 VsyncCount,CglCanvas *pcan)
{
  if(Skin_OwnerDrawText.DisableParticles==true) return;
  
  bool firststart;
  
  if(pParticle->pdstbuf==NULL){
    firststart=true;
    pParticle->pdstbuf=(u8*)safemalloc_chkmem(&MM_Process,ScreenWidth*ScreenHeight);
    }else{
    firststart=false;
  }
  
  bool touch=Particle_MouseDown;
  s32 touchx=0,touchy=0;
  
  if(touch==true){
    touchx=Particle_MouseX;
    touchy=Particle_MouseY;
  }
  
  {
    TParticle_Part *ppart=&pParticle->Parts[0];
    if(touch==true){
      ppart->x=touchx*256;
      ppart->y=touchy*256;
      }else{
      float ax=pParticle->ax,ay=pParticle->ay;
      ax+=0.0160;
      ay+=0.0100;
      float cycle=3.14159*0.9;
      if(cycle<ax) ax-=cycle;
      if(cycle<ay) ay-=cycle;
      float ofs=(cycle-(3.14159*0.5))/2;
      ppart->x=(ScreenWidth+(sin(-ofs-ax)*128))*256;
      ppart->y=(ScreenHeight+(cos(-ofs-ay)*128))*256;
      pParticle->ax=ax;
      pParticle->ay=ay;
      if(firststart==true){
        TParticle_Part *ptagpart=&pParticle->Parts[0];
        for(u32 idx=1;idx<PartsCount;idx++){ // idx0‚Íˆ—‚µ‚È‚¢
          TParticle_Part *ppart=&pParticle->Parts[idx];
          ppart->x=ptagpart->x;
          ppart->y=ptagpart->y;
        }
      }
    }
  }
  
  for(u32 loop=0;loop<VsyncCount*3;loop++){
    for(u32 idx=1;idx<PartsCount;idx++){ // idx0‚Íˆ—‚µ‚È‚¢
      TParticle_Part *ptagpart=&pParticle->Parts[idx-1];
      TParticle_Part *ppart=&pParticle->Parts[idx];
      
      const s32 range=8;
      s32 vx=(ptagpart->x-ppart->x)/range;
      s32 vy=(ptagpart->y-ppart->y)/range;
      ppart->sx+=vx;
      ppart->sy+=vy;
      
      if(touch==false){
        ppart->sx=ppart->sx*300/1024;
        ppart->sy=ppart->sy*300/1024;
        }else{
        ppart->sx=ppart->sx*696/1024;
        ppart->sy=ppart->sy*696/1024;
      }
      ppart->x+=ppart->sx;
      ppart->y+=ppart->sy;
      
      if((-1<ppart->sx)&&(ppart->sx<2)&&(-1<ppart->sy)&&(ppart->sy<2)){
        if(vx==0) ppart->x=ptagpart->x;
        if(vy==0) ppart->y=ptagpart->y;
      }
    }
  }
  
  MemSet32CPU(0,pParticle->pdstbuf,ScreenWidth*ScreenHeight);
  
  s32 lx,ly;
  {
    TParticle_Part *ppart=&pParticle->Parts[0];
    lx=ppart->x;
    ly=ppart->y;
  }
  
  for(u32 idx=1;idx<PartsCount;idx++){
    TParticle_Part *ppart=&pParticle->Parts[idx];
    s32 x=ppart->x,y=ppart->y;
    
    s32 ax=lx-x,ay=ly-y;
    lx=x;
    ly=y;
    
    s32 cnt;
    if(abs(ay)<abs(ax)){
      cnt=abs(ax)/3;
      }else{
      cnt=abs(ay)/3;
    }
    cnt/=256;
    cnt++;
    if(3<cnt) cnt=3;
    
    ax/=cnt;
    ay/=cnt;
    for(s32 idx=0;idx<cnt;idx++){
      Particle_Update_ins_Draw(pcan,x/256,y/256);
      x+=ax;
      y+=ay;
    }
  }
  
  u32 *psrc32buf=(u32*)pParticle->pdstbuf;
  u16 *pdstbuf=pcan->GetVRAMBuf();
  
  for(u32 idx=0;idx<ScreenWidth*ScreenHeight;idx+=4){
    u32 src32=*psrc32buf++;
    if(src32==0){
      pdstbuf+=4;
      }else{
      u16 dst;
      for(u32 idx=0;idx<4;idx++){
        u32 src=(src32&0xff)>>2;
        src32>>=8;
        if(src==0){
          pdstbuf++;
          }else{
          dst=*pdstbuf;
          u32 dstb=(dst>>10)&0x1f;
          u32 dstg=(dst>>5)&0x1f;
          u32 dstr=(dst>>0)&0x1f;
          dstb+=src;
          if(31<dstb) dstb=31;
          src>>=1;
          dstg+=src;
          if(31<dstg) dstg=31;
          dstr+=src;
          if(31<dstr) dstr=31;
          *pdstbuf++=RGB15(dstr,dstg,dstb)|BIT15;
        }
      }
    }
  }
  
  firststart=false;
}

void Particle_SetMouseDown(s32 x,s32 y)
{
  Particle_MouseDown=true;
  Particle_MouseX=x;
  Particle_MouseY=y;
}

void Particle_SetMouseMove(s32 x,s32 y)
{
  if(Particle_MouseDown==true){
    Particle_MouseX=x;
    Particle_MouseY=y;
  }
}

void Particle_SetMouseUp(void)
{
  Particle_MouseDown=false;
}

