
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "arm9tcm.h"
#include "lang.h"

#include "glib.h"

void msgwin_Clear(void)
{
  CglCanvas *pcan=pScreenMainOverlay->pCanvas;
  
  u32 w=ScreenWidth;
//  if(msgwin_ShowMP3Ctl==true) w-=64;
  u32 th=glCanvasTextHeight+2;
  u32 y0=(ScreenHeight-(th*2))/2;
  
  {
    u32 y=y0-4-1;
    u32 h=th*2+6;
    pcan->SetColor(0);
    pcan->FillFast(0,y,w,1+h+1);
  }
}

void msgwin_Draw(const char *pstr0,const char *pstr1,u32 pos,u32 max)
{
  CglCanvas *pcan=pScreenMainOverlay->pCanvas;
  
  u32 w=ScreenWidth;
//  if(msgwin_ShowMP3Ctl==true) w-=64;
  u32 x0=(w-pcan->GetTextWidthUTF8(pstr0))/2;
  u32 x1=(w-pcan->GetTextWidthUTF8(pstr1))/2;
  u32 th=glCanvasTextHeight+2;
  u32 y0=(ScreenHeight-(th*2))/2;
  u32 y1=y0+th;
  
  {
    u32 y=y0-4-1;
    u32 h=th*2+6;
    pcan->SetColor(RGB15(12,12,12)|BIT15);
    pcan->FillFast(0,y,w,1);
    y+=1;
    pcan->SetColor(RGB15(24,24,24)|BIT15);
    pcan->FillFast(0,y,w,h);
    y+=h;
    pcan->SetColor(RGB15(8,8,8)|BIT15);
    pcan->FillFast(0,y,w,1);
  }
  
  if((pos!=0)&&(max!=0)){
    pos=pos*w/max;
    if(w<=pos) pos=w-1;
    if(pos!=0){
      pcan->SetColor(RGB15(31,31,31)|BIT15);
      pcan->FillBox(0,y1-1,pos,th);
    }
    if(pos!=(w-1)){
      pcan->SetColor(RGB15(20,20,20)|BIT15);
      pcan->FillBox(pos,y1-1,w-pos,th);
    }
  }
  
  pcan->SetFontTextColor(RGB15(31,31,31)|BIT15);
  pcan->TextOutUTF8(x0+1,y0+1,pstr0);
  pcan->TextOutUTF8(x1+1,y1+1+1,pstr1);
  
  pcan->SetFontTextColor(RGB15(0,0,0)|BIT15);
  pcan->TextOutUTF8(x0,y0,pstr0);
  pcan->TextOutUTF8(x1,y1+1,pstr1);
}

