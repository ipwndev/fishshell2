
#ifndef smidlib_h
#define smidlib_h

#include "smidlib_sm.h"

extern void smidlibSetParam(u8 *data,u32 SampleRate,u32 GenVolume);
extern bool smidlibStart(void);
extern void smidlibFree(void);
extern int smidlibGetNearClock(void);
extern bool smidlibNextClock(bool ShowEventMessage,bool EnableNote,int DecClock);
extern void smidlibAllSoundOff(void);

#endif

