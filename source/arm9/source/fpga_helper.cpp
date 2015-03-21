/*
	DS-FPGA support function
*/

#include <nds.h>

#include "_console.h"
#include "_const.h"

#include "fpga_helper.h"

/****************************************************************************
	Bus Init
****************************************************************************/

int ds_fpga_bus_init(void)
{
	int i;

/*
REG_EXMEMCNT register
 15:main ram high priority ,0=ARM9 , 1=ARM7
 14:syncronus mode
 11:Slot1 master , 0=ARM9 , 1=ARM7
  7:SLOT2 master , 0=ARM9 , 1=ARM7
6-5:SLOT2 PHI , 0=LOW,01=4.19MHz,10=8.38MHz,11=16.76MHz
  4:SLOT2 ROM 2nd cycle ,0=6cycle,1=4cycle
3-2:SLOT2 ROM 1st cycle ,00=10cycle,01=8cycle,10=6cycle,11=18cycle
1-0:SLOT2 RAM     cycle ,00=10cycle,01=8cycle,10=6cycle,11=18cycle

REG_EXMEMCNT = 0xC8ba;

*/
//	sysSetCartOwner(1); // SLOT2 master = ARM9
//	REG_EXMEMCNT |=  0x8000; // MainRAM priority high = ARM7
//	REG_EXMEMCNT  = (REG_EXMEMCNT & ~0x001f) | 0x001a; // ROM cycle=4,6 , RAM cycle = 6
//	REG_EXMEMCNT |=  0x0060; // PHI = 16.76MHz output

//	iprintf("REG_EXMEMCNT = %04X\n",REG_EXMEMCNT);
//	while(1);

	// sense FPGA board
#if DEBUG_MSG
	iprintf("SENSE:%02X:%02X:%02X:%02X\n",
		(FPGA_CFG_STS[0<<8]>>2)&3,
		(FPGA_CFG_STS[1<<8]>>2)&3,
		(FPGA_CFG_STS[2<<8]>>2)&3,
		(FPGA_CFG_STS[3<<8]>>2)&3);
#endif
	for(i=0;i<0x1000;i++)
	{
		if( GET_FPGA_SENSE(i)==0)
		{
#if DEBUG_MSG
			iprintf("ROMEO2 can't found\n");
#endif
			return -1;
		}
	}
	return 0;
}

/****************************************************************************
	Configration
****************************************************************************/
#define TIMEOUT_LOOP 1000000

#define DEBUG_MSG 1

static int ds_fpga_configration(const u8 *config_data,int config_data_size)
{
#if REV2
	int i;

	// IRQ disable ,Can't access 8bit bus during configration
	REG_IME = IME_DISABLE;

#if DEBUG_MSG
	_consolePrint("Pulse PROG_B\n");
#endif
	// set long buscycle to pulse 450ns PROG_B drive.
//	REG_EXMEMCNT  = (REG_EXMEMCNT & ~0x001f) | 0x001b; // ROM cycle=4,6 , RAM cycle = 18
	// pulse PROG_B pin
	vu16 dummy   = *FPGA_CFG_PGM;
	// restore 8bit bus access cycle
//	REG_EXMEMCNT  = (REG_EXMEMCNT & ~0x001f) | 0x001a; // ROM cycle=4,6 , RAM cycle = 6

#if DEBUG_MSG
	_consolePrint("Wait for DONE=L\n");
#endif
	for(i=TIMEOUT_LOOP;GET_FPGA_DONE()==1;i--)
	{
		if(i==0)
		{
#if DEBUG_MSG
			_consolePrint("error , DONE != L\n");
#endif
			REG_IME = IME_ENABLE;
			return FPGA_ERR_NOT_DONE_L;
		}
	}

#if DEBUG_MSG
	_consolePrint("Wait for INIT=H\n");
#endif
	for(i=TIMEOUT_LOOP;GET_FPGA_INIT()==0;i--)
	{
		if(i==0)
		{
#if DEBUG_MSG
			_consolePrint("error , INIT != H\n");
#endif

			REG_IME = IME_ENABLE;
			return FPGA_ERR_NOT_INIT_H;
		}
	}

#if DEBUG_MSG
	_consolePrintf("Transmit ConfigData %d bytes\n",config_data_size);
#endif
	for(i=0;i<config_data_size;i++)
	{
		// swap bit oeder
		unsigned char rdata = config_data[i];
		unsigned char wdata = 
			((rdata>>7)&0x01)|
			((rdata>>5)&0x02)|
			((rdata>>3)&0x04)|
			((rdata>>1)&0x08)|
			((rdata<<1)&0x10)|
			((rdata<<3)&0x20)|
			((rdata<<5)&0x40)|
			((rdata<<7)&0x80);
#if 0
		// cause CRC error
		if(i==0x400) wdata ^= 0xaa;
#endif
		*FPGA_CFG_DATA = wdata;

#if 0
		if( GET_FPGA_INIT()==0)
		{
#if DEBUG_MSG
			_consolePrint("Configration Data Error\n");
#endif
			REG_IME = IME_ENABLE;
			return FPGA_ERR_DATA_CRC;
		}
#endif

	}

#if DEBUG_MSG
	_consolePrint("Wait for DONE=H\n");
#endif

	// Check DONE pins
	for(i=TIMEOUT_LOOP;GET_FPGA_DONE()==0;i--)
	{
		if(i==0)
		{
#if DEBUG_MSG
			_consolePrint("error , DONE != H\n");
#endif
			REG_IME = IME_ENABLE;
			return FPGA_ERR_NOT_DONE_H;
		}
	}

	_consolePrint("Configration Done\n");
	REG_IME = IME_ENABLE;

	return FPGA_NO_ERR;

#else // #if REV2

_consolePrint("Supported REV2 only. halt.\n");
	while(1);
#endif

}

