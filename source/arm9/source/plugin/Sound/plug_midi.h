
#ifndef plug_midi_h
#define plug_midi_h

#include <nds.h>

extern bool PlugMIDI_Start(FAT_FILE *pFileHandle);
extern void PlugMIDI_Free(void);
extern u32 PlugMIDI_Update(u32 *plrbuf);

extern s32 PlugMIDI_GetPosMax(void);
extern s32 PlugMIDI_GetPosOffset(void);
extern void PlugMIDI_SetPosOffset(s32 ofs);
extern u32 PlugMIDI_GetChannelCount(void);
extern u32 PlugMIDI_GetSampleRate(void);
extern u32 PlugMIDI_GetSamplePerFrame(void);
extern u32 PlugMIDI_GetPlayTimeSec(void);

extern int PlugMIDI_GetInfoIndexCount(void);
extern bool PlugMIDI_GetInfoStrL(int idx,char *str,int len);
extern bool PlugMIDI_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool PlugMIDI_GetInfoStrUTF8(int idx,char *str,int len);

#endif

