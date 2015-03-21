#ifndef _ROMEO2_H_
#define _ROMEO2_H_

/////////////////////////////////////////////////////////////////////////////
// GBA 16bit bus memory map
/////////////////////////////////////////////////////////////////////////////

// controll ports

#define RAWPCM_RAM      ( (vu16 *)0x08800000)
#define ADPCM_DEC_VAL   (*(vu16 *)0x08800000)

#define HP_DETECT       (*(vu16 *)0x08001000)
#define HP_DETECT_HDSET 0x0001
#define HP_DETECT_STDET 0x0002
#define HP_DETECT_HSSW  0x0004

#define PCM1770_SPI     (*(vu16 *)0x08002000)
#define PCM1770_SPI_STS (*(vu16 *)0x08002000)
#define PCM1770_SPI_STS_BUSY 0x0001
#define DAC_IIS_ENABLE  (*(vu16 *)0x08004000)

#define MASTER_VOLUME_L  (*(vu16 *)0x08008000)
#define MASTER_VOLUME_R  (*(vu16 *)0x08008002)
#define FBPCM_POS        (*(vu16 *)0x08008006)

#define ADPCM_VOLUME_L   (*(vu16 *)0x08008010)
#define ADPCM_VOLUME_R   (*(vu16 *)0x08008012)
#define ADPCM_RATE   R   (*(vu16 *)0x08008012)

#define ADPCM_LPF        (*(vu16 *)0x0800C01E)

#define FM_VOLUME_L      (*(vu16 *)0x08008020)
#define FM_VOLUME_R      (*(vu16 *)0x08008022)

#define SSG_VOLUME_L     (*(vu16 *)0x08008030)
#define SSG_VOLUME_R     (*(vu16 *)0x08008032)

#define RAWPCM_VOLUME_L  (*(vu16 *)0x08008040)
#define RAWPCM_VOLUME_R  (*(vu16 *)0x08008042)
#define RAWPCM_RATE      (*(vu16 *)0x08008044)
#define RAWPCM_POS       (*(vu16 *)0x08008046)
#define RAWPCM_BYPASS    (*(vu16 *)0x08008048)

#define ADPCM_DEC_DATA (*(vu16 *)0x08800000)

// sound generator

// buffer RAM 
#define SND_BUF_L_TOP  0x09000000
#define SND_BUF_R_TOP  0x09800000

#define FB_RAM_L   ((vu16 *)(SND_BUF_L_TOP))
#define FB_RAM_R   ((vu16 *)(SND_BUF_R_TOP))
#define FB_RAM_L32 ((vu32 *)(SND_BUF_L_TOP))
#define FB_RAM_R32 ((vu32 *)(SND_BUF_R_TOP))

/////////////////////////////////////////////////////////////////////////////
// GBA 8bit bus memory map
/////////////////////////////////////////////////////////////////////////////

// user MAPPED
#define LED_PWM0   (*(vu8 *)0x0a000000)
#define LED_PWM1   (*(vu8 *)0x0a000001)

#define RESET_PORT (*(vu8 *)0x0a000040)

#define RESET_PORT (*(vu8 *)0x0a000040)
#define OPM_A_PORT (*(vu8 *)0x0a000080)
#define OPM_D_PORT (*(vu8 *)0x0a000081)
#define SSG_A_PORT (*(vu8 *)0x0a000082)
#define SSG_D_PORT (*(vu8 *)0x0a000083)
#define ADPCM_PORT (*(vu8 *)0x0a000090)
//#define SBUF_POINT (*(vu8 *)0x0a0000c0)
// #define SBUF_POINT FBPCM_POS

/////////////////////////////////////////////////////////////////////////////
// PCM buffer Size
/////////////////////////////////////////////////////////////////////////////

// Feedback PCM Buffer Size
#define FB_BUF_SIZE    1024
#define FB_BUF_MASK    (FB_BUF_SIZE-1)
// legacy name
// #define SND_BUF_MASK   FB_BUF_MASK

// RAW PCM buffer size
#define RAWPCM_BUF_SIZE 2048

#endif