// ------------------ helper tools.

#define _REG_WAIT_CR (*(vuint16*)0x04000204)

void FPGA_BusOwnerARM7(void)
{
  u16 bw=_REG_WAIT_CR;
  
  bw&=BIT8 | BIT9 | BIT10 | BIT12 | BIT13;
  
  // mp2 def.0x6800
  // loader def.0x6000
  
  // ROM cycle=4,6 , RAM cycle = 6 , PHI = 16.76MHz output , CAR = ARM7
  
  bw|=0 << 0; // 0-1  RAM-region access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=0 << 2; // 2-3  ROM 1st access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=0 << 4; // 4    ROM 2nd access cycle control   0..1=6,4 cycles def.0
  bw|=0 << 5; // 5-6  PHI-terminal output control   0..3=Lowlevel, 4.19MHz, 8.38MHZ, 16.76MHz clock output def.0
  bw|=1 << 7; // 7    Cartridge access right   0=ARM9, 1=ARM7 def.0
  bw|=0 << 11; // 11   Card access right   0=ARM9, 1=ARM7 def.1
  bw|=1 << 14; // 14   Main Memory Interface mode   0=Asychronous (prohibited!), 1=Synchronous def.1
  bw|=1 << 15; // 15   Main Memory priority   0=ARM9 priority, 1=ARM7 priority def.0
  
  _REG_WAIT_CR=bw;
}

void FPGA_BusOwnerARM9(void)
{
  u16 bw=_REG_WAIT_CR;
  
  bw&=BIT8 | BIT9 | BIT10 | BIT12 | BIT13;
  
  // mp2 def.0x6800
  // loader def.0x6000
  
  // ROM cycle=4,6 , RAM cycle = 6 , PHI = 16.76MHz output , CAR = ARM9
  
  bw|=0 << 0; // 0-1  RAM-region access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=0 << 2; // 2-3  ROM 1st access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=0 << 4; // 4    ROM 2nd access cycle control   0..1=6,4 cycles def.0
  bw|=0 << 5; // 5-6  PHI-terminal output control   0..3=Lowlevel, 4.19MHz, 8.38MHZ, 16.76MHz clock output def.0
  bw|=0 << 7; // 7    Cartridge access right   0=ARM9, 1=ARM7 def.0
  bw|=0 << 11; // 11   Card access right   0=ARM9, 1=ARM7 def.1
  bw|=1 << 14; // 14   Main Memory Interface mode   0=Asychronous (prohibited!), 1=Synchronous def.1
  bw|=1 << 15; // 15   Main Memory priority   0=ARM9 priority, 1=ARM7 priority def.0
  
  _REG_WAIT_CR=bw;
}

bool FPGA_isExistCartridge(void)
{
  if(ds_fpga_bus_init()==0) return(true);
  return(false);
}

bool FPGA_CheckBitStreamFormat(void *pbuf,u32 size)
{
  u8 *p=(u8*)pbuf;
  
  if(p[0]!=0x00) return(false);
  if(p[1]!=0x09) return(false);
  if(p[2]!=0x0f) return(false);
  if(p[3]!=0xf0) return(false);
  if(p[4]!=0x0f) return(false);
  if(p[5]!=0xf0) return(false);
  if(p[6]!=0x0f) return(false);
  if(p[7]!=0xf0) return(false);
  
  return(true);
}

bool FPGA_Start(void *pbuf,u32 size)
{
	_consolePrint("NDS FPGA MDX play tester\n");
	
	ds_fpga_bus_init();

	// FPGAをコンフィグレーションする
	_consolePrint("FPGA CONFIG\n");
	
	if(ds_fpga_configration((u8*)pbuf,size)!=FPGA_NO_ERR) return(false);
	
	_consolePrintf("INIT=%d, DONE=%d\n",GET_FPGA_INIT(),GET_FPGA_DONE());
	
	// FPGA RING BUFの再生を開始する
	if(GET_FPGA_DONE()==false) return(false);
  
	return(true);
}

void FPGA_End(void)
{
  u16 bw=_REG_WAIT_CR;
  
  bw&=BIT8 | BIT9 | BIT10 | BIT12 | BIT13;
  
  // mp2 def.0x6800
  // loader def.0x6000
  
  // ROM cycle=4,6 , RAM cycle = 6 , PHI = 16.76MHz output , CAR = ARM7
  
  bw|=0 << 0; // 0-1  RAM-region access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=0 << 2; // 2-3  ROM 1st access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=0 << 4; // 4    ROM 2nd access cycle control   0..1=6,4 cycles def.0
  bw|=0 << 5; // 5-6  PHI-terminal output control   0..3=Lowlevel, 4.19MHz, 8.38MHZ, 16.76MHz clock output def.0
  bw|=0 << 7; // 7    Cartridge access right   0=ARM9, 1=ARM7 def.0
  bw|=0 << 11; // 11   Card access right   0=ARM9, 1=ARM7 def.1
  bw|=1 << 14; // 14   Main Memory Interface mode   0=Asychronous (prohibited!), 1=Synchronous def.1
  bw|=1 << 15; // 15   Main Memory priority   0=ARM9 priority, 1=ARM7 priority def.0
  
  _REG_WAIT_CR=bw;
}

#undef _REG_WAIT_CR

