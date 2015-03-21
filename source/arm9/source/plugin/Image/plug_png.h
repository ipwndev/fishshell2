
#ifndef _plug_png_h
#define _plug_png_h

#include "fat2.h"

extern bool PlugPng_Start(FAT_FILE *_FileHandle);
extern void PlugPng_Free(void);
extern void PlugPng_GetBitmap24(u32 LineY,u8 *pBM);
extern s32 PlugPng_GetWidth(void);
extern s32 PlugPng_GetHeight(void);
extern int PlugPng_GetInfoIndexCount(void);
extern bool PlugPng_GetInfoStrL(int idx,char *str,int len);
extern bool PlugPng_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool PlugPng_GetInfoStrUTF8(int idx,char *str,int len);

#endif
