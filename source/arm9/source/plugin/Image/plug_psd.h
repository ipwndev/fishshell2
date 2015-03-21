
#ifndef _plug_psd_h
#define _plug_psd_h

#include "fat2.h"

extern bool PlugPsd_Start(FAT_FILE *_FileHandle);
extern void PlugPsd_Free(void);
extern void PlugPsd_GetBitmap24(u32 LineY,u8 *pBM);
extern s32 PlugPsd_GetWidth(void);
extern s32 PlugPsd_GetHeight(void);
extern int PlugPsd_GetInfoIndexCount(void);
extern bool PlugPsd_GetInfoStrL(int idx,char *str,int len);
extern bool PlugPsd_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool PlugPsd_GetInfoStrUTF8(int idx,char *str,int len);

#endif
