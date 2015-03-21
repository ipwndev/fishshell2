
#ifndef plug_gme_h
#define plug_gme_h

#define LIBGMETitle "Game_Music_Emu 0.5.2 by Blargg"

#include "_const.h"

extern bool PlugGME_Start(const char *pEXT,FAT_FILE *pFileHandle,int TrackNum);
extern void PlugGME_Free(void);
extern u32 PlugGME_Update(u32 *plrdst);

extern s32 PlugGME_GetPosMax(void);
extern s32 PlugGME_GetPosOffset(void);
extern void PlugGME_SetPosOffset(s32 ofs);

extern u32 PlugGME_GetSampleRate(void);
extern u32 PlugGME_GetChannelCount(void);
extern u32 PlugGME_GetSamplePerFrame(void);
extern u32 PlugGME_GetPlayTimeSec(void);

#include "unicode.h"

extern int PlugGME_GetInfoIndexCount(void);
extern bool PlugGME_GetInfoStrL(int idx,char *str,int len);
extern bool PlugGME_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool PlugGME_GetInfoStrUTF8(int idx,char *str,int len);

#endif

