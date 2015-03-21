
// fastlzss16decpalasm.cpp last update 2007/06/22 by Moonlight.

#include <nds.h>

#include "fastlzss16decpalasm.h"
#include "_console.h"
#include "arm9tcm.h"

u32 fastlzss16decpalasm_getdecsize(const void *_psrc)
{
  u32 gbaheader=*(u32*)_psrc;
  
  if(((gbaheader&0xf0)>>4)!=15){ // unknown file format.
    return(0);
  }
  
  return(gbaheader>>8);
}

extern "C" {
extern void fastlzss16decpalasm_body(const u8 *psrcbuf, u16 *pdstbuf, u16 *pdstterm, const u32 *ppal32);
}

void fastlzss16decpalasm_decode(const void *_psrc,void *_pdst,u32 *ppal32)
{
  const u8 *psrcbuf=(u8*)_psrc;
  u16 *pdstbuf=(u16*)_pdst;
  
  // discard gba header info
  u32 gbaheader=*(u32*)psrcbuf;
  psrcbuf+=4;
  
  if(((gbaheader&0xf0)>>4)!=15){ // unknown file format.
    return;
  }
  
  u32 decomp_size=gbaheader>>8;
  u16 *pdstterm=&pdstbuf[decomp_size];
  
  fastlzss16decpalasm_body(psrcbuf, pdstbuf, pdstterm, ppal32);
}

void fastlzss16decpalasm_init(void)
{
}

