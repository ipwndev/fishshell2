
#ifndef _plug_gif_h
#define _plug_gif_h

#include "fat2.h"

extern bool PlugGif_Start(FAT_FILE *_FileHandle);
extern void PlugGif_Free(void);
extern void PlugGif_GetBitmap24(u32 LineY,u8 *pBM);
extern s32 PlugGif_GetWidth(void);
extern s32 PlugGif_GetHeight(void);
extern int PlugGif_GetInfoIndexCount(void);
extern bool PlugGif_GetInfoStrL(int idx,char *str,int len);
extern bool PlugGif_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool PlugGif_GetInfoStrUTF8(int idx,char *str,int len);

#endif
