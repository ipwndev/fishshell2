
#ifndef _SRTTitle_h
#define _SRTTitle_h

#include "fat2.h"

extern bool PlugSRT_Start(FAT_FILE *FileHandle);
extern void PlugSRT_Free(void);

extern bool PlugSRT_isOpened(void);

extern void PlugSRT_SetOffset(u32 ofs);
extern u32 PlugSRT_GetOffset(void);

extern const char* PlugSRT_GetTitle(void);
extern const char* PlugSRT_GetArtist(void);
extern const char* PlugSRT_GetAlbum(void);
extern const char* PlugSRT_GetAuthor(void);
extern const char* PlugSRT_GetCurSRTTitle(int start_time);
#endif
