
// fastlzss16decpalasm.cpp last update 2007/06/22 by Moonlight.

// 書き込みバッファ終端以降に最大126byteの無効な書き込みを行うので、_pdstは+126byte余分に確保しておくこと。

#ifndef _fastlzss16decpalasm_h
#define _fastlzss16decpalasm_h

#include <nds.h>

#define fastlzss16decpalasm_GUARDSIZE (((31*2)+1)*2)

extern void fastlzss16decpalasm_init(void);
extern u32 fastlzss16decpalasm_getdecsize(const void *_psrc);
extern void fastlzss16decpalasm_decode(const void *_psrc,void *_pdst,u32 *ppal32);

#endif

