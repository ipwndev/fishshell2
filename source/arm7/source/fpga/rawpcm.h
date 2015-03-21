#ifndef _RAWPCM_H_
#define _RAWPCM_H_

extern void RAWPCM_Start_48kHzBypass(void);
extern void RAWPCM_Start_VarFreq(u32 rate);
extern void RAWPCM_Stop(void);

#endif
