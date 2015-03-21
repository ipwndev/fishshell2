
#define _REG_WAIT_CR (*(vuint16*)0x04000204)

extern bool GBABUS_OwnerARM7;

static inline void SetARM9_REG_WaitCR(void)
{
  u16 bw=_REG_WAIT_CR;
  
  bw&=BIT8 | BIT9 | BIT10 | BIT12 | BIT13;
  
  // mp2 def.0x6800
  // loader def.0x6000
  
  bw|=0 << 0; // 0-1  RAM-region access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=0 << 2; // 2-3  ROM 1st access cycle control   0..3=10,8,6,18 cycles def.0
  bw|=0 << 4; // 4    ROM 2nd access cycle control   0..1=6,4 cycles def.0
  bw|=0 << 5; // 5-6  PHI-terminal output control   0..3=Lowlevel, 4.19MHz, 8.38MHZ, 16.76MHz clock output def.0
  bw|=0 << 7; // 7    Cartridge access right   0=ARM9, 1=ARM7 def.0
  bw|=0 << 11; // 11   Card access right   0=ARM9, 1=ARM7 def.1
  bw|=1 << 14; // 14   Main Memory Interface mode   0=Asychronous (prohibited!), 1=Synchronous def.1
  bw|=1 << 15; // 15   Main Memory priority   0=ARM9 priority, 1=ARM7 priority def.0
  
  if(GBABUS_OwnerARM7==true) bw|=1 << 7; // 7    Cartridge access right   0=ARM9, 1=ARM7 def.0
  
  _REG_WAIT_CR=bw;
}

#undef _REG_WAIT_CR
