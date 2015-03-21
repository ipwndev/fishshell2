
#ifndef _MP2_h
#define _MP2_h

#define MP2Title "libmad - MPEG audio decoder library"
	
#ifdef __cplusplus
extern "C" {
#endif
	
void MP2_SetFunc_consolePrintf(u32 adr);

bool StartMP2(void);
void FlashFileBufferMP2(void);
u32 UpdateMP2(s16 *lbuf,s16 *rbuf);
void FreeMP2(void);

u32 MP2_GetChannelCount(void);
u32 MP2_GetSampleRate(void);
u32 MP2_GetSamplePerFrame(void);

u32 MP2_GetBitRate(void);
	
#ifdef __cplusplus
}
#endif

#endif
