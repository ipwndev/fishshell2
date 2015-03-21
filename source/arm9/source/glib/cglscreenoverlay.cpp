
#include <stdlib.h>
#include <nds.h>

#include "glib.h"
#include "glmemtool.h"
#include "cglscreen.h"

#define VRAMBuf ((u16*)&SPRITE_GFX[0])

//a global copy of sprite attribute memory
static SpriteEntry sprites[12];

//rotation attributes overlap so assign then to the same location
static pSpriteRotation spriteRotations = (pSpriteRotation)sprites;

//turn off all the sprites
static inline void initSprites(void)
{
//  MemSet32CPU(0,OAM,128 * sizeof(SpriteEntry));  
  SpriteEntry *pOAM = (SpriteEntry*)OAM;
  for(u32 i=0;i<128;i++)
  {
      pOAM[i].attribute[0] = ATTR0_DISABLED | 255;
      pOAM[i].attribute[1] = 0 | 255;
      pOAM[i].attribute[2] = 0;
  }
  
  MemSet32CPU(0,sprites, 12 * sizeof(SpriteEntry));
  
  u32 i;
  for(i = 0; i < 12; i++)
  {
     sprites[i].attribute[0] = ATTR0_DISABLED | 0;
     sprites[i].attribute[1] = 0 | 0;
     sprites[i].attribute[2] = 0;
  }
  
  for(i=0;i<12/4;i++){
    spriteRotations[i].hdx=1*0x100;
    spriteRotations[i].hdy=0*0x100;
    spriteRotations[i].vdx=0*0x100;
    spriteRotations[i].vdy=1*0x100;
  }

}

//copy our sprite to object attribute memory
static inline void updateOAM(u32 Count)
{
  DC_FlushAll();
  MemCopy32CPU(sprites, OAM, Count * sizeof(SpriteEntry));
}

static inline void SetPosSprite(u32 objno,s32 x,s32 y,bool EnabledBlend)
{
  u32 sx=x;
  u32 sy=y;
  
  if(x<0) sx=x+512;
  if(y<0) sy=y+256;
  
  sx&=512-1;
  sy&=256-1;
  
  sx=x & (512-1);
  sy=y & (256-1);
  
  u16 attr0=ATTR0_NORMAL | ATTR0_BMP | ATTR0_SQUARE | sy;
  if(EnabledBlend==false){
    attr0|=ATTR0_TYPE_NORMAL;
    }else{
    attr0|=ATTR0_TYPE_BLENDED;
  }
  sprites[objno].attribute[0] = attr0;
  sprites[objno].attribute[1] = ATTR1_ROTDATA(0) | ATTR1_SIZE_64 | sx;
}

static inline void HiddenSprite(u32 objno)
{
  sprites[objno].attribute[0] = ATTR0_DISABLED;
}

static inline void SetSprite(u32 objno,u32 Priority,u32 num,u16 alpha)
{
  sprites[objno].attribute[2] = ATTR2_PRIORITY(Priority) | ATTR2_ALPHA(alpha) | num;
}

static void SpriteInit(bool EnabledBlend,u16 alpha)
{
  if(EnabledBlend==false) alpha=15;
  
  initSprites();
  updateOAM(12);
  
  s32 spx[12],spy[12];
  
  spx[ 0]=0; spy[ 0]=0;
  spx[ 1]=1; spy[ 1]=0;
  spx[ 2]=2; spy[ 2]=0;
  spx[ 3]=3; spy[ 3]=0;
  spx[ 4]=0; spy[ 4]=1;
  spx[ 5]=1; spy[ 5]=1;
  spx[ 6]=2; spy[ 6]=1;
  spx[ 7]=3; spy[ 7]=1;
  spx[ 8]=0; spy[ 8]=2;
  spx[ 9]=1; spy[ 9]=2;
  spx[10]=2; spy[10]=2;
  spx[11]=3; spy[11]=2;
  
  for(u32 idx=0;idx<12;idx++){
    SetSprite(idx,1,(8*8)+(8*spx[idx])+((8*8*4)*spy[idx]),alpha);
    
    s32 px=spx[idx]*64;
    s32 py=spy[idx]*64;
    
    SetPosSprite(idx,(s32)px,(s32)py,EnabledBlend);
  }
  
  updateOAM(12);
}

CglScreenMainOverlay::CglScreenMainOverlay(void)
{
  pCanvas=new CglCanvas(&MM_System,&VRAMBuf[16*256],ScreenWidth,ScreenHeight,pf15bit);
  pCanvas->SetColor(RGB15(0,0,0));
  MemSet32CPU(RGB15(0,0,0),(u32*)pCanvas->GetScanLine(0),ScreenWidth*ScreenHeight*2);
  
  SpriteInit(false,15);
}

CglScreenMainOverlay::~CglScreenMainOverlay(void)
{
  delete pCanvas; pCanvas=NULL;
}

u16* CglScreenMainOverlay::GetVRAMBuf(void) const
{
  return(&VRAMBuf[16*256]);
}

void CglScreenMainOverlay::SetPosition_for_Right64x192(s32 x,s32 y) const
{
  bool EnabledBlend=false;
  
  s32 spx[12],spy[12];
  
  spx[ 0]=0; spy[ 0]=0;
  spx[ 1]=1; spy[ 1]=0;
  spx[ 2]=2; spy[ 2]=0;
  spx[ 3]=3; spy[ 3]=0;
  spx[ 4]=0; spy[ 4]=1;
  spx[ 5]=1; spy[ 5]=1;
  spx[ 6]=2; spy[ 6]=1;
  spx[ 7]=3; spy[ 7]=1;
  spx[ 8]=0; spy[ 8]=2;
  spx[ 9]=1; spy[ 9]=2;
  spx[10]=2; spy[10]=2;
  spx[11]=3; spy[11]=2;
  
  for(u32 idx=3;idx<12;idx+=4){
    s32 px=spx[idx]*64;
    s32 py=spy[idx]*64;
    
    SetPosSprite(idx,(s32)px+x,(s32)py+y,EnabledBlend);
  }
  
  updateOAM(12);
}

void CglScreenMainOverlay::SetVisible_for_LeftTop128x64(bool Visible) const
{
  bool EnabledBlend=false;
  
  s32 spx[12],spy[12];
  
  spx[ 0]=0; spy[ 0]=0;
  spx[ 1]=1; spy[ 1]=0;
  spx[ 2]=2; spy[ 2]=0;
  spx[ 3]=3; spy[ 3]=0;
  spx[ 4]=0; spy[ 4]=1;
  spx[ 5]=1; spy[ 5]=1;
  spx[ 6]=2; spy[ 6]=1;
  spx[ 7]=3; spy[ 7]=1;
  spx[ 8]=0; spy[ 8]=2;
  spx[ 9]=1; spy[ 9]=2;
  spx[10]=2; spy[10]=2;
  spx[11]=3; spy[11]=2;
  
  for(u32 idx=0;idx<2;idx++){
    if(Visible==false){
      HiddenSprite(idx);
      }else{
      s32 px=spx[idx]*64;
      s32 py=spy[idx]*64;
      
      SetPosSprite(idx,(s32)px,(s32)py,EnabledBlend);
    }
  }
  
  updateOAM(2);
}

