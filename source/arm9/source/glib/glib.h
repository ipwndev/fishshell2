
#ifndef glib_h
#define glib_h

#include <nds.h>
#include "_const.h"
#include "arm9tcm.h"

#define GLIBID "glib ver 0.1 by Moonlight."

#ifndef ScreenWidth
#define ScreenWidth (256)
#endif

#ifndef ScreenHeight
#define ScreenHeight (192)
#endif

#ifndef BIT15
#define BIT15 (1<<15)
#endif

#include "glglobal.h"

#include "cglscreen.h"
#include "cglcanvas.h"
#include "cglfont.h"
#include "cglb15.h"
#include "cgltgf.h"

static inline u16 ColorMargeAlpha(const u16 col1,const u16 col2,const int Alpha)
{
  u32 Alpha1=32-Alpha;
  u32 Alpha2=Alpha;
  u32 r,g,b;
  
  r=((col1>>0)&0x1f)*Alpha1/32;
  r+=((col2>>0)&0x1f)*Alpha2/32;
  g=((col1>>5)&0x1f)*Alpha1/32;
  g+=((col2>>5)&0x1f)*Alpha2/32;
  b=((col1>>10)&0x1f)*Alpha1/32;
  b+=((col2>>10)&0x1f)*Alpha2/32;
  
  return(RGB15(r,g,b)|BIT15);
}

static inline u16 ColorMargeAlphaAdd(const u16 col1,const u16 col2,const int Alpha)
{
  u32 Alpha1=32-Alpha;
  u32 r,g,b;
  
  r=((col1>>0)&0x1f)*Alpha1/32;
  r+=((col2>>0)&0x1f);
  g=((col1>>5)&0x1f)*Alpha1/32;
  g+=((col2>>5)&0x1f);
  b=((col1>>10)&0x1f)*Alpha1/32;
  b+=((col2>>10)&0x1f);
  
  return(RGB15(r,g,b)|BIT15);
}

#endif

