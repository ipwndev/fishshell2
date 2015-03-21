#ifndef a7sleep_h
#define a7sleep_h

#include <nds.h>

u8 PM_GetRegister(int reg);
void PM_SetRegister(int reg, int control);

//void a7sleep_dummy(void);
//void a7sleep(void);
//void a7lcdbl(int sw);
void a7lcd_select(int control);
void a7led(int sw);
//void a7led_simple(bool onflag);
void a7poff(void);

void a7SetSoundAmplifier(bool e);

/* ---------------------------------------------------------
 arm7/main.cpp add

 #include "a7sleep.h"

 //	XKEYS¤òIPC¤Ë•ø¤­Þz¤ó¤Ç¤¤¤ë¤¢¤¿¤ê¤Ë¤Ç¤âÈë¤ì¤ë

 u32 xkeys=XKEYS;
 IPC->buttons = xkeys;

 if(xkeys == 0x00FF)	//	¥Ñ¥Í¥ë¥¯¥í©`¥º
 {
 //	¥Ñ¥Í¥ë¥¯¥í©`¥º×´‘B¤Çºô¤ÖÊÂ
 //	¥Ñ¥Í¥ë¥ª©`¥×¥ó¤ÇÍŽ¢¤·¤Þ¤¹
 a7sleep();
 }



 //	LCD¥Ð¥Ã¥¯¥é¥¤¥ÈOFF/ONÖÆÓù

 u32 xkeys=XKEYS;
 IPC->buttons = xkeys;

 if(xkeys == 0x00FF)	//	¥Ñ¥Í¥ë¥¯¥í©`¥º
 {
 a7lcdbl(0);
 }
 else
 {
 a7lcdbl(1);
 }


 -----------------------------------------------------------*/

#endif
