/*
	PCM1770 Driver

	080804 added BUSY flag handling
*/

#include <nds.h>

#include "romeo2.h"

#include "pcm1770.h"

/****************************************************************************
	DAC SPI command
****************************************************************************/
void dac_spi(int reg,int val)
{
#if 0
	int i;
	u16 dummy;

	PCM1770_SPI = (reg<<8) | val;

	// wait 32fs x 18
//	for(i=0;i<64;i++)
	for(i=0;i<256;i++)
		dummy = PCM1770_SPI;
	
	return;
#endif
	
	// wait for SPI port busy
	while(PCM1770_SPI_STS & PCM1770_SPI_STS_BUSY);

	// SPI output
	PCM1770_SPI = (reg<<8) | val;
}

/****************************************************************************
	DAC volume set
****************************************************************************/
static bool VolumeMute=false;
static int VolumeL,VolumeR;

void pcm1770_set_volumemute(bool mute)
{
  VolumeMute=mute;
  pcm1770_set_volume(VolumeL,VolumeR);
}

void pcm1770_set_volume(int lvol,int rvol)
{
  if(VolumeMute==true){
    for(u32 idx=0;idx<8;idx++){
      dac_spi(1,0 & 0x3f);
      dac_spi(2,0 & 0x3f);
    }
    return;
  }
  
  VolumeL=lvol;
  VolumeR=rvol;
  
  for(u32 idx=0;idx<8;idx++){
    dac_spi(1,lvol & 0x3f);
    dac_spi(2,rvol & 0x3f);
  }
}

/****************************************************************************
	DAC setup
****************************************************************************/
void pcm1770_setup(void)
{
	// volume mute
	dac_spi(1,0);
	dac_spi(2,0);

	// OVER,RINV,AMIX,DEM disabled , FMT=000 24bit left-justifield
	dac_spi(3,0);

	// ZCAT disable , PWRD normal
	dac_spi(4,0);

//	pcm1770_set_volume(63,63);
	pcm1770_set_volume(63-6,63-6);
    pcm1770_set_volumemute(false);
}
