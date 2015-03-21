#include <nds.h>

#include "a7sleep.h"

/*
 #define SPI_DEVICE_POWER  (0 << 8)
 #define SPI_DEVICE_TOUCH  (2 << 8)
 #define SPI_BAUDRATE_2Mhz 1
 #define SPI_BAUDRATE_1Mhz 2
 #define SPI_CONTINUOUS    (1<<11)
 #define PM_AMP_OFFSET     2
 #define PM_AMP_ON         1
 #define PM_AMP_OFF    	  0


 #define	REG_SPICNT	SERIAL_CR
 #define	SPI_BUSY	SERIAL_BUSY
 #define	SPI_ENABLE 	SERIAL_ENABLE
 //#define	SPI_DEVICE_POWER	(0<<8)
 #define	SPI_BAUDRATE_1MHz	SPI_BAUDRATE_1Mhz
 //#define	SPI_CONTINUOUS	(1<11)
 #define	REG_SPIDATA	SERIAL_DATA
 //	#define		
 #define	SWI_WaitByLoop	swiDelay
 */

u8 PM_GetRegister(int reg) {

	//	while(REG_SPICNT & SPI_BUSY)
	//		SWI_WaitByLoop(1);
	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER |SPI_BAUD_1MHz | SPI_CONTINUOUS;
	REG_SPIDATA = reg | 0x80;

	SerialWaitBusy();
	//	while(REG_SPICNT & SPI_BUSY)
	//		SWI_WaitByLoop(1);

	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER |SPI_BAUD_1MHz;
	REG_SPIDATA = 0;

	SerialWaitBusy();
	//	while(REG_SPICNT & SPI_BUSY)
	//		SWI_WaitByLoop(1);


	return REG_SPIDATA & 0xff;
}

void PM_SetRegister(int reg, int control) {

	SerialWaitBusy();
	//	while(REG_SPICNT & SPI_BUSY)
	//		SWI_WaitByLoop(1);

	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER |SPI_BAUD_1MHz | SPI_CONTINUOUS;
	REG_SPIDATA = reg;

	SerialWaitBusy();
	//	while(REG_SPICNT & SPI_BUSY)
	//		SWI_WaitByLoop(1);

	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER |SPI_BAUD_1MHz;
	REG_SPIDATA = control;

}

/*
 void PM_ResetControl(int control)
 {
 control = PM_GetRegister(0) & ~control;
 
 PM_SetRegister(0, control&255);
 }
 */

void PM_SetControl(int control) {
	PM_SetRegister(0, PM_GetRegister(0) | control);
}

/*
 void a7lcdbl(int sw)
 {
 static int lsw=1;
 if(lsw!=sw)
 {
 int control = PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP;
 switch(sw)
 {
 case 0:
 control = PM_GetRegister(0) & ~control;
 PM_SetRegister(0, control&255);

 break;
 default:
 
 PM_SetRegister(0, PM_GetRegister(0) | control);
 }
 lsw=sw;
 }
 }
 */

void a7lcd_select(int control) {
	control |= PM_GetRegister(0) & ~(PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);
	PM_SetRegister(0, control&255);
}

// 0:ON
// 1:OFF(long) and ON(short)
// 2:OFF(short) and ON(short)
// 3:OFF?

// 0:ON
// 1:OFF(long) and ON(short)
// 2:ON
// 3:OFF(short) and ON(short)
//PM_LED_CONTROL(m)
void a7led(int sw) {
	int control= PM_LED_CONTROL(3) | BIT(7);
	int sc = sw << 4;
	control = PM_GetRegister(0) & ~control;
	PM_SetRegister(0, (control|sc)&255);
}

/*
 void a7led_simple(bool onflag)
 {
 u8 control = PM_GetRegister(0) & ~PM_LED_CONTROL(3);
 
 if(onflag==true){
 PM_SetRegister(0, control | PM_LED_CONTROL(0)); // ON
 }else{
 PM_SetRegister(0, control | PM_LED_CONTROL(1)); // OFF(long) and ON(short)
 }
 }
 */

/*
 void a7sleep_dummy(void)
 {
 // Acknowledge interrupts
 REG_IF = REG_IF;
 }
 */

/*
 void a7sleep(void)
 {
 //	∏Ó§Íﬁz§ﬂÕ£÷π •œ•Û•…•Èµ«Âh
 // Set up the interrupt handler
 REG_IME = 0;
 u32 irq_w=(u32)IRQ_HANDLER;
 IRQ_HANDLER = &a7sleep_dummy;
 u32 ie_w=REG_IE;
 REG_IE = (1<<22);				// panel open
 REG_IF = ~0;
 REG_IME = 1;
 //
 // 29,13  1D(OFF) D(ON)

 //		PM_ResetControl(3<<4);
 //	    PM_SetControl(1<<4);
 //	    shared->maxleddelay=60;


 // REG_HALTCNT
 HALT_CR =0xC000; // 14-15    Pause Mode   0..3=Halt, GBA Mode, Stop, Sleep   
 swiSleep();

 //	∏Ó§Íﬁz§ﬂÕ£÷π •œ•Û•…•ÈèÕé¢
 // Set up the interrupt handler
 REG_IME = 0;
 IRQ_HANDLER =(void (*)()) irq_w;
 REG_IE = ie_w;
 REG_IF = ~0;
 REG_IME = 1;
 //
 HALT_CR =0x0000; // 14-15    Pause Mode   0..3=Halt, GBA Mode, Stop, Sleep   
 
 //		PM_ResetControl(3<<4);
 //	    PM_SetControl(2<<4);
 //	    shared->maxleddelay=1;


 }
 */

void a7poff(void) {
	PM_SetControl(1<<6);//6 DS power (0: on, 1: shut down!) 
}

void a7SetSoundAmplifier(bool e) {
	u8 control;
	control = PM_GetRegister(0) & ~PM_SOUND_AMP;
	if (e==true)
		control|=PM_SOUND_AMP;
	PM_SetRegister(0, control&255);
}

