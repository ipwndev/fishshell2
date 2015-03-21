
#include <nds.h>

#include "_console.h"
#include "_const.h"

#include "rect.h"

#define RectMax (4)
DATA_IN_AfterSystem static u32 RectCount;
DATA_IN_AfterSystem static TRect Rects[RectMax];

static void AddRect(TRect r)
{
  if(RectCount==RectMax) StopFatalError(19001,"Rect buffer overflow.\n");
  
  if((r.w<=0)||(r.h<=0)) return;
  
  Rects[RectCount]=r;
  RectCount++;
}

u32 GetDirtyRect_OutSide(TRect Rect,TRect DelRect,TRect **ppRects)
{
  // RectとDelRectのサイズ(w,h)は同じじゃなきゃダメ
  
  RectCount=0;
  *ppRects=&Rects[0];
  
  if((Rect.x==DelRect.x)&&(Rect.y==DelRect.y)) return(RectCount);
  
  if(((Rect.x+Rect.w)<DelRect.x)||((DelRect.x+DelRect.w)<Rect.x)||
     ((Rect.y+Rect.h)<DelRect.y)||((DelRect.y+DelRect.h)<Rect.y)){
    AddRect(Rect);
    return(RectCount);
  }
  
  if(Rect.y!=DelRect.y){
    TRect r=Rect;
    
    if(Rect.y<DelRect.y){
      r.h=DelRect.y-Rect.y;
      }else{
      r.y=DelRect.y+DelRect.h;
      r.h=Rect.y-DelRect.y;
    }
    
    AddRect(r);
  }
  
  if(Rect.x!=DelRect.x){
    TRect r=Rect;
    
    if(Rect.x<DelRect.x){
      r.w=DelRect.x-Rect.x;
      }else{
      r.x=DelRect.x+DelRect.w;
      r.w=Rect.x-DelRect.x;
    }
    
    if(Rect.y!=DelRect.y){
      if(Rect.y<DelRect.y){
        r.y=DelRect.y;
        r.h=Rect.h-(DelRect.y-Rect.y);
        }else{
        r.h=Rect.h-(Rect.y-DelRect.y);
      }
    }
    
    AddRect(r);
  }
  
  return(RectCount);
}

bool isInsideRect(TRect Rect,s32 x,s32 y)
{
  if((x<Rect.x)||((Rect.x+Rect.w)<=x)) return(false);
  if((y<Rect.y)||((Rect.y+Rect.h)<=y)) return(false);
  return(true);
}

