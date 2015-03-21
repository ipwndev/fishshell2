
#ifndef _WAV_h
#define _WAV_h

#include "fat2.h"

extern bool PlugWAVE_Start(FAT_FILE *FileHandle);
extern void PlugWAVE_Free(void);
extern u32 PlugWAVE_Update(u32 *plrbuf);

extern s32 PlugWAVE_GetPosMax(void);
extern s32 PlugWAVE_GetPosOffset(void);
extern void PlugWAVE_SetPosOffset(s32 ofs);

extern u32 PlugWAVE_GetChannelCount(void);
extern u32 PlugWAVE_GetSampleRate(void);
extern u32 PlugWAVE_GetSamplePerFrame(void);
extern u32 PlugWAVE_GetPlayTimeSec(void);

extern int PlugWAVE_GetInfoIndexCount(void);
extern bool PlugWAVE_GetInfoStrL(int idx,char *str,int len);
extern bool PlugWAVE_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool PlugWAVE_GetInfoStrUTF8(int idx,char *str,int len);

#endif
