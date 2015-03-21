#include <nds.h>

#include "romeo2.h"
#include "rawpcm.h"

#define RAWPCM_MODE_MIXSER   0x00000000
#define RAWPCM_MODE_BYPASS16 0x01000000
#define RAWPCM_MODE_BYPASS32 0x03000000
#define RAWPCM_MODE_ADPCM    0x04000000

void RAWPCM_Start_48kHzBypass(void)
{
  RAWPCM_Stop();
  
  RAWPCM_BYPASS = (RAWPCM_MODE_BYPASS16>>24);
  RAWPCM_RATE   = 0;
}

void RAWPCM_Start_VarFreq(u32 rate)
{
  RAWPCM_Stop();
  
//  u32 freq_rate = (u32)((double)0x10000 * rate / 16756991.0 + 0.5);
  u32 freq_rate = (u32)((0x10000>>4) * rate / (18432690>>4));
  RAWPCM_BYPASS = (RAWPCM_MODE_MIXSER>>24);
  RAWPCM_RATE     = freq_rate;
  
  RAWPCM_VOLUME_L=255;
  RAWPCM_VOLUME_R=255;
}

void RAWPCM_Stop(void)
{
  RAWPCM_VOLUME_L = 0;
  RAWPCM_VOLUME_R = 0;
  
  RAWPCM_BYPASS = 0;
  RAWPCM_RATE   = 0;
  
  s16 *pdst16=(s16*)RAWPCM_RAM;
  for(u32 idx=0;idx<RAWPCM_BUF_SIZE*2*2;idx++){
    *pdst16++ = 0;
  }
}

