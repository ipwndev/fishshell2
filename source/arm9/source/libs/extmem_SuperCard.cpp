
#include <nds.h>
#include <_const.h>

// --- SC stuff

#define SSC_Disabled (0)
#define SSC_SDRAM (1)
#define SSC_CF (2)

#define SC_REG_UNLOCK	*(vu16*)(0x09FFFFFE)
DATA_IN_IWRAM_MainPass static void SetSC_UNLOCK(int SSC)
{
  switch(SSC){
    case SSC_Disabled:
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0x0001;
      SC_REG_UNLOCK = 0x0001;
      break;
    case SSC_SDRAM:
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0x0005;
      SC_REG_UNLOCK = 0x0005;
      break;
    case SSC_CF:
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0x0003;
      SC_REG_UNLOCK = 0x0003;
  }
}
#undef SC_REG_UNLOCK

DATA_IN_IWRAM_MainPass u32 extmem_SuperCard_Start(void)
{
  SetSC_UNLOCK(SSC_SDRAM);
  return(0x08000000);
}

