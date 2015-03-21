
#ifndef _OGG_h
#define _OGG_h

#include "fat2.h"

#define OGGTitle "libogg Tremor 1.0 (c)2002 Xiph.org"

extern bool PlugOGG_Start(FAT_FILE *FileHandle);
extern u32 PlugOGG_Update(u32 *plrbuf);
extern void PlugOGG_Free(void);

extern s32 PlugOGG_GetPosMax(void);
extern s32 PlugOGG_GetPosOffset(void);
extern void PlugOGG_SetPosOffset(s32 ofs);

extern u32 PlugOGG_GetChannelCount(void);
extern u32 PlugOGG_GetSampleRate(void);
extern u32 PlugOGG_GetSamplePerFrame(void);
extern u32 PlugOGG_GetPlayTimeSec(void);

extern int PlugOGG_GetInfoIndexCount(void);
extern bool PlugOGG_GetInfoStrL(int idx,char *str,int len);
extern bool PlugOGG_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool PlugOGG_GetInfoStrUTF8(int idx,char *str,int len);

#endif
