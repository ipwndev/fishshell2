
#ifndef _LYRIC_h
#define _LYRIC_h

#include "fat2.h"

#define LRCTitle "liblyric audio plugin by Carpfish"

extern bool PlugLRC_Start(const char *pFileName);
extern void PlugLRC_Free(void);

extern bool PlugLRC_isOpened(void);

extern void PlugLRC_SetOffset(s32 ofs);
extern s32 PlugLRC_GetOffset(void);

extern const char* PlugLRC_GetTitle(void);
extern const char* PlugLRC_GetArtist(void);
extern const char* PlugLRC_GetAlbum(void);
extern const char* PlugLRC_GetAuthor(void);
extern const char* PlugLRC_GetPrevLyric(void);
extern const char* PlugLRC_GetCurLyric(int start_time);
extern const char* PlugLRC_GetNextLyric(void);

#define isblank(a) ((a) == ' ')
#endif
