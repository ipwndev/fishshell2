
#ifndef rect_h
#define rect_h

#include <nds.h>

typedef struct {
  s32 x,y,w,h;
} TRect;

typedef struct {
  s32 x,y;
} TPoint;

extern u32 GetDirtyRect_OutSide(TRect TagRect,TRect DelRect,TRect **ppRects);
extern bool isInsideRect(TRect Rect,s32 x,s32 y);

static inline TRect CreateRect(s32 x,s32 y,s32 w,s32 h)
{
  TRect r;
  
  r.x=x;
  r.y=y;
  r.w=w;
  r.h=h;
  
  return(r);
}

static inline TPoint CreatePoint(s32 x,s32 y)
{
  TPoint p;
  
  p.x=x;
  p.y=y;
  
  return(p);
}

#endif

