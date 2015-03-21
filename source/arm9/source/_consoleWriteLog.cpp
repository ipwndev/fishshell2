
#include <nds.h>

#include "_console.h"
#include "_consoleWriteLog.h"

void PrfStart(void)
{
  TIMER0_CR=0;
  TIMER0_DATA=0;
  TIMER0_CR=TIMER_ENABLE | TIMER_DIV_1;
  TIMER1_CR=0;
  TIMER1_DATA=0;
  TIMER1_CR=TIMER_ENABLE | TIMER_CASCADE;
}

void PrfEnd(int data)
{
  vu32 t0d=TIMER0_DATA;
  vu32 t1d=TIMER1_DATA;
  
  u32 us=(t1d<<16) | t0d;
  
  float fus=us/((float)33513982/1000/1000);
  
  _consolePrintf("prf data=%d %6dus\n",data,(u32)fus);
}

u32 PrfEnd_GetCPUCount(void)
{
  vu32 t0d=TIMER0_DATA;
  vu32 t1d=TIMER1_DATA;
  
  return((t1d<<16) | t0d);
}

u32 PrfEnd_Getus(void)
{
  vu32 t0d=TIMER0_DATA;
  vu32 t1d=TIMER1_DATA;
  
  u32 us=(t1d<<16) | t0d;
  
  float fus=us/((float)33513982/1000/1000);
  return(fus);
}

