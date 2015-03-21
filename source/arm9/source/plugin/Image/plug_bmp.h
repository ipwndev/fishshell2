
#ifndef _plug_bmp_h
#define _plug_bmp_h

#include "fat2.h"

extern bool PlugBmp_Start(FAT_FILE *_FileHandle);
extern void PlugBmp_Free(void);
extern void PlugBmp_GetBitmap24(u32 LineY,u8 *pBM);
extern s32 PlugBmp_GetWidth(void);
extern s32 PlugBmp_GetHeight(void);
extern int PlugBmp_GetInfoIndexCount(void);
extern bool PlugBmp_GetInfoStrL(int idx,char *str,int len);
extern bool PlugBmp_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool PlugBmp_GetInfoStrUTF8(int idx,char *str,int len);

#endif
