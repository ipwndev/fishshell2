
#include <nds.h>
#include <_const.h>

// --- DS Browser Memory stuff

#define DSBM_Header ((vu16*)(0x080000B0))
#define DSBM_HeaderSize	(16)
#define DSBM_REG_UNLOCK	(*(vu16*)(0x8240000))

DATA_IN_IWRAM_MainPass static inline bool ExistsDSBM(void)
{
  u16 *pHeaderData;
  {
    static const u8 Data[DSBM_HeaderSize]={0xFF,0xFF,0x00,0x00,0x00,0x24,0x24,0x24,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F};
    pHeaderData=(u16*)&Data[0];
  }
  
  for(u32 idx=0;idx<DSBM_HeaderSize/2;idx++){
    if(DSBM_Header[idx]!=pHeaderData[idx]) return(false);
  }
  
  return(true);
}

DATA_IN_IWRAM_MainPass static inline void SetDSBM_MemoryOpen(void)
{
  DSBM_REG_UNLOCK=0x0001;
}

DATA_IN_IWRAM_MainPass static inline void SetDSBM_MemoryClose(void)
{
  DSBM_REG_UNLOCK=0x0000;
}

DATA_IN_IWRAM_MainPass u32 extmem_DSBM_Start(void)
{
  if(ExistsDSBM()==false) return(0);
  
  SetDSBM_MemoryOpen();
  return(0x9000000);
}
