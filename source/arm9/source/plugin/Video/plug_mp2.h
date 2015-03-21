
#ifndef _MP2_h
#define _MP2_h

#define MP2Title "MP2 decode from ARM7"

#include "cstream.h"

extern bool StartMP2(void);
extern void FreeMP2(void);

extern void MP2_SetPosition(double per,u64 smp);

extern void MP2_LoadReadBuffer(void);

#endif
