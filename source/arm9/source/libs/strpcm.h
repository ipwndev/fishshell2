
#ifndef strpcm_h
#define strpcm_h

#include <nds.h>
#include "../../ipc6.h"

#define NULLSTR64bit ((u64)0x005254534c4c554e)

#define strpcmRingBufCount (16)
#define strpcmRingBufBitMask (strpcmRingBufCount-1)

extern vu64 DPGAudioStream_SyncSamples;
extern u32 DPGAudioStream_PregapSamples;

extern volatile bool VBlankPassedFlag;
extern volatile u32 VBlankPassedCount;

extern volatile bool strpcmRequestStop;

extern volatile bool strpcmRingEmptyFlag;
extern volatile u32 strpcmRingBufReadIndex;
extern volatile u32 strpcmRingBufWriteIndex;

extern u32 *strpcmRingLRBuf;

extern bool strpcm_UseLoopOnOverflow;

extern void InterruptHandler(void);
extern void InitInterrupts(void);

extern void strpcmStart(bool FastStart,u32 SampleRate,u32 SamplePerBuf,u32 ChannelCount,EstrpcmFormat strpcmFormat);
extern void strpcmStop(void);

extern void strpcmClearRingBuffer(void);

#define strpcmVolumeMax (96)

extern void strpcmSetAudioVolume64(int v);
extern int strpcmGetAudioVolume64(void);

extern void strpcmSetVideoVolume64(int v);
extern int strpcmGetVideoVolume64(void);

extern void VBlank_AutoFlip_Enabled(void);
extern void VBlank_AutoFlip_Disabled(void);

extern bool strpcm_ExclusivePause;

extern void IlligalWritedNullPointer_Check(u32 state);

#endif
